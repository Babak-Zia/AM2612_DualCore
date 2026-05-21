/*
 * ipc_channel.c — shared-memory + IpcNotify transport for dual R5F on AM2612.
 *
 * PURPOSE
 *   This file is the only place that wires together:
 *     - the shared struct `gIpcCh` (payload + sequence fields in `.bss.user_shared_mem`)
 *     - hardware mailbox interrupts via `IpcNotify`
 *     - a binary semaphore on the master so a FreeRTOS task can block until Core 1 replies
 *     - a volatile request token on the worker so Core 1 can sleep in `WFI` until notified
 *
 * WHAT BELONGS HERE
 *   Low-level transport only. No UART logging, no `Drivers_open`, no business logic,
 *   no verification of buffer contents. Application code lives under each core's src/ folder.
 *
 * PROTOCOL (single in-flight transaction)
 *   Master: fill `gIpcCh.req_buf` → `ipc_channel_master_commit_request()` publishes
 *           a new monotonic `req_seq` → `ipc_channel_master_send_request(seq, …)`
 *           doorbells the worker with that seq → `ipc_channel_master_wait_reply*()` blocks
 *           until the worker's reply ISR runs → master checks `gIpcCh.resp_seq == seq`
 *           and reads `resp_buf`.
 *   Worker: `ipc_channel_worker_wait_request()` returns the seq from the master's doorbell
 *           → read `req_buf`, produce `resp_buf` → `ipc_channel_worker_send_reply(seq, …)`
 *           sets `resp_seq` and doorbells the master.
 *
 * PREREQUISITE
 *   `Drivers_open()` (and typically `Board_driversOpen()`) must run before any
 *   `ipc_channel_*_init` / `IpcNotify_sendMsg` / `ipc_channel_sync_all`, because IpcNotify
 *   depends on driver initialization from SysConfig.
 */

#include "ipc_channel.h"

#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/ClockP.h>
#include <drivers/ipc_notify.h>

/* -------------------------------------------------------------------------- */
/* Master-side private state                                                  */
/* -------------------------------------------------------------------------- */

/*
 * Binary semaphore released from ISR when Core 1 sends a reply notify.
 * Only constructed on the master core via ipc_channel_master_init().
 */
static SemaphoreP_Object gMasterRespSem;

/*
 * IpcNotify callback: runs in interrupt context on Core 0 when Core 1 calls
 * IpcNotify_sendMsg toward the master. We only post the semaphore; keep this
 * minimal (no logging, no heavy work).
 */
static void ipc_on_worker_reply(uint32_t remoteCoreId, uint16_t localClientId,
                                uint32_t msgValue, int32_t crcStatus, void *args);
static void ipc_on_worker_reply(uint32_t remoteCoreId, uint16_t localClientId,
                                uint32_t msgValue, int32_t crcStatus, void *args)
{
    (void)remoteCoreId;
    (void)localClientId;
    (void)msgValue;
    (void)crcStatus;
    (void)args;
    SemaphoreP_post(&gMasterRespSem);
}

/*
 * Global channel: one instance in flash/BSS metadata per core image, but the
 * linker maps this section to the same physical SRAM on both CPUs so both
 * see the same bytes for req/resp buffers and sequence counters.
 */
ipc_channel_t gIpcCh
    __attribute__((aligned(128), section(".bss.user_shared_mem")));

/*
 * ipc_channel_master_init
 *
 * What it does
 *   - Constructs an initially-taken binary semaphore (pend will block until post).
 *   - Registers `IPC_CLIENT_ID` on this core so incoming notifies from the worker
 *     invoke `ipc_on_worker_reply`, which posts the semaphore.
 *
 * When to call
 *   Once on Core 0, after `Drivers_open` / board setup, before `ipc_channel_sync_all`
 *   and before any send/wait transaction. With **FreeRTOS**, the DPL semaphore is
 *   typically RTOS-backed: call **after** `vTaskStartScheduler()` (e.g. from the
 *   first task), not from `main()` before the scheduler starts.
 *
 * Return value
 *   SystemP_SUCCESS on success; on failure the semaphore is torn down if it was
 *   constructed but client registration failed.
 *
 * Notes
 *   - Does not unregister IpcNotify on deinit path mismatch; pair with
 *     ipc_channel_master_deinit for semaphore cleanup only.
 *   - `crcStatus` in the callback is unused here; see TI `IpcNotify_FxnCallback`.
 */
int32_t ipc_channel_master_init(void)
{
    int32_t status;

    status = SemaphoreP_constructBinary(&gMasterRespSem, 0);
    if (status != SystemP_SUCCESS)
    {
        return status;
    }

    status = IpcNotify_registerClient(IPC_CLIENT_ID, ipc_on_worker_reply, NULL);
    if (status != SystemP_SUCCESS)
    {
        SemaphoreP_destruct(&gMasterRespSem);
        return status;
    }

    return SystemP_SUCCESS;
}

/*
 * ipc_channel_master_deinit
 *
 * What it does
 *   Destroys the binary semaphore created in ipc_channel_master_init.
 *
 * When to call
 *   When the master task is finished and you want clean teardown (optional in
 *   embedded targets that never power down).
 *
 * What it does NOT do
 *   Unregister the IpcNotify client; the driver API used here does not add an
 *   explicit unregister in this layer.
 */
void ipc_channel_master_deinit(void)
{
    SemaphoreP_destruct(&gMasterRespSem);
}

/*
 * ipc_channel_master_commit_request
 *
 * What it does
 *   Atomically (single-writer: master only) increments `gIpcCh.req_seq` and returns
 *   the new value. That value is the contract id for this transaction: the worker
 *   should echo the same number in `resp_seq` and in the reply doorbell payload.
 *
 * Call order (required)
 *   1. Write `gIpcCh.req_buf[0 … IPC_BUF_LEN-1]` first.
 *   2. Then call this function.
 *   3. Then call ipc_channel_master_send_request(returned_seq, …).
 *
 * Why a separate step from send_request
 *   Lets the application bracket accurate timing: timestamp, then doorbell, without
 *   the library touching payload bytes.
 */
uint32_t ipc_channel_master_commit_request(void)
{
    uint32_t next = gIpcCh.req_seq + 1U;
    gIpcCh.req_seq = next;
    return next;
}

/*
 * ipc_channel_master_send_request
 *
 * Parameters
 *   seq
 *     Must be the value last returned by ipc_channel_master_commit_request for this
 *     transaction. It is sent as the 32-bit notify payload so the worker can stash it
 *     without reading shared memory first (though the worker may still read req_seq).
 *   waitFifoEmpty
 *     Passed to IpcNotify_sendMsg (TI SDK). Use **0** (`IPC_DOORBELL_SEND_FAST`) for
 *     minimum latency when only one doorbell is in flight per direction. Use **1**
 *     (`IPC_DOORBELL_SEND_WAIT`) to spin until the mailbox TX FIFO accepts the write
 *     if you pipeline multiple notifies back-to-back.
 *
 * What it does
 *   Sends an IpcNotify message to `IPC_CORE_WORKER` for client `IPC_CLIENT_ID`.
 *
 * Does NOT block for the reply; use ipc_channel_master_wait_reply after this returns.
 */
void ipc_channel_master_send_request(uint32_t seq, uint32_t waitFifoEmpty)
{
    IpcNotify_sendMsg(IPC_CORE_WORKER, IPC_CLIENT_ID, seq, waitFifoEmpty);
}

/*
 * ipc_channel_master_wait_reply
 *
 * Parameters
 *   timeoutTicks
 *     DPL clock ticks for SemaphoreP_pend. With default SysConfig `usecPerTick == 1000`,
 *     one tick is one millisecond; use IPC_MS_TO_TICKS(ms) from the header for readability.
 *
 * What it does
 *   Blocks the calling task until either:
 *     - the worker reply ISR posts the binary semaphore, or
 *     - the timeout expires.
 *
 * Return value
 *   SystemP_SUCCESS if the semaphore was posted (worker replied at the notify level).
 *   The application must still verify `gIpcCh.resp_seq` matches the expected seq and
 *   that `resp_buf` is coherent (see README: freeze vs seq mismatch vs data errors).
 *
 * Does NOT read or validate buffer contents.
 */
int32_t ipc_channel_master_wait_reply(uint32_t timeoutTicks)
{
    return SemaphoreP_pend(&gMasterRespSem, timeoutTicks);
}

/*
 * ipc_channel_master_wait_reply_usec
 *
 * Waits for the worker reply with a **microsecond** deadline.
 *
 * Why this exists
 *   `ipc_channel_master_wait_reply(timeoutTicks)` uses the DPL tick as the time base.
 *   With default SysConfig `usecPerTick == 1000`, one tick is **1 ms**, so you cannot
 *   express 200 µs (or any sub-millisecond timeout) accurately with ticks alone.
 *
 * How it works
 *   Tight loop: non-blocking `SemaphoreP_pend(..., 0)` until success, or until
 *   `ClockP_getTimeUsec() - t0 >= timeoutUsec`. This briefly busy-spins the master CPU
 *   while waiting (typically only a few µs until the ISR runs). Acceptable for very
 *   short timeouts; for multi-millisecond waits prefer `ipc_channel_master_wait_reply`
 *   with `IPC_MS_TO_TICKS` so the RTOS can block without burning cycles.
 *
 * Return value
 *   `SystemP_SUCCESS` when the semaphore was posted; `SystemP_FAILURE` on timeout.
 */
int32_t ipc_channel_master_wait_reply_usec(uint32_t timeoutUsec)
{
    uint64_t t0 = ClockP_getTimeUsec();

    for (;;)
    {
        int32_t st = SemaphoreP_pend(&gMasterRespSem, 0);
        if (st == SystemP_SUCCESS)
        {
            return SystemP_SUCCESS;
        }
        if ((ClockP_getTimeUsec() - t0) >= (uint64_t)timeoutUsec)
        {
            return SystemP_FAILURE;
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Worker-side private state                                                  */
/* -------------------------------------------------------------------------- */

/*
 * Written only from ipc_on_master_request (ISR on Core 1), read/cleared from the
 * worker main loop. Value zero means "no pending request"; non-zero holds the
 * latest request sequence from the master's doorbell.
 */
static volatile uint32_t gWorkerNewReqSeq;

/*
 * IpcNotify callback: runs on Core 1 when the master sends a notify. Stores the
 * message payload (the request seq). Do not call DebugP_log from here (static line
 * buffer in the shared-log writer is not ISR-safe on NoRTOS builds).
 */
static void ipc_on_master_request(uint32_t remoteCoreId, uint16_t localClientId,
                                  uint32_t msgValue, int32_t crcStatus, void *args);
static void ipc_on_master_request(uint32_t remoteCoreId, uint16_t localClientId,
                                  uint32_t msgValue, int32_t crcStatus, void *args)
{
    (void)remoteCoreId;
    (void)localClientId;
    (void)crcStatus;
    (void)args;
    gWorkerNewReqSeq = msgValue;
}

/*
 * ipc_channel_worker_init
 *
 * What it does
 *   Clears the pending-request flag and registers `IPC_CLIENT_ID` on this core so
 *   incoming notifies from the master invoke `ipc_on_master_request`.
 *
 * When to call
 *   Once on Core 1, after `Drivers_open` / board setup, before `ipc_channel_sync_all`.
 *
 * Return value
 *   IpcNotify_registerClient status (SystemP_SUCCESS when registration succeeds).
 */
int32_t ipc_channel_worker_init(void)
{
    gWorkerNewReqSeq = 0U;
    return IpcNotify_registerClient(IPC_CLIENT_ID, ipc_on_master_request, NULL);
}

/*
 * ipc_channel_worker_wait_request
 *
 * What it does
 *   Spins in a low-power wait (`WFI`) until `gWorkerNewReqSeq` becomes non-zero,
 *   then clears it and returns the sequence number to process for this transaction.
 *
 * When to call
 *   Once per transaction, after init and sync, before reading `gIpcCh.req_buf`.
 *
 * Notes
 *   - Not interrupt-safe for concurrent callers; single worker loop expected.
 *   - If the master floods notifies faster than the worker clears this flag, only
 *     the latest seq is retained (lossy); the protocol in ecat_bridge_app assumes one
 *     in-flight request at a time.
 */
uint32_t ipc_channel_worker_wait_request(void)
{
    while (gWorkerNewReqSeq == 0U)
    {
        __asm volatile ("wfi");
    }
    uint32_t seq = gWorkerNewReqSeq;
    gWorkerNewReqSeq = 0U;
    return seq;
}

/*
 * ipc_channel_worker_send_reply
 *
 * Parameters
 *   seq
 *     Must match the transaction id returned by ipc_channel_worker_wait_request for
 *     this iteration (and match `gIpcCh.req_seq` for a well-behaved master).
 *   waitFifoEmpty
 *     Same semantics as ipc_channel_master_send_request (typically **FAST** for this
 *     demo's single in-flight transaction pattern).
 *
 * What it does
 *   Publishes `gIpcCh.resp_seq = seq` then sends an IpcNotify toward the master so
 *   its ISR posts the master's binary semaphore.
 *
 * Call order (required)
 *   Fill `gIpcCh.resp_buf` first, then call this function.
 */
void ipc_channel_worker_send_reply(uint32_t seq, uint32_t waitFifoEmpty)
{
    gIpcCh.resp_seq = seq;
    IpcNotify_sendMsg(IPC_CORE_MASTER, IPC_CLIENT_ID, seq, waitFifoEmpty);
}

/*
 * ipc_channel_sync_all
 *
 * What it does
 *   Wraps `IpcNotify_syncAll(SystemP_WAIT_FOREVER)`: a global barrier across cores
 *   that participate in IpcNotify. Each core blocks here until every participant
 *   has entered the barrier—use after both sides have registered their clients so
 *   neither core sends traffic before the peer is ready.
 *
 * When to call
 *   Typically once at startup from both ecat_bridge_app and fsoe_worker_app, after init and
 *   before the main request/response loop.
 */
void ipc_channel_sync_all(void)
{
    IpcNotify_syncAll(SystemP_WAIT_FOREVER);
}
