/*
 * fsoe_pdo.h — FSoE process-data layout shared by Core 0 (EtherCAT PDO) and Core 1 (manager).
 *
 * Wire format matches SSC PDO mapping 0x160F / 0x1A0F (Com + Data objects):
 *   Rx: 0x70F0 (Command, CRC_0, CRC_1, ConnectioID) + 0x70F1 (Safety_Data1/2) = 11 bytes
 *   Tx: 0x60F0 + 0x60F1 = 11 bytes, same layout
 */

#ifndef FSOE_PDO_H
#define FSOE_PDO_H

#include <stdint.h>

/** Byte length of FSOE RxPDO (master → slave), mapping 0x160F. */
#define FSOE_PDO_RX_BYTES     (11U)

/** Byte length of FSOE TxPDO (slave → master), mapping 0x1A0F. */
#define FSOE_PDO_TX_BYTES     (11U)

#define FSOE_IPC_RX_OFFSET    (0U)
#define FSOE_IPC_TX_OFFSET    (0U)

/** Master → slave (0x70F0 + 0x70F1 wire order in 0x160F). */
typedef struct
{
    uint8_t  command;
    uint16_t crc_0;
    uint16_t crc_1;
    uint16_t connection_id;
    uint16_t safety_data1;
    uint16_t safety_data2;
} fsoe_pdo_rx_t;

/** Slave → master (0x60F0 + 0x60F1 wire order in 0x1A0F). */
typedef struct
{
    uint8_t  command;
    uint16_t crc_0;
    uint16_t crc_1;
    uint16_t connection_id;
    uint16_t safety_data1;
    uint16_t safety_data2;
} fsoe_pdo_tx_t;

void fsoe_pdo_rx_wire_decode(const uint8_t wire[FSOE_PDO_RX_BYTES], fsoe_pdo_rx_t *out);
void fsoe_pdo_rx_wire_encode(const fsoe_pdo_rx_t *in, uint8_t wire[FSOE_PDO_RX_BYTES]);

void fsoe_pdo_tx_wire_decode(const uint8_t wire[FSOE_PDO_TX_BYTES], fsoe_pdo_tx_t *out);
void fsoe_pdo_tx_wire_encode(const fsoe_pdo_tx_t *in, uint8_t wire[FSOE_PDO_TX_BYTES]);

#endif /* FSOE_PDO_H */
