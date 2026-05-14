# AM2612 Dual-Core IPC Ping-Pong

Real-time inter-core data exchange demo for the **TI AM2612** SoC running two ARM Cortex-R5F cores. Core 0 (FreeRTOS) acts as a master/scheduler that periodically pushes a buffer to Core 1 (NoRTOS), which processes it and returns the result. The demo measures round-trip latency and validates data integrity with per-byte verification and sequence-number-based drop/freeze detection.

The transport is the lowest-latency IPC available on this SoC: a shared SRAM region for the data plus the hardware **Mailbox** peripheral (wrapped by `IpcNotify`) for doorbell signaling. There is **no** RPMessage / virtio overhead in the critical path.

---

## Project layout

```
AM2612_DualCore/
├── AM2612_DualCore.theia-workspace      Theia / CCS multi-project workspace
├── .gitignore                           Ignores Debug/, *.o, *.out, syscfg/, etc.
├── README.md                            this file
│
├── Core0-freertos/                      Core 0 (R5F-0) - master, FreeRTOS
│   ├── main_core0.c                     entry point + FreeRTOS task creation
│   ├── ipc_pingpong.c                   IPC channel + master/worker logic
│   ├── example.syscfg                   SysConfig (drivers, MPU, FreeRTOS)
│   ├── .project / .cproject / .ccsproject   CCS project descriptors
│   ├── targetConfigs/                   debug probe config (XDS-class)
│   └── Debug/                           (auto-generated build output, gitignored)
│
├── Core1_nortos/                        Core 1 (R5F-1) - worker, NoRTOS
│   ├── main_core1.c                     entry point - calls ipc_worker_run()
│   ├── ipc_pingpong.c                   identical copy of the shared logic
│   ├── example.syscfg                   SysConfig for Core 1
│   └── (mirrors Core0-freertos/ structure)
│
└── System_DualCore/                     Meta project - combined boot image
    ├── system.xml                       lists the two .appimage participants
    └── makefile_system_ccs_bootimage_gen
```

The `ipc_pingpong.c` file is byte-identical in both core projects. Each project's `main_*` calls only the half that applies to that core; the other half is compiled but unreferenced.

---

## Runtime architecture

```
Core 0 (FreeRTOS, R5F-0)                       Core 1 (NoRTOS, R5F-1)
─────────────────────────                      ─────────────────────────
main()                                         main()
 ├ System_init / Board_init                     ├ System_init / Board_init
 └ xTaskCreateStatic(freertos_main)             └ ipc_worker_run()
   └ freertos_main()                              ├ Drivers_open
     └ ipc_master_run()                           ├ Board_driversOpen
       ├ Drivers_open                             ├ IpcNotify_registerClient(
       ├ Board_driversOpen                        │   4, on_master_request)
       ├ SemaphoreP_constructBinary               ├ IpcNotify_syncAll() ◄────┐
       ├ IpcNotify_registerClient(                │                          │
       │   4, on_worker_reply)                    │                          │
       ├ IpcNotify_syncAll() ──────────────────► ─┤  (cores rendezvous)      │
       │                                                                     │
       └ for iter = 0 .. N-1:                    └ for (;;):                 │
         ┌─ fill req_buf with rand()               ┌─ WFI until ISR sets    │
         │   gIpcCh.req_seq = expected_seq        │   gWorkerNewReqSeq      │
         │   t0 = ClockP_getTimeUsec()            │                         │
         │   IpcNotify_sendMsg(C1,4,seq) ──ISR───►│                         │
         │   SemaphoreP_pend(timeout=5xT)         │   process_buffer(       │
         │     blocks task at us precision        │     req_buf, resp_buf)  │
         │                                        │   gIpcCh.resp_seq = seq │
         │  ◄──── ISR ── IpcNotify_sendMsg(C0,4,seq) ◄┘                     │
         │   t1 = ClockP_getTimeUsec()                                       │
         │   verify resp_buf[i] == req_buf[i] + 1                            │
         │   print PASS/FAIL + RTT us                                        │
         └─ ClockP_usleep(IPC_PERIOD_MS * 1000)                              │
                                                                              │
       Final summary: pass / fail / seq-miss / freezes / RTT min,avg,max ◄───┘
```

### Why this is fast

| Choice | Effect |
|---|---|
| Shared SRAM, no copy | Buffer is visible to both cores at the same physical address |
| No spinlock | Each control word and each buffer has a single writer; the doorbell **is** the synchronization point |
| Doorbell carries sequence number | Receiver detects drops / stale replies without a second handshake |
| `SemaphoreP` posted from ISR | Master wakes within microseconds, bypassing the 1 ms FreeRTOS tick |
| `WFI` on worker | Lowest-power, lowest-jitter wake on ISR |
| `.bss.user_shared_mem` is non-cacheable (MPU) | Zero `CacheP_*` calls in the hot path |

Expected wake-up RTT (excluding `process_buffer` time): **~3-7 µs** for the doorbell/IRQ/semaphore chain alone. Total RTT also includes the time to actually write/read 1024 bytes of SRAM and run `process_buffer` (currently a 1024-byte add).

---

## Shared channel layout

```c
typedef struct
{
    volatile uint32_t req_seq;           // Core 0 writes, Core 1 reads
    volatile uint32_t resp_seq;          // Core 1 writes, Core 0 reads
    volatile uint32_t _reserved[14];     // keeps buffers off the seq cache cluster
    uint8_t           req_buf [IPC_BUF_LEN];  // 1024 bytes, Core 0 -> Core 1
    uint8_t           resp_buf[IPC_BUF_LEN];  // 1024 bytes, Core 1 -> Core 0
} ipc_channel_t;

ipc_channel_t gIpcCh __attribute__((aligned(128),
                                    section(".bss.user_shared_mem")));
```

The linker on both cores maps `.bss.user_shared_mem` to the **same physical SRAM address**, so writes from one core are visible to the other through the section symbol. 128-byte alignment prevents false sharing with the R5F cache-line policy.

---

## Sequence-number protocol

Every transaction has a monotonically increasing `uint32_t` sequence number, set by the master and echoed back by the worker. The same number is carried on the IpcNotify doorbell **and** written into the channel, which lets the master detect every interesting failure mode with a single equality test:

| Failure mode | Detection | Counter | Visible log |
|---|---|---|---|
| **Drop** (worker missed a request) | `gIpcCh.resp_seq != expected_seq` after wake-up | `mis_seq` | `SEQ MISMATCH sent=N got=M` |
| **Freeze** (worker hung, no reply ever) | `SemaphoreP_pend` returns non-zero after `IPC_RESP_TIMEOUT_MS` | `freezes` | `FREEZE no reply for seq=N within K ms` |
| **Stale reply** (late reply from prior iter) | same as drop — old `seq` fails the equality | `mis_seq` | `SEQ MISMATCH ...` |
| **Data corruption** | per-byte `resp[i] != (uint8_t)(req[i] + 1)` | `failed` | `FAIL seq=N X/1024 bytes wrong` |
| **Success** | all of the above pass | `passed` | `PASS seq=N RTT=us` |

Final summary line prints `min / avg / max` RTT in microseconds across all PASS iterations.

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

All in the top of `ipc_pingpong.c`:

| `#define` | Default | Meaning |
|---|---|---|
| `IPC_BUF_LEN` | `1024U` | bytes per direction (`req_buf` and `resp_buf` each) |
| `IPC_CLIENT_ID` | `4U` | IpcNotify endpoint ID; identical on both cores |
| `IPC_PERIOD_MS` | `1000U` | master loop period (initial 1 s, future target 1 ms) |
| `IPC_RESP_TIMEOUT_MS` | `5 * IPC_PERIOD_MS` | freeze-detect deadline |
| `IPC_TEST_ITERATIONS` | `50U` | total transactions (`0` = run forever) |
| `IPC_CORE_MASTER` | `CSL_CORE_ID_R5FSS0_0` | Core ID macro for R5F-0 |
| `IPC_CORE_WORKER` | `CSL_CORE_ID_R5FSS0_1` | Core ID macro for R5F-1 |
| `IPC_MS_TO_TICKS(ms)` | `(ms)` | valid only while DPL `usecPerTick == 1000` |

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
#define IPC_PERIOD_MS  (1U)
```

### 2. Silence per-iteration logging

On Core 0 every `DebugP_log` call **blocks the calling task** on the UART, byte-by-byte — about **5 ms for a 60-character line** at 115200 baud. At a 1 ms period the UART is 5× too slow, and the loop overruns. See [Console logging architecture](#console-logging-architecture) for the full breakdown.

At `IPC_PERIOD_MS = 1`, log only summary and failures on Core 0:

```c
if (bad == 0U) {
    passed++;
    // no DebugP_log on the PASS path
} else {
    failed++;
    DebugP_log("[Master] iter=%4u  FAIL  seq=%u  %u/%u bytes wrong  RTT=%u us\r\n", ...);
}
```

On Core 1 each `DebugP_log` is ~1500× cheaper (writes to a shared-memory ring, no UART), but Core 0 still has to drain that ring through the UART. Throttle Core 1 too:

```c
if ((seq % 1000U) == 0U) {
    DebugP_log("[Worker] heartbeat seq=%u\r\n", (unsigned)seq);
}
```

Or raise the UART baud in `example.syscfg` to relax both constraints at once.

### 3. To go below 1 ms, change SysConfig

Edit `example.syscfg` in both projects, raise the DPL tick rate (e.g. `usecPerTick = 100` → 10 kHz tick → minimum delay 100 µs). Save, rebuild, and `ClockP_usleep(N_us)` becomes accurate down to 100 µs.

### 4. Performance ceiling

Below ~10 µs period the doorbell round-trip itself becomes the limit. At that point switch from a periodic master loop to a free-running back-to-back pattern (just `goto top` after verify) or a streamed ring buffer; the current ping-pong protocol holds at most one transaction in flight.

---

## Swap-point: the `process_buffer` function

Single function inside `ipc_pingpong.c`, scoped to the worker side:

```c
static void process_buffer(const uint8_t *in, uint8_t *out, uint32_t len)
{
    for (uint32_t i = 0U; i < len; i++)
    {
        out[i] = (uint8_t)(in[i] + 1U);
    }
}
```

**Contract**:
- read `len` bytes from `in`
- write `len` bytes to `out`
- must not modify `in`
- must finish before returning (no async, no further IPC)

Anything that fits that contract — DSP routines, an inference graph, a CRC, an encryption pass — can drop in without touching the protocol, sequence numbering, RTT measurement, or master code. The master's verification step (`resp[i] == req[i] + 1`) must be updated to match whatever `process_buffer` actually computes.

---

## Cache notes

`.bss.user_shared_mem` is **non-cacheable** under the default SysConfig MPU map on AM2612, so the producer/consumer paths in the current code do **not** call `CacheP_wb` or `CacheP_inv`. This is the simplest correct setup and what the upstream TI `ipc_spinlock_sharedmem` example also relies on.

If you ever relocate the channel to a cacheable region (for higher SRAM bandwidth on very large buffers), add cache ops at the points marked `CACHE:` in `ipc_pingpong.c`:

```c
// Core 0 (master), AFTER filling req_buf, BEFORE sending the doorbell:
CacheP_wb (gIpcCh.req_buf,  IPC_BUF_LEN, CacheP_TYPE_ALLD);

// Core 0 (master), AFTER receiving the doorbell, BEFORE reading resp_buf:
CacheP_inv(gIpcCh.resp_buf, IPC_BUF_LEN, CacheP_TYPE_ALLD);

// Core 1 (worker), AFTER receiving the doorbell, BEFORE reading req_buf:
CacheP_inv(gIpcCh.req_buf,  IPC_BUF_LEN, CacheP_TYPE_ALLD);

// Core 1 (worker), AFTER filling resp_buf, BEFORE sending the doorbell:
CacheP_wb (gIpcCh.resp_buf, IPC_BUF_LEN, CacheP_TYPE_ALLD);
```

Forgetting any one of these on cacheable memory produces stale reads that look like random data corruption — the most expensive class of bug to debug. Stay non-cacheable unless profiling explicitly shows a need.

---

## Build and run (Code Composer Studio / Theia)

1. **Open** `AM2612_DualCore.theia-workspace`. The three projects (`Core0-freertos`, `Core1_nortos`, `System_DualCore`) appear in the Project Explorer.
2. **Refresh** each project (right-click → Refresh, F5) so the IDE picks up the current source files.
3. **Clean** each project (Project → Clean) to wipe stale `Debug/` artifacts left over from before the rename.
4. **Build All**. The system project bundles both `.appimage` files into a combined boot image via `makefile_system_ccs_bootimage_gen`.
5. **Launch** the debug configuration in `.theia/launch.json`. Both R5F cores boot, hit `IpcNotify_syncAll`, then start exchanging.

Console output (after both cores rendezvous):

```
[Worker] starting, client=4
[Master] IPC ping-pong: 1024 B/dir, period 1000 ms, timeout 5000 ms
[Worker] ready, entering service loop
[Master] worker is ready, starting transactions
[Master] iter=   0  PASS  seq=1  RTT=42 us
[Master] iter=   1  PASS  seq=2  RTT=39 us
...
[Master] === Summary ===
[Master]   iterations : 50
[Master]   passed     : 50
[Master]   failed     : 0
[Master]   seq miss   : 0
[Master]   freezes    : 0
[Master]   RTT min/avg/max us : 38 / 42 / 71
```

Exact numbers depend on R5F clock, UART contention, cache state, and what's happening on the other core. The `t0/t1` timestamps are taken around the doorbell only, so the printed RTT reflects actual IPC latency — not the visible delay on the console (which is dominated by UART transmission of the log lines themselves).

---

## Comparison with other IPC mechanisms on AM2612

| Mechanism | Round-trip latency | Throughput | When to use |
|---|---|---|---|
| **Shared SRAM + IpcNotify doorbell** (this demo) | **~3-7 us** | SRAM bandwidth (GB/s) | Real-time, custom protocol, fixed peers |
| Shared SRAM + busy-polling flag (no IRQ) | ~50-200 ns | SRAM bandwidth | Hard µs-scale latency, one core can be 100% spinning |
| `IpcNotify` payload only (no shared mem) | ~1-3 us | <= 4 bytes per message | Pure signaling / tiny commands |
| **RPMessage** (virtio rings) | ~10-100 us | Lower (copy + protocol) | Heterogeneous (R5F ↔ A53/Linux) or many endpoints |
| Mailbox IP raw register access | ~0.5-2 us | <= 8 x 32-bit FIFO | Bare-metal speed; little gain over IpcNotify in practice |

For two R5F cores on the same SoC, the shared-SRAM + IpcNotify combination is the lowest-latency option that's still maintainable. RPMessage would be a regression here.

---

## Roadmap

- [ ] Drop `IPC_PERIOD_MS` from 1000 to 1 (also silence the per-iteration PASS log).
- [ ] Replace `process_buffer` with the target model code (keep the `(in, out, len)` contract).
- [ ] If the model needs cycle counts beyond µs resolution, add a `ClockP_getTimeUsec` snapshot around the `process_buffer` call on Core 1 and stash it in a shared field.
- [ ] If you need < 1 ms periods, lower `usecPerTick` in `example.syscfg` (both cores) and rebuild.
- [ ] If buffers grow past ~4 KB, consider moving the channel to cacheable memory and adding the four `CacheP_*` calls noted above.

---

## License

The original source carries the TI BSD-3 example license (see header in `ipc_pingpong.c`). The modifications in this project follow the same license unless you decide otherwise for your own derived work.
