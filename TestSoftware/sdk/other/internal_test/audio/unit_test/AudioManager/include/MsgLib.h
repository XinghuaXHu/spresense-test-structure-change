#ifndef MSG_LIB_H_INCLUDED
#define MSG_LIB_H_INCLUDED

#include "common_ecode.h"	/* err_t */
#include "MsgPacket.h"
//#include "MsgQueBlock.h"	/* MsgQueBlock */
class MsgQueBlock {
public:
	/* ���b�Z�[�W�p�P�b�g�̎�M */
	MsgPacket* recv(uint32_t ms);

	/* ���b�Z�[�W�p�P�b�g�̔j�� */
	void pop();


};

#define MSG_LIB_NAME	"MsgLib"
#define MSG_LIB_VER	"2.01"

/*****************************************************************
 * ���b�Z�[�W���C�u�����N���X
 *****************************************************************/
class MsgLib {
public:
	/* �S�̂̏�����(�P���CPU��1�񂾂����s���邱��) */
	static void initFirst();

	/* CPU���̏����� */
	static void initPerCpu();

	/* �w�肳�ꂽ���b�Z�[�W�L���[�u���b�N�ւ̎Q�Ƃ��擾 */
	static MsgQueBlock& referMsgQueBlock(MsgQueId id);

	/* ���b�Z�[�W�p�P�b�g�̑��M(��^�X�N�R���e�L�X�g�A�p�����^����) */
	template<typename T>
	static err_t sendIsr(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const T& param) {
//		return referMsgQueBlock(dest).sendIsr(pri, type, reply, param);
		return ERR_OK;
	}
	/* ���b�Z�[�W�p�P�b�g�̑��M(�^�X�N�R���e�L�X�g�A�p�����^����) */

	template<typename T>
	static err_t send(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const T& param);
	//static err_t send(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const T& param);// {
//		return referMsgQueBlock(dest).send(pri, type, reply, MsgPacket::MsgFlagWaitParam, param);
//		return ERR_OK;
//	}

	/* ���b�Z�[�W�p�P�b�g�̑��M(�^�X�N�R���e�L�X�g�A�A�h���X�͈̓p�����^) */
	static err_t send(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const void* param, size_t param_size) {
//		return referMsgQueBlock(dest).send(pri, type, reply, MsgPacket::MsgFlagWaitParam, MsgRangedParam(param, param_size));
		return ERR_OK;
	}
};


#include "stub/MsgLib.h"

template<typename T>
err_t MsgLib::send(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const T& param)
{
	int n = (dest==MSGQ_AUD_APP) ? 0 : 1; /* dest==MSGQ_AUD_APP�܂胊�U���g���M����[0],����ȊO(object�ւ̑��M��)��[1] */
	/* �����̊i�[ */
	MsgLib_send_dest[n] = dest;
	MsgLib_send_pri[n] = pri;
	MsgLib_send_type[n] = type;
	MsgLib_send_reply[n] = reply;
	F_ASSERT(sizeof(T)<=sizeof(MsgLib_send_param_buf[n]));
	*((T *)(&MsgLib_send_param_buf[n])) = param; /* �p�����[�^�̓R�s�[ */

	/* ���b�Z�[�W�p�P�b�g�쐬 */
	MsgLib_send_msg[n]->m_type = type;
	MsgLib_send_msg[n]->m_reply = reply;
	MsgLib_send_msg[n]->setParam(param, 0);
	
	return MsgLib_send_ReturnValue[n];
}

#endif /* MSG_LIB_H_INCLUDED */
