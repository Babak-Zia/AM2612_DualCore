/*
 * fsoe_ipc_master.h — Core 0 synchronous FSoE IPC (Plan A).
 *
 * Called from the EtherCAT PDO path when FSOE RxPDO data arrives. Fills gIpcCh.req_buf
 * with the Rx wire image, doorbells Core 1, waits for the Tx wire image in resp_buf.
 */

#ifndef FSOE_IPC_MASTER_H
#define FSOE_IPC_MASTER_H

#include <stdint.h>
#include <kernel/dpl/SystemP.h>

#include "fsoe_pdo.h"

#ifndef FSOE_IPC_REPLY_TIMEOUT_US
/** Max wait for Core 1 FSoE handler within one bus cycle (µs). */
#define FSOE_IPC_REPLY_TIMEOUT_US     (500U)
#endif

/**
 * One FSoE cycle: master RxPDO wire → Core 1 manager → master TxPDO wire.
 *
 * \param rx_wire     FSOE RxPDO bytes from APPL_OutputMapping (0x160F), length FSOE_PDO_RX_BYTES.
 * \param tx_wire     Out: FSOE TxPDO bytes for APPL_InputMapping (0x1A0F), length FSOE_PDO_TX_BYTES.
 * \param rtt_us_out  Optional round-trip time in µs (NULL to skip).
 * \return SystemP_SUCCESS when reply seq matches and arrived within timeout.
 */
int32_t fsoe_ipc_master_exchange(const uint8_t rx_wire[FSOE_PDO_RX_BYTES],
                                 uint8_t tx_wire[FSOE_PDO_TX_BYTES],
                                 uint32_t *rtt_us_out);

#endif /* FSOE_IPC_MASTER_H */
