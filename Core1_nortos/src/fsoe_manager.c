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
     * TODO: call the real FSoE slave stack. Stub: loop back safe data and placeholder CRC.
     */
    (void)memset(tx, 0, sizeof(*tx));

    tx->command       = 0x01U;
    tx->safety_data1  = rx->safety_data1;
    tx->safety_data2  = rx->safety_data2;
    tx->connection_id = rx->connection_id;
    tx->crc_0         = 1234;//(uint16_t)(rx->crc_0 ^ 0x1234U);
    tx->crc_1         = 5678;//(uint16_t)(rx->crc_1 ^ 0x5678U);
}
