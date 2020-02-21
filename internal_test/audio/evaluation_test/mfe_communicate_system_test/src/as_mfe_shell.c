/*
* Copyright 2015 Sony Corporation
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions, and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/

/*******************************************************
    Include
*******************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <debug/dbg_shell.h>
#include <debug/dbg_log.h>
#include <audio/high_level_api/as_high_level_api.h>

#define DBG_MODULE DBG_MODULE_DBG

#define DSP_VOL_STEP  5

static int32_t g_dsp_vol = 0;

static char mfe_coef_table[1024] = {
	0x50, 0x02, 0x01, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x40, 0x9A, 0x99, 0x19, 0x3F,
	0xCD, 0xCC, 0x4C, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x9A, 0x99, 0x19, 0x3F, 0x33, 0x33, 0x33, 0x3F,
	0x9A, 0x99, 0x99, 0x3F, 0x8F, 0xC2, 0xF5, 0x3D, 0x9A, 0x99, 0x19, 0x3E, 0x0A, 0xD7, 0x23, 0x3E,
	0xB8, 0x1E, 0x05, 0x3E, 0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D,
	0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D,
	0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D,
	0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D, 0x0A, 0xD7, 0x23, 0x3D,
	0x0A, 0xD7, 0x23, 0x3D, 0x9A, 0x99, 0x19, 0x3D, 0x29, 0x5C, 0x0F, 0x3D, 0xB8, 0x1E, 0x05, 0x3D,
	0x8F, 0xC2, 0xF5, 0x3C, 0xAE, 0x47, 0xE1, 0x3C, 0xCD, 0xCC, 0xCC, 0x3C, 0xEC, 0x51, 0xB8, 0x3C,
	0x0A, 0xD7, 0xA3, 0x3C, 0x0A, 0xD7, 0xA3, 0x3C, 0x0A, 0xD7, 0xA3, 0x3C, 0x0A, 0xD7, 0xA3, 0x3C,
	0x0A, 0xD7, 0xA3, 0x3C, 0x0A, 0xD7, 0xA3, 0x3C, 0x0A, 0xD7, 0xA3, 0x3C, 0x0A, 0xD7, 0xA3, 0x3C,
	0x0A, 0xD7, 0xA3, 0x3C, 0x0A, 0xD7, 0x23, 0x3D, 0x9A, 0x99, 0x19, 0x3F, 0x33, 0x33, 0x33, 0x3F,
	0x9A, 0x99, 0x99, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0xCD, 0xCC, 0x4C, 0x3F, 0x00, 0x00, 0xC0, 0x3F,
	0x9A, 0x99, 0x19, 0x3F, 0x33, 0x33, 0x33, 0x3F, 0x9A, 0x99, 0x99, 0x3F, 0x9A, 0x99, 0x19, 0x3F,
	0x33, 0x33, 0x33, 0x3F, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F,
	0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x58, 0x02, 0x00, 0x00,
	0xD0, 0x07, 0x00, 0x00, 0xA0, 0x0F, 0x00, 0x00, 0x40, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0xA0, 0x41, 0x66, 0x66, 0x66, 0x3F, 0x00, 0x00, 0x00, 0x3F,
	0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x3F, 0x96, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x00, 0x00,
	0xF4, 0x01, 0x00, 0x00, 0x33, 0x33, 0x33, 0x3F, 0x9A, 0x99, 0x99, 0x3F, 0x0A, 0xD7, 0x23, 0x3C,
	0xCD, 0xCC, 0x4C, 0x3E, 0x00, 0x00, 0x80, 0x3F, 0xA3, 0xE8, 0xA1, 0x3E, 0x9A, 0x99, 0x19, 0x3E,
	0xCD, 0xCC, 0xCC, 0x3D, 0x00, 0x00, 0x40, 0x3F, 0x33, 0x33, 0x33, 0x3F, 0x9A, 0x99, 0x59, 0x3F,
	0x9A, 0x99, 0x59, 0x3F, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x90, 0xA9, 0x42,
	0x4B, 0xC0, 0x30, 0x43, 0x4E, 0x42, 0x8A, 0x43, 0x23, 0x5F, 0xC0, 0x43, 0x70, 0x12, 0xFB, 0x43,
	0xE3, 0x5F, 0x1D, 0x44, 0x95, 0xE9, 0x3F, 0x44, 0xE4, 0x60, 0x65, 0x44, 0xAF, 0x02, 0x87, 0x44,
	0xF9, 0x0D, 0x9D, 0x44, 0xB6, 0xF7, 0xB4, 0x44, 0x76, 0xE8, 0xCE, 0x44, 0x3A, 0x0C, 0xEB, 0x44,
	0x5E, 0xC9, 0x04, 0x45, 0xE3, 0x57, 0x15, 0x45, 0xC1, 0x4D, 0x27, 0x45, 0x6F, 0xC9, 0x3A, 0x45,
	0xFB, 0xEB, 0x4F, 0x45, 0x3D, 0xD9, 0x66, 0x45, 0x1B, 0xB8, 0x7F, 0x45, 0x62, 0x59, 0x8D, 0x45,
	0x7D, 0xFB, 0x9B, 0x45, 0x33, 0xDB, 0xAB, 0x45, 0x6F, 0x13, 0xBD, 0x45, 0x69, 0xC1, 0xCF, 0x45,
	0xCE, 0x04, 0xE4, 0x45, 0x00, 0x00, 0xFA, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x77, 0xBE, 0x7F, 0x3F,
	0x66, 0x66, 0x66, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x6F, 0x12, 0x83, 0x3A, 0x00, 0x00, 0x80, 0x3F,
	0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F,
	0xA9, 0x9E, 0xE3, 0x3F, 0xBF, 0xC2, 0x20, 0x40, 0xC2, 0x62, 0x4A, 0x40, 0xC2, 0x62, 0x4A, 0x40,
	0xC2, 0x62, 0x4A, 0x40, 0xC2, 0x62, 0x4A, 0x40, 0xBF, 0xC2, 0x20, 0x40, 0xA9, 0x9E, 0xE3, 0x3F,
	0x00, 0x00, 0x80, 0x3F, 0x9A, 0xF5, 0x0F, 0x3F, 0x9B, 0xE8, 0xA1, 0x3E, 0x87, 0x18, 0x36, 0x3E,
	0x6F, 0x12, 0x03, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static char xloud_coef_table[512] = {
	0xFC, 0x4A, 0xFF, 0x71, 0x33, 0xBE, 0xBC, 0x90, 0x50, 0x1B, 0x2E, 0x67, 0x64, 0xBE, 0x5D, 0xA6,
	0x7B, 0x0B, 0x0F, 0xED, 0x1F, 0xC4, 0x45, 0xD8, 0x28, 0x3D, 0x77, 0x35, 0xCF, 0x0B, 0x24, 0xC7,
	0xBA, 0x76, 0xF0, 0x0C, 0xED, 0xD6, 0x7C, 0x28, 0xCC, 0xCB, 0x39, 0x68, 0x9F, 0xBB, 0x34, 0x50,
	0x05, 0x85, 0x5B, 0x8F, 0x6D, 0x55, 0x15, 0x26, 0x50, 0xB1, 0x10, 0xE1, 0xD0, 0xB4, 0x4D, 0xBA,
	0x39, 0xCE, 0x0A, 0xB8, 0xB4, 0x15, 0xEF, 0x6D, 0xE4, 0x1E, 0x5B, 0xAE, 0x49, 0x9B, 0xC1, 0x72,
	0x54, 0xC9, 0xF5, 0x25, 0xB5, 0x80, 0x07, 0xD1, 0xC3, 0x06, 0x9F, 0xC9, 0x51, 0x2B, 0x25, 0xB2,
	0x75, 0x61, 0xC6, 0xC7, 0x6B, 0x0B, 0x29, 0x56, 0x96, 0xA6, 0xA1, 0x3D, 0xDA, 0x44, 0x5C, 0x35,
	0x40, 0xA5, 0x67, 0x9F, 0xA3, 0x36, 0x57, 0x10, 0x63, 0xC4, 0x68, 0xD9, 0x85, 0x19, 0x37, 0x22,
	0xB2, 0xD0, 0xF1, 0x48, 0x55, 0xA2, 0xC5, 0xE5, 0xA6, 0x52, 0x21, 0x05, 0xA4, 0xE0, 0x94, 0x36,
	0xA5, 0x65, 0x45, 0xC2, 0xD3, 0x07, 0x37, 0x5F, 0xA3, 0x54, 0x91, 0x93, 0x96, 0x72, 0xCB, 0x1F,
	0x3F, 0x2B, 0xCA, 0x94, 0x14, 0x89, 0x0C, 0xFA, 0x62, 0xEC, 0xF8, 0xB9, 0xB5, 0xEA, 0x59, 0xE6,
	0xE0, 0x86, 0xB1, 0x8A, 0x56, 0xC4, 0xF7, 0x87, 0x0E, 0xA8, 0x93, 0x8D, 0xE8, 0x66, 0xEB, 0x7C,
	0x06, 0x8E, 0x84, 0x25, 0x6F, 0x66, 0x70, 0x96, 0xAE, 0xCE, 0xB7, 0x34, 0x46, 0xC7, 0x3C, 0xF0,
	0x5D, 0x5D, 0xD6, 0xCE, 0xC0, 0x4B, 0x73, 0x49, 0x8B, 0xDB, 0x15, 0x82, 0x0E, 0xB9, 0x6C, 0x22,
	0x2A, 0x30, 0xFC, 0x7B, 0x3A, 0xA1, 0x7A, 0x38, 0x55, 0x9C, 0x03, 0xEF, 0xE0, 0xC8, 0x62, 0x01,
	0xD7, 0x7C, 0xDE, 0xAC, 0x56, 0x27, 0xF3, 0x7B, 0x62, 0xFA, 0x16, 0x39, 0x19, 0x13, 0xE5, 0xD7,
	0xE1, 0xC6, 0x3E, 0xCC, 0xB4, 0x99, 0xB9, 0x59, 0x5A, 0x6D, 0x8D, 0xC6, 0x25, 0xD1, 0xD9, 0xD2,
	0xF0, 0xC8, 0x07, 0xD0, 0x53, 0xFD, 0xCB, 0x3F, 0xED, 0x29, 0x59, 0xD4, 0xD0, 0x17, 0x94, 0xF5,
	0x13, 0x88, 0x8F, 0xB7, 0x68, 0x29, 0x13, 0x68, 0xB1, 0xD6, 0x96, 0x65, 0x0B, 0x4B, 0xD2, 0xFD,
	0x7C, 0x03, 0x01, 0xE2, 0x0C, 0xF3, 0x71, 0x1C, 0xB8, 0x8F, 0xE6, 0x58, 0x55, 0x6B, 0xB0, 0x08,
	0x55, 0x3D, 0xC6, 0xD3, 0xA1, 0xC8, 0xF5, 0x2A, 0xDF, 0x84, 0x66, 0x27, 0x5A, 0x1B, 0xC7, 0x37,
	0x1C, 0xF0, 0x88, 0x31, 0xF5, 0xC5, 0x7F, 0xAD, 0xCE, 0xC2, 0x93, 0x7C, 0x25, 0x44, 0xE5, 0x3D,
	0xD5, 0x2D, 0x62, 0x25, 0xA4, 0xE6, 0x6B, 0x28, 0x85, 0xA7, 0x3A, 0xA0, 0x2F, 0x3C, 0xEC, 0x79,
	0xA0, 0xE7, 0xA0, 0x8B, 0xF8, 0x20, 0xB6, 0x8A, 0x43, 0x87, 0xAE, 0x9E, 0x80, 0xDE, 0x10, 0xED,
	0xBB, 0xAA, 0x4A, 0xAA, 0x19, 0x01, 0x50, 0x42, 0x26, 0xE8, 0x4D, 0x16, 0x01, 0x32, 0x39, 0x7F,
	0xA1, 0xB6, 0x48, 0x22, 0x17, 0xC8, 0x22, 0xBD, 0x43, 0x8E, 0x4B, 0xCB, 0x10, 0xFC, 0x4B, 0x36,
	0xD0, 0x97, 0x12, 0x6C, 0x1B, 0x9B, 0x59, 0x42, 0xDB, 0x93, 0xEF, 0x4F, 0xA9, 0xFB, 0xD3, 0x3B,
	0x61, 0x45, 0x5B, 0x4D, 0x38, 0x8A, 0x95, 0xB9, 0xD2, 0x1A, 0x54, 0xBB, 0x58, 0xE6, 0xFD, 0x6C,
	0x28, 0xCE, 0x45, 0x55, 0x3B, 0xC0, 0x25, 0xC1, 0x73, 0xE3, 0xF1, 0xD8, 0x17, 0xAB, 0x93, 0x56,
	0xF8, 0xF4, 0x46, 0x78, 0xE3, 0x2D, 0x63, 0xB2, 0x7F, 0x70, 0x8F, 0x75, 0x8E, 0x72, 0x21, 0x91,
	0xB4, 0xC5, 0x67, 0xF8, 0x15, 0x08, 0xF9, 0xC5, 0xF9, 0xAB, 0x63, 0x9E, 0x3A, 0x54, 0xD5, 0xC2,
	0xA2, 0xB8, 0x3B, 0x83, 0x90, 0x11, 0x85, 0xBA, 0x49, 0x6C, 0xB7, 0x67, 0x88, 0xF7, 0x48, 0x91
};
static char eax_coef_table[512] = {
	0xFC, 0x4A, 0xFF, 0x71, 0x33, 0xBE, 0xBC, 0x90, 0x50, 0x1B, 0x2E, 0x67, 0x64, 0xBE, 0x5D, 0xA6,
	0x7B, 0x0B, 0x0F, 0xED, 0x1F, 0xC4, 0x45, 0xD8, 0x28, 0x3D, 0x77, 0x35, 0xCF, 0x0B, 0x24, 0xC7,
	0xBA, 0x76, 0xF0, 0x0C, 0xED, 0xD6, 0x7C, 0x28, 0xCC, 0xCB, 0x39, 0x68, 0x9F, 0xBB, 0x34, 0x50,
	0x05, 0x85, 0x5B, 0x8F, 0x6D, 0x55, 0x15, 0x26, 0x50, 0xB1, 0x10, 0xE1, 0xD0, 0xB4, 0x4D, 0xBA,
	0x39, 0xCE, 0x0A, 0xB8, 0xB4, 0x15, 0xEF, 0x6D, 0xE4, 0x1E, 0x5B, 0xAE, 0x49, 0x9B, 0xC1, 0x72,
	0x54, 0xC9, 0xF5, 0x25, 0xB5, 0x80, 0x07, 0xD1, 0xC3, 0x06, 0x9F, 0xC9, 0x51, 0x2B, 0x25, 0xB2,
	0x75, 0x61, 0xC6, 0xC7, 0x6B, 0x0B, 0x29, 0x56, 0x96, 0xA6, 0xA1, 0x3D, 0xDA, 0x44, 0x5C, 0x35,
	0x40, 0xA5, 0x67, 0x9F, 0xA3, 0x36, 0x57, 0x10, 0x63, 0xC4, 0x68, 0xD9, 0x85, 0x19, 0x37, 0x22,
	0xB2, 0xD0, 0xF1, 0x48, 0x55, 0xA2, 0xC5, 0xE5, 0xA6, 0x52, 0x21, 0x05, 0xA4, 0xE0, 0x94, 0x36,
	0xA5, 0x65, 0x45, 0xC2, 0xD3, 0x07, 0x37, 0x5F, 0xA3, 0x54, 0x91, 0x93, 0x96, 0x72, 0xCB, 0x1F,
	0x3F, 0x2B, 0xCA, 0x94, 0x14, 0x89, 0x0C, 0xFA, 0x62, 0xEC, 0xF8, 0xB9, 0xB5, 0xEA, 0x59, 0xE6,
	0xE0, 0x86, 0xB1, 0x8A, 0x56, 0xC4, 0xF7, 0x87, 0x0E, 0xA8, 0x93, 0x8D, 0xE8, 0x66, 0xEB, 0x7C,
	0x06, 0x8E, 0x84, 0x25, 0x6F, 0x66, 0x70, 0x96, 0xAE, 0xCE, 0xB7, 0x34, 0x46, 0xC7, 0x3C, 0xF0,
	0x5D, 0x5D, 0xD6, 0xCE, 0xC0, 0x4B, 0x73, 0x49, 0x8B, 0xDB, 0x15, 0x82, 0x0E, 0xB9, 0x6C, 0x22,
	0x2A, 0x30, 0xFC, 0x7B, 0x3A, 0xA1, 0x7A, 0x38, 0x55, 0x9C, 0x03, 0xEF, 0xE0, 0xC8, 0x62, 0x01,
	0xD7, 0x7C, 0xDE, 0xAC, 0x56, 0x27, 0xF3, 0x7B, 0x62, 0xFA, 0x16, 0x39, 0x19, 0x13, 0xE5, 0xD7,
	0xE1, 0xC6, 0x3E, 0xCC, 0xB4, 0x99, 0xB9, 0x59, 0x5A, 0x6D, 0x8D, 0xC6, 0x25, 0xD1, 0xD9, 0xD2,
	0xF0, 0xC8, 0x07, 0xD0, 0x53, 0xFD, 0xCB, 0x3F, 0xED, 0x29, 0x59, 0xD4, 0xD0, 0x17, 0x94, 0xF5,
	0x13, 0x88, 0x8F, 0xB7, 0x68, 0x29, 0x13, 0x68, 0xB1, 0xD6, 0x96, 0x65, 0x0B, 0x4B, 0xD2, 0xFD,
	0x7C, 0x03, 0x01, 0xE2, 0x0C, 0xF3, 0x71, 0x1C, 0xB8, 0x8F, 0xE6, 0x58, 0x55, 0x6B, 0xB0, 0x08,
	0x55, 0x3D, 0xC6, 0xD3, 0xA1, 0xC8, 0xF5, 0x2A, 0xDF, 0x84, 0x66, 0x27, 0x5A, 0x1B, 0xC7, 0x37,
	0x1C, 0xF0, 0x88, 0x31, 0xF5, 0xC5, 0x7F, 0xAD, 0xCE, 0xC2, 0x93, 0x7C, 0x25, 0x44, 0xE5, 0x3D,
	0xD5, 0x2D, 0x62, 0x25, 0xA4, 0xE6, 0x6B, 0x28, 0x85, 0xA7, 0x3A, 0xA0, 0x2F, 0x3C, 0xEC, 0x79,
	0xA0, 0xE7, 0xA0, 0x8B, 0xF8, 0x20, 0xB6, 0x8A, 0x43, 0x87, 0xAE, 0x9E, 0x80, 0xDE, 0x10, 0xED,
	0xBB, 0xAA, 0x4A, 0xAA, 0x19, 0x01, 0x50, 0x42, 0x26, 0xE8, 0x4D, 0x16, 0x01, 0x32, 0x39, 0x7F,
	0xA1, 0xB6, 0x48, 0x22, 0x17, 0xC8, 0x22, 0xBD, 0x43, 0x8E, 0x4B, 0xCB, 0x10, 0xFC, 0x4B, 0x36,
	0xD0, 0x97, 0x12, 0x6C, 0x1B, 0x9B, 0x59, 0x42, 0xDB, 0x93, 0xEF, 0x4F, 0xA9, 0xFB, 0xD3, 0x3B,
	0x61, 0x45, 0x5B, 0x4D, 0x38, 0x8A, 0x95, 0xB9, 0xD2, 0x1A, 0x54, 0xBB, 0x58, 0xE6, 0xFD, 0x6C,
	0x28, 0xCE, 0x45, 0x55, 0x3B, 0xC0, 0x25, 0xC1, 0x73, 0xE3, 0xF1, 0xD8, 0x17, 0xAB, 0x93, 0x56,
	0xF8, 0xF4, 0x46, 0x78, 0xE3, 0x2D, 0x63, 0xB2, 0x7F, 0x70, 0x8F, 0x75, 0x8E, 0x72, 0x21, 0x91,
	0xB4, 0xC5, 0x67, 0xF8, 0x15, 0x08, 0xF9, 0xC5, 0xF9, 0xAB, 0x63, 0x9E, 0x3A, 0x54, 0xD5, 0xC2,
	0xA2, 0xB8, 0x3B, 0x83, 0x90, 0x11, 0x85, 0xBA, 0x49, 0x6C, 0xB7, 0x67, 0x88, 0xF7, 0x48, 0x91
};

/* Declaration of functions */
void asGetStatus(void);
void asInitOutputSelect(void);
void asInitI2SParam(void);
void asInitMicGain(void);
void asSetActStatus(void);
void asReadyStatus(void);
void asInitMfe(void);
void asInitMpp(void);
void asStartBB(void);
void asStopBB(void);
void asSetVolume(void);
void asSetMpp(void);
void asStartCmd(void);
void asStopCmd(void);
void asPowerOn(void);
void asSetPowerOffStatus(void);

/*--------------------------------------------------------------------*/
/* Methods for sending audio APIs. */
/*--------------------------------------------------------------------*/
void asGetStatus(void)
{
	AudioCommand cmd;
	AudioResult result;
	
	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	
	/* get status */
	cmd.header.packet_length = 2;
	cmd.header.command_code  = AUDCMD_GETSTATUS;
	cmd.header.sub_code      = 0x00;
	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

 	if(AUDRLT_NOTIFYSTATUS != result.header.result_code) {
 		DBG_LOG_ERROR("getStatus Error!\n");
 	}

	DBG_LOGF_INFO("### status_info = %d, sub_status_info = %d, vad_status = %d\n", result.notify_status.status_info, result.notify_status.sub_status_info, result.notify_status.vad_status);
}

/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
void asInitOutputSelect(void)
{
	AudioCommand cmd;
	AudioResult result;

	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_INITOUTPUTSELECT;
	cmd.header.command_code  = AUDCMD_INITOUTPUTSELECT;
	cmd.header.sub_code      = 0;

	cmd.init_output_select_param.output_device_sel = AS_OUT_SP;

	AS_SendAudioCommand( &cmd );
	AS_ReceiveAudioResult( &result );

	if(AUDRLT_INITOUTPUTSELECTCMPLT != result.header.result_code) {
 		DBG_LOG_ERROR("asInitOutputSelect Error!\n");
 	}
	else {
		DBG_LOG_INFO("asInitOutputSelect Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asInitI2SParam(void)
{
	AudioCommand cmd;
	AudioResult result;

	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_INITI2SPARAM;
	cmd.header.command_code  = AUDCMD_INITI2SPARAM;
	cmd.header.sub_code      = 0;

	cmd.init_i2s_param.i2s_id = AS_I2S1;
	cmd.init_i2s_param.rate = 48000;
	cmd.init_i2s_param.bypass_mode_en = AS_I2S_BYPASS_MODE_DISABLE;

	AS_SendAudioCommand( &cmd );
	AS_ReceiveAudioResult( &result );

	if(AUDRLT_INITI2SPARAMCMPLT != result.header.result_code) {
 		DBG_LOG_ERROR("asInitI2SParam Error!\n");
 	}
	else {
		DBG_LOG_INFO("asInitI2SParam Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asInitMicGain(void)
{
	AudioCommand cmd;
	AudioResult result;

	cmd.header.packet_length = LENGTH_INITMICGAIN;
	cmd.header.command_code  = AUDCMD_INITMICGAIN;
	cmd.header.sub_code      = 0;

	cmd.init_mic_gain_param.mic_gain[0] = 210;
	cmd.init_mic_gain_param.mic_gain[1] = 210;
	cmd.init_mic_gain_param.mic_gain[2] = 210;
	cmd.init_mic_gain_param.mic_gain[3] = 210;
	cmd.init_mic_gain_param.mic_gain[4] = AS_MICGAIN_HOLD;
	cmd.init_mic_gain_param.mic_gain[5] = AS_MICGAIN_HOLD;
	cmd.init_mic_gain_param.mic_gain[6] = AS_MICGAIN_HOLD;
	cmd.init_mic_gain_param.mic_gain[7] = AS_MICGAIN_HOLD;

	AS_SendAudioCommand( &cmd );
	AS_ReceiveAudioResult( &result );

	if(AUDRLT_INITMICGAINCMPLT != result.header.result_code) {
 		DBG_LOG_ERROR("asInitMicGain Error!\n");
 	}
	else {
		DBG_LOG_INFO("asInitMicGain Complete.\n");
	}
}
/*--------------------------------------------------------------------*/
void asPowerOn(void)
{
	AudioCommand cmd;
	AudioResult result;
	
	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_POWERON;
	cmd.header.command_code  = AUDCMD_POWERON;
	cmd.header.sub_code      = 0x00;

	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

	if(AUDRLT_STATUSCHANGED != result.header.result_code) {
		DBG_LOG_ERROR("PowerOn Error!\n");
	}
	else {
		DBG_LOG_INFO("PowerOn Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asSetPowerOffStatus(void)
{
	AudioCommand cmd;
	AudioResult result;
	
	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_SET_POWEROFF_STATUS;
	cmd.header.command_code  = AUDCMD_SETPOWEROFFSTATUS;
	cmd.header.sub_code      = 0x00;

	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

	if(AUDRLT_STATUSCHANGED != result.header.result_code) {
		DBG_LOG_ERROR("PowerOff Error!\n");
	}
	else {
		DBG_LOG_INFO("PowerOff Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asSetActStatus(void)
{
	AudioCommand cmd;
	AudioResult result;
	
	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_SET_BASEBAND_STATUS;
	cmd.header.command_code  = AUDCMD_SETBASEBANDSTATUS;
	cmd.header.sub_code      = 0x00;

	cmd.set_baseband_status_param.with_MFE = AS_SET_BBSTS_WITH_MFE_ACTIVE;
	cmd.set_baseband_status_param.with_Voice_Command = AS_SET_BBSTS_WITH_VCMD_NONE;
	cmd.set_baseband_status_param.with_MPP = AS_SET_BBSTS_WITH_MPP_NONE;
	cmd.set_baseband_status_param.input_device = AS_INPUT_DEVICE_AMIC4CH;
	cmd.set_baseband_status_param.output_device = AS_OUTPUT_DEVICE_I2S2CH;

	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

 	if(AUDRLT_STATUSCHANGED != result.header.result_code) {
 		DBG_LOG_ERROR("SetActiveStatus Error!\n");
 	}
	else {
		DBG_LOG_INFO("SetActiveStatus Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asReadyStatus(void)
{
	AudioCommand cmd;
	AudioResult result;
	
	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_SET_READY_STATUS;
	cmd.header.command_code  = AUDCMD_SETREADYSTATUS;
	cmd.header.sub_code      = 0x00;

	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

	if(AUDRLT_STATUSCHANGED != result.header.result_code) {
		DBG_LOG_ERROR("SetReadyStatus Error!\n");
	}
	else {
		DBG_LOG_INFO("SetReadyStatus Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asInitMfe(void)
{
	AudioCommand cmd;
	AudioResult result;
	
	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_INITMFE;
	cmd.header.command_code  = AUDCMD_INITMFE;
	cmd.header.sub_code      = 0x00;

	cmd.init_mfe_param.input_fs           = AS_SAMPLINGRATE_16000;
	cmd.init_mfe_param.ref_channel_num    = AS_CHANNEL_STEREO;
	cmd.init_mfe_param.mic_channel_num    = AS_CHANNEL_MONO;
	cmd.init_mfe_param.enable_echocancel  = AS_ENABLE_ECHOCANCEL;
	cmd.init_mfe_param.include_echocancel = AS_INCLUDE_ECHOCANCEL;
	cmd.init_mfe_param.mfe_mode           = AS_MFE_MODE_SPEAKING;
	cmd.init_mfe_param.config_table       = (uint32_t)(mfe_coef_table);

	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

 	if(AUDRLT_INITMFECMPLT != result.header.result_code) {
 		DBG_LOG_ERROR("InitMfe Error!\n");
 	}
	else {
		DBG_LOG_INFO("InitMfe Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asInitMpp(void)
{
	AudioCommand cmd;
	AudioResult result;
	
	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_INITMPP;
	cmd.header.command_code  = AUDCMD_INITMPP;
	cmd.header.sub_code      = 0x00;

	cmd.init_mpp_param.output_fs          = AS_SAMPLINGRATE_48000;
	cmd.init_mpp_param.output_channel_num = 2;
	cmd.init_mpp_param.mpp_mode           = AS_MPP_MODE_XLOUD_ONLY;
	cmd.init_mpp_param.xloud_mode         = AS_MPP_XLOUD_MODE_NORMAL;	/* TODO: dummy */
	cmd.init_mpp_param.coef_mode          = AS_MPP_COEF_SPEAKER;
	if(cmd.init_mpp_param.xloud_mode == AS_MPP_XLOUD_MODE_DISABLE){
		cmd.init_mpp_param.eax_mode           = AS_MPP_EAX_DISABLE;
		cmd.init_mpp_param.xloud_coef_table   = 0;
		cmd.init_mpp_param.eax_coef_table     = 0;
	}
	else{
		cmd.init_mpp_param.eax_mode           = AS_MPP_EAX_ENABLE;
		cmd.init_mpp_param.xloud_coef_table   = (uint32_t)(xloud_coef_table);
		cmd.init_mpp_param.eax_coef_table     = (uint32_t)(eax_coef_table);
	}

	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

	if(AUDRLT_INITMPPCMPLT != result.header.result_code) {
		DBG_LOG_ERROR("InitMpp Error!\n");
	}
	else {
		DBG_LOG_INFO("InitMpp Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asStartBB(void)
{
	AudioCommand cmd;
	AudioResult result;

	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_STARTBB;
	cmd.header.command_code  = AUDCMD_STARTBB;
	cmd.header.sub_code      = 0x00;
	
	cmd.start_bb_param.output_device     = AS_OUTPUT_DEVICE_I2S2CH;
	cmd.start_bb_param.input_device      = AS_INPUT_DEVICE_AMIC4CH;
	cmd.start_bb_param.select_output_mic = AS_SELECT_MIC0_OR_MIC3;
	cmd.start_bb_param.I2S_output_data   = AS_MFE_OUTPUT_MICSIN;
	cmd.start_bb_param.SP_output_data    = AS_MPP_OUTPUT_I2SIN;
	
	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

	if(AUDRLT_STARTBBCMPLT != result.header.result_code) {
		DBG_LOG_ERROR("StartBB Error!\n");
	}
	else {
		DBG_LOG_INFO("StartBB Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asStopBB(void)
{
	AudioCommand cmd;
	AudioResult result;

	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_STOPBB;
	cmd.header.command_code  = AUDCMD_STOPBB;
	cmd.header.sub_code      = 0x00;
	
	cmd.stop_bb_param.stop_device = 0x00;

	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

	if(AUDRLT_STOPBBCMPLT != result.header.result_code) {
		DBG_LOG_ERROR("StopBB Error!\n");
	}
	else {
		DBG_LOG_INFO("StopBB Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asSetVolume(void)
{
	AudioCommand cmd;
	AudioResult result;
	
	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_SETVOLUME;
	cmd.header.command_code  = AUDCMD_SETVOLUME;
	cmd.header.sub_code      = 0;

	cmd.set_volume_param.input1_db = 0;
	cmd.set_volume_param.input2_db = AS_VOLUME_MUTE;
	cmd.set_volume_param.master_db = AS_AC_CODEC_VOL_DAC;

	AS_SendAudioCommand( &cmd );
	AS_ReceiveAudioResult( &result );

 	if(AUDRLT_SETVOLUMECMPLT != result.header.result_code) {
 		DBG_LOG_ERROR("SetVolume Error!\n");
 	}
	else {
		DBG_LOG_INFO("SetVolume Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
void asSetMpp(void)
{
	AudioCommand cmd;
	AudioResult result;
	
	memset(&cmd, 0x00, sizeof(AudioCommand));
	memset(&result, 0x00, sizeof(AudioResult));
	cmd.header.packet_length = LENGTH_SUB_SETMPP_XLOUD;
	cmd.header.command_code  = AUDCMD_SETMPPPARAM;
	cmd.header.sub_code      = SUB_SETMPP_XLOUD;

	cmd.set_mpp_param.mpp_xloud_set.xloud_vol = 59;

	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

 	if(AUDRLT_SETMPPCMPLT != result.header.result_code) {
 		DBG_LOG_ERROR("SetMpp Error!\n");
 	}
	else {
		DBG_LOG_INFO("SetMpp Complete.\n");
	}
}

/*--------------------------------------------------------------------*/
/* Methods for debug shell */
/*--------------------------------------------------------------------*/
void dbgShellPowerOn(int argc, const char *args[], void *userData)
{
	asPowerOn();
}

/*--------------------------------------------------------------------*/
void dbgShellSetPowerOffStatus(int argc, const char *args[], void *userData)
{
	asSetPowerOffStatus();
}

/*--------------------------------------------------------------------*/
void dbgShellInitBB(int argc, const char *args[], void *userData)
{
	DBG_LOG_INFO("Ready Status.\n");

	/* init baseband */
	asInitOutputSelect();
	asInitI2SParam();
	asInitMicGain();
}

/*--------------------------------------------------------------------*/
void dbgShellInitMFE(int argc, const char *args[], void *userData)
{
	asInitMfe();
}

/*--------------------------------------------------------------------*/
void dbgShellInitMPP(int argc, const char *args[], void *userData)
{
	asInitMpp();
}

/*--------------------------------------------------------------------*/
void dbgShellSetActive(int argc, const char *args[], void *userData)
{
	asSetActStatus();
}

/*--------------------------------------------------------------------*/
void dbgShellSetReady(int argc, const char *args[], void *userData)
{
	asReadyStatus();
}

/*--------------------------------------------------------------------*/
void dbgShellStartBB(int argc, const char *args[], void *userData)
{
	asStartBB();
}

/*--------------------------------------------------------------------*/
void dbgShellStopBB(int argc, const char *args[], void *userData)
{
	asStopBB() ;
}

/*--------------------------------------------------------------------*/
void dbgShellSetVolume(int argc, const char *args[], void *userData)
{
	asSetVolume() ;
}

/*--------------------------------------------------------------------*/
void dbgShellSetMpp(int argc, const char *args[], void *userData)
{
	asSetMpp();
}

/*--------------------------------------------------------------------*/
void dbgShellGetStatus(int argc, const char *args[], void *userData)
{
	asGetStatus();
}

/*--------------------------------------------------------------------*/
void dbgShellSetXLVol(int argc, const char *args[], void *userData)
{
	AudioCommand      cmd;
	AudioResult       result;
	int32_t           xl_vol;

	xl_vol = atoi(*(args+2));

	cmd.header.packet_length = LENGTH_SUB_SETMPP_XLOUD;
	cmd.header.command_code  = AUDCMD_SETMPPPARAM;
	cmd.header.sub_code      = SUB_SETMPP_XLOUD;

	cmd.set_mpp_param.mpp_xloud_set.xloud_vol = xl_vol;

	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult( &result );

	if( result.header.result_code == AUDRLT_SETMPPCMPLT ){
		DBG_ShellPrintf("set xLOUD volume to %d\n", xl_vol);
	} else {
		DBG_ShellPrintf("ERROR(0x%02x)\n", result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
int dbg_shell_voiceCommandVadOnlyCb( void *data )
{
	AudioFindCommandInfo *p_info = (AudioFindCommandInfo *)data;
	if(p_info->status == 0) {
		DBG_LOG_INFO("->VAD FALL(VadOnly)\n");
	}
	else if(p_info->status == 1) {
		DBG_LOG_INFO("VAD RISE(VadOnly)->\n");
	}
	else if(p_info->status == 2) {
		DBG_LOG_ERROR("[ERROR]!!!Found Command!!!(VadOnly)\n");
	}
}

int dbg_shell_voiceCommandCb( void *data )
{
	AudioFindCommandInfo *p_info = (AudioFindCommandInfo *)data;
	if(p_info->status == 0) {
		DBG_LOG_INFO("->VAD FALL\n");
	}
	else if(p_info->status == 1) {
		DBG_LOG_INFO("VAD RISE->\n");
	}
	else if(p_info->status == 2) {
		DBG_LOG_INFO("Found Command\n");
	}
}

void dbgShellStartVoiceCommand(int argc, const char *args[], void *userData)
{
	AudioCommand cmd;
	AudioResult  result;
	int vad_only;
	static AudioFindCommandInfo s_find_command_info;
	
	vad_only = atoi(*(args+2));
	
	cmd.header.packet_length = LENGTH_START_VOICE_COMMAND;
	cmd.header.command_code  = AUDCMD_STARTVOICECOMMAND;
	cmd.header.sub_code      = 0x00;
	cmd.start_voice_command_param.keyword           = 0;
	cmd.start_voice_command_param.vad_only          = vad_only;
	cmd.start_voice_command_param.callback_function = (vad_only != 0) ? dbg_shell_voiceCommandVadOnlyCb : dbg_shell_voiceCommandCb;
	cmd.start_voice_command_param.find_command_info = &s_find_command_info;
	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult(&result);
	if(result.header.result_code == AUDRLT_STARTVOICECOMMANDCMPLT){
		DBG_ShellPrintf("Start Command Complete\n");
	} else {
		DBG_ShellPrintf("Start Command ERROR(0x%02x)\n", result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
void dbgShellStopVoiceCommand(int argc, const char *args[], void *userData)
{
	AudioCommand cmd;
	AudioResult  result;

	cmd.header.packet_length = LENGTH_STOP_VOICE_COMMAND;
	cmd.header.command_code  = AUDCMD_STOPVOICECOMMAND;
	cmd.header.sub_code      = 0x00;
	
	AS_SendAudioCommand(&cmd);
	AS_ReceiveAudioResult(&result);
	if(result.header.result_code == AUDRLT_STOPVOICECOMMANDCMPLT){
		DBG_ShellPrintf("Stop Command Complete\n");
	} else {
		DBG_ShellPrintf("Stop Command ERROR(0x%02x)\n", result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
const DBG_ShellCommandTreeNode_t mfeSubCmdTree[] = {
	{
		.name = "InitBB",
		.subCommandNumber = 0,
		{.hdlr = dbgShellInitBB},
		.simpleHelp = "init Baseband parameter",
		.specificHelp = "[usage]  $ InitBB"
	},
	{
		.name = "InitMFE",
		.subCommandNumber = 0,
		{.hdlr = dbgShellInitMFE},
		.simpleHelp = "init MFE parameter",
		.specificHelp = "[usage]  $ InitMFE {mfe_mode}"
	},
	{
		.name = "InitMPP",
		.subCommandNumber = 0,
		{.hdlr = dbgShellInitMPP},
		.simpleHelp = "init MPP parameter",
		.specificHelp = "[usage]  $ InitMPP {xloud_coef_table}"
	},
	{
		.name = "SetActive",
		.subCommandNumber = 0,
		{.hdlr = dbgShellSetActive},
		.simpleHelp = "set BBReady state",
		.specificHelp = "[usage]  $ SetActive"
	},
	{
		.name = "SetReady",
		.subCommandNumber = 0,
		{.hdlr = dbgShellSetReady},
		.simpleHelp = "set Ready state",
		.specificHelp = "[usage]  $ SetReady"
	},
	{
		.name = "StartBB",
		.subCommandNumber = 0,
		{.hdlr = dbgShellStartBB},
		.simpleHelp = "set BBActive state and start",
		.specificHelp = "[usage]  $ StartBB"
	},
	{
		.name = "StopBB",
		.subCommandNumber = 0,
		{.hdlr = dbgShellStopBB},
		.simpleHelp = "stop and set BBReady state",
		.specificHelp = "[usage]  $ StopBB"
	},
	{
		.name = "SetVolume",
		.subCommandNumber = 0,
		{.hdlr = dbgShellSetVolume},
		.simpleHelp = "setVolume state",
		.specificHelp = "[usage]  $ SetVolume"
	},
	{
		.name = "SetMpp",
		.subCommandNumber = 0,
		{.hdlr = dbgShellSetMpp},
		.simpleHelp = "setMpp state",
		.specificHelp = "[usage]  $ SetMpp"
	},
	{
		.name = "GetStatus",
		.subCommandNumber = 0,
		{.hdlr = dbgShellGetStatus},
		.simpleHelp = "get status of Audio sub-system",
		.specificHelp = "[usage]  $ GetStatus"
	},
	{
		.name = "Set_Xl_Vol",
		.subCommandNumber = 0,
		{.hdlr = dbgShellSetXLVol},
		.simpleHelp = "set xLOUD volume (range:0 to 59)",
		.specificHelp = "[usage]\n  $ Set_Xl_Vol {volume}"
	},
	{
		.name = "StartCommand",
		.subCommandNumber = 0,
		{.hdlr = dbgShellStartVoiceCommand},
		.simpleHelp = "Start Voice Command [vad_only].  0:vad+wuwsr, 1:vad_only",
		.specificHelp = "[usage]\n  $ StartCommand"
	},
	{
		.name = "StopCommand",
		.subCommandNumber = 0,
		{.hdlr = dbgShellStopVoiceCommand},
		.simpleHelp = "Stop Voice Command",
		.specificHelp = "[usage]\n  $ StopCommand"
	},
	{
		.name = "PowerOn",
		.subCommandNumber = 0,
		{.hdlr = dbgShellPowerOn},
		.simpleHelp = "Power On Command",
		.specificHelp = "[usage]\n  $ PowerOn"
	},
	{
		.name = "SetPowerOff",
		.subCommandNumber = 0,
		{.hdlr = dbgShellSetPowerOffStatus},
		.simpleHelp = "Power Off Command",
		.specificHelp = "[usage]\n  $ PowerOff"
	},
};

const DBG_ShellCommandTreeNode_t mfeCmdNode = {
	"mfe",
	sizeof(mfeSubCmdTree) / sizeof(DBG_ShellCommandTreeNode_t),
	(DBG_ShellCommandTreeNode_t *)mfeSubCmdTree,
        "Mfe Communicate Mode Sub Command\n",
	"This is specific help for category mfe_communicate\n",
	NULL
};

