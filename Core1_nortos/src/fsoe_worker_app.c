/*
 * fsoe_worker_app.c — Core 1 FSoE safety worker (NoRTOS).
 *
 * Waits on ipc_channel doorbell, processes req_buf into resp_buf, replies.
 * Replace fsoe_worker_process_payload() with the real FSoE stack handler.
 */

#include <stdint.h>

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>

#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#include "ipc_channel.h"
#include "fsoe_worker_app.h"

/**
 * Process one FSoE-related IPC payload (stub: byte-wise increment).
 * Plan A: keep this bounded; Core 0 waits up to FSOE_IPC_REPLY_TIMEOUT_US.
 */
static void fsoe_worker_process_payload(const uint8_t *req,
                                        uint8_t *resp,
                                        uint32_t len)
{
    for (uint32_t i = 0U; i < len; i++)
    {
        resp[i] = (uint8_t)(req[i] + 1U);
    }
}

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

    ipc_channel_sync_all();
}

void fsoe_worker_main(void *args)
{
    (void)args;

    fsoe_worker_board_init();

    DebugP_log("[FSOE] worker starting (client=%u)\r\n", (unsigned)IPC_CLIENT_ID);

    fsoe_ipc_service_init();
    DebugP_log("[FSOE] ready — service loop\r\n");

    for (;;)
    {
        uint32_t seq = ipc_channel_worker_wait_request();

        fsoe_worker_process_payload((const uint8_t *)gIpcCh.req_buf,
                                    (uint8_t *)gIpcCh.resp_buf,
                                    IPC_BUF_LEN);

        ipc_channel_worker_send_reply(seq, IPC_DOORBELL_SEND_FAST);
    }
}
