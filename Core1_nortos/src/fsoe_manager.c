/*
 * fsoe_manager.c — Core 1 FSoE handler (stub until product stack is integrated).
 */

#include <string.h>

#include "fsoe_manager.h"

void fsoe_manager_init(void)
{
    /* Connection tables, CRC tables, safe-application state, etc. */
}

void fsoe_manager_process(const fsoe_pdo_rx_t *rx, fsoe_pdo_tx_t *tx)
{
    /*
     * TODO: call the real FSoE slave stack (frame decode, connection state machine,
     * safeInputs/safeOutputs, CRC). This stub only preserves connectionId and
     * passes safe_outputs → safe_inputs for bring-up.
     */
    (void)memset(tx, 0, sizeof(*tx));

    tx->fsoe_status     = 0x01U;
    tx->safe_inputs     = rx->safe_outputs;
    tx->connection_id   = rx->connection_id;
    tx->fsoe_crc        = (uint16_t)(rx->fsoe_crc ^ 0xFFFFU);
}
