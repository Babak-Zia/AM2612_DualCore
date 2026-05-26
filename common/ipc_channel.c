/*
 * ipc_channel.c — poll-only shared-memory IPC (req_seq / resp_seq + 512 B buffers).
 */

#include "ipc_channel.h"

#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/SystemP.h>

#define IPC_DSB()    __asm volatile ("dsb sy" ::: "memory")

ipc_channel_t gIpcCh
    __attribute__((aligned(128), section(".bss.user_shared_mem")));

static uint32_t sWorkerLastHandledReqSeq;

int32_t ipc_channel_master_init(void)
{
    return SystemP_SUCCESS;
}

uint32_t ipc_channel_master_commit_request(void)
{
    uint32_t next = gIpcCh.req_seq + 1U;

    gIpcCh.req_seq = next;
    IPC_DSB();

    return next;
}

int32_t ipc_channel_master_wait_reply_usec(uint32_t expected_seq, uint32_t timeoutUsec)
{
    uint64_t t0 = ClockP_getTimeUsec();

    for (;;)
    {
        if (gIpcCh.resp_seq == expected_seq)
        {
            IPC_DSB();
            return SystemP_SUCCESS;
        }
        if ((ClockP_getTimeUsec() - t0) >= (uint64_t)timeoutUsec)
        {
            return SystemP_FAILURE;
        }
    }
}

int32_t ipc_channel_worker_init(void)
{
    sWorkerLastHandledReqSeq = gIpcCh.req_seq;
    return SystemP_SUCCESS;
}

uint32_t ipc_channel_worker_wait_request(void)
{
    while (gIpcCh.req_seq == sWorkerLastHandledReqSeq)
    {
        __asm volatile ("nop");
    }

    IPC_DSB();

    return gIpcCh.req_seq;
}

void ipc_channel_worker_send_reply(uint32_t seq)
{
    gIpcCh.resp_seq = seq;
    IPC_DSB();
    sWorkerLastHandledReqSeq = seq;
}
