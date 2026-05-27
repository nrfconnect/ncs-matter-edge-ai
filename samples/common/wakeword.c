/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "wakeword.h"

#include <nrf_edgeai/nrf_edgeai.h>
#include <nrf_edgeai/rt/nrf_edgeai_runtime_aux.h>
#include <zephyr/logging/log.h>

#include "dmic.h"

LOG_MODULE_REGISTER(ww, CONFIG_MATTER_EDGEAI_WW_KW_LOG_LEVEL);

static nrf_edgeai_t *wwModel;

int WwInit(void)
{
	wwModel = WakewordAppModelGet();
	__ASSERT_NO_MSG(wwModel);

	const nrf_edgeai_err_t err = nrf_edgeai_init(wwModel);

	if (err) {
		LOG_ERR("Model initialization failed (err %d)", err);
		return -ENOENT;
	}

	return 0;
}

void WwResetModel(void)
{
	nrf_edgeai_model_axon_init_persistent_vars(wwModel);
}

static bool WwPostprocess(void)
{
	static uint32_t wwCount;
	static uint32_t wwHistory;

	const float wwThreshold = CONFIG_MATTER_EDGEAI_WAKEWORD_PROBABILITY_THRESHOLD / 1000.f;

	const uint16_t predictedClass = wwModel->decoded_output.classif.predicted_class;
	const float classProbability =
		wwModel->decoded_output.classif.probabilities.p_f32[predictedClass];
	const bool wwDetected = classProbability > wwThreshold;

	const bool oldestEntry =
		(bool)(wwHistory & BIT(CONFIG_MATTER_EDGEAI_WAKEWORD_HISTORY_SIZE - 1));

	wwCount = wwCount + wwDetected - oldestEntry;
	wwHistory = (wwHistory << 1) | wwDetected;

	LOG_DBG("postprocess: count: %2u, probability: %f", wwCount, (double)classProbability);

	if (wwCount >= CONFIG_MATTER_EDGEAI_WAKEWORD_COUNT_THRESHOLD) {
		wwCount = 0;
		wwHistory = 0;

		return true;
	}

	return false;
}

int WwProcess(uint8_t *const audioBuffer, const uint16_t numSamples, bool *const wwDetected)
{
	__ASSERT_NO_MSG(audioBuffer);
	__ASSERT_NO_MSG(wwModel);
	__ASSERT_NO_MSG(numSamples == nrf_edgeai_input_window_size(wwModel));
	__ASSERT_NO_MSG(wwDetected);

	nrf_edgeai_err_t err = nrf_edgeai_feed_inputs(wwModel, audioBuffer, numSamples);
	FreeDmicBuffer(audioBuffer);

	if (err == NRF_EDGEAI_ERR_INPROGRESS) {
		return -EBUSY;
	} else if (err) {
		LOG_ERR("Failed to feed inputs (err %d)", err);
		return -EPERM;
	}

	err = nrf_edgeai_run_inference(wwModel);
	if (err == NRF_EDGEAI_ERR_INPROGRESS) {
		return -EBUSY;
	} else if (err) {
		LOG_ERR("Failed to run inference (err %d)", err);
		return -EPERM;
	}

#ifdef CONFIG_MATTER_EDGEAI_DETAILED_MODEL_LOGS
	const uint16_t predictedClass = wwModel->decoded_output.classif.predicted_class;

	LOG_INF("%u with probability %f", predictedClass,
		wwModel->decoded_output.classif.probabilities.p_f32[predictedClass]);
#endif

	*wwDetected = WwPostprocess();

	return 0;
}
