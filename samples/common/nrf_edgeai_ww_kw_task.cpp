/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "nrf_edgeai_ww_kw_task.h"

#include <errno.h>

#include <zephyr/audio/dmic.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "dmic.h"
#include "keyword.h"
#include "wakeword.h"

LOG_MODULE_REGISTER(edgeai_ww_kw, CONFIG_MATTER_EDGEAI_WW_KW_LOG_LEVEL);

extern struct k_sem gMatterStartedSem;

namespace {

enum detection_state_e {
	WAITING_FOR_WAKEWORD,
	WAITING_FOR_KEYWORDS,
};

struct EdgeAITaskContext {
	detection_state_e app_state{WAITING_FOR_WAKEWORD};
	uint32_t kws_start_time{0};
	uint16_t class_detected{0};

	static constexpr int32_t kReadTimeoutMs = 100;
};

const struct device *const dmic_dev = DEVICE_DT_GET(DT_NODELABEL(dmic_dev));

uint16_t AudioSampleCount(size_t audio_buffer_size)
{
	return static_cast<uint16_t>(audio_buffer_size / DMIC_SAMPLE_BYTES);
}

bool ProcessAudio(void *&audio_buffer, size_t &audio_buffer_size)
{
	const int err = dmic_read(dmic_dev, 0, &audio_buffer, &audio_buffer_size,
				  EdgeAITaskContext::kReadTimeoutMs);
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
		free_dmic_buffer(audio_buffer);
		return true;
	}

	return false;
}

void ProcessWakeword(EdgeAITaskContext &ctx, void *audio_buffer, size_t audio_buffer_size)
{
	bool ww_detected = false;

	const int err = ww_process(reinterpret_cast<uint8_t *>(audio_buffer),
				   AudioSampleCount(audio_buffer_size), &ww_detected);
	if (err == -EBUSY) {
		return;
	}
	if (err < 0) {
		LOG_ERR("Wakeword detection failed (err %d)", err);
		return;
	}
	if (!ww_detected) {
		return;
	}

	LOG_INF("wakeword detected, looking for keywords");
	kw_reset_model();
	ctx.kws_start_time = k_uptime_get_32();
	ctx.app_state = WAITING_FOR_KEYWORDS;
	EdgeAIWwKwOnWakewordDetected();
}

void ProcessKeyword(EdgeAITaskContext &ctx, void *audio_buffer, size_t audio_buffer_size)
{
	if (k_uptime_get_32() - ctx.kws_start_time > CONFIG_KEYWORD_DETECTION_TIMEOUT_S * 1000) {
		ww_reset_model();
		ctx.app_state = WAITING_FOR_WAKEWORD;
		LOG_INF("Waiting for wakeword...");
		return;
	}

	const int kws = kw_process(reinterpret_cast<uint8_t *>(audio_buffer),
				   AudioSampleCount(audio_buffer_size), &ctx.class_detected);
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

	EdgeAIWwKwHandleKeyword(ctx.class_detected);

	ww_reset_model();
	ctx.app_state = WAITING_FOR_WAKEWORD;
	LOG_INF("Waiting for wakeword...");
	EdgeAIWwKwOnKeywordSessionEnd();
}

void ProcessState(EdgeAITaskContext &ctx, void *audio_buffer, size_t audio_buffer_size)
{
	switch (ctx.app_state) {
	case WAITING_FOR_WAKEWORD:
		ProcessWakeword(ctx, audio_buffer, audio_buffer_size);
		break;
	case WAITING_FOR_KEYWORDS:
		ProcessKeyword(ctx, audio_buffer, audio_buffer_size);
		break;
	}
}

void EdgeAITaskThread();

K_THREAD_DEFINE(sEdgeAITaskThreadId, CONFIG_EDGEAI_TASK_THREAD_STACK_SIZE, EdgeAITaskThread, NULL,
		NULL, NULL, CONFIG_EDGEAI_TASK_THREAD_PRIORITY, K_FP_REGS, SYS_FOREVER_MS);

void EdgeAITaskThread()
{
	if (!EdgeAITask::Instance().Init()) {
		LOG_ERR("Edge AI task initialization failed");
		return;
	}

	EdgeAITaskContext ctx;

	while (true) {
		void *audio_buffer;
		size_t audio_buffer_size;

		if (ProcessAudio(audio_buffer, audio_buffer_size)) {
			continue;
		}

		ProcessState(ctx, audio_buffer, audio_buffer_size);
	}
}

} /* namespace */

bool EdgeAITask::Init()
{
	if (dmic_init()) {
		LOG_ERR("Failed to initialize DMIC");
		return false;
	}

	if (ww_init() != 0) {
		LOG_ERR("Failed to initialize wakeword detection");
		return false;
	}

	if (kw_init() != 0) {
		LOG_ERR("Failed to initialize keyword detection");
		return false;
	}

	LOG_INF("Edge AI initialization completed, waiting for Matter server");
	k_sem_take(&gMatterStartedSem, K_FOREVER);
	LOG_INF("Matter server is ready, starting Edge AI voice capture");

	if (dmic_trigger(dmic_dev, DMIC_TRIGGER_START) < 0) {
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
	LOG_INF("Waiting for wakeword...");
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
