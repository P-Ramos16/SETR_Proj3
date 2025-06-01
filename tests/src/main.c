#include <ztest.h>

static void test_assert(void) {
	zassert_equal(1, 2, "1 was not equal to 2");
}

void test_main(void) {
	ztest_test_suite(proj3_tests,
					 ztest_unit_test(test_assert));

	ztest_run_test_suite(proj3_tests);
}