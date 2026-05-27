/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @defgroup dmic DMIC control functions
 * @{
 * @ingroup ww_kws
 */

#ifndef __DMIC_H__
#define __DMIC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DMIC_SAMPLE_BYTES	(2)
#define DMIC_PCM_RATE		(16000)
#define SAMPLES_BLOCK_LENGTH_MS (10)

/**
 * @brief Initialize DMIC.
 *
 * @return Operation status result, 0 for success.
 */
int DmicInit(void);

/**
 * @brief Free the audio buffer acquired with @c dmic_read.
 *
 * @param buffer Audio buffer.
 */
void FreeDmicBuffer(void *buffer);

/**
 * @brief Get the DMIC device pointer.
 *
 * @return Pointer to the DMIC device, or NULL if not available.
 */
const struct device *GetDmicDevice(void);



/**
 * @brief Trigger or start the DMIC hardware to begin sampling.
 *
 * This function should be called to start the DMIC. Implementation may depend
 * on platform specifics.
 *
 * @return Operation status result, 0 for success.
 */
int DmicTriggerStart(void);

/**
 * @brief Read the audio buffer from the DMIC.
 *
 * @param buffer Pointer to a variable that receives the audio buffer address.
 * @param buffer_size Pointer to the received audio buffer size in bytes.
 * @param timeout_ms Timeout in milliseconds.
 * @return Operation status result, 0 for success.
 */
int DmicRead(void **buffer, size_t *buffer_size, int32_t timeout_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DMIC_H__ */

/**
 * @}
 */
