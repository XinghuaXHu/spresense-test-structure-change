test/mqtt
^^^^^^^^^

  This application is a sample that connect to the LTE network
  and check MQTT functions using iot.eclipse.org.

  Supported LTE modem is ALT1250.

  Build kernel and SDK:

  $ make buildkernel KERNCONF=release

  This application can be selected by menuconfig command.

  $ ./tools/config.py feature/lte
  $ make menuconfig
      LTE   ---> [*] Stub mbedTLS Support
        ( or Externals  ---> [*] mbed TLS Library )
      Test  ---> [*] MQTT test
  $ make

  Execute under nsh:

  Type 'mqtt <number>' on nsh like this.
  nsh> mqtt 10

  Some test sequense is exist.
    Library test     : 03 - 04
    QoS test         : 10 - 15
    Retain test      : 16 - 17
    Will test        : 18 - 19
    Multi topic test : 20 - 23
    Wildcard test    : 30 - 35
    API test         : 40 - 45

Notice:

  Need to update Kconfig parameters and prepare files to use this example.
    TEST_MQTT_ROOTCA

