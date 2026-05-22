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
#include <objdef.h>

/*
 * OD variables are instantiated only in tiescappl.c (_SSC_AM2612_=1 with _TIESC_HW_).
 * This TU must reference them as extern: SSC-AM2612Objects.h maps PROTO to definitions when
 * _SSC_AM2612_==1, so force _SSC_AM2612_ to 0 here (initializers in SSC are skipped; linkage
 * resolves to tiescappl.c).
 */
#define _SSC_AM2612_ 0
#include <ecatslv.h>
#include "SSC-AM2612Objects.h"

#include "ethercat_app.h"
#include "fsoe_ipc_master.h"

/* ========================================================================== */
/*                         Types / locals                                      */
/* ========================================================================== */

/** Snapshot of RxPDO image (0x1600 → 0x7000..0x7003), little-endian wire layout. */
typedef struct
{
    UINT8 u7000_byte;
    INT8 u7001_arr[3];
    INT16 u7002_short;
    INT8 u7003_complex_byte;
    INT16 u7003_complex_short;
    UINT64 u7003_complex_long;
    REAL32 u7003_complex_float;
    REAL64 u7003_complex_double;
} EthercatApp_PdoRxDecoded;

/* ========================================================================== */
/*                       Local helpers                                           */
/* ========================================================================== */

/** Decode `pdo_rx` exactly like unpack into OD (must stay in sync with manage_pdo_tx mirror). */
static void ethercat_app_decode_pdo_rx(const uint8_t *pdo_rx, EthercatApp_PdoRxDecoded *out)
{
    uint16_t o = 0;
    const uint8_t *s = pdo_rx;

    out->u7000_byte = s[o++];
    out->u7001_arr[0] = (INT8)s[o++];
    out->u7001_arr[1] = (INT8)s[o++];
    out->u7001_arr[2] = (INT8)s[o++];
    (void)memcpy(&out->u7002_short, &s[o], sizeof(out->u7002_short));
    o = (uint16_t)(o + (uint16_t)sizeof(out->u7002_short));
    out->u7003_complex_byte = (INT8)s[o++];
    (void)memcpy(&out->u7003_complex_short, &s[o], sizeof(out->u7003_complex_short));
    o = (uint16_t)(o + (uint16_t)sizeof(out->u7003_complex_short));
    (void)memcpy(&out->u7003_complex_long, &s[o], sizeof(out->u7003_complex_long));
    o = (uint16_t)(o + (uint16_t)sizeof(out->u7003_complex_long));
    (void)memcpy(&out->u7003_complex_float, &s[o], sizeof(out->u7003_complex_float));
    o = (uint16_t)(o + (uint16_t)sizeof(out->u7003_complex_float));
    (void)memcpy(&out->u7003_complex_double, &s[o], sizeof(out->u7003_complex_double));
}

/* ========================================================================== */
/*                       API                                                     */
/* ========================================================================== */

void manage_pdo_rx(uint8_t *pdo_rx)
{
    EthercatApp_PdoRxDecoded dec;

    ethercat_app_decode_pdo_rx(pdo_rx, &dec);

    Single_Byte_Rx0x7000 = dec.u7000_byte;
    Array_Byte_RX0x7001.aEntries[0] = dec.u7001_arr[0];
    Array_Byte_RX0x7001.aEntries[1] = dec.u7001_arr[1];
    Array_Byte_RX0x7001.aEntries[2] = dec.u7001_arr[2];
    Single_Short_Rx0x7002 = dec.u7002_short;
    /* Debug: 0x7002 → 0x6002 so the master TxPDO reflects the last RxPDO short (packet dump / tracing). */
    Single_Short_Tx0x6002 = dec.u7002_short;
    Complex_Rx0x7003.Complex_Byte = dec.u7003_complex_byte;
    Complex_Rx0x7003.Complex_Short = dec.u7003_complex_short;
    Complex_Rx0x7003.Complex_Long = dec.u7003_complex_long;
    Complex_Rx0x7003.Complex_Float = dec.u7003_complex_float;
    Complex_Rx0x7003.Complex_Double = dec.u7003_complex_double;
}

void manage_pdo_tx(uint8_t *pdo_tx)
{
    uint16_t o = 0;
    uint8_t *d = pdo_tx;

    d[o++] = Single_Byte_Tx0x6000;
    d[o++] = (uint8_t)Array_Byte_TX0x6001.aEntries[0];
    d[o++] = (uint8_t)Array_Byte_TX0x6001.aEntries[1];
    d[o++] = (uint8_t)Array_Byte_TX0x6001.aEntries[2];
    (void)memcpy(&d[o], &Single_Short_Tx0x6002, sizeof(Single_Short_Tx0x6002));
    o = (uint16_t)(o + (uint16_t)sizeof(Single_Short_Tx0x6002));
    d[o++] = (uint8_t)Complex_Tx0x6003.Complex_Byte;
    (void)memcpy(&d[o], &Complex_Tx0x6003.Complex_Short, sizeof(Complex_Tx0x6003.Complex_Short));
    o = (uint16_t)(o + (uint16_t)sizeof(Complex_Tx0x6003.Complex_Short));
    (void)memcpy(&d[o], &Complex_Tx0x6003.Complex_Long, sizeof(Complex_Tx0x6003.Complex_Long));
    o = (uint16_t)(o + (uint16_t)sizeof(Complex_Tx0x6003.Complex_Long));
    (void)memcpy(&d[o], &Complex_Tx0x6003.Complex_Float, sizeof(Complex_Tx0x6003.Complex_Float));
    o = (uint16_t)(o + (uint16_t)sizeof(Complex_Tx0x6003.Complex_Float));
    (void)memcpy(&d[o], &Complex_Tx0x6003.Complex_Double, sizeof(Complex_Tx0x6003.Complex_Double));
}

static void fsoe_od_apply_rx_wire(const uint8_t wire[FSOE_PDO_RX_BYTES])
{
    fsoe_pdo_rx_t rx;

    fsoe_pdo_rx_wire_decode(wire, &rx);
    FSOE_Rx0x7100.FsoeCommand   = rx.fsoe_command;
    FSOE_Rx0x7100.SafeOutputs   = rx.safe_outputs;
    FSOE_Rx0x7100.ConnectionId  = rx.connection_id;
    FSOE_Rx0x7100.FsoeCRC       = rx.fsoe_crc;
}

static void fsoe_od_apply_tx_wire(const uint8_t wire[FSOE_PDO_TX_BYTES])
{
    fsoe_pdo_tx_t tx;

    fsoe_pdo_tx_wire_decode(wire, &tx);
    FSOE_Tx0x6100.FsoeStatus    = tx.fsoe_status;
    FSOE_Tx0x6100.SafeInputs    = tx.safe_inputs;
    FSOE_Tx0x6100.ConnectionId  = tx.connection_id;
    FSOE_Tx0x6100.FsoeCRC       = tx.fsoe_crc;
}

void manage_pdo_fsoe_rx(uint8_t *pdo_rx)
{
    uint8_t tx_wire[FSOE_PDO_TX_BYTES];
    int32_t status;

    /*
     * Master FSOE RxPDO (0x1610) → Core 1 manager → FSOE TxPDO for master (0x1A10).
     * CoE objects 0x7100 / 0x6100 are updated for visibility; cyclic Tx uses 0x6100
     * via manage_pdo_fsoe_tx() in APPL_InputMapping.
     */
    fsoe_od_apply_rx_wire(pdo_rx);

    status = fsoe_ipc_master_exchange(pdo_rx, tx_wire, NULL);
    if (status == SystemP_SUCCESS)
    {
        fsoe_od_apply_tx_wire(tx_wire);
    }
}

void manage_pdo_fsoe_tx(uint8_t *pdo_tx)
{
    fsoe_pdo_tx_t tx;
    uint8_t       wire[FSOE_PDO_TX_BYTES];

    tx.fsoe_status    = FSOE_Tx0x6100.FsoeStatus;
    tx.safe_inputs    = FSOE_Tx0x6100.SafeInputs;
    tx.connection_id  = FSOE_Tx0x6100.ConnectionId;
    tx.fsoe_crc       = FSOE_Tx0x6100.FsoeCRC;

    fsoe_pdo_tx_wire_encode(&tx, wire);
    (void)memcpy(pdo_tx, wire, FSOE_PDO_TX_BYTES);
}
