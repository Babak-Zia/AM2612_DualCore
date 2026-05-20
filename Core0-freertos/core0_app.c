/*
 * core0_app.c — FreeRTOS master: policy, logging, timing, verification.
 * Transport: common/ipc_channel.c (zero-copy — read/write gIpcCh buffers in place).
 *
 * Drivers_open / board_flash_reset / Board_driversOpen run at start of core0_app_run()
 * (after scheduler). All DebugP_log calls run after that block.
 */

#include <stdint.h>
#include <stdlib.h>

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/ClockP.h>

#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#include "ipc_channel.h"
#include "core0_app.h"

#define IPC_TEST_ITERATIONS    (50U)

typedef struct
{
    uint32_t passed;
    uint32_t failed;
    uint32_t mis_seq;
    uint32_t freezes;
    uint32_t rtt_min;
    uint32_t rtt_max;
    uint32_t rtt_sum;
    uint32_t rtt_n;
} Core0IpcStats;

/** Seed libc rand() from a free-running µs counter so test payloads differ each boot. */
static void core0_seed_rand_from_clock(void)
{
    srand((unsigned int)(uint32_t)ClockP_getTimeUsec());
}

static void core0_fill_req_random(void)
{
    for (uint32_t i = 0U; i < IPC_BUF_LEN; i++)
    {
        gIpcCh.req_buf[i] = (uint8_t)rand();
    }
}

static uint32_t core0_count_resp_mismatch_bytes(void)
{
    uint32_t bad = 0U;
    for (uint32_t i = 0U; i < IPC_BUF_LEN; i++)
    {
        if (gIpcCh.resp_buf[i] != (uint8_t)(gIpcCh.req_buf[i] + 1U))
        {
            bad++;
        }
    }
    return bad;
}

static void core0_ipc_startup(void)
{
    int32_t status;
    
    DebugP_log("[Core0] IPC (zero-copy): %u B/dir, period %u ms, reply timeout %u us\r\n",
               (unsigned)IPC_BUF_LEN, (unsigned)CORE0_IPC_PERIOD_MS,
               (unsigned)CORE0_IPC_RESP_TIMEOUT_US);
    core0_seed_rand_from_clock();

    status = ipc_channel_master_init();
    DebugP_assert(status == SystemP_SUCCESS);

    ipc_channel_sync_all();
    DebugP_log("[Core0] worker ready, starting transactions\r\n");
}

static void core0_ipc_one_iteration(uint32_t iter, Core0IpcStats *st)
{
    int32_t  status;
    uint32_t expected_seq;
    uint32_t t0;
    uint32_t t1;
    uint32_t rtt_us;

    core0_fill_req_random();

    expected_seq = ipc_channel_master_commit_request();
    t0           = (uint32_t)ClockP_getTimeUsec();

    ipc_channel_master_send_request(expected_seq, IPC_DOORBELL_SEND_FAST);

    status = ipc_channel_master_wait_reply_usec(CORE0_IPC_RESP_TIMEOUT_US);

    t1     = (uint32_t)ClockP_getTimeUsec();
    rtt_us = t1 - t0;

    if (status != SystemP_SUCCESS)
    {
        st->freezes++;
        DebugP_log("[Core0] iter=%4u  FREEZE  seq=%u within %u us  RTT=%u us\r\n",
                   (unsigned)iter, (unsigned)expected_seq,
                   (unsigned)CORE0_IPC_RESP_TIMEOUT_US, (unsigned)rtt_us);
    }
    else if (gIpcCh.resp_seq != expected_seq)
    {
        st->mis_seq++;
        DebugP_log("[Core0] iter=%4u  SEQ MISMATCH sent=%u got=%u  RTT=%u us\r\n",
                   (unsigned)iter, (unsigned)expected_seq,
                   (unsigned)gIpcCh.resp_seq, (unsigned)rtt_us);
    }
    else
    {
        uint32_t bad = core0_count_resp_mismatch_bytes();
        if (bad == 0U)
        {
            st->passed++;
            if (rtt_us < st->rtt_min)
            {
                st->rtt_min = rtt_us;
            }
            if (rtt_us > st->rtt_max)
            {
                st->rtt_max = rtt_us;
            }
            st->rtt_sum += rtt_us;
            st->rtt_n++;
            DebugP_log("[Core0] iter=%4u  PASS  seq=%u  RTT=%u us\r\n",
                       (unsigned)iter, (unsigned)expected_seq,
                       (unsigned)rtt_us);
        }
        else
        {
            st->failed++;
            DebugP_log("[Core0] iter=%4u  FAIL  seq=%u  %u/%u bytes wrong  RTT=%u us\r\n",
                       (unsigned)iter, (unsigned)expected_seq,
                       (unsigned)bad, (unsigned)IPC_BUF_LEN,
                       (unsigned)rtt_us);
        }
    }
}

static void core0_ipc_print_summary(uint32_t iterations, const Core0IpcStats *st)
{
    DebugP_log("[Core0] === Summary ===\r\n");
    DebugP_log("[Core0]   iterations : %u\r\n", (unsigned)iterations);
    DebugP_log("[Core0]   passed     : %u\r\n", (unsigned)st->passed);
    DebugP_log("[Core0]   failed     : %u\r\n", (unsigned)st->failed);
    DebugP_log("[Core0]   seq miss   : %u\r\n", (unsigned)st->mis_seq);
    DebugP_log("[Core0]   freezes    : %u\r\n", (unsigned)st->freezes);
    if (st->rtt_n > 0U)
    {
        DebugP_log("[Core0]   RTT min/avg/max us : %u / %u / %u\r\n",
                   (unsigned)st->rtt_min,
                   (unsigned)(st->rtt_sum / st->rtt_n),
                   (unsigned)st->rtt_max);
    }
}

void core0_app_run(void *args)
{
    Core0IpcStats st = { .rtt_min = 0xFFFFFFFFU };
    uint32_t        iter = 0U;

    (void)args;

    Drivers_open();
#if (defined(CONFIG_OSPI_NUM_INSTANCES) && (CONFIG_OSPI_NUM_INSTANCES > 0))
    board_flash_reset(gOspiHandle[CONFIG_OSPI0]);
#endif
    Board_driversOpen();

    core0_ipc_startup();

    while ((IPC_TEST_ITERATIONS == 0U) || (iter < IPC_TEST_ITERATIONS))
    {
        core0_ipc_one_iteration(iter, &st);
        iter++;
        ClockP_usleep(CORE0_IPC_PERIOD_MS * 1000U);
    }

    core0_ipc_print_summary(iter, &st);
    ipc_channel_master_deinit();
}
