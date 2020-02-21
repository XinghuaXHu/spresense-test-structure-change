/*
SoundEffect Object‚Ìstub
*/
#include <stdbool.h>
#include "stub/sound_effect.h"

/* AS_activateSoundEffect */
int AS_activateSoundEffect_Called = 0;
bool AS_activateSoundEffect_ReturnValue = true;
bool AS_activateSoundEffect(void)
{
	AS_activateSoundEffect_Called++;
	return AS_activateSoundEffect_ReturnValue;
}

/* AS_deactivateSoundEffect */
int AS_deactivateSoundEffect_Called = 0;
bool AS_deactivateSoundEffect_ReturnValue = true;
bool AS_deactivateSoundEffect(void)
{
	AS_deactivateSoundEffect_Called++;
	return AS_deactivateSoundEffect_ReturnValue;
}

