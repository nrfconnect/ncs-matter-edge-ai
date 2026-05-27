/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "app_task.h"

#include "app/matter_init.h"
#include "app/task_executor.h"
#include "board/board.h"
#include "clusters/identify.h"
#include "lib/core/CHIPError.h"

#include "descriptor-namespaces.h"
#include "nrf_edgeai_ww_kw_task.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::OnOff;
using namespace ::chip::DeviceLayer;

K_SEM_DEFINE(gMatterStartedSem, 0, 1);

#define APPLICATION_BUTTON_MASK DK_BTN2_MSK

namespace
{

using SemanticTag = Globals::Structs::SemanticTagStruct::Type;

const SemanticTag gEp0TagList[] = {
	{.namespaceID = kNamespaceSwitches,
	 .tag = kTagSwitchesUp,
	 .label = chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
		 {chip::app::DataModel::MakeNullable(chip::CharSpan("All", 3))})}};

const SemanticTag gEp1TagList[] = {{.namespaceID = kNamespaceSwitches,
				    .tag = kTagSwitchesUp,
				    .label = Optional<DataModel::Nullable<CharSpan>>(
					    {DataModel::MakeNullable(CharSpan("Scene 1", 8))})}};

const SemanticTag gEp2TagList[] = {{.namespaceID = kNamespaceSwitches,
				    .tag = kTagSwitchesUp,
				    .label = Optional<DataModel::Nullable<CharSpan>>(
					    {DataModel::MakeNullable(CharSpan("Scene 2", 8))})}};

const SemanticTag gEp3TagList[] = {{.namespaceID = kNamespaceSwitches,
				    .tag = kTagSwitchesUp,
				    .label = Optional<DataModel::Nullable<CharSpan>>(
					    {DataModel::MakeNullable(CharSpan("Scene 3", 8))})}};

const SemanticTag gEp4TagList[] = {{.namespaceID = kNamespaceSwitches,
				    .tag = kTagSwitchesUp,
				    .label = Optional<DataModel::Nullable<CharSpan>>(
					    {DataModel::MakeNullable(CharSpan("Scene 4", 8))})}};

Nrf::Matter::IdentifyCluster sIdentifyCluster(AppTask::Instance().GetSwitchAll().GetEndpointId());
Nrf::Matter::IdentifyCluster
	sIdentifyClusterScene1(AppTask::Instance().GetSwitchScene1().GetEndpointId());
Nrf::Matter::IdentifyCluster
	sIdentifyClusterScene2(AppTask::Instance().GetSwitchScene2().GetEndpointId());
Nrf::Matter::IdentifyCluster
	sIdentifyClusterScene3(AppTask::Instance().GetSwitchScene3().GetEndpointId());
Nrf::Matter::IdentifyCluster
	sIdentifyClusterScene4(AppTask::Instance().GetSwitchScene4().GetEndpointId());

void ButtonEventHandler(Nrf::ButtonState state, Nrf::ButtonMask hasChanged)
{
	if ((APPLICATION_BUTTON_MASK & state & hasChanged)) {
		auto position = AppTask::Instance().GetSwitchAll().GetPosition();
		position = (position == Nrf::Matter::GenericSwitch::Position::On)
				   ? Nrf::Matter::GenericSwitch::Position::Off
				   : Nrf::Matter::GenericSwitch::Position::On;

		if (position == Nrf::Matter::GenericSwitch::Position::On) {
			(void)AppTask::Instance().GetSwitchAll().ShortPress();
		} else {
			(void)AppTask::Instance().GetSwitchAll().LongPress();
		}
	}
}

void MatterAppEventHandler(const chip::DeviceLayer::ChipDeviceEvent *event, intptr_t arg)
{
	ARG_UNUSED(arg);

	switch (event->Type) {
	case chip::DeviceLayer::DeviceEventType::kServerReady:
		EdgeAITask::Instance().Enable();
		k_sem_give(&gMatterStartedSem);
		break;
	case chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished:
		EdgeAITask::Instance().Disable();
		break;
	default:
		break;
	}
}

} // namespace

AppTask::AppTask()
	: mGenericSwitch_All(1, gEp0TagList, 1), mGenericSwitch_Scene1(2, gEp1TagList, 1),
	  mGenericSwitch_Scene2(3, gEp2TagList, 1), mGenericSwitch_Scene3(4, gEp3TagList, 1),
	  mGenericSwitch_Scene4(5, gEp4TagList, 1)
{
}

CHIP_ERROR AppTask::InitSwitches()
{
	ReturnErrorOnFailure(mGenericSwitch_All.Init());
	ReturnErrorOnFailure(mGenericSwitch_Scene1.Init());
	ReturnErrorOnFailure(mGenericSwitch_Scene2.Init());
	ReturnErrorOnFailure(mGenericSwitch_Scene3.Init());
	ReturnErrorOnFailure(mGenericSwitch_Scene4.Init());

	return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::Init()
{
	ReturnErrorOnFailure(
		Nrf::Matter::PrepareServer(Nrf::Matter::InitData{.mPostServerInitClbk = [] {
			ReturnErrorOnFailure(AppTask::Instance().InitSwitches());
			return CHIP_NO_ERROR;
		}}));

	if (!Nrf::GetBoard().Init(ButtonEventHandler)) {
		LOG_ERR("User interface initialization failed.");
		return CHIP_ERROR_INCORRECT_STATE;
	}

	ReturnErrorOnFailure(Nrf::Matter::RegisterEventHandler(MatterAppEventHandler, 0));
	ReturnErrorOnFailure(
		Nrf::Matter::RegisterEventHandler(Nrf::Board::DefaultMatterEventHandler, 0));

	ReturnErrorOnFailure(sIdentifyCluster.Init());
	ReturnErrorOnFailure(sIdentifyClusterScene1.Init());
	ReturnErrorOnFailure(sIdentifyClusterScene2.Init());
	ReturnErrorOnFailure(sIdentifyClusterScene3.Init());
	ReturnErrorOnFailure(sIdentifyClusterScene4.Init());

	return Nrf::Matter::StartServer();
}

CHIP_ERROR AppTask::StartApp()
{
	ReturnErrorOnFailure(Init());

	if (!EdgeAITask::Instance().Start()) {
		return CHIP_ERROR_INTERNAL;
	}

	while (true) {
		Nrf::DispatchNextTask();
	}

	return CHIP_NO_ERROR;
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath &attributePath,
				       uint8_t type, uint16_t size, uint8_t *value)
{
	ClusterId pathClusterId = attributePath.mClusterId;
	AttributeId pathAttributeId = attributePath.mAttributeId;

	if (pathClusterId == OnOff::Id && pathAttributeId == OnOff::Attributes::OnOff::Id) {
		if (!value) {
			LOG_ERR("Invalid value for OnOff attribute");
			return;
		}

		const bool valueInvert = !*value;

		LOG_INF("Cluster OnOff: attribute OnOff set to %" PRIu8 "", valueInvert);

		Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).Set(valueInvert);
	}
}
