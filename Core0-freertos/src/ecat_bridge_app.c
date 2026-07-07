/*
 * ecat_bridge_app.c — Core 0 EtherCAT bridge (FreeRTOS).
 *
 * Responsibilities: board init, ipc_channel master bring-up, optional self-test,
 * start EtherCAT TaskP stack. Cyclic FSoE uses fsoe_ipc_master_exchange() from
 * ethercat_app.c (manage_pdo_fsoe_rx).
 */

#include <stdint.h>
#include <stdlib.h>

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/ClockP.h>
#include <FreeRTOS.h>
#include <task.h>

#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#include "ipc_channel.h"
#include "fsoe_pdo.h"
#include "fsoe_ipc_master.h"
#include "ecat_bridge_app.h"
#include "tiescutils.h"

/* -------------------------------------------------------------------------- */
/* Board + IPC bring-up                                                       */
/* -------------------------------------------------------------------------- */

static void ecat_bridge_board_init(void)
{
    Drivers_open();
#if (defined(CONFIG_OSPI_NUM_INSTANCES) && (CONFIG_OSPI_NUM_INSTANCES > 0))
    board_flash_reset(gOspiHandle[CONFIG_OSPI0]);
#endif
    Board_driversOpen();
}

static void fsoe_ipc_bringup(void)
{
    int32_t status;

    DebugP_log("[ECAT] FSoE IPC: PDO %u+%u B, reply timeout %u us\r\n",
               (unsigned)FSOE_PDO_RX_BYTES,
               (unsigned)FSOE_PDO_TX_BYTES,
               (unsigned)FSOE_IPC_REPLY_TIMEOUT_US);

    status = ipc_channel_master_init();
    DebugP_assert(status == SystemP_SUCCESS);
    DebugP_log("[ECAT] Core1 FSoE worker ready\r\n");
}

/* -------------------------------------------------------------------------- */
/* Optional bring-up self-test (exercises fsoe_ipc_master + fsoe_manager)     */
/* -------------------------------------------------------------------------- */

#if (DUALCORE_IPC_SELF_TEST != 0)

typedef struct
{
    uint32_t passed;
    uint32_t failed;
    uint32_t timeouts;
    uint32_t rtt_min;
    uint32_t rtt_max;
    uint32_t rtt_sum;
    uint32_t rtt_n;
} FsoeIpcSelfTestStats;

static void fsoe_ipc_self_test_seed_rand(void)
{
    srand((unsigned int)(uint32_t)ClockP_getTimeUsec());
}

static void fsoe_ipc_self_test_make_rx(uint8_t rx_wire[FSOE_PDO_RX_BYTES])
{
    fsoe_pdo_rx_t rx;

    rx.command        = (uint8_t)rand();
    rx.safety_data1   = (uint16_t)((uint32_t)rand() & 0xFFFFU);
    rx.safety_data2   = (uint16_t)((uint32_t)rand() & 0xFFFFU);
    rx.connection_id  = (uint16_t)((uint32_t)rand() & 0xFFFFU);
    rx.crc_0          = (uint16_t)((uint32_t)rand() & 0xFFFFU);
    rx.crc_1          = (uint16_t)((uint32_t)rand() & 0xFFFFU);
    fsoe_pdo_rx_wire_encode(&rx, rx_wire);
}

static uint32_t fsoe_ipc_self_test_verify(const uint8_t rx_wire[FSOE_PDO_RX_BYTES],
                                          const uint8_t tx_wire[FSOE_PDO_TX_BYTES])
{
    fsoe_pdo_rx_t rx;
    fsoe_pdo_tx_t tx;
    fsoe_pdo_tx_t expected;

    fsoe_pdo_rx_wire_decode(rx_wire, &rx);
    fsoe_pdo_tx_wire_decode(tx_wire, &tx);

    expected.command       = 0x01U;
    expected.safety_data1  = rx.safety_data1;
    expected.safety_data2  = rx.safety_data2;
    expected.connection_id = rx.connection_id;
    expected.crc_0         = (uint16_t)(rx.crc_0 ^ 0xFFFFU);
    expected.crc_1         = (uint16_t)(rx.crc_1 ^ 0xFFFFU);

    if ((tx.command != expected.command) ||
        (tx.safety_data1 != expected.safety_data1) ||
        (tx.safety_data2 != expected.safety_data2) ||
        (tx.connection_id != expected.connection_id) ||
        (tx.crc_0 != expected.crc_0) ||
        (tx.crc_1 != expected.crc_1))
    {
        return 1U;
    }

    return 0U;
}

static void fsoe_ipc_self_test_one(uint32_t iter, FsoeIpcSelfTestStats *st)
{
    uint8_t  rx_wire[FSOE_PDO_RX_BYTES];
    uint8_t  tx_wire[FSOE_PDO_TX_BYTES];
    uint32_t rtt_us;
    int32_t  status;

    fsoe_ipc_self_test_make_rx(rx_wire);

    status = fsoe_ipc_master_exchange(rx_wire, tx_wire, &rtt_us);

    if (status != SystemP_SUCCESS)
    {
        st->timeouts++;
        DebugP_log("[ECAT] self-test iter=%4u  TIMEOUT/SEQ  RTT=%u us\r\n",
                   (unsigned)iter, (unsigned)rtt_us);
        return;
    }

    if (fsoe_ipc_self_test_verify(rx_wire, tx_wire) == 0U)
    {
        st->passed++;
        if (rtt_us < st->rtt_min)
        {
            st->rtt_min = rtt_us;
        }
        if (rtt_us > st->rtt_max)
        {
            st->rtt_max = rtt_us;
        }
        st->rtt_sum += rtt_us;
        st->rtt_n++;
        DebugP_log("[ECAT] self-test iter=%4u  PASS  RTT=%u us\r\n",
                   (unsigned)iter, (unsigned)rtt_us);
    }
    else
    {
        st->failed++;
        DebugP_log("[ECAT] self-test iter=%4u  FAIL  FSoE PDO  RTT=%u us\r\n",
                   (unsigned)iter, (unsigned)rtt_us);
    }
}

static void fsoe_ipc_self_test_print_summary(uint32_t iterations,
                                             const FsoeIpcSelfTestStats *st)
{
    DebugP_log("[ECAT] === FSoE IPC self-test ===\r\n");
    DebugP_log("[ECAT]   iterations : %u\r\n", (unsigned)iterations);
    DebugP_log("[ECAT]   passed     : %u\r\n", (unsigned)st->passed);
    DebugP_log("[ECAT]   failed     : %u\r\n", (unsigned)st->failed);
    DebugP_log("[ECAT]   timeouts   : %u\r\n", (unsigned)st->timeouts);
    if (st->rtt_n > 0U)
    {
        DebugP_log("[ECAT]   RTT min/avg/max us : %u / %u / %u\r\n",
                   (unsigned)st->rtt_min,
                   (unsigned)(st->rtt_sum / st->rtt_n),
                   (unsigned)st->rtt_max);
    }
}

static void fsoe_ipc_self_test_run(void)
{
    FsoeIpcSelfTestStats st = { .rtt_min = 0xFFFFFFFFU };
    uint32_t             iter = 0U;

    fsoe_ipc_self_test_seed_rand();
    DebugP_log("[ECAT] FSoE IPC self-test: %u iters, %u ms period\r\n",
               (unsigned)DUALCORE_IPC_SELF_TEST_ITERS,
               (unsigned)DUALCORE_IPC_SELF_TEST_PERIOD_MS);

    while (iter < DUALCORE_IPC_SELF_TEST_ITERS)
    {
        fsoe_ipc_self_test_one(iter, &st);
        iter++;
        ClockP_usleep(DUALCORE_IPC_SELF_TEST_PERIOD_MS * 1000U);
    }

    fsoe_ipc_self_test_print_summary(iter, &st);
}

#endif /* DUALCORE_IPC_SELF_TEST */

/* -------------------------------------------------------------------------- */
/* Task entry                                                                 */
/* -------------------------------------------------------------------------- */

void ecat_bridge_task(void *args)
{
    (void)args;

    ecat_bridge_board_init();
    fsoe_ipc_bringup();

#if (DUALCORE_IPC_SELF_TEST != 0)
    fsoe_ipc_self_test_run();
#endif

    DebugP_log("[ECAT] starting EtherCAT stack (TaskP MainLoop)...\r\n");
    ethercat_subdevice_start();

    /*
     * EtherCAT runs in TaskP task1 (and PDI/LED/SYNC tasks from task1_init).
     * This FreeRTOS task exits; main_core0 deletes it (same pattern as Babak).
     */
}
