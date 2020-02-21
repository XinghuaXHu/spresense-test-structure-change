= Battery gauge driver test

== Configuration

Enable [ CXD56xx Configuration -> CXD5247 battery gauge ] from SDK
configuration menu.

and also enable this test.

== Usage

Just type 'gauge' on nsh command line.

  nsh> gauge

If test success, show battery information without any error/failure messages.

Additionally, if you want to polling battery gauge information, then use
loop from shell script.

  e.g. Show battery information every 1 second
  nsh> echo "while true; do gauge; sleep 1; done" > /mnt/spif/gauge.sh
  nsh> sh /mnt/spif/gauge.sh
(...show gauge informations...)

