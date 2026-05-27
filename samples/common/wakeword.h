/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __WAKEWORD_H__
#define __WAKEWORD_H__

#include <stdbool.h>
#include <stdint.h>

#include <nrf_edgeai/nrf_edgeai.h>

#ifdef __cplusplus
extern "C" {
#endif

nrf_edgeai_t *wakeword_app_model_get(void);

int ww_init(void);
void ww_reset_model(void);

/**
 * @return 0 on success, -EBUSY if more data is needed, negative errno on error.
 */
int ww_process(uint8_t *const audio_buffer, const uint16_t num_samples, bool *const ww_detected);

#ifdef __cplusplus
}
#endif

#endif /* __WAKEWORD_H__ */
