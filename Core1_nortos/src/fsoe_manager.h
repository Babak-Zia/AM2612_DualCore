/*
 * fsoe_manager.h — Core 1 FSoE safety logic (NoRTOS).
 *
 * Consumes master RxPDO content from IPC and produces slave TxPDO content for IPC.
 * Replace the implementation with the product FSoE stack when available.
 */

#ifndef FSOE_MANAGER_H
#define FSOE_MANAGER_H

#include "fsoe_pdo.h"

/** One-time setup (parameters, connections). Safe to call once at worker start. */
void fsoe_manager_init(void);

/**
 * Process one FSoE request and build the response.
 *
 * \param rx  Decoded FSOE_Rx PDO from Core 0 (master outputs).
 * \param tx  Out: FSOE_Tx PDO to send back to Core 0 (master inputs).
 */
void fsoe_manager_process(const fsoe_pdo_rx_t *rx, fsoe_pdo_tx_t *tx);

#endif /* FSOE_MANAGER_H */
