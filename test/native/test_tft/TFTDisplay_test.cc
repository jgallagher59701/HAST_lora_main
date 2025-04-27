#include "TFTDisplay.h"

#include <unity.h>

#include "messages.h"

void test_tft_get_data_line() {
    data_message_t data;
    build_data_message(&data, 1, 1, 1, 1, 1, 1, 1, 1);

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
