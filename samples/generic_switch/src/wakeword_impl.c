/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "wakeword.h"

#include "models/wakeword/nrf_edgeai_generated/nrf_edgeai_user_model.h"

nrf_edgeai_t *WakewordAppModelGet(void)
{
	return nrf_edgeai_user_model_ww();
}
