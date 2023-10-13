
#ifndef TFTDisplay_h
#define TFTDisplay_h

#include "data_packet.h"
#include "messages.h"

#define DATA_LINE_CHARS 161

void tft_setup();
void tft_display_data(const char text[DATA_LINE_CHARS]);

void tft_get_data_line(const packet_t *data, unsigned int, unsigned int, char text[DATA_LINE_CHARS]);
void tft_get_data_line(const data_message_t *data, unsigned int, unsigned int, char text[DATA_LINE_CHARS]);

void tft_time_request(const time_request_t *data, unsigned int min, unsigned int sec, char text[DATA_LINE_CHARS]);

#endif
