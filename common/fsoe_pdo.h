/*
 * fsoe_pdo.h — FSoE process-data layout shared by Core 0 (EtherCAT PDO) and Core 1 (manager).
 *
 * Wire format matches SSC PDO mapping 0x1610 (FSOE_Rx / 0x7100) and 0x1A10 (FSOE_Tx / 0x6100):
 * six bytes each, little-endian multi-byte fields.
 */

#ifndef FSOE_PDO_H
#define FSOE_PDO_H

#include <stdint.h>

/** Byte length of FSOE RxPDO (master → slave), object 0x7100 mapping. */
#define FSOE_PDO_RX_BYTES     (6U)

/** Byte length of FSOE TxPDO (slave → master), object 0x6100 mapping. */
#define FSOE_PDO_TX_BYTES     (6U)

/** Offset of FSoE frames inside ipc_channel req_buf / resp_buf. */
#define FSOE_IPC_RX_OFFSET    (0U)
#define FSOE_IPC_TX_OFFSET    (0U)

/** Master → slave (EtherCAT output / slave RxPDO). */
typedef struct
{
    uint8_t  fsoe_command;
    uint8_t  safe_outputs;
    uint16_t connection_id;
    uint16_t fsoe_crc;
} fsoe_pdo_rx_t;

/** Slave → master (EtherCAT input / slave TxPDO). */
typedef struct
{
    uint8_t  fsoe_status;
    uint8_t  safe_inputs;
    uint16_t connection_id;
    uint16_t fsoe_crc;
} fsoe_pdo_tx_t;

void fsoe_pdo_rx_wire_decode(const uint8_t wire[FSOE_PDO_RX_BYTES], fsoe_pdo_rx_t *out);
void fsoe_pdo_rx_wire_encode(const fsoe_pdo_rx_t *in, uint8_t wire[FSOE_PDO_RX_BYTES]);

void fsoe_pdo_tx_wire_decode(const uint8_t wire[FSOE_PDO_TX_BYTES], fsoe_pdo_tx_t *out);
void fsoe_pdo_tx_wire_encode(const fsoe_pdo_tx_t *in, uint8_t wire[FSOE_PDO_TX_BYTES]);

#endif /* FSOE_PDO_H */
