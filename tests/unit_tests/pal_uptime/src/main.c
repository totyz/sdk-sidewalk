#include <zephyr/ztest.h>
#include <sid_pal_uptime_ifc.h>
#include <zephyr/sys_clock.h>
#include <zephyr/fff.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(uint64_t, zephyr_uptime_ns);

static void uptime_test_time(uint64_t uptime_nanoseconds)
{
	uint32_t seconds, nanoseconds;
	struct sid_timespec sid_time;

	seconds = (uint32_t)(uptime_nanoseconds / NSEC_PER_SEC);
	nanoseconds = (uint32_t)(uptime_nanoseconds % NSEC_PER_SEC);

	zephyr_uptime_ns_fake.return_val = uptime_nanoseconds;
	zassert_equal(SID_ERROR_NONE, sid_pal_uptime_now(&sid_time), NULL);

	zassert_equal(seconds, sid_time.tv_sec, NULL);
	zassert_equal(nanoseconds, sid_time.tv_nsec, NULL);
}

ZTEST(sid_pal_uptime, test_sid_pal_uptime_get_now)
{
	zassert_equal(SID_ERROR_NULL_POINTER, sid_pal_uptime_now(NULL), NULL);

	uptime_test_time(0ull);
	uptime_test_time(1ull * NSEC_PER_SEC + 10ull);
	uptime_test_time(10ull * NSEC_PER_SEC + 100ull);
	uptime_test_time(LONG_MAX);
	uptime_test_time(LLONG_MAX);
	uptime_test_time(ULLONG_MAX);
}

ZTEST(sid_pal_uptime, test_sid_pal_uptime_accuracy)
{
	/* Note: This is a oversimplified test for dummy implementation.*/
	int16_t ppm;

	ppm = sid_pal_uptime_get_xtal_ppm();
	sid_pal_uptime_set_xtal_ppm(ppm);
	zassert_equal(ppm, sid_pal_uptime_get_xtal_ppm(), NULL);
}

ZTEST_SUITE(sid_pal_uptime, NULL, NULL, NULL, NULL, NULL);