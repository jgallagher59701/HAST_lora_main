/*
  Main node for the HAST project's leaf node sensor.
  6/27/2020
  by jhrg

  Modified to use the ESP8266 'NodeMCU' board since the AT328 does not
  have enough memory for the SD card, LoRa and DS3231 clock.
  10/24/20

  And modified again to use the Adafruit Feather M0 (an SAMD21 board
  with LoRa).
  3/25/21
*/

#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <Arduino.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <RTClib.h>
#include <SPI.h>
#include <SdFat.h>
#include <Wire.h>

#include "TFTDisplay.h"
#include "data_packet.h"
#include "messages.h"

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

#if BUILD_ESP8266_NODEMCU

#define RFM95_RST D0 // GPIO 16
#define RFM95_INT D2 // GPIO 4
#define RFM95_CS D8  // GPIO 15

#define I2C_SDA D3 // GPIO 0
#define I2C_SCL D1 // GPIO 5

#define SD_CS 10 // SDD3

#define BAUD_RATE 115200

#elif FEATHER_M0

#define RFM95_INT 3
#define RFM95_CS 8
#define RFM95_RST 4

#define I2C_SDA 20
#define I2C_SCL 21

#define SD_CS 10

#define BAUD_RATE 115200

#else

#error "Must define on of FEATHER_M0 or BUILD_ESP8266_NODEMCU"

#endif

// Real time clock
RTC_DS3231 DS3231; // we are using the DS3231 RTC

// Since this is a compile-time option, first set it, compile and upload the
// code. That will set the time to something close to the real time (there is
// some error due to the upload time). Then clear the option, rebuild and
// upload the code _again_. That will enable restart of the main node so it
// uses the time value saved by the DS3231 (using the battery backup).
//
// Set the value using the platformio.ini file. jhrg 6/25/23
#ifndef ADJUST_TIME
#define ADJUST_TIME 0
#endif

#define MAIN_NODE_ADDRESS 0

// #define FREQUENCY 915.0
#define FREQUENCY 902.3
#define SIGNAL_STRENGTH 13 // dBm

// Use these settings:
#define BANDWIDTH 125000
#define SPREADING_FACTOR 10
#define CODING_RATE 5
// RH_CAD_DEFAULT_TIMEOUT 10seconds

// Should the main node send a reply to a leaf node? If so, that
// reply will be a time code and the leaf node may reset its internal
// clock to that time. If this time is in the past relative to the
// leaf node's boot time value, it may never wake up.
#define REPLY 0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
// Singleton instance for the reliable datagram manager
RHReliableDatagram rf95_manager(rf95, MAIN_NODE_ADDRESS);

// Singletons for the SD card objects
SdFat sd;    // File system object.
SdFile file; // Log file.

#define FILE_NAME "Sensor_data.csv"

bool sd_card_status = false; // true == SD card init'd

// Given a DateTime instance, return a pointer to static string that holds
// an ISO 8601 print representation of the object.

char *iso8601_date_time(DateTime &t) {
    static char date_time_str[32];

    char val[12];
    date_time_str[0] = '\0';
    strncat(date_time_str, itoa(t.year(), val, 10), sizeof(date_time_str) - 1);
    strncat(date_time_str, "-", sizeof(date_time_str) - 1);
    if (t.month() < 10)
        strncat(date_time_str, "0", sizeof(date_time_str) - 1);
    strncat(date_time_str, itoa(t.month(), val, 10), sizeof(date_time_str) - 1);
    strncat(date_time_str, "-", sizeof(date_time_str) - 1);
    if (t.day() < 10)
        strncat(date_time_str, "0", sizeof(date_time_str) - 1);
    strncat(date_time_str, itoa(t.day(), val, 10), sizeof(date_time_str) - 1);
    strncat(date_time_str, "T", sizeof(date_time_str) - 1);
    if (t.hour() < 10)
        strncat(date_time_str, "0", sizeof(date_time_str) - 1);
    strncat(date_time_str, itoa(t.hour(), val, 10), sizeof(date_time_str) - 1);
    strncat(date_time_str, ":", sizeof(date_time_str) - 1);
    if (t.minute() < 10)
        strncat(date_time_str, "0", sizeof(date_time_str) - 1);
    strncat(date_time_str, itoa(t.minute(), val, 10), sizeof(date_time_str) - 1);
    strncat(date_time_str, ":", sizeof(date_time_str) - 1);
    if (t.second() < 10)
        strncat(date_time_str, "0", sizeof(date_time_str) - 1);
    strncat(date_time_str, itoa(t.second(), val, 10), sizeof(date_time_str) - 1);

    return date_time_str;
}

/**
    @brief RF95 off the SPI bus to enable SD card access
*/
void yield_spi_to_sd() {
    //digitalWrite(SD_CS, LOW);
    digitalWrite(RFM95_CS, HIGH);
}

/**
    @brief RF95 off the SPI bus to enable SD card access
*/
void yield_spi_to_rf95() {
    digitalWrite(SD_CS, HIGH);
    //digitalWrite(RFM95_CS, LOW);
}

/**
   @brief Write a header for the new log file.
   @param file_name open/create this file, append if it exists
   @note Claim the SPI bus
*/
void write_header(const char *file_name) {
    if (!sd_card_status)
        return;

    yield_spi_to_sd();

    noInterrupts(); // disable interrupts

    if (!file.open(file_name, O_WRONLY | O_CREAT | O_APPEND)) {
        Serial.println(F("Couldn't write file header"));
        sd_card_status = false;
        return;
    }

    file.println(F("# Start Log"));
    file.println(F("Node, Message, Time, Battery V, Last TX Dur ms, Temp C, Hum %, Status"));
    file.close();

    interrupts(); // enable interrupts
}

/**
   @brief log data
   write data to the log, append a new line
   @param file_name open for append
   @param data write this char string
   @note Claim the SPI bus (calls yield_spi_to_sd()().
*/
void log_data(const char *file_name, const char *data) {
    if (!sd_card_status)
        return;

    yield_spi_to_sd();
    noInterrupts(); // disable interrupts

    if (file.open(file_name, O_WRONLY | O_CREAT | O_APPEND)) {
        file.println(data);
        file.close();
    } else {
        Serial.print(F("Failed to log data."));
    }

    interrupts(); // enable interrupts
}

void status_on() {
    digitalWrite(LED_BUILTIN, HIGH);
}

void status_off() {
    digitalWrite(LED_BUILTIN, LOW);
}

void yield(unsigned long duration_ms) {
    unsigned long start = millis();
    do {
        yield();
    } while (millis() - start < duration_ms);
}

void print_rfm95_info() {
    Serial.print(F("RSSI "));
    Serial.print(rf95.lastRssi(), DEC);
    Serial.print(F(" dBm, SNR "));
    Serial.print(rf95.lastSNR(), DEC);
    Serial.print(F(" dB, good/bad packets: "));
    Serial.print(rf95.rxGood(), DEC);
    Serial.print(F("/"));
    Serial.println(rf95.rxBad(), DEC);
}

#define SERIAL_WAIT_TIME 10000      // 10s
#define ONE_SECOND 1000             // ms

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RFM95_RST, OUTPUT);
    pinMode(RFM95_CS, OUTPUT);

    long start_time = millis();
    Serial.begin(BAUD_RATE);
    // wait for the Serial interface to come up or for the SERIAL_WAIT_TIME
    while (!Serial && (millis() - start_time < SERIAL_WAIT_TIME)) {
        status_on();
        delay(ONE_SECOND);
        status_off();
        delay(ONE_SECOND);
    }

    Serial.println(F("boot"));

    int sda = I2C_SDA;
    int scl = I2C_SCL;
    Wire.begin(sda, scl);

    tft_setup();

    // Initialize the SD card
    yield_spi_to_sd();

    Serial.print(F("Initializing SD card..."));

    // Initialize at the highest speed supported by the board that is
    // not over 50 MHz. Try a lower speed if SPI errors occur.
    if (sd.begin(SD_CS, SPI_HALF_SPEED)) { //, SD_SCK_MHZ(50))) {
        Serial.println(F(" OK"));
        sd_card_status = true;
    } else {
        Serial.println(F(" Couldn't init the SD Card"));
        sd_card_status = false;
    }

    // Write data header.
    write_header(FILE_NAME);

    yield_spi_to_rf95();

    Serial.print(F("Starting receiver..."));

    // LORA manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(20);
    digitalWrite(RFM95_RST, HIGH);
    delay(20);

    if (rf95_manager.init()) {
        Serial.println(F(" OK"));

        // 6 octets + preamble at SF = 10, CR = 5, BW = 125kHz is 327ms
        // or 207ms, depending on who you ask... Either way, it's more than
        // 200, which is the default. Setting the timeout to 400ms seems
        // to improve the success rate at getting the replay back to the
        // leaf node. jhrg 11/4/20
        rf95_manager.setTimeout(400);

        // Setup ISM FREQUENCY
        rf95.setFrequency(FREQUENCY);
        // Setup Power,dBm
        rf95.setTxPower(SIGNAL_STRENGTH);
        // Setup Spreading Factor (CPS == 2^n, N is 6 ~ 12)
        rf95.setSpreadingFactor(SPREADING_FACTOR);
        // Setup BandWidth, option: 7800,10400,15600,20800,31200,41700,62500,125000,250000,500000
        rf95.setSignalBandwidth(BANDWIDTH);
        // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8)
        rf95.setCodingRate4(CODING_RATE);
        // Set the CAD timeout to 10s
        rf95.setCADTimeout(RH_CAD_DEFAULT_TIMEOUT);

        Serial.print(F("Listening on frequency: "));
        Serial.println(FREQUENCY);
    } else {
        Serial.println(F(" receiver initialization failed"));
    }

    if (!DS3231.begin()) {
        Serial.println("Couldn't find DS3231");
    }

    if (DS3231.lostPower() || ADJUST_TIME) {
        if (ADJUST_TIME)
            Serial.println("ADJUST_TIME option set, set time to compiled value");
        else
            Serial.println("RTC lost power, let's set the time!");
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        DS3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    Serial.print(F("Startup time: "));
    DateTime t = DS3231.now();
    Serial.println(iso8601_date_time(t));

    Serial.flush();

    status_off();
}

/**
 * @brief Send a reply that includes a time code (unixtime)
 * @note Resets the rf95 manager's retransmissions counter
 * @param from The node number
 * @param msg
 */
void send_time_as_reply(uint8_t from)
{
    char msg[RH_RF95_MAX_MESSAGE_LEN];
    yield_spi_to_rf95();
    uint32_t now = DS3231.now().unixtime();
    unsigned long start = millis();
    if (rf95_manager.sendtoWait((uint8_t *)&now, sizeof(now), from)) {
        snprintf(msg, RH_RF95_MAX_MESSAGE_LEN, "...sent a reply, %ld retransmissions, %ld ms", rf95_manager.retransmissions(),
                 millis() - start);
        Serial.println(msg);
        Serial.flush();
    } else {
        snprintf(msg, RH_RF95_MAX_MESSAGE_LEN, "...reply failed, %ld retransmissions, %ld ms", rf95_manager.retransmissions(),
                 millis() - start);
        Serial.println(msg);
        Serial.flush();
    }

    rf95_manager.resetRetransmissions();
}

#define MSG_LEN 128

/**
 * @brief Send the response to a time request
 * @note Resets the rf95 manager's retransmissions counter
 * @param to The node number
 */
void send_time_response(uint8_t to)
{
    yield_spi_to_rf95();

    time_response_t tr;
    build_time_response(&tr, MAIN_NODE_ADDRESS, DS3231.now().unixtime());

    unsigned long start = millis();
    if (rf95_manager.sendtoWait((uint8_t *)&tr, sizeof(time_response_t), to)) {
        char msg[MSG_LEN];
        snprintf(msg, MSG_LEN, "...sent a reply, %ld retransmissions, %ld ms", rf95_manager.retransmissions(), millis() - start);
        Serial.println(msg);
        Serial.flush();
    } else {
        char msg[MSG_LEN];
        snprintf(msg, MSG_LEN, "...reply failed, %ld retransmissions, %ld ms", rf95_manager.retransmissions(), millis() - start);
        Serial.println(msg);
        Serial.flush();
    }

    rf95_manager.resetRetransmissions();
}

uint8_t rf95_buf[RH_RF95_MAX_MESSAGE_LEN];

void loop() {
    yield_spi_to_rf95();

    if (rf95_manager.available()) {
        status_on();

        Serial.println();
        Serial.print(F("Current time: "));
        DateTime t = DS3231.now();
        Serial.println(iso8601_date_time(t));

        uint8_t len = sizeof(rf95_buf);
        uint8_t from, to, id, header;
        char msg[256];
        if (rf95_manager.recvfromAck(rf95_buf, &len, &from, &to, &id, &header)) {

            if (len == sizeof(packet_t)) {  // Backward compatibility hack for packet_t
                snprintf(msg, 256,
                         "Received length: %d, from: 0x%02x, to: 0x%02x, id: 0x%02x, header: 0x%02x, type: %s",
                         len, from, to, id, header, "data packet");
            }
            else {
                snprintf(msg, 256,
                         "Received length: %d, from: 0x%02x, to: 0x%02x, id: 0x%02x, header: 0x%02x, type: %s",
                         len, from, to, id, header,
                         get_message_type_string(get_message_type((char *) rf95_buf)));

            }

            Serial.println(msg);
            Serial.flush();

            MessageType type = (len == sizeof(packet_t)) ? data_packet : get_message_type((char *)rf95_buf);

            switch (type) {
                case data_packet: {             // Compatibility with the original packet_t
                    // Print received packet
                    Serial.print(F("Data: "));
                    Serial.print(data_packet_to_string((packet_t *)rf95_buf, /* pretty */ true));

                    Serial.print(F(", "));
                    print_rfm95_info();

                    // log reading to the SD card
                    const char *pretty_buf = data_packet_to_string((packet_t *)rf95_buf, false);
                    log_data(FILE_NAME, pretty_buf);

#if REPLY
                    send_time_as_reply(from);
#endif

                    char text[DATA_LINE_CHARS];
                    tft_get_data_line((packet_t *)rf95_buf, DS3231.now().minute(), DS3231.now().second(), text);
                    tft_display_data(text);

                    break;
                }

                // This case depends on changes in soil_sensor_common on the message_changes branch
                // jhrg 6/25/23
                case data_message: {            // New data message with type indicator
                    // Print received packet
                    Serial.print(F("Data: "));
                    Serial.print(data_message_to_string((data_message_t *)rf95_buf, /* pretty */ true));

                    Serial.print(F(", "));
                    print_rfm95_info();

                    // log reading to the SD card, not pretty-printed
                    const char *buf = data_message_to_string((data_message_t *)rf95_buf, false);
                    log_data(FILE_NAME, buf);

                    char text[DATA_LINE_CHARS];
                    tft_get_data_line((data_message_t *)rf95_buf, DS3231.now().minute(), DS3231.now().second(), text);
                    tft_display_data(text);

                    break;
                }

                case text: {
                    Serial.print(F("Got: "));
                    // Add a null to the end of the packet and print as text
                    //rf95_buf[len] = 0;
                    Serial.println(text_message_to_string((text_t *)rf95_buf, true /*pretty*/));

                    Serial.print(F("RFM95 info: "));
                    print_rfm95_info();

                    log_data(FILE_NAME, text_message_to_string((text_t *)rf95_buf, false /*pretty*/));
                    break;
                }
 
                case join_request:
                    // extract the EUI. Record it and assign a byte node number.
                    break;

                case time_request: {
                    Serial.print(F("Time request: "));
                    Serial.print(time_request_to_string((time_request_t *)rf95_buf, /* pretty */ true));

                    Serial.print(F(", "));
                    print_rfm95_info();

                    // log reading to the SD card, not pretty-printed
                    const char *buf = time_request_to_string((time_request_t *)rf95_buf, false);
                    log_data(FILE_NAME, buf);
                    break;
                }


                default:
                    Serial.println(F("Got unrecognized message."));
            }
        }

        status_off();
    }
}
