pmic
^^^^

  This example monitors/controls the power status of each target device on board.
  This is supported only for CONFIG_ARCH_CHIP="cxd56xx" and CONFIG_CXD56_PMIC=y.

    * CONFIG_EXAMPLES_PMIC - Enabled the example

  Supported board:

    * CONFIG_ARCH_BOARD="corvo"
    * CONFIG_ARCH_BOARD="sp_yoc"
    * CONFIG_ARCH_BOARD="collet"

    If you want to support for your board, add POWER_XXX to list[] in pmic_main.c.
    POWER_XXX is described into configs/<your board>/include/board.h.

  Example: Show help message

    nsh> pmic -h

      Usage: pmic [-e <target>] [-d <target>] [-h]
                  [-r <addr>] [-w <addr> -v <value>]

      Description:
       Show the power status of each target device
      Options:
       -e <target>: Enable power to the target
       -d <target>: Disable power to the target
       -r <addr>: Single read from <addr>
       -w <addr> -v <value>: Single write <value> to <addr>
       -h: Show this message

  Example: Show the power status

    nsh> pmic

     Target Name : on/off
          DDC_IO : on       +---
        LDO_EMMC : off      |
         DDC_ANA : on       |
         LDO_ANA : on       |
        DDC_CORE : on       |
        LDO_PERI : off      |
            LSW2 : off      |
            LSW3 : on       | Common Target Name
            LSW4 : on       |
            GPO0 : off      |
            GPO1 : on       |
            GPO2 : off      |
            GPO3 : off      |
            GPO4 : off      |
            GPO5 : on       |
            GPO6 : on       |
            GPO7 : off      +---
      AUDIO_DVDD : on       +---
           FLASH : on       |
            TCXO : on       |
      AUDIO_AVDD : off      |
         DDC_18V : on       |
          BMI160 : on       | Board-specific Target Name
       HPADC_MIC : off      |
             LNA : off      |
        AK09912C : off      |
          BMP280 : off      |
         DDC_33V : on       |
            EMMC : on       +---

  Example: Change AUDIO_DVDD to off and LNA to on

    nsh>pmic -d AUDIO_DVDD -e LNA
    Disable: AUDIO_DVDD
    Enable : LNA

     Target Name : on/off
          DDC_IO : on
        LDO_EMMC : off
         DDC_ANA : on
         LDO_ANA : on
        DDC_CORE : on
        LDO_PERI : off
            LSW2 : off
            LSW3 : on
            LSW4 : on
            GPO0 : off
            GPO1 : on
            GPO2 : off
            GPO3 : on
            GPO4 : off
            GPO5 : on
            GPO6 : on
            GPO7 : off
      AUDIO_DVDD : off
           FLASH : on
            TCXO : on
      AUDIO_AVDD : off
         DDC_18V : on
          BMI160 : on
       HPADC_MIC : off
             LNA : on
        AK09912C : off
          BMP280 : off
         DDC_33V : on
            EMMC : on

  Example: Read a value from CONFIG_INT_WKUP(38h) register

    nsh>pmic -r 38
    @[38]=>02

  Example: Write a value(00h) to CONFIG_INT_WKUP(38h) register

    nsh>pmic -w 38 -v 00
    @[38]<=00

