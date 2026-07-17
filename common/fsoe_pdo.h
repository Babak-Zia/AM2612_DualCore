/*
 * fsoe_pdo.h — FSoE process-data layout shared by Core 0 (EtherCAT PDO) and Core 1 (manager).
 *
 * Wire format matches SSC PDO mapping 0x160F / 0x1A0F (FSOE_*_RAW):
 *   Rx: 0x70F0 Word0..Word5 = 12 bytes
 *   Tx: 0x60F0 Word0..Word5 = 12 bytes
 */

#ifndef FSOE_PDO_H
#define FSOE_PDO_H

#include <stdint.h>

/** Number of UINT16 words in FSOE RAW PDO objects 0x70F0 / 0x60F0. */
#define FSOE_PDO_WORD_COUNT   (6U)

/** Byte length of FSOE RxPDO (master → slave), mapping 0x160F. */
#define FSOE_PDO_RX_BYTES     (12U)

/** Byte length of FSOE TxPDO (slave → master), mapping 0x1A0F. */
#define FSOE_PDO_TX_BYTES     (12U)

#define FSOE_IPC_RX_OFFSET    (0U)
#define FSOE_IPC_TX_OFFSET    (0U)

/** Master → slave (0x70F0 Word0..Word5 in 0x160F). */
typedef struct
{
    uint16_t word0;
    uint16_t word1;
    uint16_t word2;
    uint16_t word3;
    uint16_t word4;
    uint16_t word5;
} fsoe_pdo_rx_t;

/** Slave → master (0x60F0 Word0..Word5 in 0x1A0F). */
typedef struct
{
    uint16_t word0;
    uint16_t word1;
    uint16_t word2;
    uint16_t word3;
    uint16_t word4;
    uint16_t word5;
} fsoe_pdo_tx_t;

void fsoe_pdo_rx_wire_decode(const uint8_t wire[FSOE_PDO_RX_BYTES], fsoe_pdo_rx_t *out);
void fsoe_pdo_rx_wire_encode(const fsoe_pdo_rx_t *in, uint8_t wire[FSOE_PDO_RX_BYTES]);

void fsoe_pdo_tx_wire_decode(const uint8_t wire[FSOE_PDO_TX_BYTES], fsoe_pdo_tx_t *out);
void fsoe_pdo_tx_wire_encode(const fsoe_pdo_tx_t *in, uint8_t wire[FSOE_PDO_TX_BYTES]);

#endif /* FSOE_PDO_H */
