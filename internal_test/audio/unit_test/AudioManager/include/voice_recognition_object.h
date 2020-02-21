/*
stub version
本物はaudio/objects/recognition/voice_recognition_object.h
*/
#ifndef _VOICE_RECOGNITION_OBJECT_H_
#define _VOICE_RECOGNITION_OBJECT_H_
#include <stdbool.h>
#include <stdint.h>
#include "common_ecode.h" // audio/common/chateau/include

/* VADのデフォルトパラメータ */
/* VADのデフォルトパラメータ */
extern uint32_t VADCoef_table[16];

typedef void (*voiceTriggerCB)(void);
typedef void (*voiceCommandCB)(uint16_t key_word, uint8_t status);

#ifdef  __cplusplus
extern "C"{
#endif
err_t AS_voiceRecognitionObjectSetWaitKeyStatus(voiceTriggerCB cbFunc, uint32_t mathfuncConfigTableAddress,uint32_t vadParamTableAddress);
err_t AS_voiceRecognitionObjectSetReadyStatus(void);
err_t AS_startVoiceCmd(uint16_t KeyWord, uint8_t vad_only, voiceCommandCB cbFunc, uint8_t *p_vad_param);
err_t AS_stopVoiceCmd(void);
bool AS_activateVoiceCmd(void);
bool AS_deactivateVoiceCmd(void);
#ifdef  __cplusplus
}
#endif

#endif /* _VOICE_RECOGNITION_OBJECT_H_ */
