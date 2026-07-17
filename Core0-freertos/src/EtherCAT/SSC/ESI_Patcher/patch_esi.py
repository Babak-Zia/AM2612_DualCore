from __future__ import annotations
import argparse, difflib, hashlib
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path
from typing import Any
import yaml

@dataclass
class Change:
    description: str
    old: str
    new: str

@dataclass
class Result:
    patch_id: str
    name: str
    status: str
    message: str = ""
    changes: list[Change] = field(default_factory=list)

def read_text_preserve(path: Path):
    raw = path.read_bytes()
    if raw.startswith(b"\xef\xbb\xbf"):
        return raw.decode("utf-8-sig"), "utf-8-sig"
    try:
        return raw.decode("utf-8"), "utf-8"
    except UnicodeDecodeError:
        return raw.decode("utf-16"), "utf-16"

def write_text_preserve(path: Path, text: str, encoding: str):
    path.write_bytes(text.encode(encoding))

def sha256(path: Path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def replace_exact_text(text: str, old: str, new: str, occurrence: int = 1):
    # YAML stores patch blocks with LF. Adapt them to the input file's
    # line-ending convention so the patch works with SSC's CRLF output too.
    newline = "\r\n" if "\r\n" in text else "\n"
    old_effective = old.replace("\r\n", "\n").replace("\n", newline)
    new_effective = new.replace("\r\n", "\n").replace("\n", newline)

    count = text.count(old_effective)
    if count < occurrence:
        # Make patches idempotent: an already-patched input is a success.
        if text.count(new_effective) >= occurrence:
            return text, Result("", "", "ALREADY APPLIED",
                                f"Replacement text is already present (occurrence {occurrence}).")
        return text, Result("", "", "NOT APPLICABLE",
                            f"Expected occurrence {occurrence} of exact text, found {count}. Searched for: {old!r}")

    pos = -1
    search_start = 0
    for _ in range(occurrence):
        pos = text.find(old_effective, search_start)
        search_start = pos + len(old_effective)

    patched = text[:pos] + new_effective + text[pos + len(old_effective):]
    return patched, Result("", "", "APPLIED",
                           changes=[Change("Exact text replacement", old_effective, new_effective)])

def apply_patch(text: str, patch: dict[str, Any]):
    pid, name = str(patch["id"]), str(patch["name"])
    if patch["handler"] != "replace_exact_text":
        return text, Result(pid, name, "FAILED", f"Unknown handler: {patch['handler']}")
    p = patch["parameters"]
    patched, result = replace_exact_text(
        text, str(p["old"]), str(p["new"]), int(p.get("occurrence", 1))
    )
    result.patch_id, result.name = pid, name
    return patched, result

def main():
    ap = argparse.ArgumentParser(description="Minimal-diff ESI patcher")
    ap.add_argument("--input", type=Path, required=True)
    ap.add_argument("--config", type=Path,
                    default=Path(__file__).with_name("esi_patch_config.yaml"))
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--list-patches", action="store_true")
    args = ap.parse_args()

    cfg = yaml.safe_load(args.config.read_text(encoding="utf-8"))
    patches = cfg.get("patches", [])

    if args.list_patches:
        for p in patches:
            print(f"{p['id']}: {'ENABLED' if p.get('enabled') else 'DISABLED'} - {p['name']}")
        return 0

    inp = args.input.resolve()
    if not inp.exists():
        print(f"ERROR: Input file does not exist: {inp}")
        return 2

    original, encoding = read_text_preserve(inp)
    required = cfg.get("validation", {}).get("require_text")
    if required and str(required).lower() not in original.lower():
        print(f"ERROR: Required text '{required}' was not found.")
        return 3

    current = original
    results = []
    failed = False

    for patch in patches:
        if not patch.get("enabled", False):
            results.append(Result(str(patch["id"]), str(patch["name"]), "DISABLED"))
            continue
        current, result = apply_patch(current, patch)
        results.append(result)
        if result.status in ("FAILED", "NOT APPLICABLE"):
            failed = True
            break

    suffix = cfg.get("output", {}).get("suffix", "_patched")
    out = inp.with_name(inp.stem + suffix + inp.suffix)
    report = inp.with_name(inp.stem + "_patch_report.txt")
    diff = inp.with_name(inp.stem + "_changes.diff")

    lines = [
        "ESI PATCH REPORT", "================", "",
        f"Date: {datetime.now().isoformat(timespec='seconds')}",
        f"Input: {inp}",
        f"Configuration: {args.config.resolve()}",
        f"Detected encoding: {encoding}", ""
    ]
    for r in results:
        lines += [f"[{r.patch_id}] {r.name}", f"Status: {r.status}"]
        if r.message:
            lines.append(f"Message: {r.message}")
        for c in r.changes:
            lines.append(f"Change: {c.description}: {c.old!r} -> {c.new!r}")
        lines.append("")
    lines.append("Result: " + ("FAILED" if failed else "SUCCESS"))

    if args.dry_run:
        print("\n".join(lines))
        return 1 if failed else 0

    report.write_text("\n".join(lines) + "\n", encoding="utf-8")
    if failed:
        print(f"FAILED. Report: {report}")
        return 1

    write_text_preserve(out, current, encoding)
    lines += [f"Input SHA-256: {sha256(inp)}", f"Output SHA-256: {sha256(out)}"]
    report.write_text("\n".join(lines) + "\n", encoding="utf-8")
    diff.write_text("".join(difflib.unified_diff(
        original.splitlines(True), current.splitlines(True),
        fromfile=inp.name, tofile=out.name, n=3
    )), encoding="utf-8")

    print(f"SUCCESS: {out}")
    print(f"REPORT:  {report}")
    print(f"DIFF:    {diff}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
