#include <zephyr/ztest.h>
#include <zephyr/fff.h>

#include <sid_ble_advert.h>
#include <sid_ble_uuid.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <errno.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, bt_le_adv_start, const struct bt_le_adv_param *, const struct bt_data *,
                size_t, const struct bt_data *, size_t);
FAKE_VALUE_FUNC(int, bt_le_adv_stop);
FAKE_VALUE_FUNC(int, bt_le_adv_update_data, const struct bt_data *, size_t, const struct bt_data *,
                size_t);

#define FFF_FAKES_LIST(FAKE)                                                                       \
        FAKE(bt_le_adv_start)                                                                      \
        FAKE(bt_le_adv_stop)                                                                       \
        FAKE(bt_le_adv_update_data)

#define ESUCCESS (0)
#define TEST_BUFFER_LEN (100)
#define BT_COMP_ID_LEN 2

static void setup(void *f)
{
        FFF_FAKES_LIST(RESET_FAKE);
        FFF_RESET_HISTORY();
}

ZTEST_SUITE(sid_ble_advert, NULL, NULL, setup, NULL, NULL);

ZTEST(sid_ble_advert, test_sid_ble_advert_start)
{
        size_t adv_start_call_count = 0;

        bt_le_adv_start_fake.return_val = ESUCCESS;
        zassert_equal(ESUCCESS, sid_ble_advert_start(), NULL);
        adv_start_call_count++;
        zassert_equal(adv_start_call_count, bt_le_adv_start_fake.call_count, NULL);

        bt_le_adv_start_fake.return_val = -ENOENT;
        zassert_equal(-ENOENT, sid_ble_advert_start(), NULL);
        adv_start_call_count++;
        zassert_equal(adv_start_call_count, bt_le_adv_start_fake.call_count, NULL);
}

ZTEST(sid_ble_advert, test_sid_ble_advert_stop)
{
        size_t adv_stop_call_count = 0;

        bt_le_adv_stop_fake.return_val = ESUCCESS;
        zassert_equal(ESUCCESS, sid_ble_advert_stop(), NULL);
        adv_stop_call_count++;
        zassert_equal(adv_stop_call_count, bt_le_adv_stop_fake.call_count, NULL);

        bt_le_adv_stop_fake.return_val = -ENOENT;
        zassert_equal(-ENOENT, sid_ble_advert_stop(), NULL);
        adv_stop_call_count++;
        zassert_equal(adv_stop_call_count, bt_le_adv_stop_fake.call_count, NULL);
}

ZTEST(sid_ble_advert, test_sid_ble_advert_update)
{
        uint8_t test_data[] = "Lorem ipsum.";
        size_t adv_update_call_count = 0;

        bt_le_adv_start_fake.return_val = ESUCCESS;
        bt_le_adv_update_data_fake.return_val = ESUCCESS;
        zassert_equal(ESUCCESS, sid_ble_advert_start(), NULL);
        zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)), NULL);
        adv_update_call_count++;
        zassert_equal(adv_update_call_count, bt_le_adv_update_data_fake.call_count, NULL);

        bt_le_adv_update_data_fake.return_val = -ENOENT;
        zassert_equal(-ENOENT, sid_ble_advert_update(test_data, sizeof(test_data)), NULL);
        adv_update_call_count++;
        zassert_equal(adv_update_call_count, bt_le_adv_update_data_fake.call_count, NULL);

        bt_le_adv_update_data_fake.return_val = ESUCCESS;
        zassert_equal(-EINVAL, sid_ble_advert_update(NULL, sizeof(test_data)), NULL);
        zassert_equal(-EINVAL, sid_ble_advert_update(test_data, 0), NULL);
}

ZTEST(sid_ble_advert, test_sid_ble_advert_update_in_every_state)
{
        uint8_t test_data[] = "Lorem ipsum.";

        zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)), NULL);

        bt_le_adv_start_fake.return_val = ESUCCESS;
        bt_le_adv_update_data_fake.return_val = ESUCCESS;
        zassert_equal(ESUCCESS, sid_ble_advert_start(), NULL);
        zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)), NULL);

        bt_le_adv_stop_fake.return_val = ESUCCESS;
        zassert_equal(ESUCCESS, sid_ble_advert_stop(), NULL);
        zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)), NULL);
}

bool advert_data_manuf_data_get(const struct bt_data *ad, size_t ad_len, uint8_t *result,
                                uint8_t *result_len)
{
        for (size_t i = 0; i < ad_len; i++) {
                if (ad[i].type == BT_DATA_MANUFACTURER_DATA) {
                        zassert_true(ad[i].data_len >= BT_COMP_ID_LEN, NULL);
                        zassert_equal(((BT_COMP_ID_AMA) & 0xff), ad[i].data[0], NULL);
                        zassert_equal((((BT_COMP_ID_AMA) >> 8) & 0xff), ad[i].data[1], NULL);

                        *result_len = ad[i].data_len - BT_COMP_ID_LEN;
                        memcpy(result, &ad[i].data[BT_COMP_ID_LEN], *result_len);
                        return true;
                }
        }
        return false;
}

ZTEST(sid_ble_advert, test_sid_ble_advert_update_before_start)
{
        uint8_t test_result[TEST_BUFFER_LEN] = { 0 };
        uint8_t test_result_size;
        uint8_t test_data[] = "Lorem ipsum.";
        const struct bt_data *advert_data;
        size_t advert_data_size;
        bool found;

        bt_le_adv_stop_fake.return_val = ESUCCESS;
        zassert_equal(ESUCCESS, sid_ble_advert_stop(), NULL);

        bt_le_adv_update_data_fake.return_val = ESUCCESS;
        zassert_equal(ESUCCESS, sid_ble_advert_update(test_data, sizeof(test_data)), NULL);

        bt_le_adv_start_fake.return_val = ESUCCESS;
        zassert_equal(ESUCCESS, sid_ble_advert_start(), NULL);

        advert_data = bt_le_adv_start_fake.arg1_val;
        advert_data_size = bt_le_adv_start_fake.arg2_val;
        zassert_not_null(advert_data, NULL);
        zassert_true(advert_data_size > 0, NULL);

        found = advert_data_manuf_data_get(advert_data, advert_data_size, test_result,
                                           &test_result_size);
        zassert_true(found, "Manufacturer data not found in advertising data.");
        zassert_equal(sizeof(test_data), test_result_size, NULL);
        zassert_mem_equal(test_data, test_result, sizeof(test_data), NULL);
}

void check_sid_ble_advert_update(uint8_t *data, uint8_t data_len)
{
        uint8_t test_result[TEST_BUFFER_LEN] = { 0 };
        uint8_t test_result_size;
        const struct bt_data *advert_data;
        size_t advert_data_size;
        bool found;

        bt_le_adv_start_fake.return_val = ESUCCESS;
        bt_le_adv_update_data_fake.return_val = ESUCCESS;
        zassert_equal(ESUCCESS, sid_ble_advert_start(), NULL);
        zassert_equal(ESUCCESS, sid_ble_advert_update(data, data_len), NULL);

        advert_data = bt_le_adv_update_data_fake.arg0_val;
        advert_data_size = bt_le_adv_update_data_fake.arg1_val;
        zassert_not_null(advert_data, NULL);
        zassert_true(advert_data_size > 0, NULL);

        found = advert_data_manuf_data_get(advert_data, advert_data_size, test_result,
                                           &test_result_size);
        zassert_true(found, "Manufacturer data not found in advertising data.");
        zassert_equal(data_len, test_result_size, NULL);
        zassert_mem_equal(data, test_result, data_len, NULL);
}

ZTEST(sid_ble_advert, test_sid_ble_advert_update_value)
{
        uint8_t test_data[] = "normal";
        uint8_t test_data_short[] = "s";
        uint8_t test_data_very_long[] = "very long data";

        check_sid_ble_advert_update(test_data, sizeof(test_data));
        check_sid_ble_advert_update(test_data_short, sizeof(test_data_short));
        check_sid_ble_advert_update(test_data_very_long, sizeof(test_data_very_long));
}