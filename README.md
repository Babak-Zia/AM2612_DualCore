# AM2612 Dual-Core — EtherCAT Bridge + FSoE Worker

Dual-core firmware for the **TI AM2612**: Core 0 (FreeRTOS) is the **EtherCAT bridge**; Core 1 (NoRTOS) is the **FSoE safety worker**. Inter-core traffic uses **`common/ipc_channel`**: **6-byte FSoE PDO** in shared SRAM, synchronized by **`req_seq` / `resp_seq`** (poll only, no mailbox IRQ). An optional startup **IPC self-test** in `ecat_bridge_app.c` exercises the same path before EtherCAT runs.

---

## Project layout

```
AM2612_DualCore/
├── AM2612_DualCore.theia-workspace      Theia / CCS multi-project workspace
├── .gitignore                           Ignores Debug/, *.o, *.out, syscfg/, etc.
├── README.md                            this file
│
├── common/                              Shared transport (linked into both core projects)
│   ├── ipc_channel.h / ipc_channel.c    poll-only `gIpcCh` (req/resp seq + buffers)
│   ├── fsoe_pdo.h / fsoe_pdo.c          6-byte FSoE PDO wire codec
│
├── Core0-freertos/                      Core 0 (R5F-0) — EtherCAT bridge, FreeRTOS
│   ├── src/
│   │   ├── main_core0.c                 System/Board init + `ecat_main` task
│   │   ├── ecat_bridge_app.h / .c       board, IPC, optional self-test, `ethercat_subdevice_start()`
│   │   └── EtherCAT/                    Babak SSC stack (TaskP `task1` / MainLoop)
│   ├── example.syscfg                   SysConfig (drivers, MPU, memory, IPC, debug log)
│   ├── .project / .cproject / .ccsproject   CCS project descriptors
│   ├── targetConfigs/                   debug probe config (XDS-class)
│   └── Debug/                           (auto-generated build output, gitignored)
│
├── Core1_nortos/                        Core 1 (R5F-1) — FSoE worker, NoRTOS
│   ├── src/
│   │   ├── main_core1.c                 entry point → `fsoe_worker_main()`
│   │   ├── fsoe_worker_app.h / .c       poll loop + `fsoe_manager_process()` swap-point
│   │   └── fsoe_manager.c / .h          FSoE handler stub (replace with product stack)
│   ├── example.syscfg                   SysConfig for Core 1
│   └── (mirrors Core0-freertos/ structure)
│
└── System_DualCore/                     Meta project - combined boot image
    ├── system.xml                       lists the two .appimage participants
    └── makefile_system_ccs_bootimage_gen
```

`common/ipc_channel.c` is **linked** into both CCS projects (see `.project` → `linkedResources`).

Both cores use **`gIpcCh.req_buf[0..5]`** and **`gIpcCh.resp_buf[0..5]`** for FSoE (`common/fsoe_pdo.h`). Only **one** synchronous exchange may be in flight per bus cycle.

---

## Core 0 — where FSoE is sent to / received from Core 1

EtherCAT stack calls application hooks; FSoE IPC is only in **`manage_pdo_fsoe_*`** and **`fsoe_ipc_master_exchange`**.

```
TwinCAT master (outputs)                    TwinCAT master (inputs)
        │                                            ▲
        ▼                                            │
   ESC SM2 (RxPDO)                              ESC SM3 (TxPDO)
        │                                            │
PDO_OutputMapping()  ──ecatappl.c──►  APPL_OutputMapping()  tiescappl.c
        │                                  case 0x1610:
        │                                    manage_pdo_fsoe_rx(pdo_rx)     ethercat_app.c
        │                                      └─ fsoe_ipc_master_exchange()  fsoe_ipc_master.c
        │                                           ├─ memcpy → gIpcCh.req_buf   (6 B to Core 1)
        │                                           ├─ ipc_channel_master_commit_request()
        │                                           ├─ ipc_channel_master_wait_reply_usec()
        │                                           └─ memcpy ← gIpcCh.resp_buf  (6 B from Core 1)
        │                                      └─ fsoe_od_apply_tx_wire → FSOE_Tx0x6100
        │
PDO_InputMapping()   ◄──ecatappl.c──   APPL_InputMapping()   tiescappl.c
                                           case 0x1A10:
                                             manage_pdo_fsoe_tx(pdo_tx)  ← packs FSOE_Tx0x6100 → ESC
```

| Step | File | What |
|------|------|------|
| **Receive from master (FSoE Rx)** | `tiescappl.c` → `manage_pdo_fsoe_rx()` | 6 B from PDO `0x1610` (master → slave outputs) |
| **Send to Core 1** | `fsoe_ipc_master.c` | `memcpy` into `gIpcCh.req_buf`, bump `req_seq` |
| **Wait for Core 1** | `fsoe_ipc_master.c` | Poll until `gIpcCh.resp_seq == expected_seq` |
| **Receive from Core 1** | `fsoe_ipc_master.c` | `memcpy` from `gIpcCh.resp_buf` into local `tx_wire` |
| **Send to master (FSoE Tx)** | `tiescappl.c` → `manage_pdo_fsoe_tx()` | 6 B to PDO `0x1A10` from object `0x6100` |

Core 1 mirror path: `fsoe_worker_app.c` — poll `req_seq` → read `req_buf` → `fsoe_manager_process()` → write `resp_buf` → set `resp_seq`.

---

## Runtime architecture

```
Core 0 (FreeRTOS)                         Core 1 (NoRTOS)
─────────────────                         ─────────────────
ecat_bridge_task: ipc_channel_master_init
ethercat_subdevice_start → MainLoop
  PDO_OutputMapping → manage_pdo_fsoe_rx    fsoe_worker_main:
    → fsoe_ipc_master_exchange                poll req_seq
  PDO_InputMapping → manage_pdo_fsoe_tx         handle 6 B → resp_buf
                                                set resp_seq
```

---

## IPC design (poll-only)

| Choice | Why |
|---|---|
| **`req_seq` / `resp_seq`** | Core 0 publishes a request; Core 1 publishes a reply — no IpcNotify, no ISR on the data path. |
| **6 B in shared SRAM** | `FSOE_IPC_RX_OFFSET` / `FSOE_IPC_TX_OFFSET` in `fsoe_pdo.h` (both 0). |
| **Non-cacheable `.bss.user_shared_mem`** | Both cores see the same physical bytes; no `CacheP_*` in the loop. |
| **One in-flight transaction** | Do not start a new exchange until the previous `wait_reply` completes. |

Typical round-trip (stub manager): **~10–20 µs** in the PDO path, dominated by memcpy and `fsoe_manager_process()`.

---

## Shared channel layout

```c
typedef struct
{
    volatile uint32_t req_seq;           // Core 0 writes, Core 1 reads
    volatile uint32_t resp_seq;          // Core 1 writes, Core 0 reads
    volatile uint32_t _reserved[14];     // keeps buffers off the seq cache cluster
    uint8_t           req_buf [IPC_BUF_LEN];  // 512 bytes (IPC_BUF_LEN), Core 0 -> Core 1
    uint8_t           resp_buf[IPC_BUF_LEN];  // 512 bytes, Core 1 -> Core 0
} ipc_channel_t;

ipc_channel_t gIpcCh __attribute__((aligned(128),
                                    section(".bss.user_shared_mem")));
```

The linker on both cores maps `.bss.user_shared_mem` to the **same physical SRAM address**, so writes from one core are visible to the other through the section symbol. 128-byte alignment prevents false sharing with the R5F cache-line policy.

### `ipc_channel` API (`common/ipc_channel.h`)

| Function | Core | Role |
|----------|------|------|
| `ipc_channel_master_init` | 0 | No-op placeholder (call after `Drivers_open` in `ecat_bridge_task`). |
| `ipc_channel_master_commit_request` | 0 | Increment `gIpcCh.req_seq` after filling `req_buf`. |
| `ipc_channel_master_wait_reply_usec(expected, timeout)` | 0 | Poll until `resp_seq == expected` or timeout (`FSOE_IPC_REPLY_TIMEOUT_US`). |
| `ipc_channel_worker_init` | 1 | Remember current `req_seq`. |
| `ipc_channel_worker_wait_request` | 1 | Spin until `req_seq` changes. |
| `ipc_channel_worker_send_reply(seq)` | 1 | Write `resp_seq` after filling `resp_buf`. |

---

## Console logging architecture

`DebugP_log` works on **both** cores but the routing and the cost are very different. This matters because every call from Core 0 spends real time on the wire, while every call from Core 1 spends almost none.

```
                Core 1 (NoRTOS)
                ───────────────
                DebugP_log("...")
                    │
                    ▼
                putchar_  ──►  DebugP_shmLogWriterPutChar
                                       │
                                       ▼
                ┌──────────────────────────────────────────┐
                │ gDebugShmLog[]   (.bss.log_shared_mem)   │ ◄── shared SRAM
                │   [0] = R5FSS0_0 ring                    │
                │   [1] = R5FSS0_1 ring  ◄── Core 1 writes here
                └──────────────────────────────────────────┘
                                       │
                                       ▼   (reader task on Core 0)
                Core 0 (FreeRTOS)      │
                ───────────────        ▼
                DebugP_log("...") ──► UART0 ──► your terminal
                                       ▲
                                       │  (Core 1's chars relayed by Core 0)
```

| Core | Path | Per char | A 60-char line | Blocking? |
|---|---|---|---|---|
| **Core 0** | `UART_write` per char, polled | ~87 µs @ 115200 | **~5 ms** | Yes — busy-wait |
| **Core 1** | byte store + ring flush on `\n` | ~10 ns | ~1-3 µs | No |

That ~1500× difference drives a few rules:

- **Inside the `t0..t1` window: never log.** The current code respects this — the printed RTT is real IPC latency, not print latency.
- **At 1 s period: log freely on both cores.**
- **At 1 ms period:**
  - On Core 0, silence the PASS path. A 60-char log line eats 5× the whole period.
  - On Core 1, throttle to e.g. one heartbeat per 1000 transactions. The shared-memory ring is drained at UART speed (~11 bytes/ms at 115200 baud); faster than that and it overflows silently.
- **Don't `DebugP_log` from ISRs on Core 1.** `DebugP_shmLogWriterPutChar` uses a static line buffer with no locking — an ISR mid-line corrupts the next line. Stick to the worker's main loop. The callback `on_master_request` correctly stays log-free.

Two cheap ways to relax the constraint:

- **Raise UART baud** in `example.syscfg` (Core 0 owns UART0). `115200` → `921600` cuts every cost above by 8×; `3000000` cuts it by ~26×.
- **Disable the Core 1 timestamp prelude** with `DebugP_shmLogWriterPreludeDisable()` once at startup on Core 1. Each line is currently prefixed with `[R5FSS1] SSSSS.UUUUUUs : ` (~24 chars); disabling halves the ring traffic.

---

## Tuning knobs

Application knobs are in `Core0-freertos/src/ecat_bridge_app.h`:

| `#define` | Default | Meaning |
|---|---|---|
| `DUALCORE_IPC_SELF_TEST` | `1U` | run bring-up ping-pong before idle (`0` when EtherCAT owns IPC) |
| `DUALCORE_IPC_SELF_TEST_PERIOD_MS` | `100U` | delay between self-test iterations |
| `DUALCORE_IPC_SELF_TEST_ITERS` | `50U` | self-test transaction count |
| `FSOE_IPC_REPLY_TIMEOUT_US` | `500U` | Plan A reply budget (`fsoe_ipc_master_exchange`) |

Transport constants (`IPC_BUF_LEN`, `IPC_CLIENT_ID`, core IDs, `IPC_MS_TO_TICKS`) are in `common/ipc_channel.h` — change there when both sides must agree.

---

## Going faster than 1 ms

The default DPL configuration sets the tick period to **1 ms**:

```
Core0-freertos/Debug/syscfg/ti_dpl_config.c:
    .usecPerTick = 1000,
```

This sets the minimum non-zero FreeRTOS delay to one tick = 1 ms. To run the loop faster:

### 1. Reduce the period

```c
#define DUALCORE_IPC_SELF_TEST_PERIOD_MS  (1U)
```

### 2. Silence per-iteration logging

On Core 0 every `DebugP_log` call **blocks the calling task** on the UART, byte-by-byte — about **5 ms for a 60-character line** at 115200 baud. At a 1 ms period the UART is 5× too slow, and the loop overruns. See [Console logging architecture](#console-logging-architecture) for the full breakdown.

At `DUALCORE_IPC_SELF_TEST_PERIOD_MS = 1`, log only summary and failures on Core 0:

```c
if (bad == 0U) {
    passed++;
    // no DebugP_log on the PASS path
} else {
    failed++;
    DebugP_log("[Core0] iter=%4u  FAIL  seq=%u  %u/%u bytes wrong  RTT=%u us\r\n", ...);
}
```

On Core 1 each `DebugP_log` is ~1500× cheaper (writes to a shared-memory ring, no UART), but Core 0 still has to drain that ring through the UART. Throttle Core 1 too:

```c
if ((seq % 1000U) == 0U) {
    DebugP_log("[Core1] heartbeat seq=%u\r\n", (unsigned)seq);
}
```

Or raise the UART baud in `example.syscfg` to relax both constraints at once.

### 3. To go below 1 ms, change SysConfig

Edit `example.syscfg` in both projects, raise the DPL tick rate (e.g. `usecPerTick = 100` → 10 kHz tick → minimum delay 100 µs). Save, rebuild, and `ClockP_usleep(N_us)` becomes accurate down to 100 µs.

### 4. Performance ceiling

Below ~10 µs period the doorbell round-trip itself becomes the limit. At that point switch from a periodic master loop to a free-running back-to-back pattern (just `goto top` after verify) or a streamed ring buffer; the current ping-pong protocol holds at most one transaction in flight.

---

## FSoE modules

| Module | Core | Role |
|--------|------|------|
| `ethercat_app.c` | 0 | `manage_pdo_fsoe_rx` / `tx` — PDO ↔ IPC ↔ CoE `0x7100` / `0x6100` |
| `fsoe_ipc_master.c` | 0 | One exchange: `req_buf` → wait → `resp_buf` |
| `fsoe_worker_app.c` | 1 | Poll loop servicing `gIpcCh` |
| `fsoe_manager.c` | 1 | **Swap-point** — product FSoE stack |
| `common/fsoe_pdo.c` | both | 6-byte wire encode/decode |

**Swap-point:** replace `fsoe_manager_process()`; keep handler time within `FSOE_IPC_REPLY_TIMEOUT_US` (500 µs default in `fsoe_ipc_master.h`).

---

## Cache notes

`.bss.user_shared_mem` is **non-cacheable** under the default SysConfig MPU map on AM2612, so the producer/consumer paths in the current code do **not** call `CacheP_wb` or `CacheP_inv`. This is the simplest correct setup and what the upstream TI `ipc_spinlock_sharedmem` example also relies on.

If you ever relocate the channel to a cacheable region (for higher SRAM bandwidth on very large buffers), add cache ops at producer/consumer boundaries in the core `src/` application files (or wrap calls around `ipc_channel_*`):

```c
// After filling req_buf / before reading resp_buf on Core 0 (and the inverse on Core 1):
CacheP_wb / CacheP_inv on gIpcCh.req_buf and gIpcCh.resp_buf as appropriate.
```

Forgetting any one of these on cacheable memory produces stale reads that look like random data corruption — the most expensive class of bug to debug. Stay non-cacheable unless profiling explicitly shows a need.

---

## Build and run (Code Composer Studio / Theia)

1. **Open** `AM2612_DualCore.theia-workspace`. The three projects (`Core0-freertos`, `Core1_nortos`, `System_DualCore`) appear in the Project Explorer.
2. **Refresh** each project (right-click → Refresh, F5) so the IDE picks up the current source files.
3. **Clean** each project (Project → Clean) to wipe stale `Debug/` artifacts left over from before the rename.
4. **Build All**. The system project bundles both `.appimage` files into a combined boot image via `makefile_system_ccs_bootimage_gen`.
5. **Launch** the debug configuration in `.theia/launch.json`. Both R5F cores boot; Core 1 enters the poll loop; Core 0 runs EtherCAT (and optional IPC self-test first).

Console output (after both cores rendezvous):

```
[Core1] starting, client=4
[Core0] IPC ping-pong: 1024 B/dir, period 1000 ms, timeout 5000 ms
[Core1] ready, service loop
[Core0] worker ready, starting transactions
[Core0] iter=   0  PASS  seq=1  RTT=42 us
[Core0] iter=   1  PASS  seq=2  RTT=39 us
...
[Core0] === Summary ===
[Core0]   iterations : 50
[Core0]   passed     : 50
[Core0]   failed     : 0
[Core0]   seq miss   : 0
[Core0]   freezes    : 0
[Core0]   RTT min/avg/max us : 38 / 42 / 71
```

Exact numbers depend on R5F clock, UART contention, cache state, and what's happening on the other core. The `t0/t1` timestamps are taken around the doorbell only, so the printed RTT reflects actual IPC latency — not the visible delay on the console (which is dominated by UART transmission of the log lines themselves).

---

## Comparison with other IPC mechanisms on AM2612

| Mechanism | Round-trip latency | Throughput | When to use |
|---|---|---|---|
| **Shared SRAM + req_seq poll** (this project) | **~10-20 us** (FSoE stub) | SRAM bandwidth | Cyclic PDO, dedicated Core 1 worker loop |
| Shared SRAM + IpcNotify doorbell | ~3-7 us wake + handler | Same SRAM | Optional; removed here after notify-loss issues |
| `IpcNotify` payload only (no shared mem) | ~1-3 us | <= 4 bytes per message | Pure signaling / tiny commands |
| **RPMessage** (virtio rings) | ~10-100 us | Lower (copy + protocol) | Heterogeneous (R5F ↔ A53/Linux) or many endpoints |
| Mailbox IP raw register access | ~0.5-2 us | <= 8 x 32-bit FIFO | Bare-metal speed; little gain over IpcNotify in practice |

For this FSoE worker, **poll-only shared SRAM** is the simplest reliable pattern. RPMessage or doorbells add complexity without helping the 6-byte cyclic path.

---

## Roadmap

- [ ] Set `DUALCORE_IPC_SELF_TEST` to `0` when EtherCAT cyclic code owns IPC.
- [x] PDO path on Core 0: `manage_pdo_fsoe_rx()` → `fsoe_ipc_master_exchange()`.
- [ ] Replace `fsoe_manager_process()` in `Core1_nortos/src/fsoe_manager.c` with the product FSoE stack.
- [ ] If you need < 1 ms periods, lower `usecPerTick` in `example.syscfg` (both cores) and rebuild.
- [ ] If buffers grow past ~4 KB, consider moving the channel to cacheable memory and adding the four `CacheP_*` calls noted above.

---

## License

The transport files (`common/ipc_channel.c`) and app files carry the TI BSD-3 style headers where applicable. The modifications in this project follow the same license unless you decide otherwise for your own derived work.
