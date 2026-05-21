/*
 * Core 0 — EtherCAT bridge (FreeRTOS).
 * Owns board/flash, ipc_channel master, and Plan A sync FSoE IPC to Core 1.
 */

#ifndef ECAT_BRIDGE_APP_H
#define ECAT_BRIDGE_APP_H

#include <stdint.h>

/** Run IPC ping-pong self-test at startup (disable when EtherCAT cyclic path owns IPC). */
#ifndef DUALCORE_IPC_SELF_TEST
#define DUALCORE_IPC_SELF_TEST            (0U)
#endif

/** Self-test iteration count (only when DUALCORE_IPC_SELF_TEST is 1). */
#define DUALCORE_IPC_SELF_TEST_ITERS      (50U)

/** Delay between self-test iterations (ms). */
#define DUALCORE_IPC_SELF_TEST_PERIOD_MS  (100U)

/**
 * Plan A: maximum wait for Core 1 FSoE handler reply within one bus cycle (µs).
 * Used by fsoe_ipc_exchange_sync(); tune when EtherCAT cycle is 1 ms.
 */
#define FSOE_IPC_REPLY_TIMEOUT_US         (500U)

/** Plan A sync exchange on gIpcCh (fill req_buf before calling). */
int32_t fsoe_ipc_exchange_sync(uint32_t *rtt_us_out);

/** FreeRTOS task entry — board, IPC, optional self-test, EtherCAT TaskP stack. */
void ecat_bridge_task(void *args);

#endif /* ECAT_BRIDGE_APP_H */
