#
# Copyright (c) 2026 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

set(MATTER_EDGEAI_COMMON_DIR ${ZEPHYR_NCS_MATTER_EDGE_AI_MODULE_DIR}/samples/common)

target_include_directories(app PRIVATE
    ${MATTER_EDGEAI_COMMON_DIR}
)

target_sources_ifdef(CONFIG_MATTER_EDGEAI_DMIC app PRIVATE ${MATTER_EDGEAI_COMMON_DIR}/dmic.c)
target_sources_ifdef(CONFIG_MATTER_EDGEAI_DIMMING app PRIVATE ${MATTER_EDGEAI_COMMON_DIR}/dimming_effect.cpp)
target_sources_ifdef(CONFIG_MATTER_EDGEAI app PRIVATE
    ${MATTER_EDGEAI_COMMON_DIR}/nrf_edgeai_ww_kw_task.cpp
    ${MATTER_EDGEAI_COMMON_DIR}/keyword.c
    ${MATTER_EDGEAI_COMMON_DIR}/wakeword.c
)
