/* 正常終了するコマンド */
/* (state違反でなければ)正常終了するパラメータでコマンド実行 */


void cmd_initbbmode(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_INITBBMODE;
	cmd.header.packet_length = LENGTH_SUB_INITBB_ADNIOSET;
	cmd.header.sub_code = SUB_INITBB_ADNIOSET;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_INITBB);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	initbb_ReturnValue = AUDRLT_INITBBCMPLT;
	initbb_Called = 0;
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_INITBBCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □個別の関数(initbb)依存の確認 */
	CHECK(initbb_Called == 1);
	CHECK(memcmp(&initbb_cmd,&cmd,sizeof(AudioCommand))==0); /* initbbの引数確認 */

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_getstatus(uint8_t status, uint8_t sub_status, uint8_t vad_status)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_GETSTATUS;
	cmd.header.packet_length = LENGTH_GETSTATUS;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_GETSTATUS);
	
	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_NOTIFYSTATUS, LENGTH_AUDRLT, cmd.header.sub_code);
	AudioResult *result = (AudioResult *)(MsgLib_send_param[0]);
	CHECK(result->notify_status.status_info == status);
	CHECK(result->notify_status.sub_status_info == sub_status);
	if(sub_status == AS_NOTIFY_SUB_STATUS_INFO_WAITCOMMANDWORD )
		CHECK(result->notify_status.vad_status == vad_status);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setbbparam(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETBBPARAM;
	cmd.header.packet_length = LENGTH_SUB_SETBB_ADNIOSET;
	cmd.header.sub_code = SUB_SETBB_ADNIOSET;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETBB);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	setbb_ReturnValue = AUDRLT_SETBBPARAMCMPLT;
	setbb_Called = 0;
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_SETBBPARAMCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □個別の関数(setbb)依存の確認 */
	CHECK(setbb_Called == 1);
	CHECK(memcmp(&setbb_cmd,&cmd,sizeof(AudioCommand))==0); /* setbbの引数確認 */

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_initmfe(void)
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
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_INITMFECMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_START, &cmd);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_debugmfeparam(void)
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
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check( AUDRLT_DEBUGMFECMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_DEBUG, &cmd);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setplayerstatus(void)
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
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	prepare_common_power_on();
	setActiveBaseband_Called = 0;
	AS_activatePlayer_Called = 0, AS_activatePlayer_ReturnValue = true;
	prepare_common_memmgr_create();
	// 関数の呼び出し順もチェックしたい・・・
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_INIT, &cmd);
	/* コマンド(メッセージ)依存の設定 */
	check_common_power_on();
	CHECK(setActiveBaseband_Called == 1);
	CHECK(AS_activatePlayer_Called == 1);
	check_common_memmgr_create(MEM_LAYOUT_PLAYER);

	/* objectの代わりにリザルト生成メッセージ送信 */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_playplayer(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_PLAYPLAYER ;
	cmd.header.packet_length = LENGTH_PLAY_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_PLAY, &cmd);

	/* objectの代わりにリザルト生成メッセージ送信 */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_PLAYCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_stopplayer(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPPLAYER ;
	cmd.header.packet_length = LENGTH_STOP_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_STOP, &cmd);

	/* objectの代わりにリザルト生成メッセージ送信 */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STOPCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_pauseplayer(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_PAUSEPLAYER ;
	cmd.header.packet_length = LENGTH_PAUSE_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_PAUSE, &cmd);

	/* objectの代わりにリザルト生成メッセージ送信 */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_PAUSECMPLT, LENGTH_AUDRLT, cmd.header.sub_code);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_nextplay(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_NEXTPLAY ;
	cmd.header.packet_length = LENGTH_NEXT_PLAY;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_NEXT, &cmd);

	/* objectの代わりにリザルト生成メッセージ送信 */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_NEXTCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_prevplay(void)
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
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_PREV, &cmd);

	/* objectの代わりにリザルト生成メッセージ送信 */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_PREVCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setplayerparam(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETPLAYERPARAM ;
	cmd.header.packet_length = LENGTH_SET_PLAYER_PARAM;
	cmd.header.sub_code = 0;

	/* ■コマンド発行→メッセージ の確認 */ /* ★SetPlayerParamは今は内部的にSetBBのメッセージが送信されてエラーが返る(未実装なのでエラーを返すようにしているとのこと) */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETBB);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	
#if 0
	/* ★SetPlayerParamが正しく実装されれば以下のようになるはず */
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_SET, &cmd);

	/* objectの代わりにリザルト生成メッセージ送信 */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_SETPLAYERCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
#endif
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_initmpp(void)
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
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_INITMPPCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_INIT, &cmd);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setmppparam(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETMPPPARAM;
	cmd.header.packet_length = LENGTH_SUB_SETMPP_XLOUD;
	cmd.header.sub_code = SUB_SETMPP_XLOUD;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MPP);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_SETMPPCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_SETPARAM, &cmd);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_debugmppparam(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_DEBUGMPPPARAM;
	cmd.header.packet_length = LENGTH_SUB_DEBUGMPP_XLOUD;
	cmd.header.sub_code = SUB_DEBUGMPP_XLOUD;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MPP);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_DEBUGMPPCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_DEBUG, &cmd);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_startbb(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STARTBB;
	cmd.header.packet_length = LENGTH_STARTBB;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SOUNDFX);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STARTBBCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_START, &cmd);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_stopbb(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPBB;
	cmd.header.packet_length = LENGTH_STOPBB;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SOUNDFX);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STOPBBCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_STOP, &cmd);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_startvoicecommand(void)
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
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	AS_voiceRecognitionObjectStartVoiceCommand_Called = 0;
	AS_voiceRecognitionObjectStartVoiceCommand_ReturnValue = ERR_OK;
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STARTVOICECOMMANDCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □個別の関数依存の確認 */
	CHECK(AS_voiceRecognitionObjectStartVoiceCommand_Called == 1);
	CHECK(AS_voiceRecognitionObjectStartVoiceCommand_KeyWord == cmd.start_voice_command_param.keyword );
	CHECK(AS_voiceRecognitionObjectStartVoiceCommand_vad_only == cmd.start_voice_command_param.vad_only );
	CHECK(AS_voiceRecognitionObjectStartVoiceCommand_cbFunc == mng->execFindCommandCallback);
	CHECK(mng->m_findCommandCBFunc == cmd.start_voice_command_param.callback_function );
	
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_stopvoicecommand(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPVOICECOMMAND;
	cmd.header.packet_length = LENGTH_STOP_VOICE_COMMAND;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_VOICECOMMAND);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	AS_voiceRecognitionObjectStopVoiceCommand_Called = 0;
	AS_voiceRecognitionObjectStopVoiceCommand_ReturnValue = ERR_OK;
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STOPVOICECOMMANDCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □個別の関数依存の確認 */
	CHECK(AS_voiceRecognitionObjectStopVoiceCommand_Called == 1);
	
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setrecoderstatus(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code =  AUDCMD_SETRECORDERSTATUS;
	cmd.header.packet_length = LENGTH_SET_RECORDER_STATUS;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETRECORDER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	prepare_common_power_on();
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	setActiveBaseband_Called = 0;
	AS_activateVoiceRecorder_Called = 0;
	prepare_common_memmgr_create();
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_RECORDER, MSG_AUD_VOICE_RECORDER_CMD_INIT, &cmd);
	/* コマンド(メッセージ)依存の設定 */
	check_common_power_on();
	CHECK(setActiveBaseband_Called == 1);
	CHECK(AS_activateVoiceRecorder_Called == 1);
	check_common_memmgr_create(MEM_LAYOUT_RECORDER);

	/* objectの代わりにリザルト生成メッセージ送信 */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_RECORDER;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_startrecoder(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STARTREC;
	cmd.header.packet_length = LENGTH_START_RECORDER ;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_RECORDER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_RECORDER, MSG_AUD_VOICE_RECORDER_CMD_PLAY, &cmd);

	/* objectの代わりにリザルト生成メッセージ送信 */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_RECORDER;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_RECCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_stoprecoder(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPREC;
	cmd.header.packet_length = LENGTH_STOP_RECORDER ;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_RECORDER);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	MsgLib_send_ReturnValue[1] = ERR_OK; // objectへの送信時のsend関数の戻り値
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_RECORDER, MSG_AUD_VOICE_RECORDER_CMD_STOP, &cmd);

	/* objectの代わりにリザルト生成メッセージ送信 */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_RECORDER;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STOPRECCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setwaitkeystatus(void)
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
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	AS_voiceRecognitionObjectSetWaitKeyStatus_Called = 0;
	AS_voiceRecognitionObjectSetWaitKeyStatus_ReturnValue = ERR_OK;
	PM_AudioPowerOn_Called = 0; /* setwaitkeystatusはpowerOnAudio()を実行しないのでPM_AudioPowerOn()は1回だけ呼ばれる */
	PM_AudioPowerOn_Sequence[0] = 0;
	PM_AudioPowerOn_ReturnValue[0] = 0;
	prepare_common_memmgr_create();
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	/* コマンド(メッセージ)依存の設定 */
	CHECK(AS_voiceRecognitionObjectSetWaitKeyStatus_Called == 1);
	CHECK(AS_voiceRecognitionObjectSetWaitKeyStatus_cbFunc == mng->execFindTriggerCallback);
	CHECK(AS_voiceRecognitionObjectSetWaitKeyStatus_mathfuncConfigTableAddress
		== cmd.set_waitkey_status_param.mathfunc_config_table);

	CHECK(PM_AudioPowerOn_id[0] == PM_ADONIS);
	CHECK(PM_AudioPowerOn_fp[0] == NULL);
	CHECK(PM_AudioPowerOn_Called == 1);
	check_common_memmgr_create(MEM_LAYOUT_VOICETRIGGER);

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setreadystatus(void)
{
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETREADYSTATUS;
	cmd.header.packet_length = LENGTH_SET_READY_STATUS;
	cmd.header.sub_code = 0;
	
	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETREADY);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* コマンド(メッセージ)依存の設定(個別の関数の戻り値等) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	AS_deactivateSoundEffect_Called = 0;
	AS_deactivateVoiceCmd_Called = 0;
	AS_voiceRecognitionObjectSetReadyStatus_Called = 0;
	AS_deactivatePlayer_Called = 0;
	AS_deactivateVoiceRecorder_Called = 0;
	powerOff_Called = 0;
	powerOff_ReturnValue = AUDRLT_STATUSCHANGED;
	prepare_common_power_off();
	prepare_common_memmgr_destroy();
	int state = mng->m_State;
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	/* コマンド(メッセージ)依存の設定 */
	switch(state){
	case AS_NOTIFY_STATUS_INFO_BASEBAND:
		CHECK(AS_deactivateSoundEffect_Called == 1);
		CHECK(AS_deactivateVoiceCmd_Called == 1);
		CHECK(AS_voiceRecognitionObjectSetReadyStatus_Called == 0);
		CHECK(AS_deactivatePlayer_Called == 0);
		CHECK(AS_deactivateVoiceRecorder_Called == 0);
		CHECK(powerOff_Called == 1);
		check_common_power_off();
		check_common_memmgr_destroy();
		break;
	case AS_NOTIFY_STATUS_INFO_WAIT_KEYWORD:
		CHECK(AS_deactivateSoundEffect_Called == 0);
		CHECK(AS_deactivateVoiceCmd_Called == 0);
		CHECK(AS_voiceRecognitionObjectSetReadyStatus_Called == 1);
		CHECK(AS_deactivatePlayer_Called == 0);
		CHECK(AS_deactivateVoiceRecorder_Called == 0);
		CHECK(PM_AudioPowerOff_id[0] == PM_ADONIS); /* WaitKeyWordからの戻りは(他と異なり)powerOffAdonis()のみ */
		CHECK(PM_AudioPowerOff_fp[0] == NULL);
		check_common_memmgr_destroy();
		break;
	case AS_NOTIFY_STATUS_INFO_PLAYER:
		CHECK(AS_deactivateSoundEffect_Called == 0);
		CHECK(AS_deactivateVoiceCmd_Called == 0);
		CHECK(AS_voiceRecognitionObjectSetReadyStatus_Called == 0);
		CHECK(AS_deactivatePlayer_Called == 1);
		CHECK(AS_deactivateVoiceRecorder_Called == 0);
		CHECK(powerOff_Called == 1);
		check_common_power_off();
		check_common_memmgr_destroy();
		break;
	case AS_NOTIFY_STATUS_INFO_RECORDER:
		CHECK(AS_deactivateSoundEffect_Called == 0);
		CHECK(AS_deactivateVoiceCmd_Called == 0);
		CHECK(AS_voiceRecognitionObjectSetReadyStatus_Called == 0);
		CHECK(AS_deactivatePlayer_Called == 0);
		CHECK(AS_deactivateVoiceRecorder_Called == 1);
		CHECK(powerOff_Called == 1);
		check_common_power_off();
		check_common_memmgr_destroy();
		break;
	case AS_NOTIFY_STATUS_INFO_READY:
	default:
		FAIL("unexpected state");
	}

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

void cmd_setbasebandstatus(void)
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
	prepare_common_power_on();
	MsgLib_send_ReturnValue[0] = ERR_OK; // result送信時のsend関数の戻り値
	setActiveBaseband_Called = 0;
	AS_activateSoundEffect_Called = 0;
	AS_activateVoiceCmd_Called = 0;
	prepare_common_memmgr_create();
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	/* コマンド(メッセージ)依存の設定 */
	CHECK(setActiveBaseband_Called == 1);
	CHECK(AS_activateSoundEffect_Called == 1);
	CHECK(AS_activateVoiceCmd_Called == 1);
	check_common_power_on();
	check_common_memmgr_create(MEM_LAYOUT_SOUNDEFFECT);
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

/* 正常系の全ての状態遷移を確認 */
TEST(audio_manager, cmd_succeed) {
	mng->m_SubState = Wien2::AudioManager::MNG_SUBSTATE_NONE; //★バグ[#2820]修正までの暫定対応

	cmd_initbbmode();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);

	cmd_setbasebandstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);
	cmd_setbbparam();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);
	cmd_startbb();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDACTIVE,0);
	cmd_setbbparam();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDACTIVE,0);
	cmd_startvoicecommand();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_WAITCOMMANDWORD,AS_NOTIFY_VAD_STATUS_OUT_OF_VOICE_SECTION);
	cmd_setbbparam();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_WAITCOMMANDWORD,AS_NOTIFY_VAD_STATUS_OUT_OF_VOICE_SECTION);
	cmd_stopvoicecommand();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDACTIVE,0);
	cmd_setbbparam();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDACTIVE,0);
	cmd_stopbb();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);
	cmd_setbbparam();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);
	cmd_setreadystatus(); //BBReadyから戻り
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setbasebandstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);
	cmd_startbb();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDACTIVE,0);
	cmd_setreadystatus(); //BBActiveから戻り
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setbasebandstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);
	cmd_startbb();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDACTIVE,0);
	cmd_startvoicecommand();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_WAITCOMMANDWORD,AS_NOTIFY_VAD_STATUS_OUT_OF_VOICE_SECTION);
	cmd_setreadystatus(); //WaitCommandWordから戻り
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);

	cmd_setplayerstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);
	cmd_setbbparam();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);
	cmd_playplayer(); // PlayerReady -> PlayerActive
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYACTIVE,0);
	cmd_setbbparam();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYACTIVE,0);
	cmd_pauseplayer(); // PlayerActive -> PlayerPause
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYPAUSE,0);
	cmd_setbbparam();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYPAUSE,0);
	cmd_stopplayer(); // PlayerPause -> PlayerReady
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);
	cmd_setbbparam();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);
	cmd_playplayer(); // PlayerReady -> PlayerActive(重複)
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYACTIVE,0);
	cmd_stopplayer(); // PlayerActive -> PlayerReady
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);
	cmd_playplayer(); // PlayerReady -> PlayerActive(重複)
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYACTIVE,0);
	cmd_pauseplayer(); // PlayerActive -> PlayerPause(重複)
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYPAUSE,0);
	cmd_playplayer(); // PlayerPause -> PlayerActive
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYACTIVE,0);
	cmd_setreadystatus(); //PlayerActiveから戻り
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setplayerstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);
	cmd_setreadystatus(); //PlayerReadyから戻り
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setplayerstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);
	cmd_playplayer();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYACTIVE,0);
	cmd_pauseplayer();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYPAUSE,0);
	cmd_setreadystatus(); //PlayerPauseから戻り
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);

	cmd_setrecoderstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERREADY ,0);
	cmd_startrecoder();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERACTIVE ,0);
	cmd_stoprecoder();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERREADY ,0);
	cmd_setreadystatus(); //RecorderReadyから戻り
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setrecoderstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERREADY ,0);
	cmd_startrecoder();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERACTIVE ,0);
	cmd_setreadystatus(); //RecorderActiveから戻り
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);

	cmd_setwaitkeystatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_WAIT_KEYWORD, AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setreadystatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setwaitkeystatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_WAIT_KEYWORD, AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
//	cmd_setbasebandstatus(); // WaitKeyWord -> BaseBandの遷移は現状未実装★
//	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);

}
