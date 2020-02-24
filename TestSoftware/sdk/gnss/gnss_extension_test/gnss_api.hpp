//***************************************************************************
// test/sqa/singlefunction/gnss_extension_test/gnss_api.hpp
//
//   Data  : 2019/01/31
//   Author: Neusoft
//***************************************************************************
#include <sys/time.h>
#include "common_def.hpp"
#include "gpsutils/cxd56_gnss_nmea.h"

#ifndef gnss_api_HPP_
#define gnss_api_HPP_ 1
class gnss_api {
public:
    const int kGnssIDEL        = 0x00;
    const int kGnssStarting    = 0x01;
    const int kGnssStartingPVT = 0x02;
    const int kGnssPosition    = 0x04;
    struct PositionData
    {
        double x;
        double y;
        double z;
        double longitude;
        double latitude;
        double altitude;
        PositionData *pNext;
    };
    struct cxd56_gnss_dms_s
    {
        int8_t   sign;
        uint8_t  degree;
        uint8_t  minute;
        uint32_t frac;
    };
    ~gnss_api();

    void DMS_RAD(double DMS, double *Rad);
    void RAD_DMS(double Rad, double *DMS);
    double BLH2XYZ_To_Accuracy(PositionData *pPosDataBegin);
    bool processUserInput(Command_ST &cmd);
    bool executeCommand(Command_ST cmd);
    void executeResult(int cmdRet, char *cmdStr);
    void double_to_dmf(double x, struct cxd56_gnss_dms_s *dmf);
    double dms_to_double(const int degree, const int minute, const int second, const int fraction);
    static gnss_api *pInstantce;
    static gnss_api *GetSingletonInstance();
    static void FreeSingletonInstance();
    void DebugPrintfSatellites(struct cxd56_gnss_positiondata_s posdat);
    double gnss_check_SNR(int time, int siglevel1, int siglevel2);
    double gnss_check_PosAccuracy(int time);
    void gnss_getFirstPOS();
    bool gnss_read_and_print_POS();
    bool savePvtLogToLocal(cxd56_pvtlog_s &pvtlogdat);
    bool gnss_save_check_PVTLog(int time);
    float gnss_CheckPositioning_Continuous(int time);
    float gnss_CheckNMEA_Continuous(int time);
    uint32_t satellite_setting;
private:
    gnss_api();
    int stateMachine;
    int gnss_fd;
    sigset_t mask;
    NMEA_OUTPUT_CB  nmea_funcs;
    int sig_gnss;
    int sig_pvtlog;
    int sig_agps;
    int sig_rtk;
    int sig_spectrum;
    int sig_gpsephemeris;
    int sig_glnephemeris;
    int sig_sbas;
    struct timeval startTime;
    struct timeval positionTime;
    struct timeval localBackupFileTime[100];
    char pGPS_AlmanacBuffer[CXD56_GNSS_GPS_ALMANAC_SIZE];
    char pGLONASS_AlmanacBuffer[CXD56_GNSS_GLONASS_ALMANAC_SIZE];
    char pQZSSL1CA_AlmanacBuffer[CXD56_GNSS_QZSSL1CA_ALMANAC_SIZE];

    char pGPS_EphemerisBuffer[CXD56_GNSS_GPS_EPHEMERIS_SIZE];
    char pGLONASS_EphemerisBuffer[CXD56_GNSS_GLONASS_EPHEMERIS_SIZE];
    char pQZSSL1CA_EphemerisBuffer[CXD56_GNSS_QZSSL1CA_EPHEMERIS_SIZE];


    int cycle;
    int pvtLogIndex;
    int startMode;
    int ttff;
    int stability;
    int posdata;
    PositionData *pPosDataList;
};

#endif
