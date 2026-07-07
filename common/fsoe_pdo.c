/*
 * fsoe_pdo.c — pack/unpack FSoE PDO wire images (little-endian, 0x160F / 0x1A0F order).
 */

#include <string.h>

#include "fsoe_pdo.h"

void fsoe_pdo_rx_wire_decode(const uint8_t wire[FSOE_PDO_RX_BYTES], fsoe_pdo_rx_t *out)
{
    uint16_t o = 0U;

    out->command = wire[o++];
    (void)memcpy(&out->crc_0, &wire[o], sizeof(out->crc_0));
    o = (uint16_t)(o + (uint16_t)sizeof(out->crc_0));
    (void)memcpy(&out->crc_1, &wire[o], sizeof(out->crc_1));
    o = (uint16_t)(o + (uint16_t)sizeof(out->crc_1));
    (void)memcpy(&out->connection_id, &wire[o], sizeof(out->connection_id));
    o = (uint16_t)(o + (uint16_t)sizeof(out->connection_id));
    (void)memcpy(&out->safety_data1, &wire[o], sizeof(out->safety_data1));
    o = (uint16_t)(o + (uint16_t)sizeof(out->safety_data1));
    (void)memcpy(&out->safety_data2, &wire[o], sizeof(out->safety_data2));
}

void fsoe_pdo_rx_wire_encode(const fsoe_pdo_rx_t *in, uint8_t wire[FSOE_PDO_RX_BYTES])
{
    uint16_t o = 0U;

    wire[o++] = in->command;
    (void)memcpy(&wire[o], &in->crc_0, sizeof(in->crc_0));
    o = (uint16_t)(o + (uint16_t)sizeof(in->crc_0));
    (void)memcpy(&wire[o], &in->crc_1, sizeof(in->crc_1));
    o = (uint16_t)(o + (uint16_t)sizeof(in->crc_1));
    (void)memcpy(&wire[o], &in->connection_id, sizeof(in->connection_id));
    o = (uint16_t)(o + (uint16_t)sizeof(in->connection_id));
    (void)memcpy(&wire[o], &in->safety_data1, sizeof(in->safety_data1));
    o = (uint16_t)(o + (uint16_t)sizeof(in->safety_data1));
    (void)memcpy(&wire[o], &in->safety_data2, sizeof(in->safety_data2));
}

void fsoe_pdo_tx_wire_decode(const uint8_t wire[FSOE_PDO_TX_BYTES], fsoe_pdo_tx_t *out)
{
    uint16_t o = 0U;

    out->command = wire[o++];
    (void)memcpy(&out->crc_0, &wire[o], sizeof(out->crc_0));
    o = (uint16_t)(o + (uint16_t)sizeof(out->crc_0));
    (void)memcpy(&out->crc_1, &wire[o], sizeof(out->crc_1));
    o = (uint16_t)(o + (uint16_t)sizeof(out->crc_1));
    (void)memcpy(&out->connection_id, &wire[o], sizeof(out->connection_id));
    o = (uint16_t)(o + (uint16_t)sizeof(out->connection_id));
    (void)memcpy(&out->safety_data1, &wire[o], sizeof(out->safety_data1));
    o = (uint16_t)(o + (uint16_t)sizeof(out->safety_data1));
    (void)memcpy(&out->safety_data2, &wire[o], sizeof(out->safety_data2));
}

void fsoe_pdo_tx_wire_encode(const fsoe_pdo_tx_t *in, uint8_t wire[FSOE_PDO_TX_BYTES])
{
    uint16_t o = 0U;

    wire[o++] = in->command;
    (void)memcpy(&wire[o], &in->crc_0, sizeof(in->crc_0));
    o = (uint16_t)(o + (uint16_t)sizeof(in->crc_0));
    (void)memcpy(&wire[o], &in->crc_1, sizeof(in->crc_1));
    o = (uint16_t)(o + (uint16_t)sizeof(in->crc_1));
    (void)memcpy(&wire[o], &in->connection_id, sizeof(in->connection_id));
    o = (uint16_t)(o + (uint16_t)sizeof(in->connection_id));
    (void)memcpy(&wire[o], &in->safety_data1, sizeof(in->safety_data1));
    o = (uint16_t)(o + (uint16_t)sizeof(in->safety_data1));
    (void)memcpy(&wire[o], &in->safety_data2, sizeof(in->safety_data2));
}
