/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/ztest.h>
#include <zephyr/fff.h>

#include <sid_ble_connection.h>

#include <cmock_sid_ble_adapter_callbacks.h>

#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>

#include <stdbool.h>
#include <errno.h>

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(bt_conn_cb_register, struct bt_conn_cb *);
FAKE_VOID_FUNC(bt_gatt_cb_register, struct bt_gatt_cb *);
FAKE_VALUE_FUNC(struct bt_conn *, bt_conn_ref, struct bt_conn *);
FAKE_VOID_FUNC(bt_conn_unref, struct bt_conn *);
FAKE_VALUE_FUNC(const bt_addr_le_t *, bt_conn_get_dst, const struct bt_conn *);
FAKE_VALUE_FUNC(int, bt_conn_disconnect, struct bt_conn *, uint8_t);

#define FFF_FAKES_LIST(FAKE)                                                                       \
	FAKE(bt_conn_cb_register)                                                                  \
	FAKE(bt_gatt_cb_register)                                                                  \
	FAKE(bt_conn_ref)                                                                          \
	FAKE(bt_conn_unref)                                                                        \
	FAKE(bt_conn_get_dst)                                                                      \
	FAKE(bt_conn_disconnect)

#define CONNECTED (true)
#define DISCONNECTED (false)
#define ESUCCESS (0)

struct bt_conn {
	uint8_t dummy;
};

typedef struct {
	size_t num_calls;
	uint8_t *addr;
	bool state;
} connection_callback_test_t;

static connection_callback_test_t conn_cb_test;
static struct bt_conn_cb *sid_bt_conn_cb;
static struct bt_gatt_cb *sid_bt_gatt_cb;

static void connection_callback(const uint8_t *ble_addr, int cmock_num_calls)
{
	conn_cb_test.num_calls++;
	conn_cb_test.addr = (uint8_t *)ble_addr;
}

static void setUp(void *unused)
{
	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();
	memset(&conn_cb_test, 0x00, sizeof(conn_cb_test));
	cmock_sid_ble_adapter_callbacks_Init();
}

static void tearDown(void *unused)
{
	cmock_sid_ble_adapter_callbacks_Verify();
}

ZTEST_SUITE(sid_ble_conn, NULL, NULL, setUp, tearDown, NULL);

ZTEST(sid_ble_conn, test_sid_ble_conn_init)
{
	sid_ble_conn_init();
	zassert_equal(1, bt_conn_cb_register_fake.call_count);
	zassert_equal(1, bt_gatt_cb_register_fake.call_count);

	sid_bt_conn_cb = bt_conn_cb_register_fake.arg0_history[0];
	zassert_not_null(sid_bt_conn_cb);
	zassert_not_null(sid_bt_conn_cb->connected);
	zassert_not_null(sid_bt_conn_cb->disconnected);
	sid_bt_gatt_cb = bt_gatt_cb_register_fake.arg0_history[0];
	zassert_not_null(sid_bt_gatt_cb);
	zassert_not_null(sid_bt_gatt_cb->att_mtu_updated);
}

ZTEST(sid_ble_conn, test_sid_ble_conn_params_get)
{
	const sid_ble_conn_params_t *params = NULL;

	sid_ble_conn_init();
	sid_ble_conn_deinit();
	sid_ble_conn_deinit();

	sid_ble_conn_init();
	params = sid_ble_conn_params_get();
	zassert_not_null(params);

	sid_ble_conn_deinit();
	params = sid_ble_conn_params_get();
	zassert_is_null(params);
}

ZTEST(sid_ble_conn, test_sid_ble_conn_positive)
{
	uint8_t test_no_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const sid_ble_conn_params_t *params = NULL;
	struct bt_conn test_conn = { .dummy = 0xDC };
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;

	sid_ble_conn_deinit();
	sid_ble_conn_init();

	__cmock_sid_ble_adapter_conn_connected_ExpectWithArray(test_addr.a.val, BT_ADDR_SIZE);
	sid_bt_conn_cb->connected(&test_conn, test_no_error);

	params = sid_ble_conn_params_get();
	zassert_equal_ptr(&test_conn, params->conn);
	zassert_mem_equal(test_addr.a.val, params->addr, BT_ADDR_SIZE);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectWithArray(test_addr.a.val, BT_ADDR_SIZE);
	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
}

ZTEST(sid_ble_conn, test_sid_ble_set_conn_cb_positive)
{
	uint8_t test_no_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const sid_ble_conn_params_t *params = NULL;
	struct bt_conn test_conn = { .dummy = 0xDC };
	const bt_addr_le_t test_addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a = { { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 } },
	};

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;

	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	zassert_mem_equal(test_addr.a.val, conn_cb_test.addr, BT_ADDR_SIZE);

	params = sid_ble_conn_params_get();
	zassert_equal_ptr(&test_conn, params->conn);
	zassert_mem_equal(test_addr.a.val, params->addr, BT_ADDR_SIZE);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
	zassert_equal(DISCONNECTED, conn_cb_test.state);
	zassert_mem_equal(test_addr.a.val, conn_cb_test.addr, BT_ADDR_SIZE);
}

ZTEST(sid_ble_conn, test_sid_ble_conn_cb_set_call_count)
{
	size_t conn_cb_cnt_expected = 0;
	uint8_t test_no_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_error_timeout = BT_HCI_ERR_CONN_TIMEOUT;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const bt_addr_le_t test_addr;
	struct bt_conn test_conn = { .dummy = 0xDC };

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;

	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, test_error_timeout);

	bt_conn_get_dst_fake.return_val = NULL;
	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_cnt_expected, conn_cb_test.num_calls);
}

ZTEST(sid_ble_conn, test_sid_ble_disconnected_wrong_conn)
{
	size_t conn_cb_cnt_expected = 0;
	struct bt_conn test_wrong_conn;
	uint8_t test_no_error = BT_HCI_ERR_SUCCESS;
	uint8_t test_reason = BT_HCI_ERR_UNKNOWN_LMP_PDU;
	const bt_addr_le_t test_addr;
	struct bt_conn test_conn = { .dummy = 0xDC };

	bt_conn_get_dst_fake.return_val = &test_addr;
	bt_conn_ref_fake.return_val = &test_conn;

	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, test_no_error);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_cnt_expected, conn_cb_test.num_calls);

	sid_bt_conn_cb->disconnected(&test_wrong_conn, test_no_error);
	zassert_equal(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	sid_bt_conn_cb->disconnected(&test_conn, test_reason);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_cnt_expected, conn_cb_test.num_calls);
}

ZTEST(sid_ble_conn, test_sid_ble_cb_set_before_init)
{
	size_t conn_cb_cnt_expected = 0;
	struct bt_conn test_conn = { .dummy = 0xDC };

	bt_conn_ref_fake.return_val = &test_conn;

	sid_ble_conn_deinit();
	sid_ble_conn_init();
	__cmock_sid_ble_adapter_conn_connected_StubWithCallback(connection_callback);
	__cmock_sid_ble_adapter_conn_disconnected_StubWithCallback(connection_callback);

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&test_conn, 0);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_cnt_expected, conn_cb_test.num_calls);

	__cmock_sid_ble_adapter_conn_disconnected_ExpectAnyArgs();
	sid_bt_conn_cb->disconnected(&test_conn, 19);
	conn_cb_cnt_expected++;
	zassert_equal(conn_cb_cnt_expected, conn_cb_test.num_calls);
}

ZTEST(sid_ble_conn, test_sid_ble_conn_mtu_callback)
{
	struct bt_conn test_conn = { .dummy = 0xDC };

	sid_ble_conn_init();

	uint16_t tx_mtu = 32, rx_mtu = 44;

	__cmock_sid_ble_adapter_mtu_changed_Expect(tx_mtu);
	sid_bt_gatt_cb->att_mtu_updated(&test_conn, tx_mtu, rx_mtu);
}

ZTEST(sid_ble_conn, test_sid_ble_conn_mtu_callback_curent_connection)
{
	struct bt_conn curr_conn = { 0 };
	struct bt_conn unknow_conn = { 0 };

	sid_ble_conn_init();
	bt_conn_ref_fake.return_val = &curr_conn;

	__cmock_sid_ble_adapter_conn_connected_ExpectAnyArgs();
	sid_bt_conn_cb->connected(&curr_conn, 0);

	uint16_t tx_mtu = 32, rx_mtu = 44;

	__cmock_sid_ble_adapter_mtu_changed_Expect(tx_mtu);
	sid_bt_gatt_cb->att_mtu_updated(&curr_conn, tx_mtu, rx_mtu);

	sid_bt_gatt_cb->att_mtu_updated(&unknow_conn, tx_mtu, rx_mtu);
}

ZTEST(sid_ble_conn, test_sid_ble_conn_disconnect)
{
	bt_conn_disconnect_fake.return_val = ESUCCESS;
	zassert_equal(ESUCCESS, sid_ble_conn_disconnect());

	bt_conn_disconnect_fake.return_val = -ENOTCONN;
	zassert_not_equal(ESUCCESS, sid_ble_conn_disconnect());
}