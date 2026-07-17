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
     * TODO: call the real FSoE slave stack. Stub: acknowledge + loop back payload words.
     */
    (void)memset(tx, 0, sizeof(*tx));

    tx->word0 = 0x0001U;
    tx->word1 = (uint16_t)(rx->word1 ^ 0xFFFFU);
    tx->word2 = (uint16_t)(rx->word2 ^ 0xFFFFU);
    tx->word3 = rx->word3;
    tx->word4 = rx->word4;
    tx->word5 = rx->word5;
}
