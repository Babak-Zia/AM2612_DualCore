/*
 * Core 1 — FSoE safety worker (NoRTOS).
 * Services ipc_channel requests from the EtherCAT bridge on Core 0.
 */

#ifndef FSOE_WORKER_APP_H
#define FSOE_WORKER_APP_H

void fsoe_worker_main(void *args);

#endif /* FSOE_WORKER_APP_H */
