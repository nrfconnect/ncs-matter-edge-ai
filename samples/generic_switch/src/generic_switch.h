/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace Nrf {
namespace Matter {

class GenericSwitch
{
public:
    using SemanticTag = chip::app::Clusters::Globals::Structs::SemanticTagStruct::Type;

    enum class Position : uint8_t
    {
        On = 0,
        Off = 1,
    };

    GenericSwitch(chip::EndpointId endpointId, const SemanticTag * tagList = nullptr, size_t tagListSize = 0) :
        mEndpointId(endpointId), mTagList(tagList), mTagListSize(tagListSize)
    {}

    CHIP_ERROR Init();
    CHIP_ERROR Latch();
    CHIP_ERROR ShortPress();
    CHIP_ERROR LongPress();
    CHIP_ERROR MultiplePress(uint8_t count);

    Position GetPosition() const { return mPosition; }
    chip::EndpointId GetEndpointId() const { return mEndpointId; }

private:
    bool HasFeature(chip::app::Clusters::Switch::Feature feature) const;

    chip::EndpointId mEndpointId;
    const SemanticTag * mTagList;
    size_t mTagListSize;
    bool mLatchState = false;
    Position mPosition = Position::Off;
};

} // namespace Matter
} // namespace Nrf
