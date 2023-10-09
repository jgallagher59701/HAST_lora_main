#include <unity.h>

#include "TFTDisplay.h"
#include "data_packet.h"

// tft_get_data_line(const packet_t *data, unsigned int min, unsigned int sec, char text[DATA_LINE_CHARS])
#if 0
build_data_packet(packet_t *data, const uint8_t node, const uint32_t message, const uint32_t time,
                       const uint16_t battery, const uint16_t last_tx_duration,
                       const int16_t temp, const uint16_t humidity, const uint8_t status);
#endif

void test_tft_get_data_line() {
    packet_t data;
    build_data_packet(&data, 1, 1, 1, 1, 1, 1, 1, 1);
    char text[DATA_LINE_CHARS];
    tft_get_data_line(&data, 10, 17, text);
    
    //printf("text: %s\n", text);
    TEST_ASSERT(text != nullptr);
    //TEST_ASSERT_EQUAL(join_request, get_message_type((void*)&jr));
}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_tft_get_data_line);
   
    UNITY_END();
}
