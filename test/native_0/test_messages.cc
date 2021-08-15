
#include <unity.h>

#include "messages.h"

void test_join_request_type() {
    join_request_t jr;
    uint64_t dev_eui = 0xff00ff00ff00ff00;
    build_join_request(&jr, dev_eui);
    
    TEST_ASSERT_EQUAL(join_request, get_message_type((void*)&jr));
}

void test_parse_join_request() {
    join_request_t jr;
    uint64_t dev_eui = 0;
    build_join_request(&jr, 0xff00ff00ff00ff00);

    TEST_ASSERT_TRUE(parse_join_request(&jr, &dev_eui));
    TEST_ASSERT_EQUAL(0xff00ff00ff00ff00, dev_eui);
}


// join_request_to_string(const join_request_t *jr, bool pretty /*false*/)
void test_join_request_to_string() {
    join_request_t jr;
    uint64_t dev_eui = 0;
    build_join_request(&jr, 0xff00ff00ff00ff00);

    // Since these 'string' functions use their own static storage and
    // these tests compare two different results, copy the results to local
    // storage here.
    char str1[64];
    char *s = join_request_to_string(&jr);
    strncpy(str1, s, 64);
    printf("Join Request as a string: %s\n", str1);
    TEST_ASSERT_TRUE_MESSAGE(strlen(str1) < 64, str1);
       
    char str2[64];
    strncpy(str2, join_request_to_string(&jr, false), 64);
    TEST_ASSERT_TRUE(strcmp(str1, str2) == 0);

    char str3[64];
    strncpy(str3, join_request_to_string(&jr, true), 64);
    printf("Join Request as a string: %s\n", str3);
    TEST_ASSERT_TRUE(strlen(str3) < 64);
    // length of the 'pretty' string (str3) should be longer then the !pretty string
    TEST_ASSERT_TRUE(strlen(str2) < strlen(str3));
}


int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_join_request_type);
    RUN_TEST(test_parse_join_request);
    RUN_TEST(test_join_request_to_string);

    UNITY_END();
}
