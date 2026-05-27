/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "wakeword.h"

#include <zephyr/logging/log.h>
#include <nrf_edgeai/nrf_edgeai.h>
#include <nrf_edgeai/rt/nrf_edgeai_runtime_aux.h>

#include "dmic.h"

LOG_MODULE_REGISTER(ww, CONFIG_MATTER_EDGEAI_WW_KW_LOG_LEVEL);

static nrf_edgeai_t *ww_model;

int ww_init(void)
{
	ww_model = wakeword_app_model_get();
	__ASSERT_NO_MSG(ww_model);

	const nrf_edgeai_err_t err = nrf_edgeai_init(ww_model);

	if (err) {
		LOG_ERR("Model initialization failed (err %d)", err);
		return -ENOENT;
	}

	return 0;
}

void ww_reset_model(void)
{
	nrf_edgeai_model_axon_init_persistent_vars(ww_model);
}

static bool ww_postprocess(void)
{
	static uint32_t ww_count;
	static uint32_t ww_history;

	const float ww_threshold = CONFIG_WAKEWORD_PROBABILITY_THRESHOLD / 1000.f;

	const uint16_t predicted_class = ww_model->decoded_output.classif.predicted_class;
	const float class_probability =
		ww_model->decoded_output.classif.probabilities.p_f32[predicted_class];
	const bool ww_detected = class_probability > ww_threshold;

	const bool oldest_entry = (bool)(ww_history & BIT(CONFIG_WAKEWORD_HISTORY_SIZE - 1));

	ww_count = ww_count + ww_detected - oldest_entry;
	ww_history = (ww_history << 1) | ww_detected;

	LOG_DBG("postprocess: count: %2u, probability: %f", ww_count, (double)class_probability);

	if (ww_count >= CONFIG_WAKEWORD_COUNT_THRESHOLD) {
		ww_count = 0;
		ww_history = 0;

		return true;
	}

	return false;
}

int ww_process(uint8_t *const audio_buffer, const uint16_t num_samples, bool *const ww_detected)
{
	__ASSERT_NO_MSG(audio_buffer);
	__ASSERT_NO_MSG(ww_model);
	__ASSERT_NO_MSG(num_samples == nrf_edgeai_input_window_size(ww_model));
	__ASSERT_NO_MSG(ww_detected);

	nrf_edgeai_err_t err;

	err = nrf_edgeai_feed_inputs(ww_model, audio_buffer, num_samples);
	free_dmic_buffer(audio_buffer);

	if (err == NRF_EDGEAI_ERR_INPROGRESS) {
		return -EBUSY;
	} else if (err) {
		LOG_ERR("Failed to feed inputs (err %d)", err);
		return -EPERM;
	}

	err = nrf_edgeai_run_inference(ww_model);
	if (err == NRF_EDGEAI_ERR_INPROGRESS) {
		return -EBUSY;
	} else if (err) {
		LOG_ERR("Failed to run inference (err %d)", err);
		return -EPERM;
	}

#ifdef CONFIG_DETAILED_MODEL_LOGS
	const uint16_t predicted_class = ww_model->decoded_output.classif.predicted_class;

	LOG_INF("%u with probability %f", predicted_class,
		ww_model->decoded_output.classif.probabilities.p_f32[predicted_class]);
#endif

	*ww_detected = ww_postprocess();

	return 0;
}
