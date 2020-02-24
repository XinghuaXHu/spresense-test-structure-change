//***************************************************************************
// test/sqa/singlefunction/gnss_extension_test/common_def.hpp
//
//   Data  : 2019/01/31
//   Author: Neusoft
//***************************************************************************
#include "libgetopt.hpp"
#include <arch/chip/gnss.h>


#ifndef COMMON_DEF_HPP_
#define COMMON_DEF_HPP_ 1

#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define COMMAND_MAX_SIZE 128
#define SCRIPT_COMMAND_BUFFER_MAX 128
#define TEST_RECORDING_CYCLE      1
#define TEST_NOTIFY_THRESHOLD     CXD56_GNSS_PVTLOG_THRESHOLD_HALF
#define GNSS_POLL_FD_NUM          1
#define FILE_NAME_LEN             256
#define CONFIG_EXAMPLES_HELLO_TASK_PRIORITY  SCHED_PRIORITY_DEFAULT
#define CONFIG_EXAMPLES_HELLO_TASK_STACKSIZE 9216
#define LONG_OPTIONS_MAX 15


enum CommandType_ST
{
    kStart,
    kStop,
    kSignalSet,
    kGetOPE,
    kSetOPE,
    kSetSatellite,
    kGetSatellite,
    kSetEllipsoidalPosition,
    kSetOrthogonalPosition,
    kSetTime,
    kSaveBackupData,
    kEraseBackupData,
    kOpenCEP,
    kCloseCEP,
    kCheckCEP,
    kResetCEP,
    kGetCEP,
    kFactoryStart,
    kFactoryStop,
    kFactoryGetResult,
    kGetTCXO_Offset,
    kSetTCXO_Offset,
    kSetAlmanac,
    kGetAlmanac,
    kSetEphemeris,
    kGetEphemeris,
    kPvtLogStart,
    kPvtLogStop,
    kPvtDeleteLog,
    kPvtStatusGet,


    kSaveConfig,
    kLoadConfig,

    kCheckGnssTTFF,
    kCheckGnssPosAccuracy,
    kCheckGnssNMEA,
    kCheckGnssPosition,
    kCheckOffline,
    kCheckPvtLog,
    kCheckSNR,
    kQuit,
    kPause,
    kInvalid,

};

union CommandSetting_ST
{
    uint32_t start_setting;                                                     //  kStart,
    uint32_t stop_setting;                                                      //  kStop,
    struct cxd56_gnss_signal_setting_s signal_setting;                          //  kSignalSet,
    struct cxd56_gnss_ope_mode_param_s opemode_setting;                         //  kGetOPE,
    uint32_t satellite_setting;                                                 //  kSetSatellite,
    struct cxd56_gnss_ellipsoidal_position_s ellipsoidal_position;              //  kSetEllipsoidalPosition,
    struct cxd56_gnss_orthogonal_position_s orthogonal_position;                //  kSetOrthogonalPosition,
    struct cxd56_gnss_datetime_s settime;                                       //  kSetTime,
    unsigned long tcxo_setting;                                                 //  kGetTCXO_Offset,
    struct cxd56_gnss_orbital_param_s orbitalparam;                             //  kSetAlmanac,
    uint32_t localSDNumber;
    int32_t time;
    double value;
};

struct Command_ST
{
    enum CommandType_ST type;
    char cmdStr[COMMAND_MAX_SIZE];
    int command_id;
    union CommandSetting_ST setting;
    union CommandSetting_ST setting1;
    union CommandSetting_ST setting2;
};

struct CommandList
{
    struct Command_ST cmd;
    struct CommandList *prev;
    struct CommandList *next;
};



struct cmd_option
{
    char *cmd;
    char *optstring;
    enum CommandType_ST case_value;
    int cmd_id;
    struct option long_options[LONG_OPTIONS_MAX];
};



#endif

