/*
 * Core 0 — EtherCAT bridge (FreeRTOS).
 * Owns board/flash, ipc_channel master, and EtherCAT TaskP stack start.
 * FSoE PDO ↔ IPC path: fsoe_ipc_master.h (called from ethercat_app.c).
 */

#ifndef ECAT_BRIDGE_APP_H
#define ECAT_BRIDGE_APP_H

#include <stdint.h>

/** Run IPC FSoE PDO self-test at startup (disable when cyclic PDO path owns IPC). */
#ifndef DUALCORE_IPC_SELF_TEST
#define DUALCORE_IPC_SELF_TEST            (0U)
#endif

/** Self-test iteration count (only when DUALCORE_IPC_SELF_TEST is 1). */
#define DUALCORE_IPC_SELF_TEST_ITERS      (50U)

/** Delay between self-test iterations (ms). */
#define DUALCORE_IPC_SELF_TEST_PERIOD_MS  (100U)

/** FreeRTOS task entry — board, IPC, optional self-test, EtherCAT TaskP stack. */
void ecat_bridge_task(void *args);

#endif /* ECAT_BRIDGE_APP_H */
