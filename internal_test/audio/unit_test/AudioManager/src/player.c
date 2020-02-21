/*
Player Object‚Ìstub
*/
#include <stdbool.h>
#include "stub/player.h"

/* AS_activatePlayer */
int AS_activatePlayer_Called = 0;
bool AS_activatePlayer_ReturnValue = true;
bool AS_activatePlayer(void)
{
	AS_activatePlayer_Called++;
	return AS_activatePlayer_ReturnValue;
}

/* AS_deactivatePlayer */
int AS_deactivatePlayer_Called = 0;
bool AS_deactivatePlayer_ReturnValue = true;
bool AS_deactivatePlayer(void)
{
	AS_deactivatePlayer_Called++;
	return AS_deactivatePlayer_ReturnValue;
}

