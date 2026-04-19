#!/usr/bin/env python3
"""
Cross-check claimed hook call-site offsets against a real game binary.

Given a list of (AddressLib ID, claimed offset) pairs plus a "target" function
ID that each call site is supposed to call (e.g. PlayerCharacter::GetDifficultyLevel),
this tool disassembles the binary and reports which claimed offsets land on a
direct CALL to the target, and which don't.

Caught the latent NG bug where PipboyMenu::PipboyMenu was claimed at 0x34C
(correct on AE) but the NG CALL is actually at 0x347. Run this after any hook
addition or runtime change to avoid a repeat.

Usage:
    python tools/audit_hook_offsets.py \\
        --exe  '.../Fallout4.exe.unpacked.exe' \\
        --bin  '.../version-1-10-984-0.bin' \\
        --target-id 2233056 \\
        --sites tools/hook-sites-ng.json

Sites JSON: a list of objects with "id", "offset" (int or "0x..." hex string),
and "label". A "skip": true field marks a site as deliberately absent on the
runtime being audited. Optional "target_id" overrides --target-id per entry
(useful when one file covers hooks calling different targets, e.g. GDL vs
GetMenuDifficultyLevel).

Dependencies: pefile, capstone  (pip install pefile capstone)
"""

from __future__ import annotations

import argparse
import json
import struct
import sys

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_64


def load_id_to_rva(bin_path: str) -> dict[int, int]:
    """Parse an F4SE V0 AddressLib .bin file into {id: rva}."""
    with open(bin_path, "rb") as fp:
        data = fp.read()
    (count,) = struct.unpack_from("<Q", data, 0)
    result: dict[int, int] = {}
    for i in range(count):
        aid, rva = struct.unpack_from("<QQ", data, 8 + i * 16)
        result[aid] = rva
    return result


def find_text(pe: pefile.PE):
    for section in pe.sections:
        if section.Name.rstrip(b"\x00") == b".text":
            return section
    raise RuntimeError("no .text section")


def parse_offset(value) -> int:
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        return int(value, 16) if value.lower().startswith("0x") else int(value)
    raise TypeError(f"offset must be int or hex string, got {type(value).__name__}")


def call_offsets_to_target(text_data: bytes, text_rva: int, fn_rva: int,
                           target_va: int, image_base: int, max_scan: int = 0x2000) -> list[int]:
    """Return offsets within the function at fn_rva where a direct CALL targets target_va."""
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.skipdata = True  # jump tables in .text
    fn_va = image_base + fn_rva
    fn_off = fn_rva - text_rva
    buf = text_data[fn_off:fn_off + max_scan]
    hits: list[int] = []
    for insn in md.disasm(buf, fn_va):
        if insn.mnemonic != "call":
            continue
        try:
            tgt = int(insn.op_str, 16)
        except ValueError:
            continue
        if tgt == target_va:
            hits.append(insn.address - fn_va)
    return hits


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", required=True, help="unpacked game binary (.exe)")
    ap.add_argument("--bin", required=True, help="AddressLib .bin for same runtime")
    ap.add_argument("--target-id", type=int, required=False,
                    help="default AddressLib ID of the function every claimed offset should CALL "
                         "(required unless every site provides target_id)")
    ap.add_argument("--sites", required=True,
                    help="JSON file with list of {id, offset, label[, skip]}")
    ap.add_argument("--max-scan", type=lambda v: int(v, 0), default=0x2000,
                    help="max bytes to scan per function (default 0x2000)")
    args = ap.parse_args()

    id_to_rva = load_id_to_rva(args.bin)

    pe = pefile.PE(args.exe, fast_load=True)
    image_base = pe.OPTIONAL_HEADER.ImageBase
    text = find_text(pe)
    text_rva = text.VirtualAddress
    text_data = text.get_data()

    def resolve_target(tid: int) -> int | None:
        rva = id_to_rva.get(tid)
        return None if rva is None else image_base + rva

    default_target_va = None
    if args.target_id is not None:
        default_target_va = resolve_target(args.target_id)
        if default_target_va is None:
            print(f"target id {args.target_id} not present in {args.bin}", file=sys.stderr)
            return 2

    with open(args.sites, encoding="utf-8") as fp:
        sites = json.load(fp)
    if not isinstance(sites, list):
        print(f"sites file must be a JSON list", file=sys.stderr)
        return 2

    ok = 0
    mismatch = 0
    missing = 0
    skipped = 0
    header = f"{'status':<9}{'id':>10}  {'claim':>7}  {'actual':<28}  label"
    print(header)
    print("-" * len(header) * 1)
    for entry in sites:
        label = entry.get("label", "?")
        if entry.get("skip"):
            print(f"{'SKIP':<9}{entry.get('id', 0):>10}  {'-':>7}  {'(skipped on this runtime)':<28}  {label}")
            skipped += 1
            continue
        aid = entry["id"]
        claimed = parse_offset(entry["offset"])
        tgt_va = default_target_va
        if "target_id" in entry:
            tgt_va = resolve_target(entry["target_id"])
            if tgt_va is None:
                print(f"{'NO-TGT':<9}{aid:>10}  0x{claimed:<5X}  {'<target_id not in bin>':<28}  {label}")
                missing += 1
                continue
        if tgt_va is None:
            print(f"{'NO-TGT':<9}{aid:>10}  0x{claimed:<5X}  {'<no --target-id and no entry target_id>':<28}  {label}")
            missing += 1
            continue
        fn_rva = id_to_rva.get(aid)
        if fn_rva is None:
            print(f"{'NO-ID':<9}{aid:>10}  0x{claimed:<5X}  {'<id not in bin>':<28}  {label}")
            missing += 1
            continue
        actuals = call_offsets_to_target(text_data, text_rva, fn_rva, tgt_va, image_base, args.max_scan)
        actuals_str = ",".join(f"0x{x:X}" for x in actuals) or "<no call to target>"
        if claimed in actuals:
            print(f"{'OK':<9}{aid:>10}  0x{claimed:<5X}  {actuals_str:<28}  {label}")
            ok += 1
        else:
            print(f"{'MISMATCH':<9}{aid:>10}  0x{claimed:<5X}  {actuals_str:<28}  {label}")
            mismatch += 1

    print()
    print(f"summary: {ok} ok, {mismatch} mismatch, {missing} id-missing, {skipped} skipped")
    return 0 if (mismatch == 0 and missing == 0) else 1


if __name__ == "__main__":
    sys.exit(main())
