/*
VoiceRecoder Object‚Ìstub
*/
#include <stdbool.h>
#include "stub/recorder.h"

/* AS_activateVoiceRecorder */
int AS_activateVoiceRecorder_Called = 0;
bool AS_activateVoiceRecorder_ReturnValue = true;
bool AS_activateVoiceRecorder(void)
{
	AS_activateVoiceRecorder_Called++;
	return AS_activateVoiceRecorder_ReturnValue;
}

/* AS_deactivateVoiceRecorder */
int AS_deactivateVoiceRecorder_Called = 0;
bool AS_deactivateVoiceRecorder_ReturnValue = true;
bool AS_deactivateVoiceRecorder(void)
{
	AS_deactivateVoiceRecorder_Called++;
	return AS_deactivateVoiceRecorder_ReturnValue;
}

