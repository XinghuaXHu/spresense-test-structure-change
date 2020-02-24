//***************************************************************************
// test/sqa/singlefunction/gnss_extension_test/gnss_api.cxx
//
//   Data  : 2019/01/31
//   Author: Neusoft
//***************************************************************************
#include <sdk/config.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <arch/chip/gnss.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <cstdio>
#include <debug.h>
#include <nuttx/init.h>

#include <math.h>

#include "gnss_api.hpp"
#include "libgetopt.hpp"
#include "system/readline.h"

static char  nmea_buf[NMEA_SENTENCE_MAX_LEN];
struct nmealog_ST
{
    char *gnssKeyWord;
    int gnssKeyCount;
};
static nmealog_ST emealog[] = {{"GGA", 0}, {"GLL", 0}, {"GSA", 0}, {"GSV", 0},
    {"RMC", 0}, {"VTG", 0}, {"ZDA", 0}, {"GSA", 0}
};
static bool checkNMEAlog()
{
    bool ret = true;
    for(int loop = 0; loop < (sizeof(emealog) / sizeof(emealog[0])); loop++)
    {
        if(emealog[loop].gnssKeyCount <= 0)
        {
            ret = false;
        }
        emealog[loop].gnssKeyCount = 0;
    }
    return ret;
}
FAR static char *reqbuf(uint16_t size)
{
    if (size > sizeof(nmea_buf))
    {
        printf("reqbuf error: oversize %s\n", size);
        return NULL;
    }
    return nmea_buf;
}
static void freebuf(FAR char *buf)
{
}
static int outbin(FAR char *buf, uint32_t len)
{
    return 0;
}

static int outnmea(FAR char *buf)
{
    for(int loop = 0; loop < (sizeof(emealog) / sizeof(emealog[0])); loop++)
    {
        if(strstr(buf, emealog[loop].gnssKeyWord))
        {
            emealog[loop].gnssKeyCount++;
        }
    }
    return 0;
}

static uint32_t writeSD0(uint32_t file_index, char *pFilePath, char *pSrc, uint32_t srcLength)
{
    uint32_t ret = 0;
    char filename[FILE_NAME_LEN] = {0,};
    snprintf(filename, FILE_NAME_LEN, "%s%d", pFilePath, file_index);
    FILE *fp_write = fopen(filename, "w");
    char *pBegin = pSrc;
    if (fp_write != NULL)
    {
        char *pEnd = pBegin + srcLength;
        while(pBegin < pEnd)
        {
            fputc(*pBegin, fp_write);
            pBegin++;
            ret++;
        }
        fclose(fp_write);
        fp_write = 0;
    }
    else
    {
        printf("writeSD0 %s failed\n", filename);
    }
    return ret;
}

static uint32_t readSD0(uint32_t file_index, char *pFilePath, char *pDst, uint32_t dstLength)
{
    uint32_t ret = 0;
    char filename[FILE_NAME_LEN] = {0,};
    snprintf(filename, FILE_NAME_LEN, "%s%d", pFilePath, file_index);
    FILE *fp_read = fopen(filename, "r");
    char *pBegin = pDst;
    if(fp_read != NULL)
    {
        int32_t readCount = 0;
        do
        {
            int c = fgetc(fp_read);
            if( feof(fp_read) )
            {
                break ;
            }
            else
            {
                ret++;
                readCount++;
                *pBegin = 0x000000FF & c;
                pBegin++;
            }
        }
        while(readCount < dstLength);
        fclose(fp_read);
        fp_read = 0;
    }
    else
    {
        printf("readSD0 %s failed\n", filename);
    }
    return ret;
}


void gnss_api::DebugPrintfSatellites(struct cxd56_gnss_positiondata_s posdat)
{
    struct cxd56_gnss_dms_s      dmf;
    printf("DebugPrintfSatellites %d-%02d-%02d_%d:%d:%d:%d [%8.8f][%8.8f][%8.8f] \n", posdat.receiver.date.year,
           posdat.receiver.date.month, posdat.receiver.date.day,
           posdat.receiver.time.hour, posdat.receiver.time.minute,
           posdat.receiver.time.sec, posdat.receiver.time.usec,
           posdat.receiver.latitude, posdat.receiver.longitude, posdat.receiver.altitude);
    printf( "info: ");
    if(satellite_setting &  CXD56_GNSS_SAT_GPS)
        printf( "gps ");
    if(satellite_setting &  CXD56_GNSS_SAT_GLONASS)
        printf( "GLONASS ");
    if(satellite_setting &  CXD56_GNSS_SAT_QZ_L1CA)
        printf( "QZ_L1CA ");
    if(satellite_setting &  CXD56_GNSS_SAT_QZ_L1S)
        printf( "QZ_L1S ");


    if(startMode == CXD56_GNSS_STMOD_COLD)
        printf( "cold \n");
    if(startMode == CXD56_GNSS_STMOD_HOT)
        printf( "hot \n");
    if(startMode == CXD56_GNSS_STMOD_WARM)
        printf( "warm \n");

    int loop = 0;
    while(loop < CXD56_GNSS_MAX_SV_NUM)
    {
        if(posdat.sv[loop].svid)
        {
            printf("id[%d]#%d siglevel#%f type#%d  state#%d ", loop,  posdat.sv[loop].svid,
                   posdat.sv[loop].siglevel,
                   posdat.sv[loop].type,
                   posdat.sv[loop].stat);
            (posdat.sv[loop].type & 0x01) ? printf("GPS ") : printf(" ");
            (posdat.sv[loop].type & 0x02) ? printf("GLONASS ") : printf(" ");
            (posdat.sv[loop].type & 0x04) ? printf("SBAS ") : printf(" ");
            (posdat.sv[loop].type & 0x08) ? printf("QZSS_L1CA ") : printf(" ");
            (posdat.sv[loop].type & 0x10) ? printf("IMES ") : printf(" ");
            (posdat.sv[loop].type & 0x20) ? printf("QZSS_L1SAIF ") : printf(" ");
            (posdat.sv[loop].type & 0x40) ? printf("Beidu ") : printf(" ");
            (posdat.sv[loop].type & 0x80) ? printf("Galileo ") : printf(" ");

            (posdat.sv[loop].stat & 0x01) ? printf("tracking ") : printf(" ");
            (posdat.sv[loop].stat & 0x02) ? printf("positioning ") : printf(" ");
            (posdat.sv[loop].stat & 0x04) ? printf("calculating ") : printf(" ");
            (posdat.sv[loop].stat & 0x08) ? printf("visible ") : printf(" ");
            printf("\n");
        }
        loop++;
    }

    printf("pos_fixmode#%d pos_dataexist#%d \n",
           posdat.receiver.pos_fixmode,  posdat.receiver.pos_dataexist);
    if ( posdat.receiver.pos_dataexist && (posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID))
    {
        double_to_dmf(posdat.receiver.latitude, &dmf);
        printf("LAT %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);
        double_to_dmf(posdat.receiver.longitude, &dmf);
        printf("LNG %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);
    }
}


gnss_api *gnss_api::pInstantce = 0;
gnss_api *gnss_api::GetSingletonInstance()
{
    if(pInstantce == NULL)
    {
        pInstantce = new gnss_api();
    }
    return pInstantce;
}
void gnss_api::FreeSingletonInstance()
{
    if(pInstantce != NULL)
    {
        delete pInstantce;
        pInstantce = NULL;
    }
}

gnss_api::gnss_api()
{
    stateMachine = kGnssIDEL;
    gnss_fd = 0;
    sig_gnss = 0;
    sig_pvtlog = 0;
    sig_agps = 0;
    sig_rtk = 0;
    sig_spectrum = 0;
    sig_gpsephemeris = 0;
    sig_glnephemeris = 0;
    sig_sbas = 0;
    gnss_fd = open("/dev/gps", O_RDONLY);
    pvtLogIndex = 0;
    cycle = 1000;
    startMode = 0xff;
    sigemptyset(&mask);

    for(int loop = 0; loop < (sizeof(localBackupFileTime) / sizeof(timeval)); loop++)
    {
        localBackupFileTime[loop].tv_sec = 0;
        localBackupFileTime[loop].tv_usec = 0;
    }

    NMEA_InitMask();
    nmea_funcs.bufReq  = reqbuf;
    nmea_funcs.out     = outnmea;
    nmea_funcs.outBin  = outbin;
    nmea_funcs.bufFree = freebuf;
    NMEA_RegistOutputFunc(&nmea_funcs);
    pPosDataList = NULL;

}

gnss_api::~gnss_api()
{
    stateMachine = kGnssIDEL;
    gnss_fd = 0;
    sig_gnss = 0;
    sig_pvtlog = 0;
    sig_agps = 0;
    sig_rtk = 0;
    sig_spectrum = 0;
    sig_gpsephemeris = 0;
    sig_glnephemeris = 0;
    sig_sbas = 0;
    pvtLogIndex = 0;
    cycle = 0;
    startMode = 0xff;
    pPosDataList = NULL;

    if(gnss_fd != 0)
        close(gnss_fd);
}

bool gnss_api::processUserInput(Command_ST &cmd)
{
    // get tester input from Serial
    char string[SCRIPT_COMMAND_BUFFER_MAX] = {0,};
    int len = 0;
    do
    {
        len = readline(string, sizeof(string) - 1, stdin, stdout);
        string[len] = 0;
    }
    while(string[len - 1] != 13 && string[len - 1] != 10);
    string[len - 1] = 0;

    if(string[0] == 'q' && string[1] == 'u' && string[2] == 'i' &&
            string[3] == 't' && string[4] == 0 )
    {
        return false;
    }

    //parse string to argc and argv
    char *pStr = string;
    int strLength = sizeof(string);
    int argc = 0;
    char *argv[100] = {0,};

    argc = 0;
    while(*pStr)
    {
        if(*pStr == ' ')
        {
            *pStr = 0;
            if(argv[argc] == 0 && *(pStr + 1) != ' ')
            {
                argv[argc++] = pStr + 1;
            }
        }
        else if(argc == 0)
        {
            argv[argc++] = pStr;
        }
        pStr++;
    }

    //define input cmd parms
    static const struct cmd_option command_options[] =
    {
        {"start",                "z", kStart ,                CXD56_GNSS_IOCTL_START,                                   {{"cold", no_argument, NULL, 'c'},\
                                                                                                                         {"hot",  no_argument, NULL, 'h'},\
                                                                                                                         {"warm", no_argument, NULL, 'w'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"stop"   ,              "z", kStop,                  CXD56_GNSS_IOCTL_STOP,                                    {{0, 0, 0, 0},}},
        {"setSatellite",         "z", kSetSatellite,          CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM,                 {{"GPS", no_argument, NULL, 'g'},\
                                                                                                                         {"GLONASS", no_argument, NULL, 'l'},\
                                                                                                                         {"L1CA", no_argument, NULL, 'c'},\
                                                                                                                         {"L1S", no_argument, NULL, 's'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"getSatellite",         "z", kGetSatellite,          CXD56_GNSS_IOCTL_GET_SATELLITE_SYSTEM,                    {{0, 0, 0, 0},}},
        {"setEllipsoidal",       "z", kSetEllipsoidalPosition,  CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ELLIPSOIDAL,     {{"latitude", required_argument, NULL, 't'},\
                                                                                                                         {"longitude", required_argument, NULL, 'g'},\
                                                                                                                         {"altitude", required_argument, NULL, 'a'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"setOrthogonal", "x:y:z:",   kSetOrthogonalPosition,  CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ORTHOGONAL,       {{0, 0, 0, 0},}},
        {"getOPE",               "z", kGetOPE,                CXD56_GNSS_IOCTL_GET_OPE_MODE,                            {{0, 0, 0, 0},}},
        {"setOPE",               "z", kSetOPE,                CXD56_GNSS_IOCTL_SET_OPE_MODE,                            {{"mode", required_argument, NULL, 'm'},\
                                                                                                                         {"cycle", required_argument, NULL, 'c'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"getTCXO",              "z", kGetTCXO_Offset,        CXD56_GNSS_IOCTL_GET_TCXO_OFFSET,                         {{0, 0, 0, 0},}},
        {"setTCXO",              "z", kSetTCXO_Offset,        CXD56_GNSS_IOCTL_SET_TCXO_OFFSET,                         {{"offset", required_argument, NULL, 'o'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"setTime",              "z", kSetTime,               CXD56_GNSS_IOCTL_SET_TIME,                                {{"year", required_argument, NULL, 'y'},\
                                                                                                                         {"month", required_argument, NULL, 'm'},\
                                                                                                                         {"day", required_argument, NULL, 'd'},\
                                                                                                                         {"hour", required_argument, NULL, 'h'},\
                                                                                                                         {"minute", required_argument, NULL, 't'},\
                                                                                                                         {"sec", required_argument, NULL, 's'},\
                                                                                                                         {"usec", required_argument, NULL, 'u'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"setAlmanac",           "z", kSetAlmanac,            CXD56_GNSS_IOCTL_SET_ALMANAC,                             {{"GPS", no_argument, NULL, 'g'},\
                                                                                                                         {"GLONASS", no_argument, NULL, 'l'},\
                                                                                                                         {"QZSSL1CA", no_argument, NULL, 'c'},\
                                                                                                                         {"num", required_argument, NULL, 'n'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"getAlmanac",           "z", kGetAlmanac,            CXD56_GNSS_IOCTL_GET_ALMANAC,                             {{"GPS", no_argument, NULL, 'g'},\
                                                                                                                         {"GLONASS", no_argument, NULL, 'l'},\
                                                                                                                         {"QZSSL1CA", no_argument, NULL, 'c'},\
                                                                                                                         {"num", required_argument, NULL, 'n'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"setEphemeris",         "z", kSetEphemeris,          CXD56_GNSS_IOCTL_SET_EPHEMERIS,                           {{"GPS", no_argument, NULL, 'g'},\
                                                                                                                         {"GLONASS", no_argument, NULL, 'l'},\
                                                                                                                         {"QZSSL1CA", no_argument, NULL, 'c'},\
                                                                                                                         {"num", required_argument, NULL, 'n'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"getEphemeris",         "z", kGetEphemeris,          CXD56_GNSS_IOCTL_GET_EPHEMERIS,                           {{"GPS", no_argument, NULL, 'g'},\
                                                                                                                         {"GLONASS", no_argument, NULL, 'l'},\
                                                                                                                         {"QZSSL1CA", no_argument, NULL, 'c'},\
                                                                                                                         {"num", required_argument, NULL, 'n'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"saveBackupData",       "z", kSaveBackupData,        CXD56_GNSS_IOCTL_SAVE_BACKUP_DATA,                        {{0, 0, 0, 0},}},
        {"eraseBackupData",      "z", kEraseBackupData,       CXD56_GNSS_IOCTL_ERASE_BACKUP_DATA,                       {{0, 0, 0, 0},}},
        {"setSignal",            "z", kSignalSet,             CXD56_GNSS_IOCTL_SIGNAL_SET,                              {{"SIG_GNSS", no_argument, NULL, 'g'},\
                                                                                                                         {"SIG_PVTLOG", no_argument, NULL, 'p'},\
                                                                                                                         {"SIG_AGPS", no_argument, NULL, 'a'},\
                                                                                                                         {"SIG_RTK", no_argument, NULL, 'r'},\
                                                                                                                         {"SIG_SPECTRUM", no_argument, NULL, 's'},\
                                                                                                                         {"SIG_GPSEPHEMERIS", no_argument, NULL, 'i'},\
                                                                                                                         {"SIG_GLNEPHEMERIS", no_argument, NULL, 'l'},\
                                                                                                                         {"SIG_SBAS", no_argument, NULL, 'b'},\
                                                                                                                         {"signo", required_argument, NULL, 'n'},\
                                                                                                                         {"enable", required_argument, NULL, 'e'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"pvtLogStart",          "z", kPvtLogStart,           CXD56_GNSS_IOCTL_PVTLOG_START ,                           {{0, 0, 0, 0},}},
        {"pvtLogStop",           "z", kPvtLogStop,            CXD56_GNSS_IOCTL_PVTLOG_STOP  ,                           {{0, 0, 0, 0},}},
        {"pvtDeleteLog",         "z", kPvtDeleteLog,          CXD56_GNSS_IOCTL_PVTLOG_DELETE_LOG ,                      {{0, 0, 0, 0},}},
        {"pvtStatusGet",         "z", kPvtStatusGet,          CXD56_GNSS_IOCTL_PVTLOG_GET_STATUS ,                      {{0, 0, 0, 0},}},
        {"openCEP",              "z", kOpenCEP,               CXD56_GNSS_IOCTL_OPEN_CEP_DATA,                           {{0, 0, 0, 0},}},
        {"closeCEP",             "z", kCloseCEP,              CXD56_GNSS_IOCTL_CLOSE_CEP_DATA,                          {{0, 0, 0, 0},}},
        {"checkCEP",             "z", kCheckCEP,              CXD56_GNSS_IOCTL_CHECK_CEP_DATA,                          {{0, 0, 0, 0},}},
        {"resetCEP",             "z", kResetCEP,              CXD56_GNSS_IOCTL_RESET_CEP_FLAG,                          {{0, 0, 0, 0},}},
        {"getCEP",               "z", kGetCEP,                CXD56_GNSS_IOCTL_GET_CEP_AGE,                             {{0, 0, 0, 0},}},
        {"factoryStart",         "z", kFactoryStart,          CXD56_GNSS_IOCTL_FACTORY_START_TEST ,                     {{0, 0, 0, 0},}},
        {"factoryStop",          "z", kFactoryStop,           CXD56_GNSS_IOCTL_FACTORY_STOP_TEST ,                      {{0, 0, 0, 0},}},
        {"factoryGetResult",     "z", kFactoryGetResult,      CXD56_GNSS_IOCTL_FACTORY_GET_TEST_RESULT ,                {{0, 0, 0, 0},}},
        {"quit"  ,               "z", kQuit,                  0,                                                        {{0, 0, 0, 0},}},
        {"checkGnssTTFF",        "z", kCheckGnssTTFF,         0,                                                        {{"time1", required_argument, NULL, '1'},\
                                                                                                                         {"time2", required_argument, NULL, '2'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"checkGnssPosAccuracy", "z", kCheckGnssPosAccuracy,  0,                                                        {{"time", required_argument, NULL, 't'},\
                                                                                                                         {"value1", required_argument, NULL, '1'},\
                                                                                                                         {"value2", required_argument, NULL, '2'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"checkGnssNMEA",        "z", kCheckGnssNMEA,         0,                                                        {{"time", required_argument, NULL, 't'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"checkGnssPosition",    "z", kCheckGnssPosition,     0,                                                        {{"time", required_argument, NULL, 't'},\
                                                                                                                         {"value", required_argument, NULL, 'v'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"checkPvtLog",          "z", kCheckPvtLog,           0,                                                        {{"time", required_argument, NULL, 't'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"checkSNR",             "z", kCheckSNR,              0,                                                        {{"time", required_argument, NULL, 't'},\
                                                                                                                         {"value1", required_argument, NULL, '1'},\
                                                                                                                         {"value2", required_argument, NULL, '2'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"saveConfig",           "z", kSaveConfig,            0,                                                        {{"num", required_argument, NULL, 'n'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"loadConfig",           "z", kLoadConfig,            0,                                                        {{"num", required_argument, NULL, 'n'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"checkOffline",         "z", kCheckOffline,          0,                                                        {{"time", required_argument, NULL, 't'},\
                                                                                                                         {"value", required_argument, NULL, 'v'},\
                                                                                                                         {0, 0, 0, 0},}},
        {"pause",                "z", kPause,                 0,                                                        {{"time", required_argument, NULL, 't'},\
                                                                                                                         {0, 0, 0, 0},}},
    };
    //check input command index in command_options
    int commandIndex = 0;
    int commandTableMax = sizeof(command_options) / sizeof(cmd_option);
    while(commandIndex < commandTableMax)
    {
        if(strcmp(argv[0], command_options[commandIndex].cmd) == 0)
        {
            break;
        }
        commandIndex++;
    }
    // find input command index
    if(commandIndex < commandTableMax)
    {
        int opt;
        int option_index = 0;
        //struct Command_ST cmd;
        memset(cmd.cmdStr, 0, sizeof(cmd.cmdStr));
        memset(&cmd.setting, 0, sizeof(cmd.setting));
        strcat(cmd.cmdStr, command_options[commandIndex].cmd);
        cmd.type = command_options[commandIndex].case_value;
        cmd.command_id = command_options[commandIndex].cmd_id;

        switch(cmd.type)
        {
        case kStart:
        {
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 'c':
                    cmd.setting.start_setting = CXD56_GNSS_STMOD_COLD;
                    break;
                case 'h':
                    cmd.setting.start_setting = CXD56_GNSS_STMOD_HOT;
                    break;
                case 'w':
                    cmd.setting.start_setting = CXD56_GNSS_STMOD_WARM;
                    break;
                }
            }
            break;
        }
        case kStop:
        {
            cmd.setting.stop_setting = 0;
            break;
        }
        case kSetSatellite:
        {
            cmd.setting.satellite_setting = 0;
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 'g':
                    cmd.setting.satellite_setting |=  CXD56_GNSS_SAT_GPS;
                    break;
                case 'l':
                    cmd.setting.satellite_setting |= CXD56_GNSS_SAT_GLONASS;
                    break;
                case 'c':
                    cmd.setting.satellite_setting |= CXD56_GNSS_SAT_QZ_L1CA;
                    break;
                case 's':
                    cmd.setting.satellite_setting |= CXD56_GNSS_SAT_QZ_L1S;
                    break;
                }
            }
            break;
        }
        case kSetEllipsoidalPosition:
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 't':
                    cmd.setting.ellipsoidal_position.latitude = atof(optarg);
                    break;
                case 'g':
                    cmd.setting.ellipsoidal_position.longitude = atof(optarg);
                    break;
                case 'a':
                    cmd.setting.ellipsoidal_position.altitude = atof(optarg);
                    break;
                }
            }
            break;
        case kSetOrthogonalPosition:
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 'x':
                    cmd.setting.orthogonal_position.x = atof(optarg);
                    break;
                case 'y':
                    cmd.setting.orthogonal_position.y = atof(optarg);
                    break;
                case 'z':
                    cmd.setting.orthogonal_position.z = atof(optarg);
                    break;
                }
            }
            break;
        case kSetOPE:
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 'm':
                    cmd.setting.opemode_setting.mode = atoi(optarg);
                    break;
                case 'c':
                    cmd.setting.opemode_setting.cycle = atoi(optarg);
                    break;
                }
            }
            break;
        case kSetTCXO_Offset:
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 'o':
                    cmd.setting.tcxo_setting = atoi(optarg);
                    break;
                }
            }
            break;
        case kSetTime:
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 'y':
                    cmd.setting.settime.date.year = atoi(optarg);
                    break;
                case 'm':
                    cmd.setting.settime.date.month = atoi(optarg);
                    break;
                case 'd':
                    cmd.setting.settime.date.day = atoi(optarg);
                    break;
                case 'h':
                    cmd.setting.settime.time.hour = atoi(optarg);
                    break;
                case 't':
                    cmd.setting.settime.time.minute = atoi(optarg);
                    break;
                case 's':
                    cmd.setting.settime.time.sec = atoi(optarg);
                    break;
                case 'u':
                    cmd.setting.settime.time.usec = atoi(optarg);
                    break;
                }
            }
            break;
        case kSetAlmanac:
        case kGetAlmanac:
        case kSetEphemeris:
        case kGetEphemeris:
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 'g':
                    cmd.setting.orbitalparam.type = CXD56_GNSS_DATA_GPS;
                    break;
                case 'l':
                    cmd.setting.orbitalparam.type = CXD56_GNSS_DATA_GLONASS;
                    break;
                case 'c':
                    cmd.setting.orbitalparam.type = CXD56_GNSS_DATA_QZSSL1CA;
                    break;
                case 'n':
                    cmd.setting2.localSDNumber = atoi(optarg);
                    break;
                }
            }
            break;
        case kSignalSet:
        {
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                cmd.setting.signal_setting.data = NULL;
                switch(opt)
                {
                case 'p':
                    cmd.setting.signal_setting.gnsssig = CXD56_GNSS_SIG_PVTLOG;
                    break;
                case 'g':
                    cmd.setting.signal_setting.gnsssig = CXD56_GNSS_SIG_GNSS;
                    break;
                case 'a':
                    cmd.setting.signal_setting.gnsssig = CXD56_GNSS_SIG_AGPS;
                    break;
                case 'r':
                    cmd.setting.signal_setting.gnsssig = CXD56_GNSS_SIG_RTK;
                    break;
                case 's':
                    cmd.setting.signal_setting.gnsssig = CXD56_GNSS_SIG_SPECTRUM;
                    break;
                case 'i':
                    cmd.setting.signal_setting.gnsssig = CXD56_GNSS_SIG_GPSEPHEMERIS;
                    break;
                case 'l':
                    cmd.setting.signal_setting.gnsssig = CXD56_GNSS_SIG_GLNEPHEMERIS;
                    break;
                case 'b':
                    cmd.setting.signal_setting.gnsssig = CXD56_GNSS_SIG_SBAS;
                    break;
                case 'e':
                    cmd.setting.signal_setting.enable = atoi(optarg);
                    break;
                case 'n':
                    cmd.setting.signal_setting.signo = atoi(optarg);
                    break;
                }
            }
            break;
        }
        case kLoadConfig:
        case kSaveConfig:
        {
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 'n':
                    cmd.setting.localSDNumber = atoi(optarg);
                    break;
                }
            }
            break;
        }
        case kCheckGnssTTFF:
        {
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case '1':
                    cmd.setting.time = atoi(optarg);
                    break;
                case '2':
                    cmd.setting2.time = atoi(optarg);
                    break;
                }
            }
            break;
        }
        case kCheckOffline:
        case kCheckGnssPosition:
        {
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 't':
                    cmd.setting.time = atoi(optarg);
                    break;
                case 'v':
                    cmd.setting1.value = atof(optarg);
                    break;
                }
            }
            break;
        }
        case kCheckGnssPosAccuracy:
        case kCheckPvtLog:
        case kCheckSNR:
        case kPause:
        case kCheckGnssNMEA:
        {
            optind = 1;
            while ( (opt = parse_arg_opt_long(argc, argv, command_options[commandIndex].optstring, command_options[commandIndex].long_options, &option_index)) != -1)
            {
                switch(opt)
                {
                case 't':
                    cmd.setting.time = atoi(optarg);
                    break;
                case '1':
                    cmd.setting1.value = atoi(optarg);
                    break;
                case '2':
                    cmd.setting2.value = atoi(optarg);
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    return true;
}

void gnss_api::DMS_RAD(double DMS, double *Rad)
{
    int Deg, Min;
    double Sec;
    Deg = (int)DMS;
    Min = (int)((DMS - Deg) * 100);
    Sec = ((DMS - Deg) * 100 - Min) * 100;
    *Rad = (Deg + Min / 60.0 + Sec / 3600.0) / 180.0 * M_PI;
    return;
}

double gnss_api::dms_to_double(const int degree, const int minute,
                               const int second, const int fraction)
{
    return (double)degree + (double)minute / 60.0 + (double)second / 3600.0 +
           (double)fraction / 3600000.0;
}

void gnss_api::RAD_DMS(double Rad, double *DMS)
{
    int Deg, Min;
    double Sec;
    double AR, AM;
    AR = Rad;
    if (Rad < 0)
        AR = -Rad;
    AR = AR + 1.0e-10;
    AR = AR * 180.0 / M_PI;
    Deg = (int)AR;
    AM = (AR - Deg) * 60.0;
    Min = (int)AM;
    Sec = (AM - Min) * 60;
    *DMS = Deg + Min / 100.0 + Sec / 10000.0;
    if(Rad < 0)
        *DMS = -*DMS;
    return;
}

void gnss_api::double_to_dmf(double x, struct cxd56_gnss_dms_s *dmf)
{
    int    b;
    int    d;
    int    m;
    double f;
    double t;

    if (x < 0)
    {
        b = 1;
        x = -x;
    }
    else
    {
        b = 0;
    }
    d = (int)x; /* = floor(x), x is always positive */
    t = (x - d) * 60;
    m = (int)t; /* = floor(t), t is always positive */
    f = (t - m) * 10000;

    dmf->sign   = b;
    dmf->degree = d;
    dmf->minute = m;
    dmf->frac   = f;
}

bool gnss_api::savePvtLogToLocal(cxd56_pvtlog_s &pvtlogdat)
{
    bool exitValue = false;
    int ret;
    ret = lseek(gnss_fd, CXD56_GNSS_READ_OFFSET_PVTLOG, SEEK_SET);
    if (ret >= 0)
    {
        /* Read PVTLOG. */
        ret = read(gnss_fd, &pvtlogdat, sizeof(pvtlogdat));
        if(ret > 0)
        {
            if(stateMachine & kGnssPosition)
            {
                if(startMode == CXD56_GNSS_STMOD_COLD)
                    writeSD0(pvtLogIndex++, "/mnt/spif/pvtlog_pos_cold_", (char *)&pvtlogdat, sizeof(pvtlogdat));
                else if(startMode == CXD56_GNSS_STMOD_HOT)
                    writeSD0(pvtLogIndex++, "/mnt/spif/pvtlog_pos_hot_", (char *)&pvtlogdat, sizeof(pvtlogdat));
                else if(startMode == CXD56_GNSS_STMOD_WARM)
                    writeSD0(pvtLogIndex++, "/mnt/spif/pvtlog_pos_warm_", (char *)&pvtlogdat, sizeof(pvtlogdat));
            }
            else
            {
                if(startMode == CXD56_GNSS_STMOD_COLD)
                    writeSD0(pvtLogIndex++, "/mnt/spif/pvtlog_none_cold_", (char *)&pvtlogdat, sizeof(pvtlogdat));
                else if(startMode == CXD56_GNSS_STMOD_HOT)
                    writeSD0(pvtLogIndex++, "/mnt/spif/pvtlog_none_hot_", (char *)&pvtlogdat, sizeof(pvtlogdat));
                else if(startMode == CXD56_GNSS_STMOD_WARM)
                    writeSD0(pvtLogIndex++, "/mnt/spif/pvtlog_none_warm_", (char *)&pvtlogdat, sizeof(pvtlogdat));
            }

            exitValue = true;
        }
        else
            printf("read error %d\n", ret);
    }
    else
    {
        ret = errno;
        printf("lseek error %d\n", ret);
    }
    return exitValue;
}

bool gnss_api::gnss_read_and_print_POS()
{
    int exit = false;
    struct cxd56_gnss_dms_s      dmf;
    struct cxd56_gnss_positiondata_s posdat;
    /* Seek data */
    int ret;
    if((stateMachine & kGnssStartingPVT) == kGnssStartingPVT)
    {
        lseek(gnss_fd, CXD56_GNSS_READ_OFFSET_LAST_GNSS, SEEK_SET);
    }
    /* Read POS data. */
    memset(&posdat, 0, sizeof(posdat));
    ret = read(gnss_fd, &posdat, sizeof(posdat));
    DebugPrintfSatellites(posdat);
    NMEA_Output(&posdat);
    if ((ret == sizeof(posdat)) &&  posdat.receiver.pos_dataexist &&
            (posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID))
    {
        /* 2D fix or 3D fix.
         * Convert latitude and longitude into dmf format and print it. */
        double_to_dmf(posdat.receiver.latitude, &dmf);
        printf("LAT %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);
        double_to_dmf(posdat.receiver.longitude, &dmf);
        printf("LNG %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);
        exit = true;
    }
    return exit;
}

void gnss_api::gnss_getFirstPOS()
{
    if((stateMachine & kGnssStarting) == kGnssStarting)
    {
        timespec timeout;
        timeout.tv_sec = 10;
        timeout.tv_nsec = 0;
        int sig_no = sigtimedwait(&mask, NULL, &timeout);

        if(sig_no == sig_gnss)
        {
            gnss_read_and_print_POS();
        }

        while(1)
        {
            sig_no = sigtimedwait(&mask, NULL, &timeout);
            if(sig_no == sig_gnss)
            {
                if(gnss_read_and_print_POS())
                {
                    struct timezone tz;
                    gettimeofday(&positionTime, &tz);
                    stateMachine |= kGnssPosition;
                    break;
                }
            }
            else if(sig_no == sig_pvtlog)
            {
                struct cxd56_pvtlog_s pvtlogdat;
                savePvtLogToLocal(pvtlogdat);
            }
        }
    }
}

double gnss_api::BLH2XYZ_To_Accuracy(PositionData *pPosDataHead)
{
    double ret = 0.0f;
    PositionData *pPosDataBegin = pPosDataHead;
    double averageX = 0;
    double averageY = 0;
    double averageZ = 0;
    uint32_t posLoop = 0;
    double XX = 0;
    double YY = 0;
    double ZZ = 0;
    if(pPosDataHead == NULL)
        return ret;

    while(pPosDataBegin != NULL)
    {
        double dSemiMajorAxis = 6378137.0;
        double dFlattening = 1 / 298.257223563;

        double N;
        double B_, L_;

        double B = pPosDataBegin->latitude;
        double L = pPosDataBegin->longitude;
        double H = pPosDataBegin->altitude;

        DMS_RAD(B, &B_);
        DMS_RAD(L, &L_);
        N = dSemiMajorAxis / sqrt(1.0 - dFlattening * (2 - dFlattening) * sin(B_) * sin(B_));
        pPosDataBegin->x = (N + H) * cos(B_) * cos(L_);
        pPosDataBegin->y = (N + H) * cos(B_) * sin(L_);
        pPosDataBegin->z = (N * (1.0 - dFlattening * (2 - dFlattening)) + H) * sin(B_);
        averageX += pPosDataBegin->x;
        averageY += pPosDataBegin->y;
        averageZ += pPosDataBegin->z;
        printf("latitude#%8.8f latitude#%8.8f  latitude#%8.8f  x#%8.8f y#%8.8f z#%8.8f\n", pPosDataBegin->latitude, pPosDataBegin->longitude, pPosDataBegin->altitude, pPosDataBegin->x, pPosDataBegin->y, pPosDataBegin->z);
        posLoop++;
        pPosDataBegin = pPosDataBegin->pNext;
    }
    averageZ = averageZ / posLoop;
    averageX = averageX / posLoop;
    averageY = averageY / posLoop;

    pPosDataBegin = pPosDataHead;
    while(pPosDataBegin != NULL)
    {
        XX += (pPosDataBegin->x - averageX) * (pPosDataBegin->x - averageX);
        YY += (pPosDataBegin->y - averageY) * (pPosDataBegin->y - averageY);
        ZZ += (pPosDataBegin->z - averageZ) * (pPosDataBegin->z - averageZ);
        PositionData *pTemp = pPosDataBegin;
        pPosDataBegin = pPosDataBegin->pNext;
        free(pTemp);
    }
    XX = XX / posLoop;
    YY = YY / posLoop;
    ZZ = ZZ / posLoop;
    ret = sqrt(XX + YY + ZZ);
    pPosDataList = NULL;
    return ret;
}

double gnss_api::gnss_check_SNR(int time, int siglevel1, int siglevel2)
{
    double availableSNR = 0.0f;
    bool calcSNR = false;
    struct cxd56_gnss_dms_s      dmf;
    int cmdRet;

    if((stateMachine & kGnssStarting) == kGnssStarting)
    {
        timespec timeout;
        timeout.tv_sec = (10 * cycle / 1000);
        timeout.tv_nsec = 0;
        int sig_no = sigtimedwait(&mask, NULL, &timeout);

        struct timeval    tvCurrent;
        struct timeval    tvEnd;
        struct timezone   tz;
        gettimeofday(&tvEnd, &tz);
        tvEnd.tv_sec += time;

        while(1)
        {
            sig_no = sigtimedwait(&mask, NULL, &timeout);



            if(sig_no == sig_gnss)
            {
                struct cxd56_gnss_positiondata_s posdat;
                if((stateMachine & kGnssStartingPVT) == kGnssStartingPVT)
                {
                    cmdRet = lseek(gnss_fd, CXD56_GNSS_READ_OFFSET_LAST_GNSS, SEEK_SET);
                }
                /* Read POS data. */
                memset(&posdat, 0, sizeof(posdat));
                cmdRet = read(gnss_fd, &posdat, sizeof(posdat));
                DebugPrintfSatellites(posdat);
                if(cmdRet == sizeof(posdat))
                {
                    int loop = 0;
                    bool checkSNR_PassFlag = true;
                    while(loop < CXD56_GNSS_MAX_SV_NUM)
                    {
                        if(posdat.sv[loop].svid && (posdat.sv[loop].stat & 0x02 || posdat.sv[loop].stat & 0x04) && !(
                                    posdat.sv[loop].siglevel > siglevel1 && posdat.sv[loop].siglevel < siglevel2 ||
                                    posdat.sv[loop].siglevel > siglevel2 && posdat.sv[loop].siglevel < siglevel1 ) )
                        {
                            checkSNR_PassFlag = false;
                        }
                        loop++;
                    }
                    if(checkSNR_PassFlag)
                    {
                        availableSNR += 1.0f;
                        //printf("availableSNR =%f\n", availableSNR);
                    }
                }
            }
            else if(sig_no == sig_pvtlog)
            {
                struct cxd56_pvtlog_s pvtlogdat;
                savePvtLogToLocal(pvtlogdat);
            }

            gettimeofday(&tvCurrent, &tz);
            if(tvEnd.tv_sec == tvCurrent.tv_sec && tvEnd.tv_usec <= tvCurrent.tv_usec || tvEnd.tv_sec < tvCurrent.tv_sec || availableSNR >= (time * 1000 / cycle))
                break;
        }
    }
    availableSNR = ((1.0f * cycle * availableSNR) / (time * 1000));
    return availableSNR;
}

double gnss_api::gnss_check_PosAccuracy(int time)
{
    double ret = 0.0f;
    bool calcPosAccuracy = false;

    struct cxd56_gnss_dms_s      dmf;
    int cmdRet = ioctl(gnss_fd, CXD56_GNSS_IOCTL_PVTLOG_DELETE_LOG, 0);
    if (cmdRet < 0)
    {
        printf("Delete log error\n");
        return ret;
    }
    if((stateMachine & kGnssStarting) == kGnssStarting)
    {
        timespec timeout;
        timeout.tv_sec = (10 * cycle / 1000);
        timeout.tv_nsec = 0;
        int sig_no = sigtimedwait(&mask, NULL, &timeout);

        struct timeval    tvCurrent;
        struct timeval    tvEnd;
        struct timezone   tz;
        gettimeofday(&tvEnd, &tz);
        tvEnd.tv_sec += time;

        while(1)
        {
            sig_no = sigtimedwait(&mask, NULL, &timeout);



            if(sig_no == sig_gnss)
            {
                struct cxd56_gnss_positiondata_s posdat;
                if((stateMachine & kGnssStartingPVT) == kGnssStartingPVT)
                {
                    cmdRet = lseek(gnss_fd, CXD56_GNSS_READ_OFFSET_LAST_GNSS, SEEK_SET);
                }
                /* Read POS data. */
                memset(&posdat, 0, sizeof(posdat));
                cmdRet = read(gnss_fd, &posdat, sizeof(posdat));
                DebugPrintfSatellites(posdat);
                if(cmdRet == sizeof(posdat) && !calcPosAccuracy)
                {
                    PositionData *pPosData = (PositionData *)malloc(sizeof(PositionData));
                    if(pPosData)
                    {
                        pPosData->latitude = posdat.receiver.latitude;
                        pPosData->longitude = posdat.receiver.longitude;
                        pPosData->altitude = posdat.receiver.altitude;
                        pPosData->pNext = NULL;
                        if(pPosDataList == NULL)
                        {
                            pPosDataList = pPosData;
                        }
                        else
                        {
                            PositionData *pPosDataEnd = pPosDataList;
                            while(pPosDataEnd->pNext)
                            {
                                pPosDataEnd = pPosDataEnd->pNext;
                            }
                            pPosDataEnd->pNext = pPosData;
                        }
                    }
                }
                if(calcPosAccuracy)
                {
                    if(pPosDataList != NULL)
                    {
                        ret = BLH2XYZ_To_Accuracy(pPosDataList);
                    }
                    break;
                }
            }
            else if(sig_no == sig_pvtlog)
            {
                struct cxd56_pvtlog_s pvtlogdat;
                savePvtLogToLocal(pvtlogdat);
            }

            gettimeofday(&tvCurrent, &tz);
            if(tvEnd.tv_sec == tvCurrent.tv_sec  || tvEnd.tv_sec < tvCurrent.tv_sec)
                calcPosAccuracy = true;
        }
    }
    return ret;
}

bool gnss_api::gnss_save_check_PVTLog(int time)
{
    bool ret = true;
    bool waitingForLastPvtlog = false;

    struct cxd56_gnss_dms_s      dmf;
    struct cxd56_pvtlog_s pvtlogdat, posdatlog;
    int posdatlogIndex = 0;
    int posdatlogLoop = 0;
    int pvtlogdatLoop = 0;
    int cmdRet = ioctl(gnss_fd, CXD56_GNSS_IOCTL_PVTLOG_DELETE_LOG, 0);
    if (cmdRet < 0)
    {
        printf("Delete log error\n");
        return false;
    }
    if((stateMachine & kGnssStarting) == kGnssStarting)
    {
        timespec timeout;
        timeout.tv_sec = (10 * cycle / 1000);
        timeout.tv_nsec = 0;
        int sig_no = sigtimedwait(&mask, NULL, &timeout);

        struct timeval    tvCurrent;
        struct timeval    tvEnd;
        struct timezone   tz;
        gettimeofday(&tvEnd, &tz);
        tvEnd.tv_sec += time;

        while(1)
        {
            sig_no = sigtimedwait(&mask, NULL, &timeout);


            if(sig_no == sig_gnss)
            {
                struct cxd56_gnss_positiondata_s posdat;
                if((stateMachine & kGnssStartingPVT) == kGnssStartingPVT)
                {
                    cmdRet = lseek(gnss_fd, CXD56_GNSS_READ_OFFSET_LAST_GNSS, SEEK_SET);
                }
                /* Read POS data. */
                memset(&posdat, 0, sizeof(posdat));
                cmdRet = read(gnss_fd, &posdat, sizeof(posdat));
                DebugPrintfSatellites(posdat);
                if(cmdRet == sizeof(posdat) && !waitingForLastPvtlog && posdatlogIndex < CXD56_GNSS_PVTLOG_MAXNUM)
                {
                    double_to_dmf(posdat.receiver.latitude, &dmf);
                    posdatlog.log_data[posdatlogIndex].latitude.degree = dmf.degree;
                    posdatlog.log_data[posdatlogIndex].latitude.minute = dmf.minute;
                    posdatlog.log_data[posdatlogIndex].latitude.frac = dmf.frac;
                    posdatlog.log_data[posdatlogIndex].latitude.sign = dmf.sign;
                    double_to_dmf(posdat.receiver.longitude, &dmf);
                    posdatlog.log_data[posdatlogIndex].longitude.degree = dmf.degree;
                    posdatlog.log_data[posdatlogIndex].longitude.minute = dmf.minute;
                    posdatlog.log_data[posdatlogIndex].longitude.frac = dmf.frac;
                    posdatlog.log_data[posdatlogIndex].longitude.sign = dmf.sign;

                    posdatlog.log_data[posdatlogIndex].date.year     = (posdat.receiver.date.year % 100);
                    posdatlog.log_data[posdatlogIndex].date.month     = posdat.receiver.date.month;
                    posdatlog.log_data[posdatlogIndex].date.day     = posdat.receiver.date.day;
                    posdatlog.log_data[posdatlogIndex].time.hour     = posdat.receiver.time.hour;
                    posdatlog.log_data[posdatlogIndex].time.minute     = posdat.receiver.time.minute;
                    posdatlog.log_data[posdatlogIndex].time.sec     = posdat.receiver.time.sec;
                    posdatlog.log_data[posdatlogIndex].time.msec     = posdat.receiver.time.usec;
                    posdatlogIndex++;
                }
                if(posdatlogIndex >= CXD56_GNSS_PVTLOG_MAXNUM)
                {
                    printf("Failed %d\n", posdatlogIndex++);
                }
            }
            else if(sig_no == sig_pvtlog)
            {
                struct cxd56_pvtlog_s pvtlogdat;
                savePvtLogToLocal(pvtlogdat);

                //check pvt log
                for(posdatlogLoop = 0; posdatlogLoop < posdatlogIndex; posdatlogLoop++)
                {
                    for(pvtlogdatLoop = 0; pvtlogdatLoop < CXD56_GNSS_PVTLOG_MAXNUM; pvtlogdatLoop++)
                    {
                        if(
                            posdatlog.log_data[posdatlogLoop].date.year       == pvtlogdat.log_data[pvtlogdatLoop].date.year         &&
                            posdatlog.log_data[posdatlogLoop].date.month      == pvtlogdat.log_data[pvtlogdatLoop].date.month        &&
                            posdatlog.log_data[posdatlogLoop].date.day        == pvtlogdat.log_data[pvtlogdatLoop].date.day          &&
                            posdatlog.log_data[posdatlogLoop].time.hour       == pvtlogdat.log_data[pvtlogdatLoop].time.hour         &&
                            posdatlog.log_data[posdatlogLoop].time.minute     == pvtlogdat.log_data[pvtlogdatLoop].time.minute       &&
                            posdatlog.log_data[posdatlogLoop].time.sec        == pvtlogdat.log_data[pvtlogdatLoop].time.sec
                        )
                        {
                            if(
                                posdatlog.log_data[posdatlogLoop].latitude.degree != pvtlogdat.log_data[pvtlogdatLoop].latitude.degree   ||
                                posdatlog.log_data[posdatlogLoop].latitude.minute != pvtlogdat.log_data[pvtlogdatLoop].latitude.minute   ||
                                posdatlog.log_data[posdatlogLoop].latitude.frac   != pvtlogdat.log_data[pvtlogdatLoop].latitude.frac     ||
                                posdatlog.log_data[posdatlogLoop].latitude.sign   != pvtlogdat.log_data[pvtlogdatLoop].latitude.sign     ||
                                posdatlog.log_data[posdatlogLoop].longitude.degree != pvtlogdat.log_data[pvtlogdatLoop].longitude.degree  ||
                                posdatlog.log_data[posdatlogLoop].longitude.minute != pvtlogdat.log_data[pvtlogdatLoop].longitude.minute  ||
                                posdatlog.log_data[posdatlogLoop].longitude.frac  != pvtlogdat.log_data[pvtlogdatLoop].longitude.frac    ||
                                posdatlog.log_data[posdatlogLoop].longitude.sign  != pvtlogdat.log_data[pvtlogdatLoop].longitude.sign
                            )
                            {
                                ret = false;
                                printf("PositionData[%d] LAT and LNG are not in pvt log:\n [%d-%2d-%2d_%d:%d:%d]LAT(%d.%d.%04d)LNG(%d.%d.%04d) \n",
                                       posdatlogLoop,
                                       posdatlog.log_data[posdatlogLoop].date.year,
                                       posdatlog.log_data[posdatlogLoop].date.month,
                                       posdatlog.log_data[posdatlogLoop].date.day,
                                       posdatlog.log_data[posdatlogLoop].time.hour,
                                       posdatlog.log_data[posdatlogLoop].time.minute,
                                       posdatlog.log_data[posdatlogLoop].time.sec,
                                       posdatlog.log_data[posdatlogLoop].latitude.degree,
                                       posdatlog.log_data[posdatlogLoop].latitude.minute,
                                       posdatlog.log_data[posdatlogLoop].latitude.frac,
                                       posdatlog.log_data[posdatlogLoop].longitude.degree,
                                       posdatlog.log_data[posdatlogLoop].longitude.minute,
                                       posdatlog.log_data[posdatlogLoop].longitude.frac);
                            }
                            break;
                        }
                    }
                    if(pvtlogdatLoop >= CXD56_GNSS_PVTLOG_MAXNUM)
                    {
                        printf("PositionData[%d] date and time are not in pvt log:\n [%d-%2d-%2d_%d:%d:%d]LAT(%d.%d.%04d)LNG(%d.%d.%04d) \n",
                               posdatlogLoop,
                               posdatlog.log_data[posdatlogLoop].date.year,
                               posdatlog.log_data[posdatlogLoop].date.month,
                               posdatlog.log_data[posdatlogLoop].date.day,
                               posdatlog.log_data[posdatlogLoop].time.hour,
                               posdatlog.log_data[posdatlogLoop].time.minute,
                               posdatlog.log_data[posdatlogLoop].time.sec,
                               posdatlog.log_data[posdatlogLoop].latitude.degree,
                               posdatlog.log_data[posdatlogLoop].latitude.minute,
                               posdatlog.log_data[posdatlogLoop].latitude.frac,
                               posdatlog.log_data[posdatlogLoop].longitude.degree,
                               posdatlog.log_data[posdatlogLoop].longitude.minute,
                               posdatlog.log_data[posdatlogLoop].longitude.frac);
                        ret = false;
                    }
                }
                posdatlogIndex = 0;
                if(waitingForLastPvtlog)
                    break;
            }
            gettimeofday(&tvCurrent, &tz);

            if(tvEnd.tv_sec == tvCurrent.tv_sec || tvEnd.tv_sec < tvCurrent.tv_sec)
                waitingForLastPvtlog = true;
        }
    }
    return ret;
}

float gnss_api::gnss_CheckPositioning_Continuous(int time)
{
    float pos_count = 0.0f;
    if((stateMachine & kGnssStarting) == kGnssStarting)
    {
        timespec timeout;
        timeout.tv_sec = (10 * cycle / 1000);
        timeout.tv_nsec = 0;
        int sig_no = sigtimedwait(&mask, NULL, &timeout);

        struct timeval    tvCurrent;
        struct timeval    tvEnd;
        struct timezone   tz;
        gettimeofday(&tvEnd, &tz);
        tvEnd.tv_sec += time;

        while(1)
        {
            sig_no = sigtimedwait(&mask, NULL, &timeout);


            if(sig_no == sig_gnss)
            {
                struct cxd56_gnss_positiondata_s posdat;
                // Read POS data.
                memset(&posdat, 0, sizeof(posdat));
                int ret = read(gnss_fd, &posdat, sizeof(posdat));
                DebugPrintfSatellites(posdat);
                if (ret == sizeof(posdat))
                {
                    struct cxd56_gnss_dms_s      dmf;
                    double_to_dmf(posdat.receiver.latitude, &dmf);
                    printf("LAT %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);
                    double_to_dmf(posdat.receiver.longitude, &dmf);
                    printf("LNG %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);
                    checkNMEAlog();
                    NMEA_Output(&posdat);
                    if(checkNMEAlog() && posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
                    {
                        pos_count += 1.0f;
                        //printf("pos_count =%f\n", pos_count);
                    }
                }
            }
            else if(sig_no == sig_pvtlog)
            {
                struct cxd56_pvtlog_s pvtlogdat;
                savePvtLogToLocal(pvtlogdat);
            }

            gettimeofday(&tvCurrent, &tz);
            if(tvEnd.tv_sec == tvCurrent.tv_sec && tvEnd.tv_usec <= tvCurrent.tv_usec  || tvEnd.tv_sec < tvCurrent.tv_sec ||  pos_count >= (time * 1000 / cycle))
                break;

        }
    }
    return ((1.0f * cycle * pos_count) / (time * 1000));
}


float gnss_api::gnss_CheckNMEA_Continuous(int time)
{
    float nmea_count = 0.0f;
    if((stateMachine & kGnssStarting) == kGnssStarting)
    {
        timespec timeout;
        timeout.tv_sec = (10 * cycle / 1000);
        timeout.tv_nsec = 0;
        int sig_no = sigtimedwait(&mask, NULL, &timeout);

        if(sig_no == sig_gnss)
        {
            struct cxd56_gnss_positiondata_s posdat;
            // Read POS data.
            memset(&posdat, 0, sizeof(posdat));
            int ret = read(gnss_fd, &posdat, sizeof(posdat));
            DebugPrintfSatellites(posdat);
        }

        struct timeval    tvCurrent;
        struct timeval    tvEnd;
        struct timezone   tz;
        gettimeofday(&tvEnd, &tz);
        tvEnd.tv_sec += time;

        while(1)
        {
            sig_no = sigtimedwait(&mask, NULL, &timeout);
            if(sig_no == sig_gnss)
            {
                struct cxd56_gnss_positiondata_s posdat;
                // Read POS data.
                memset(&posdat, 0, sizeof(posdat));
                int ret = read(gnss_fd, &posdat, sizeof(posdat));
                DebugPrintfSatellites(posdat);
                if (ret == sizeof(posdat))
                {
                    struct cxd56_gnss_dms_s      dmf;
                    double_to_dmf(posdat.receiver.latitude, &dmf);
                    printf("LAT %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);
                    double_to_dmf(posdat.receiver.longitude, &dmf);
                    printf("LNG %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);
                    checkNMEAlog();
                    NMEA_Output(&posdat);
                    if(checkNMEAlog())
                    {
                        nmea_count += 1.0f;
                        //printf("nmea_count =%f\n", nmea_count);
                    }
                }
            }
            else if(sig_no == sig_pvtlog)
            {
                struct cxd56_pvtlog_s pvtlogdat;
                savePvtLogToLocal(pvtlogdat);
            }
            gettimeofday(&tvCurrent, &tz);
            if(tvEnd.tv_sec == tvCurrent.tv_sec && tvEnd.tv_usec <= tvCurrent.tv_usec  || tvEnd.tv_sec < tvCurrent.tv_sec || nmea_count >= (time * 1000 / cycle))
                break;
        }
    }
    return ((1.0f * cycle * nmea_count) / (time * 1000));
}

void gnss_api::executeResult(int cmdRet, char *cmdStr)
{
    (cmdRet < 0) ? printf("FAILED: gnss ") : printf("PASS: gnss ");
    printf("%s\n", cmdStr);
}

bool gnss_api::executeCommand(Command_ST cmd)
{
    bool ret = true;

    switch(cmd.type )
    {
    case kQuit:
    {
        ret = false;
        break;
    }
    case kStart:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, cmd.setting.start_setting);

        executeResult(cmdRet, cmd.cmdStr);
        if(cmdRet >= 0)
        {
            startMode = cmd.setting.start_setting;
            //get start time
            struct timezone tz;
            gettimeofday(&startTime, &tz);
            stateMachine |= kGnssStarting;
            gnss_getFirstPOS();
        }
        break;
    }
    case kStop:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, cmd.setting.stop_setting);
        executeResult(cmdRet, cmd.cmdStr);
        if(cmdRet >= 0)
        {
            stateMachine = kGnssIDEL;
        }
        break;
    }
    case kSetSatellite:
    {
        satellite_setting = cmd.setting.satellite_setting;
        int cmdRet = ioctl(gnss_fd, cmd.command_id, cmd.setting.satellite_setting);
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kGetSatellite:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (uint32_t) & (cmd.setting.satellite_setting));
        executeResult(cmdRet, cmd.cmdStr);
        if(cmdRet >= 0)
        {
            if((cmd.setting.satellite_setting & CXD56_GNSS_SAT_GPS) == CXD56_GNSS_SAT_GPS)
            {
                printf("GPS ");
            }
            if((cmd.setting.satellite_setting & CXD56_GNSS_SAT_GLONASS) == CXD56_GNSS_SAT_GLONASS)
            {
                printf("GLONASS ");
            }
            if((cmd.setting.satellite_setting & CXD56_GNSS_SAT_QZ_L1CA) == CXD56_GNSS_SAT_QZ_L1CA)
            {
                printf("L1CA ");
            }
            if((cmd.setting.satellite_setting & CXD56_GNSS_SAT_QZ_L1S) == CXD56_GNSS_SAT_QZ_L1S)
            {
                printf("L1S");
            }
            printf("\n");
        }
        break;
    }
    case kSetEllipsoidalPosition:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (uint32_t) & (cmd.setting.ellipsoidal_position));
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kSetOrthogonalPosition:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (uint32_t) & (cmd.setting.orthogonal_position));
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kSetOPE:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (uint32_t) & (cmd.setting.opemode_setting));
        executeResult(cmdRet, cmd.cmdStr);
        if(cmdRet >= 0)
        {
            cycle = cmd.setting.opemode_setting.cycle;
        }
        break;
    }
    case kGetOPE:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (uint32_t) & (cmd.setting.opemode_setting));
        executeResult(cmdRet, cmd.cmdStr);
        if(cmdRet >= 0)
        {
            printf("mode=%d\n", cmd.setting.opemode_setting.mode);
            printf("cycle=%d\n", cmd.setting.opemode_setting.cycle);
        }
        break;
    }
    case kSetTCXO_Offset:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, cmd.setting.tcxo_setting);
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kGetTCXO_Offset:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.tcxo_setting));
        executeResult(cmdRet, cmd.cmdStr);
        if(cmdRet >= 0)
        {
            printf("offset=%d\n", cmd.setting.tcxo_setting);
        }
        break;
    }
    case kSetAlmanac:
    {
        if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_GPS)
        {
            uint32_t bufferSize = readSD0(cmd.setting2.localSDNumber, "/mnt/spif/Almanac_GPS_", pGPS_AlmanacBuffer, CXD56_GNSS_GPS_ALMANAC_SIZE);
            if(bufferSize > 0)
            {
                cmd.setting.orbitalparam.data = (uint32_t *)pGPS_AlmanacBuffer;
                int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
                executeResult(cmdRet, cmd.cmdStr);
            }
            else
                executeResult(-1, cmd.cmdStr);
        }
        else if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_GLONASS)
        {
            uint32_t bufferSize = readSD0(cmd.setting2.localSDNumber, "/mnt/spif/Almanac_GLONASS_", pGLONASS_AlmanacBuffer, CXD56_GNSS_GLONASS_ALMANAC_SIZE);
            if(bufferSize > 0)
            {
                cmd.setting.orbitalparam.data = (uint32_t *)pGLONASS_AlmanacBuffer;
                int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
                executeResult(cmdRet, cmd.cmdStr);
            }
            else
                executeResult(-1, cmd.cmdStr);
        }
        else if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_QZSSL1CA)
        {
            uint32_t bufferSize = readSD0(cmd.setting2.localSDNumber, "/mnt/spif/Almanac_L1CA_", pQZSSL1CA_AlmanacBuffer, CXD56_GNSS_QZSSL1CA_ALMANAC_SIZE);
            if(bufferSize > 0)
            {
                cmd.setting.orbitalparam.data = (uint32_t *)pQZSSL1CA_AlmanacBuffer;
                int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
                executeResult(cmdRet, cmd.cmdStr);
            }
            else
                executeResult(-1, cmd.cmdStr);
        }
        break;
    }
    case kGetAlmanac:
    {
        if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_GPS)
        {
            memset(pGPS_AlmanacBuffer, CXD56_GNSS_GPS_ALMANAC_SIZE, 0);
            cmd.setting.orbitalparam.data = (uint32_t *)pGPS_AlmanacBuffer;
            int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
            executeResult(cmdRet, cmd.cmdStr);
            if(cmdRet >= 0)
            {
                uint32_t bufferSize = writeSD0(cmd.setting2.localSDNumber, "/mnt/spif/Almanac_GPS_", pGPS_AlmanacBuffer, CXD56_GNSS_GPS_ALMANAC_SIZE);
                if(bufferSize > 0)
                {
                    char filename[FILE_NAME_LEN] = {0,};
                    snprintf(filename, FILE_NAME_LEN, "%s%d", "/mnt/spif/Almanac_GPS_", cmd.setting2.localSDNumber);
                    printf("file=%s\n",  filename);
                }
                else
                    executeResult(-1, cmd.cmdStr);
            }
        }
        else if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_GLONASS)
        {
            memset(pGLONASS_AlmanacBuffer, CXD56_GNSS_GLONASS_ALMANAC_SIZE, 0);
            cmd.setting.orbitalparam.data = (uint32_t *)pGLONASS_AlmanacBuffer;
            int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
            executeResult(cmdRet, cmd.cmdStr);
            if(cmdRet >= 0)
            {
                uint32_t bufferSize = writeSD0(cmd.setting2.localSDNumber, "/mnt/spif/Almanac_GLONASS_", pGLONASS_AlmanacBuffer, CXD56_GNSS_GLONASS_ALMANAC_SIZE);
                if(bufferSize > 0)
                {
                    char filename[FILE_NAME_LEN] = {0,};
                    snprintf(filename, FILE_NAME_LEN, "%s%d", "/mnt/spif/Almanac_GLONASS_", cmd.setting2.localSDNumber);
                    printf("file=%s\n",  filename);
                }
                else
                    executeResult(-1, cmd.cmdStr);

            }
        }
        else if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_QZSSL1CA)
        {
            memset(pQZSSL1CA_AlmanacBuffer, CXD56_GNSS_QZSSL1CA_ALMANAC_SIZE, 0);
            cmd.setting.orbitalparam.data = (uint32_t *)pQZSSL1CA_AlmanacBuffer;
            int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
            executeResult(cmdRet, cmd.cmdStr);
            if(cmdRet >= 0)
            {
                uint32_t bufferSize = writeSD0(cmd.setting2.localSDNumber, "/mnt/spif/Almanac_L1CA_", pQZSSL1CA_AlmanacBuffer, CXD56_GNSS_QZSSL1CA_ALMANAC_SIZE);
                if(bufferSize > 0)
                {
                    char filename[FILE_NAME_LEN] = {0,};
                    snprintf(filename, FILE_NAME_LEN, "%s%d", "/mnt/spif/Almanac_L1CA_", cmd.setting2.localSDNumber);
                    printf("file=%s\n",  filename);
                }
                else
                    executeResult(-1, cmd.cmdStr);
            }
        }
        break;
    }
    case kSetTime:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (uint32_t) & (cmd.setting.settime));
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kSetEphemeris:
    {
        if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_GPS)
        {
            readSD0(cmd.setting2.localSDNumber, "/mnt/spif/Ephemeris_GPS_", pGPS_EphemerisBuffer, CXD56_GNSS_GPS_EPHEMERIS_SIZE);
            cmd.setting.orbitalparam.data = (uint32_t *)pGPS_EphemerisBuffer;
            int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
            executeResult(cmdRet, cmd.cmdStr);
        }
        else if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_GLONASS)
        {
            readSD0(cmd.setting2.localSDNumber, "/mnt/spif/Ephemeris_GLONASS_", pGLONASS_EphemerisBuffer, CXD56_GNSS_GLONASS_EPHEMERIS_SIZE);
            cmd.setting.orbitalparam.data = (uint32_t *)pGLONASS_EphemerisBuffer;
            int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
            executeResult(cmdRet, cmd.cmdStr);

        }
        else if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_QZSSL1CA)
        {
            readSD0(cmd.setting2.localSDNumber, "/mnt/spif/Ephemeris_L1CA_", pQZSSL1CA_EphemerisBuffer, CXD56_GNSS_QZSSL1CA_EPHEMERIS_SIZE);
            cmd.setting.orbitalparam.data = (uint32_t *)pQZSSL1CA_EphemerisBuffer;
            int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
            executeResult(cmdRet, cmd.cmdStr);
        }
        break;
    }
    case kGetEphemeris:
    {
        if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_GPS)
        {
            memset(pGPS_EphemerisBuffer, CXD56_GNSS_GPS_EPHEMERIS_SIZE, 0);
            cmd.setting.orbitalparam.data = (uint32_t *)pGPS_EphemerisBuffer;
            int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
            executeResult(cmdRet, cmd.cmdStr);
            if(cmdRet >= 0)
            {
                writeSD0(cmd.setting2.localSDNumber, "/mnt/spif/Ephemeris_GPS_", pGPS_EphemerisBuffer, CXD56_GNSS_GPS_EPHEMERIS_SIZE);
                char filename[FILE_NAME_LEN] = {0,};
                snprintf(filename, FILE_NAME_LEN, "%s%d", "/mnt/spif/Ephemeris_GPS_", cmd.setting2.localSDNumber);
                printf("file=%s\n", filename);
            }
        }
        else if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_GLONASS)
        {
            memset(pGLONASS_EphemerisBuffer, CXD56_GNSS_GLONASS_EPHEMERIS_SIZE, 0);
            cmd.setting.orbitalparam.data = (uint32_t *)pGLONASS_EphemerisBuffer;
            int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
            executeResult(cmdRet, cmd.cmdStr);
            if(cmdRet >= 0)
            {
                writeSD0(cmd.setting2.localSDNumber, "/mnt/spif/Ephemeris_GLONASS_", pGLONASS_EphemerisBuffer, CXD56_GNSS_GLONASS_EPHEMERIS_SIZE);
                char filename[FILE_NAME_LEN] = {0,};
                snprintf(filename, FILE_NAME_LEN, "%s%d", "/mnt/spif/Ephemeris_GLONASS_", cmd.setting2.localSDNumber);
                printf("file=%s\n", filename);
            }
        }
        else if(cmd.setting.orbitalparam.type == CXD56_GNSS_DATA_QZSSL1CA)
        {
            memset(pQZSSL1CA_EphemerisBuffer, CXD56_GNSS_QZSSL1CA_EPHEMERIS_SIZE, 0);
            cmd.setting.orbitalparam.data = (uint32_t *)pQZSSL1CA_EphemerisBuffer;
            int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.orbitalparam));
            executeResult(cmdRet, cmd.cmdStr);
            if(cmdRet >= 0)
            {
                writeSD0(cmd.setting2.localSDNumber, "/mnt/spif/Ephemeris_L1CA_", pQZSSL1CA_EphemerisBuffer, CXD56_GNSS_QZSSL1CA_EPHEMERIS_SIZE);
                char filename[FILE_NAME_LEN] = {0,};
                snprintf(filename, FILE_NAME_LEN, "%s%d", "/mnt/spif/Ephemeris_L1CA_", cmd.setting2.localSDNumber);
                printf("file=%s\n", filename);
            }
        }
        break;
    }
    case kSaveBackupData:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, 0);
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kEraseBackupData:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, 0);
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kSignalSet:
    {

        switch(cmd.setting.signal_setting.gnsssig)
        {
        case CXD56_GNSS_SIG_GNSS:
            sig_gnss = cmd.setting.signal_setting.signo;
            break;
        case CXD56_GNSS_SIG_PVTLOG:
            sig_pvtlog = cmd.setting.signal_setting.signo;
            break;
        case CXD56_GNSS_SIG_AGPS:
            sig_agps = cmd.setting.signal_setting.signo;
            break;
        case CXD56_GNSS_SIG_RTK:
            sig_rtk = cmd.setting.signal_setting.signo;
            break;
        case CXD56_GNSS_SIG_SPECTRUM:
            sig_spectrum = cmd.setting.signal_setting.signo;
            break;
        case CXD56_GNSS_SIG_GPSEPHEMERIS:
            sig_gpsephemeris = cmd.setting.signal_setting.signo;
            break;
        case CXD56_GNSS_SIG_GLNEPHEMERIS:
            sig_glnephemeris = cmd.setting.signal_setting.signo;
            break;
        case CXD56_GNSS_SIG_SBAS:
            sig_sbas = cmd.setting.signal_setting.signo;
            break;
        }

        sigset_t *pmask = &mask;
        sigaddset(pmask, cmd.setting.signal_setting.signo);
        int cmdRet = sigprocmask(SIG_BLOCK, pmask, NULL);
        if (cmdRet == OK)
        {
            cmd.setting.signal_setting.fd = gnss_fd;
            cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long) & (cmd.setting.signal_setting));
            executeResult(cmdRet, cmd.cmdStr);
        }
        else
            executeResult(-1, "sigSet");

        break;
    }
    case kPvtLogStart:
    {
        pvtLogIndex = 1;
        struct cxd56_pvtlog_setting_s pvtlog_setting;
        pvtlog_setting.cycle     = TEST_RECORDING_CYCLE;
        pvtlog_setting.threshold = TEST_NOTIFY_THRESHOLD;
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long)&pvtlog_setting);
        executeResult(cmdRet, cmd.cmdStr);
        stateMachine |= kGnssStartingPVT;
        break;
    }
    case kPvtLogStop:
    {
        pvtLogIndex = 0;
        int cmdRet = ioctl(gnss_fd, cmd.command_id, 0);
        executeResult(cmdRet, cmd.cmdStr);
        stateMachine &= (~kGnssStartingPVT);
        break;
    }
    case kPvtDeleteLog:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, 0);
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kOpenCEP:
    case kCloseCEP:
    case kCheckCEP:
    case kResetCEP:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, 0);
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kGetCEP:
    {
        struct cxd56_gnss_cep_age_s cep_age = { 0.0, 0.0 };
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long)&cep_age);
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kFactoryStop:
    {
        int cmdRet = ioctl(gnss_fd, cmd.command_id, 0);
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kFactoryGetResult:
    {
        struct cxd56_gnss_test_result_s get;
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long)&get);
        executeResult(cmdRet, cmd.cmdStr);
        if (cmdRet == OK)
        {
            printf("cn=%d\n", (int)(get.cn * 1000000));
            printf("doppler=%ld\n", (int64_t)(get.doppler * 1000000));
        }
        break;
    }
    case kFactoryStart:
    {
        struct cxd56_gnss_test_info_s setparam =  { 1, 0, 0, 0 };
        int cmdRet = ioctl(gnss_fd, cmd.command_id,  (unsigned long)&setparam);
        executeResult(cmdRet, cmd.cmdStr);
        break;
    }
    case kPvtStatusGet:
    {
        struct cxd56_pvtlog_status_s pvtlog_status;
        int cmdRet = ioctl(gnss_fd, cmd.command_id, (unsigned long)&pvtlog_status);
        executeResult(cmdRet, cmd.cmdStr);
        if(cmdRet >= 0)
        {
            printf("PVT_Log:No=%d\n", pvtlog_status.status.log_count);
        }
        break;
    }
    case kSaveConfig:
    {
        if(stateMachine == kGnssIDEL)
        {
            char buffer[4096 * 16] = {0,};
            int bufferSize = 0;
            FILE *fp_read = fopen(CONFIG_CXD56_GNSS_BACKUP_FILENAME, "rb");
            char *pBegin = buffer;
            if(fp_read != NULL)
            {
                int32_t readCount = 0;
                do
                {
                    int c = fgetc(fp_read);
                    if( feof(fp_read) )
                    {
                        break ;
                    }
                    else
                    {
                        bufferSize++;
                        readCount++;
                        *pBegin = 0x000000FF & c;
                        pBegin++;
                    }
                }
                while(readCount < sizeof(buffer));
                fclose(fp_read);
                fp_read = 0;
            }

            if (bufferSize > 0)
            {
                struct timezone tz;
                gettimeofday(&(localBackupFileTime[cmd.setting.localSDNumber]), &tz);
                int cmdRet = writeSD0(cmd.setting.localSDNumber, "/mnt/spif/gnss_config_", buffer, bufferSize);
                executeResult(cmdRet, cmd.cmdStr);
            }
            else
            {
                executeResult(-1, cmd.cmdStr);
            }
        }
        else
        {
            executeResult(-1, cmd.cmdStr);
        }
        break;
    }
    case kLoadConfig:
    {
        if(stateMachine == kGnssIDEL)
        {
            struct timeval    tv;
            struct timezone tz;
            gettimeofday(&tv, &tz);
            if((tv.tv_sec - localBackupFileTime[cmd.setting.localSDNumber].tv_sec) < (60 * 60) &&
                    localBackupFileTime[cmd.setting.localSDNumber].tv_sec != 0)
            {
                char buffer[4096 * 16] = {0,};
                int cmdRet = readSD0(cmd.setting.localSDNumber, "/mnt/spif/gnss_config_", buffer, sizeof(buffer));
                int bufferSize = 0;
                FILE *fp_write = fopen(CONFIG_CXD56_GNSS_BACKUP_FILENAME, "wb");
                char *pBegin = buffer;
                if (fp_write != NULL)
                {
                    char *pEnd = pBegin + cmdRet;
                    while(pBegin < pEnd)
                    {
                        fputc(*pBegin, fp_write);
                        pBegin++;
                        bufferSize++;
                    }
                    fclose(fp_write);
                    fp_write = 0;
                }
                if (bufferSize == cmdRet)
                {
                    executeResult(cmdRet, cmd.cmdStr);
                }
                else
                {
                    executeResult(-1, cmd.cmdStr);
                }
            }
            else
            {
                printf("Backup file is out of data\n");
            }
        }
        else
        {
            executeResult(-1, cmd.cmdStr);
        }
        break;
    }
    case kPause:
    {
        sleep(cmd.setting.time);
        executeResult(1, cmd.cmdStr);
        break;
    }
    case kCheckGnssNMEA:
    {
        float cmdRet = gnss_CheckNMEA_Continuous(cmd.setting.time);
        if(cmdRet == 1.0f)
            executeResult(1, cmd.cmdStr);
        else
            executeResult(-1, cmd.cmdStr);
        printf("NMEA=%1.2f\n", cmdRet);
        break;
    }

    case kCheckPvtLog:
    {
        bool cmdRet = gnss_save_check_PVTLog(cmd.setting.time);
        if(cmdRet)
            executeResult(1, cmd.cmdStr);
        else
            executeResult(-1, cmd.cmdStr);
        break;
    }
    case kCheckGnssPosAccuracy:
    {
        double cmdRet = gnss_check_PosAccuracy(cmd.setting.time);
        if((cmd.setting1.value <= cmdRet) && (cmdRet <= cmd.setting2.value) ||
                (cmd.setting2.value <= cmdRet) && (cmdRet <= cmd.setting1.value))
            executeResult(1, cmd.cmdStr);
        else
            executeResult(-1, cmd.cmdStr);
        printf("PosAccuracy=%1.2f\n", cmdRet);
        break;
    }
    case kCheckSNR:
    {
        double cmdRet = gnss_check_SNR(cmd.setting.time, cmd.setting1.value, cmd.setting2.value);
        if(cmdRet == 1.0f)
            executeResult(1, cmd.cmdStr);
        else
            executeResult(-1, cmd.cmdStr);
        printf("SNR=%1.2f\n", cmdRet);
        break;
    }
    case kCheckGnssPosition:
    {
        float cmdRet = gnss_CheckPositioning_Continuous(cmd.setting.time);
        if(cmdRet >= cmd.setting1.value)
            executeResult(1, cmd.cmdStr);
        else
            executeResult(-1, cmd.cmdStr);
        printf("Position=%1.2f\n", cmdRet);
        break;
    }
    case kCheckOffline:
    {
        float cmdRet = gnss_CheckPositioning_Continuous(cmd.setting.time);
        if(cmdRet == cmd.setting1.value)
            executeResult(1, cmd.cmdStr);
        else
            executeResult(-1, cmd.cmdStr);
        printf("Position(OffLine)=%1.2f\n", cmdRet);
        break;
    }
    case kCheckGnssTTFF:
    {
        if((stateMachine & kGnssStarting) && (stateMachine & kGnssPosition))
        {

            if(cmd.setting.time <= cmd.setting2.time)
            {
                if((positionTime.tv_sec - startTime.tv_sec) >= cmd.setting.time &&
                        (positionTime.tv_sec - startTime.tv_sec) <= cmd.setting2.time)
                    executeResult(1, cmd.cmdStr);
                else
                    executeResult(-1, cmd.cmdStr);
            }
            else
            {
                if((positionTime.tv_sec - startTime.tv_sec) >= cmd.setting2.time &&
                        (positionTime.tv_sec - startTime.tv_sec) <= cmd.setting.time)
                    executeResult(1, cmd.cmdStr);
                else
                    executeResult(-1, cmd.cmdStr);

            }
            printf("ttff time= %d second\n", positionTime.tv_sec - startTime.tv_sec);
        }
        else
            executeResult(-1, cmd.cmdStr);
        break;
    }
    }
    return ret;
}

