
#include <unity.h>
#include "messages.h"

void test_jojn_request_type() {
    join_request_t jr;
    uint64_t dev_eui = 0xff00ff00ff00ff00;
    build_join_request(&jr, dev_eui);
    
    TEST_ASSERT_EQUAL(join_request, get_message_type((void*)&jr));
}


int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_jojn_request_type);

    UNITY_END();
}
