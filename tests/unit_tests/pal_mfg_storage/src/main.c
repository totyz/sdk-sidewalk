#include <zephyr/ztest.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_error.h>
#include <zephyr/drivers/cmock_flash.h>
#include <string.h>
#include <zephyr/fff.h>

static uint8_t test_data_buffer[512];

DEFINE_FFF_GLOBALS;
FAKE_VALUE_FUNC(int, flash_erase, const struct device *, off_t, size_t);
FAKE_VALUE_FUNC(int, flash_write, const struct device *, off_t, const void *, size_t);
FAKE_VALUE_FUNC(int, flash_read, const struct device *, off_t, void *, size_t);

void setUp(void *f)
{
	RESET_FAKE(flash_erase);
	RESET_FAKE(flash_write);
	RESET_FAKE(flash_read);
}

ZTEST(mfg_store, test_sid_pal_mfg_storage_no_init)
{
	uint8_t read_buffer[SID_PAL_MFG_STORE_VERSION_SIZE] = { 0 };

	zassert_equal(SID_ERROR_UNINITIALIZED,
		      sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, read_buffer,
					      SID_PAL_MFG_STORE_VERSION_SIZE));
	zassert_equal(SID_ERROR_UNINITIALIZED, sid_pal_mfg_store_erase());
	zassert_false(sid_pal_mfg_store_is_empty());
	memset(read_buffer, 0xAA, SID_PAL_MFG_STORE_VERSION_SIZE);
	memset(test_data_buffer, 0xAA, SID_PAL_MFG_STORE_VERSION_SIZE);
	sid_pal_mfg_store_read(SID_PAL_MFG_STORE_VERSION, read_buffer,
			       SID_PAL_MFG_STORE_VERSION_SIZE);
	zassert_mem_equal(read_buffer, test_data_buffer, SID_PAL_MFG_STORE_VERSION_SIZE);
}

ZTEST(mfg_store, test_sid_pal_mfg_storage_init)
{
	static const sid_pal_mfg_store_region_t mfg_store_region = {
		.addr_start = 0,
		.addr_end = 0x1000,
	};

	sid_pal_mfg_store_init(mfg_store_region);
	flash_erase_fake.return_val = 0;
	zassert_equal(0, sid_pal_mfg_store_erase());
}

ZTEST(mfg_store, test_sid_pal_mfg_storage_write)
{
	uint8_t write_buff[SID_PAL_MFG_STORE_MAX_FLASH_WRITE_LEN] = { 0 };

	zassert_equal(SID_ERROR_INVALID_ARGS,
		      sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, write_buff, 0));
	zassert_equal(SID_ERROR_OUT_OF_RESOURCES,
		      sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, write_buff, 128));
	zassert_equal(SID_ERROR_INCOMPATIBLE_PARAMS,
		      sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, write_buff, 3));
	zassert_equal(SID_ERROR_NOT_FOUND,
		      sid_pal_mfg_store_write(999, write_buff, SID_PAL_MFG_STORE_VERSION_SIZE));
	zassert_equal(SID_ERROR_NULL_POINTER,
		      sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, NULL,
					      SID_PAL_MFG_STORE_VERSION_SIZE));

	flash_write_fake.return_val = 0;
	zassert_equal(0, sid_pal_mfg_store_write(SID_PAL_MFG_STORE_VERSION, write_buff,
						 SID_PAL_MFG_STORE_VERSION_SIZE));
}

ZTEST(mfg_store, test_sid_pal_mfg_storage_dev_id_get)
{
	uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE] = { 0 };

	memset(dev_id, 0x00, sizeof(dev_id));

	zassert_false(sid_pal_mfg_store_dev_id_get(dev_id));
	zassert_equal(0xBF, dev_id[0]);
	// DEV_ID_REG=0x33AABB99
	zassert_equal(0x33, dev_id[1]);
	zassert_equal(0xAA, dev_id[2]);
	zassert_equal(0xBB, dev_id[3]);
	zassert_equal(0x99, dev_id[4]);
}

ZTEST(mfg_store, test_sid_pal_mfg_storage_sn_get)
{
	uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE] = { 0 };
	uint8_t empty_serial[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE] = { 0 };
	memset(empty_serial, 0xFF, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);

	// No serial number.
	flash_read_fake.return_val = 0;
	flash_read_fake.custom_fake = [](const struct device *, off_t, void *data, size_t) {
		memcpy(data, empty_serial, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);
		return 0;
	};
	zassert_false(sid_pal_mfg_store_serial_num_get(serial_num));
}

ZTEST_SUITE(mfg_store, NULL, NULL, setUp, NULL, NULL);