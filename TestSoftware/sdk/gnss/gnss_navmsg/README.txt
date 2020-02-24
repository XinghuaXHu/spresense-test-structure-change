test/gnss_navmsg
^^^^^^^^^^^^^^^^^^^^^^^

  Example for GNSS navigation message OUTPUT.

  Carrierphase data, GPS Ephemeris data and GLONASS Ephemeris data, 
  SBAS message data are output.
  Carrierphase data also outputs files.
  
  The following configuration options can be selected:

  CONFIG_EXAMPLES_GNSS_NAVMSG -- GNSS RTK output example. Default: n
  CONFIG_EXAMPLES_GNSS_NAVMSG_PROGNAME -- You can choice other name for
    this program if change this field. Default: "gnss_navmsg"
  CONFIG_EXAMPLES_NAVMSG_PRIORITY -- Specified this example's task priority.
	Default: 100
  CONFIG_EXAMPLES_NAVMSG_STACKSIZE -- Specified this example's stack size.
	Default: 2048

  In addition to the above, the following definitions are required:
    CONFIG_CXD56_GNSS
