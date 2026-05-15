/*
 * ipc_channel.h — public API for shared-memory + IpcNotify transport (both R5F cores).
 *
 * Policy (logging, timing, buffer fill, verification) stays out of this header's
 * implementation file pairing; see core0_app.c / core1_app.c.
 */

#ifndef IPC_CHANNEL_H
#define IPC_CHANNEL_H

#include <stdint.h>
#include <kernel/dpl/SystemP.h>
#include <drivers/hw_include/am261x/cslr_soc_defines.h>

/** Byte size of each directional payload (`req_buf` and `resp_buf`). */
#define IPC_BUF_LEN         (512U)

/** Local client id for IpcNotify; must match on master and worker. */
#define IPC_CLIENT_ID       (4U)

/** Core id macros from CSL; used as IpcNotify destination addresses. */
#define IPC_CORE_MASTER     (CSL_CORE_ID_R5FSS0_0)
#define IPC_CORE_WORKER     (CSL_CORE_ID_R5FSS0_1)

/*
 * Converts a wall-time timeout in milliseconds to DPL ticks for SemaphoreP_pend.
 * Valid while SysConfig keeps usecPerTick == 1000 (1 ms per tick); if you change DPL
 * tick rate, update this macro or pass ticks explicitly.
 */
#define IPC_MS_TO_TICKS(ms) ((uint32_t)(ms))

/*
 * Last argument to ipc_channel_master_send_request / ipc_channel_worker_send_reply
 * (forwarded to IpcNotify_sendMsg):
 *   FAST (0) — lowest latency; OK when at most one notify is in flight per direction.
 *   WAIT (1) — spin until mailbox TX FIFO accepts; use if you burst multiple notifies.
 */
#define IPC_DOORBELL_SEND_FAST   (0U)
#define IPC_DOORBELL_SEND_WAIT   (1U)

/**
 * Layout of the dual-core channel. Single-writer fields avoid spinlocks:
 *   - req_seq, req_buf: master only
 *   - resp_seq, resp_buf: worker only
 */
typedef struct
{
    volatile uint32_t req_seq;
    volatile uint32_t resp_seq;
    volatile uint32_t _reserved[14];
    uint8_t           req_buf[IPC_BUF_LEN];
    uint8_t           resp_buf[IPC_BUF_LEN];
} ipc_channel_t;

/** Shared instance; linker places both cores' view on the same physical SRAM. */
extern ipc_channel_t gIpcCh;

/* ---------- Master (Core 0) ---------- */

/** @see ipc_channel.c — construct semaphore + register reply notify client. */
int32_t ipc_channel_master_init(void);

/** @see ipc_channel.c — destroy master semaphore. */
void    ipc_channel_master_deinit(void);

/** @see ipc_channel.c — publish new req_seq after filling req_buf; returns new seq. */
uint32_t ipc_channel_master_commit_request(void);

/** @see ipc_channel.c — doorbell worker with transaction id `seq`. */
void ipc_channel_master_send_request(uint32_t seq, uint32_t waitFifoEmpty);

/** @see ipc_channel.c — block until reply ISR or timeout (DPL ticks, ≥1 ms/tick default). */
int32_t ipc_channel_master_wait_reply(uint32_t timeoutTicks);

/**
 * Wait for worker reply with a microsecond timeout (busy-poll + SemaphoreP_pend(0)).
 * Use when the timeout is shorter than the DPL tick period (e.g. 200 us with 1 ms/tick).
 * For multi-ms waits, prefer ipc_channel_master_wait_reply + IPC_MS_TO_TICKS.
 */
int32_t ipc_channel_master_wait_reply_usec(uint32_t timeoutUsec);

/* ---------- Worker (Core 1) ---------- */

/** @see ipc_channel.c — register request notify client. */
int32_t ipc_channel_worker_init(void);

/** @see ipc_channel.c — block until master request; returns seq payload. */
uint32_t ipc_channel_worker_wait_request(void);

/** @see ipc_channel.c — set resp_seq and doorbell master. */
void ipc_channel_worker_send_reply(uint32_t seq, uint32_t waitFifoEmpty);

/* ---------- Both cores ---------- */

/** @see ipc_channel.c — IpcNotify_syncAll barrier (wait forever). */
void ipc_channel_sync_all(void);

#endif /* IPC_CHANNEL_H */
