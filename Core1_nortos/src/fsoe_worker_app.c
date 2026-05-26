/*
 * fsoe_worker_app.c — Core 1: poll req_seq, process 6 B FSoE PDO, publish resp_seq.
 */

#include <stdint.h>
#include <string.h>

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>

#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#include "ipc_channel.h"
#include "fsoe_pdo.h"
#include "fsoe_manager.h"
#include "fsoe_worker_app.h"

static void fsoe_worker_board_init(void)
{
    Drivers_open();
    Board_driversOpen();
}

static void fsoe_ipc_service_init(void)
{
    int32_t status;

    status = ipc_channel_worker_init();
    DebugP_assert(status == SystemP_SUCCESS);

    fsoe_manager_init();
}

static void fsoe_worker_handle_request(void)
{
    fsoe_pdo_rx_t rx;
    fsoe_pdo_tx_t tx;
    uint8_t       tx_wire[FSOE_PDO_TX_BYTES];

    fsoe_pdo_rx_wire_decode(&gIpcCh.req_buf[FSOE_IPC_RX_OFFSET], &rx);
    fsoe_manager_process(&rx, &tx);
    fsoe_pdo_tx_wire_encode(&tx, tx_wire);
    (void)memcpy(&gIpcCh.resp_buf[FSOE_IPC_TX_OFFSET], tx_wire, FSOE_PDO_TX_BYTES);
}

void fsoe_worker_main(void *args)
{
    (void)args;

    fsoe_worker_board_init();
    fsoe_ipc_service_init();

    DebugP_log("[FSOE] worker ready (poll IPC, %u B PDO)\r\n", (unsigned)FSOE_PDO_TX_BYTES);

    for (;;)
    {
        (void)ipc_channel_worker_wait_request();
        fsoe_worker_handle_request();
        ipc_channel_worker_send_reply(gIpcCh.req_seq);
    }
}
