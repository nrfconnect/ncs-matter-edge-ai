/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stddef.h>

#include <haly/nrfy_pdm.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "dmic.h"

LOG_MODULE_REGISTER(dmic);

#define DMIC_NODE DT_NODELABEL(dmic_dev)
#define BLOCK_SIZE (DMIC_SAMPLE_BYTES * DMIC_PCM_RATE * SAMPLES_BLOCK_LENGTH_MS / 1000)

K_MEM_SLAB_DEFINE_STATIC(dmicMemSlab, BLOCK_SIZE, CONFIG_MATTER_EDGEAI_DMIC_MEM_SLAB_NUM_BLOCKS,
			 16);

static const struct device *const kdmicDev = DEVICE_DT_GET(DMIC_NODE);
static NRF_PDM_Type *const kPdmReg = (NRF_PDM_Type *)DT_REG_ADDR(DMIC_NODE);

int DmicInit(void)
{
	int err;

	if (!device_is_ready(kdmicDev)) {
		LOG_ERR("Device is not ready");
		return -ENODEV;
	}

	struct pcm_stream_cfg stream = {
		.pcm_rate = DMIC_PCM_RATE,
		.pcm_width = DMIC_SAMPLE_BYTES * 8,
		.block_size = BLOCK_SIZE,
		.mem_slab = &dmicMemSlab,
	};
	struct dmic_cfg cfg = {
		.io =
			{
				.min_pdm_clk_freq = 3200000,
				.max_pdm_clk_freq = 3700000,
				.min_pdm_clk_dc = 40,
				.max_pdm_clk_dc = 60,
			},
		.streams = &stream,
		.channel =
			{
				.req_chan_map_lo = dmic_build_channel_map(0, 0, PDM_CHAN_LEFT),
				.req_chan_map_hi = 0,
				.req_num_chan = 1,
				.req_num_streams = 1,
			},
	};

	err = dmic_configure(kdmicDev, &cfg);
	if (err < 0) {
		LOG_ERR("Failed to configure (err %d)", err);
		return err;
	}

	nrfy_pdm_gain_set(kPdmReg, CONFIG_MATTER_EDGEAI_DMIC_GAIN, CONFIG_MATTER_EDGEAI_DMIC_GAIN);

	return 0;
}

void FreeDmicBuffer(void *buffer)
{
	k_mem_slab_free(&dmicMemSlab, buffer);
}

const struct device *GetDmicDevice(void)
{
	return kdmicDev;
}

int DmicTriggerStart(void)
{
	return dmic_trigger(kdmicDev, DMIC_TRIGGER_START);
}

int DmicRead(void **buffer, size_t *buffer_size, int32_t timeout_ms)
{
	return dmic_read(kdmicDev, 0, buffer, buffer_size, timeout_ms);
}
