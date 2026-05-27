/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __KEYWORD_H__
#define __KEYWORD_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <nrf_edgeai/nrf_edgeai.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *name;
	size_t count_needed;
	uint8_t threshold_percent;
} keyword_class_cfg_t;

typedef struct {
	bool has_active_class;
	uint16_t predicted_class;
	size_t count;
	size_t non_keyword_count;
	uint16_t non_keyword_class;
	float average_probability;
	bool wait_for_class_change;
	uint16_t blocked_class;
} keyword_runtime_ctx_t;

typedef struct {
	bool has_first_keyword;
	uint16_t first_class;
	float first_probability;
	uint32_t detected_at_ms;
} keyword_phrase_ctx_t;

/** @return Pointer to the application keyword spotting model. */
nrf_edgeai_t *keyword_app_model_get(void);

/** @return Table indexed by model class id. */
const keyword_class_cfg_t *keyword_app_get_classes_cfg(void);

/** @return Number of entries in @ref keyword_app_get_classes_cfg. */
size_t keyword_app_get_classes_count(void);

/** @return Class index used for non-command / unknown detections. */
uint16_t keyword_app_get_other_class(void);

/** @return Class index used for silence / background. */
uint16_t keyword_app_get_silence_class(void);

bool keyword_app_is_command_component(uint16_t predicted_class);
bool keyword_app_is_actionable(uint16_t predicted_class);

int kw_init(void);
void kw_reset_model(void);

/**
 * @return 0 while still listening, 1 when a full keyword command is recognized (see @a kw_class),
 *         -EBUSY if more audio is needed, or another negative errno on error.
 */
int kw_process(uint8_t *const audio_buffer, const uint16_t num_samples, uint16_t *const kw_class);

#ifdef __cplusplus
}
#endif

#endif
