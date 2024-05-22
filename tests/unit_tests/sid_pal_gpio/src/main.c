/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include "sid_error.h"
#include <zephyr/drivers/gpio.h>
#include <sid_pal_gpio_ifc.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <fff.h>

#include <sid_gpio_utils.h>

DEFINE_FFF_GLOBALS;
FAKE_VALUE_FUNC(int, gpio_pin_configure, const struct device *, gpio_pin_t, gpio_flags_t);
FAKE_VALUE_FUNC(int, gpio_pin_get_raw, const struct device *, gpio_pin_t);
FAKE_VALUE_FUNC(int, gpio_pin_set_raw, const struct device *, gpio_pin_t, int);
FAKE_VALUE_FUNC(int, gpio_pin_toggle, const struct device *, gpio_pin_t);
FAKE_VALUE_FUNC(int, gpio_pin_interrupt_configure, const struct device *, gpio_pin_t, gpio_flags_t);
FAKE_VOID_FUNC(gpio_init_callback, struct gpio_callback *, gpio_callback_handler_t, gpio_port_pins_t);
FAKE_VALUE_FUNC(int, gpio_add_callback, const struct device *, struct gpio_callback *);
FAKE_VALUE_FUNC(int, gpio_remove_callback, const struct device *, struct gpio_callback *);

#define INVALID_GPIO (99)
#define GPIO_NUMBER (0)

#define GPIO_LV_HI (1)
#define GPIO_LV_LO (0)

#define INVALID_DIRECTION_VALUE (20)
#define INVALID_INPUT_MODE_VALUE (20)
#define INVALID_PULL_MODE_VALUE (20)
#define INVALID_IRQ_TRIGGER_VALUE (20)

#define E_OK (0)

static void test_register_too_much(void)
{
	for (int i = 0; i < CONFIG_SIDEWALK_GPIO_MAX; i++) {
		int ret = sid_gpio_utils_register_gpio(
			(struct gpio_dt_spec){ .port = (struct device *)0x123 + i, .pin = i });
		zassert_true(ret >= 0, NULL);
	}
	int ret = sid_gpio_utils_register_gpio((struct gpio_dt_spec){
		.port = (struct device *)0x123 + CONFIG_SIDEWALK_GPIO_MAX + 1,
		.pin = CONFIG_SIDEWALK_GPIO_MAX + 1 });
	zassert_equal(ret, -ENOSPC, NULL);
}

static void test_read_without_register(void)
{
	uint8_t test_gpio_state = 123;

	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_read(0, &test_gpio_state), NULL);
}

static void test_read_not_registered(void)
{
	uint8_t test_gpio_state = 123;
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_read(gpio + 1, &test_gpio_state), NULL);
	zassert_equal(123, test_gpio_state, NULL);
}

static void test_read_without_direction(void)
{
	uint8_t test_gpio_state = 123;
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_read(gpio, &test_gpio_state), NULL);
}

static void test_read_gpio_OUTPUT(void)
{
	uint8_t test_gpio_state = 123;
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_read(gpio, &test_gpio_state), NULL);
	zassert_equal(123, test_gpio_state, NULL);
}

static void test_read_NULL(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	zassert_equal(SID_ERROR_NULL_POINTER, sid_pal_gpio_read(gpio, NULL), NULL);
}

static void test_read_gpio(void)
{
	uint8_t test_gpio_state = 123;
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_get_raw_fake.return_val = 0;
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_read(gpio, &test_gpio_state), NULL);
	zassert_equal(0, test_gpio_state, NULL);
}

static void test_set_direction_not_registered(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	sid_error_t err = sid_pal_gpio_set_direction(gpio + 1, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_set_direction_not_registered_internal(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	int err = sid_gpio_utils_gpio_set_flags(gpio + 1, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(-EINVAL, err, NULL);
}

static void test_set_direction_input(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_set_direction_output(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_set_direction_invalid(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT + 1);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_write_without_register(void)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_write(0, 1), NULL);
}

static void test_write_not_registered(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	const uint8_t gpio_value = 1;
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_write(gpio + 1, gpio_value), NULL);
}

static void test_write_without_direction(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_write(gpio, 0), NULL);
}

static void test_write_gpio_INPUT(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	const uint8_t gpio_value = 1;
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_write(gpio, gpio_value), NULL);
}

static void test_write_gpio(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	const uint8_t gpio_value = 1;
	gpio_pin_set_raw_fake.return_val = 0;
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_write(gpio, gpio_value), NULL);
}

static void test_toggle_without_register(void)
{
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_toggle(0), NULL);
}

static void test_toggle_not_registered(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_toggle(gpio + 1), NULL);
}

static void test_toggle_without_direction(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_toggle(gpio), NULL);
}

static void test_toggle_gpio_INPUT(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_toggle(gpio), NULL);
}

static void test_toggle_gpio(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_toggle_fake.return_val = 0;
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_toggle(gpio), NULL);
}

static void test_toggle_set_gpio(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_toggle_fake.return_val = 0;
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_toggle(gpio), NULL);
	const uint8_t gpio_value = 1;
	gpio_pin_set_raw_fake.return_val = 0;
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_write(gpio, gpio_value), NULL);
}

static void test_set_toggle_gpio(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	const uint8_t gpio_value = 1;
	gpio_pin_set_raw_fake.return_val = 0;
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_write(gpio, gpio_value), NULL);
	gpio_pin_toggle_fake.return_val = 0;
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_toggle(gpio), NULL);
}

static void test_sid_gpio_utils_gpio_get_flags_unregistered(void)
{
	zassert_equal(-EINVAL, sid_gpio_utils_gpio_get_flags(0, NULL), NULL);
}

static void test_sid_gpio_utils_gpio_get_flags_null(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	zassert_equal(-ENOENT, sid_gpio_utils_gpio_get_flags(gpio, NULL), NULL);
}

static void test_input_mode_connect_to_output_gpio(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_CONNECT);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_input_mode_connect_invalid_gpio(void)
{
	sid_error_t err = sid_pal_gpio_input_mode(0, SID_PAL_GPIO_INPUT_CONNECT);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_input_mode_connect(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_CONNECT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_input_mode_disconnect(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_DISCONNECT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_input_mode_invalid(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_DISCONNECT + 1);
	zassert_equal(SID_ERROR_NOSUPPORT, err, NULL);
}

static void test_output_mode_connect_to_input_gpio(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	err = sid_pal_gpio_output_mode(gpio, SID_PAL_GPIO_OUTPUT_PUSH_PULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_output_mode_connect_invalid_gpio(void)
{
	sid_error_t err = sid_pal_gpio_output_mode(0, SID_PAL_GPIO_OUTPUT_PUSH_PULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_output_mode_push_pull(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	err = sid_pal_gpio_output_mode(gpio, SID_PAL_GPIO_OUTPUT_PUSH_PULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_output_mode_open_drain(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	err = sid_pal_gpio_output_mode(gpio, SID_PAL_GPIO_OUTPUT_OPEN_DRAIN);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_output_mode_invalid(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val =

continue
```c
0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_OUTPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	err = sid_pal_gpio_output_mode(gpio, SID_PAL_GPIO_OUTPUT_OPEN_DRAIN + 1);
	zassert_equal(SID_ERROR_NOSUPPORT, err, NULL);
}

static void test_pull_mode_unregistered(void)
{
	sid_error_t err = sid_pal_gpio_pull_mode(0, SID_PAL_GPIO_PULL_NONE);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_pull_mode_unregistered2(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	sid_error_t err = sid_pal_gpio_pull_mode(gpio + 1, SID_PAL_GPIO_PULL_NONE);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_pull_mode_NONE(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_NONE);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_pull_mode_UP(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_UP);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_pull_mode_DOWN(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_DOWN);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_pull_mode_DOWN_UP(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_DOWN);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_configure_fake.return_val = 0;
	err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_UP);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_pull_mode_UP_DOWN(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_UP);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_configure_fake.return_val = 0;
	err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_DOWN);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_pull_mode_invalid(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	sid_error_t err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_DOWN + 1);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_set_irq_unregistered(void)
{
	sid_error_t err = sid_pal_gpio_set_irq(0, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_set_irq_unregistered2(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	sid_error_t err = sid_pal_gpio_set_irq(gpio + 1, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_set_irq_none(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_set_irq_rising(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_RISING, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_set_irq_falling(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_FALLING, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_set_irq_edge(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_EDGE, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_set_irq_low(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_LOW, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_set_irq_high(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_set_irq_invalid(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH + 1, NULL, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_set_irq_configure_and_disable(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_remove_callback_fake.return_val = 0;
	err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_set_irq_configure_cb_error(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_init_callback_fake.custom_fake = gpio_init_callback;
	gpio_add_callback_fake.return_val = -22;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_set_irq_configure_and_disable_cb_error(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_remove_callback_fake.return_val = -22;
	err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL, NULL);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static gpio_callback_handler_t gpio_init_callback_STUB_handler;
static struct gpio_callback *gpio_init_callback_STUB_callback;
static gpio_port_pins_t gpio_init_callback_STUB_pin_mask;
void gpio_init_callback_STUB(struct gpio_callback *callback, gpio_callback_handler_t handler,
			     gpio_port_pins_t pin_mask, int cmock_num_calls)
{
	gpio_init_callback_STUB_callback = callback;
	gpio_init_callback_STUB_handler = handler;
	gpio_init_callback_STUB_pin_mask = pin_mask;
}

struct gpio_irq_arg {
	uint32_t gpio_number;
	int call_count;
};
void sid_pal_gpio_irq_handler_test(uint32_t gpio_number, void *callback_arg)
{
	struct gpio_irq_arg *ctx = (struct gpio_irq_arg *)callback_arg;
	ctx->gpio_number = gpio_number;
	ctx->call_count++;
}

static void test_irq_handler(void)
{
	gpio_init_callback_STUB_handler = NULL;
	gpio_init_callback_STUB_callback = NULL;
	struct device dev;
	struct gpio_irq_arg arg = { 0, 0 };
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback_STUB;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH,
					       sid_pal_gpio_irq_handler_test, &arg);
	zassert_equal(SID_ERROR_NONE, err, NULL);

	zassert_not_null(gpio_init_callback_STUB_handler, NULL);
	zassert_not_null(gpio_init_callback_STUB_callback, NULL);
	gpio_init_callback_STUB_handler(&dev, gpio_init_callback_STUB_callback,
					gpio_init_callback_STUB_pin_mask);
	k_yield();
	zassert_equal(1, arg.call_count, NULL);
	zassert_equal(gpio, arg.gpio_number, NULL);
}

static void test_irq_handler_invalid_dev_in_callback(void)
{
	gpio_init_callback_STUB_handler = NULL;
	gpio_init_callback_STUB_callback = NULL;
	struct device dev;
	struct gpio_irq_arg arg = { 123, 0 };
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback_STUB;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH,
					       sid_pal_gpio_irq_handler_test, &arg);
	zassert_equal(SID_ERROR_NONE, err, NULL);

	zassert_not_null(gpio_init_callback_STUB_handler, NULL);
	zassert_not_null(gpio_init_callback_STUB_callback, NULL);
	gpio_init_callback_STUB_handler(NULL, gpio_init_callback_STUB_callback,
					gpio_init_callback_STUB_pin_mask);
	k_yield();
	zassert_equal(0, arg.call_count, NULL);
	zassert_equal(123, arg.gpio_number, NULL);
}

static void test_sid_pal_gpio_irq_enable(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_irq_enable(gpio);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_sid_pal_gpio_irq_enable_set_irq(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_interrupt_configure_fake.return_val = 0;
	err = sid_pal_gpio_irq_enable(gpio);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_sid_pal_gpio_irq_disable(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_irq_disable(gpio);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_sid_pal_gpio_irq_disable_unregister(void)
{
	sid_error_t err = sid_pal_gpio_irq_disable(0);
	zassert_equal(SID_ERROR_INVALID_ARGS, err, NULL);
}

static void test_error_mapping(void)
{
	struct device dev;
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = -EINVAL;
	zassert_equal(SID_ERROR_INVALID_ARGS, sid_pal_gpio_irq_disable(gpio), NULL);
	gpio_pin_interrupt_configure_fake.return_val = -ENOTSUP;
	zassert_equal(SID_ERROR_NOSUPPORT, sid_pal_gpio_irq_disable(gpio), NULL);
	gpio_pin_interrupt_configure_fake.return_val = -EIO;
	zassert_equal(SID_ERROR_IO_ERROR, sid_pal_gpio_irq_disable(gpio), NULL);
	gpio_pin_interrupt_configure_fake.return_val = -EBUSY;
	zassert_equal(SID_ERROR_BUSY, sid_pal_gpio_irq_disable(gpio), NULL);
	gpio_pin_interrupt_configure_fake.return_val = -0xffff;
	zassert_equal(SID_ERROR_GENERIC, sid_pal_gpio_irq_disable(gpio), NULL);
}

static void test_unused_pins(void)
{
	uint8_t val;
	gpio_flags_t flag;
	zassert_equal(-ENOTSUP, sid_gpio_utils_gpio_read(GPIO_UNUSED_PIN, &val), NULL);
	zassert_equal(-ENOTSUP, sid_gpio_utils_gpio_set(GPIO_UNUSED_PIN, 123), NULL);
	zassert_equal(-ENOTSUP, sid_gpio_utils_gpio_toggle(GPIO_UNUSED_PIN), NULL);
	zassert_equal(-ENOTSUP, sid_gpio_utils_gpio_set_flags(GPIO_UNUSED_PIN, 123), NULL);
	zassert_equal(-ENOTSUP, sid_gpio_utils_disconnect(GPIO_UNUSED_PIN), NULL);
	zassert_equal(-ENOTSUP, sid_gpio_utils_gpio_get_flags(GPIO_UNUSED_PIN, &flag), NULL);
	zassert_equal(-ENOTSUP, sid_gpio_utils_irq_handler_set(GPIO_UNUSED_PIN, NULL, NULL), NULL);
	zassert_equal(-ENOTSUP, sid_gpio_utils_irq_configure(GPIO_UNUSED_PIN, 123), NULL);
	zassert_equal(-ENOTSUP, sid_gpio_utils_irq_set(GPIO_UNUSED_PIN, true), NULL);

	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_set_direction(GPIO_UNUSED_PIN,
								     SID_PAL_GPIO_DIRECTION_INPUT), NULL);
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_read(GPIO_UNUSED_PIN, &val), NULL);
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_write(GPIO_UNUSED_PIN, 123), NULL);
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_toggle(GPIO_UNUSED_PIN), NULL);
	zassert_equal(SID_ERROR_NONE,
			  sid_pal_gpio_set_irq(GPIO_UNUSED_PIN, SID_PAL_GPIO_IRQ_TRIGGER_NONE, NULL,
					       NULL), NULL);
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_irq_enable(GPIO_UNUSED_PIN), NULL);
	zassert_equal(SID_ERROR_NONE, sid_pal_gpio_irq_disable(GPIO_UNUSED_PIN), NULL);
	zassert

continue
```c
_equal(SID_ERROR_NONE,
			  sid_pal_gpio_input_mode(GPIO_UNUSED_PIN, SID_PAL_GPIO_INPUT_CONNECT), NULL);
	zassert_equal(SID_ERROR_NONE,
			  sid_pal_gpio_output_mode(GPIO_UNUSED_PIN, SID_PAL_GPIO_OUTPUT_PUSH_PULL), NULL);
	zassert_equal(SID_ERROR_NONE,
			  sid_pal_gpio_pull_mode(GPIO_UNUSED_PIN, SID_PAL_GPIO_PULL_NONE), NULL);
}

struct read_intput_on_irq_handler_args {
	int call_count;
	uint8_t readed_gpio_value;
	sid_error_t read_return_code;
};

static void read_intput_on_irq_handler(uint32_t gpio_number, void *callback_arg)
{
	struct read_intput_on_irq_handler_args *arg =
		(struct read_intput_on_irq_handler_args *)callback_arg;
	arg->call_count++;
	arg->read_return_code = sid_pal_gpio_read(gpio_number, &arg->readed_gpio_value);
}

static void test_read_intput_on_irq(void)
{
	gpio_init_callback_STUB_handler = NULL;
	gpio_init_callback_STUB_callback = NULL;
	struct device dev;
	struct read_intput_on_irq_handler_args handler_arg = { 0, 0, 0 };

	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback_STUB;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, read_intput_on_irq_handler,
				   &handler_arg);
	zassert_equal(SID_ERROR_NONE, err, NULL);

	gpio_pin_get_raw_fake.return_val = 1;

	zassert_not_null(gpio_init_callback_STUB_handler, NULL);
	zassert_not_null(gpio_init_callback_STUB_callback, NULL);
	gpio_init_callback_STUB_handler(&dev, gpio_init_callback_STUB_callback,
					gpio_init_callback_STUB_pin_mask);
	k_yield();
	zassert_equal(1, handler_arg.call_count, NULL);
	zassert_equal(1, handler_arg.readed_gpio_value, NULL);
	zassert_equal(SID_ERROR_NONE, handler_arg.read_return_code, NULL);
}

static void test_read_intput_on_irq_gpio_not_input(void)
{
	gpio_init_callback_STUB_handler = NULL;
	gpio_init_callback_STUB_callback = NULL;
	struct device dev;
	struct read_intput_on_irq_handler_args handler_arg = { 0, 0, 0 };

	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback_STUB;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH,
					       read_intput_on_irq_handler, &handler_arg);
	zassert_equal(SID_ERROR_NONE, err, NULL);

	gpio_pin_get_raw_fake.return_val = 1;

	zassert_not_null(gpio_init_callback_STUB_handler, NULL);
	zassert_not_null(gpio_init_callback_STUB_callback, NULL);
	gpio_init_callback_STUB_handler(&dev, gpio_init_callback_STUB_callback,
					gpio_init_callback_STUB_pin_mask);
	k_yield();
	zassert_equal(1, handler_arg.call_count, NULL);
	zassert_equal(1, handler_arg.readed_gpio_value, NULL);
	zassert_equal(SID_ERROR_NONE, handler_arg.read_return_code, NULL);
}

static void test_set_irq_flags_disable_enable(void)
{
	struct device dev;

	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);

	gpio_pin_interrupt_configure_fake.return_val = 0;
	gpio_init_callback_fake.custom_fake = gpio_init_callback;
	gpio_add_callback_fake.return_val = 0;
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_irq(gpio, SID_PAL_GPIO_IRQ_TRIGGER_HIGH, NULL, NULL);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_interrupt_configure_fake.return_val = 0;
	err = sid_pal_gpio_irq_disable(gpio);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_interrupt_configure_fake.return_val = 0;
	err = sid_pal_gpio_irq_enable(gpio);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_set_flags_disable_enable(void)
{
	struct device dev;

	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	gpio_pin_configure_fake.return_val = 0;
	sid_error_t err = sid_pal_gpio_set_direction(gpio, SID_PAL_GPIO_DIRECTION_INPUT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_configure_fake.return_val = 0;
	err = sid_pal_gpio_pull_mode(gpio, SID_PAL_GPIO_PULL_UP);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_configure_fake.return_val = 0;
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_DISCONNECT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
	gpio_pin_configure_fake.return_val = 0;
	err = sid_pal_gpio_input_mode(gpio, SID_PAL_GPIO_INPUT_CONNECT);
	zassert_equal(SID_ERROR_NONE, err, NULL);
}

static void test_register_twice_gpio(void)
{
	struct device dev;

	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio >= 0, NULL);
	uint32_t gpio2 =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = &dev, .pin = 2 });
	zassert_true(gpio2 >= 0, NULL);
	zassert_equal(gpio, gpio2, NULL);
}

static void test_register_invalid_gpio(void)
{
	uint32_t gpio =
		sid_gpio_utils_register_gpio((struct gpio_dt_spec){ .port = NULL, .pin = 0 });
	zassert_equal(GPIO_UNUSED_PIN, gpio, NULL);
}

ZTEST_SUITE(sid_pal_gpio, NULL, NULL, NULL, NULL, NULL);

ZTEST(sid_pal_gpio, test_register_too_much)
{
    test_register_too_much();
}

ZTEST(sid_pal_gpio, test_read_without_register)
{
    test_read_without_register();
}

ZTEST(sid_pal_gpio, test_read_not_registered)
{
    test_read_not_registered();
}

ZTEST(sid_pal_gpio, test_read_without_direction)
{
    test_read_without_direction();
}

ZTEST(sid_pal_gpio, test_read_gpio_OUTPUT)
{
    test_read_gpio_OUTPUT();
}

ZTEST(sid_pal_gpio, test_read_NULL)
{
    test_read_NULL();
}

ZTEST(sid_pal_gpio, test_read_gpio)
{
    test_read_gpio();
}

ZTEST(sid_pal_gpio, test_set_direction_not_registered)
{
    test_set_direction_not_registered();
}

ZTEST(sid_pal_gpio, test_set_direction_not_registered_internal)
{
    test_set_direction_not_registered_internal();
}

ZTEST(sid_pal_gpio, test_set_direction_input)
{
    test_set_direction_input();
}

ZTEST(sid_pal_gpio, test_set_direction_output)
{
    test_set_direction_output();
}

ZTEST(sid_pal_gpio, test_set_direction_invalid)
{
    test_set_direction_invalid();
}

ZTEST(sid_pal_gpio, test_write_without_register)
{
    test_write_without_register();
}

ZTEST(sid_pal_gpio, test_write_not_registered)
{
    test_write_not_registered();
}

ZTEST(sid_pal_gpio, test_write_without_direction)
{
    test_write_without_direction();
}

ZTEST(sid_pal_gpio, test_write_gpio_INPUT)
{
    test_write_gpio_INPUT();
}

ZTEST(sid_pal_gpio, test_write_gpio)
{
    test_write_gpio();
}

ZTEST(sid_pal_gpio, test_toggle_without_register)
{
    test_toggle_without_register();
}

ZTEST(sid_pal_gpio, test_toggle_not_registered)
{
    test_toggle_not_registered();
}

ZTEST(sid_pal_gpio, test_toggle_without_direction)
{
    test_toggle_without_direction();
}

ZTEST(sid_pal_gpio, test_toggle_gpio_INPUT)
{
    test_toggle_gpio_INPUT();
}

ZTEST(sid_pal_gpio, test_toggle_gpio)
{
    test_toggle_gpio();
}

ZTEST(sid_pal_gpio, test_toggle_set_gpio)
{
    test_toggle_set_gpio();
}

ZTEST(sid_pal_gpio, test_set_toggle_gpio)
{
    test_set_toggle_gpio();
}

ZTEST(sid_pal_gpio, test_sid_gpio_utils_gpio_get_flags_unregistered)
{
    test_sid_gpio_utils_gpio_get_flags_unregistered();
}

ZTEST(sid_pal_gpio, test_sid_gpio_utils_gpio_get_flags_null)
{
    test_sid_gpio_utils_gpio_get_flags_null();
}

ZTEST(sid_pal_gpio, test_input_mode_connect_to_output_gpio)
{
    test_input_mode_connect_to_output_gpio();
}

ZTEST(sid_pal_gpio, test_input_mode_connect_invalid_gpio)
{
    test_input_mode_connect_invalid_gpio();
}

ZTEST(sid_pal_gpio, test_input_mode_connect)
{
    test_input_mode_connect();
}

ZTEST(sid_pal_gpio, test_input_mode_disconnect)
{
    test_input_mode_disconnect();
}

ZTEST(sid_pal_gpio, test_input_mode_invalid)
{
    test_input_mode_invalid();
}

ZTEST(sid_pal_gpio, test_output_mode_connect_to_input_gpio)
{
    test_output_mode_connect_to_input_gpio();
}

ZTEST(sid_pal_gpio, test_output_mode_connect_invalid_gpio)
{
    test_output_mode_connect_invalid_gpio();
}

ZTEST(sid_pal_gpio, test_output_mode_push_pull)
{
    test_output_mode_push_pull();
}

ZTEST(sid_pal_gpio, test_output_mode_open_drain)
{
    test_output_mode_open_drain();
}

ZTEST(sid_pal_gpio, test_output_mode_invalid)
{
    test_output_mode_invalid();
}

ZTEST(sid_pal_gpio, test_pull_mode_unregistered)
{
    test_pull_mode_unregistered();
}

ZTEST(sid_pal_gpio, test_pull_mode_unregistered2)
{
    test_pull_mode_unregistered2();
}

ZTEST(sid_pal_gpio, test_pull_mode_NONE)
{
    test_pull_mode_NONE();
}

ZTEST(sid_pal_gpio, test_pull_mode_UP)
{
    test_pull_mode_UP();
}

ZTEST(sid_pal_gpio, test_pull_mode_DOWN)
{
    test_pull_mode_DOWN();
}

ZTEST(sid_pal_gpio, test_pull_mode_DOWN_UP)
{
    test_pull_mode_DOWN_UP();
}

ZTEST(sid_pal_gpio, test_pull_mode_UP_DOWN)
{
    test_pull_mode_UP_DOWN();
}

ZTEST(sid_pal_gpio, test_pull_mode_invalid)
{
    test_pull_mode_invalid();
}

ZTEST(sid_pal_gpio, test_set_irq_unregistered)
{
    test_set_irq_unregistered();
}

ZTEST(sid_pal_gpio, test_set_irq_unregistered2)
{
    test_set_irq_unregistered2();
}

ZTEST(sid_pal_gpio, test_set_irq_none)
{
    test_set_irq_none();
}

ZTEST(sid_pal_gpio, test_set_irq_rising)
{
    test_set_irq_rising();
}

ZTEST(sid_pal_gpio, test_set_irq_falling)
{
    test_set_irq_falling();
}

ZTEST(sid_pal_gpio, test_set_irq_edge)
{
    test_set_irq_edge();
}

ZTEST(sid_pal_gpio, test_set_irq_low)
{
    test_set_irq_low();
}

ZTEST(sid_pal_gpio, test_set_irq_high)
{
    test_set_irq_high();
}

ZTEST(sid_pal_gpio, test_set_irq_invalid)
{
    test_set_irq_invalid();
}

ZTEST(sid_pal_gpio, test_set_irq_configure_and_disable)
{
    test_set_irq_configure_and_disable();
}

ZTEST(sid_pal_gpio, test_set_irq_configure_cb_error)
{
    test_set_irq_configure_cb_error();
}

ZTEST(sid_pal_gpio, test_set_irq_configure_and_disable_cb_error)
{
    test_set_irq_configure_and_disable_cb_error();
}

ZTEST(sid_pal_gpio, test_irq_handler)
{
    test_irq_handler();
}

ZTEST(sid_pal_gpio, test_irq_handler_invalid_dev_in_callback)
{
    test_irq_handler_invalid_dev_in_callback();
}

ZTEST(sid_pal_gpio, test_sid_pal_gpio_irq_enable)
{
    test_sid_pal_gpio_irq_enable();
}

ZTEST(sid_pal_gpio, test_sid_pal_gpio_irq_enable_set_irq)
{
    test_sid_pal_gpio_irq_enable_set_irq();
}

ZTEST(sid_pal_gpio, test_sid_pal_gpio_irq_disable)
{
    test_sid_pal_gpio_irq_disable();
}

ZTEST(sid_pal_gpio, test_sid_pal_gpio_irq_disable_unregister)
{
    test_sid_pal_gpio_irq_disable_unregister();
}

ZTEST(sid_pal_gpio, test_error_mapping)
{
    test_error_mapping();
}

ZTEST(sid_pal_gpio, test_unused_pins)
{
    test_unused_pins();
}

ZTEST(sid_pal_gpio, test_read_intput_on_irq)
{
    test_read_intput_on_irq();
}

ZTEST(sid_pal_gpio, test_read_intput_on_irq_gpio_not_input)
{
    test_read_intput_on_irq_gpio_not_input();
}

ZTEST(sid_pal_gpio, test_set_irq_flags_disable_enable)
{
    test_set_irq_flags_disable_enable();
}

ZTEST(sid_pal_gpio, test_set_flags_disable_enable)
{
    test_set_flags_disable_enable();
}

ZTEST(sid_pal_gpio, test_register_twice_gpio)
{
    test_register_twice_gpio();
}

ZTEST(sid_pal_gpio, test_register_invalid_gpio)
{
    test_register_invalid_gpio();
}
