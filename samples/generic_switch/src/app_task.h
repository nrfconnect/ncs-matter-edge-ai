/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "generic_switch.h"

#include <lib/core/CHIPError.h>

class AppTask
{
public:
	static AppTask &Instance()
	{
		static AppTask sAppTask;
		return sAppTask;
	}

	CHIP_ERROR StartApp();

	Nrf::Matter::GenericSwitch &GetSwitchAll()
	{
		return mGenericSwitch_All;
	}
	Nrf::Matter::GenericSwitch &GetSwitchScene1()
	{
		return mGenericSwitch_Scene1;
	}
	Nrf::Matter::GenericSwitch &GetSwitchScene2()
	{
		return mGenericSwitch_Scene2;
	}
	Nrf::Matter::GenericSwitch &GetSwitchScene3()
	{
		return mGenericSwitch_Scene3;
	}
	Nrf::Matter::GenericSwitch &GetSwitchScene4()
	{
		return mGenericSwitch_Scene4;
	}

private:
	AppTask();

	CHIP_ERROR Init();
	CHIP_ERROR InitSwitches();

	Nrf::Matter::GenericSwitch mGenericSwitch_All;
	Nrf::Matter::GenericSwitch mGenericSwitch_Scene1;
	Nrf::Matter::GenericSwitch mGenericSwitch_Scene2;
	Nrf::Matter::GenericSwitch mGenericSwitch_Scene3;
	Nrf::Matter::GenericSwitch mGenericSwitch_Scene4;
};
