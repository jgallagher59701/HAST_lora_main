/**************************************************************************
  Functions to display information from leaf nodes on a 1.8" TFT display.
  Hack from the library described by the rest of this header. 

  This code implements a simple display that shows a header and 11 lines 
  of text, one line for each leaf node data packet received. The display
  adds the newest line at the bottom after scrolling the existing lines
  up.

  James Gallagher 1/19/22
 
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
    ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
    ----> https://www.adafruit.com/product/2088
  The 1.14" TFT breakout
  ----> https://www.adafruit.com/product/4383
  The 1.3" TFT breakout
  ----> https://www.adafruit.com/product/4313
  The 1.54" TFT breakout
    ----> https://www.adafruit.com/product/3787
  The 1.69" TFT breakout
    ----> https://www.adafruit.com/product/5206
  The 2.0" TFT breakout
    ----> https://www.adafruit.com/product/4311
  as well as Adafruit raw 1.8" TFT display
    ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

#include <SPI.h>
#include <RTClib.h>          // Access time info from the main node.

#include "data_packet.h"     // Decode information in a data packet

// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT_CS         6
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         5

extern RTC_DS3231 DS3231;    // see main-node.cc

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

// For 1.44" and 1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// OPTION 2 lets you interface the display using ANY TWO or THREE PINS,
// tradeoff being that performance is not as fast as hardware SPI above.
//#define TFT_MOSI 11  // Data out
//#define TFT_SCLK 13  // Clock out

// For ST7735-based displays, we will use this call
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

/**
 * @brief Write the leaf node data header.
 * 
 * This refreshess the display and names the columns of data. Once this is
 * called, all the lines will need to be redrawn. Start the header indented
 * 2 pixels and with 3 blank rows of pixels at the top.
 */
void tft_display_header() 
{
    tft.fillScreen(ST77XX_BLACK);

    tft.setCursor(2, 3);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(1); 
    tft.println("Node time  C  %rh bat stat");

    tft.drawFastHLine(2, 13, tft.width()-4, ST77XX_RED);
}

#define DATA_LINE_CHARS 161

/**
 * @brief Write decoded packet values to display to 'text'
 */
void tft_get_data_line(const packet_t *data, char text[DATA_LINE_CHARS])
{
    uint8_t node;
    uint32_t message; // not used
    uint32_t time;  // not used
    uint16_t battery;
    uint16_t last_tx_duration; // not used
    int16_t temp;
    uint16_t humidity;
    uint8_t status;

    parse_data_packet(data, &node, &message, &time, &battery, &last_tx_duration, &temp, &humidity, &status);

    unsigned int min = DS3231.now().minute();
    unsigned int sec = DS3231.now().second();

    // write the current line of text to the array 'text'
    snprintf((char *)text, DATA_LINE_CHARS, "%u %02u:%02u %3.1f %u %3.2f 0x%02x",
                 node, min, sec, temp/100.0, humidity/100, battery/100.0, (unsigned int)status);

}

#define MAX_LINE 11          // 11 lines can be displayed

// Global storage holds the line text and currnet line.
char tft_line_text[MAX_LINE][DATA_LINE_CHARS] = {};
unsigned int tft_current_line = 0;

/**
 * @brief Display information in the packet to the TFT.
 * 
 * The display can show the header, which is underlined, and 11 lines of
 * text below that using the default font. Each line of text has two rows
 * of 'leading.' so the height of a text line is 10 pixels. Because the header
 * starts 2 pixels from the left edge and 3 from the top, the first row of 
 * text starts at 2, 15 and each subsequent row is 10 pixels more below that
 * (2 and 25, 2 and 35, ...).
 */
void tft_display_data_packet(const char text[DATA_LINE_CHARS])
{
    // copy the current line of text so it will persist between calls
    memcpy(tft_line_text[tft_current_line], text, DATA_LINE_CHARS); 

    // display header
    tft_display_header();

    // Change color and (re)write the lines, including the current line
    tft.setTextColor(ST77XX_GREEN);
    for (unsigned int line = 0; line <= tft_current_line; ++line) {
        tft.setCursor(2, 15 + (10 * line));
        tft.println(tft_line_text[line]);
    }
    
    // once the screen is full, only add a line at the bottom,
    // so tft_current_line doesn't get updated and stays at 10. Once
    // that happens, start 'scrolling' the lines up.
    if (tft_current_line < MAX_LINE-1) {
        ++tft_current_line;
    }
    else {
        // scroll lines up if we need to make space for the next line
        // when this function is called again.
        for (unsigned int line = 0; line < MAX_LINE-1; ++line) {
            memcpy(tft_line_text[line], tft_line_text[line+1], DATA_LINE_CHARS);
        }
    }
}

#define LANDSCAPE_1 1        // landscape with upper left near pins; used in tft_setup()

void tft_setup() 
{
    // If the serial port is connected and initialized ...
    Serial.print(F("Hello! ST77xx TFT Test..."));

    // Use this initializer if using a 1.8" TFT screen:
    tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

    // OR use this initializer if using a 1.8" TFT screen with offset such as WaveShare:
    // tft.initR(INITR_GREENTAB);      // Init ST7735S chip, green tab

    // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
    // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
    // may end up with a black screen some times, or all the time.
    // tft.setSPISpeed(40000000);

    Serial.println(F(" TFT Initialized"));

    tft.setRotation(LANDSCAPE_1);
    tft.setTextWrap(false);
    tft_display_header();
}
