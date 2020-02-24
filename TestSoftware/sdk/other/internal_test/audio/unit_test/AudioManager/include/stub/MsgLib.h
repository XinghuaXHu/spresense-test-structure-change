/* スタブ制御/観測用(実体はスタブ側) */
extern MsgQueId MsgLib_send_dest[2];
extern MsgPri MsgLib_send_pri[2];
extern MsgType MsgLib_send_type[2];
extern MsgQueId MsgLib_send_reply[2];
extern uint8_t MsgLib_send_param_buf[2][100];
extern void *MsgLib_send_param[2];
extern err_t MsgLib_send_ReturnValue[2];

extern uint8_t MsgLib_send_msg_buf[2][100];
extern MsgPacket *MsgLib_send_msg[2];

extern int MsgLib_initFirst_Called;
extern int MsgLib_initFirst_Sequence;
extern int MsgLib_initPerCpu_Called;
extern int MsgLib_initPerCpu_Sequence;
extern int MsgQueBlock_recv_Called;
extern uint32_t MsgQueBlock_recv_ms;
extern MsgPacket *MsgQueBlock_recv_ReturnValue;
extern int MsgQueBlock_pop_Called;
extern int referMsgQueBlock_Called;
extern MsgQueId referMsgQueBlock_id;
extern MsgQueBlock referMsgQueBlock_ReturnValue;

/* 関数呼び出し順カウンタ(実体はテストコード側) */
extern int cpputest_call_sequence;

