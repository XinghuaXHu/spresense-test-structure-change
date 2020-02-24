/* 状態遷移違反でfailすることを確認 */
/* パラメータは正しいがresultが ERROR */
/* エラーパケットの中身のチェックは今はしない ★現状正しく実装されていない[#2821]ので */
/* コマンド個別のチェックは省略 */

void cmd_initbbmode_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_INITBBMODE;
	cmd.header.packet_length = LENGTH_SUB_INITBB_ADNIOSET;
	cmd.header.sub_code = SUB_INITBB_ADNIOSET;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_INITBB);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

/* GetStatusはどのstateでも発行できるので対象外 */
void cmd_getstatus_state_fail(uint8_t status, uint8_t sub_status, uint8_t vad_status)
{
}

void cmd_setbbparam_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETBBPARAM;
	cmd.header.packet_length = LENGTH_SUB_SETBB_ADNIOSET;
	cmd.header.sub_code = SUB_SETBB_ADNIOSET;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETBB);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_initmfe_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_INITMFE;
	cmd.header.packet_length = LENGTH_INITMFE;
	cmd.header.sub_code = 0;
	cmd.init_mfe_param.config_table = 1; // NULL check回避
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MFE);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_debugmfeparam_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_DEBUGMFEPARAM;
	cmd.header.packet_length = LENGTH_DEBUGMFEPARAM;
	cmd.header.sub_code = 0;
	cmd.debug_mfe_param.mfe_config_table = 1; // NULL check回避
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MFE);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check( AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setplayerstatus_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code =  AUDCMD_SETPLAYERSTATUS;
	cmd.header.packet_length =  LENGTH_SET_PLAYER_STATUS;
	cmd.header.sub_code = 0;
	cmd.debug_mfe_param.mfe_config_table = 1; // NULL check回避
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETPLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_playplayer_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_PLAYPLAYER ;
	cmd.header.packet_length = LENGTH_PLAY_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_stopplayer_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPPLAYER ;
	cmd.header.packet_length = LENGTH_STOP_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_pauseplayer_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_PAUSEPLAYER ;
	cmd.header.packet_length = LENGTH_PAUSE_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_nextplay_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_NEXTPLAY ;
	cmd.header.packet_length = LENGTH_NEXT_PLAY;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_prevplay_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_PREVPLAY ;
	cmd.header.packet_length =  LENGTH_PREVIOUS_PLAY;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setplayerparam_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETPLAYERPARAM ;
	cmd.header.packet_length = LENGTH_SET_PLAYER_PARAM;
	cmd.header.sub_code = 0;

	/* ■コマンド発行→メッセージ の確認 */ /* ★SetPlayerParamは今は内部的にSetBBのメッセージが送信されてエラーが返る(未実装なのでエラーを返すようにしているとのこと) */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETBB);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_initmpp_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_INITMPP;
	cmd.header.packet_length = LENGTH_INITMPP;
	cmd.header.sub_code = 0;
	cmd.init_mpp_param.xloud_mode = 2; //disable, NULL check回避
	cmd.init_mpp_param.eax_mode = 0; //disable, NULL check回避
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MPP);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setmppparam_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETMPPPARAM;
	cmd.header.packet_length = LENGTH_SUB_SETMPP_XLOUD;
	cmd.header.sub_code = SUB_SETMPP_XLOUD;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MPP);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_debugmppparam_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_DEBUGMPPPARAM;
	cmd.header.packet_length = LENGTH_SUB_DEBUGMPP_XLOUD;
	cmd.header.sub_code = SUB_DEBUGMPP_XLOUD;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MPP);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_startbb_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STARTBB;
	cmd.header.packet_length = LENGTH_STARTBB;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SOUNDFX);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_stopbb_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPBB;
	cmd.header.packet_length = LENGTH_STOPBB;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SOUNDFX);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_startvoicecommand_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STARTVOICECOMMAND;
	cmd.header.packet_length = LENGTH_START_VOICE_COMMAND;
	cmd.header.sub_code = 0;
	cmd.start_voice_command_param.callback_function = (void (*)(short unsigned int, unsigned char))1; // NULL check回避
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_VOICECOMMAND);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_stopvoicecommand_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPVOICECOMMAND;
	cmd.header.packet_length = LENGTH_STOP_VOICE_COMMAND;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_VOICECOMMAND);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setrecoderstatus_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code =  AUDCMD_SETRECORDERSTATUS;
	cmd.header.packet_length = LENGTH_SET_RECORDER_STATUS;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETRECORDER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_startrecoder_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STARTREC;
	cmd.header.packet_length = LENGTH_START_RECORDER ;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_RECORDER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_stoprecoder_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPREC;
	cmd.header.packet_length = LENGTH_STOP_RECORDER ;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_RECORDER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setwaitkeystatus_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETWAITKEYSTATUS;
	cmd.header.packet_length = LENGTH_SET_WAITKEY_STATUS;
	cmd.header.sub_code = 0;
	cmd.set_waitkey_status_param.mathfunc_config_table = 1; // NULL check回避
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETWAIT);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setreadystatus_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETREADYSTATUS;
	cmd.header.packet_length = LENGTH_SET_READY_STATUS;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETREADY);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setbasebandstatus_state_fail(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETBASEBANDSTATUS;
	cmd.header.packet_length = LENGTH_SET_BASEBAND_STATUS;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETACTIVE);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

TEST(audio_manager, state_fail) {
	mng->m_SubState = Wien2::AudioManager::MNG_SUBSTATE_NONE; //★バグ[#2820]修正までの暫定対応

	/* Ready stateで実行できないコマンド */
	fprintf(stderr,"READY ");
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0); /* 現在のstate確認 */

	//cmd_initbbmode_state_fail();
	//cmd_getstatus_state_fail();
	cmd_setbbparam_state_fail();
	cmd_initmfe_state_fail();
	cmd_debugmfeparam_state_fail();
	//cmd_setplayerstatus_state_fail();
	cmd_playplayer_state_fail();
	cmd_stopplayer_state_fail();
	cmd_pauseplayer_state_fail();
	cmd_nextplay_state_fail();
	cmd_prevplay_state_fail();
	cmd_setplayerparam_state_fail();
	cmd_initmpp_state_fail();
	cmd_setmppparam_state_fail();
	cmd_debugmppparam_state_fail();
	cmd_startbb_state_fail();
	cmd_stopbb_state_fail();
	cmd_startvoicecommand_state_fail();
	cmd_stopvoicecommand_state_fail();
	//cmd_setrecoderstatus_state_fail();
	cmd_startrecoder_state_fail();
	cmd_stoprecoder_state_fail();
	//cmd_setwaitkeystatus_state_fail();
	cmd_setreadystatus_state_fail();
	//cmd_setbasebandstatus_state_fail();

	/* BaseBand stateで実行できないコマンド */
	fprintf(stderr,"BASEBAND ");
	cmd_setbasebandstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);

	//cmd_initbbmode_state_fail(); //★バグ[#2883]修正までの暫定対応
	//cmd_getstatus_state_fail();
	//cmd_setbbparam_state_fail(); //BBReady/BBActiveでのみ発行可能
	//cmd_initmfe_state_fail(); //BBreadyでのみ、setbasebandstatusでMFE有効時のみ
	//cmd_debugmfeparam_state_fail(); //BBReady/BBActiveでのみ、setbasebandstatusでMFE有効時のみ
	cmd_setplayerstatus_state_fail();
	cmd_playplayer_state_fail();
	cmd_stopplayer_state_fail();
	cmd_pauseplayer_state_fail();
	cmd_nextplay_state_fail();
	cmd_prevplay_state_fail();
	cmd_setplayerparam_state_fail();
	//cmd_initmpp_state_fail(); //BBReadyのみ
	//cmd_setmppparam_state_fail(); //BBReady/BBActiveでのみ
	//cmd_debugmppparam_state_fail();
	//cmd_startbb_state_fail(); //BBReadyのみ
	//cmd_stopbb_state_fail(); //BBActiveのみ
	//cmd_startvoicecommand_state_fail(); //BBActiveのみ、setbasebandstatusでVoiceCommand有効時のみ
	//cmd_stopvoicecommand_state_fail(); //WaitCommandWordのみ
	cmd_setrecoderstatus_state_fail();
	cmd_startrecoder_state_fail();
	cmd_stoprecoder_state_fail();
	cmd_setwaitkeystatus_state_fail();
	//cmd_setreadystatus_state_fail(); //BBActiveのみ
	cmd_setbasebandstatus_state_fail();
	
	// start/stop voice commandのみ、sub stateをaudio managerで見ているのでチェック
	// BBReady状態でスタート
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);
	cmd_startvoicecommand_state_fail(); //BBActiveのみ
	cmd_stopvoicecommand_state_fail(); //WaitCommandWordのみ
	cmd_startbb();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDACTIVE,0);
	//cmd_startvoicecommand_state_fail(); //BBActiveのみ
	cmd_stopvoicecommand_state_fail(); //WaitCommandWordのみ
	cmd_startvoicecommand();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_WAITCOMMANDWORD,AS_NOTIFY_VAD_STATUS_OUT_OF_VOICE_SECTION);
	cmd_startvoicecommand_state_fail(); //BBActiveのみ
	//cmd_stopvoicecommand_state_fail(); //WaitCommandWordのみ
	
	/* Recorder stateで実行できないコマンド */
	fprintf(stderr,"RECORDER ");
	cmd_setreadystatus();
	cmd_setrecoderstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERREADY ,0);

	cmd_initbbmode_state_fail();
	//cmd_getstatus_state_fail();
	cmd_setbbparam_state_fail();
	cmd_initmfe_state_fail();
	cmd_debugmfeparam_state_fail();
	cmd_setplayerstatus_state_fail();
	cmd_playplayer_state_fail();
	cmd_stopplayer_state_fail();
	cmd_pauseplayer_state_fail();
	cmd_nextplay_state_fail();
	cmd_prevplay_state_fail();
	cmd_setplayerparam_state_fail();
	cmd_initmpp_state_fail();
	cmd_setmppparam_state_fail();
	cmd_debugmppparam_state_fail();
	cmd_startbb_state_fail();
	cmd_stopbb_state_fail();
	cmd_startvoicecommand_state_fail();
	cmd_stopvoicecommand_state_fail();
	cmd_setrecoderstatus_state_fail();
	//cmd_startrecoder_state_fail(); //RecoderReadyのみ
	//cmd_stoprecoder_state_fail(); //RecoderActiveのみ
	cmd_setwaitkeystatus_state_fail();
	//cmd_setreadystatus_state_fail();
	cmd_setbasebandstatus_state_fail();

	/* Player stateで実行できないコマンド */
	fprintf(stderr,"PLAYER ");
	cmd_setreadystatus();
	cmd_setplayerstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);

	cmd_initbbmode_state_fail();
	//cmd_getstatus_state_fail();
	//cmd_setbbparam_state_fail();
	cmd_initmfe_state_fail();
	cmd_debugmfeparam_state_fail();
	cmd_setplayerstatus_state_fail();
	//cmd_playplayer_state_fail(); //PlayerReady/PlayerPauseのみ
	//cmd_stopplayer_state_fail(); //PlayerActive/PlayerPauseのみ
	//cmd_pauseplayer_state_fail(); //PlayerActiveのみ
	//cmd_nextplay_state_fail();
	//cmd_prevplay_state_fail();
	//cmd_setplayerparam_state_fail();
	//cmd_initmpp_state_fail(); //PlayerReadyのみ
	//cmd_setmppparam_state_fail();
	//cmd_debugmppparam_state_fail();
	cmd_startbb_state_fail();
	cmd_stopbb_state_fail();
	cmd_startvoicecommand_state_fail();
	cmd_stopvoicecommand_state_fail();
	cmd_setrecoderstatus_state_fail();
	cmd_startrecoder_state_fail();
	cmd_stoprecoder_state_fail();
	cmd_setwaitkeystatus_state_fail();
	//cmd_setreadystatus_state_fail();
	cmd_setbasebandstatus_state_fail();

	/* WaitKeyWord stateで実行できないコマンド */
	fprintf(stderr,"WAITKEYWORD ");
	cmd_setreadystatus();
	cmd_setwaitkeystatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_WAIT_KEYWORD, AS_NOTIFY_SUB_STATUS_INFO_NONE,0);

	cmd_initbbmode_state_fail();
	//cmd_getstatus_state_fail();
	cmd_setbbparam_state_fail();
	cmd_initmfe_state_fail();
	cmd_debugmfeparam_state_fail();
	cmd_setplayerstatus_state_fail();
	cmd_playplayer_state_fail();
	cmd_stopplayer_state_fail();
	cmd_pauseplayer_state_fail();
	cmd_nextplay_state_fail();
	cmd_prevplay_state_fail();
	cmd_setplayerparam_state_fail();
	cmd_initmpp_state_fail();
	cmd_setmppparam_state_fail();
	cmd_debugmppparam_state_fail();
	cmd_startbb_state_fail();
	cmd_stopbb_state_fail();
	cmd_startvoicecommand_state_fail();
	cmd_stopvoicecommand_state_fail();
	cmd_setrecoderstatus_state_fail();
	cmd_startrecoder_state_fail();
	cmd_stoprecoder_state_fail();
	cmd_setwaitkeystatus_state_fail();
	//cmd_setreadystatus_state_fail();
	cmd_setbasebandstatus_state_fail();

}
