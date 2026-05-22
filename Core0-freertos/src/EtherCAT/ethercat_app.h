/*
 *  Copyright (C) 2021-2025 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ETHERCAT_APP_H_
#define ETHERCAT_APP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** PDO byte length for 0x1600 / 0x1A00 (0x7000..0x7003 / 0x6000..0x6003), 232 bits. */
#define ETHERCAT_APP_PDO_IMAGE_BYTES      (29U)

/** PDO byte length for 0x1610 / 0x1A10 (0x7100 / 0x6100 FSoE), 48 bits. */
#define ETHERCAT_APP_FSOE_PDO_BYTES       (6U)

/** Total RxPDO (SM2) / TxPDO (SM3) process data per ESI (29 + 6). */
#define ETHERCAT_APP_PD_OUTPUT_BYTES      (ETHERCAT_APP_PDO_IMAGE_BYTES + ETHERCAT_APP_FSOE_PDO_BYTES)
#define ETHERCAT_APP_PD_INPUT_BYTES       (ETHERCAT_APP_PDO_IMAGE_BYTES + ETHERCAT_APP_FSOE_PDO_BYTES)

void manage_pdo_rx(uint8_t *pdo_rx);
void manage_pdo_tx(uint8_t *pdo_tx);
void manage_pdo_fsoe_rx(uint8_t *pdo_rx);
void manage_pdo_fsoe_tx(uint8_t *pdo_tx);

#ifdef __cplusplus
}
#endif

#endif /* ETHERCAT_APP_H_ */
