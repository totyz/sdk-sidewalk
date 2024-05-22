#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOARD unit_testing)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sidewalk_test_assert)
set(SIDEWALK_BASE $ENV{ZEPHYR_BASE}/../sidewalk)

# add test file
FILE(GLOB app_sources src/*.c)
target_sources(testbinary PRIVATE
    ${app_sources}
    ${SIDEWALK_BASE}/subsys/sal/sid_pal/src/sid_assert.c    
)

target_include_directories(testbinary PRIVATE
    ${SIDEWALK_BASE}/subsys/sal/common/sid_pal_ifc
)
