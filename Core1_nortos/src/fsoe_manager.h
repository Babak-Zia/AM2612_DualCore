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
 * \param rx  Decoded FSoE RxPDO from Core 0 (0x160F wire, 11 bytes).
 * \param tx  Out: FSoE TxPDO for Core 0 (0x1A0F wire, 11 bytes).
 */
void fsoe_manager_process(const fsoe_pdo_rx_t *rx, fsoe_pdo_tx_t *tx);

#endif /* FSOE_MANAGER_H */
