/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file keyword_app.h
 * @brief Application-specific keyword class indices for the generic_switch sample.
 *
 * Indices must match the KWS model output classes.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum keyword_labels_e {
	KEYWORD_LIGHT = 0,
	KEYWORD_OFF = 1,
	KEYWORD_ON = 2,
	KEYWORD_OTHER = 3,
	KEYWORD_SCENE_FOUR = 4,
	KEYWORD_SCENE_ONE = 5,
	KEYWORD_SCENE_THREE = 6,
	KEYWORD_SCENE_TWO = 7,
	KEYWORD_SILENCE = 8,
	KEYWORD_TOGGLE_LIGHT = 9,

	KEYWORDS_cnt
} keyword_labels_t;
