/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "keyword.h"

#include <string.h>

#include <nrf_edgeai/nrf_edgeai.h>
#include <nrf_edgeai/rt/nrf_edgeai_runtime_aux.h>
#include <zephyr/logging/log.h>

#include "dmic.h"
#include "zephyr/kernel.h"

LOG_MODULE_REGISTER(kw, CONFIG_MATTER_EDGEAI_WW_KW_LOG_LEVEL);

static nrf_edgeai_t *kwModel;

static keyword_runtime_ctx_t sKeywordRuntimeCtx;
static keyword_phrase_ctx_t sKeywordPhraseCtx;

static void ResetKeywordDetectionState(void);
static const keyword_class_cfg_t *GetKeywordClassCfg(uint16_t predictedClass);
static bool IsKeywordProbabilityAboveThreshold(uint16_t predictedClass, float probability);
static bool TryDetectKeywordCommand(uint16_t predictedClass, float probability,
				    const char **ppFirstKeywordName,
				    const char **ppSecondKeywordName, bool *pHasSecondKeyword,
				    uint8_t *pAverageProbability);

int KwInit(void)
{
	kwModel = KeywordAppModelGet();
	__ASSERT_NO_MSG(kwModel);

	const nrf_edgeai_err_t err = nrf_edgeai_init(kwModel);

	if (err) {
		LOG_ERR("Model initialization failed (err %d)", err);
		return -ENOENT;
	}

	return 0;
}

int KwProcess(uint8_t *const audioBuffer, const uint16_t numSamples, uint16_t *const kwClass)
{
	__ASSERT_NO_MSG(kwModel);
	__ASSERT_NO_MSG(numSamples == nrf_edgeai_input_window_size(kwModel));
	__ASSERT_NO_MSG(audioBuffer);
	__ASSERT_NO_MSG(kwClass);

	nrf_edgeai_err_t err;

	err = nrf_edgeai_feed_inputs(kwModel, audioBuffer, numSamples);

	FreeDmicBuffer(audioBuffer);

	if (err == NRF_EDGEAI_ERR_INPROGRESS) {
		return -EBUSY;
	} else if (err) {
		LOG_ERR("Failed to feed inputs (err %d)", err);
		return -EPERM;
	}

	err = nrf_edgeai_run_inference(kwModel);

	if (err == NRF_EDGEAI_ERR_INPROGRESS) {
		return -EBUSY;
	} else if (err) {
		LOG_ERR("Failed to run inference (err %d)", err);
		return -EPERM;
	}

#ifdef CONFIG_MATTER_EDGEAI_DETAILED_MODEL_LOGS
	const uint16_t predictedClass = kwModel->decoded_output.classif.predicted_class;
	const size_t numClasses = kwModel->decoded_output.classif.num_classes;
	const float *pProbabilities = kwModel->decoded_output.classif.probabilities.p_f32;

	if (predictedClass != KeywordAppGetSilenceClass()) {
		LOG_INF("kws probs [");
		for (size_t i = 0; i < numClasses; i++) {
			const keyword_class_cfg_t *pClassCfg = GetKeywordClassCfg(i);

			LOG_INF("%u (%s) with probability %f", (unsigned int)i, pClassCfg->name,
				pProbabilities[i]);
		}
		LOG_INF("]");
	}
#endif

	const uint16_t rawClass = kwModel->decoded_output.classif.predicted_class;
	const uint16_t nOut = nrf_edgeai_model_outputs_num(kwModel);

	if (nOut == 0U || rawClass >= nOut) {
		*kwClass = KeywordAppGetOtherClass();
		return 0;
	}

	const flt32_t probability = kwModel->decoded_output.classif.probabilities.p_f32[rawClass];

	*kwClass = (rawClass < KeywordAppGetClassesCount()) ? rawClass : KeywordAppGetOtherClass();

	const char *firstKeywordName = NULL;
	const char *secondKeywordName = NULL;
	bool hasSecondKeyword = false;
	uint8_t averageProbabilityPct = 0;

	if (TryDetectKeywordCommand(*kwClass, probability, &firstKeywordName, &secondKeywordName,
				    &hasSecondKeyword, &averageProbabilityPct)) {
		LOG_INF("Keyword detected: %s, %d %%", firstKeywordName, averageProbabilityPct);
		ResetKeywordDetectionState();
		return 1;
	}

	*kwClass = KeywordAppGetOtherClass();
	return 0;
}

void KwResetModel(void)
{
	nrf_edgeai_model_axon_init_persistent_vars(kwModel);
	ResetKeywordDetectionState();
}

static void ResetKeywordDetectionState(void)
{
	memset(&sKeywordRuntimeCtx, 0, sizeof(sKeywordRuntimeCtx));
	memset(&sKeywordPhraseCtx, 0, sizeof(sKeywordPhraseCtx));
}

static const keyword_class_cfg_t *GetKeywordClassCfg(uint16_t predictedClass)
{
	__ASSERT(predictedClass < KeywordAppGetClassesCount(), "Invalid keyword class: %u",
		 predictedClass);
	return &KeywordAppGetClassesCfg()[predictedClass];
}

static bool IsKeywordProbabilityAboveThreshold(uint16_t predictedClass, flt32_t probability)
{
	const keyword_class_cfg_t *pClassCfg = GetKeywordClassCfg(predictedClass);

	if (pClassCfg->thresholdPercent == 0) {
		return true;
	}

	return (probability * 100.0f) >= (flt32_t)pClassCfg->thresholdPercent;
}

static bool TryDetectKeywordCommand(uint16_t predictedClass, flt32_t probability,
				    const char **ppFirstKeywordName,
				    const char **ppSecondKeywordName, bool *pHasSecondKeyword,
				    uint8_t *pAverageProbability)
{
	const keyword_class_cfg_t *pClassCfg = GetKeywordClassCfg(predictedClass);
	const bool isCommandComponent = KeywordAppIsCommandComponent(predictedClass);
	const bool passesThreshold = isCommandComponent &&
				     IsKeywordProbabilityAboveThreshold(predictedClass, probability);

	*ppFirstKeywordName = NULL;
	*ppSecondKeywordName = NULL;
	*pHasSecondKeyword = false;
	*pAverageProbability = 0;

	const uint32_t nowMs = k_uptime_get_32();

	if (sKeywordRuntimeCtx.waitForClassChange) {
		if (predictedClass == sKeywordRuntimeCtx.blockedClass) {
			return false;
		}

		sKeywordRuntimeCtx.waitForClassChange = false;
	}

	if (!passesThreshold) {
		if (isCommandComponent) {
			sKeywordRuntimeCtx.hasActiveClass = false;
			sKeywordRuntimeCtx.count = 0;
			sKeywordRuntimeCtx.averageProbability = 0;
		}

		if ((sKeywordRuntimeCtx.nonKeywordCount == 0) ||
		    (sKeywordRuntimeCtx.nonKeywordClass != predictedClass)) {
			sKeywordRuntimeCtx.nonKeywordClass = predictedClass;
			sKeywordRuntimeCtx.nonKeywordCount = 0;
		}

		sKeywordRuntimeCtx.nonKeywordCount++;

		if (sKeywordPhraseCtx.hasFirstKeyword &&
		    (sKeywordRuntimeCtx.nonKeywordCount < pClassCfg->countNeeded)) {
			sKeywordPhraseCtx.detectedAtMs = nowMs;
		}

		if (sKeywordRuntimeCtx.nonKeywordCount >= pClassCfg->countNeeded) {
			sKeywordRuntimeCtx.hasActiveClass = false;
			sKeywordRuntimeCtx.count = 0;
			sKeywordRuntimeCtx.nonKeywordCount = 0;
			sKeywordRuntimeCtx.nonKeywordClass = 0;
			sKeywordRuntimeCtx.averageProbability = 0;
		}

		return false;
	}

	sKeywordRuntimeCtx.nonKeywordCount = 0;

	if (!sKeywordRuntimeCtx.hasActiveClass ||
	    (predictedClass != sKeywordRuntimeCtx.predictedClass)) {
		sKeywordRuntimeCtx.hasActiveClass = true;
		sKeywordRuntimeCtx.predictedClass = predictedClass;
		sKeywordRuntimeCtx.count = 0;
		sKeywordRuntimeCtx.nonKeywordCount = 0;
		sKeywordRuntimeCtx.nonKeywordClass = 0;
		sKeywordRuntimeCtx.averageProbability = 0;
	}

	sKeywordRuntimeCtx.count++;
	sKeywordRuntimeCtx.averageProbability +=
		(probability - sKeywordRuntimeCtx.averageProbability) / sKeywordRuntimeCtx.count;

	if (sKeywordPhraseCtx.hasFirstKeyword) {
		sKeywordPhraseCtx.detectedAtMs = nowMs;
	}

	if (sKeywordRuntimeCtx.count < pClassCfg->countNeeded) {
		return false;
	}

	const flt32_t detectedProbability = sKeywordRuntimeCtx.averageProbability;

	sKeywordRuntimeCtx.hasActiveClass = false;
	sKeywordRuntimeCtx.count = 0;
	sKeywordRuntimeCtx.nonKeywordCount = 0;
	sKeywordRuntimeCtx.nonKeywordClass = 0;
	sKeywordRuntimeCtx.averageProbability = 0;
	sKeywordRuntimeCtx.waitForClassChange = true;
	sKeywordRuntimeCtx.blockedClass = predictedClass;

	if (KeywordAppIsActionable(predictedClass)) {
		unsigned avgPct = (unsigned)(detectedProbability * 100.0f + 0.5f);

		if (avgPct > 100U) {
			avgPct = 100U;
		}
		*ppFirstKeywordName = pClassCfg->name;
		*ppSecondKeywordName = NULL;
		*pHasSecondKeyword = false;
		*pAverageProbability = (uint8_t)avgPct;
		return true;
	}

	return false;
}
