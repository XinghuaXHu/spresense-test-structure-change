#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* テストは必ず1つのテストグループに属します。
 * テストグループはTEST_GROUPマクロで定義します。
 */

#include "sdk_message_types.h"
#include "../include/msgq_id.h"
#include "../include/MsgPacket.h"
#include "../include/MsgLib.h"
#include "common_ecode.h"	/* err_t */
#include <audio/high_level_api/as_high_level_api.h>
#include "../include/chateau_osal.h"
#include "audio_manager.h"
#include "../include/mem_layout.h"
#include "audio_layout.h"

/* 関数呼び出し順カウンタ */
int cpputest_call_sequence = 0;

/* stub変数のextern宣言 */
#include "stub/assertion.h"
#include "stub/audio_io_config.h"
#include "stub/baseband.h"
#include "stub/MemMgr.h"
#include "stub/MsgLib.h"
#include "stub/player.h"
#include "stub/pm.h"
#include "stub/recognition.h"
#include "stub/recorder.h"
#include "stub/sound_effect.h"
#include "stub/system.h"


Wien2::AudioManager *mng;

TEST_GROUP(audio_manager) {
//	Wien2::AudioManager *mng;
	TEST_SETUP() {
		mng  = new Wien2::AudioManager(MSGQ_AUD_MGR, MSGQ_AUD_PLY);
		AssertionFailed = 0;
		cpputest_call_sequence = 0;
	}
	TEST_TEARDOWN() {
		delete mng;
	}
};


/* テストコード(環境動作確認用サンプル) */

/* 共通ルーチン */
#include "common_message.c"
#include "common_power.c"
#include "common_memmgr.c"

/* 全ての許容された状態遷移を確認(コマンドのパラメータは振らない) */
#include "state_change_ok.c"

/* 全ての許容されない状態遷移を確認(コマンドのパラメータは振らない) */
/* 内部でstate_change_ok.cで定義した関数を使うのでstate_change_ok.cより後にincludeする */
#include "state_change_fail.c"

/* Entryからrunまでのパスの確認 */
extern "C" void AS_AudioManagerEntry(void *);
TEST(audio_manager, entry) {
	
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_INITBBMODE;
	cmd.header.packet_length = LENGTH_SUB_INITBB_ADNIOSET;
	cmd.header.sub_code = SUB_INITBB_ADNIOSET;
	/* スタブが返すメッセージを先に作っておく */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_INITBB);
	MsgQueBlock_recv_ReturnValue = MsgLib_send_msg[1];

	/* スタブが返すリザルト */
	initbb_ReturnValue = AUDRLT_INITBBCMPLT;
	
	int tmp;
	PM_RamControlByAddress_Called = 0;
	PM_RamControlByAddress_ReturnValue = 0;
	MsgLib_initFirst_Called = 0;
	MsgLib_initPerCpu_Called = 0;
	MemMgrLite::translatePoolAddrToVa_Called = 0;
	MemMgrLite::translatePoolAddrToVa_ReturnValue = (void *)&tmp;
	MemMgrLite::initFirst_Called = 0;
	MemMgrLite::initPerCpu_Called = 0;
	SYS_SetFlag_Called = 0;
	/* enrty -> runまで実行 */
	AS_AudioManagerEntry(NULL);

	/* result 確認 */
	result_message_check(AUDRLT_INITBBCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □個別の関数(initbb)依存の確認 */
	CHECK(initbb_Called == 1);
	CHECK(memcmp(&initbb_cmd,&cmd,sizeof(AudioCommand))==0); /* ★initbbの引数確認 */

	/* 初期化シーケンス確認 */
	/* AS_CreateAudioManager()からのcall */
	CHECK(PM_RamControlByAddress_Called == 1);
	CHECK(PM_RamControlByAddress_address == 0x0d100000);
	CHECK(PM_RamControlByAddress_size == 512*1024-1);
	CHECK(PM_RamControlByAddress_status == PM_SRAM_ON);
	CHECK(PM_RamControlByAddress_fp == NULL);
	CHECK(MsgLib_initFirst_Called == 1);
	CHECK(MsgLib_initPerCpu_Called == 1);
	CHECK(MemMgrLite::translatePoolAddrToVa_Called == 1);
	CHECK(MemMgrLite::translatePoolAddrToVa_addr == MEMMGR_DATA_AREA_ADDR);
	CHECK(MemMgrLite::initFirst_Called == 1);
	CHECK(MemMgrLite::initFirst_manager_area == (void *)&tmp);
	CHECK(MemMgrLite::initFirst_area_size == MEMMGR_DATA_AREA_SIZE);
	CHECK(MemMgrLite::initPerCpu_Called == 1);
	CHECK(MemMgrLite::initPerCpu_manager_area == (void *)&tmp);
	CHECK(SYS_SetFlag_Called == 1);
	/* AudioManager::run()の最後のque.pop() */
	CHECK(MsgQueBlock_pop_Called == 1);
	
	/* call順の確認 */
	/* RAMの電源ONがMsgLib,MemMerの初期化より先 */
	CHECK(PM_RamControlByAddress_Sequence < MsgLib_initFirst_Sequence );
	CHECK(PM_RamControlByAddress_Sequence < MemMgrLite::initFirst_Sequence );
	/* MsgLibの初期化はinitFirst,initPerCpuの順 */
	CHECK(MsgLib_initFirst_Sequence < MsgLib_initPerCpu_Sequence );
	/* MemMgeの初期化はinitFirst,initPerCpuの順 */
	CHECK(MemMgrLite::initFirst_Sequence < MemMgrLite::initPerCpu_Sequence );
	/* ReadyFlagのセットはMsgLib,MemMerの初期化の後 */
	CHECK(MsgLib_initPerCpu_Sequence < SYS_SetFlag_Sequence);
	CHECK(MemMgrLite::initPerCpu_Sequence < SYS_SetFlag_Sequence);
	
	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}

/* AS_SendAdonisCommandの確認 */
extern "C" void AS_SendAdonisCommand(AudioCommand*);
TEST(audio_manager, send_adonis_command) {
	AudioCommand cmd;
	
	/* initbbのコマンドパケットで呼ぶ */
	cmd.header.command_code = AUDCMD_INITBBMODE;
	cmd.header.packet_length = LENGTH_SUB_INITBB_ADNIOSET;
	cmd.header.sub_code = SUB_INITBB_ADNIOSET;
	initbb_ReturnValue = AUDRLT_INITBBCMPLT;
	initbb_Called = 0;
	AS_SendAdonisCommand(&cmd);
	CHECK(initbb_Called == 1);
	CHECK(memcmp(&initbb_cmd,&cmd,sizeof(AudioCommand))==0); /* initbbの引数確認 */
	CHECK(!AssertionFailed);

	/* setbbのコマンドパケットで呼ぶ */
	cmd.header.command_code = AUDCMD_SETBBPARAM;
	cmd.header.packet_length = LENGTH_SUB_SETBB_ADNIOSET;
	cmd.header.sub_code = SUB_SETBB_ADNIOSET;
	setbb_ReturnValue = AUDRLT_SETBBPARAMCMPLT;
	setbb_Called = 0;
	AS_SendAdonisCommand(&cmd);
	CHECK(setbb_Called == 1);
	CHECK(memcmp(&setbb_cmd,&cmd,sizeof(AudioCommand))==0); /* setbbの引数確認 */
	CHECK(!AssertionFailed);

	/* その他のコマンドパケットで呼ぶ(ここではgetstatus) */
	cmd.header.command_code = AUDCMD_GETSTATUS;
	cmd.header.packet_length = LENGTH_SUB_SETBB_ADNIOSET;
	cmd.header.sub_code = SUB_SETBB_ADNIOSET;
	initbb_Called = 0;
	setbb_Called = 0;
	AS_SendAdonisCommand(&cmd);
	CHECK(initbb_Called == 0);
	CHECK(setbb_Called == 0);
	CHECK(AssertionFailed == 1); /* assertすることを確認 */
}

/* コマンド実行のサンプル(共通ルーチン未使用) */
TEST(audio_manager, startbb) {
/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_SETBASEBANDSTATUS;
	cmd.header.packet_length = LENGTH_SET_BASEBAND_STATUS;
	cmd.header.sub_code = 0;

	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SETACTIVE);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* 予め、コマンド(メッセージ)に応じた関数が返す値を設定しておく */
	setActiveBaseband_ReturnValue = AUDRLT_STATUSCHANGED;
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_STATUSCHANGED, LENGTH_AUDRLT, cmd.header.sub_code);
	
	/* □個別の関数((setActiveBaseband)が呼ばれたこと確認 */
	CHECK(setActiveBaseband_Called == 1);
	
/* ■コマンドパケット作成 */
	cmd.header.command_code = AUDCMD_STARTBB;
	cmd.header.packet_length = LENGTH_STARTBB;
	cmd.header.sub_code = 0;

	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_SOUNDFX);

	/* ■メッセージ→個別の関数→リザルト/Objectへのメッセージ の確認 */
	/* 予め、コマンド(メッセージ)に応じた関数が関数が返す値を設定しておく */
	MsgLib_send_ReturnValue[1] = ERR_OK;
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);

	/* result 確認 */
	result_message_check(AUDRLT_STARTBBCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □Objectに送信したメッセージの確認 */
	object_message_check(MSGQ_AUD_SOUND_EFFECT, MSG_AUD_SOUND_EFFECT_CMD_START, &cmd);
}

/* コマンド実行のサンプル(共通ルーチン未使用, パラメータ詳細設定あり) */
TEST(audio_manager, initbbmode) {
	/* ■コマンドパケット作成 */
	AudioCommand cmd;
	cmd.header.command_code = AUDCMD_INITBBMODE;
	cmd.header.packet_length = LENGTH_SUB_INITBB_ADNIOSET;
	cmd.header.sub_code = SUB_INITBB_ADNIOSET;
	cmd.init_bb_param.adn_io_set.xtal_sel         = AS_ADN_XTAL_24_576MHZ;
	cmd.init_bb_param.adn_io_set.clk_mode         = AS_CLK_MODE_NORMAL;
	cmd.init_bb_param.adn_io_set.ser_mode = AS_SER_MODE_64FS;		/* AdonisからSpritzerへのシリアルデータ伝送のモードを指定 */
	cmd.init_bb_param.adn_io_set.mic_bias_sel = 0;			/* ES */
	cmd.init_bb_param.adn_io_set.mic_gain_d = AS_ADN_MIC_GAIN_HOLD;	/* Adonisのマイクゲインを設定 */
	cmd.init_bb_param.adn_io_set.mic_gain_c = AS_ADN_MIC_GAIN_HOLD;	/* Adonisのマイクゲインを設定 */
	cmd.init_bb_param.adn_io_set.mic_gain_b = AS_ADN_MIC_GAIN_HOLD;	/* Adonisのマイクゲインを設定 */
	cmd.init_bb_param.adn_io_set.mic_gain_a = AS_ADN_MIC_GAIN_HOLD;	/* Adonisのマイクゲインを設定 */
	cmd.init_bb_param.adn_io_set.pga_gain_d = AS_ADN_PGA_GAIN_HOLD;	/* AdonisのPGAゲインを10倍の整数値で設定 */
	cmd.init_bb_param.adn_io_set.pga_gain_c = AS_ADN_PGA_GAIN_HOLD;	/* AdonisのPGAゲインを10倍の整数値で設定 */
	cmd.init_bb_param.adn_io_set.pga_gain_b = AS_ADN_PGA_GAIN_HOLD;	/* AdonisのPGAゲインを10倍の整数値で設定 */
	cmd.init_bb_param.adn_io_set.pga_gain_a = AS_ADN_PGA_GAIN_HOLD;	/* AdonisのPGAゲインを10倍の整数値で設定 */
	cmd.init_bb_param.adn_io_set.vgain_d = AS_ADN_VGAIN_HOLD;				/* AdonisのVGAINを10倍の整数値で設定 */
	cmd.init_bb_param.adn_io_set.vgain_c = AS_ADN_VGAIN_HOLD;				/* AdonisのVGAINを10倍の整数値で設定 */
	cmd.init_bb_param.adn_io_set.vgain_b = AS_ADN_VGAIN_HOLD;				/* AdonisのVGAINを10倍の整数値で設定 */
	cmd.init_bb_param.adn_io_set.vgain_a = AS_ADN_VGAIN_HOLD;				/* AdonisのVGAINを10倍の整数値で設定 */
	cmd.init_bb_param.adn_io_set.mic_channel_sel = 0xffffffff;		/* Adonisのマイク入力をマトリックス形式で指定 */
	cmd.init_bb_param.adn_io_set.input_device_sel = AS_ADN_IN_OFF;	/* Adonisから??するデバイスを指定 */
	cmd.init_bb_param.adn_io_set.output_device_sel = AS_ADN_OUT_HP;	/* Adonisから発音するデバイスを指定 */
	cmd.init_bb_param.adn_io_set.pwm_sel_b = AS_ADN_PWM_OFF;		/* AdonisのPWM出力ポートを指定 */
	cmd.init_bb_param.adn_io_set.pwm_sel_a = AS_ADN_PWM_OFF;		/* AdonisのPWM出力ポートを指定 */
	cmd.init_bb_param.adn_io_set.mclk_ds     = AS_ADN_IO_DS_HOLD;	/* ES */
	cmd.init_bb_param.adn_io_set.dmic_clk_ds = AS_ADN_IO_DS_HOLD;	/* ES */
	cmd.init_bb_param.adn_io_set.ad_data_ds  = AS_ADN_IO_DS_HOLD;	/* ES */
	cmd.init_bb_param.adn_io_set.gpo_ds      = AS_ADN_IO_DS_HOLD;	/* ES */

	/* ■コマンド発行→メッセージ の確認 */
	command_send_and_check(cmd, MSG_AUD_MGR_CMD_INITBB);

	/* ■メッセージ→個別の関数→リザルトメッセージ の確認 */
	/* 予め、コマンド(メッセージ)に応じた関数が返す値を設定しておく */
	initbb_ReturnValue = AUDRLT_INITBBCMPLT;
	/* audio managerのメインルーチン(run)の代わりにparse呼び出し */
	mng->parse(MsgLib_send_msg[1]);
	/* result 確認 */
	result_message_check(AUDRLT_INITBBCMPLT, LENGTH_AUDRLT, cmd.header.sub_code);
	/* □個別の関数(initbb)依存の確認 */
	CHECK(initbb_Called == 1);
	CHECK(memcmp(&initbb_cmd,&cmd,sizeof(AudioCommand))==0); /* ★initbbの引数確認 */

	/* ■アサートが起きていないこと */
	CHECK(!AssertionFailed);
}


#if 0
/** simpleテストグループを定義 */
TEST_GROUP(simple) {
};

/* テストはプロダクションコードを評価するプログラムです。
 * テストはTESTマクロで定義し、第1項にテストグループ名、第2項にテスト名を記述します。
 * テストグループ名＋テスト名は一意でなければなりません。
 *
 * テストは関数のように記述できますが、引数はありません。
 * プログラムが満たさなければならない性質は、フレームワークから提供されるマクロを使用して記述します。
 * 主なものを示します。
 * <li> 任意の式を評価するCHECK </li>
 * <li> 整数を比較するLONGS_EQUALS, UNSIGNED_LONG_EQUALS </li>
 * <li> 浮動小数点を比較するDOUBLES_EQUALS </li>
 * <li> 文字列を比較するSTRCMP_EQUALS, STRCMP_CONTAINS </li>
 *
 * テストが失敗する可能性があることが分かっている場合、TESTマクロの代わりにIGNORE_TESTマクロを使用することができます。
 * IGNORE_TESTマクロで記述されたテストは、期待動作が得られない場合でも失敗にはなりません。
 */

extern "C" bool AS_activatePlayer();
TEST(simple, check) {
	/* CHECK(condition): conditionが成立する */
	//char str123[] = "123";
	//CHECK(atoi(str123) == 123);
	CHECK(AS_activatePlayer()==true);
}

TEST(simple, check_true) {
	/* CHECK_TRUE(condition): conditionがTRUE(非0)である */
	CHECK_TRUE(1 == 1);
}

TEST(simple, check_false) {
	/* CHECK_FALSE(condition): conditionがFALSE(0)である */
	CHECK_FALSE(0 == 1);
}

TEST(simple, check_equal) {
	/* CHECK_EQUAL(expected,actual): expectedとactualが一致する */
	int val = 1 + 1;
	CHECK_EQUAL(2, val);
}

TEST(simple, strcmp_equal) {
	/* STRCMP_EQUAL(expected,actual): expectedとactualがstrcmpで一致する */
	char str[10];
	strcpy(str, "abc");
	STRCMP_EQUAL("abc", str);
}

TEST(simple, strcmp_nocase_equal) {
	/* STRCMP_NOCASE_EQUAL(expected,actual): expectedとactualがで一致する (大文字小文字を区別しない) */
	char str[10];
	strcpy(str, "ABC");
	STRCMP_NOCASE_EQUAL("abc", str);
}

TEST(simple, strcmp_contains) {
	/* STRCMP_CONTAINS(expected,actual): actualがexpectedを含む */
	char str[10];
	strcpy(str, "abcde");
	STRCMP_CONTAINS("abc", str);
}

TEST(simple, strcmp_nocase_contains) {
	/* STRCMP_NOCASE_CONTAINS(expected,actual): actualがexpectedを含む (大文字小文字を区別しない) */
	char str[10];
	strcpy(str, "ABCDE");
	STRCMP_NOCASE_CONTAINS("abc", str);
}

TEST(simple, longs_equal) {
	/* LONGS_EQUAL(expected,actual): actualとexpectedがlong値として一致する */
	long val = 1 + 1;
	LONGS_EQUAL(2l, val);
}

TEST(simple, unsigned_longs_equal) {
	/* UNSIGNED_LONGS_EQUAL(expected,actual): actualとexpectedがunsigned long値として一致する */
	unsigned long val = 1 + 1;
	UNSIGNED_LONGS_EQUAL(2ul, val);
}

TEST(simple, bytes_equal) {
	/* BYTES_EQUAL(expected, actual): actualとexpectedが1バイト値として一致する */
	unsigned char val = (unsigned char) 256;
	BYTES_EQUAL(0, val);
}

TEST(simple, pointer) {
	/* POINTERS_EQUAL(expected, actual): actualとexpectedのポインタが一致する */
	POINTERS_EQUAL(printf, printf);
}

TEST(simple, doubles) {
	/* DOUBLES_EQUAL(expected,actual,threshold): actualとexpectedの差がthreshold以内である */
	DOUBLES_EQUAL(1.0f, 1.01f, 0.05f);
}

IGNORE_TEST(simple, ignore_but_fail) {
	/* 必ず失敗する */
	FAIL("always fail");
}

/* 同じようなテストを繰り返す場合、fixtureの使用を検討してください。
 * fixtureを使うと、テストグループの中で共通の前処理・後処理をくくり出すことができます。
 */

TEST_GROUP(fixture) {
	char *a_to_z;
	TEST_SETUP() {
		int i;
		a_to_z = (char*) malloc(100);
		for (i = 0; i < 26; i++) {
			a_to_z[i] = 'a' + i;
		}
	}
	TEST_TEARDOWN() {
		free(a_to_z);
	}
};

TEST(fixture, find_a) {
	STRCMP_CONTAINS("a", a_to_z);
	*(strstr(a_to_z, "a")) = '_';
	CHECK(strstr(a_to_z, "a") == NULL);
}

TEST(fixture, find_b) {
	STRCMP_CONTAINS("b", a_to_z);
	*(strstr(a_to_z, "b")) = '_';
	CHECK(strstr(a_to_z, "b") == NULL);
}

TEST(fixture, find_c) {
	STRCMP_CONTAINS("c", a_to_z);
	*(strstr(a_to_z, "c")) = '_';
	CHECK(strstr(a_to_z, "c") == NULL);
}
#endif

/* main関数でフレームワークのテストランナーを呼び出すと、記述したすべてのテストが実行されます。
 */

int main(int argc, char *argv[]) {
	return CommandLineTestRunner::RunAllTests(argc, argv);
}
