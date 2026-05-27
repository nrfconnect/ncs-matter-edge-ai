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

#include <haly/nrfy_pdm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(edgeai_ww_kw, CONFIG_MATTER_EDGEAI_WW_KW_LOG_LEVEL);

using namespace chip::DeviceLayer;

void EdgeAIWwKwHandleKeyword(uint16_t keyword_class)
{
    switch (keyword_class)
    {
    case KEYWORD_TOGGLE_LIGHT:
        SystemLayer().ScheduleLambda([] {
            LOG_INF("Toggling the light");
            auto position = AppTask::Instance().GetSwitchAll().GetPosition();
            position      = (position == Nrf::Matter::GenericSwitch::Position::On) ? Nrf::Matter::GenericSwitch::Position::Off
                                                                                   : Nrf::Matter::GenericSwitch::Position::On;
            if (position == Nrf::Matter::GenericSwitch::Position::On)
            {
                (void) AppTask::Instance().GetSwitchAll().ShortPress();
            }
            else
            {
                (void) AppTask::Instance().GetSwitchAll().LongPress();
            }
        });
        break;
    case KEYWORD_SCENE_ONE:
        SystemLayer().ScheduleLambda([] {
            LOG_INF("On/Off switch (endpoint 2)");
            (void) AppTask::Instance().GetSwitchScene1().ShortPress();
        });
        break;
    case KEYWORD_SCENE_TWO:
        SystemLayer().ScheduleLambda([] {
            LOG_INF("Scene 2");
            (void) AppTask::Instance().GetSwitchScene2().ShortPress();
        });
        break;
    case KEYWORD_SCENE_THREE:
        SystemLayer().ScheduleLambda([] {
            LOG_INF("Scene 3");
            (void) AppTask::Instance().GetSwitchScene3().ShortPress();
        });
        break;
    case KEYWORD_SCENE_FOUR:
        SystemLayer().ScheduleLambda([] {
            LOG_INF("Scene 4");
            (void) AppTask::Instance().GetSwitchScene4().ShortPress();
        });
        break;
    default:
        LOG_DBG("Not a valid keyword (class %u)", keyword_class);
        break;
    }
}

void EdgeAIWwKwOnWakewordDetected(void)
{
    if (Nrf::DimmingEffect::IsAvailable())
    {
        Nrf::DimmingEffect::RequestStartViaTimer();
    }
}

void EdgeAIWwKwOnKeywordSessionEnd(void)
{
    Nrf::DimmingEffect::Stop();
}

bool EdgeAIWwKwAppInit(void)
{
    if (Nrf::DimmingEffect::IsAvailable())
    {
        Nrf::DimmingEffect::Config dim_cfg;

        dim_cfg.effect_timeout_s       = CONFIG_KEYWORD_DETECTION_TIMEOUT_S;
        dim_cfg.blink_pairs_multiplier = 3;
        if (Nrf::DimmingEffect::Init(dim_cfg) != 0)
        {
            return false;
        }
    }

    nrfy_pdm_gain_set(NRF_PDM20, 0x40, 0x40);

    return true;
}
