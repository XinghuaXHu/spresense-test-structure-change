#include "stub/audio_io_config.h"

int setAudioIoMclk_Called = 0;
void setAudioIoMclk(void)
{
	setAudioIoMclk_Called++; // �����Ȃ��A�߂�l�Ȃ��Ȃ̂ŁA�Ă΂ꂽ���Ƃ����L�^
}

