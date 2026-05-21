/*
 *  Copyright (C) 2021-2025 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <tiescsoc.h>
#include <drivers/hw_include/hw_types.h>
#include <drivers/hw_include/cslr_spinlock.h>
#include <drivers/mdio.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/AddrTranslateP.h>
#include "ti_board_open_close.h"
#include "ti_drivers_config.h"
#include "tieschw.h"
#include "tiesc_eeprom.h" /* header equivalent of ESI bin file */
#include <drivers/hw_include/cslr_soc.h>
#include <board/ethphy/icss_emac/ethphy_dp83826e.h> /* Needed only for RevE1 and RevE2 which uses DP83826E PHY */
#include <board/ethphy/icss_emac/ethphy_dp83869.h> /* Needed only for RevA which uses DP83869 PHY */
/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  Value to be set in ICSSM_PRU0_GPIO_OUT_CTRL and ICSSM_PRU1_GPIO_OUT_CTRL registers
 *  when using pins in MII mode.
 *  This configures PRG0_PRUx_GPIO0, PRG0_PRUx_GPIO1, PRG0_PRUx_GPIO2, PRG0_PRUx_GPIO3,
 *  PRG0_PRUx_GPIO4, PRG0_PRUx_GPIO5, PRG0_PRUx_GPIO6, PRG0_PRUx_GPIO8, PRG0_PRUx_GPIO9,
 *  PRG0_PRUx_GPIO10 and PRG0_PRUx_GPIO16 as input pins.
 **/
#define MSS_CTRL_ICSSM_PRU_GPIO_OUT_CTRL_VALUE   (0x0001077F)
#define MSS_CTRL_ICSSM_PRU1_GPIO_OUT_CTRL_VALUE  (0x0001037F)

/* EEPROM data offset in I2C EEPROM Flash */
#define I2C_EEPROM_DATA_OFFSET                  (0x8000)

/* 
 * PHY Link Polarity configuration
 * When Enhanced Link is enabled, polarities are configured through SysConfig
 * Otherwise, default to ACTIVE_HIGH for both PHYs
 */
#ifdef ENHANCED_LINK_ENABLED
    /* PHY0 Link Polarity setting */
    #ifdef PHY0_LINK_POLARITY_ACTIVE_HIGH
        #define TIESC_LINK0_POL                 TIESC_LINK_POL_ACTIVE_HIGH
    #else
        #define TIESC_LINK0_POL                 TIESC_LINK_POL_ACTIVE_LOW
    #endif

    /* PHY1 Link Polarity setting */
    #ifdef PHY1_LINK_POLARITY_ACTIVE_HIGH
        #define TIESC_LINK1_POL                 TIESC_LINK_POL_ACTIVE_HIGH
    #else
        #define TIESC_LINK1_POL                 TIESC_LINK_POL_ACTIVE_LOW
    #endif
#else
    /* Default to ACTIVE_HIGH for both PHYs when Enhanced Link is disabled */
    #define TIESC_LINK0_POL                     TIESC_LINK_POL_ACTIVE_HIGH
    #define TIESC_LINK1_POL                     TIESC_LINK_POL_ACTIVE_HIGH
#endif

#define DP83869_RX_ERR_CNT_REG_ADDRESS          (0x15)

/*I2C Instance and Index for IO Expander programming*/
#define I2C_ADDRESS_IO_EXPANDER_TCA6408         (0x21)
#define MDIO_MDC_MUX_SEL                        (0x03)

/*Defines for PCB Rev read from EEPROM*/
#define EEPROM_OFFSET_READ_PCB_REV              (0x0022U)
#define EEPROM_READ_PCB_REV_DATA_LEN            (0x2U)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern PRUICSS_Handle pruIcss1Handle;

extern const unsigned char tiesc_eeprom[];  /* EEPROM array corresponding to XML */

extern uint8_t *pEEPROM;                    /* EEPROM array used by the stack */

/* ========================================================================== */
/*                       Function Declarations                                */
/* ========================================================================== */

int32_t tiesc_getArmInterruptOffset();

int32_t tiesc_eepromRead(uint8_t *buf, uint32_t len);

int32_t tiesc_eepromWrite(uint8_t *buf,uint32_t len);

int32_t enableOspiResetSignal(uint16_t enable);

bool isPcbRevisionA(void);

/**
*  @brief    This API configures port0 and port1 phys for enhanced link detection, enable RXERR detection during IDLE,
*   disable detection of transmit error in odd-nibble boundary (for odd nibble insertion) and fast RXDV detection.
*   This is called only during initialization.
*
*   @param pruIcssHandle PRUSS Handle
*   @param phy0addr Phy address of port0
*   @param phy1addr Phy address of port1
*   @param enhancedlink_enable Fast link detect (enhanced link detection) enable/disable flag
*/
void tiesc_ethphyInit(PRUICSS_Handle pruIcssHandle, uint8_t phy0addr,
                      uint8_t phy1addr, uint8_t enhancedlink_enable);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

uint8_t tiesc_isEthercatDevice(void)
{
    uint8_t data;

    PRUICSS_readEfuse(pruIcss1Handle, &data);

    data = (data && 0x01);
    if(0x00 == data)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void tiesc_bspSoftReset()
{
    /* FIXME */
    /* Sciclient_pmDeviceReset(0xFFFFFFFFU); */
    return;
}

void tiesc_socEvmInit()
{
    pruIcss1Handle = PRUICSS_open(CONFIG_PRU_ICSS0);

    PRUICSS_HwAttrs const   *pruicssHwAttrs = (PRUICSS_HwAttrs const *)(pruIcss1Handle->hwAttrs);

    /* Set bits for input pins in ICSSM_PRU0_GPIO_OUT_CTRL and ICSSM_PRU1_GPIO_OUT_CTRL registers based on instance */
    if(pruicssHwAttrs->instance == 0)
    {
        /* For ICSS0 */
        HW_WR_REG32(CSL_MSS_CTRL_U_BASE + CSL_MSS_CTRL_ICSSM0_PRU0_GPIO_OUT_CTRL, MSS_CTRL_ICSSM_PRU_GPIO_OUT_CTRL_VALUE);
        HW_WR_REG32(CSL_MSS_CTRL_U_BASE + CSL_MSS_CTRL_ICSSM0_PRU1_GPIO_OUT_CTRL, MSS_CTRL_ICSSM_PRU_GPIO_OUT_CTRL_VALUE);
    }
    else
    {
        /* For ICSS1 */
        HW_WR_REG32(CSL_MSS_CTRL_U_BASE + CSL_MSS_CTRL_ICSSM1_PRU0_GPIO_OUT_CTRL, MSS_CTRL_ICSSM_PRU1_GPIO_OUT_CTRL_VALUE);
        HW_WR_REG32(CSL_MSS_CTRL_U_BASE + CSL_MSS_CTRL_ICSSM1_PRU1_GPIO_OUT_CTRL, MSS_CTRL_ICSSM_PRU1_GPIO_OUT_CTRL_VALUE);
    }

    /* Selecting MII-RT mode in GPCFG mux */
    PRUICSS_setGpMuxSelect(pruIcss1Handle, PRUICSS_PRU0, PRUICSS_GP_MUX_SEL_MODE_MII);
    PRUICSS_setGpMuxSelect(pruIcss1Handle, PRUICSS_PRU1, PRUICSS_GP_MUX_SEL_MODE_MII);

    PRUICSS_setGpiMode(pruIcss1Handle, PRUICSS_PRU0, PRUICSS_GPI_MODE_MII_RT);
    PRUICSS_setGpiMode(pruIcss1Handle, PRUICSS_PRU1, PRUICSS_GPI_MODE_MII_RT);

    PRUICSS_setIcssCfgMiiMode(pruIcss1Handle, 0, PRUICSS_ICSS_CFG_MII_MODE_MII);
    PRUICSS_setIcssCfgMiiMode(pruIcss1Handle, 1, PRUICSS_ICSS_CFG_MII_MODE_MII);

    PRUICSS_setIcssCfgTxFifo(pruIcss1Handle, PRUICSS_TX_L1_FIFO, 1);
    PRUICSS_setIcssCfgTxFifo(pruIcss1Handle, PRUICSS_TX_L2_FIFO, 0);

    /*Disable PRUs. This is to ensure PRUs are not running when application is not initialized */
    PRUICSS_disableCore(pruIcss1Handle, PRUICSS_PRU0);
    PRUICSS_disableCore(pruIcss1Handle, PRUICSS_PRU1);
}

void tiesc_socParamsInit(bsp_params *bspInitParams)
{
    bsp_params_init(bspInitParams);
    bspInitParams->pruicss_handle = pruIcss1Handle;
    bspInitParams->interrupt_offset = tiesc_getArmInterruptOffset();
    bspInitParams->eeprom_read = tiesc_eepromRead;
    bspInitParams->eeprom_write = tiesc_eepromWrite;
    bspInitParams->spinlock_base_address = CSL_SPINLOCK0_BASE;
    bspInitParams->ethphy_init = tiesc_ethphyInit;
    /* For EtherCAT, Fast link detection using MLINK mode is required to support complete functionality */
    /* Check example documentation for more details */
#ifdef ENHANCED_LINK_ENABLED
    bspInitParams->enhancedlink_enable = TIESC_MDIO_RX_LINK_ENABLE;
#else
    bspInitParams->enhancedlink_enable = TIESC_MDIO_RX_LINK_DISABLE;
#endif
    bspInitParams->link0_polarity = TIESC_LINK0_POL;
    bspInitParams->link1_polarity = TIESC_LINK1_POL;
    bspInitParams->phy0_address = ((const ETHPHY_Attrs *)ETHPHY_getAttrs(CONFIG_ETHPHY0))->phyAddress;
    bspInitParams->phy1_address = ((const ETHPHY_Attrs *)ETHPHY_getAttrs(CONFIG_ETHPHY1))->phyAddress;
    bspInitParams->default_tiesc_eeprom = (const unsigned char *)(&(tiesc_eeprom));
    bspInitParams->eeprom_pointer_for_stack = &(pEEPROM);
#if CONFIG_PRU_ICSS0_IEP_CLK_FREQ_HZ == (250000000U)
    bspInitParams->pruicssIepClkFreq = TIESC_PRUICSS_IEP_CLOCK_FREQUENCY_250_MHZ;
#else
    bspInitParams->pruicssIepClkFreq = TIESC_PRUICSS_IEP_CLOCK_FREQUENCY_200_MHZ;
#endif
#ifndef ENABLE_PDI_TASK
    bspInitParams->pdi_isr = PDI_Isr;
#endif
#ifndef ENABLE_SYNC_TASK
    bspInitParams->sync0_isr = Sync0_Isr;
    bspInitParams->sync1_isr = Sync1_Isr;
#endif
}

void tiesc_displayEscVersion(uint16_t revision, uint16_t build)
{
    DebugP_log("Revision/Type : x%04X Build : x%04X\n\r", revision, build);
    DebugP_log("Firmware Version : %d.%d.%d\n\r", (revision >> 8), (build >> 8), (build & 0xFF));
}

void tiesc_setOutputLed(uint8_t mask)
{
	LED_setMask(gLedHandle[CONFIG_LED_DIGITAL_OUTPUT], (uint32_t)(0xFFFF & mask));
}

void tiesc_setRunLed(uint8_t value)
{
     if(value == 0U)
         LED_off(gLedHandle[CONFIG_LED_RUN], 0);
     if(value == 1U)
         LED_on(gLedHandle[CONFIG_LED_RUN], 0);
}

void tiesc_setErrorLed(uint8_t value)
{
    /* FIXME : To be added */
    /* if(value == 0U) */
    /*     LED_off(gLedHandle[CONFIG_LED_ERROR], 0); */
    /* if(value == 1U) */
    /*     LED_on(gLedHandle[CONFIG_LED_ERROR], 0); */
}

int32_t tiesc_getArmInterruptOffset()
{
    int32_t interruptOffset = CSLR_R5FSS0_CORE0_INTR_PRU_ICSSM0_PR1_HOST_INTR_PEND_0;
    return interruptOffset;
}

int32_t tiesc_eepromRead(uint8_t *buf, uint32_t len)
{
    int32_t retVal;
    retVal = EEPROM_read(gEepromHandle[CONFIG_EEPROM0], I2C_EEPROM_DATA_OFFSET, buf, len);
    return retVal;
}

int32_t tiesc_eepromWrite(uint8_t *buf,uint32_t len)
{
    int32_t retVal;
    retVal = EEPROM_write(gEepromHandle[CONFIG_EEPROM0], I2C_EEPROM_DATA_OFFSET, buf, len);
    return retVal;
}

void tiesc_ethphyInit(PRUICSS_Handle pruIcssHandle, uint8_t phy0addr,
                     uint8_t phy1addr, uint8_t enhancedlink_enable)
{
    uint32_t mdioBaseAddress = ((const ETHPHY_Attrs *)ETHPHY_getAttrs(CONFIG_ETHPHY0))->mdioBaseAddress;
    bool isRevA = false;

    /* Check if board is RevA (uses DP83869 PHY) */
    isRevA = isPcbRevisionA();
    
    /* Enable Auto MDI-X feature on both PHYs */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_AUTO_MDIX, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_AUTO_MDIX, NULL, 0);

    /* Configure LED0 as LINK_OK indicator if enhanced link detection is enabled */
    if(TIESC_MDIO_RX_LINK_ENABLE == enhancedlink_enable)
    {
        if(isRevA)
        {
            ETHPHY_DP83869_LedSourceConfig ledConfig;
            ledConfig.ledNum = ETHPHY_DP83869_LED0;
            ledConfig.mode = ETHPHY_DP83869_LED_MODE_LINK_OK;
            ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
            ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
        }
        else
        {
            ETHPHY_DP83826E_LedSourceConfig ledConfig;
            ledConfig.ledNum = ETHPHY_DP83826E_LED0;
            ledConfig.mode = ETHPHY_DP83826E_LED_MODE_LINK_OK;
            ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
            ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
        }
    }

    /* Common PHY configuration for both PHY types */
    /* Enable Extended Full-Duplex */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_EXTENDED_FD_ABILITY, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_EXTENDED_FD_ABILITY, NULL, 0);

    /* Enable Odd Nibble Detection */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_ODD_NIBBLE_DETECTION, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_ODD_NIBBLE_DETECTION, NULL, 0);

    /* Enable detection of RXERR during IDLE */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_ENHANCED_IPG_DETECTION, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_ENHANCED_IPG_DETECTION, NULL, 0);

    /* Configure LEDs based on PHY type */
    if(isRevA)
    {
        /* DP83869 PHY configuration */
        ETHPHY_DP83869_LedSourceConfig ledConfig;
        ETHPHY_DP83869_LedBlinkRateConfig ledBlinkConfig;
        ETHPHY_DP83869_FastLinkDownDetectionConfig fastLinkDownDetConfig;

        /* PHY pin LED_0 as link for fast link detection */
        ledConfig.ledNum = ETHPHY_DP83869_LED0;
        ledConfig.mode = ETHPHY_DP83869_LED_MODE_100BTX_LINK_UP;
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));

        /* PHY pin LED_1 as RX_ER. Needed for detecting RX_ER during frame. */
        ledConfig.ledNum = ETHPHY_DP83869_LED1;
        ledConfig.mode = ETHPHY_DP83869_LED_MODE_RX_ERROR;
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));

        /* PHY pin LED_2 as Rx/Tx Activity */
        ledConfig.ledNum = ETHPHY_DP83869_LED2;
        ledConfig.mode = ETHPHY_DP83869_LED_MODE_LINK_OK_AND_BLINK_ON_RX_TX;
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));

        /* Configure LED blink rate to 200ms */
        ledBlinkConfig.rate = ETHPHY_DP83869_LED_BLINK_RATE_200_MS;
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_BLINK_RATE, (void *)&ledBlinkConfig, sizeof(ledBlinkConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_BLINK_RATE, (void *)&ledBlinkConfig, sizeof(ledBlinkConfig));

        /* Enable fast link drop detection for EtherCAT */
        fastLinkDownDetConfig.mode = ETHPHY_DP83869_FAST_LINKDOWN_MODE_RX_ERR;
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_FAST_LINK_DOWN_DETECTION, (void *)&fastLinkDownDetConfig, sizeof(fastLinkDownDetConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_FAST_LINK_DOWN_DETECTION, (void *)&fastLinkDownDetConfig, sizeof(fastLinkDownDetConfig));
    }
    else
    {
        /* DP83826E PHY configuration */
        ETHPHY_DP83826E_LedSourceConfig ledConfig;
        ETHPHY_DP83826E_LedBlinkRateConfig ledBlinkConfig;
        ETHPHY_DP83826E_FastLinkDownDetectionConfig fastLinkDownDetConfig;
        
        /* PHY pin LED_0 as link for fast link detection */
        ledConfig.ledNum = ETHPHY_DP83826E_LED0;
        ledConfig.mode = ETHPHY_DP83826E_LED_MODE_MII_LINK_100BT_FD;
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
    
        /* PHY pin LED_1 as RX_ER. Needed for detecting RX_ER during frame. */
        ledConfig.ledNum = ETHPHY_DP83826E_LED1;
        /*For DP83286E, RX_ER is a separate pin (not an LED pin like DP83869E). Configuring LED_1 for 10M speed indication. */
        ledConfig.mode = ETHPHY_DP83826E_LED_MODE_SPEED_10BT;
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
    
        /* PHY pin LED_2 as Rx/Tx Activity */
        ledConfig.ledNum = ETHPHY_DP83826E_LED2;
        ledConfig.mode = ETHPHY_DP83826E_LED_MODE_RX_TX_ACTIVITY;
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_SOURCE, (void *)&ledConfig, sizeof(ledConfig));
    
        /* Configure LED blink rate to 200ms */
        ledBlinkConfig.rate = ETHPHY_DP83826E_LED_BLINK_RATE_200_MS;
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_CONFIGURE_LED_BLINK_RATE, (void *)&ledBlinkConfig, sizeof(ledBlinkConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_CONFIGURE_LED_BLINK_RATE, (void *)&ledBlinkConfig, sizeof(ledBlinkConfig));
    
        /* Enable fast link drop detection for EtherCAT */
        fastLinkDownDetConfig.mode = ETHPHY_DP83826E_FAST_LINKDOWN_MODE_RX_ERR;
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_FAST_LINK_DOWN_DETECTION, (void *)&fastLinkDownDetConfig, sizeof(fastLinkDownDetConfig));
        ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_FAST_LINK_DOWN_DETECTION, (void *)&fastLinkDownDetConfig, sizeof(fastLinkDownDetConfig));
    }

    /* Configure link interrupt mode based on enhancedlink_enable parameter */
    if(enhancedlink_enable == 0)
    {
        MDIO_enableLinkInterrupt(mdioBaseAddress, 0, phy0addr, MDIO_LINKSEL_MDIO_MODE);
        MDIO_enableLinkInterrupt(mdioBaseAddress, 1, phy1addr, MDIO_LINKSEL_MDIO_MODE);
    }
    else
    {
        MDIO_enableLinkInterrupt(mdioBaseAddress, 0, phy0addr, MDIO_LINKSEL_MLINK_MODE);
        MDIO_enableLinkInterrupt(mdioBaseAddress, 1, phy1addr, MDIO_LINKSEL_MLINK_MODE);
    }

    /* Common configuration for both PHY types */
    /* Enable MII mode */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_MII, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_MII, NULL, 0);

    /* Disable 1G advertisement and soft-reset to restart auto-negotiation */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_DISABLE_1000M_ADVERTISEMENT, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_DISABLE_1000M_ADVERTISEMENT, NULL, 0);

    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_SOFT_RESTART, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_SOFT_RESTART, NULL, 0);
}

void tiesc_ethphyEnablePowerDown()
{
    /*TODO: Need to enable the mux selection from the generated files*/
    /* Ensure that PHY register access is working by checking the Identifier register */
    while(SystemP_SUCCESS != ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_VERIFY_IDENTIFIER_REGISTER, NULL, 0));
    while(SystemP_SUCCESS != ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_VERIFY_IDENTIFIER_REGISTER, NULL, 0));

    /* Enable IEEE Power Down mode so that PHY does not establish any link */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_ENABLE_IEEE_POWER_DOWN, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_ENABLE_IEEE_POWER_DOWN, NULL, 0);
}

void tiesc_ethphyDisablePowerDown()
{
    /* Disable IEEE Power Down mode so that PHY does not establish any link */
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY0], ETHPHY_CMD_DISABLE_IEEE_POWER_DOWN, NULL, 0);
    ETHPHY_command(gEthPhyHandle[CONFIG_ETHPHY1], ETHPHY_CMD_DISABLE_IEEE_POWER_DOWN, NULL, 0);
}

/**
 * @brief Reads PCB revision from EEPROM and determines if it's RevA
 * 
 * @return bool True if board is RevA, False otherwise or if read fails
 */
bool isPcbRevisionA(void)
{
    uint8_t boardVer[EEPROM_READ_PCB_REV_DATA_LEN] = {0};
    int32_t status = EEPROM_read(gEepromHandle[CONFIG_EEPROM1], EEPROM_OFFSET_READ_PCB_REV, 
                                boardVer, EEPROM_READ_PCB_REV_DATA_LEN);
    
    if (status != SystemP_SUCCESS)
    {
        DebugP_log("EEPROM Read for PCB Revision failed!!!\n\r");
        return false;  /* Return false on read failure */
    }
    
    /* Check if board is Rev A - first character is 'A' and properly null-terminated */
    return (boardVer[0] == 'A' && boardVer[1] == '\0');
}
