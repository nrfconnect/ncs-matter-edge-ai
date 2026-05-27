/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "nrf_edgeai_ww_kw_task.h"

#include "app_task.h"
#include "dimming_effect.h"
#include "keyword_app.h"
#include "platform/CHIPDeviceLayer.h"
#include "platform/PlatformManager.h"

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(edgeai_ww_kw, CONFIG_MATTER_EDGEAI_WW_KW_LOG_LEVEL);

using namespace chip::DeviceLayer;

/**
 * Currently, commercial ecosystems support the Generic Switch only in momentary mode.
 * The switch recognizes three actions: Short Press, Long Press, and Multiple Press.
 * However, within the ecosystem app, only a single steering device state can be assigned to each
 * action. To enable toggling the device state, we map Short Press and Long Press to "on" and "off"
 * actions, respectively. As a result, the "Toggle" keyword can be handled by alternately triggering
 * either a Short Press or a Long Press.
 */

void EdgeAIWwKwHandleKeyword(uint16_t keywordClass)
{
	switch (keywordClass) {
	case KEYWORD_TOGGLE_LIGHT:
		SystemLayer().ScheduleLambda([] {
			LOG_DBG("Detected Toggle keyword");
			auto position = AppTask::Instance().GetSwitchAll().GetPosition();
			position = (position == Nrf::Matter::GenericSwitch::Position::On)
					   ? Nrf::Matter::GenericSwitch::Position::Off
					   : Nrf::Matter::GenericSwitch::Position::On;
			if (position == Nrf::Matter::GenericSwitch::Position::On) {
				(void)AppTask::Instance().GetSwitchAll().ShortPress();
			} else {
				(void)AppTask::Instance().GetSwitchAll().LongPress();
			}
		});
		break;
	case KEYWORD_SCENE_ONE:
		SystemLayer().ScheduleLambda([] {
			LOG_DBG("Detected Scene 1");
			(void)AppTask::Instance().GetSwitchScene1().ShortPress();
		});
		break;
	case KEYWORD_SCENE_TWO:
		SystemLayer().ScheduleLambda([] {
			LOG_DBG("Detected Scene 2");
			(void)AppTask::Instance().GetSwitchScene2().ShortPress();
		});
		break;
	case KEYWORD_SCENE_THREE:
		SystemLayer().ScheduleLambda([] {
			LOG_DBG("Detected Scene 3");
			(void)AppTask::Instance().GetSwitchScene3().ShortPress();
		});
		break;
	case KEYWORD_SCENE_FOUR:
		SystemLayer().ScheduleLambda([] {
			LOG_DBG("Detected Scene 4");
			(void)AppTask::Instance().GetSwitchScene4().ShortPress();
		});
		break;
	default:
		LOG_DBG("Not a valid keyword (class %u)", keywordClass);
		break;
	}
}

void EdgeAIWwKwOnWakewordDetected(void)
{
	if (Nrf::DimmingEffect::IsAvailable()) {
		Nrf::DimmingEffect::RequestStartViaTimer();
	}
}

void EdgeAIWwKwOnKeywordSessionEnd(void)
{
	Nrf::DimmingEffect::Stop();
}

bool EdgeAIWwKwAppInit(void)
{
	if (Nrf::DimmingEffect::IsAvailable()) {
		Nrf::DimmingEffect::Config dimCfg;

		dimCfg.effectTimeoutS = CONFIG_MATTER_EDGEAI_KEYWORD_DETECTION_TIMEOUT_S;
		dimCfg.blinkPairsMultiplier = 3;
		if (Nrf::DimmingEffect::Init(dimCfg) != 0) {
			return false;
		}
	}

	return true;
}
