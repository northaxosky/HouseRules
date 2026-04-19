#!/usr/bin/env python3
"""
Cross-reference a function name against the IDA name-port JSON + OG AddressLib CSV
to produce OG AddressLib IDs. Useful for porting Baka's NG hook IDs to OG.

Usage:
    python tools/lookup_og_id.py "MenuOpenHandler::HandleEvent"
    python tools/lookup_og_id.py --regex "HandlePositionPlayer"

Defaults:
    --json: .local/re-resources/.../fallout4_og_funcs.json
    --csv : /tmp/addrlib/og.csv
"""
from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from pathlib import Path


DEFAULT_JSON = (
    Path(__file__).resolve().parent.parent
    / ".local/re-resources/Fallout4_IDA_OG_to_AE1-11-191_NamePort/fallout4_og_funcs.json"
)
DEFAULT_CSV = Path(__file__).resolve().parent.parent / ".local/re-resources/og-addresslib.csv"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("query", help="substring or regex to match against mangled names")
    ap.add_argument("--regex", action="store_true", help="treat query as a regex")
    ap.add_argument("--json", default=str(DEFAULT_JSON))
    ap.add_argument("--csv", default=str(DEFAULT_CSV))
    ap.add_argument("--limit", type=int, default=20, help="max matches to print")
    args = ap.parse_args()

    data = json.loads(Path(args.json).read_text(encoding="utf-8"))
    image_base = data["image_base"]

    # Build RVA -> AddressLib ID map
    rva_to_id: dict[int, int] = {}
    with open(args.csv, encoding="utf-8") as fp:
        for row in csv.DictReader(fp):
            rva_to_id[int(row["fo4_addr"], 16)] = int(row["id"])

    pattern = (
        re.compile(args.query)
        if args.regex
        else re.compile(re.escape(args.query))
    )

    matches = []
    for fn in data["functions"]:
        name = fn["name"]
        if pattern.search(name):
            va = fn["start"]
            rva = va - image_base
            aid = rva_to_id.get(rva)
            matches.append((name, va, rva, aid))

    if not matches:
        print(f"no match for {args.query!r}", file=sys.stderr)
        return 1

    print(f"{'og_id':>10}  {'og_rva':>10}  name")
    print("-" * 100)
    for name, va, rva, aid in matches[: args.limit]:
        aid_s = f"{aid}" if aid is not None else "<no id>"
        trimmed = name if len(name) < 80 else name[:77] + "..."
        print(f"{aid_s:>10}  0x{rva:08X}  {trimmed}")
    if len(matches) > args.limit:
        print(f"... ({len(matches) - args.limit} more)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
