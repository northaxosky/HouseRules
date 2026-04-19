#!/usr/bin/env python3
"""
Find every CALL instruction targeting a given function, and report the
AddressLib ID of the containing function + the offset within that function.

Use this to locate hook sites for Hooks::Unlocks. Example: given
GetDifficultyLevel's AddressLib ID (922962 on OG 1.10.163), the script finds
every place the engine calls it (MenuOpenHandler::HandleEvent, PipboyMenu,
etc.) and reports the REL::Hook parameters we need: (function_id, offset).

Usage:
    python tools/find_call_sites.py \\
        --exe /path/to/Fallout4.exe \\
        --csv /path/to/offsets-1-10-163-0.csv \\
        --target-id 922962

Dependencies: pefile, capstone  (pip install pefile capstone)
"""

from __future__ import annotations

import argparse
import csv
import struct
import sys
from bisect import bisect_right
from dataclasses import dataclass

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
from capstone.x86 import X86_OP_IMM, X86_OP_MEM


@dataclass
class CallSite:
    call_rva: int
    function_rva: int
    function_id: int
    offset_in_function: int


def load_id_map(path: str) -> tuple[dict[int, int], list[tuple[int, int]]]:
    """Return (id->rva, sorted [(rva, id), ...]).

    Accepts either a CSV (`id,fo4_addr_hex`) or an F4SE V0 AddressLib `.bin`
    (`u64 count` + `count x {u64 id, u64 offset}`).
    """
    id_to_rva: dict[int, int] = {}
    if path.endswith(".bin"):
        with open(path, "rb") as fp:
            (count,) = struct.unpack("<Q", fp.read(8))
            data = fp.read(count * 16)
        for i in range(count):
            aid, off = struct.unpack_from("<QQ", data, i * 16)
            id_to_rva[aid] = off
    else:
        for row in csv.DictReader(open(path, encoding="utf-8")):
            aid = int(row["id"])
            rva = int(row["fo4_addr"], 16)
            id_to_rva[aid] = rva
    rva_sorted = sorted(((rva, aid) for aid, rva in id_to_rva.items()))
    return id_to_rva, rva_sorted


def containing_function(call_rva: int, rva_sorted: list[tuple[int, int]]) -> tuple[int, int] | None:
    """Return (function_rva, function_id) for the function that contains call_rva."""
    idx = bisect_right(rva_sorted, (call_rva, 1 << 63)) - 1
    if idx < 0:
        return None
    return rva_sorted[idx]


def find_text_section(pe: pefile.PE) -> pefile.SectionStructure:
    for section in pe.sections:
        if section.Name.rstrip(b"\x00") == b".text":
            return section
    raise RuntimeError("no .text section found")


def find_calls_to(exe_path: str, target_rva: int) -> list[int]:
    """Return RVAs of every direct CALL instruction whose destination is target_rva."""
    pe = pefile.PE(exe_path, fast_load=True)
    text = find_text_section(pe)
    text_rva = text.VirtualAddress
    text_data = text.get_data()

    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True
    md.skipdata = True  # vital: Fallout4.exe has data mixed into .text (jump tables, etc.)

    hits: list[int] = []
    for insn in md.disasm(text_data, text_rva):
        if insn.mnemonic != "call":
            continue
        ops = insn.operands
        if not ops:
            continue
        op = ops[0]
        # Direct call: CALL rel32 -> immediate operand is absolute VA (capstone resolves it)
        if op.type == X86_OP_IMM:
            if op.imm == target_rva:
                hits.append(insn.address)
    return hits


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", required=True)
    ap.add_argument("--csv", required=True,
                    help="AddressLib CSV (id,fo4_addr) or F4SE V0 .bin")
    ap.add_argument("--target-id", type=int, required=True,
                    help="AddressLib ID of the function whose call sites to find")
    args = ap.parse_args()

    id_to_rva, rva_sorted = load_id_map(args.csv)

    if args.target_id not in id_to_rva:
        print(f"error: target id {args.target_id} not in CSV", file=sys.stderr)
        return 1
    target_rva = id_to_rva[args.target_id]
    print(f"target id {args.target_id} -> RVA 0x{target_rva:X}")
    print()

    call_rvas = find_calls_to(args.exe, target_rva)
    print(f"found {len(call_rvas)} direct call sites")
    print()

    sites: list[CallSite] = []
    for call_rva in call_rvas:
        fn = containing_function(call_rva, rva_sorted)
        if fn is None:
            sites.append(CallSite(call_rva, 0, -1, 0))
            continue
        fn_rva, fn_id = fn
        sites.append(CallSite(call_rva, fn_rva, fn_id, call_rva - fn_rva))

    print(f"{'call_rva':>10}  {'func_id':>8}  {'func_rva':>10}  {'offset':>8}  REL::Hook candidate")
    print("-" * 78)
    for s in sites:
        print(f"0x{s.call_rva:08X}  {s.function_id:8d}  0x{s.function_rva:08X}  "
              f"0x{s.offset_in_function:06X}  REL::ID({{{s.function_id}, <NG>, <AE>}}), 0x{s.offset_in_function:X}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
