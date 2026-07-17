/*
 * fsoe_pdo.c — pack/unpack FSoE RAW PDO wire images (little-endian Word0..Word5).
 */

#include <string.h>

#include "fsoe_pdo.h"

void fsoe_pdo_rx_wire_decode(const uint8_t wire[FSOE_PDO_RX_BYTES], fsoe_pdo_rx_t *out)
{
    (void)memcpy(out, wire, FSOE_PDO_RX_BYTES);
}

void fsoe_pdo_rx_wire_encode(const fsoe_pdo_rx_t *in, uint8_t wire[FSOE_PDO_RX_BYTES])
{
    (void)memcpy(wire, in, FSOE_PDO_RX_BYTES);
}

void fsoe_pdo_tx_wire_decode(const uint8_t wire[FSOE_PDO_TX_BYTES], fsoe_pdo_tx_t *out)
{
    (void)memcpy(out, wire, FSOE_PDO_TX_BYTES);
}

void fsoe_pdo_tx_wire_encode(const fsoe_pdo_tx_t *in, uint8_t wire[FSOE_PDO_TX_BYTES])
{
    (void)memcpy(wire, in, FSOE_PDO_TX_BYTES);
}
