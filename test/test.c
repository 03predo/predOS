#include <unity.h>

void setUp (void) {}
void tearDown (void) {}

void test(void){
  TEST_ASSERT_EQUAL(0, 0);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test);

    return UNITY_END();
}
