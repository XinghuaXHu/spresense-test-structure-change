/* ���ʏ��� */
/* ���b�Z�[�W�֘A �̊m�F�p */

#include <string.h>

/* ���R�}���h���s�����b�Z�[�W �̊m�F */
void command_send_and_check(AudioCommand &cmd, MsgType exp_message)
{
	AS_SendAudioCommand(&cmd);
	CHECK(MsgLib_send_dest[1] == MSGQ_AUD_MGR); /* ���s�����R�}���h�ɑΉ����郁�b�Z�[�W���M�̊m�F */
	CHECK(MsgLib_send_pri[1]  == MsgPriNormal);
	CHECK(MsgLib_send_type[1] == exp_message);
	CHECK(MsgLib_send_reply[1]== MSGQ_NULL);
	CHECK(memcmp(MsgLib_send_param[1],&cmd,sizeof(AudioCommand))==0);
}

/* �����U���g���b�Z�[�W �̊m�F */
void result_message_check(uint8_t exp_result_code, uint8_t exp_packet_length, uint8_t exp_sub_code)
{
	/* �����U���g�Ƃ��đ��M�������b�Z�[�W�̊m�F */
	CHECK(MsgLib_send_dest[0] == MSGQ_AUD_APP); /* ���U���g�̊m�F */
	CHECK(MsgLib_send_pri[0]  == MsgPriNormal);
	CHECK(MsgLib_send_type[0] == MSG_AUD_MGR_RST);
	CHECK(MsgLib_send_reply[0]== MSGQ_AUD_MGR);
	
	/* �����U���g�̊m�F */
	AudioResult *result = (AudioResult *)(MsgLib_send_param[0]);
	//fprintf(stderr,"r: %d e: %d\n", result->header.result_code, exp_result_code);
	CHECK(result->header.result_code == exp_result_code );
	CHECK(result->header.packet_length == exp_packet_length);
	CHECK(result->header.sub_code == exp_sub_code );
	
	/* AS_ReceiveAudioResult()���Ă�Ŋm�F */
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

/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
void object_message_check(MsgQueId exp_dest, MsgType exp_message, AudioCommand *exp_cmd)
{
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	CHECK(MsgLib_send_dest[1] == exp_dest);
	CHECK(MsgLib_send_pri[1]  == MsgPriNormal);
	CHECK(MsgLib_send_type[1] == exp_message);
	CHECK(MsgLib_send_reply[1]== MSGQ_AUD_MGR);
	CHECK(memcmp(MsgLib_send_param_buf[1], exp_cmd, sizeof(AudioCommand))==0); /* ���b�Z�[�W�w�b�_�̃`�F�b�N�͏ȗ��Apayload�̊m�F�̂� */
}

