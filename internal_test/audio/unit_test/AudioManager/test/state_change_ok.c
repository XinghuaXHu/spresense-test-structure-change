/* ����I������R�}���h */
/* (state�ᔽ�łȂ����)����I������p�����[�^�ŃR�}���h���s */


void cmd_initbbmode(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_INITBBMODE;
	cmd.header.packet_length = LENGTH_SUB_INITBB_ADNIOSET;
	cmd.header.sub_code = SUB_INITBB_ADNIOSET;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_INITBB);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	initbb_ReturnValue = AUDRLT_INITBBCMPLT;
	initbb_Called = 0;
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_INITBBCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���ʂ̊֐�(initbb)�ˑ��̊m�F */
	CHECK(initbb_Called == 1);
	CHECK(memcmp(&initbb_cmd,&cmd,sizeof(AudioCommand))==0); /* initbb�̈����m�F */

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_getstatus(uint8_t status, uint8_t sub_status, uint8_t vad_status)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_GETSTATUS;
	cmd.header.packet_length = LENGTH_GETSTATUS;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_GETSTATUS);
	
	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_NOTIFYSTATUS, LENGTH_AUDRLT, cmd.header.sub_code);
	AudioResult *result = (AudioResult *)(MsgLib_send_param[0]);
	CHECK(result->notify_status.status_info == status);
	CHECK(result->notify_status.sub_status_info == sub_status);
	if(sub_status == AS_NOTIFY_SUB_STATUS_INFO_WAITCOMMANDWORD )
		CHECK(result->notify_status.vad_status == vad_status);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setbbparam(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETBBPARAM;
	cmd.header.packet_length = LENGTH_SUB_SETBB_ADNIOSET;
	cmd.header.sub_code = SUB_SETBB_ADNIOSET;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETBB);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	setbb_ReturnValue = AUDRLT_SETBBPARAMCMPLT;
	setbb_Called = 0;
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_SETBBPARAMCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���ʂ̊֐�(setbb)�ˑ��̊m�F */
	CHECK(setbb_Called == 1);
	CHECK(memcmp(&setbb_cmd,&cmd,sizeof(AudioCommand))==0); /* setbb�̈����m�F */

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_initmfe(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_INITMFE;
	cmd.header.packet_length = LENGTH_INITMFE;
	cmd.header.sub_code = 0;
	cmd.init_mfe_param.config_table = 1; // NULL check���
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MFE);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_INITMFECMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_START, &cmd);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_debugmfeparam(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_DEBUGMFEPARAM;
	cmd.header.packet_length = LENGTH_DEBUGMFEPARAM;
	cmd.header.sub_code = 0;
	cmd.debug_mfe_param.mfe_config_table = 1; // NULL check���
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MFE);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check( AUDRLT_DEBUGMFECMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_DEBUG, &cmd);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setplayerstatus(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code =  AUDCMD_SETPLAYERSTATUS;
	cmd.header.packet_length =  LENGTH_SET_PLAYER_STATUS;
	cmd.header.sub_code = 0;
	cmd.debug_mfe_param.mfe_config_table = 1; // NULL check���
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETPLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	prepare_common_power_on();
	setActiveBaseband_Called = 0;
	AS_activatePlayer_Called = 0, AS_activatePlayer_ReturnValue = true;
	prepare_common_memmgr_create();
	// �֐��̌Ăяo�������`�F�b�N�������E�E�E
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_INIT, &cmd);
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ� */
	check_common_power_on();
	CHECK(setActiveBaseband_Called == 1);
	CHECK(AS_activatePlayer_Called == 1);
	check_common_memmgr_create(MEM_LAYOUT_PLAYER);

	/* object�̑���Ƀ��U���g�������b�Z�[�W���M */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_playplayer(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_PLAYPLAYER ;
	cmd.header.packet_length = LENGTH_PLAY_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_PLAY, &cmd);

	/* object�̑���Ƀ��U���g�������b�Z�[�W���M */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_PLAYCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_stopplayer(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPPLAYER ;
	cmd.header.packet_length = LENGTH_STOP_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_STOP, &cmd);

	/* object�̑���Ƀ��U���g�������b�Z�[�W���M */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STOPCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_pauseplayer(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_PAUSEPLAYER ;
	cmd.header.packet_length = LENGTH_PAUSE_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_PAUSE, &cmd);

	/* object�̑���Ƀ��U���g�������b�Z�[�W���M */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_PAUSECMPLT, LENGTH_AUDRLT, cmd.header.sub_code);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_nextplay(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_NEXTPLAY ;
	cmd.header.packet_length = LENGTH_NEXT_PLAY;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_NEXT, &cmd);

	/* object�̑���Ƀ��U���g�������b�Z�[�W���M */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_NEXTCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_prevplay(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_PREVPLAY ;
	cmd.header.packet_length =  LENGTH_PREVIOUS_PLAY;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_PREV, &cmd);

	/* object�̑���Ƀ��U���g�������b�Z�[�W���M */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_PREVCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setplayerparam(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETPLAYERPARAM ;
	cmd.header.packet_length = LENGTH_SET_PLAYER_PARAM;
	cmd.header.sub_code = 0;

	/* ���R�}���h���s�����b�Z�[�W �̊m�F */ /* ��SetPlayerParam�͍��͓����I��SetBB�̃��b�Z�[�W�����M����ăG���[���Ԃ�(�������Ȃ̂ŃG���[��Ԃ��悤�ɂ��Ă���Ƃ̂���) */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETBB);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	
#if 0
	/* ��SetPlayerParam�����������������Έȉ��̂悤�ɂȂ�͂� */
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_PLY, MSG_AUD_PLY_CMD_SET, &cmd);

	/* object�̑���Ƀ��U���g�������b�Z�[�W���M */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_PLY;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_SETPLAYERCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
#endif
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_initmpp(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_INITMPP;
	cmd.header.packet_length = LENGTH_INITMPP;
	cmd.header.sub_code = 0;
	cmd.init_mpp_param.xloud_mode = 2; //disable, NULL check���
	cmd.init_mpp_param.eax_mode = 0; //disable, NULL check���
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MPP);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_INITMPPCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_INIT, &cmd);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setmppparam(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETMPPPARAM;
	cmd.header.packet_length = LENGTH_SUB_SETMPP_XLOUD;
	cmd.header.sub_code = SUB_SETMPP_XLOUD;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MPP);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_SETMPPCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_SETPARAM, &cmd);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_debugmppparam(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_DEBUGMPPPARAM;
	cmd.header.packet_length = LENGTH_SUB_DEBUGMPP_XLOUD;
	cmd.header.sub_code = SUB_DEBUGMPP_XLOUD;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MPP);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_DEBUGMPPCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_DEBUG, &cmd);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_startbb(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STARTBB;
	cmd.header.packet_length = LENGTH_STARTBB;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SOUNDFX);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STARTBBCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_START, &cmd);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_stopbb(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPBB;
	cmd.header.packet_length = LENGTH_STOPBB;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SOUNDFX);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STOPBBCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_STOP, &cmd);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_startvoicecommand(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STARTVOICECOMMAND;
	cmd.header.packet_length = LENGTH_START_VOICE_COMMAND;
	cmd.header.sub_code = 0;
	cmd.start_voice_command_param.callback_function = (void (*)(short unsigned int, unsigned char))1; // NULL check���
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_VOICECOMMAND);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	AS_voiceRecognitionObjectStartVoiceCommand_Called = 0;
	AS_voiceRecognitionObjectStartVoiceCommand_ReturnValue = ERR_OK;
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STARTVOICECOMMANDCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���ʂ̊֐��ˑ��̊m�F */
	CHECK(AS_voiceRecognitionObjectStartVoiceCommand_Called == 1);
	CHECK(AS_voiceRecognitionObjectStartVoiceCommand_KeyWord == cmd.start_voice_command_param.keyword );
	CHECK(AS_voiceRecognitionObjectStartVoiceCommand_vad_only == cmd.start_voice_command_param.vad_only );
	CHECK(AS_voiceRecognitionObjectStartVoiceCommand_cbFunc == mng->execFindCommandCallback);
	CHECK(mng->m_findCommandCBFunc == cmd.start_voice_command_param.callback_function );
	
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_stopvoicecommand(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPVOICECOMMAND;
	cmd.header.packet_length = LENGTH_STOP_VOICE_COMMAND;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_VOICECOMMAND);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	AS_voiceRecognitionObjectStopVoiceCommand_Called = 0;
	AS_voiceRecognitionObjectStopVoiceCommand_ReturnValue = ERR_OK;
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STOPVOICECOMMANDCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���ʂ̊֐��ˑ��̊m�F */
	CHECK(AS_voiceRecognitionObjectStopVoiceCommand_Called == 1);
	
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setrecoderstatus(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code =  AUDCMD_SETRECORDERSTATUS;
	cmd.header.packet_length = LENGTH_SET_RECORDER_STATUS;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETRECORDER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	prepare_common_power_on();
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	setActiveBaseband_Called = 0;
	AS_activateVoiceRecorder_Called = 0;
	prepare_common_memmgr_create();
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_RECORDER, MSG_AUD_VOICE_RECORDER_CMD_INIT, &cmd);
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ� */
	check_common_power_on();
	CHECK(setActiveBaseband_Called == 1);
	CHECK(AS_activateVoiceRecorder_Called == 1);
	check_common_memmgr_create(MEM_LAYOUT_RECORDER);

	/* object�̑���Ƀ��U���g�������b�Z�[�W���M */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_RECORDER;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_startrecoder(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STARTREC;
	cmd.header.packet_length = LENGTH_START_RECORDER ;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_RECORDER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_RECORDER, MSG_AUD_VOICE_RECORDER_CMD_PLAY, &cmd);

	/* object�̑���Ƀ��U���g�������b�Z�[�W���M */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_RECORDER;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_RECCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_stoprecoder(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPREC;
	cmd.header.packet_length = LENGTH_STOP_RECORDER ;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_RECORDER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* ��Object�ɑ��M�������b�Z�[�W�̊m�F */
	object_message_check(MSGQ_AUD_RECORDER, MSG_AUD_VOICE_RECORDER_CMD_STOP, &cmd);

	/* object�̑���Ƀ��U���g�������b�Z�[�W���M */
	MsgLib_send_msg[1]->m_type = MSG_AUD_MGR_RST;
	MsgLib_send_msg[1]->m_reply = MSGQ_AUD_RECORDER;
	MsgLib_send_msg[1]->setParam(Wien2::AudioMngCmdCmpltResult(cmd.header.command_code, cmd.header.sub_code, true),0);
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STOPRECCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setwaitkeystatus(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETWAITKEYSTATUS;
	cmd.header.packet_length = LENGTH_SET_WAITKEY_STATUS;
	cmd.header.sub_code = 0;
	cmd.set_waitkey_status_param.mathfunc_config_table = 1; // NULL check���
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETWAIT);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	AS_voiceRecognitionObjectSetWaitKeyStatus_Called = 0;
	AS_voiceRecognitionObjectSetWaitKeyStatus_ReturnValue = ERR_OK;
	PM_AudioPowerOn_Called = 0; /* setwaitkeystatus��powerOnAudio()�����s���Ȃ��̂�PM_AudioPowerOn()��1�񂾂��Ă΂�� */
	PM_AudioPowerOn_Sequence[0] = 0;
	PM_AudioPowerOn_ReturnValue[0] = 0;
	prepare_common_memmgr_create();
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ� */
	CHECK(AS_voiceRecognitionObjectSetWaitKeyStatus_Called == 1);
	CHECK(AS_voiceRecognitionObjectSetWaitKeyStatus_cbFunc == mng->execFindTriggerCallback);
	CHECK(AS_voiceRecognitionObjectSetWaitKeyStatus_mathfuncConfigTableAddress
		== cmd.set_waitkey_status_param.mathfunc_config_table);

	CHECK(PM_AudioPowerOn_id[0] == PM_ADONIS);
	CHECK(PM_AudioPowerOn_fp[0] == NULL);
	CHECK(PM_AudioPowerOn_Called == 1);
	check_common_memmgr_create(MEM_LAYOUT_VOICETRIGGER);

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setreadystatus(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETREADYSTATUS;
	cmd.header.packet_length = LENGTH_SET_READY_STATUS;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETREADY);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
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
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ� */
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
		CHECK(PM_AudioPowerOff_id[0] == PM_ADONIS); /* WaitKeyWord����̖߂��(���ƈقȂ�)powerOffAdonis()�̂� */
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

	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setbasebandstatus(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETBASEBANDSTATUS;
	cmd.header.packet_length = LENGTH_SET_BASEBAND_STATUS;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETACTIVE);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ�(�ʂ̊֐��̖߂�l��) */
	prepare_common_power_on();
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	setActiveBaseband_Called = 0;
	AS_activateSoundEffect_Called = 0;
	AS_activateVoiceCmd_Called = 0;
	prepare_common_memmgr_create();
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	/* �R�}���h(���b�Z�[�W)�ˑ��̐ݒ� */
	CHECK(setActiveBaseband_Called == 1);
	CHECK(AS_activateSoundEffect_Called == 1);
	CHECK(AS_activateVoiceCmd_Called == 1);
	check_common_power_on();
	check_common_memmgr_create(MEM_LAYOUT_SOUNDEFFECT);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

/* ����n�̑S�Ă̏�ԑJ�ڂ��m�F */
TEST(audio_manager, cmd_succeed) {
	mng->m_SubState = Wien2::AudioManager::MNG_SUBSTATE_NONE; //���o�O[#2820]�C���܂ł̎b��Ή�

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
	cmd_setreadystatus(); //BBReady����߂�
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setbasebandstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);
	cmd_startbb();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDACTIVE,0);
	cmd_setreadystatus(); //BBActive����߂�
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setbasebandstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);
	cmd_startbb();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDACTIVE,0);
	cmd_startvoicecommand();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_WAITCOMMANDWORD,AS_NOTIFY_VAD_STATUS_OUT_OF_VOICE_SECTION);
	cmd_setreadystatus(); //WaitCommandWord����߂�
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
	cmd_playplayer(); // PlayerReady -> PlayerActive(�d��)
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYACTIVE,0);
	cmd_stopplayer(); // PlayerActive -> PlayerReady
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);
	cmd_playplayer(); // PlayerReady -> PlayerActive(�d��)
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYACTIVE,0);
	cmd_pauseplayer(); // PlayerActive -> PlayerPause(�d��)
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYPAUSE,0);
	cmd_playplayer(); // PlayerPause -> PlayerActive
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYACTIVE,0);
	cmd_setreadystatus(); //PlayerActive����߂�
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setplayerstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);
	cmd_setreadystatus(); //PlayerReady����߂�
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setplayerstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYREADY,0);
	cmd_playplayer();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYACTIVE,0);
	cmd_pauseplayer();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_PLAYER ,AS_NOTIFY_SUB_STATUS_INFO_PLAYPAUSE,0);
	cmd_setreadystatus(); //PlayerPause����߂�
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);

	cmd_setrecoderstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERREADY ,0);
	cmd_startrecoder();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERACTIVE ,0);
	cmd_stoprecoder();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERREADY ,0);
	cmd_setreadystatus(); //RecorderReady����߂�
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setrecoderstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERREADY ,0);
	cmd_startrecoder();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_RECORDER ,AS_NOTIFY_SUB_STATUS_INFO_RECORDERACTIVE ,0);
	cmd_setreadystatus(); //RecorderActive����߂�
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);

	cmd_setwaitkeystatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_WAIT_KEYWORD, AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setreadystatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
	cmd_setwaitkeystatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_WAIT_KEYWORD, AS_NOTIFY_SUB_STATUS_INFO_NONE,0);
//	cmd_setbasebandstatus(); // WaitKeyWord -> BaseBand�̑J�ڂ͌��󖢎�����
//	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);

}
