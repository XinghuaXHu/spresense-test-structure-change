/* 共通処理 */
/* メッセージ関連 の確認用 */

#include <string.h>

/* ■コマンド発行→メッセージ の確認 */
void command_send_and_check(AudioCommand &cmd, MsgType exp_message)
{
	AS_SendAudioCommand(&cmd);
	CHECK(MsgLib_send_dest[1] == MSGQ_AUD_MGR); /* 発行したコマンドに対応するメッセージ送信の確認 */
	CHECK(MsgLib_send_pri[1]  == MsgPriNormal);
	CHECK(MsgLib_send_type[1] == exp_message);
	CHECK(MsgLib_send_reply[1]== MSGQ_NULL);
	CHECK(memcmp(MsgLib_send_param[1],&cmd,sizeof(AudioCommand))==0);
}

/* ■リザルトメッセージ の確認 */
void result_message_check(uint8_t exp_result_code, uint8_t exp_packet_length, uint8_t exp_sub_code)
{
	/* □リザルトとして送信したメッセージの確認 */
	CHECK(MsgLib_send_dest[0] == MSGQ_AUD_APP); /* リザルトの確認 */
	CHECK(MsgLib_send_pri[0]  == MsgPriNormal);
	CHECK(MsgLib_send_type[0] == MSG_AUD_MGR_RST);
	CHECK(MsgLib_send_reply[0]== MSGQ_AUD_MGR);
	
	/* □リザルトの確認 */
	AudioResult *result = (AudioResult *)(MsgLib_send_param[0]);
	//fprintf(stderr,"r: %d e: %d\n", result->header.result_code, exp_result_code);
	CHECK(result->header.result_code == exp_result_code );
	CHECK(result->header.packet_length == exp_packet_length);
	CHECK(result->header.sub_code == exp_sub_code );
	
	/* AS_ReceiveAudioResult()を呼んで確認 */
	AudioResult result2;
	SYS_WaitFlag_Called = 0;
	referMsgQueBlock_Called = 0;
	MsgQueBlock_recv_ReturnValue = MsgLib_send_msg[0];
	MsgQueBlock_recv_Called = 0;
	MsgQueBlock_pop_Called = 0;
	AS_ReceiveAudioResult(&result2);
	CHECK(SYS_WaitFlag_Called == 1);
	CHECK(referMsgQueBlock_id == MSGQ_AUD_APP);
	CHECK(referMsgQueBlock_Called == 1);
	CHECK(MsgQueBlock_recv_ms == TIME_FOREVER);
	CHECK(MsgQueBlock_recv_Called == 1);
	CHECK(MsgQueBlock_pop_Called == 1);
	CHECK(result2.header.result_code == exp_result_code );
	CHECK(result2.header.packet_length == exp_packet_length);
	CHECK(result2.header.sub_code == exp_sub_code );
}

/* ■Objectに送信したメッセージの確認 */
void object_message_check(MsgQueId exp_dest, MsgType exp_message, AudioCommand *exp_cmd)
{
	/* □Objectに送信したメッセージの確認 */
	CHECK(MsgLib_send_dest[1] == exp_dest);
	CHECK(MsgLib_send_pri[1]  == MsgPriNormal);
	CHECK(MsgLib_send_type[1] == exp_message);
	CHECK(MsgLib_send_reply[1]== MSGQ_AUD_MGR);
	CHECK(memcmp(MsgLib_send_param_buf[1], exp_cmd, sizeof(AudioCommand))==0); /* メッセージヘッダのチェックは省略、payloadの確認のみ */
}

