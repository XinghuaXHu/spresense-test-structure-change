test/sqa/singlefunction/gnss
^^^^^^^^^^^^^^^^^^^

  TEST for GNSS.

  The following configuration options can be selected:

  CONFIG_EXAMPLES_GNSS_TEST -- GNSS test. Default: n
  CONFIG_EXAMPLES_GNSS_TEST_PROGNAME -- You can choice other name for
    this program if change this field. Default: "gnss_test"
  CONFIG_EXAMPLES_GNSS_TEST_PRIORITY -- Specified this example's task priority.
	  Default: 100
  CONFIG_EXAMPLES_GNSS_TEST_STACKSIZE -- Specified this example's stack size.
	  Default: 2048

  In addition to the above, the following definitions are required:
    CONFIG_CXD56_GNSS
