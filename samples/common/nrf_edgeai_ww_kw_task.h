/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <atomic>
#include <stdint.h>

/**
 * @defgroup edgeai_ww_kw_task Edge AI wakeword and keyword task
 * @{
 *
 * Generic Matter Edge AI voice pipeline: DMIC capture, wakeword detection,
 * keyword spotting, and dispatch to application hooks.
 *
 * Application-specific behavior (Matter actions, model classes, dimming, PDM
 * gain, etc.) must be implemented in the sample via the @ref edgeai_ww_kw_app
 * callbacks declared below.
 */

class EdgeAITask
{
public:
	/**
	 * @brief Get the singleton instance of EdgeAITask.
	 *
	 * @return Reference to the EdgeAITask instance.
	 */
	static EdgeAITask &Instance()
	{
		static EdgeAITask sEdgeAITask;
		return sEdgeAITask;
	}

	/**
	 * @brief Initialize DMIC, models, and wait for Matter server readiness.
	 *
	 * @return true on success, false otherwise.
	 */
	bool Init();

	/**
	 * @brief Start the Edge AI task thread.
	 *
	 * @return true if the thread was started, false otherwise.
	 */
	bool Start();

	/** @brief Enable voice capture processing. */
	void Enable();

	/** @brief Disable voice capture processing. */
	void Disable();

	/**
	 * @brief Check if voice processing is enabled.
	 *
	 * @return true if enabled, false otherwise.
	 */
	bool IsEnabled() const { return enabled.load(std::memory_order_acquire); }

private:
	std::atomic<bool> enabled{false};
};

/**
 * @defgroup edgeai_ww_kw_app Application hooks for Edge AI wakeword/keyword task
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handle a fully recognized keyword command.
 *
 * Called from the Edge AI thread; schedule Matter work onto the Matter
 * thread if needed.
 *
 * @param keyword_class Application-defined keyword class index.
 */
void EdgeAIWwKwHandleKeyword(uint16_t keyword_class);

/**
 * @brief Notification that a wakeword was detected and keyword listening started.
 */
void EdgeAIWwKwOnWakewordDetected(void);

/**
 * @brief Notification that the keyword listening session ended.
 *
 * Called after a recognized command or when returning to wakeword listening
 * following keyword processing.
 */
void EdgeAIWwKwOnKeywordSessionEnd(void);

/**
 * @brief Application-specific setup after generic Edge AI initialization.
 *
 * Invoked after DMIC trigger succeeds. Use for PDM gain, dimming effect
 * configuration, or other board-specific setup.
 *
 * @return true on success, false on failure.
 */
bool EdgeAIWwKwAppInit(void);

#ifdef __cplusplus
}
#endif

/** @} */ /* edgeai_ww_kw_app */
/** @} */ /* edgeai_ww_kw_task */
