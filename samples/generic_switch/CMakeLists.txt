#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(matter_switch)

# Enable GNU STD support.
include(${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/config/nrfconnect/app/enable-gnu-std.cmake)

# NORDIC SDK APP START

# Include all source files that are located in the Matter common directory.
include(${ZEPHYR_NRF_MODULE_DIR}/samples/matter/common/cmake/source_common.cmake)

# Include Data Model utils
include(${ZEPHYR_NRF_MODULE_DIR}/samples/matter/common/cmake/data_model.cmake)

include(${ZEPHYR_NRF_MODULE_DIR}/samples/matter/common/cmake/zap_helpers.cmake)
ncs_get_zap_parent_dir(ZAP_PARENT_DIR)

include(${ZEPHYR_NCS_MATTER_EDGE_AI_MODULE_DIR}/samples/common/matter_edge_ai_common.cmake)

target_include_directories(app PRIVATE
  src
  src/models/kws/nrf_edgeai_generated
  src/models/wakeword/nrf_edgeai_generated
  src/edgeAI
  ${ZAP_PARENT_DIR}
)

target_compile_options(app PRIVATE -Wno-deprecated-declarations)

target_sources(app PRIVATE
  src/main.cpp
  src/app_task.cpp
  src/nrf_edgeai_ww_kw_impl.cpp
  src/keyword_impl.c
  src/wakeword_impl.c
  src/models/wakeword/nrf_edgeai_generated/nrf_edgeai_user_model.c
  src/models/kws/nrf_edgeai_generated/nrf_edgeai_user_model.c
  src/generic_switch.cpp
)

ncs_configure_data_model()
