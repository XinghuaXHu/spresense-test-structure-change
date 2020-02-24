#ifndef MSG_LIB_H_INCLUDED
#define MSG_LIB_H_INCLUDED

#include "common_ecode.h"	/* err_t */
#include "MsgPacket.h"
//#include "MsgQueBlock.h"	/* MsgQueBlock */
class MsgQueBlock {
public:
	/* メッセージパケットの受信 */
	MsgPacket* recv(uint32_t ms);

	/* メッセージパケットの破棄 */
	void pop();


};

#define MSG_LIB_NAME	"MsgLib"
#define MSG_LIB_VER	"2.01"

/*****************************************************************
 * メッセージライブラリクラス
 *****************************************************************/
class MsgLib {
public:
	/* 全体の初期化(単一のCPUで1回だけ実行すること) */
	static void initFirst();

	/* CPU毎の初期化 */
	static void initPerCpu();

	/* 指定されたメッセージキューブロックへの参照を取得 */
	static MsgQueBlock& referMsgQueBlock(MsgQueId id);

	/* メッセージパケットの送信(非タスクコンテキスト、パラメタあり) */
	template<typename T>
	static err_t sendIsr(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const T& param) {
//		return referMsgQueBlock(dest).sendIsr(pri, type, reply, param);
		return ERR_OK;
	}
	/* メッセージパケットの送信(タスクコンテキスト、パラメタあり) */

	template<typename T>
	static err_t send(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const T& param);
	//static err_t send(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const T& param);// {
//		return referMsgQueBlock(dest).send(pri, type, reply, MsgPacket::MsgFlagWaitParam, param);
//		return ERR_OK;
//	}

	/* メッセージパケットの送信(タスクコンテキスト、アドレス範囲パラメタ) */
	static err_t send(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const void* param, size_t param_size) {
//		return referMsgQueBlock(dest).send(pri, type, reply, MsgPacket::MsgFlagWaitParam, MsgRangedParam(param, param_size));
		return ERR_OK;
	}
};


#include "stub/MsgLib.h"

template<typename T>
err_t MsgLib::send(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const T& param)
{
	int n = (dest==MSGQ_AUD_APP) ? 0 : 1; /* dest==MSGQ_AUD_APPつまりリザルト送信時は[0],それ以外(objectへの送信時)は[1] */
	/* 引数の格納 */
	MsgLib_send_dest[n] = dest;
	MsgLib_send_pri[n] = pri;
	MsgLib_send_type[n] = type;
	MsgLib_send_reply[n] = reply;
	F_ASSERT(sizeof(T)<=sizeof(MsgLib_send_param_buf[n]));
	*((T *)(&MsgLib_send_param_buf[n])) = param; /* パラメータはコピー */

	/* メッセージパケット作成 */
	MsgLib_send_msg[n]->m_type = type;
	MsgLib_send_msg[n]->m_reply = reply;
	MsgLib_send_msg[n]->setParam(param, 0);
	
	return MsgLib_send_ReturnValue[n];
}

#endif /* MSG_LIB_H_INCLUDED */
