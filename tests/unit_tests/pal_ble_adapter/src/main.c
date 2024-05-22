#include <zephyr/ztest.h>
#include <zephyr/fff.h>

#include <sid_pal_ble_adapter_ifc.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/addr.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, bt_enable, bt_ready_cb_t);
FAKE_VALUE_FUNC(int, bt_disable);
FAKE_VALUE_FUNC(int, bt_le_adv_start, const struct bt_le_adv_param *, const struct bt_data *,
		size_t, const struct bt_data *, size_t);

FAKE_VALUE_FUNC(int, bt_le_adv_stop);
FAKE_VOID_FUNC(bt_conn_cb_register, struct bt_conn_cb *);
FAKE_VALUE_FUNC(struct bt_conn *, bt_conn_ref, struct bt_conn *);
FAKE_VOID_FUNC(bt_conn_unref, struct bt_conn *);
FAKE_VALUE_FUNC(const bt_addr_le_t *, bt_conn_get_dst, const struct bt_conn *);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_service, struct bt_conn *, const struct bt_gatt_attr *,
		void *, uint16_t, uint16_t);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_chrc, struct bt_conn *, const struct bt_gatt_attr *,
		void *, uint16_t, uint16_t);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_read_ccc, struct bt_conn *, const struct bt_gatt_attr *,
		void *, uint16_t, uint16_t);
FAKE_VALUE_FUNC(ssize_t, bt_gatt_attr_write_ccc, struct bt_conn *, const struct bt_gatt_attr *,
		const void *, uint16_t, uint16_t, uint8_t);

#define FFF_FAKES_LIST(FAKE)                                                                       \
	FAKE(bt_enable)                                                                            \
	FAKE(bt_disable)                                                                           \
	FAKE(bt_le_adv_start)                                                                      \
	FAKE(bt_le_adv_stop)                                                                       \
	FAKE(bt_conn_cb_register)                                                                  \
	FAKE(bt_conn_ref)                                                                          \
	FAKE(bt_conn_unref)                                                                        \
	FAKE(bt_conn_get_dst)                                                                      \
	FAKE(bt_gatt_attr_read_service)                                                            \
	FAKE(bt_gatt_attr_read_chrc)                                                               \
	FAKE(bt_gatt_attr_read_ccc)                                                                \
	FAKE(bt_gatt_attr_write_ccc)

#define ESUCCESS (0)
#define FAKE_SERVICE (9)
#define TEST_DATA_CHUNK (128)

struct bt_conn {
	uint8_t dummy;
};

typedef struct {
	uint8_t num_calls;
	uint8_t *ptr_data;
	uint8_t data_len;
} data_callback_test_t;

static sid_ble_config_t test_ble_cfg;
static data_callback_test_t data_cb_test;

static void ble_data_callback(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length)
{
	++data_cb_test.num_calls;
	data_cb_test.ptr_data = data;
	data_cb_test.data_len = length;
}

static void ble_notify_callback(sid_ble_cfg_service_identifier_t id, bool state)
{
}

static void connection_callback(bool state, uint8_t *addr)
{
}

static void ble_indication_callback(bool status)
{
}

static void ble_mtu_callback(uint16_t size)
{
}

static void ble_adv_start_callback(void)
{
}

static void set_callbacks(sid_pal_ble_adapter_callbacks_t *cb)
{
	cb->conn_callback = connection_callback;
	cb->mtu_callback = ble_mtu_callback;
	cb->adv_start_callback = ble_adv_start_callback;
	cb->ind_callback = ble_indication_callback;
	cb->data_callback = ble_data_callback;
	cb->notify_callback = ble_notify_callback;
}

static void *setup(void)
{
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
	memset(&data_cb_test, 0x00, sizeof(data_cb_test));
	return NULL;
}

ZTEST_SUITE(sid_pal_ble_adapter, NULL, setup, NULL, NULL, NULL);

ZTEST(sid_pal_ble_adapter, test_sid_pal_ble_adapter_create_negative)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_ble_adapter_create(NULL));
}

ZTEST(sid_pal_ble_adapter, test_sid_pal_ble_adapter_create_positive)
{
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
}

ZTEST(sid_pal_ble_adapter, test_sid_pal_ble_adapter_init)
{
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	bt_enable_fake.return_val = ESUCCESS;

	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->init(&test_ble_cfg));
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->init(NULL));

	bt_enable_fake.return_val = -ENOENT;
	zassert_equal(SID_ERROR_GENERIC, p_test_ble_ifc->init(&test_ble_cfg));

	bt_enable_fake.return_val = ESUCCESS;
	zassert_equal(SID_ERROR_GENERIC, p_test_ble_ifc->init(&test_ble_cfg));
}

ZTEST(sid_pal_ble_adapter, test_sid_pal_ble_adapter_deinit)
{
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->init(&test_ble_cfg));
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->deinit());
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_start_service)
{
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->start_service());
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_set_adv_data)
{
	uint8_t test_data[] = "Lorem ipsum.";
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->set_adv_data(test_data, sizeof(test_data)));
	zassert_equal(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_adv_data(NULL, 0));
	zassert_equal(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_adv_data(test_data, 0));
	zassert_equal(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->set_adv_data(NULL, sizeof(test_data)));
	zassert_equal(SID_ERROR_GENERIC, p_test_ble_ifc->set_adv_data(test_data, sizeof(test_data)));
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_start_advertisement)
{
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->start_adv());
	zassert_equal(SID_ERROR_GENERIC, p_test_ble_ifc->start_adv());
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_stop_advertisement)
{
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->stop_adv());
	zassert_equal(SID_ERROR_GENERIC, p_test_ble_ifc->stop_adv());
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_adapter_callback_pass)
{
	sid_pal_ble_adapter_callbacks_t cb;
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	set_callbacks(&cb);
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_adapter_callback_null)
{
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_NULL_POINTER, p_test_ble_ifc->set_callback(NULL));
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_adapter_callback_fail)
{
	sid_pal_ble_adapter_callbacks_t cb;
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	set_callbacks(&cb);
	cb.ind_callback = NULL;
	zassert_not_equal(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
	set_callbacks(&cb);
	cb.data_callback = NULL;
	zassert_not_equal(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
	set_callbacks(&cb);
	cb.notify_callback = NULL;
	zassert_not_equal(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
	set_callbacks(&cb);
	cb.conn_callback = NULL;
	zassert_not_equal(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
	set_callbacks(&cb);
	cb.mtu_callback = NULL;
	zassert_not_equal(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
	set_callbacks(&cb);
	cb.adv_start_callback = NULL;
	zassert_not_equal(SID_ERROR_NONE, p_test_ble_ifc->set_callback(&cb));
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_send_data_pass)
{
	uint8_t data[TEST_DATA_CHUNK];
	const sid_ble_conn_params_t test_conn_params;
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->send(AMA_SERVICE, data, sizeof(data)));
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_send_data_fail)
{
	uint8_t data[TEST_DATA_CHUNK];
	const sid_ble_conn_params_t test_conn_params;
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_INVALID_ARGS, p_test_ble_ifc->send(AMA_SERVICE, data, sizeof(data)));
	zassert_equal(SID_ERROR_GENERIC, p_test_ble_ifc->send(AMA_SERVICE, data, sizeof(data)));
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_send_data_unsupported)
{
	uint8_t data[TEST_DATA_CHUNK];
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_NOSUPPORT, p_test_ble_ifc->send(FAKE_SERVICE, data, sizeof(data)));
}

ZTEST(sid_pal_ble_adapter, test_ble_adapter_disconnect)
{
	sid_pal_ble_adapter_interface_t p_test_ble_ifc;
	zassert_equal(SID_ERROR_NONE, sid_pal_ble_adapter_create(&p_test_ble_ifc));
	zassert_equal(SID_ERROR_NONE, p_test_ble_ifc->disconnect());
	zassert_equal(SID_ERROR_GENERIC, p_test_ble_ifc->disconnect());
}