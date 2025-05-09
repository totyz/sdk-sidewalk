/*
 * Copyright 2021-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_900_CFG_H
#define SID_900_CFG_H

#include <stdint.h>

/// @cond sid_ifc_ep_en

/** @file
 *
 * @defgroup SIDEWALK_API Sidewalk API
 * @brief API for communicating with the Sidewalk network
 * @{
 * @ingroup  SIDEWALK_API
 */

/**
 * Describes the profile type of the device
 */
enum sid_device_profile_id {
    /** Device Profile ID for Synchronous Network */
    SID_LINK2_PROFILE_1 = 0x01,
    SID_LINK2_PROFILE_2 = 0x02,
    SID_LINK2_PROFILE_LAST,

    /** Device Profile ID for Asynchronous Network */
    SID_LINK3_PROFILE_A = 0x80,
    SID_LINK3_PROFILE_B = 0x81,
    SID_LINK3_PROFILE_D = 0x83,
    SID_LINK3_PROFILE_LAST,
};
#define IS_LINK3_PROFILE_ID(X) ((X == SID_LINK3_PROFILE_A) || (X == SID_LINK3_PROFILE_B) || (X == SID_LINK3_PROFILE_D))
#define IS_LINK2_PROFILE_ID(X) ((X == SID_LINK2_PROFILE_1) || (X == SID_LINK2_PROFILE_2))


/**
 * Describes the number of RX windows opened by the device
 */
enum sid_rx_window_count {
    /** Used to indicate device opens infinite RX windows */
    SID_RX_WINDOW_CNT_INFINITE = 0,
    /** Used to indicate device opens 5 RX windows */
    SID_RX_WINDOW_CNT_2 = 5,
    SID_RX_WINDOW_CNT_3 = 10,
    SID_RX_WINDOW_CNT_4 = 15,
    SID_RX_WINDOW_CNT_5 = 20,
   /** Used to indicate device is in continuous RX mode */
    SID_RX_WINDOW_CONTINUOUS = 0xFFFF,
};

/**
 * Describes the frequency of RX windows opened by the device (in ms) in synchronous mode
 */
enum sid_link2_rx_window_separation_ms {
    /** Used to indicate device opens a RX window every 63 ms */
    SID_LINK2_RX_WINDOW_SEPARATION_1 = 63,
    /** Used to indicate device opens a RX window every 315 (63*5) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_2 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 5),
    /** Used to indicate device opens a RX window every 630 (63*10) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_3 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 10),
    /** Used to indicate device opens a RX window every 945 (63*15) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_4 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 15),
    /** Used to indicate device opens a RX window every 2520 (63*40) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_5 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 40),
    /** Used to indicate device opens a RX window every 3150 (63*50) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_6 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 50),
    /** Used to indicate device opens a RX window every 5040 (63*80) ms */
    SID_LINK2_RX_WINDOW_SEPARATION_7 = (SID_LINK2_RX_WINDOW_SEPARATION_1 * 80),
};
#define IS_VALID_SID_LINK2_RX_WINDOW_SEPERATION(X) ( \
    X <= SID_LINK2_RX_WINDOW_SEPARATION_7 && X >= SID_LINK2_RX_WINDOW_SEPARATION_1 && \
    (X == SID_LINK2_RX_WINDOW_SEPARATION_1 || X % (SID_LINK2_RX_WINDOW_SEPARATION_2) == 0))


/**
 * Describes the Link Allocation Duration for FSK-WAN Link Profile 2 (Allocated)
 */
enum sid_link2_allocation_duration {
    SID_LINK2_ALLOCATION_DURATION_1HOUR = 0
};

/**
 * Describes the frequency of RX windows opened by the device (in ms) in asynchronous mode
 */
enum sid_link3_rx_window_separation_ms {
    /** Used to indicate device opens a RX window every 5000 ms */
    SID_LINK3_RX_WINDOW_SEPARATION_3 = 5000
};

/**
 * Describes the TX/RX wake up configuration of the device
 */
enum sid_unicast_wakeup_type {
    /** Used to indicate device does not participate in TX or RX events */
    SID_NO_WAKEUP = 0,
    /** Used to indicate device only participates in TX events */
    SID_TX_ONLY_WAKEUP = 1,
    /** Used to indicate device only participates in RX events */
    SID_RX_ONLY_WAKEUP = 2,
    /** Used to indicate device participates in both TX and RX events */
    SID_TX_AND_RX_WAKEUP = 3
};

/**
 * Describes unicast attributes of the device's configuration
 */
struct sid_device_profile_unicast_params {
    /** Used to indicate profile type of the device */
    enum sid_device_profile_id device_profile_id;
    /** Used to indicate the number of RX windows opened by the device */
    enum sid_rx_window_count rx_window_count;
    /** Used to indicate the frequency of RX windows opened by the device (in ms) */
    union sid_unicast_window_interval {
        /** Used to indicate the frequency of RX windows opened by the device (in ms) in synchronous mode */
        enum sid_link2_rx_window_separation_ms sync_rx_interval_ms;
        /** Used to indicate the frequency of RX windows opened by the device (in ms) in asynchronous mode */
        enum sid_link3_rx_window_separation_ms async_rx_interval_ms;
    } unicast_window_interval;
    /** Used to indicate the TX/RX wake up configuration of the device */
    enum sid_unicast_wakeup_type wakeup_type;
};

/**
 * Describes the LINK3 low latency configuration options
 */
enum sid_link3_low_latency {
    /** Used to indicate low latency feature is disabled */
    SID_LINK3_LOW_LATENCY_DISABLE = 0,
    /** Used to indicate low latency feature is enabled */
    SID_LINK3_LOW_LATENCY_ENABLE = 1,
    /** Delimiter to enum sid_link3_low_latency */
    SID_LINK3_LOW_LATENCY_LAST,
};

/**
 * Describes the LINK3 low latency attributes
 * These attributes are notified to the developer when there is a change in the attributes pertaining to
 * low latency.
 * There is no provision to set these attributes by the developer
 */
struct sid_link3_low_latency_config {
    /** Used to indicate the low latency feature status */
    bool enabled;
    /** latency index the Sidewalk stack uses to schedule the message */
    uint8_t latency;
    /** Number of repetitions the Sidewalk stack performs on a low latency message */
    uint8_t repetitions;
    /** Maximum number of low latency messages within one downlink separation */
    uint8_t rate_limit;
};

/**
 * Describes the configuration attributes of the device's profile
 */
struct sid_device_profile {
    /** Describes the unicast attributes of the device's synchronous configuration */
    struct sid_device_profile_unicast_params unicast_params;
    /** Describes the miscellaneous configuration for the two links supported */
    union sid_device_profile_misc_config {
        /** Describes the miscellaneous configuration for link3 */
        struct sid_link3_misc_config {
            enum sid_link3_low_latency low_latency;
        } link3_misc_config;
    } profile_misc_config;
};

/**
 * Describes the configuration attributes of registration over link2
 */
struct sid_link_type_2_registration_config {
    /** Used to enable registration over link2*/
    bool enable;
    /** Used to indicate the periodicity (in seconds) of registration process attempts */
    uint32_t periodicity_s;
};

struct sid_sub_ghz_links_config {
    /** Enable transmission Sidewalk stack metrics to Sidewalk cloud services using explicit commands */
    bool enable_link_metrics;
    /** Number of retries that Sidewalk stack metrics message can be retried by Sidewalk stack
     * configuring to 0 will disable the retries of the message
     */
    uint8_t metrics_msg_retries;
    /** sar dcr config*/
    uint8_t sar_dcr;
    int8_t link2_max_tx_power_in_dbm;
    int8_t link3_max_tx_power_in_dbm;
    struct sid_link_type_2_registration_config registration_config;
};

/**
 * Describes the gateway discovery policy modes for FSK
 */
enum sid_link_type_2_gw_discovery_policy {
    /** Default policy applied. The GW discovery behavior is same as 1.16 SDK release
        which is to connect to the first beacon which has opt-in flag set to true
     */
    SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_DEFAULT = 0,
    /** Policy optimized for latency to establish a connection,
     * and therefore choose the first-one that is available.
     */
    SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_FAST_CONNECTION = 1,
    /** Policy optimized for highest reliability to establish a connection,
     * the gateway with the best RSSI and healthy connection is selected.
     */
    SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_RELIABLE_CONNECTION = 2,
    /** Policy optimized for minimum power consumption. */
    SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_POWER_SAVE = 3,
    /** Policy for implementing a custom user scenario */
    SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_CUSTOM = 4,
    /** Delimiter to enum sid_link_type_2_gw_discovery_policy */
    SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_LAST,
};

/** Defines the necessary constants for GW discovery */
enum sid_link_type_2_gw_discovery_defines {
    SID_LINK_TYPE_2_GWD_BACKOFF_ARRAY_MAX_SIZE = 4,
};

/** Describes parameters for gateway discovery backoff intervals */
struct sid_link_type_2_gw_discovery_backoff_params {
    /** Number of elements in the backoff table */
    uint8_t retry_arr_size;
    /** Retry threshold for transition to the next intervals */
    uint8_t retry_threshold[SID_LINK_TYPE_2_GWD_BACKOFF_ARRAY_MAX_SIZE];
    /** Scan duration in seconds */
    uint16_t retry_scan_duration_s[SID_LINK_TYPE_2_GWD_BACKOFF_ARRAY_MAX_SIZE];
    /** Sleep time in seconds between scanning */
    uint16_t retry_sleep_time_s[SID_LINK_TYPE_2_GWD_BACKOFF_ARRAY_MAX_SIZE];
};

/**
 * Describes parameters for the custom gateway discovery policy
 */
struct sid_link_type_2_gw_discovery_custom_policy_params {
    /** Gateway discovery scanning timeout in seconds */
    uint16_t scan_timeout_secs;
    /** Continue to scan even after scan_timeout_secs has expired */
    bool continue_scanning_after_fail;
    /** Gateway discovery backoff parameters.
     * Set this parameter to NULL to use default values.
     */
    struct sid_link_type_2_gw_discovery_backoff_params *backoff_params;
    /** Minimum RSSI level at which connection is allowed.
     * Zero means the parameter is not used
     */
    int8_t min_rssi_to_connect;
    /** RSSI level at which EP initiates disconnection from the connected gateway.
     * Should be less than min_rssi_to_connect
     */
    int8_t threshold_rssi_to_disconnect;
    /** Number of beacons to calculate the average RSSI level
     * for threshold_rssi_to_disconnect. Zero means the parameter is not used
     */
    int8_t threshold_rssi_msg_num;
    /** The maximum number of consecutive beacons that can be missed
     * before the EP notifies disconnection.
     */
    uint8_t max_beacon_miss;
    /** The maximum number of beacons the EP can skip listening */
    uint8_t max_beacon_skip;
    /** Use gateway load to calculate gw score */
    bool use_gateway_load_for_selection;
    /** Gateway load at which EP initiates disconnection from the connected gateway.
     * The range is [0,3]. Zero means the parameter is not used
     */
    uint8_t gateway_load_to_disconnect;
    /** Use gateway connection health to calculate gw score */
    bool use_gateway_connection_health_for_selection;
    /** Use gateway transmit power rank to calculate gw score */
    bool use_gateway_tx_power_rank_for_selection;
};

/**
 * The structure used to configure the gateway discovery policy
 */
struct sid_link_type_2_gw_discovery_policy_config {
    /** Gateway discovery policy */
    enum sid_link_type_2_gw_discovery_policy policy;
    /** Parameters for the custom gateway discovery policy */
    struct sid_link_type_2_gw_discovery_custom_policy_params *policy_params;
    /** True: Set policy config, False: Get policy config */
    bool is_set;
};

/**
 * Describes the LINK2 GW Discovery events.
 * These events are notified to the developer through network control events
 */
enum sid_link_type_2_gw_discovery_event {
    /** Event indicating gateway under the selected discovered policy is not found */
    SID_LINK_TYPE_2_GW_DISCOVERY_EVENT_DISC_FAILED = 0,
    /** Event indicating the gateway is selected and beacon sync is done */
    SID_LINK_TYPE_2_GW_DISCOVERY_EVENT_BEACON_SYNC = 1,
    /** Event indicating the beacon sync was lost with the selected gateway */
    SID_LINK_TYPE_2_GW_DISCOVERY_EVENT_BEACON_SYNC_LOST = 2,
};

/** @} */

/// @endcond

#endif /* SID_900_CFG_H */
