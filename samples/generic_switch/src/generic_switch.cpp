/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "generic_switch.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/TypeTraits.h>

#include <zephyr/logging/log.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using chip::Protocols::InteractionModel::Status;

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {

CHIP_ERROR VerifyFeatureEnabled(bool enabled)
{
    return enabled ? CHIP_NO_ERROR : CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace

namespace Nrf {
namespace Matter {

bool GenericSwitch::HasFeature(Feature feature) const
{
    uint32_t featureMap = 0;

    if (Attributes::FeatureMap::Get(mEndpointId, &featureMap) != Status::Success)
    {
        return false;
    }

    return (featureMap & to_underlying(feature)) != 0;
}

CHIP_ERROR GenericSwitch::Init()
{
    if ((mTagList != nullptr) && (mTagListSize > 0))
    {
        ReturnErrorOnFailure(SetTagList(mEndpointId, Span<const SemanticTag>(mTagList, mTagListSize)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericSwitch::Latch()
{
    ReturnErrorOnFailure(VerifyFeatureEnabled(HasFeature(Feature::kLatchingSwitch)));

    mLatchState = !mLatchState;

    const uint8_t position = mLatchState ? 1 : 0;

    SwitchServer::Instance().OnSwitchLatch(mEndpointId, position);
    LOG_INF("Latch endpoint %u position %u", mEndpointId, position);

    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericSwitch::ShortPress()
{
    mPosition = Position::On;
    ReturnErrorOnFailure(VerifyFeatureEnabled(HasFeature(Feature::kMomentarySwitch)));

    SwitchServer::Instance().OnInitialPress(mEndpointId, static_cast<uint8_t>(mPosition));
    SwitchServer::Instance().OnShortRelease(mEndpointId, static_cast<uint8_t>(mPosition));
    LOG_INF("ShortPress endpoint %u position %u", mEndpointId, static_cast<uint8_t>(mPosition));

    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericSwitch::LongPress()
{
    mPosition = Position::Off;
    ReturnErrorOnFailure(VerifyFeatureEnabled(HasFeature(Feature::kMomentarySwitchLongPress)));

    SwitchServer::Instance().OnLongPress(mEndpointId, static_cast<uint8_t>(mPosition));
    SwitchServer::Instance().OnLongRelease(mEndpointId, static_cast<uint8_t>(mPosition));
    LOG_INF("LongRelease endpoint %u position %u", mEndpointId, static_cast<uint8_t>(mPosition));

    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericSwitch::MultiplePress(uint8_t count)
{
    ReturnErrorOnFailure(VerifyFeatureEnabled(HasFeature(Feature::kMomentarySwitchRelease)));

    SwitchServer::Instance().OnInitialPress(mEndpointId, static_cast<uint8_t>(mPosition));
    SwitchServer::Instance().OnMultiPressOngoing(mEndpointId, static_cast<uint8_t>(mPosition), count);
    LOG_INF("MultiplePress endpoint %u position %u count %u", mEndpointId, static_cast<uint8_t>(mPosition), count);

    return CHIP_NO_ERROR;
}
} // namespace Matter
} // namespace Nrf
