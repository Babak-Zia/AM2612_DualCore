/*

 * ipc_channel.h — shared SRAM IPC between Core 0 (master) and Core 1 (worker).

 *

 * Handshake: Core 0 bumps req_seq after writing req_buf; Core 1 polls req_seq,

 * writes resp_buf, sets resp_seq. Core 0 polls until resp_seq matches. No mailbox IRQ.

 */



#ifndef IPC_CHANNEL_H

#define IPC_CHANNEL_H



#include <stdint.h>

#include <kernel/dpl/SystemP.h>



#define IPC_BUF_LEN         (512U)



typedef struct

{

    volatile uint32_t req_seq;

    volatile uint32_t resp_seq;

    volatile uint32_t _reserved[14];

    uint8_t           req_buf[IPC_BUF_LEN];

    uint8_t           resp_buf[IPC_BUF_LEN];

} ipc_channel_t;



extern ipc_channel_t gIpcCh;



int32_t ipc_channel_master_init(void);

uint32_t ipc_channel_master_commit_request(void);

int32_t ipc_channel_master_wait_reply_usec(uint32_t expected_seq, uint32_t timeoutUsec);



int32_t ipc_channel_worker_init(void);

uint32_t ipc_channel_worker_wait_request(void);

void ipc_channel_worker_send_reply(uint32_t seq);



#endif /* IPC_CHANNEL_H */

