# AM2612 ESI Patcher

This version applies three traceable changes:

1. Corrects ESC DPRAM size from `60416` to `28672`.
2. Classifies module `0x71000001` as `Functional-Safety` with module class `195`.
3. Adds `SafetyParaMapping` metadata so TwinSAFE can recognize the FSoE module.

The existing six-word Rx/Tx PDO layout, object dictionary, slots, ProductCode, RevisionNo, and ModuleIdent are preserved.

## Install

```powershell
py -m pip install -r requirements.txt
```

## Run

```powershell
.\patch_esi.bat .\SSC-AM2612.xml
```

Generated beside the input:

- `*_patched.xml`
- `*_patch_report.txt`
- `*_changes.diff`

## Configuration

Edit `esi_patch_config.yaml`:

- `PATCH-001`: DPRAM correction
- `PATCH-002`: Functional-Safety module classification
- `PATCH-003`: TwinSAFE safety-parameter mapping

`PATCH-002` and `PATCH-003` together are the TwinSAFE-detection feature.

The patcher is idempotent. Running it on an already patched file reports `ALREADY APPLIED` rather than failing.
