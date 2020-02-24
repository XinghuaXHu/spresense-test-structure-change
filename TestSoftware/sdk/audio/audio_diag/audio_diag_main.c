/****************************************************************************
 * test/audio_diag/audio_diag_main.c
 *
 *   Copyright (C) 2017 Sony Corporation. All rights reserved.
 *   Author: Naoya Haneda<Naoya.Haneda@sony.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor Sony nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

/* TODO: Remake diag */

#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <arch/board/board.h>
#include <arch/chip/cxd56_audio.h>

//#define DMA_TEST_ENABLE
#ifdef DMA_TEST_ENABLE
extern void baseband_test(void);
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define AUDIO_BASEBAND_DEV_NAME  "/dev/audio/baseband"

#define HELP(str)     {int i; for(i=0;i<sizeof(str)/sizeof(str[0]);i++) printf("%s\n",str[i]);printf("\n");}
#define EXIT_SETPATH  {HELP(help_setpath); ret=-1; goto _exit_diag;}
#define EXIT_SETVOL   {HELP(help_setvol);  ret=-1; goto _exit_diag;}
#define EXIT_SETBEEP  {HELP(help_setbeep); ret=-1; goto _exit_diag;}
#define EXIT_SETMIC   {HELP(help_setmic);  ret=-1; goto _exit_diag;}
#define EXIT_SETGAIN  {HELP(help_setgain); ret=-1; goto _exit_diag;}
#define EXIT_HELP     {HELP(help_help);    ret=-1; goto _exit_diag;}

/****************************************************************************
 * Private Data
 ****************************************************************************/
const char help_setpath[][64] = {
	"[data path setting]",
	"\t> adiag setpath {0|1|2|3}",
	"\t\t0: speaker only (default)",
	"\t\t1: mic in -> speaker out",
	"\t\t2: i2s in -> i2s out",
	"\t\t3: mic in -> i2s out, i2s in -> speaker out"
};
const char help_setvol[][64] = {
	"[volume setting]",
	"\t> adiag setvol {volume}",
	"\t\tvolume : -102 to -2 [dB]"
};
const char help_setbeep[][64] = {
	"[beep setting]",
	"\t> adiag beep on {volume} {frequency}",
	"\t\tvolume   : -90 to -10 [dB] (option)",
	"\t\tfreqency : 94 to 4085 [Hz] (option)",
	"\t> adiag beep off"
};
const char help_setmic[][64] = {
	"[mic select]",
	"\t> adiag setmic {Lch} {Rch}",
	"\t\tanalog mic  : A to D",
	"\t\tdigital mic : 1 to 8"
};
const char help_setgain[][64] = {
	"[mic gain setting]",
	"\t> adiag setgain {gain-L} {gain-R}",
	"\t\tanalog gain : 0 to 21 [dB]",
};
const char help_help[][64] = {
	"[help]",
	"\t> adiag help"
};

static int fd = -1;

/****************************************************************************
 * Audio Driver User Functions
 ****************************************************************************/
void AS_AudioIntHandler( void )
{

	uint32_t int_irq = *(volatile uint32_t *)(AS_INT_IRQ1_REG);

	if (int_irq & (1 << AS_INT_IRQ1_BIT_AU3)) {
		uint32_t int_au  = read32_bca_reg( bcaRegMap[BCA_Int_hresp_err].addr );
		if( int_au != 0 ){
			write32_bca_reg( bcaRegMap[BCA_Int_clr_hresp_err].addr, int_au );
		  /* dma error occurred */
		}
	}
  return;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
void help(void)
{
	HELP(help_setpath);
	HELP(help_setvol);
	HELP(help_setbeep);
	HELP(help_setmic);
	HELP(help_setgain);
	HELP(help_help);
}

void power_on_baseband(void)
{
	int err;

	uint32_t mode = AUDIO_CXD5247|AUDIO_CXD56xx;
	err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_POWER|AUDIO_IOCTL_TYPE_ENABLE, (unsigned long)mode);
	if(err != 0){
		printf("ERROR: Audio Power ON, err(%d)\n", err);
	}

	_info("*** AS_PowerOnBaseBand() ***\n");
	uint32_t rate[2] = {48000};
	asBypassModeId bypass_mode[2] = {AS_I2S_BYPASS_MODE_DISABLE};

	struct audio_bb_power_param_s power_param;
	power_param.rate = rate;
	power_param.bypass_mode = bypass_mode;
	err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_COMMON|AUDIO_IOCTL_TYPE_ENABLE, (unsigned long)&power_param);
	if(err != 0){
		printf("ERROR: AS_PowerOnBaseBand() err(%d)\n", err);
	}

	_info("*** AS_BaseBandEnable_input() ***\n");
	int32_t mic_gain[8] = {0};

	struct audio_bb_input_param_s input_param;
	input_param.micMode = AS_MICMODE_ACAPULCO;
	input_param.micGain = mic_gain;
	err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_INPUT|AUDIO_IOCTL_TYPE_ENABLE, (unsigned long)&input_param);
	if(err != 0){
		printf("ERROR: AS_BaseBandEnable_input() err(%d)\n", err);
	}

	_info("*** AS_BaseBandEnable_output() ***\n");
	err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_OUTPUT|AUDIO_IOCTL_TYPE_ENABLE, (unsigned long)AS_OUT_SP);
	if(err != 0){
		printf("ERROR: AS_BaseBandEnable_output() err(%d)\n", err);
	}
}

void power_off_baseband(void)
{
	E_AS err;

	_info("*** AS_BaseBandDisable_input() ***\n");
	err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_INPUT|AUDIO_IOCTL_TYPE_DISABLE, (unsigned long)AS_MICMODE_ACAPULCO);
	if(err != 0){
		printf("ERROR: AS_BaseBandDisable_input() err(%d)\n", err);
	}

	_info("*** AS_BaseBandDisable_output() ***\n");
	err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_OUTPUT|AUDIO_IOCTL_TYPE_DISABLE, (unsigned long)NULL);
	if(err != 0){
		printf("ERROR: AS_BaseBandDisable_output() err(%d)\n", err);
	}

	_info("*** AS_PowerOffBaseBand() ***\n");
	err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_COMMON|AUDIO_IOCTL_TYPE_DISABLE, (unsigned long)NULL);
	if(err != 0){
		printf("ERROR: AS_PowerOffBaseBand() err(%d)\n", err);
	}

	uint32_t mode = AUDIO_CXD5247|AUDIO_CXD56xx;
	err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_POWER|AUDIO_IOCTL_TYPE_DISABLE, (unsigned long)mode);
	if(err != 0){
		printf("ERROR: Audio Power OFF, err(%d)\n", err);
	}
}

void set_power_status(int mode)
{
	static bool power_on = false;
	if(mode == 1){
		if(!power_on){
			power_on_baseband();
			power_on = true;
		}
	}
	else if(mode == 0){
		if(power_on){
			power_off_baseband();
			power_on = false;
		}
	}
	else {
		printf("ERROR: %s(%d)\n", __func__, mode);
	}
}

void clear_data_path(void)
{
	_info("*** AS_ClearAudioDataPathAll() ***\n");
	ioctl(fd, CXD56_AUDIO_IOCTL_BB_DATAPATH|AUDIO_IOCTL_TYPE_RESET, (unsigned long)NULL);
}

int set_data_path(asPathFromId from, asPathToId to)
{
	_info("*** AS_SetAudioDataPath(%d,%d) ***\n", from, to);
	asDmacSelId dmac_id;
	asPathSelParam path_sel_param;
	path_sel_param.pathFrom = from;
	path_sel_param.pathTo = to;
	path_sel_param.mic_dma_channel = 0;

	struct audio_bb_datapath_param_s param;
	param.pPathSelParam = &path_sel_param;
	param.getDmacId = &dmac_id;
	param.setDmacId = AS_DMAC_ID_NONE;
	int err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_DATAPATH|AUDIO_IOCTL_TYPE_SET, (unsigned long)&param);
	if(err != 0){
		printf("ERROR: AS_SetAudioDataPath() err(%d)\n", err);
		return -1;
	}
	return 0;
}

int set_beep(int mode, int vol, int freq)
{
	set_power_status(1);
	if(mode == 1){
		_info("*** AS_SetBeepParam(%d,%d) ***\n", freq, vol);
		struct audio_bb_beep_param_s param;
		param.beepFreq = freq;
		param.beepVol = vol;
		int err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_BEEP|AUDIO_IOCTL_TYPE_SET, (unsigned long)&param);
		if(err != 0){
			printf("ERROR: AS_SetBeepParam() err(%d)\n", err);
			return -1;
		}
		ioctl(fd, CXD56_AUDIO_IOCTL_BB_BEEP|AUDIO_IOCTL_TYPE_ENABLE, (unsigned long)NULL);
	}
	else if(mode == 0){
		ioctl(fd, CXD56_AUDIO_IOCTL_BB_BEEP|AUDIO_IOCTL_TYPE_DISABLE, (unsigned long)NULL);
	}
	else {
		return -1;
	}
	return 0;
}

int set_mute(int mode)
{
	if(mode == 1){
		_info("*** AS_MuteVolume() ***\n");
		ioctl(fd, CXD56_AUDIO_IOCTL_BB_VOLUME|AUDIO_IOCTL_TYPE_MUTE, (unsigned long)AS_VOLUME_MASTER);
	}
	else if(mode == 0){
		_info("*** AS_UnMuteVolume() ***\n");
		ioctl(fd, CXD56_AUDIO_IOCTL_BB_VOLUME|AUDIO_IOCTL_TYPE_UNMUTE, (unsigned long)AS_VOLUME_MASTER);
	}
	else {
		return -1;
	}
	return 0;
}

int set_volume(int input1, int master)
{
	if(-2 < master){
		return -1;
	}
	set_power_status(1);
	_info("*** AS_SetVolume(%d) ***\n", master);
	asCodecVol codec_vol;
	codec_vol.input1_db = (input1==AS_VOLUME_MUTE)? AS_VOLUME_MUTE:(input1 * 10);
	codec_vol.input2_db = AS_VOLUME_MUTE;
	codec_vol.master_db = (master==AS_VOLUME_MUTE)? AS_VOLUME_MUTE:(master * 10);
	int err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_VOLUME|AUDIO_IOCTL_TYPE_SET, (unsigned long)&codec_vol);
	if(err != 0){
		printf("ERROR: AS_SetVolume() err(%d)\n", err);
		return -1;
	}
	return 0;
}

int set_path(int mode)
{
    int ret = 0;

	set_power_status(1);
	set_mute(1);
	clear_data_path();

	switch(mode){
	case 0: /* sp only */
		break;
	case 1: /* mic loopback */
		set_data_path(AS_PATH_FROM_MIC12, AS_PATH_TO_MIXER1);
		set_mute(0);
		break;
	case 2: /* i2s loopback */
		set_data_path(AS_PATH_FROM_I2S1, AS_PATH_TO_MIXER1);
		set_data_path(AS_PATH_FROM_MIXER, AS_PATH_TO_I2S1);
		set_volume(0, AS_VOLUME_MUTE);
		break;
	case 3: /* bb through */
		set_data_path(AS_PATH_FROM_MIC12, AS_PATH_TO_I2S1);
		set_data_path(AS_PATH_FROM_I2S1, AS_PATH_TO_MIXER1);
		set_mute(0);
		break;
#ifdef DMA_TEST_ENABLE
	case 4: /* not supported */
		baseband_test();
		break;
#endif
	default:
		return -1;
	}

    ret = board_external_amp_mute_control(false);
    if (ret)
      {
        printf("ERROR: Amp mute off error. %d\n", ret);
        return false;
      }

	return 0;
}

int mic_ch_val(char *str)
{
	switch(str[0]){
	case 'A': return 1;
	case 'B': return 2;
	case 'C': return 3;
	case 'D': return 4;
	case '1': return 5;
	case '2': return 6;
	case '3': return 7;
	case '4': return 8;
	case '5': return 9;
	case '6': return 10;
	case '7': return 11;
	case '8': return 12;
	default:
		printf("ERROR: mic channel err(%c)\n", str[0]);
		return -1;
	}
}

int set_mic(char *ch0, char *ch1)
{
	set_power_status(1);
	_info("*** asAca_SetSerDes(%s,%s) ***\n", ch0, ch1);
	asSerDesParam sdesParam;
	sdesParam.serMode     = AS_ACA_SER_MODE_8CH;
	sdesParam.serFs       = AS_ACA_SER_FS_64;
	sdesParam.selCh.in[0] = mic_ch_val(ch0);
	sdesParam.selCh.in[1] = mic_ch_val(ch1);
	sdesParam.selCh.in[2] = AS_SDES_DES_SEL_UNKNOWN;
	sdesParam.selCh.in[3] = AS_SDES_DES_SEL_UNKNOWN;
	sdesParam.selCh.in[4] = AS_SDES_DES_SEL_UNKNOWN;
	sdesParam.selCh.in[5] = AS_SDES_DES_SEL_UNKNOWN;
	sdesParam.selCh.in[6] = AS_SDES_DES_SEL_UNKNOWN;
	sdesParam.selCh.in[7] = AS_SDES_DES_SEL_UNKNOWN;
	E_AS err = AS_AcaControl(AS_ACA_SET_SERDES, (uint32_t)&sdesParam);
	if(err != E_AS_OK){
		printf("ERROR: asAca_SetSerDes() err(%d)\n", err);
		return -1;
	}
	return 0;
}

int set_gain(int gain_l, int gain_r)
{
	set_power_status(1);
	_info("*** AS_SetMicGain(%d,%d) ***\n", gain_l, gain_r);
	int gain[AS_MIC_CHANNEL_MAX] = {AS_MICGAIN_HOLD};
	gain[0] = gain_l * 10;
	gain[1] = gain_r * 10;
	int err = ioctl(fd, CXD56_AUDIO_IOCTL_BB_MICGAIN|AUDIO_IOCTL_TYPE_SET, (unsigned long)gain);
	if(err != 0){
		printf("ERROR: AS_SetMicGain() err(%d)\n", err);
		return -1;
	}
	return 0;
}

/****************************************************************************
 * audio_diag_main
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int adiag_main(int argc, char *argv[])
#endif
{
	int ret = 0;

	cxd56_audio_bb_register(AUDIO_BASEBAND_DEV_NAME);
	fd = open(AUDIO_BASEBAND_DEV_NAME, O_RDWR);
	if (fd < 0)
		{
		  printf("ERROR: failed open driver of audio baseband\n");
		  cxd56_audio_bb_unregister(AUDIO_BASEBAND_DEV_NAME);
		  return -1;
		}

	if(argc > 1){
		if(strncmp(*(argv+1), "beep", 4) == 0){
			int mode = 0;
			int vol  = -40;
			int freq = AS_BEEP_FREQ_HOLD;
			if(argc > 2){
				if(strncmp(*(argv+2), "on", 2) == 0){
					mode = 1;
				}
				else if(strncmp(*(argv+2), "off", 2) == 0){
					mode = 0;
				}
				else {
					EXIT_SETBEEP;
				}
			}
			else {
				EXIT_SETBEEP;
			}
			if(argc > 3){
				vol = atoi(*(argv+3));
			}
			if(argc > 4){
				freq = atoi(*(argv+4));
			}
			if(set_beep(mode, vol, freq) < 0){
				EXIT_SETBEEP;
			}
		}
		else if(strncmp(*(argv+1), "setpath", 7) == 0){
			if(argc > 2){
				int mode = atoi(*(argv+2));
				if(set_path(mode) < 0){
					EXIT_SETPATH;
				}
			}
			else {
				EXIT_SETPATH;
			}
		}
		else if(strncmp(*(argv+1), "setvol", 6) == 0){
			if(argc > 2){
				int vol = atoi(*(argv+2));
				if(set_volume(0, vol) < 0){
					EXIT_SETVOL;
				}
			}
			else {
				EXIT_SETVOL;
			}
		}
		else if(strncmp(*(argv+1), "setmic", 6) == 0){
			if(argc > 3){
				char mic_l[4], mic_r[4];
				strncpy(mic_l, *(argv+2), 2);
				strncpy(mic_r, *(argv+3), 2);
				if(set_mic(mic_l, mic_r) < 0){
					EXIT_SETMIC;
				}
			}
			else {
				EXIT_SETMIC;
			}
		}
		else if(strncmp(*(argv+1), "setgain", 7) == 0){
			if(argc > 3){
				int gain_l = atoi(*(argv+2));
				int gain_r = atoi(*(argv+3));
				if(set_gain(gain_l, gain_r) < 0){
					EXIT_SETGAIN;
				}
			}
			else {
				EXIT_SETGAIN;
			}
		}
		else if(strncmp(*(argv+1), "help", 4) == 0){
			help();
		}
		else {
			printf("ERROR: unknown command(%s)\n", *(argv+1));
			EXIT_HELP;
		}
	}
	else {
		help();
	}

_exit_diag:
	close(fd);
	cxd56_audio_bb_unregister(AUDIO_BASEBAND_DEV_NAME);

	return ret;
}
