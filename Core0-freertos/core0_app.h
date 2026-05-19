/*
 * Declarations shared with main_core0.c
 */

#ifndef CORE0_APP_H
#define CORE0_APP_H

/** IPC test timing (used by main_core0.c startup log and core0_app.c). */
#define CORE0_IPC_PERIOD_MS          (100U)
#define CORE0_IPC_RESP_TIMEOUT_US    (200U)

void core0_app_run(void *args);

#endif /* CORE0_APP_H */
