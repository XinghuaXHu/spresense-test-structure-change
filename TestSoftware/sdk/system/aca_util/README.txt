aca_util
^^^^^^^^

  This command is the utility tool for CXD5247 Audio

  Required configurations

    * CONFIG_TEST_ACA_UTIL=y - This command app
    * CONFIG_CXD56_AUDIO=y - Audio basic feature
    * CONFIG_SDK_AUDIO=y - Audio basic feature

  The simplest configuration way to use this command is:

    $ ./tools/config.py examples/audio_player -- +TEST_ACA_UTIL

  Example: Show help message

    nsh> aca

    Usage: aca [-h]
               [-b <bank>] [-r <addr>] [-w <addr> -v <value>]

    Description:
     Aca utility tool
    Options:
     -b <bank>: Bank number (default: 0)
     -r <addr>: Single read from <addr>
     -w <addr> -v <value>: Single write <value> to <addr>
     -h: Show this message

  Example: read ChipID

    nsh> aca -b 0 -r 0xfd
    @(0)[fd]=>4c

  Appendix.
    Below is aca register map for reference in SpritzerPty/drivers/peripheral/aca/aca_reg_map.c.
    For more details, please see the CXD5247 specification.

        typedef struct {
            uint32_t bank;
            uint32_t addr;
            uint32_t pos;
            uint32_t len;
            uint32_t init;
        } acaReg;

        const acaReg acaRegMap[AU_REG_MAX_ENTRY] = {
            {0x00, 0x10, 7, 1, 0x00},   // IIC_MICA_EN_MIC
            {0x00, 0x10, 6, 1, 0x00},   // IIC_MICB_EN_MIC
            {0x00, 0x10, 5, 1, 0x00},   // IIC_MICC_EN_MIC
            {0x00, 0x10, 4, 1, 0x00},   // IIC_MICD_EN_MIC
            {0x00, 0x10, 3, 1, 0x00},   // IIC_MICA_EN_ADC
            {0x00, 0x10, 2, 1, 0x00},   // IIC_MICB_EN_ADC
            {0x00, 0x10, 1, 1, 0x00},   // IIC_MICC_EN_ADC
            {0x00, 0x10, 0, 1, 0x00},   // IIC_MICD_EN_ADC
            {0x00, 0x11, 3, 3, 0x00},   // IIC_MICA_MICGAIN[2:0]
            {0x00, 0x11, 0, 3, 0x00},   // IIC_MICB_MICGAIN[2:0]
            {0x00, 0x12, 3, 3, 0x00},   // IIC_MICC_MICGAIN[2:0]
            {0x00, 0x12, 0, 3, 0x00},   // IIC_MICD_MICGAIN[2:0]
            {0x00, 0x13, 4, 4, 0x00},   // IIC_MICA_PGAGAIN[3:0]
            {0x00, 0x13, 0, 4, 0x00},   // IIC_MICB_PGAGAIN[3:0]
            {0x00, 0x14, 4, 4, 0x00},   // IIC_MICC_PGAGAIN[3:0]
            {0x00, 0x14, 0, 4, 0x00},   // IIC_MICD_PGAGAIN[3:0]
            {0x00, 0x15, 4, 1, 0x00},   // IIC_MIC_EN_VM
            {0x00, 0x15, 3, 1, 0x00},   // IIC_MICA_EN_DIG
            {0x00, 0x15, 2, 1, 0x00},   // IIC_MICB_EN_DIG
            {0x00, 0x15, 1, 1, 0x00},   // IIC_MICC_EN_DIG
            {0x00, 0x15, 0, 1, 0x00},   // IIC_MICD_EN_DIG
            {0x00, 0x16, 3, 1, 0x00},   // IIC_MICA_EN_DEMX
            {0x00, 0x16, 2, 1, 0x00},   // IIC_MICB_EN_DEMX
            {0x00, 0x16, 1, 1, 0x00},   // IIC_MICC_EN_DEMX
            {0x00, 0x16, 0, 1, 0x00},   // IIC_MICD_EN_DEMX
            {0x00, 0x17, 7, 1, 0x00},   // IIC_MICA_EN_CHAX
            {0x00, 0x17, 6, 1, 0x00},   // IIC_MICB_EN_CHAX
            {0x00, 0x17, 5, 1, 0x00},   // IIC_MICC_EN_CHAX
            {0x00, 0x17, 4, 1, 0x00},   // IIC_MICD_EN_CHAX
            {0x00, 0x17, 3, 1, 0x00},   // IIC_MICA_EN_CHSX
            {0x00, 0x17, 2, 1, 0x00},   // IIC_MICB_EN_CHSX
            {0x00, 0x17, 1, 1, 0x00},   // IIC_MICC_EN_CHSX
            {0x00, 0x17, 0, 1, 0x00},   // IIC_MICD_EN_CHSX
            {0x00, 0x18, 7, 1, 0x00},   // IIC_MICA_XEN_CHVM
            {0x00, 0x18, 6, 1, 0x00},   // IIC_MICB_XEN_CHVM
            {0x00, 0x18, 5, 1, 0x00},   // IIC_MICC_XEN_CHVM
            {0x00, 0x18, 4, 1, 0x00},   // IIC_MICD_XEN_CHVM
            {0x00, 0x18, 3, 1, 0x00},   // IIC_MICA_CHPHASE
            {0x00, 0x18, 2, 1, 0x00},   // IIC_MICB_CHPHASE
            {0x00, 0x18, 1, 1, 0x00},   // IIC_MICC_CHPHASE
            {0x00, 0x18, 0, 1, 0x00},   // IIC_MICD_CHPHASE
            {0x00, 0x19, 2, 1, 0x00},   // IIC_BGR50_EN_BGR
            {0x00, 0x19, 1, 1, 0x00},   // IIC_BGR50_EN_BIAS
            {0x00, 0x19, 0, 1, 0x00},   // IIC_BGR50_SEL
            {0x00, 0x1A, 7, 1, 0x00},   // IIC_MIC_BIAS_BIAS_A
            {0x00, 0x1A, 6, 1, 0x00},   // IIC_MIC_BIAS_BIAS_B
            {0x00, 0x1A, 5, 1, 0x00},   // IIC_MIC_EN_BGR
            {0x00, 0x1A, 4, 1, 0x00},   // IIC_MIC_EN_BIAS
            {0x00, 0x1A, 3, 1, 0x00},   // IIC_MIC_BIASSEL
            {0x00, 0x1A, 1, 1, 0x00},   // IIC_MIC_BIAS_CTRL
            {0x00, 0x1A, 0, 1, 0x00},   // IIC_MIC_BIAS_EN_REF
            {0x00, 0x1B, 5, 1, 0x00},   // IIC_MIC_BIAS_EN_MAIN_A
            {0x00, 0x1B, 4, 1, 0x00},   // IIC_MIC_BIAS_EN_SUB_A
            {0x00, 0x1B, 3, 1, 0x00},   // IIC_MIC_BIAS_EN_MAIN_B
            {0x00, 0x1B, 2, 1, 0x00},   // IIC_MIC_BIAS_EN_SUB_B
            {0x00, 0x1B, 1, 1, 0x00},   // IIC_MIC_BIAS_TIEL_A
            {0x00, 0x1B, 0, 1, 0x00},   // IIC_MIC_BIAS_TIEL_B
            {0x00, 0x1C, 3, 1, 0x00},   // IIC_MIC_EN_DMIC12
            {0x00, 0x1C, 2, 1, 0x00},   // IIC_MIC_EN_DMIC34
            {0x00, 0x1C, 1, 1, 0x00},   // IIC_MIC_EN_DMIC56
            {0x00, 0x1C, 0, 1, 0x00},   // IIC_MIC_EN_DMIC78
            {0x00, 0x1D, 3, 1, 0x00},   // IIC_MICA_RESERVE
            {0x00, 0x1D, 2, 1, 0x00},   // IIC_MICB_RESERVE
            {0x00, 0x1D, 1, 1, 0x00},   // IIC_MICC_RESERVE
            {0x00, 0x1D, 0, 1, 0x00},   // IIC_MICD_RESERVE
            {0x00, 0x1E, 0, 5, 0x00},   // IIC_MIC_ATBSEL[4:0]
            {0x00, 0x1F, 7, 1, 0x00},   // IIC_MIC_EN_LDO
            {0x00, 0x1F, 5, 1, 0x00},   // IIC_MIC_EN_OLP
            {0x00, 0x1F, 2, 3, 0x00},   // IIC_MIC_LDO_TRM[2:0]
            {0x00, 0x1F, 0, 2, 0x00},   // IIC_MIC_LDO_OLP_TRM[1:0]
            {0x00, 0x20, 3, 1, 0x00},   // IIC_SP_EN_SPAN
            {0x00, 0x20, 2, 1, 0x00},   // IIC_SP_EN_SPAP
            {0x00, 0x20, 1, 1, 0x00},   // IIC_SP_EN_SPBN
            {0x00, 0x20, 0, 1, 0x00},   // IIC_SP_EN_SPBP
            {0x00, 0x20, 0, 4, 0x00},   // IIC_SP_EN_SPAN, IIC_SP_EN_SPAP, IIC_SP_EN_SPBN, IIC_SP_EN_SPBP
            {0x00, 0x21, 4, 1, 0x00},   // IIC_SP_VMSEL
            {0x00, 0x21, 2, 1, 0x00},   // IIC_SP_DLYFREEA
            {0x00, 0x21, 1, 1, 0x00},   // IIC_SP_DLYFREEB
            {0x00, 0x21, 0, 1, 0x00},   // IIC_SP_CAPSEL
            {0x00, 0x22, 3, 1, 0x00},   // IIC_SP_EN_LOOP
            {0x00, 0x22, 2, 1, 0x00},   // IIC_SP_LOOPSEL
            {0x00, 0x22, 1, 1, 0x00},   // IIC_SP_LAMPISEL
            {0x00, 0x22, 0, 1, 0x00},   // IIC_SP_REFAMPISEL
            {0x00, 0x23, 1, 1, 0x00},   // IIC_SP_EN_BGR
            {0x00, 0x23, 0, 1, 0x00},   // IIC_SP_EN_BIAS
            {0x00, 0x24, 6, 2, 0x00},   // IIC_SP_SPLITONSELA[1:0]
            {0x00, 0x24, 4, 2, 0x00},   // IIC_SP_DELAYSELAP[1:0]
            {0x00, 0x24, 2, 2, 0x00},   // IIC_SP_SPLITONSELB[1:0]
            {0x00, 0x24, 0, 2, 0x00},   // IIC_SP_DELAYSELBP[1:0]
            {0x00, 0x25, 4, 2, 0x00},   // IIC_SP_DRVSELA[1:0]
            {0x00, 0x25, 0, 2, 0x00},   // IIC_SP_DRVSELB[1:0]
            {0x00, 0x26, 0, 1, 0x00},   // IIC_MIC_LDO_TIEL
            {0x00, 0x27, 3, 1, 0x00},   // IIC_MICA_VM_EXC_SEL
            {0x00, 0x27, 2, 1, 0x00},   // IIC_MICB_VM_EXC_SEL
            {0x00, 0x27, 1, 1, 0x00},   // IIC_MICC_VM_EXC_SEL
            {0x00, 0x27, 0, 1, 0x00},   // IIC_MICD_VM_EXC_SEL
            {0x00, 0x30, 3, 1, 0x00},   // IIC_XO_EN_OSC
            {0x00, 0x30, 2, 1, 0x00},   // IIC_XO_EN_CLK_ANA_A
            {0x00, 0x30, 1, 1, 0x00},   // IIC_XO_EN_CLK_ANA_B
            {0x00, 0x30, 0, 1, 0x00},   // IIC_XO_EN_CLK_DIG
            {0x00, 0x31, 4, 1, 0x00},   // IIC_XO_EN_BGR
            {0x00, 0x31, 3, 1, 0x00},   // IIC_XO_EN_LDO
            {0x00, 0x31, 2, 1, 0x00},   // IIC_XO_HIZ_BGR
            {0x00, 0x31, 1, 1, 0x00},   // IIC_XO_HIZ_LDO
            {0x00, 0x31, 0, 1, 0x00},   // IIC_XO_EN_EXT
            {0x00, 0x32, 0, 8, 0x00},   // IIC_XO_ATBSEL[7:0]
            {0x00, 0x33, 0, 8, 0x00},   // IIC_XO_LDOTRM[7:0]
            {0x00, 0x34, 4, 2, 0x00},   // IIC_XO_DRV_DIG[1:0]
            {0x00, 0x34, 2, 2, 0x00},   // IIC_XO_DRV_ANA_A[1:0]
            {0x00, 0x34, 0, 2, 0x00},   // IIC_XO_DRV_ANA_B[1:0]
            {0x00, 0x35, 0, 4, 0x00},   // IIC_XO_OSC_I[3:0]
            {0x00, 0x36, 0, 2, 0x00},   // IIC_XO_RD[1:0]
            {0x00, 0x37, 4, 1, 0x00},   // IIC_XO_REF_I
            {0x00, 0x37, 0, 2, 0x00},   // IIC_XO_CLKSEL[1:0]
            {0x00, 0x38, 0, 8, 0x00},   // IIC_XO_CAP_CG[7:0]
            {0x00, 0x39, 0, 8, 0x00},   // IIC_XO_CAP_CD[7:0]
            {0x00, 0x80, 7, 1, 0x01},   // DA_DATA_PE
            {0x00, 0x80, 6, 1, 0x00},   // DA_DATA_PS
            {0x00, 0x80, 5, 1, 0x01},   // FSCLK_PE
            {0x00, 0x80, 4, 1, 0x00},   // FSCLK_PS
            {0x00, 0x81, 4, 2, 0x00},   // GPO_DS[1:0]
            {0x00, 0x81, 2, 2, 0x00},   // AD_DATA_DS[1:0]
            {0x00, 0x82, 6, 2, 0x00},   // CLKOUT_DMIC_DS[1:0]
            {0x00, 0x82, 4, 2, 0x00},   // MCLKOUT_DS[1:0]
            {0x00, 0xFD, 0, 8, 0x49},   // CHIPID[7:0]
            {0x01, 0x10, 6, 1, 0x00},   // PWM_CLK_SEL
            {0x01, 0x10, 5, 1, 0x00},   // M_CLK_SEL
            {0x01, 0x10, 3, 1, 0x00},   // MIC_CLK_SEL
            {0x01, 0x10, 2, 1, 0x00},   // TEST_FS
            {0x01, 0x10, 1, 1, 0x00},   // SCL_HSMODE
            {0x01, 0x11, 7, 1, 0x00},   // LOGIC_CLK_EN
            {0x01, 0x11, 6, 1, 0x00},   // SER_CLK_EN
            {0x01, 0x11, 5, 1, 0x00},   // AMIC1_CLK_EN
            {0x01, 0x11, 4, 1, 0x00},   // AMIC2_CLK_EN
            {0x01, 0x11, 3, 1, 0x00},   // AMIC3_CLK_EN
            {0x01, 0x11, 2, 1, 0x00},   // AMIC4_CLK_EN
            {0x01, 0x11, 1, 1, 0x00},   // DMIC_CLK_EN
            {0x01, 0x11, 0, 1, 0x00},   // ADCPOST_CLK_EN
            {0x01, 0x12, 1, 1, 0x00},   // SU_CLK_EN
            {0x01, 0x12, 0, 1, 0x00},   // O_MCLK_EN
            {0x01, 0x13, 4, 1, 0x00},   // GPO
            {0x01, 0x13, 0, 2, 0x00},   // TESTIRQ[1:0]
            {0x01, 0x14, 4, 4, 0x01},   // SEL_CH1[3:0]
            {0x01, 0x14, 0, 4, 0x02},   // SEL_CH2[3:0]
            {0x01, 0x15, 4, 4, 0x03},   // SEL_CH3[3:0]
            {0x01, 0x15, 0, 4, 0x04},   // SEL_CH4[3:0]
            {0x01, 0x16, 4, 4, 0x05},   // SEL_CH5[3:0]
            {0x01, 0x16, 0, 4, 0x06},   // SEL_CH6[3:0]
            {0x01, 0x17, 4, 4, 0x07},   // SEL_CH7[3:0]
            {0x01, 0x17, 0, 4, 0x08},   // SEL_CH8[3:0]
            {0x01, 0x18, 4, 4, 0x00},   // AMIC_POL[3:0]
            {0x01, 0x18, 0, 1, 0x00},   // SER_MODE
            {0x01, 0x19, 4, 2, 0x00},   // STR_PWM_OUT[1:0]
            {0x01, 0x19, 3, 1, 0x00},   // COUNT_RSTX
            {0x01, 0x19, 2, 1, 0x00},   // COUNT_EN
            {0x01, 0x19, 1, 1, 0x00},   // COUNT_UD
            {0x01, 0x19, 0, 1, 0x00},   // STR_SIG_SEL
            {0x01, 0x1A, 4, 2, 0x02},   // FUNC_SEL[1:0]
            {0x01, 0x1A, 0, 3, 0x00},   // STR_TC[2:0]
            {0x01, 0x1B, 4, 3, 0x00},   // PWM_OUTSEL_L[2:0]
            {0x01, 0x1B, 0, 3, 0x00},   // PWM_OUTSEL_R[2:0]
            {0x01, 0x1C, 4, 1, 0x00},   // CHSEL
            {0x01, 0x1C, 0, 1, 0x01},   // MCKSELI
            {0x01, 0x1D, 4, 4, 0x00},   // OUT2DLY[3:0]
            {0x01, 0x1D, 0, 2, 0x01},   // PWMMD[1:0]
            {0x01, 0x1E, 4, 3, 0x00},   // SUBSELA[2:0]
            {0x01, 0x1E, 0, 3, 0x00},   // SUBSELB[2:0]
            {0x01, 0x1F, 4, 1, 0x00},   // FS_LOCK
            {0x01, 0x1F, 1, 1, 0x00},   // COUNT_FULL
            {0x01, 0x1F, 0, 1, 0x00},   // COUNT_ZERO
            {0x01, 0x20, 0, 1, 0x00},   // FS128
            {0x01, 0x21, 0, 6, 0x13},   // VGAIN1[5:0]
            {0x01, 0x22, 0, 6, 0x13},   // VGAIN2[5:0]
            {0x01, 0x23, 0, 6, 0x13},   // VGAIN3[5:0]
            {0x01, 0x24, 0, 6, 0x13},   // VGAIN4[5:0]
            {0x01, 0x25, 4, 4, 0x00},   // DITHEN[3:0]
            {0x01, 0x25, 0, 3, 0x00},   // DITHMODE[2:0]
            {0x01, 0x26, 0, 6, 0x19},   // OFST1[5:0]
            {0x01, 0x27, 0, 6, 0x19},   // OFST2[5:0]
            {0x01, 0x28, 0, 6, 0x19},   // OFST3[5:0]
            {0x01, 0x29, 0, 6, 0x19},   // OFST4[5:0]
            {0x01, 0x2A, 7, 1, 0x00},   // DET_EN2
            {0x01, 0x2A, 4, 3, 0x00},   // DET_TIME2[2:0]
            {0x01, 0x2A, 3, 1, 0x01},   // LATCH_EN2
            {0x01, 0x2A, 2, 1, 0x00},   // OUT_HIZ_EN2
            {0x01, 0x2A, 1, 1, 0x00},   // VOLMUTE_EN2
            {0x01, 0x2B, 7, 1, 0x00},   // DET_EN4
            {0x01, 0x2B, 4, 3, 0x00},   // DET_TIME4[2:0]
            {0x01, 0x2B, 3, 1, 0x01},   // LATCH_EN4
            {0x01, 0x2B, 2, 1, 0x00},   // OUT_HIZ_EN4
            {0x01, 0x2B, 1, 1, 0x00},   // VOLMUTE_EN4
            {0x01, 0x2C, 6, 1, 0x01},   // MASK_IRQ_OCP_OUT
            {0x01, 0x2C, 4, 1, 0x01},   // MASK_IRQ_DATAERR_DET_OUT
            {0x01, 0x2D, 6, 1, 0x00},   // INT_RST_OCP_OUT
            {0x01, 0x2D, 4, 1, 0x00},   // INT_RST_DATAERR_DET_OUT
            {0x01, 0x2E, 0, 1, 0x00},   // RD_STB
            {0x01, 0x2F, 4, 1, 0x00},   // SP_OUTOCD_DET
            {0x01, 0x2F, 0, 1, 0x00},   // DATAERR_DET_OUT
        };
