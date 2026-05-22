/*
 * fsoe_pdo.c — pack/unpack FSoE PDO wire images (little-endian).
 */

#include <string.h>

#include "fsoe_pdo.h"

void fsoe_pdo_rx_wire_decode(const uint8_t wire[FSOE_PDO_RX_BYTES], fsoe_pdo_rx_t *out)
{
    out->fsoe_command    = wire[0];
    out->safe_outputs    = wire[1];
    (void)memcpy(&out->connection_id, &wire[2], sizeof(out->connection_id));
    (void)memcpy(&out->fsoe_crc, &wire[4], sizeof(out->fsoe_crc));
}

void fsoe_pdo_rx_wire_encode(const fsoe_pdo_rx_t *in, uint8_t wire[FSOE_PDO_RX_BYTES])
{
    wire[0] = in->fsoe_command;
    wire[1] = in->safe_outputs;
    (void)memcpy(&wire[2], &in->connection_id, sizeof(in->connection_id));
    (void)memcpy(&wire[4], &in->fsoe_crc, sizeof(in->fsoe_crc));
}

void fsoe_pdo_tx_wire_decode(const uint8_t wire[FSOE_PDO_TX_BYTES], fsoe_pdo_tx_t *out)
{
    out->fsoe_status     = wire[0];
    out->safe_inputs     = wire[1];
    (void)memcpy(&out->connection_id, &wire[2], sizeof(out->connection_id));
    (void)memcpy(&out->fsoe_crc, &wire[4], sizeof(out->fsoe_crc));
}

void fsoe_pdo_tx_wire_encode(const fsoe_pdo_tx_t *in, uint8_t wire[FSOE_PDO_TX_BYTES])
{
    wire[0] = in->fsoe_status;
    wire[1] = in->safe_inputs;
    (void)memcpy(&wire[2], &in->connection_id, sizeof(in->connection_id));
    (void)memcpy(&wire[4], &in->fsoe_crc, sizeof(in->fsoe_crc));
}
