/*
 * ipc_pingpong.c - dual-core request/response IPC channel for AM2612
 *
 * ARCHITECTURE
 *   Core 0 (master, FreeRTOS):
 *     - fills req_buf with pseudo-random bytes
 *     - publishes a new sequence number and fires an IpcNotify doorbell
 *     - blocks on a binary semaphore until Core 1 replies
 *     - verifies resp_buf == req_buf + 1 (byte-wise, wrap at 0xFF)
 *     - prints PASS/FAIL plus round-trip time in microseconds
 *   Core 1 (worker, NoRTOS):
 *     - wakes on the master's doorbell (ISR sets a volatile flag)
 *     - calls process_buffer(req_buf -> resp_buf)
 *     - publishes resp_seq and fires a doorbell back to Core 0
 *     - process_buffer() is the swap-point for future ML / model code
 *
 * SYNCHRONIZATION
 *   No spinlock is needed: every control word and every buffer has a
 *   single writer.
 *       req_seq  : written only by Core 0, read by Core 1
 *       resp_seq : written only by Core 1, read by Core 0
 *       req_buf  : written only by Core 0, read by Core 1
 *       resp_buf : written only by Core 1, read by Core 0
 *   The IpcNotify doorbell acts as both wake-up and memory barrier; its
 *   32-bit payload carries the publisher's sequence number, used to
 *   detect packet drops, stale replies, and packet freezes (timeout).
 *
 * CACHE
 *   .bss.user_shared_mem is mapped non-cacheable by the SysConfig MPU
 *   setup on AM2612, so no CacheP_wb/CacheP_inv calls are required.
 *   If you ever relocate this struct to a cacheable region, insert
 *   cache operations at the points marked with "CACHE:" below.
 *
 * TUNING KNOBS (see #defines)
 *   IPC_BUF_LEN          payload size per direction
 *   IPC_PERIOD_MS        master loop period (1000 now, target 1 ms)
 *   IPC_RESP_TIMEOUT_MS  freeze-detect deadline
 *   IPC_TEST_ITERATIONS  0 == run forever
 *
 * NOTE on going below 1 ms period: the DPL is currently configured with
 *   usecPerTick = 1000  (see Debug/syscfg/ti_dpl_config.c)
 *   which gives a 1 ms FreeRTOS tick. To run faster than that, lower
 *   usecPerTick in SysConfig (e.g. 100 -> 10 kHz tick).
 */

#include <stdint.h>
#include <stdlib.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/ClockP.h>
#include <drivers/ipc_notify.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* ------------------------------------------------------------------ */
/* Configuration                                                       */
/* ------------------------------------------------------------------ */
#define IPC_BUF_LEN          (1024U)
#define IPC_CLIENT_ID        (4U)
#define IPC_PERIOD_MS        (1000U)
#define IPC_RESP_TIMEOUT_MS  (5U * IPC_PERIOD_MS)
#define IPC_TEST_ITERATIONS  (50U)            /* 0 = forever */

/* Remote core IDs. If your SDK uses different names, change here only. */
#define IPC_CORE_MASTER      (CSL_CORE_ID_R5FSS0_0)
#define IPC_CORE_WORKER      (CSL_CORE_ID_R5FSS0_1)

/* DPL is set to usecPerTick=1000, so 1 tick == 1 ms. */
#define IPC_MS_TO_TICKS(ms)  (ms)

/* ------------------------------------------------------------------ */
/* Shared channel layout (placed in shared SRAM by linker)             */
/* ------------------------------------------------------------------ */
typedef struct
{
    volatile uint32_t req_seq;           /* Core 0 writes, Core 1 reads */
    volatile uint32_t resp_seq;          /* Core 1 writes, Core 0 reads */
    volatile uint32_t _reserved[14];     /* keep buffers off the seq cache cluster */
    uint8_t           req_buf [IPC_BUF_LEN];
    uint8_t           resp_buf[IPC_BUF_LEN];
} ipc_channel_t;

ipc_channel_t gIpcCh
    __attribute__((aligned(128), section(".bss.user_shared_mem")));

/* ================================================================== */
/* Core 0 / Master                                                     */
/* ================================================================== */
static SemaphoreP_Object gMasterRespSem;

static void on_worker_reply(uint32_t remoteCoreId, uint16_t localClientId,
                            uint32_t msgValue, int32_t crcStatus, void *args) __attribute__((unused));
static void on_worker_reply(uint32_t remoteCoreId, uint16_t localClientId,
                            uint32_t msgValue, int32_t crcStatus, void *args)
{
    (void)remoteCoreId; (void)localClientId; (void)msgValue;
    (void)crcStatus;    (void)args;
    SemaphoreP_post(&gMasterRespSem);
}

void ipc_master_run(void *args)
{
    int32_t  status;
    uint32_t passed = 0U, failed = 0U, mis_seq = 0U, freezes = 0U;
    uint32_t rtt_min = 0xFFFFFFFFU, rtt_max = 0U, rtt_sum = 0U, rtt_n = 0U;
    uint32_t iter = 0U;

    (void)args;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("[Master] IPC ping-pong: %u B/dir, period %u ms, timeout %u ms\r\n",
               (unsigned)IPC_BUF_LEN, (unsigned)IPC_PERIOD_MS,
               (unsigned)IPC_RESP_TIMEOUT_MS);

    status = SemaphoreP_constructBinary(&gMasterRespSem, 0);
    DebugP_assert(status == SystemP_SUCCESS);

    status = IpcNotify_registerClient(IPC_CLIENT_ID, on_worker_reply, NULL);
    DebugP_assert(status == SystemP_SUCCESS);

    /* Seed PRNG with low 32 bits of microsecond clock */
    srand((unsigned int)(uint32_t)ClockP_getTimeUsec());

    /* Wait for Core 1 to have its callback registered before any traffic */
    IpcNotify_syncAll(SystemP_WAIT_FOREVER);
    DebugP_log("[Master] worker is ready, starting transactions\r\n");

    while ((IPC_TEST_ITERATIONS == 0U) || (iter < IPC_TEST_ITERATIONS))
    {
        for (uint32_t i = 0U; i < IPC_BUF_LEN; i++)
        {
            gIpcCh.req_buf[i] = (uint8_t)rand();
        }
        /* CACHE: if cacheable -> CacheP_wb(gIpcCh.req_buf, IPC_BUF_LEN, ...); */

        uint32_t expected_seq = gIpcCh.req_seq + 1U;
        gIpcCh.req_seq = expected_seq;

        uint32_t t0 = (uint32_t)ClockP_getTimeUsec();

        IpcNotify_sendMsg(IPC_CORE_WORKER, IPC_CLIENT_ID,
                          expected_seq, /*waitForFifoEmpty=*/1U);

        status = SemaphoreP_pend(&gMasterRespSem,
                                 IPC_MS_TO_TICKS(IPC_RESP_TIMEOUT_MS));

        uint32_t t1     = (uint32_t)ClockP_getTimeUsec();
        uint32_t rtt_us = t1 - t0;

        if (status != SystemP_SUCCESS)
        {
            freezes++;
            DebugP_log("[Master] iter=%4u  FREEZE  no reply for seq=%u within %u ms\r\n",
                       (unsigned)iter, (unsigned)expected_seq,
                       (unsigned)IPC_RESP_TIMEOUT_MS);
        }
        else if (gIpcCh.resp_seq != expected_seq)
        {
            mis_seq++;
            DebugP_log("[Master] iter=%4u  SEQ MISMATCH sent=%u got=%u (drop?)  RTT=%u us\r\n",
                       (unsigned)iter, (unsigned)expected_seq,
                       (unsigned)gIpcCh.resp_seq, (unsigned)rtt_us);
        }
        else
        {
            /* CACHE: if cacheable -> CacheP_inv(gIpcCh.resp_buf, IPC_BUF_LEN, ...); */
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
                rtt_sum += rtt_us; rtt_n++;
                DebugP_log("[Master] iter=%4u  PASS  seq=%u  RTT=%u us\r\n",
                           (unsigned)iter, (unsigned)expected_seq,
                           (unsigned)rtt_us);
            }
            else
            {
                failed++;
                DebugP_log("[Master] iter=%4u  FAIL  seq=%u  %u/%u bytes wrong  RTT=%u us\r\n",
                           (unsigned)iter, (unsigned)expected_seq,
                           (unsigned)bad, (unsigned)IPC_BUF_LEN,
                           (unsigned)rtt_us);
            }
        }

        iter++;
        ClockP_usleep(IPC_PERIOD_MS * 1000U);
    }

    DebugP_log("[Master] === Summary ===\r\n");
    DebugP_log("[Master]   iterations : %u\r\n", (unsigned)iter);
    DebugP_log("[Master]   passed     : %u\r\n", (unsigned)passed);
    DebugP_log("[Master]   failed     : %u\r\n", (unsigned)failed);
    DebugP_log("[Master]   seq miss   : %u\r\n", (unsigned)mis_seq);
    DebugP_log("[Master]   freezes    : %u\r\n", (unsigned)freezes);
    if (rtt_n > 0U)
    {
        DebugP_log("[Master]   RTT min/avg/max us : %u / %u / %u\r\n",
                   (unsigned)rtt_min,
                   (unsigned)(rtt_sum / rtt_n),
                   (unsigned)rtt_max);
    }

    SemaphoreP_destruct(&gMasterRespSem);
    /* Keep drivers open so the UART keeps flushing the summary lines. */
}

/* ================================================================== */
/* Core 1 / Worker                                                     */
/* ================================================================== */
static volatile uint32_t gWorkerNewReqSeq = 0U;

static void on_master_request(uint32_t remoteCoreId, uint16_t localClientId,
                              uint32_t msgValue, int32_t crcStatus, void *args) __attribute__((unused));
static void on_master_request(uint32_t remoteCoreId, uint16_t localClientId,
                              uint32_t msgValue, int32_t crcStatus, void *args)
{
    (void)remoteCoreId; (void)localClientId; (void)crcStatus; (void)args;
    gWorkerNewReqSeq = msgValue;
}

/*
 * Swap-point: replace this body with the real model / heavier compute
 * when ready. Contract: read len bytes from `in`, write len bytes to
 * `out`. Must not write to `in` and must finish before returning.
 */
static void process_buffer(const uint8_t *in, uint8_t *out, uint32_t len)
{
    for (uint32_t i = 0U; i < len; i++)
    {
        out[i] = (uint8_t)(in[i] + 1U);
    }
}

void ipc_worker_run(void *args)
{
    int32_t status;
    (void)args;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("[Worker] starting, client=%u\r\n", (unsigned)IPC_CLIENT_ID);

    status = IpcNotify_registerClient(IPC_CLIENT_ID, on_master_request, NULL);
    DebugP_assert(status == SystemP_SUCCESS);

    IpcNotify_syncAll(SystemP_WAIT_FOREVER);
    DebugP_log("[Worker] ready, entering service loop\r\n");

    for (;;)
    {
        while (gWorkerNewReqSeq == 0U)
        {
            __asm volatile ("wfi");
        }
        uint32_t seq = gWorkerNewReqSeq;
        gWorkerNewReqSeq = 0U;

        /* CACHE: if cacheable -> CacheP_inv(gIpcCh.req_buf, IPC_BUF_LEN, ...); */
        process_buffer((const uint8_t *)gIpcCh.req_buf,
                       (uint8_t *)gIpcCh.resp_buf,
                       IPC_BUF_LEN);
        /* CACHE: if cacheable -> CacheP_wb(gIpcCh.resp_buf, IPC_BUF_LEN, ...); */

        gIpcCh.resp_seq = seq;
        IpcNotify_sendMsg(IPC_CORE_MASTER, IPC_CLIENT_ID, seq, 1U);
    }
}
