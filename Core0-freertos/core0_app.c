/*
 * core0_app.c — FreeRTOS master: policy, logging, timing, verification.
 * Transport: common/ipc_channel.c (zero-copy — read/write gIpcCh buffers in place).
 */

#include <stdint.h>
#include <stdlib.h>

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/ClockP.h>

#include "ipc_channel.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#define IPC_PERIOD_MS        (1000U)
#define IPC_RESP_TIMEOUT_MS  (5U * IPC_PERIOD_MS)
#define IPC_TEST_ITERATIONS  (50U)

void core0_app_run(void *args)
{
    int32_t  status;
    uint32_t passed = 0U, failed = 0U, mis_seq = 0U, freezes = 0U;
    uint32_t rtt_min = 0xFFFFFFFFU, rtt_max = 0U, rtt_sum = 0U, rtt_n = 0U;
    uint32_t iter = 0U;

    (void)args;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("[Core0] IPC (zero-copy): %u B/dir, period %u ms, timeout %u ms\r\n",
               (unsigned)IPC_BUF_LEN, (unsigned)IPC_PERIOD_MS,
               (unsigned)IPC_RESP_TIMEOUT_MS);

    status = ipc_channel_master_init();
    DebugP_assert(status == SystemP_SUCCESS);

    srand((unsigned int)(uint32_t)ClockP_getTimeUsec());

    ipc_channel_sync_all();
    DebugP_log("[Core0] worker ready, starting transactions\r\n");

    while ((IPC_TEST_ITERATIONS == 0U) || (iter < IPC_TEST_ITERATIONS))
    {
        for (uint32_t i = 0U; i < IPC_BUF_LEN; i++)
        {
            gIpcCh.req_buf[i] = (uint8_t)rand();
        }

        uint32_t expected_seq = ipc_channel_master_commit_request();

        uint32_t t0 = (uint32_t)ClockP_getTimeUsec();

        /* IPC_DOORBELL_SEND_FAST: do not spin for mailbox TX FIFO — lowest latency
         * for one in-flight transaction; use IPC_DOORBELL_SEND_WAIT if you burst notifies. */
        ipc_channel_master_send_request(expected_seq, IPC_DOORBELL_SEND_FAST);

        status = ipc_channel_master_wait_reply(IPC_MS_TO_TICKS(IPC_RESP_TIMEOUT_MS));

        uint32_t t1     = (uint32_t)ClockP_getTimeUsec();
        uint32_t rtt_us = t1 - t0;

        if (status != SystemP_SUCCESS)
        {
            freezes++;
            DebugP_log("[Core0] iter=%4u  FREEZE  seq=%u within %u ms  RTT=%u us\r\n",
                       (unsigned)iter, (unsigned)expected_seq,
                       (unsigned)IPC_RESP_TIMEOUT_MS, (unsigned)rtt_us);
        }
        else if (gIpcCh.resp_seq != expected_seq)
        {
            mis_seq++;
            DebugP_log("[Core0] iter=%4u  SEQ MISMATCH sent=%u got=%u  RTT=%u us\r\n",
                       (unsigned)iter, (unsigned)expected_seq,
                       (unsigned)gIpcCh.resp_seq, (unsigned)rtt_us);
        }
        else
        {
            uint32_t bad = 0U;
            for (uint32_t i = 0U; i < IPC_BUF_LEN; i++)
            {
                if (gIpcCh.resp_buf[i] != (uint8_t)(gIpcCh.req_buf[i] + 1U))
                {
                    bad++;
                }
            }
            if (bad == 0U)
            {
                passed++;
                if (rtt_us < rtt_min) rtt_min = rtt_us;
                if (rtt_us > rtt_max) rtt_max = rtt_us;
                rtt_sum += rtt_us;
                rtt_n++;
                DebugP_log("[Core0] iter=%4u  PASS  seq=%u  RTT=%u us\r\n",
                           (unsigned)iter, (unsigned)expected_seq,
                           (unsigned)rtt_us);
            }
            else
            {
                failed++;
                DebugP_log("[Core0] iter=%4u  FAIL  seq=%u  %u/%u bytes wrong  RTT=%u us\r\n",
                           (unsigned)iter, (unsigned)expected_seq,
                           (unsigned)bad, (unsigned)IPC_BUF_LEN,
                           (unsigned)rtt_us);
            }
        }

        iter++;
        ClockP_usleep(IPC_PERIOD_MS * 1000U);
    }

    DebugP_log("[Core0] === Summary ===\r\n");
    DebugP_log("[Core0]   iterations : %u\r\n", (unsigned)iter);
    DebugP_log("[Core0]   passed     : %u\r\n", (unsigned)passed);
    DebugP_log("[Core0]   failed     : %u\r\n", (unsigned)failed);
    DebugP_log("[Core0]   seq miss   : %u\r\n", (unsigned)mis_seq);
    DebugP_log("[Core0]   freezes    : %u\r\n", (unsigned)freezes);
    if (rtt_n > 0U)
    {
        DebugP_log("[Core0]   RTT min/avg/max us : %u / %u / %u\r\n",
                   (unsigned)rtt_min,
                   (unsigned)(rtt_sum / rtt_n),
                   (unsigned)rtt_max);
    }

    ipc_channel_master_deinit();
}
