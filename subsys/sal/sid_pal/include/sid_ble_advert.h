/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NRF_BLE_ADVERT_H
#define NRF_BLE_ADVERT_H

#include <stdint.h>

/**
 * @brief Initialize Bluetooth Advertising.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_advert_init(void);

/**
 * @brief Deinitialize Bluetooth Advertising.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_advert_deinit(void);

/**
 * @brief Start Bluetooth advertising.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_advert_start(void);

/**
 * @brief Notify sid_ble_advert that connection has been made
 * 
 */
void sid_ble_advert_notify_connection(void);

/**
 * @brief Stop Bluetooth advertising.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_advert_stop(void);

/**
 * @brief Update advertising data.
 *
 * @note update the value of manufacturing section in ble advertising.
 * Data may be trimmed to meet bluetooth advertising size requirements.
 * Too long manufacuring data may affect Device Name.
 *
 * @param data buffor of data to be updated.
 * @param data_len length of data to be updated in bytes.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_advert_update(uint8_t *data, uint8_t data_len);

#endif /* NRF_BLE_ADVERT_H */
