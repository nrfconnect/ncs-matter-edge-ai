/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "dimming_effect.h"

#include "app/task_executor.h"
#include "board/board.h"
#include "pwm/pwm_device.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h> /* MAX, MIN, ARG_UNUSED */

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dimming_effect, LOG_LEVEL_INF);

namespace Nrf
{
namespace DimmingEffect
{
namespace
{

#if defined(CONFIG_PWM) && DT_HAS_ALIAS(pwm_led0) && DT_HAS_ALIAS(pwm_led1) &&                     \
	DT_HAS_ALIAS(pwm_led2) && DT_HAS_ALIAS(pwm_led3)

constexpr uint8_t kPwmMinLevel = 0;
constexpr uint8_t kPwmMaxLevel = 254;

const struct pwm_dt_spec kPwmSpecs[] = {
	PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0)),
	PWM_DT_SPEC_GET(DT_ALIAS(pwm_led1)),
	PWM_DT_SPEC_GET(DT_ALIAS(pwm_led2)),
	PWM_DT_SPEC_GET(DT_ALIAS(pwm_led3)),
};

Nrf::PWMDevice sPwmDevices[ARRAY_SIZE(kPwmSpecs)];
bool sPwmChannelOk[ARRAY_SIZE(kPwmSpecs)];

k_timer sEffectTimer;
k_work_delayable sDimWork;
int sElapsedMs;

Config sCfg;
OnComplete sOnComplete;
void *sOnCompleteCtx;

int BlinkPairCount()
{
	return MAX(10, sCfg.blinkPairsMultiplier * sCfg.effectTimeoutS);
}

/** @return 0 if at least one channel initialized, -ENODEV if all failed */
int InitPwmDevicesAndTurnOn()
{
	unsigned ok = 0;

	for (unsigned i = 0; i < ARRAY_SIZE(sPwmDevices); i++) {
		Nrf::PWMDevice &dev = sPwmDevices[i];

		sPwmChannelOk[i] = false;
		if (dev.Init(&kPwmSpecs[i], kPwmMinLevel, kPwmMaxLevel, kPwmMaxLevel) != 0) {
			LOG_ERR("PWMDevice init failed for channel %u", i);
			continue;
		}
		dev.SetCallbacks(nullptr, nullptr);
		(void)dev.InitiateAction(Nrf::PWMDevice::ON_ACTION, 0, nullptr);
		sPwmChannelOk[i] = true;
		ok++;
	}

	return ok > 0 ? 0 : -ENODEV;
}

void SuppressAllPwmOutputs()
{
	for (unsigned i = 0; i < ARRAY_SIZE(sPwmDevices); i++) {
		if (sPwmChannelOk[i]) {
			sPwmDevices[i].SuppressOutput();
		}
	}
}

void DimWorkHandler(struct k_work *work)
{
	ARG_UNUSED(work);

	const int totalMs = sCfg.effectTimeoutS * 1000;
	const int numPairs = BlinkPairCount();
	const int fx = sCfg.fxTickMs;
	const int pairMs = MAX(2 * fx, totalMs / numPairs);
	const int halfMs = MAX(fx, pairMs / 2);

	const int e = MIN(sElapsedMs, totalMs - 1);
	const int pos = (pairMs > 0) ? (e % pairMs) : 0;
	uint8_t level;

	if (pos < halfMs) {
		level = static_cast<uint8_t>(
			(static_cast<uint32_t>(kPwmMaxLevel) * (halfMs - pos)) /
			static_cast<unsigned>(halfMs));
	} else {
		const int up = pairMs - halfMs;

		if (up < 1) {
			level = kPwmMaxLevel;
		} else {
			const int pos2 = pos - halfMs;

			level = static_cast<uint8_t>((static_cast<uint32_t>(kPwmMaxLevel) *
						      static_cast<unsigned>(pos2)) /
						     static_cast<unsigned>(up));
		}
	}

	for (unsigned i = 0; i < ARRAY_SIZE(sPwmDevices); i++) {
		if (!sPwmChannelOk[i]) {
			continue;
		}
		uint8_t value = level;

		(void)sPwmDevices[i].InitiateAction(Nrf::PWMDevice::LEVEL_ACTION, 0, &value);
	}

	sElapsedMs += fx;
	if (sElapsedMs < totalMs) {
		(void)k_work_reschedule(&sDimWork, K_MSEC(fx));
	} else {
		SuppressAllPwmOutputs();
		Nrf::GetBoard().RunLedStateHandler();
		if (sOnComplete != nullptr) {
			sOnComplete(sOnCompleteCtx);
		}
	}
}

void EffectTimerCallback(k_timer *timer)
{
	ARG_UNUSED(timer);
	Nrf::PostTask([] { Start(); });
}

#endif /* CONFIG_PWM && pwm_led aliases */

} // namespace

bool IsAvailable()
{
#if defined(CONFIG_PWM) && DT_HAS_ALIAS(pwm_led0) && DT_HAS_ALIAS(pwm_led1) &&                     \
	DT_HAS_ALIAS(pwm_led2) && DT_HAS_ALIAS(pwm_led3)
	return true;
#else
	return false;
#endif
}

int Init(const Config &cfg, OnComplete onComplete, void *onCompleteCtx)
{
#if defined(CONFIG_PWM) && DT_HAS_ALIAS(pwm_led0) && DT_HAS_ALIAS(pwm_led1) &&                     \
	DT_HAS_ALIAS(pwm_led2) && DT_HAS_ALIAS(pwm_led3)
	sCfg = cfg;
	sOnComplete = onComplete;
	sOnCompleteCtx = onCompleteCtx;

	k_timer_init(&sEffectTimer, EffectTimerCallback, nullptr);
	k_work_init_delayable(&sDimWork, DimWorkHandler);
	return 0;
#else
	ARG_UNUSED(cfg);
	ARG_UNUSED(onComplete);
	ARG_UNUSED(onCompleteCtx);
	return 0;
#endif
}

void Start()
{
#if defined(CONFIG_PWM) && DT_HAS_ALIAS(pwm_led0) && DT_HAS_ALIAS(pwm_led1) &&                     \
	DT_HAS_ALIAS(pwm_led2) && DT_HAS_ALIAS(pwm_led3)

	(void)k_work_cancel_delayable(&sDimWork);

	sElapsedMs = 0;

	Nrf::GetBoard().ForEachLED([](Nrf::LEDWidget &led) { led.Set(false); });

	if (InitPwmDevicesAndTurnOn() != 0) {
		LOG_ERR("Dimming effect: no PWM channel initialized");
		Nrf::GetBoard().RunLedStateHandler();
		if (sOnComplete != nullptr) {
			sOnComplete(sOnCompleteCtx);
		}
		return;
	}

	(void)k_work_schedule(&sDimWork, K_NO_WAIT);
#endif
}

void Stop()
{
#if defined(CONFIG_PWM) && DT_HAS_ALIAS(pwm_led0) && DT_HAS_ALIAS(pwm_led1) &&                     \
	DT_HAS_ALIAS(pwm_led2) && DT_HAS_ALIAS(pwm_led3)
	k_timer_stop(&sEffectTimer);
	(void)k_work_cancel_delayable(&sDimWork);
	SuppressAllPwmOutputs();
	Nrf::GetBoard().RunLedStateHandler();
#endif
}

void RequestStartViaTimer()
{
#if defined(CONFIG_PWM) && DT_HAS_ALIAS(pwm_led0) && DT_HAS_ALIAS(pwm_led1) &&                     \
	DT_HAS_ALIAS(pwm_led2) && DT_HAS_ALIAS(pwm_led3)
	k_timer_stop(&sEffectTimer);
	k_timer_start(&sEffectTimer, K_NO_WAIT, K_NO_WAIT);
#endif
}

} // namespace DimmingEffect
} // namespace Nrf
