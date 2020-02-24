test/websocket
^^^^^^^^^^^^^^

  This application is a sample that connect to the LTE network
  and check WebSocket functions
  using echo.websocket.org and ruby-websockets-chat.herokuapp.com.

  Supported LTE modem is ALT1250.

  Build kernel and SDK:

  $ make buildkernel KERNCONF=release

  This application can be selected by menuconfig command.

  $ ./tools/config.py feature/lte
  $ make menuconfig
      LTE   ---> [*] Stub mbedTLS Support
        ( or Externals  ---> [*] mbed TLS Library )
      Test  ---> [*] WebSocket test
  $ make

  Execute under nsh:

  Type 'websocket <number>' on nsh like this.
  nsh> websocket 10

  Some test sequense is exist.
    Library test       : 03 - 04
    Echo protocol test : 10 - 15
    Chat protocol test : 20 - 21

Notice:

  Need to update Kconfig parameters and prepare files to use this example.
    TEST_WEBSOCKET_ROOTCA_ECHO
    TEST_WEBSOCKET_ROOTCA_CHAT

