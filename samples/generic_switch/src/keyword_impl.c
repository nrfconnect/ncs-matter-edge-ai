/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "keyword.h"
#include "keyword_app.h"

#include "models/kws/nrf_edgeai_generated/nrf_edgeai_user_model.h"

static const keyword_class_cfg_t s_keyword_classes_cfg[] = {
    [KEYWORD_LIGHT]        = { .name = "LIGHT", .count_needed = 2, .threshold_percent = 0 },
    [KEYWORD_OFF]          = { .name = "OFF", .count_needed = 2, .threshold_percent = 20 },
    [KEYWORD_ON]           = { .name = "ON", .count_needed = 2, .threshold_percent = 20 },
    [KEYWORD_OTHER]        = { .name = "OTHER", .count_needed = 4, .threshold_percent = 0 },
    [KEYWORD_SCENE_FOUR]   = { .name = "SCENE_FOUR", .count_needed = 2, .threshold_percent = 0 },
    [KEYWORD_SCENE_ONE]    = { .name = "SCENE_ONE", .count_needed = 2, .threshold_percent = 0 },
    [KEYWORD_SCENE_THREE]  = { .name = "SCENE_THREE", .count_needed = 2, .threshold_percent = 0 },
    [KEYWORD_SCENE_TWO]    = { .name = "SCENE_TWO", .count_needed = 2, .threshold_percent = 0 },
    [KEYWORD_SILENCE]      = { .name = "SILENCE", .count_needed = 4, .threshold_percent = 0 },
    [KEYWORD_TOGGLE_LIGHT] = { .name = "TOGGLE_LIGHT", .count_needed = 2, .threshold_percent = 0 },
};

nrf_edgeai_t *keyword_app_model_get(void)
{
	return nrf_edgeai_user_model_kws();
}

const keyword_class_cfg_t * keyword_app_get_classes_cfg(void)
{
    return s_keyword_classes_cfg;
}

size_t keyword_app_get_classes_count(void)
{
    return KEYWORDS_cnt;
}

uint16_t keyword_app_get_other_class(void)
{
    return KEYWORD_OTHER;
}

uint16_t keyword_app_get_silence_class(void)
{
    return KEYWORD_SILENCE;
}

bool keyword_app_is_command_component(uint16_t predicted_class)
{
    return (predicted_class < KEYWORDS_cnt) && (predicted_class != KEYWORD_OTHER) && (predicted_class != KEYWORD_SILENCE);
}

bool keyword_app_is_actionable(uint16_t predicted_class)
{
    return (predicted_class == KEYWORD_SCENE_FOUR) || (predicted_class == KEYWORD_SCENE_ONE) ||
        (predicted_class == KEYWORD_SCENE_THREE) || (predicted_class == KEYWORD_SCENE_TWO) ||
        (predicted_class == KEYWORD_TOGGLE_LIGHT);
}
