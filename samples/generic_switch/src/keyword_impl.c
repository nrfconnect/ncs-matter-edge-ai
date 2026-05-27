/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "keyword.h"
#include "keyword_app.h"

#include "models/kws/nrf_edgeai_generated/nrf_edgeai_user_model.h"

static const keyword_class_cfg_t sKeywordClassesCfg[] = {
	[KEYWORD_OTHER] = {.name = "OTHER", .countNeeded = 4, .thresholdPercent = 0},
	[KEYWORD_SCENE_FOUR] = {.name = "SCENE_FOUR", .countNeeded = 2, .thresholdPercent = 0},
	[KEYWORD_SCENE_ONE] = {.name = "SCENE_ONE", .countNeeded = 2, .thresholdPercent = 0},
	[KEYWORD_SCENE_THREE] = {.name = "SCENE_THREE", .countNeeded = 2, .thresholdPercent = 0},
	[KEYWORD_SCENE_TWO] = {.name = "SCENE_TWO", .countNeeded = 2, .thresholdPercent = 0},
	[KEYWORD_SILENCE] = {.name = "SILENCE", .countNeeded = 4, .thresholdPercent = 0},
	[KEYWORD_TOGGLE_LIGHT] = {.name = "TOGGLE_LIGHT",
				  .countNeeded = 2,
				  .thresholdPercent = 0},
};

nrf_edgeai_t *KeywordAppModelGet(void)
{
	return nrf_edgeai_user_model_kws();
}

const keyword_class_cfg_t *KeywordAppGetClassesCfg(void)
{
	return sKeywordClassesCfg;
}

size_t KeywordAppGetClassesCount(void)
{
	return KEYWORDS_cnt;
}

uint16_t KeywordAppGetOtherClass(void)
{
	return KEYWORD_OTHER;
}

uint16_t KeywordAppGetSilenceClass(void)
{
	return KEYWORD_SILENCE;
}

bool KeywordAppIsCommandComponent(uint16_t predictedClass)
{
	return (predictedClass < KEYWORDS_cnt) && (predictedClass != KEYWORD_OTHER) &&
	       (predictedClass != KEYWORD_SILENCE);
}

bool KeywordAppIsActionable(uint16_t predictedClass)
{
	return (predictedClass == KEYWORD_SCENE_FOUR) || (predictedClass == KEYWORD_SCENE_ONE) ||
	       (predictedClass == KEYWORD_SCENE_THREE) || (predictedClass == KEYWORD_SCENE_TWO) ||
	       (predictedClass == KEYWORD_TOGGLE_LIGHT);
}
