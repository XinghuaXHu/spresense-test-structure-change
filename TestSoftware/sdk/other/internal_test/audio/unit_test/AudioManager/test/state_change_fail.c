/* ��ԑJ�ڈᔽ��fail���邱�Ƃ��m�F */
/* �p�����[�^�͐�������result�� ERROR */
/* �G���[�p�P�b�g�̒��g�̃`�F�b�N�͍��͂��Ȃ� �����󐳂�����������Ă��Ȃ�[#2821]�̂� */
/* �R�}���h�ʂ̃`�F�b�N�͏ȗ� */

void cmd_initbbmode_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_INITBBMODE;
	cmd.header.packet_length = LENGTH_SUB_INITBB_ADNIOSET;
	cmd.header.sub_code = SUB_INITBB_ADNIOSET;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_INITBB);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

/* GetStatus�͂ǂ�state�ł����s�ł���̂őΏۊO */
void cmd_getstatus_state_fail(uint8_t status, uint8_t sub_status, uint8_t vad_status)
{
}

void cmd_setbbparam_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETBBPARAM;
	cmd.header.packet_length = LENGTH_SUB_SETBB_ADNIOSET;
	cmd.header.sub_code = SUB_SETBB_ADNIOSET;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETBB);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_initmfe_state_fail(void)
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
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_debugmfeparam_state_fail(void)
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
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);
	/* result �m�F */
	result_message_check( AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setplayerstatus_state_fail(void)
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
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_playplayer_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_PLAYPLAYER ;
	cmd.header.packet_length = LENGTH_PLAY_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_stopplayer_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPPLAYER ;
	cmd.header.packet_length = LENGTH_STOP_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_pauseplayer_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_PAUSEPLAYER ;
	cmd.header.packet_length = LENGTH_PAUSE_PLAYER;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_nextplay_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_NEXTPLAY ;
	cmd.header.packet_length = LENGTH_NEXT_PLAY;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_PLAYER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_prevplay_state_fail(void)
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

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setplayerparam_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETPLAYERPARAM ;
	cmd.header.packet_length = LENGTH_SET_PLAYER_PARAM;
	cmd.header.sub_code = 0;

	/* ���R�}���h���s�����b�Z�[�W �̊m�F */ /* ��SetPlayerParam�͍��͓����I��SetBB�̃��b�Z�[�W�����M����ăG���[���Ԃ�(�������Ȃ̂ŃG���[��Ԃ��悤�ɂ��Ă���Ƃ̂���) */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETBB);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_initmpp_state_fail(void)
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
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setmppparam_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETMPPPARAM;
	cmd.header.packet_length = LENGTH_SUB_SETMPP_XLOUD;
	cmd.header.sub_code = SUB_SETMPP_XLOUD;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MPP);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_debugmppparam_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_DEBUGMPPPARAM;
	cmd.header.packet_length = LENGTH_SUB_DEBUGMPP_XLOUD;
	cmd.header.sub_code = SUB_DEBUGMPP_XLOUD;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_MPP);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_startbb_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STARTBB;
	cmd.header.packet_length = LENGTH_STARTBB;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SOUNDFX);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_stopbb_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPBB;
	cmd.header.packet_length = LENGTH_STOPBB;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SOUNDFX);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_startvoicecommand_state_fail(void)
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
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_stopvoicecommand_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPVOICECOMMAND;
	cmd.header.packet_length = LENGTH_STOP_VOICE_COMMAND;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_VOICECOMMAND);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setrecoderstatus_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code =  AUDCMD_SETRECORDERSTATUS;
	cmd.header.packet_length = LENGTH_SET_RECORDER_STATUS;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETRECORDER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_startrecoder_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STARTREC;
	cmd.header.packet_length = LENGTH_START_RECORDER ;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_RECORDER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_stoprecoder_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_STOPREC;
	cmd.header.packet_length = LENGTH_STOP_RECORDER ;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_RECORDER);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	MsgLib_send_ReturnValue[1] = ERR_OK; // object�ւ̑��M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setwaitkeystatus_state_fail(void)
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
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setreadystatus_state_fail(void)
{
	/* ���R�}���h�p�P�b�g�쐬 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETREADYSTATUS;
	cmd.header.packet_length = LENGTH_SET_READY_STATUS;
	cmd.header.sub_code = 0;
	
	/* ���R�}���h���s�����b�Z�[�W �̊m�F */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETREADY);

	/* �����b�Z�[�W���ʂ̊֐������U���g���b�Z�[�W �̊m�F */
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

void cmd_setbasebandstatus_state_fail(void)
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
	MsgLib_send_ReturnValue[0] = ERR_OK; // result���M����send�֐��̖߂�l
	/* audio manager�̃��C�����[�`��(run)�̑����parse�Ăяo�� */
	mng->parse(MsgLib_send_msg[1]);

	/* result �m�F */
	result_message_check(AUDRLT_ERRORRESPONSE, LENGTH_AUDRLT, cmd.header.sub_code);
	/* ���A�T�[�g���N���Ă��Ȃ����� */
	CHECK(!AssertionFailed);
}

TEST(audio_manager, state_fail) {
	mng->m_SubState = Wien2::AudioManager::MNG_SUBSTATE_NONE; //���o�O[#2820]�C���܂ł̎b��Ή�

	/* Ready state�Ŏ��s�ł��Ȃ��R�}���h */
	fprintf(stderr,"READY ");
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_READY,AS_NOTIFY_SUB_STATUS_INFO_NONE,0); /* ���݂�state�m�F */

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

	/* BaseBand state�Ŏ��s�ł��Ȃ��R�}���h */
	fprintf(stderr,"BASEBAND ");
	cmd_setbasebandstatus();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);

	//cmd_initbbmode_state_fail(); //���o�O[#2883]�C���܂ł̎b��Ή�
	//cmd_getstatus_state_fail();
	//cmd_setbbparam_state_fail(); //BBReady/BBActive�ł̂ݔ��s�\
	//cmd_initmfe_state_fail(); //BBready�ł̂݁Asetbasebandstatus��MFE�L�����̂�
	//cmd_debugmfeparam_state_fail(); //BBReady/BBActive�ł̂݁Asetbasebandstatus��MFE�L�����̂�
	cmd_setplayerstatus_state_fail();
	cmd_playplayer_state_fail();
	cmd_stopplayer_state_fail();
	cmd_pauseplayer_state_fail();
	cmd_nextplay_state_fail();
	cmd_prevplay_state_fail();
	cmd_setplayerparam_state_fail();
	//cmd_initmpp_state_fail(); //BBReady�̂�
	//cmd_setmppparam_state_fail(); //BBReady/BBActive�ł̂�
	//cmd_debugmppparam_state_fail();
	//cmd_startbb_state_fail(); //BBReady�̂�
	//cmd_stopbb_state_fail(); //BBActive�̂�
	//cmd_startvoicecommand_state_fail(); //BBActive�̂݁Asetbasebandstatus��VoiceCommand�L�����̂�
	//cmd_stopvoicecommand_state_fail(); //WaitCommandWord�̂�
	cmd_setrecoderstatus_state_fail();
	cmd_startrecoder_state_fail();
	cmd_stoprecoder_state_fail();
	cmd_setwaitkeystatus_state_fail();
	//cmd_setreadystatus_state_fail(); //BBActive�̂�
	cmd_setbasebandstatus_state_fail();
	
	// start/stop voice command�̂݁Asub state��audio manager�Ō��Ă���̂Ń`�F�b�N
	// BBReady��ԂŃX�^�[�g
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDREADY,0);
	cmd_startvoicecommand_state_fail(); //BBActive�̂�
	cmd_stopvoicecommand_state_fail(); //WaitCommandWord�̂�
	cmd_startbb();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_BASEBANDACTIVE,0);
	//cmd_startvoicecommand_state_fail(); //BBActive�̂�
	cmd_stopvoicecommand_state_fail(); //WaitCommandWord�̂�
	cmd_startvoicecommand();
	cmd_getstatus(AS_NOTIFY_STATUS_INFO_BASEBAND,AS_NOTIFY_SUB_STATUS_INFO_WAITCOMMANDWORD,AS_NOTIFY_VAD_STATUS_OUT_OF_VOICE_SECTION);
	cmd_startvoicecommand_state_fail(); //BBActive�̂�
	//cmd_stopvoicecommand_state_fail(); //WaitCommandWord�̂�
	
	/* Recorder state�Ŏ��s�ł��Ȃ��R�}���h */
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
	//cmd_startrecoder_state_fail(); //RecoderReady�̂�
	//cmd_stoprecoder_state_fail(); //RecoderActive�̂�
	cmd_setwaitkeystatus_state_fail();
	//cmd_setreadystatus_state_fail();
	cmd_setbasebandstatus_state_fail();

	/* Player state�Ŏ��s�ł��Ȃ��R�}���h */
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
	//cmd_playplayer_state_fail(); //PlayerReady/PlayerPause�̂�
	//cmd_stopplayer_state_fail(); //PlayerActive/PlayerPause�̂�
	//cmd_pauseplayer_state_fail(); //PlayerActive�̂�
	//cmd_nextplay_state_fail();
	//cmd_prevplay_state_fail();
	//cmd_setplayerparam_state_fail();
	//cmd_initmpp_state_fail(); //PlayerReady�̂�
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

	/* WaitKeyWord state�Ŏ��s�ł��Ȃ��R�}���h */
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
