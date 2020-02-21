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

#include <string.h>
#include <assert.h>
#include <debug/dbg_shell.h>
#include <debug/dbg_log.h>
#include <audio/high_level_api/as_high_level_api.h>
#include <FreeRTOS.h>
#include <task.h>

#define F_ASSERT(x) assert(x)
#define DBG_MODULE  DBG_MODULE_AS

#define APP_COMMAND_KEYWORD_HARO (0)
#define APP_COMMAND_VAD_WUWSR (0)
#define APP_COMMAND_VAD_ONLY (1)


/* Definition of constant */
#define TEST_COUNT	(30)

extern "C" {

extern DBG_ShellCommandTreeNode_t mfeCmdNode;

void app_main(void)
{
	int cnt;

	DBG_LOGF_DEBUG("[%s] task started\n", __func__);

	DBG_LOG_INFO("Ready Status.\n");

	DBG_ShellRegisterCommandTree(&mfeCmdNode);

	vTaskSuspend(NULL);
}

} /* extern "C"  */
