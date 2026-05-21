/*
 * ecat_bridge_app.c — Core 0 EtherCAT bridge (FreeRTOS).
 *
 * Responsibilities (today): board init, ipc_channel master, Plan A sync exchange
 * API for FSoE payloads. EtherCAT stack hooks will call fsoe_ipc_exchange_sync()
 * from the cyclic PDO path once integrated.
 *
 * Transport: common/ipc_channel.c (zero-copy on gIpcCh.req_buf / resp_buf).
 */

#include <stdint.h>
#include <stdlib.h>

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/ClockP.h>
#include <FreeRTOS.h>
#include <task.h>

#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#include "ipc_channel.h"
#include "ecat_bridge_app.h"
#include "tiescutils.h"

/* -------------------------------------------------------------------------- */
/* Board + IPC bring-up                                                       */
/* -------------------------------------------------------------------------- */

static void ecat_bridge_board_init(void)
{
    Drivers_open();
#if (defined(CONFIG_OSPI_NUM_INSTANCES) && (CONFIG_OSPI_NUM_INSTANCES > 0))
    board_flash_reset(gOspiHandle[CONFIG_OSPI0]);
#endif
    Board_driversOpen();
}

static void fsoe_ipc_bringup(void)
{
    int32_t status;

    DebugP_log("[ECAT] FSoE IPC: %u B/dir, reply timeout %u us (Plan A)\r\n",
               (unsigned)IPC_BUF_LEN, (unsigned)FSOE_IPC_REPLY_TIMEOUT_US);

    status = ipc_channel_master_init();
    DebugP_assert(status == SystemP_SUCCESS);

    ipc_channel_sync_all();
    DebugP_log("[ECAT] Core1 FSoE worker ready\r\n");
}

/* -------------------------------------------------------------------------- */
/* Plan A — one in-flight request, sync reply (future hot path)               */
/* -------------------------------------------------------------------------- */

/**
 * Single master→worker→master round trip on gIpcCh.
 * Caller must fill req_buf before commit (or use self-test helpers).
 *
 * \param rtt_us_out  Optional RTT in µs (NULL to skip).
 * \return SystemP_SUCCESS if reply seq matches and arrived within timeout.
 */
int32_t fsoe_ipc_exchange_sync(uint32_t *rtt_us_out)
{
    uint32_t expected_seq;
    uint32_t t0;
    uint32_t t1;
    int32_t  status;

    expected_seq = ipc_channel_master_commit_request();
    t0           = (uint32_t)ClockP_getTimeUsec();

    ipc_channel_master_send_request(expected_seq, IPC_DOORBELL_SEND_FAST);

    status = ipc_channel_master_wait_reply_usec(FSOE_IPC_REPLY_TIMEOUT_US);

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

    return SystemP_SUCCESS;
}

/* -------------------------------------------------------------------------- */
/* Optional bring-up self-test (stub FSoE: resp[i] = req[i] + 1)              */
/* -------------------------------------------------------------------------- */

#if (DUALCORE_IPC_SELF_TEST != 0)

typedef struct
{
    uint32_t passed;
    uint32_t failed;
    uint32_t timeouts;
    uint32_t rtt_min;
    uint32_t rtt_max;
    uint32_t rtt_sum;
    uint32_t rtt_n;
} FsoeIpcSelfTestStats;

static void fsoe_ipc_self_test_seed_rand(void)
{
    srand((unsigned int)(uint32_t)ClockP_getTimeUsec());
}

static void fsoe_ipc_self_test_fill_req(void)
{
    for (uint32_t i = 0U; i < IPC_BUF_LEN; i++)
    {
        gIpcCh.req_buf[i] = (uint8_t)rand();
    }
}

static uint32_t fsoe_ipc_self_test_count_bad_bytes(void)
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

static void fsoe_ipc_self_test_one(uint32_t iter, FsoeIpcSelfTestStats *st)
{
    uint32_t rtt_us;
    int32_t  status;

    fsoe_ipc_self_test_fill_req();

    status = fsoe_ipc_exchange_sync(&rtt_us);

    if (status != SystemP_SUCCESS)
    {
        st->timeouts++;
        DebugP_log("[ECAT] self-test iter=%4u  TIMEOUT/SEQ  RTT=%u us\r\n",
                   (unsigned)iter, (unsigned)rtt_us);
        return;
    }

    {
        uint32_t bad = fsoe_ipc_self_test_count_bad_bytes();

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
            DebugP_log("[ECAT] self-test iter=%4u  PASS  RTT=%u us\r\n",
                       (unsigned)iter, (unsigned)rtt_us);
        }
        else
        {
            st->failed++;
            DebugP_log("[ECAT] self-test iter=%4u  FAIL  %u/%u bytes  RTT=%u us\r\n",
                       (unsigned)iter, (unsigned)bad, (unsigned)IPC_BUF_LEN,
                       (unsigned)rtt_us);
        }
    }
}

static void fsoe_ipc_self_test_print_summary(uint32_t iterations,
                                             const FsoeIpcSelfTestStats *st)
{
    DebugP_log("[ECAT] === FSoE IPC self-test ===\r\n");
    DebugP_log("[ECAT]   iterations : %u\r\n", (unsigned)iterations);
    DebugP_log("[ECAT]   passed     : %u\r\n", (unsigned)st->passed);
    DebugP_log("[ECAT]   failed     : %u\r\n", (unsigned)st->failed);
    DebugP_log("[ECAT]   timeouts   : %u\r\n", (unsigned)st->timeouts);
    if (st->rtt_n > 0U)
    {
        DebugP_log("[ECAT]   RTT min/avg/max us : %u / %u / %u\r\n",
                   (unsigned)st->rtt_min,
                   (unsigned)(st->rtt_sum / st->rtt_n),
                   (unsigned)st->rtt_max);
    }
}

static void fsoe_ipc_self_test_run(void)
{
    FsoeIpcSelfTestStats st = { .rtt_min = 0xFFFFFFFFU };
    uint32_t             iter = 0U;

    fsoe_ipc_self_test_seed_rand();
    DebugP_log("[ECAT] FSoE IPC self-test: %u iters, %u ms period\r\n",
               (unsigned)DUALCORE_IPC_SELF_TEST_ITERS,
               (unsigned)DUALCORE_IPC_SELF_TEST_PERIOD_MS);

    while (iter < DUALCORE_IPC_SELF_TEST_ITERS)
    {
        fsoe_ipc_self_test_one(iter, &st);
        iter++;
        ClockP_usleep(DUALCORE_IPC_SELF_TEST_PERIOD_MS * 1000U);
    }

    fsoe_ipc_self_test_print_summary(iter, &st);
}

#endif /* DUALCORE_IPC_SELF_TEST */

/* -------------------------------------------------------------------------- */
/* Task entry                                                                 */
/* -------------------------------------------------------------------------- */

void ecat_bridge_task(void *args)
{
    (void)args;

    ecat_bridge_board_init();
    fsoe_ipc_bringup();

#if (DUALCORE_IPC_SELF_TEST != 0)
    fsoe_ipc_self_test_run();
#endif

    DebugP_log("[ECAT] starting EtherCAT stack (TaskP MainLoop)...\r\n");
    ethercat_subdevice_start();

    /*
     * EtherCAT runs in TaskP task1 (and PDI/LED/SYNC tasks from task1_init).
     * This FreeRTOS task exits; main_core0 deletes it (same pattern as Babak).
     */
}
