/*
 * Copyright 2015 Sony Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <stdint.h>
#include <customer_config.h>
#include <audio/high_level_api/as_high_level_api.h>

#if defined (SUPPORT_HIGHRES_AUDIO)
uint8_t clock = AS_AUDIO_BLOCK_CLOCK_49M;
#else
uint8_t clock = AS_AUDIO_BLOCK_CLOCK_24M;
#endif

BaseBandConfigTbl	bb_config_tbl = {
	0,							//mic_bias_sel
	0,							//reserved
#if defined (SUPPORT_HIGHRES_AUDIO)
	AS_CLK_MODE_HIRES,			//clk_mode
	AS_XTAL_49_152MHZ,			//xtal_sel
#else
	AS_CLK_MODE_NORMAL,			//clk_mode
	AS_XTAL_24_576MHZ,			//xtal_sel
#endif
	0xffff4321,					//mic_channel_sel

	AS_IO_DS_WEAKEST,			//gpo_ds
	AS_IO_DS_WEAKEST,			//ad_data_ds
	AS_IO_DS_WEAKEST,			//dmic_clk_ds
	AS_IO_DS_STRONGER,			//mclk_ds

	AS_I2S_DEVICE_I2S_SLAVE,	//i2s_device_1
	AS_I2S_FORMAT_I2S,			//i2s_format_1
	0,							//reserved
	AS_I2S_DATA_PATH_1,			//i2s_data_path

	AS_I2S_DEVICE_I2S_SLAVE,	//i2s_device_2
	AS_I2S_FORMAT_I2S,			//i2s_format_2
	1100,						//mic_boot_wait

	AS_LOWEMI_2MA,				//pdm_lowemi
	AS_LOWEMI_2MA,				//i2s_lowemi
	AS_CIC_IN_SEL_CXD,			//cic_input_sel
	0,							//reserved

	AS_ALC_SPC_SEL_OFF,			//alc_spc_sel
	0,							//reserved
	0,							//spc_limit

	0,							//alc_target
	-40,						//alc_knee

	0,							//reserved
	AS_DMA_DATA_FORMAT_RL,		//dma_data_format
	AS_HPADC_MIC_BIAS_OFF,		//hpadc_mic_bias
	0,							//reserved

	0,							//sp_delay
	0,							//loop_mode
	0,							//pwm_mode
	0							//reserved
};

BEGIN_DEFINE_CONFIG_SECTION(AUDIO)
DEFINE_CONFIG_KEY(AUDIO_CLOCK, clock)
DEFINE_CONFIG_KEY(BASEBAND_CONFIG, bb_config_tbl)
END_DEFINE_CONFIG_SECTION()
