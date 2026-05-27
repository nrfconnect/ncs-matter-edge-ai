/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <zephyr/autoconf.h>

namespace Nrf
{
namespace DimmingEffect
{

struct Config {
	/** Total duration of the effect in seconds. */
	int effectTimeoutS{5};
	/**
	 * Number of bright/dim pairs is MAX(10, blinkPairsMultiplier * effectTimeoutS),
	 * matching the Matter ambient sensing / switch wakeword PWM behavior.
	 */
	int blinkPairsMultiplier{3};
	/** Work reschedule period in milliseconds (smaller = smoother, more CPU). */
	int fxTickMs{CONFIG_MATTER_EDGEAI_DIMMING_FX_TICK_MS};
};

using OnComplete = void (*)(void *ctx);

/** True when built with CONFIG_PWM and pwm_led0..pwm_led3 devicetree aliases. */
bool IsAvailable();

/**
 * Initialize timer/work items and static PWM devices.
 * Safe to call when IsAvailable() is false (no-op, returns 0).
 *
 * @return 0 on success, negative errno on failure
 */
int Init(const Config &cfg, OnComplete onComplete = nullptr, void *onCompleteCtx = nullptr);

/** Start the dimming animation (typically from Nrf::PostTask). */
void Start();

/** Stop the dimming animation */
void Stop();

/**
 * Restart the one-shot timer so expiry runs Start via Nrf::PostTask (switch wakeword path).
 * No-op when IsAvailable() is false.
 */
void RequestStartViaTimer();

} // namespace DimmingEffect
} // namespace Nrf
