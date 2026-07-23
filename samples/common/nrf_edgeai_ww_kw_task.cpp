/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "nrf_edgeai_ww_kw_task.h"

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "dmic.h"
#include "keyword.h"
#include "wakeword.h"

LOG_MODULE_REGISTER(edgeai_ww_kw, CONFIG_MATTER_EDGEAI_WW_KW_LOG_LEVEL);

extern struct k_sem gMatterStartedSem;

namespace
{

enum DetectionState {
	waitingForWakeword,
	waitingForKeywords,
};

struct EdgeAITaskContext {
	DetectionState appState{waitingForWakeword};
	uint32_t kwsStartTime{0};
	uint16_t classDetected{0};

	static constexpr int32_t kReadTimeoutMs = 100;
};

uint16_t AudioSampleCount(size_t audioBufferSize)
{
	return static_cast<uint16_t>(audioBufferSize / DMIC_SAMPLE_BYTES);
}

bool ProcessAudio(void *&audioBuffer, size_t &audioBufferSize)
{
	const int err = DmicRead(&audioBuffer, &audioBufferSize, EdgeAITaskContext::kReadTimeoutMs);
	if (err != 0) {
		if (err == -EAGAIN || err == -EBUSY) {
			k_yield();
		} else {
			LOG_WRN("DMIC read failed (err %d)", err);
			k_sleep(K_MSEC(1));
		}
		return true;
	}

	if (!EdgeAITask::Instance().IsEnabled()) {
		FreeDmicBuffer(audioBuffer);
		return true;
	}

	return false;
}

void ProcessWakeword(EdgeAITaskContext &ctx, void *audioBuffer, size_t audioBufferSize)
{
	bool wwDetected = false;

	const int err = WwProcess(reinterpret_cast<uint8_t *>(audioBuffer),
				  AudioSampleCount(audioBufferSize), &wwDetected);
	if (err == -EBUSY) {
		return;
	}
	if (err < 0) {
		LOG_ERR("Wakeword detection failed (err %d)", err);
		return;
	}
	if (!wwDetected) {
		return;
	}

	LOG_INF("wakeword detected, looking for keywords");
	KwResetModel();
	ctx.kwsStartTime = k_uptime_get_32();
	ctx.appState = waitingForKeywords;
	EdgeAIWwKwOnWakewordDetected();
}

void ProcessKeyword(EdgeAITaskContext &ctx, void *audioBuffer, size_t audioBufferSize)
{
	if (k_uptime_get_32() - ctx.kwsStartTime >
	    CONFIG_MATTER_EDGEAI_KEYWORD_DETECTION_TIMEOUT_S * 1000) {
		WwResetModel();
		ctx.appState = waitingForWakeword;
		LOG_INF("Waiting for wakeword...");
		return;
	}

	const int kws = KwProcess(reinterpret_cast<uint8_t *>(audioBuffer),
				  AudioSampleCount(audioBufferSize), &ctx.classDetected);
	if (kws == -EBUSY) {
		return;
	}
	if (kws < 0) {
		LOG_ERR("Keyword detection failed (err %d)", kws);
		return;
	}
	if (kws == 0) {
		return;
	}

	EdgeAIWwKwHandleKeyword(ctx.classDetected);

	WwResetModel();
	ctx.appState = waitingForWakeword;
	LOG_INF("Waiting for wakeword...");
	EdgeAIWwKwOnKeywordSessionEnd();
}

void ProcessState(EdgeAITaskContext &ctx, void *audioBuffer, size_t audioBufferSize)
{
	if (!audioBuffer || audioBufferSize == 0) {
		return;
	}

	switch (ctx.appState) {
	case waitingForWakeword:
		ProcessWakeword(ctx, audioBuffer, audioBufferSize);
		break;
	case waitingForKeywords:
		ProcessKeyword(ctx, audioBuffer, audioBufferSize);
		break;
	}
}

void EdgeAITaskThread();

K_THREAD_DEFINE(sEdgeAITaskThreadId, CONFIG_MATTER_EDGEAI_TASK_THREAD_STACK_SIZE, EdgeAITaskThread,
		NULL, NULL, NULL, CONFIG_MATTER_EDGEAI_TASK_THREAD_PRIORITY, K_FP_REGS,
		SYS_FOREVER_MS);

void EdgeAITaskThread()
{
	if (!EdgeAITask::Instance().Init()) {
		LOG_ERR("Edge AI task initialization failed");
		return;
	}

	LOG_INF("Waiting for wakeword...");

	EdgeAITaskContext ctx;

	while (true) {
		void *audioBuffer = nullptr;
		size_t audioBufferSize = 0;

		if (ProcessAudio(audioBuffer, audioBufferSize)) {
			continue;
		}

		ProcessState(ctx, audioBuffer, audioBufferSize);
	}
}

} /* namespace */

bool EdgeAITask::Init()
{
	if (DmicInit()) {
		LOG_ERR("Failed to initialize DMIC");
		return false;
	}

	if (WwInit() != 0) {
		LOG_ERR("Failed to initialize wakeword detection");
		return false;
	}

	if (KwInit() != 0) {
		LOG_ERR("Failed to initialize keyword detection");
		return false;
	}

	LOG_INF("Edge AI initialization completed, waiting for Matter server");
	k_sem_take(&gMatterStartedSem, K_FOREVER);
	LOG_INF("Matter server is ready, starting Edge AI voice capture");

	if (DmicTriggerStart() < 0) {
		LOG_ERR("Failed to start DMIC");
		return false;
	}

	if (!EdgeAIWwKwAppInit()) {
		LOG_ERR("Application Edge AI setup failed");
		return false;
	}

	return true;
}

bool EdgeAITask::Start()
{
	k_thread_start(sEdgeAITaskThreadId);
	return true;
}

void EdgeAITask::Enable()
{
	LOG_INF("Edge AI task enabled");
	enabled.store(true, std::memory_order_release);
}

void EdgeAITask::Disable()
{
	enabled.store(false, std::memory_order_release);
	LOG_INF("Edge AI listening disabled");
}
