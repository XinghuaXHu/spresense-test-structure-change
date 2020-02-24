test/awsiot
^^^^^^^^^^^

  This application is a sample that connect to the LTE network
  and check AWS IoT functions.

  Supported LTE modem is ALT1250.

  Build kernel and SDK:

  $ make buildkernel KERNCONF=release

  This application can be selected by menuconfig command.

  $ ./tools/config.py feature/lte
  $ make menuconfig
      LTE   ---> [*] Stub mbedTLS Support
        ( or Externals  ---> [*] mbed TLS Library )
      Test  ---> [*] AWS IoT test
  $ make

  Execute under nsh:

  Type 'awsiot <number>' on nsh like this.
  nsh> awsiot 10

  Some test sequense is exist.
    Library connect test : 04 - 07
    MQTT QoS test        : 10 - 11
    MQTT Will test       : 12
    MQTT Wildcard test   : 13 - 15
    Shadow get test      : 20
    Shadow delete test   : 21
    Shadow delta test    : 22 - 26
    Shadow update test   : 27

Notice:

  Need to update Kconfig parameters and prepare files to use this example.
    TEST_AWSIOT_HOST
    TEST_AWSIOT_PORT
    TEST_AWSIOT_ROOTCA
    TEST_AWSIOT_CLIENT_CERT
    TEST_AWSIOT_CLIENT_KEY

