#!/usr/bin/env python3
"""Parse House Rules HRVERIFY audit log lines and report PASS/FAIL.

Reads the F4SE plugin log produced by House Rules and validates structured
HRVERIFY / HRVERIFY_SUMMARY records emitted at load time.

Exit codes:
  0  -- every HRVERIFY_SUMMARY parsed has result=PASS, at least one summary
        was found (unless --allow-empty), and every required module appeared.
  1  -- any summary FAIL, any per-target FAIL/SKIP, missing required module,
        or no summaries found without --allow-empty.
  2  -- log file not found / unreadable / argument error.
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable

DEFAULT_LOG_PATH = Path(
    r"C:\Users\Kuz\Documents\My Games\Fallout4\F4SE\HouseRules.log"
)

SESSION_MARKER = "House Rules loading"
TOKEN_RE = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)=(\"[^\"]*\"|\S+)")


@dataclass
class Record:
    kind: str  # "verify" or "summary"
    fields: dict[str, str]
    raw: str
    lineno: int


@dataclass
class ModuleReport:
    module: str
    summaries: list[Record] = field(default_factory=list)
    fails: list[Record] = field(default_factory=list)
    skips: list[Record] = field(default_factory=list)
    passes: int = 0

    def summary_totals(self) -> tuple[int, int, int, int, str]:
        applied = passed = failed = skipped = 0
        result = "PASS"
        for rec in self.summaries:
            f = rec.fields
            applied += int_field(f, "applied")
            passed += int_field(f, "passed")
            failed += int_field(f, "failed")
            skipped += int_field(f, "skipped")
            if f.get("result", "").upper() != "PASS":
                result = "FAIL"
        return applied, passed, failed, skipped, result


def parse_tokens(payload: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for m in TOKEN_RE.finditer(payload):
        key = m.group(1)
        val = m.group(2)
        if val.startswith('"') and val.endswith('"'):
            val = val[1:-1]
        out[key] = val
    return out


def int_field(fields: dict[str, str], key: str) -> int:
    try:
        return int(fields.get(key, "0"))
    except ValueError:
        return 0


def iter_records(lines: Iterable[tuple[int, str]]) -> Iterable[Record]:
    for lineno, line in lines:
        # Find HRVERIFY token regardless of leading log prefix like
        # "[09:47:01.184] [14076] [I] HRVERIFY ...".
        idx = line.find("HRVERIFY")
        if idx < 0:
            continue
        rest = line[idx:].rstrip()
        head, _, payload = rest.partition(" ")
        if head == "HRVERIFY_SUMMARY":
            kind = "summary"
        elif head == "HRVERIFY":
            kind = "verify"
        else:
            continue
        yield Record(kind=kind, fields=parse_tokens(payload), raw=rest, lineno=lineno)


def select_session(all_lines: list[str], latest: bool) -> list[tuple[int, str]]:
    numbered = list(enumerate(all_lines, start=1))
    if not latest:
        return numbered
    last_marker = -1
    for i, line in numbered:
        if SESSION_MARKER in line:
            last_marker = i
    if last_marker < 0:
        return numbered
    return [(i, l) for (i, l) in numbered if i >= last_marker]


def build_reports(records: list[Record]) -> dict[str, ModuleReport]:
    reports: dict[str, ModuleReport] = {}
    for rec in records:
        mod = rec.fields.get("module", "<unknown>")
        rep = reports.setdefault(mod, ModuleReport(module=mod))
        result = rec.fields.get("result", "").upper()
        if rec.kind == "summary":
            rep.summaries.append(rec)
        else:
            if result == "PASS":
                rep.passes += 1
            elif result == "FAIL":
                rep.fails.append(rec)
            elif result == "SKIP":
                rep.skips.append(rec)
    return reports


def fmt_target(rec: Record) -> str:
    f = rec.fields
    parts = [f"target={f.get('target','?')}"]
    for k in ("type", "mode", "baseline", "user", "expected", "readback", "reason"):
        if k in f:
            parts.append(f"{k}={f[k]}")
    return f"  L{rec.lineno}: " + " ".join(parts)


def render(reports: dict[str, ModuleReport], required: list[str], allow_empty: bool) -> tuple[int, str]:
    out: list[str] = []
    summaries = [s for r in reports.values() for s in r.summaries]
    any_target_fail = any(r.fails for r in reports.values())
    any_target_skip = any(r.skips for r in reports.values())
    any_summary_fail = any(
        s.fields.get("result", "").upper() != "PASS" for s in summaries
    )
    missing_required = [m for m in required if m not in reports]

    fail = False
    reasons: list[str] = []
    if not summaries and not allow_empty:
        fail = True
        reasons.append("no HRVERIFY_SUMMARY records found")
    if any_summary_fail:
        fail = True
        reasons.append("one or more module summaries FAILED")
    if any_target_fail:
        fail = True
        reasons.append("one or more targets FAILED")
    if any_target_skip:
        fail = True
        reasons.append("one or more targets SKIPPED")
    if missing_required:
        fail = True
        reasons.append(f"missing required modules: {', '.join(missing_required)}")

    headline = "FAIL" if fail else "PASS"
    out.append(f"House Rules validation: {headline}")
    if reasons:
        for r in reasons:
            out.append(f"  - {r}")

    if reports:
        out.append("")
        out.append(f"{'Module':<20} {'Applied':>7} {'Passed':>6} {'Failed':>6} {'Skipped':>7} {'Result':>6}")
        out.append("-" * 60)
        for mod in sorted(reports):
            rep = reports[mod]
            if rep.summaries:
                applied_i, passed_i, failed_i, skipped_i, result = rep.summary_totals()
                applied = str(applied_i)
                passed = str(passed_i)
                failed = str(failed_i)
                skipped = str(skipped_i)
            else:
                applied = str(rep.passes + len(rep.fails) + len(rep.skips))
                passed = str(rep.passes)
                failed = str(len(rep.fails))
                skipped = str(len(rep.skips))
                result = "NO-SUM"
            out.append(f"{mod:<20} {applied:>7} {passed:>6} {failed:>6} {skipped:>7} {result:>6}")
    else:
        out.append("(no HRVERIFY records in selected session)")

    detail_lines: list[str] = []
    for mod in sorted(reports):
        rep = reports[mod]
        if rep.fails or rep.skips:
            detail_lines.append(f"\n[{mod}] issues:")
            for rec in rep.fails:
                detail_lines.append("FAIL " + fmt_target(rec).lstrip())
            for rec in rep.skips:
                detail_lines.append("SKIP " + fmt_target(rec).lstrip())
    if detail_lines:
        out.append("")
        out.extend(detail_lines)

    return (1 if fail else 0), "\n".join(out) + "\n"


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("log", nargs="?", default=str(DEFAULT_LOG_PATH),
                   help=f"Path to HouseRules.log (default: {DEFAULT_LOG_PATH}).")
    p.add_argument("--allow-empty", action="store_true",
                   help="Do not fail when no HRVERIFY_SUMMARY records are found.")
    p.add_argument("--require-module", action="append", default=[], metavar="NAME",
                   help="Required module name; pass multiple times to require several.")
    p.add_argument("--all-sessions", action="store_true",
                   help="Parse the entire log instead of only the latest 'House Rules loading' session.")
    args = p.parse_args(argv)

    log_path = Path(args.log)
    try:
        text = log_path.read_text(encoding="utf-8", errors="replace")
    except OSError as e:
        print(f"error: cannot read log: {e}", file=sys.stderr)
        return 2

    lines = text.splitlines()
    selected = select_session(lines, latest=not args.all_sessions)
    records = list(iter_records(selected))
    reports = build_reports(records)
    code, output = render(reports, args.require_module, args.allow_empty)
    sys.stdout.write(output)
    return code


if __name__ == "__main__":
    raise SystemExit(main())
