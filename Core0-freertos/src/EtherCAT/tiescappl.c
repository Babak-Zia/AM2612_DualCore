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
#include <string.h>

#include <ecat_def.h>

#if TIESC_APPLICATION

#if COE_SUPPORTED
#include <objdef.h>
#endif
#include <ecatappl.h>

#include <tiescutils.h>
#include <sdoserv.h>

/*
 * Define OD / process-data instances in this TU (SSC PROTO expands to definitions).
 * SSC is included only from tiescappl.h — do not include SSC-AM2612Objects.h here or
 * PROTO objects are redefined when the header is pulled in again.
 */
#define _SSC_AM2612_ 1
#define _TIESC_HW_
#include <ecatslv.h>
/* "…" picks src/tiescappl.h; <…> can resolve to SDK demo tiescappl.h first on the include path. */
#include "tiescappl.h"
#undef _TIESC_HW_

#if COE_SUPPORTED
/* OBJ_Read invokes this before the object Read callback; used to disambiguate SSC's "yes"
 * when the same symbol is both Read and Write (see yes() below). Declared in applInterface.h. */
extern void (*pAPPL_CoeReadInd)(UINT16 Index, UINT8 Subindex, BOOL CompleteAccess);
#endif

#include <tiescsoc.h>

#include "ethercat_app.h"

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern uint32_t appState;

/*
 * PDO layout (SSC-AM2612Objects.h / ESI SM2/SM3 DefaultSize 35):
 *   0x1600 / 0x1A00 — 29 bytes (0x7000..0x7003 / 0x6000..0x6003)
 *   0x160F / 0x1A0F — 11 bytes (0x70F0+0x70F1 / 0x60F0+0x60F1)
 */
#define AM2612_PDO_IMAGE_BYTES   ETHERCAT_APP_PDO_IMAGE_BYTES
#define AM2612_FSOE_PDO_BYTES    ETHERCAT_APP_FSOE_PDO_BYTES

/* ========================================================================== */
/*                       Function Definitions                                 */
/* ========================================================================== */

static void reset_rx_od(void)
{
    Single_Byte_Rx0x7000 = 0U;
    Array_Byte_RX0x7001.aEntries[0] = 0;
    Array_Byte_RX0x7001.aEntries[1] = 0;
    Array_Byte_RX0x7001.aEntries[2] = 0;
    Single_Short_Rx0x7002 = 0;
    Complex_Rx0x7003.Complex_Byte = 0;
    Complex_Rx0x7003.Complex_Short = 0;
    Complex_Rx0x7003.Complex_Long = (UINT64)0;
    Complex_Rx0x7003.Complex_Float = (REAL32)0;
    Complex_Rx0x7003.Complex_Double = (REAL64)0;
    FSOE_Com_Rx0x70F0.Command = 0U;
    FSOE_Com_Rx0x70F0.CRC_0 = 0U;
    FSOE_Com_Rx0x70F0.CRC_1 = 0U;
    FSOE_Com_Rx0x70F0.ConnectioID = 0U;
    FSOE_Data_Rx0x70F1.Safety_Data1 = 0U;
    FSOE_Data_Rx0x70F1.Safety_Data2 = 0U;
    FSOE_Com_TX0x60F0.Command = 0U;
    FSOE_Com_TX0x60F0.CRC_0 = 0U;
    FSOE_Com_TX0x60F0.CRC_1 = 0U;
    FSOE_Com_TX0x60F0.ConnectioID = 0U;
    FSOE_Data_TX0x60F1.Safety_Data1 = 0U;
    FSOE_Data_TX0x60F1.Safety_Data2 = 0U;
}

/*-----------------------------------------------------------------------------------------
------
------    application specific functions
------
-----------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
------
------    generic functions
------
-----------------------------------------------------------------------------------------*/
/////////////////////////////////////////////////////////////////////////////////////////
/**

\brief This function resets the outputs
*////////////////////////////////////////////////////////////////////////////////////////

void PDO_ResetOutputs(void)
{
    reset_rx_od();
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    The function is called when an error state was acknowledged by the master

*////////////////////////////////////////////////////////////////////////////////////////

void    APPL_AckErrorInd(uint16_t stateTrans)
{
    (void)stateTrans;
}

UINT8 No(UINT16 index, UINT8 subindex, UINT32 dataSize, UINT16 MBXMEM * pData, UINT8 bCompleteAccess)
{
    (void)index;
    (void)subindex;
    (void)dataSize;
    (void)pData;
    (void)bCompleteAccess;
    return ABORTIDX_READ_ONLY_ENTRY;
}

#if COE_SUPPORTED
/*
 * SSC Tool registers CoE hooks as "yes" / "No" in ApplicationObjDic (SSC-AM2612Objects.h).
 * Implementation normally lives in SSC-AM2612.c; with that file excluded, provide yes() here so
 * regeneration does not require editing generated headers.
 *
 * For 0x7000 / 0x7002 the OD uses yes for both Read and Write; OBJ_Read calls pAPPL_CoeReadInd
 * before the callback — we use that to choose read vs write path inside yes().
 */
UINT8 yes(UINT16 index, UINT8 subindex, UINT32 dataSize, UINT16 MBXMEM * pData, UINT8 bCompleteAccess);

static volatile uint8_t s_coeReadCbPending;

static void tiesc_CoeReadInd(UINT16 Index, UINT8 Subindex, BOOL CompleteAccess)
{
    OBJCONST TOBJECT OBJMEM *const pEntry = OBJ_GetObjectHandle(Index);

    (void)Subindex;
    (void)CompleteAccess;
    if ((pEntry != NULL) && (pEntry->Read == yes))
    {
        s_coeReadCbPending = 1U;
    }
}

static UINT8 coe_delegate_read(UINT16 index, UINT8 subindex, UINT32 dataSize, UINT16 MBXMEM * pData,
                               UINT8 bCompleteAccess)
{
    OBJCONST TOBJECT OBJMEM *const pEntry = OBJ_GetObjectHandle(index);
    TOBJECT stackEntry;

    if (pEntry == NULL)
    {
        return ABORTIDX_OBJECT_NOT_EXISTING;
    }

    (void)memcpy(&stackEntry, (const void *)pEntry, SIZEOF(TOBJECT));
    stackEntry.Read = NULL;
    stackEntry.Write = NULL;

    return OBJ_Read(index, subindex, dataSize, (OBJCONST TOBJECT OBJMEM *)&stackEntry, pData, bCompleteAccess);
}

static UINT8 coe_delegate_write(UINT16 index, UINT8 subindex, UINT32 dataSize, UINT16 MBXMEM * pData,
                                UINT8 bCompleteAccess)
{
    OBJCONST TOBJECT OBJMEM *const pEntry = OBJ_GetObjectHandle(index);
    TOBJECT stackEntry;

    if (pEntry == NULL)
    {
        return ABORTIDX_OBJECT_NOT_EXISTING;
    }

    (void)memcpy(&stackEntry, (const void *)pEntry, SIZEOF(TOBJECT));
    stackEntry.Read = NULL;
    stackEntry.Write = NULL;

    return OBJ_Write(index, subindex, dataSize, (OBJCONST TOBJECT OBJMEM *)&stackEntry, pData, bCompleteAccess);
}

UINT8 yes(UINT16 index, UINT8 subindex, UINT32 dataSize, UINT16 MBXMEM * pData, UINT8 bCompleteAccess)
{
    if (s_coeReadCbPending != 0U)
    {
        s_coeReadCbPending = 0U;
        return coe_delegate_read(index, subindex, dataSize, pData, bCompleteAccess);
    }

    return coe_delegate_write(index, subindex, dataSize, pData, bCompleteAccess);
}
#endif /* COE_SUPPORTED */
/*ECATCHANGE_START(V4.42.1) ECAT2*/
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from INIT to PREOP when
             all general settings were checked to start the mailbox handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
            The return code NOERROR_INWORK can be used, if the application cannot confirm
            the state transition immediately, in that case this function will be called cyclically
            until a value unequal NOERROR_INWORK is returned

*////////////////////////////////////////////////////////////////////////////////////////

uint16_t APPL_StartMailboxHandler(void)
{
#if COE_SUPPORTED
    pAPPL_CoeReadInd = tiesc_CoeReadInd;
#endif
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from PREEOP to INIT
             to stop the mailbox handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

uint16_t APPL_StopMailboxHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param    pIntMask    pointer to the AL Event Mask which will be written to the AL event Mask
                        register (0x204) when this function is succeeded. The event mask can be adapted
                        in this function
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from PREOP to SAFEOP when
             all general settings were checked to start the input handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
             When returning ALSTATUSCODE_NOERROR, the inputs has to be updated once before return.
            The return code NOERROR_INWORK can be used, if the application cannot confirm
            the state transition immediately, in that case the application need to be complete
            the transition by calling ECAT_StateChange
*////////////////////////////////////////////////////////////////////////////////////////

uint16_t APPL_StartInputHandler(uint16_t *pIntMask)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from SAFEOP to PREEOP
             to stop the input handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

uint16_t APPL_StopInputHandler(void)
{
#if ESC_EEPROM_EMULATION

    /*
        Since both SPI Flash and HVS (input) are on the same SPI instance, application will have to make sure that
        access to both does not happen concurrently. At this context, it is ensured that input(HVS) read wont happen.
    */
    if(bsp_get_eeprom_update_status())
    {
        bsp_set_eeprom_update_status(0);
        bsp_eeprom_emulation_flush();
    }

#endif
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from SAFEOP to OP when
             all general settings were checked to start the output handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
            The return code NOERROR_INWORK can be used, if the application cannot confirm
            the state transition immediately, in that case this function will be called cyclically
            until a value unequal NOERROR_INWORK is returned
*////////////////////////////////////////////////////////////////////////////////////////

uint16_t APPL_StartOutputHandler(void)
{
    /*always return NOERROR_INWORK the state transition will be finished by calling AL_ControlRes*/
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from OP to SAFEOP
             to stop the output handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

uint16_t APPL_StopOutputHandler(void)
{
    PDO_ResetOutputs();
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\return     0(ALSTATUSCODE_NOERROR), NOERROR_INWORK
\param      pInputSize  pointer to save the input process data length
\param      pOutputSize  pointer to save the output process data length

\brief    This function calculates the process data sizes from the actual SM-PDO-Assign
            and PDO mapping
*////////////////////////////////////////////////////////////////////////////////////////
uint16_t APPL_GenerateMapping(uint16_t *pInputSize, uint16_t *pOutputSize)
{
    uint16_t result = 0;
    uint16_t PDOAssignEntryCnt = 0;
    OBJCONST TOBJECT OBJMEM *pPDO = NULL;
    uint16_t PDOSubindex0 = 0;
    uint32_t *pPDOEntry = NULL;
    uint16_t PDOEntryCnt = 0;
    uint16_t InputSize = 0;
    uint16_t OutputSize = 0;

    /*Scan object 0x1C12 RXPDO assign*/
    for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sRxPDOassign.u16SubIndex0;
            PDOAssignEntryCnt++)
    {
        pPDO = OBJ_GetObjectHandle(sRxPDOassign.aEntries[PDOAssignEntryCnt]);

        if(pPDO != NULL)
        {
            PDOSubindex0 = *((uint16_t *)pPDO->pVarPtr);

            for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
            {
                pPDOEntry = (uint32_t *)((uint8_t *)pPDO->pVarPtr + (OBJ_GetEntryOffset((
                                             PDOEntryCnt + 1), pPDO) >> 3));     //goto PDO entry
                // we increment the expected output size depending on the mapped Entry
                OutputSize += (uint16_t)((*pPDOEntry) & 0xFF);
            }
        }
    }

    OutputSize = (OutputSize + 7) >> 3;

    /*Scan Object 0x1C13 TXPDO assign*/
    for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sTxPDOassign.u16SubIndex0;
            PDOAssignEntryCnt++)
    {
        pPDO = OBJ_GetObjectHandle(sTxPDOassign.aEntries[PDOAssignEntryCnt]);

        if(pPDO != NULL)
        {
            PDOSubindex0 = *((uint16_t *)pPDO->pVarPtr);

            for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
            {
                pPDOEntry = (uint32_t *)((uint8_t *)pPDO->pVarPtr + (OBJ_GetEntryOffset((
                                             PDOEntryCnt + 1), pPDO) >> 3));     //goto PDO entry
                // we increment the expected output size depending on the mapped Entry
                InputSize += (uint16_t)((*pPDOEntry) & 0xFF);
            }
        }
    }

    InputSize = (InputSize + 7) >> 3;

    *pInputSize = InputSize;
    *pOutputSize = OutputSize;
    return result;
}


/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to input process data
\brief      This function will copies the inputs from the local memory to the ESC memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
void APPL_InputMapping(uint16_t *pData)
{
    uint16_t j = 0;
    uint8_t *pTmpData = (uint8_t *)pData;

    for(j = 0; j < sTxPDOassign.u16SubIndex0; j++)
    {
        switch(sTxPDOassign.aEntries[j])
        {
            case 0x1A00:
                manage_pdo_tx(pTmpData);
                pTmpData += AM2612_PDO_IMAGE_BYTES;
                break;
            case 0x1A0F:
                manage_pdo_fsoe_tx(pTmpData);
                pTmpData += AM2612_FSOE_PDO_BYTES;
                break;
            default:
                break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to output process data

\brief    This function will copies the outputs from the ESC memory to the local memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
void APPL_OutputMapping(uint16_t *pData)
{
    uint16_t j = 0;
    uint8_t *pTmpData = (uint8_t *)pData;

    for(j = 0; j < sRxPDOassign.u16SubIndex0; j++)
    {
        switch(sRxPDOassign.aEntries[j])
        {
            case 0x1600:
                manage_pdo_rx(pTmpData);
                pTmpData += AM2612_PDO_IMAGE_BYTES;
                break;
            case 0x160F:
                manage_pdo_fsoe_rx(pTmpData);
                pTmpData += AM2612_FSOE_PDO_BYTES;
                break;
            default:
                break;
        }
    }
}

void APPL_Application(void)
{
    uint8_t rx = Single_Byte_Rx0x7000;

    /* TxPDO mirrors RxPDO image (loopback for bring-up / test) */
    Single_Byte_Tx0x6000 = Single_Byte_Rx0x7000;
    Array_Byte_TX0x6001.aEntries[0] = Array_Byte_RX0x7001.aEntries[0];
    Array_Byte_TX0x6001.aEntries[1] = Array_Byte_RX0x7001.aEntries[1];
    Array_Byte_TX0x6001.aEntries[2] = Array_Byte_RX0x7001.aEntries[2];
    Single_Short_Tx0x6002 = Single_Short_Rx0x7002;
    Complex_Tx0x6003.Complex_Byte = Complex_Rx0x7003.Complex_Byte;
    Complex_Tx0x6003.Complex_Short = Complex_Rx0x7003.Complex_Short;
    Complex_Tx0x6003.Complex_Long = Complex_Rx0x7003.Complex_Long;
    Complex_Tx0x6003.Complex_Float = Complex_Rx0x7003.Complex_Float;
    Complex_Tx0x6003.Complex_Double = Complex_Rx0x7003.Complex_Double;

    appState = (uint32_t)rx;

#ifdef ENABLE_PDI_TASK
    {
        static uint8_t prevLed = 0xFFU;

        if((rx & 0x1U) != prevLed)
        {
            tiesc_setOutputLed(rx & 0x1U);
            prevLed = (rx & 0x1U);
        }
    }
#endif
}

#if EXPLICIT_DEVICE_ID
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    The Explicit Device ID of the EtherCAT SubDevice

 \brief     Calculate the Explicit Device ID
*////////////////////////////////////////////////////////////////////////////////////////
uint16_t APPL_GetDeviceID()
{
    return 0xBABA;
}
#endif

#endif //#if TIESC_APPLICATION
