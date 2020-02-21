test/gnss_ioctl
^^^^^^^^^^^^^^^^^^^

Overview:
This sample shows the use gnss ioctl command.
  Please set test number after example name.
    gnss_ioctl 0:run all test
    gnss_ioctl 1:IOCTL_SET_OPE_MODE
    gnss_ioctl 2:IOCTL_SELECT_SATELLITE_SYSTEM
    gnss_ioctl 3:IOCTL_START
    gnss_ioctl 4:IOCTL_SET_RECEIVER_POSITION
    gnss_ioctl 5:IOCTL_SET_TIME
    gnss_ioctl 6:IOCTL_SAVE_BACKUP_DATA
    gnss_ioctl 7:IOCTL_SET_CEP_DATA
    gnss_ioctl 8:IOCTL_SET_TCXO_OFFSET
    gnss_ioctl 9:IOCTL_SET_ALMANAC
    gnss_ioctl 10:IOCTL_SET_EPHEMERIS

Configuration:
[System Type]
  CXD56xx Peripheral Support -->
    [*] GNSS device

[Application Configuration]
  Examples -->
    [*] GNSS_IOCTL ioctl test

Output example:
Most tests return 0 on normal operation.
But some tests require manual checking.(These tests return 0 fixed.)

Command:
Enter test name and test number

Details of each test:
  gnss_ioctl 0
    (For detailed output see later.)
    #     , test ioctl                   , result
    test 1, IOCTL_SET_OPE_MODE           , ret 0
    test 2, IOCTL_SELECT_SATELLITE_SYSTEM, ret 0
    test 3, IOCTL_START                  , ret 0
    test 4, IOCTL_SET_RECEIVER_POSITION  , ret 0
    test 5, IOCTL_SET_TIME               , ret 0
    test 6, IOCTL_SAVE_BACKUP_DATA       , ret 0
    test 7, IOCTL_SET_CEP_DATA           , ret 0
    test 8:IOCTL_SET_TCXO_OFFSET         , ret 0
    test 9:IOCTL_SET_ALMANAC             , ret 0
    test 10:IOCTL_SET_EPHEMERIS          , ret 0
    done.

  gnss_ioctl 1
    Test the following commands.
      CXD56_GNSS_IOCTL_SET_OPE_MODE
      CXD56_GNSS_IOCTL_GET_OPE_MODE
    Set the positioning cycle parameter to 1 sec, 2 sec, 3 sec, and 10 sec.
    Please check the return value(0:OK).

  gnss_ioctl 2
    Test the following commands.
      CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM
      CXD56_GNSS_IOCTL_GET_SATELLITE_SYSTEM
    Set satellite system parameter to use GPS, GLN, GPS&GLN.
    Please check the return value(0:OK).

  gnss_ioctl 3
    Test the following commands.
      CXD56_GNSS_IOCTL_START
      CXD56_GNSS_IOCTL_STOP
    Set the start mode to Cold start, warm start, hot start.
    Please check the positioning time according to the start mode.
    
  gnss_ioctl 4
    Test the following commands.
      CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ELLIPSOIDAL
      CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ORTHOGONAL
    Set the position before start.
    Please check the return value(0:OK).
      In this test, an error of 0.000100 degrees is allowed due to calculation errors.

  gnss_ioctl 5
    Test the following commands.
      CXD56_GNSS_IOCTL_SET_TIME
    Set the time before start.
    Please check the return value(0:OK).
      In this test, an error of 20 seconds is allowed due to the initialization time.
      In this test, does not support the judgment over the date.(ex. from 23:59:59 to 0:00:01)

  gnss_ioctl 6
    Test the following commands.
      CXD56_GNSS_IOCTL_SAVE_BACKUP_DATA
      CXD56_GNSS_IOCTL_ERASE_BACKUP_DATA
    Save and erase backup data.
    Please check the positioning time when backup data is SAVED or ERASED.

  gnss_ioctl 7
    Test the following commands.
      CXD56_GNSS_IOCTL_OPEN_CEP_DATA
      CXD56_GNSS_IOCTL_CLOSE_CEP_DATA
      CXD56_GNSS_IOCTL_CHECK_CEP_DATA
      CXD56_GNSS_IOCTL_GET_CEP_AGE
      CXD56_GNSS_IOCTL_RESET_CEP_FLAG
    Open cep data,and check Validity period.
    Please check the positioning time when CEP data is Opened.
    Open CEP file of the following path
      '/mnt/vfat/gnss_cep.bin'

  gnss_ioctl 8
    Test the following commands.
      CXD56_GNSS_IOCTL_SET_TCXO_OFFSET
      CXD56_GNSS_IOCTL_GET_TCXO_OFFSET
    Please check the positioning time when KEEP or CLEAR offset.

  gnss_ioctl 9
    Test the following commands.
      CXD56_GNSS_IOCTL_GET_ALMANAC
      CXD56_GNSS_IOCTL_SET_ALMANAC
    Please check the return value(0:OK).
      In this test, ALMANAC data details are not checked.

  gnss_ioctl 10
    Test the following commands.
      CXD56_GNSS_IOCTL_GET_EPHEMERIS
      CXD56_GNSS_IOCTL_SET_EPHEMERIS
    Please check the return value(0:OK).
      In this test, EPHEMERIS data details are not checked.
