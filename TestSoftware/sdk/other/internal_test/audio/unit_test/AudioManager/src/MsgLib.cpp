#include <stdint.h>
#include <string.h>
#include "MsgPacket.h"
#include "MsgLib.h"

extern int cpputest_call_sequence;
/* MsgLib */
int MsgLib_initFirst_Called = 0;
int MsgLib_initFirst_Sequence = 0;
void MsgLib::initFirst()
{
	MsgLib_initFirst_Called++;
	MsgLib_initFirst_Sequence = cpputest_call_sequence++;
}

int MsgLib_initPerCpu_Called = 0;
int MsgLib_initPerCpu_Sequence = 0;
void MsgLib::initPerCpu()
{
	MsgLib_initPerCpu_Called++;
	MsgLib_initPerCpu_Sequence = cpputest_call_sequence++;
}

MsgQueId MsgLib_send_dest[2];
MsgPri MsgLib_send_pri[2];
MsgType MsgLib_send_type[2];
MsgQueId MsgLib_send_reply[2];
uint8_t MsgLib_send_param_buf[2][100];
void *MsgLib_send_param[2] = {&MsgLib_send_param_buf[0],&MsgLib_send_param_buf[1]};
err_t MsgLib_send_ReturnValue[2] = {ERR_OK,ERR_OK};

uint8_t MsgLib_send_msg_buf[2][100];
MsgPacket *MsgLib_send_msg[2] = {(MsgPacket *)&MsgLib_send_msg_buf[0],(MsgPacket *)&MsgLib_send_msg_buf[1]};
/*
template<typename T>
err_t MsgLib::send(MsgQueId dest, MsgPri pri, MsgType type, MsgQueId reply, const T& param)
{
	MsgLib_send_dest = dest;
	MsgLib_send_pri = pri;
	MsgLib_send_type = type;
	MsgLib_send_reply = reply;
	MsgLib_send_param = (void *)&param;
	return MsgLib_send_ReturnValue;
}
*/
/* MsgQueBlock */
int MsgQueBlock_recv_Called = 0;
uint32_t MsgQueBlock_recv_ms;
MsgPacket *MsgQueBlock_recv_ReturnValue;
MsgPacket *MsgQueBlock::recv(uint32_t ms)
{
	MsgQueBlock_recv_Called++;
	MsgQueBlock_recv_ms = ms;
	return MsgQueBlock_recv_ReturnValue;
}

int MsgQueBlock_pop_Called = 0;
void MsgQueBlock::pop()
{
	MsgQueBlock_pop_Called++;
}

int referMsgQueBlock_Called = 0;
MsgQueId referMsgQueBlock_id;
MsgQueBlock referMsgQueBlock_ReturnValue;
MsgQueBlock& MsgLib::referMsgQueBlock(MsgQueId id)
{
	referMsgQueBlock_Called++;
	referMsgQueBlock_id = id;
	return referMsgQueBlock_ReturnValue;
}

