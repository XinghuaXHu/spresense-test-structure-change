/* スタブ制御/観測用(実体はスタブ側) */
extern int asAdo_PowerOnMicBiasA_Called;
extern E_AS asAdo_PowerOnMicBiasA_ReturnValue;
extern int asAdo_PowerOffMicBiasA_Called;
extern E_AS asAdo_PowerOffMicBiasA_ReturnValue;
extern int powerOff_Called;
extern uint32_t powerOff_ReturnValue;
extern int setActiveBaseband_Called;
extern uint32_t setActiveBaseband_ReturnValue;
extern int initbb_Called;
extern AudioCommand initbb_cmd;
extern uint32_t initbb_ReturnValue;
extern int setbb_Called;
extern AudioCommand setbb_cmd;
extern uint32_t setbb_ReturnValue;
extern int clearBasebandInitFlag_Called;

/* 関数呼び出し順カウンタ(実体はテストコード側) */
extern int cpputest_call_sequence;

