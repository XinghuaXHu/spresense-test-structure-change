#ifndef _STUB_RECOGNITION_OBJECT_H_
#define _STUB_RECOGNITION_OBJECT_H_

//#include "voice_recognition_object.h" // stub version

/* �X�^�u����/�ϑ��p(���̂̓X�^�u��) */
extern int AS_voiceRecognitionObjectSetWaitKeyStatus_Called;
extern void *AS_voiceRecognitionObjectSetWaitKeyStatus_cbFunc;
extern uint32_t AS_voiceRecognitionObjectSetWaitKeyStatus_mathfuncConfigTableAddress;
extern err_t AS_voiceRecognitionObjectSetWaitKeyStatus_ReturnValue;
extern int AS_voiceRecognitionObjectSetReadyStatus_Called;
extern err_t AS_voiceRecognitionObjectSetReadyStatus_ReturnValue;
extern int AS_voiceRecognitionObjectStartVoiceCommand_Called;
extern uint16_t AS_voiceRecognitionObjectStartVoiceCommand_KeyWord;
extern uint8_t AS_voiceRecognitionObjectStartVoiceCommand_vad_only;
extern void *AS_voiceRecognitionObjectStartVoiceCommand_cbFunc;
extern err_t AS_voiceRecognitionObjectStartVoiceCommand_ReturnValue;
extern int AS_voiceRecognitionObjectStopVoiceCommand_Called;
extern err_t AS_voiceRecognitionObjectStopVoiceCommand_ReturnValue;

extern int AS_deactivateVoiceCmd_Called;
extern bool AS_deactivateVoiceCmd_ReturnValue;
extern int AS_activateVoiceCmd_Called;
extern bool AS_activateVoiceCmd_Called_ReturnValue;

/* �֐��Ăяo�����J�E���^(���̂̓e�X�g�R�[�h��) */
extern int cpputest_call_sequence;

#endif /* _STUB_RECOGNITION_OBJECT_H_ */
