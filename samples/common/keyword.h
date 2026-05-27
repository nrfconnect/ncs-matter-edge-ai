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
	size_t countNeeded;
	uint8_t thresholdPercent;
} keyword_class_cfg_t;

typedef struct {
	bool hasActiveClass;
	uint16_t predictedClass;
	size_t count;
	size_t nonKeywordCount;
	uint16_t nonKeywordClass;
	float averageProbability;
	bool waitForClassChange;
	uint16_t blockedClass;
} keyword_runtime_ctx_t;

typedef struct {
	bool hasFirstKeyword;
	uint16_t firstClass;
	float firstProbability;
	uint32_t detectedAtMs;
} keyword_phrase_ctx_t;

/** @return Pointer to the application keyword spotting model. */
nrf_edgeai_t *KeywordAppModelGet(void);

/** @return Table indexed by model class id. */
const keyword_class_cfg_t *KeywordAppGetClassesCfg(void);

/** @return Number of entries in @ref KeywordAppGetClassesCfg. */
size_t KeywordAppGetClassesCount(void);

/** @return Class index used for non-command / unknown detections. */
uint16_t KeywordAppGetOtherClass(void);

/** @return Class index used for silence / background. */
uint16_t KeywordAppGetSilenceClass(void);

bool KeywordAppIsCommandComponent(uint16_t predictedClass);
bool KeywordAppIsActionable(uint16_t predictedClass);

int KwInit(void);
void KwResetModel(void);

/**
 * @return 0 while still listening, 1 when a full keyword command is recognized (see @a kwClass),
 *         -EBUSY if more audio is needed, or another negative errno on error.
 */
int KwProcess(uint8_t *const audioBuffer, const uint16_t numSamples, uint16_t *const kwClass);

#ifdef __cplusplus
}
#endif

#endif
