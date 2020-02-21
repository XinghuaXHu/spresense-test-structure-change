test/lwm2m
^^^^^^^^^^

  This application is a sample that connect to the LTE network
  and check LightweightM2M functions
  using selftest.iot.nokia.com and leshan.eclipse.org.

  Supported LTE modem is ALT1250.

  Build kernel and SDK:

  $ make buildkernel KERNCONF=release

  This application can be selected by menuconfig command.

  $ ./tools/config.py feature/lte
  $ make menuconfig
      Test  ---> [*] Lightweight M2M test
  $ make

  Execute under nsh:

  Type 'lwm2m <number>' on nsh like this.
  nsh> lwm2m 10

  Some test sequense is exist.
    Library test         : 01
    Original server test : 10
    Nokia test           : 11
    Leshan test          : 12 - 13

Notice:

  Need to update Kconfig parameters and prepare files to use this example.
    TEST_LWM2M_CLIENT_NAME
    TEST_LWM2M_CLIENT_PORT
    TEST_LWM2M_SERVER_HOST
    TEST_LWM2M_SERVER_PORT
    TEST_LWM2M_LESHAN_IDENTITY
    TEST_LWM2M_LESHAN_PSK

