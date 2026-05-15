/*
 * core1_app.c — NoRTOS worker: receive, process, reply. Replace process_buffer
 * with your model. Transport: common/ipc_channel.c (zero-copy on gIpcCh).
 */

#include <stdint.h>

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>

#include "ipc_channel.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

static void process_buffer(const uint8_t *in, uint8_t *out, uint32_t len)
{
    for (uint32_t i = 0U; i < len; i++)
    {
        out[i] = (uint8_t)(in[i] + 1U);
    }
}

void core1_app_run(void *args)
{
    int32_t status;

    (void)args;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("[Core1] starting, client=%u\r\n", (unsigned)IPC_CLIENT_ID);

    status = ipc_channel_worker_init();
    DebugP_assert(status == SystemP_SUCCESS);

    ipc_channel_sync_all();
    DebugP_log("[Core1] ready, service loop\r\n");

    for (;;)
    {
        uint32_t seq = ipc_channel_worker_wait_request();

        process_buffer((const uint8_t *)gIpcCh.req_buf,
                       (uint8_t *)gIpcCh.resp_buf,
                       IPC_BUF_LEN);

        ipc_channel_worker_send_reply(seq, IPC_DOORBELL_SEND_FAST);
    }
}
