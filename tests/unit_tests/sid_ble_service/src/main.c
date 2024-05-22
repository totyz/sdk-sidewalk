#include <zephyr/ztest.h>
#include <zephyr/fff.h>
#include <sid_ble_service.h>
#include <sid_ble_connection.h>
#include <sid_ble_ama_service.h>
#include <sid_ble_adapter_callbacks.h>
#include <zephyr/bluetooth/conn.h>
#include <stdbool.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(uint16_t, bt_gatt_get_mtu, struct bt_conn *);
FAKE_VALUE_FUNC(bool, bt_gatt_is_subscribed, struct bt_conn *, const struct bt_gatt_attr *, uint16_t);
FAKE_VALUE_FUNC(int, bt_gatt_notify_cb, struct bt_conn *, struct bt_gatt_notify_params *);
FAKE_VALUE_FUNC(struct bt_gatt_attr *, bt_gatt_find_by_uuid, const struct bt_gatt_attr *, uint16_t, const struct bt_uuid *);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_service, struct bt_conn *, const struct bt_gatt_attr *, void *, uint16_t, uint16_t);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_chrc, struct bt_conn *, const struct bt_gatt_attr *, void *, uint16_t, uint16_t);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_ccc, struct bt_conn *, const struct bt_gatt_attr *, void *, uint16_t, uint16_t);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_write_ccc, struct bt_conn *, const struct bt_gatt_attr *, const void *, uint16_t, uint16_t, uint8_t);

#define TEST_DATA_CHUNK (128)

struct bt_conn {
	uint8_t dummy;
};

static void reset_fakes(void *fixture)
{
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
}

ZTEST_SUITE(sid_ble_tests, NULL, NULL, reset_fakes, NULL, NULL);

ZTEST(sid_ble_tests, test_sid_ble_send_data_null_ptr)
{
	uint8_t data[TEST_DATA_CHUNK];
	zassert_equal(-ENOENT, sid_ble_send_data(NULL, data, sizeof(data)), NULL);
}

ZTEST(sid_ble_tests, test_sid_ble_send_data_pass)
{
	struct bt_gatt_notify_params *notify_params;
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	struct bt_gatt_attr attr;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = true;
	bt_gatt_notify_cb_fake.return_val = 0;
	zassert_equal(0, sid_ble_send_data(&params, data, sizeof(data)), NULL);

	zassert_not_null(bt_gatt_notify_cb_fake.arg1_val, NULL);
	if (NULL != bt_gatt_notify_cb_fake.arg1_val) {
		notify_params = (struct bt_gatt_notify_params *)bt_gatt_notify_cb_fake.arg1_val;
		notify_params->func(&conn, NULL);
	}
}

ZTEST(sid_ble_tests, test_sid_ble_send_data_attr_fail)
{
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = NULL;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = true;
	bt_gatt_notify_cb_fake.return_val = 0;
	zassert_equal(-ENOENT, sid_ble_send_data(&params, data, sizeof(data)), NULL);
}

ZTEST(sid_ble_tests, test_sid_ble_send_data_wo_subscription)
{
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	struct bt_gatt_attr attr;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = false;
	bt_gatt_notify_cb_fake.return_val = 0;
	zassert_equal(-EINVAL, sid_ble_send_data(&params, data, sizeof(data)), NULL);
}

ZTEST(sid_ble_tests, test_sid_ble_send_data_incorrect_data_len)
{
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	struct bt_gatt_attr attr;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data) - 5;
	bt_gatt_is_subscribed_fake.return_val = false;
	bt_gatt_notify_cb_fake.return_val = 0;
	zassert_equal(-EINVAL, sid_ble_send_data(&params, data, sizeof(data)), NULL);

	bt_gatt_get_mtu_fake.return_val = sizeof(data) - 1;
	zassert_equal(-EINVAL, sid_ble_send_data(&params, data, sizeof(data)), NULL);
}

ZTEST(sid_ble_tests, test_sid_ble_send_data_fail)
{
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	struct bt_gatt_attr attr;
	int test_error_code = -ENOENT;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = true;
	bt_gatt_notify_cb_fake.return_val = test_error_code;
	zassert_equal(test_error_code, sid_ble_send_data(&params, data, sizeof(data)), NULL);
}

ZTEST(sid_ble_tests, test_sid_ble_send_data_incorrect_arguments)
{
	struct bt_gatt_service_static srv;
	struct bt_conn conn;
	sid_ble_srv_params_t params;
	struct bt_gatt_attr attr;
	uint8_t data[TEST_DATA_CHUNK];

	params.conn = &conn;
	params.service = &srv;
	params.uuid = AMA_SID_BT_CHARACTERISTIC_NOTIFY;

	bt_gatt_find_by_uuid_fake.return_val = &attr;
	bt_gatt_get_mtu_fake.return_val = sizeof(data);
	bt_gatt_is_subscribed_fake.return_val = false;
	bt_gatt_notify_cb_fake.return_val = 0;
	zassert_equal(-EINVAL, sid_ble_send_data(&params, NULL, 0), NULL);
	zassert_equal(-EINVAL, sid_ble_send_data(&params, data, 0), NULL);
	zassert_equal(-EINVAL, sid_ble_send_data(&params, NULL, sizeof(data)), NULL);
}