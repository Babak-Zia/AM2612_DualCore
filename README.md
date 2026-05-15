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
├── common/                              Shared transport (linked into both core projects)
│   ├── ipc_channel.h / ipc_channel.c    **Production path** — zero-copy `gIpcCh` + IpcNotify
│
├── Core0-freertos/                      Core 0 (R5F-0) - master, FreeRTOS
│   ├── main_core0.c                     entry point + FreeRTOS task creation
│   ├── core0_app.h / core0_app.c      master: startup helpers + IPC loop + summary
│   ├── example.syscfg                   SysConfig (drivers, MPU, memory, IPC, debug log)
│   ├── .project / .cproject / .ccsproject   CCS project descriptors
│   ├── targetConfigs/                   debug probe config (XDS-class)
│   └── Debug/                           (auto-generated build output, gitignored)
│
├── Core1_nortos/                        Core 1 (R5F-1) - worker, NoRTOS
│   ├── main_core1.c                     entry point - calls core1_app_run()
│   ├── core1_app.c                      worker loop + process_buffer swap-point
│   ├── example.syscfg                   SysConfig for Core 1
│   └── (mirrors Core0-freertos/ structure)
│
└── System_DualCore/                     Meta project - combined boot image
    ├── system.xml                       lists the two .appimage participants
    └── makefile_system_ccs_bootimage_gen
```

`common/ipc_channel.c` is **linked** into both CCS projects (see `.project` → `linkedResources`).

`core0_app.c` / `core1_app.c` use **`ipc_channel` only**: they read and write **`gIpcCh.req_buf` / `gIpcCh.resp_buf` directly** (no extra `memcpy` in the hot path). Doorbells use **`IPC_DOORBELL_SEND_FAST` (0)** from `ipc_channel.h` for minimum mailbox latency with **one transaction in flight** per direction; switch to **`IPC_DOORBELL_SEND_WAIT` (1)** if you pipeline multiple notifies and see drops.

---

## Performance choices (this project)

| Choice | Why |
|---|---|
| **`ipc_channel` + in-place `gIpcCh` buffers** | Avoids copying the whole payload twice per round trip (the main cost the old helper layer added). |
| **`IPC_DOORBELL_SEND_FAST`** | Skips the “wait for TX FIFO empty” spin in `IpcNotify_sendMsg` when the mailbox is not back-pressured — saves cycles on every transaction. |
| **Non-cacheable `.bss.user_shared_mem`** | No `CacheP_*` in the loop; mandatory for correctness unless you move the section and add explicit cache ops. |
| **Single in-flight request** | Keeps the worker’s single-slot `gWorkerNewReqSeq` model valid; do not start a new `send_request` before `wait_reply` completes unless you redesign for pipelining. |

If you later need **RPMessage**, Linux coupling, or many endpoints, expect higher latency — stay on **`ipc_channel`** for bare R5F↔R5F throughput.

---

## Runtime architecture

```
Core 0 (FreeRTOS, R5F-0)                       Core 1 (NoRTOS, R5F-1)
─────────────────────────                      ─────────────────────────
main()                                         main()
 ├ System_init / Board_init                     ├ System_init / Board_init
 └ xTaskCreateStatic(freertos_main)             └ core1_app_run()
   └ freertos_main()                                ├ Drivers_open
     └ core0_app_run()                              ├ Board_driversOpen
       ├ Drivers_open / Board_driversOpen           ├ ipc_channel_worker_init
       ├ srand(...)                                 ├ ipc_channel_sync_all()
       ├ ipc_channel_master_init                    │
       ├ ipc_channel_sync_all() ◄───────────────────┘
       └ for iter = 0 .. N-1:
         ┌─ fill req_buf with rand()
         │   ipc_channel_master_commit_request
         │   t0 = ClockP_getTimeUsec()
         │   ipc_channel_master_send_request(FAST)
         │   ipc_channel_master_wait_reply_usec
         │   t1 = ClockP_getTimeUsec()
         │   verify resp_buf[i] == req_buf[i] + 1
         │   print PASS/FAIL + RTT us
         └─ ClockP_usleep(IPC_PERIOD_MS * 1000)

       Final summary + ipc_channel_master_deinit()

Core 1 (right column, after sync): `for (;;)` → wait request → process_buffer → send_reply → WFI.
```

### Why this is fast

| Choice | Effect |
|---|---|
| Shared SRAM, zero-copy in demo | Master and worker touch **`gIpcCh.req_buf` / `resp_buf`** directly — no helper-layer `memcpy`. |
| **`IPC_DOORBELL_SEND_FAST` (0)** | Demo uses fast doorbell mode; use **`IPC_DOORBELL_SEND_WAIT` (1)** if you burst notifies and see drops. |
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

### Shared transport API (`common/ipc_channel.h` / `ipc_channel.c`)

Source of truth for behavior is the **block comments on each function** in `ipc_channel.c`. The following is a prose reference for the README.

#### Prerequisites

- Call **`Drivers_open()`** (and normally **`Board_driversOpen()`**) on each core *before* **`ipc_channel_*_init`**, **`ipc_channel_sync_all`**, or any **`ipc_channel_*_send_*`**. IpcNotify depends on SysConfig-generated driver init.
- On Core 0, **`Drivers_open()`** / **`Board_driversOpen()`**, **`srand()`**, **`ipc_channel_master_init()`**, and **`ipc_channel_sync_all()`** all run at the start of **`core0_app_run()`** (invoked from **`freertos_main`** after **`vTaskStartScheduler()`**). **`ipc_channel_master_init()`** uses RTOS-backed **`SemaphoreP`** and must not run from **`main()`** before the scheduler starts.
- **`ipc_channel_sync_all()`** must run on **both** cores **after** each side has called its `*_init`, so neither core sends doorbells before the peer has registered its notify client.

#### One transaction, correct order

| Step | Core 0 (master) | Core 1 (worker) |
|---|---|---|
| 1 | Write `gIpcCh.req_buf[]` | — |
| 2 | `seq = ipc_channel_master_commit_request()` | — |
| 3 | `ipc_channel_master_send_request(seq, IPC_DOORBELL_SEND_FAST)` | ISR stores `seq` → main wakes |
| 4 | `ipc_channel_master_wait_reply` **or** `ipc_channel_master_wait_reply_usec` | `seq = ipc_channel_worker_wait_request()` |
| 5 | (optional) check `gIpcCh.resp_seq == seq` | Read `req_buf`, compute into `resp_buf` |
| 6 | Read `resp_buf` | `ipc_channel_worker_send_reply(seq, IPC_DOORBELL_SEND_FAST)` → master ISR posts sem |

Use **`ipc_channel_master_wait_reply_usec`** when the budget is **shorter than one DPL tick** (default **1 ms/tick** → `IPC_MS_TO_TICKS(1)` is the minimum tick-based wait). The µs variant **busy-spins** with `SemaphoreP_pend(..., 0)` until the ISR fires or the deadline elapses — fine for hundreds of µs; for multi-millisecond waits prefer **`ipc_channel_master_wait_reply`** + **`IPC_MS_TO_TICKS`** so FreeRTOS can block without burning CPU.

The **`seq`** value ties together: `req_seq` publish, notify payload, `resp_seq`, and application-level “which transaction is this?” checks (drops, stale replies).

#### Functions (summary)

| Function | Core | Role |
|---|---|---|
| **`ipc_channel_master_init`** | 0 | Build binary semaphore (starts “taken” so first `pend` blocks); register **`IPC_CLIENT_ID`** so worker → master notifies run **`ipc_on_worker_reply`**, which **`SemaphoreP_post`**s the master task. |
| **`ipc_channel_master_deinit`** | 0 | Destroy that semaphore. Does **not** unregister IpcNotify (this layer does not expose unregister). |
| **`ipc_channel_master_commit_request`** | 0 | **Single-writer bump** of `gIpcCh.req_seq`. Call **after** filling `req_buf`, **before** `send_request`. Returns the new sequence used as the transaction id. |
| **`ipc_channel_master_send_request(seq, waitFifoEmpty)`** | 0 | **`IpcNotify_sendMsg`** to **`IPC_CORE_WORKER`**. Pass **`IPC_DOORBELL_SEND_FAST`** for minimum latency (one in-flight notify); **`IPC_DOORBELL_SEND_WAIT`** if the mailbox FIFO might fill. |
| **`ipc_channel_master_wait_reply(timeoutTicks)`** | 0 | **`SemaphoreP_pend`** — timeout in **DPL ticks** (default **1 ms/tick**). Minimum practical tick timeout ≈ **1 ms**. |
| **`ipc_channel_master_wait_reply_usec(timeoutUsec)`** | 0 | **`ClockP_getTimeUsec`** deadline + **`SemaphoreP_pend(..., 0)`** spin. Use for **sub-millisecond** budgets (e.g. **200 µs**). Busy-waits until reply or timeout. |
| **`ipc_channel_worker_init`** | 1 | Clear pending-request flag; register **`IPC_CLIENT_ID`** so master → worker notifies run **`ipc_on_master_request`**, which stores the **32-bit payload** (`seq`) for the main loop. |
| **`ipc_channel_worker_wait_request`** | 1 | **`WFI`** loop until `gWorkerNewReqSeq != 0`, then **clear** it and **return** `seq`. **Not** safe for nested/concurrent callers; one worker loop is assumed. If the master overruns the worker, only the **latest** seq is kept (single-slot flag). |
| **`ipc_channel_worker_send_reply(seq, waitFifoEmpty)`** | 1 | Set **`gIpcCh.resp_seq = seq`**, then **`IpcNotify_sendMsg`** to **`IPC_CORE_MASTER`**. Call **after** `resp_buf` is fully written. Same **`FAST` / `WAIT`** trade-off as master send. |
| **`ipc_channel_sync_all`** | both | **`IpcNotify_syncAll(WAIT_FOREVER)`** — rendezvous barrier so startup ordering is safe. |

#### Private ISR callbacks (in `ipc_channel.c` only)

- **`ipc_on_worker_reply`** (Core 0): posts **`gMasterRespSem`**. Keep ISR minimal; no **`DebugP_log`**.
- **`ipc_on_master_request`** (Core 1): assigns **`gWorkerNewReqSeq = msgValue`**. Same rule: no logging from ISR on NoRTOS shared-log path.

#### What this layer deliberately does not do

- No **spinlocks** (single-writer fields + doorbell ordering are enough for this ping-pong).
- No **cache maintenance** (assumes **`.bss.user_shared_mem`** is **non-cacheable** per MPU; if you change that, add **`CacheP_*`** in the app around buffer access).
- No **payload validation** or **UART** output.

For failure-mode terminology (freeze vs seq mismatch vs corrupt bytes), see **Sequence-number protocol** below (`ipc_channel_master_wait_reply` **or** `ipc_channel_master_wait_reply_usec`).

---

## Sequence-number protocol

Every transaction has a monotonically increasing `uint32_t` sequence number, set by the master and echoed back by the worker. The same number is carried on the IpcNotify doorbell **and** written into the channel, which lets the master detect every interesting failure mode with a single equality test:

| Failure mode | Detection | Counter | Typical log (demo) |
|---|---|---|---|
| **Drop / stale** | `gIpcCh.resp_seq != expected_seq` after `ipc_channel_master_wait_reply` succeeds | `mis_seq` | `SEQ MISMATCH sent=… got=…` |
| **Freeze** | `ipc_channel_master_wait_reply` **or** `ipc_channel_master_wait_reply_usec` returns non-success after timeout | `freezes` | `FREEZE …` |
| **Data corruption** | per-byte `resp_buf[i] != (uint8_t)(req_buf[i] + 1)` | `failed` | `FAIL … bytes wrong` |
| **Success** | pend OK, `resp_seq` match, bytes verify | `passed` | `PASS seq=… RTT=…` |

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

Application timing and iteration count are in `Core0-freertos/core0_app.c`:

| `#define` | Default | Meaning |
|---|---|---|
| `IPC_PERIOD_MS` | `1000U` | master loop period |
| `IPC_RESP_TIMEOUT_US` | `200U` | reply wait budget in **microseconds** (`ipc_channel_master_wait_reply_usec`) |
| `IPC_TEST_ITERATIONS` | `50U` | total transactions (`0` = run forever) |

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

## Swap-point: the `process_buffer` function

Single `static` function in `Core1_nortos/core1_app.c`:

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

If you ever relocate the channel to a cacheable region (for higher SRAM bandwidth on very large buffers), add cache ops at producer/consumer boundaries in `core0_app.c` / `core1_app.c` (or wrap calls around `ipc_channel_*`):

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
| **Shared SRAM + IpcNotify doorbell** (this demo) | **~3-7 us** | SRAM bandwidth (GB/s) | Real-time, custom protocol, fixed peers |
| Shared SRAM + busy-polling flag (no IRQ) | ~50-200 ns | SRAM bandwidth | Hard µs-scale latency, one core can be 100% spinning |
| `IpcNotify` payload only (no shared mem) | ~1-3 us | <= 4 bytes per message | Pure signaling / tiny commands |
| **RPMessage** (virtio rings) | ~10-100 us | Lower (copy + protocol) | Heterogeneous (R5F ↔ A53/Linux) or many endpoints |
| Mailbox IP raw register access | ~0.5-2 us | <= 8 x 32-bit FIFO | Bare-metal speed; little gain over IpcNotify in practice |

For two R5F cores on the same SoC, the shared-SRAM + IpcNotify combination is the lowest-latency option that's still maintainable. RPMessage would be a regression here.

---

## Roadmap

- [ ] In `core0_app.c`: drop `IPC_PERIOD_MS` from 1000 to 1 (also silence the per-iteration PASS log).
- [ ] Replace `process_buffer` in `core1_app.c` with the target model code (keep the `(in, out, len)` contract).
- [ ] If the model needs cycle counts beyond µs resolution, add a `ClockP_getTimeUsec` snapshot around the `process_buffer` call on Core 1 and stash it in a shared field.
- [ ] If you need < 1 ms periods, lower `usecPerTick` in `example.syscfg` (both cores) and rebuild.
- [ ] If buffers grow past ~4 KB, consider moving the channel to cacheable memory and adding the four `CacheP_*` calls noted above.

---

## License

The transport files (`common/ipc_channel.c`) and app files carry the TI BSD-3 style headers where applicable. The modifications in this project follow the same license unless you decide otherwise for your own derived work.
