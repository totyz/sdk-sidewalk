#include <zephyr/ztest.h>
#include <sid_pal_assert_ifc.h>
#include <stdbool.h>
#include <zephyr/toolchain.h>

/******************************************************************
* sid_pal_assert_ifc
* ****************************************************************/
static bool should_assert;
void assert_post_action(const char *file, unsigned int line)
{
	ARG_UNUSED(file);
	ARG_UNUSED(line);

	if (should_assert) {
		should_assert = false;
		ztest_test_pass();
	}
	ztest_test_fail();
}

static void test_sid_pal_assert(void)
{
	should_assert = false;
	SID_PAL_ASSERT(true);

	should_assert = true;
	SID_PAL_ASSERT(false);
	zassert_false(should_assert, "No assert when it should be.");
}

ZTEST_SUITE(sid_pal_assert, NULL, NULL, NULL, NULL, NULL);

ZTEST(sid_pal_assert, test_sid_pal_assert)
{
	test_sid_pal_assert();
}