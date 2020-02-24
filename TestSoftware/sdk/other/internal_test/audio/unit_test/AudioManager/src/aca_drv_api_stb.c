/***********************************************************************
 *
 *      File Name: aca_drv_api_stb.c
 *
 *      Description: Stub file of aca_drv_api.c
 *
 *      Notes: (C) Copyright 2016 Sony Corporation
 *
 *      Author: Hsingying Ho
 *
 ***********************************************************************
 */

#include "as_drv_common.h"

int asAdo_PowerOnMicBiasA_Called = 0;
E_AS asAdo_PowerOnMicBiasA_ReturnValue = E_AS_OK; // åªèÛñﬂÇËílÇÕï]âøÇ≥ÇÍÇƒÇ¢Ç»Ç¢ÇÃÇ≈ébíË
E_AS asAdo_PowerOnMicBiasA( void )
{
	asAdo_PowerOnMicBiasA_Called++;
	return asAdo_PowerOnMicBiasA_ReturnValue;
}

/*--------------------------------------------------------------------*/
int asAdo_PowerOffMicBiasA_Called = 0;
E_AS asAdo_PowerOffMicBiasA_ReturnValue = E_AS_OK; // åªèÛñﬂÇËílÇÕï]âøÇ≥ÇÍÇƒÇ¢Ç»Ç¢ÇÃÇ≈ébíË
E_AS asAdo_PowerOffMicBiasA( void )
{
	asAdo_PowerOffMicBiasA_Called++;
	return asAdo_PowerOffMicBiasA_ReturnValue;
}

