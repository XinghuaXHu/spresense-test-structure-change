/***********************************************************************
 *
 *      File Name: test_define.h
 *
 *      Description: Defines for Test
 *
 *      Notes: (C) Copyright 2016 Sony Corporation
 *
 *      Author: Takuya OHASHI
 *
 ***********************************************************************
 */
#ifndef __TEST_DEFINE_H_INCLUDED__
#define __TEST_DEFINE_H_INCLUDED__

/* SDK の中で参照したいヘッダを選択する */
#include "../sdk/spritzer/defconfig_debug.h"
//#include "../sdk/spritzer/defconfig_release.h"

/*
 * 以下で、必要な定義を上書きする
 *
 * 例)
 * #undef SUPPORT_MP3_PLAYER
 * #define SUPPORT_WAV_PLAYER
 */

//#undef ENABLE_FLASH_BOOT

#undef NDEBUG

#endif /* __TEST_DEFINE_H_INCLUDED__ */
