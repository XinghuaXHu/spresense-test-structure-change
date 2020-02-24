#include "stub/audio_io_config.h"

int setAudioIoMclk_Called = 0;
void setAudioIoMclk(void)
{
	setAudioIoMclk_Called++; // 引数なし、戻り値なしなので、呼ばれたことだけ記録
}

