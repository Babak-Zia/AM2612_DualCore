/*
 * fsoe_ipc_master.c — Core 0: one FSoE PDO exchange (6 B) with Core 1 via gIpcCh.
 */

#include <string.h>

#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/SystemP.h>

#include "ipc_channel.h"
#include "fsoe_pdo.h"
#include "fsoe_ipc_master.h"

int32_t fsoe_ipc_master_exchange(const uint8_t rx_wire[FSOE_PDO_RX_BYTES],
                                 uint8_t tx_wire[FSOE_PDO_TX_BYTES],
                                 uint32_t *rtt_us_out)
{
    uint32_t expected_seq;
    uint32_t t0;
    uint32_t t1;
    int32_t  status;

    (void)memcpy(&gIpcCh.req_buf[FSOE_IPC_RX_OFFSET], rx_wire, FSOE_PDO_RX_BYTES);

    expected_seq = ipc_channel_master_commit_request();
    t0           = (uint32_t)ClockP_getTimeUsec();

    status = ipc_channel_master_wait_reply_usec(expected_seq, FSOE_IPC_REPLY_TIMEOUT_US);

    t1 = (uint32_t)ClockP_getTimeUsec();
    if (rtt_us_out != NULL)
    {
        *rtt_us_out = t1 - t0;
    }

    if (status != SystemP_SUCCESS)
    {
        return status;
    }

    if (gIpcCh.resp_seq != expected_seq)
    {
        return SystemP_FAILURE;
    }

    (void)memcpy(tx_wire, &gIpcCh.resp_buf[FSOE_IPC_TX_OFFSET], FSOE_PDO_TX_BYTES);

    return SystemP_SUCCESS;
}
