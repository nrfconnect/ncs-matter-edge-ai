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

nrf_edgeai_t *WakewordAppModelGet(void);

int WwInit(void);
void WwResetModel(void);

/**
 * @return 0 on success, -EBUSY if more data is needed, negative errno on error.
 */
int WwProcess(uint8_t *const audioBuffer, const uint16_t numSamples, bool *const wwDetected);

#ifdef __cplusplus
}
#endif

#endif /* __WAKEWORD_H__ */
