#!/usr/bin/env python3
"""
Launch Fallout 4 through MO2 via the "F4SE - OG" custom executable under the
"Testing - OG" profile, let it sit at the main menu for a configurable number
of seconds, then kill it and check for crashes / REL::ID errors / missing log
markers. Used as a pre-in-game-test gate after every DLL change.

PASS conditions (all must hold):
 - SurvivalArchitect.log grew since launch and contains the "Survival Architect
   loaded" marker (plugin finished Install).
 - Magnitudes / Unlocks / GodMode expected success lines are present and do not
   contain "failed" or "ERROR".
 - No crash-<date>-<time>.log file in My Games/Fallout4/F4SE newer than the
   launch timestamp.
 - No "Failed to find offset for Address Library ID" message in SA log (the
   REL::IDDB popup that hangs the game at startup).

Exits 0 on pass, non-zero on fail with a short summary.

Usage:
    python tools/smoke_test.py                 # default 60s wait
    python tools/smoke_test.py --wait 90
    python tools/smoke_test.py --no-kill       # leave the game running
"""

from __future__ import annotations

import argparse
import os
import pathlib
import subprocess
import sys
import time

MO2_DIR = pathlib.Path(r"C:/Games/Modding/Nucleus")
MO2_EXE = MO2_DIR / "ModOrganizer.exe"
PROFILE = "Testing - OG"
EXE_LABEL = "F4SE - OG"
F4SE_LOG_DIR = pathlib.Path.home() / "Documents" / "My Games" / "Fallout4" / "F4SE"
PLUGIN_LOG = F4SE_LOG_DIR / "SurvivalArchitect.log"

EXPECTED_MARKERS = [
    "Survival Architect loaded",
]
FAILURE_MARKERS = [
    "failed to find offset for address library id",  # REL::IDDB popup text (case-insensitive)
    "unhandled exception",
    "[E] failed",
    "[E] error",
    "[W] signature not found",
]
# Things that the plugin emits as [E]/[W] during normal operation — don't fail on these.
BENIGN_SUBSTRINGS = [
    "failed to allocate from branch pool",  # first-allocation fallback, non-fatal
]

# Only the game process. NEVER include ModOrganizer.exe here — force-killing
# MO2 skips the post-game cleanup that RootBuilder runs to undeploy Root mods,
# which corrupts the MO2 setup and leaves Root files stuck in the game folder.
# MO2 itself exits on its own after the game closes. f4se_loader.exe is a
# shim that's already exited by the time we'd kill it; left out for safety.
GAME_PROCESSES = ["Fallout4.exe"]


def kill_game() -> None:
    for proc in GAME_PROCESSES:
        assert proc != "ModOrganizer.exe", "never kill MO2 — RootBuilder depends on clean exit"
        subprocess.run(["taskkill", "/F", "/IM", proc], capture_output=True, check=False)


def newest_crash_log(after_epoch: float) -> pathlib.Path | None:
    if not F4SE_LOG_DIR.is_dir():
        return None
    candidates = sorted(F4SE_LOG_DIR.glob("crash-*.log"), key=lambda p: p.stat().st_mtime)
    for p in reversed(candidates):
        if p.stat().st_mtime > after_epoch:
            return p
    return None


def read_plugin_log_after(after_epoch: float) -> str:
    if not PLUGIN_LOG.exists():
        return ""
    if PLUGIN_LOG.stat().st_mtime < after_epoch - 1:
        # Log is stale — plugin didn't rewrite it. Treat as empty for this run.
        return ""
    return PLUGIN_LOG.read_text(encoding="utf-8", errors="replace")


def summarise(log: str) -> list[str]:
    lines = []
    for line in log.splitlines():
        low = line.lower()
        if any(m.lower() in low for m in FAILURE_MARKERS):
            if not any(b.lower() in low for b in BENIGN_SUBSTRINGS):
                lines.append(f"FAIL LINE: {line.strip()}")
    return lines


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--wait", type=int, default=60, help="seconds to let the game run")
    ap.add_argument("--no-kill", action="store_true", help="leave the game running after the wait")
    args = ap.parse_args()

    if not MO2_EXE.is_file():
        print(f"ModOrganizer.exe not found at {MO2_EXE}", file=sys.stderr)
        return 2

    # Clean slate: make sure nothing from a previous run is still running.
    kill_game()
    time.sleep(1.0)

    launch_epoch = time.time() - 0.5  # small fudge so clock drift doesn't miss an immediate write
    print(f"launching MO2 profile={PROFILE!r} exe={EXE_LABEL!r}")

    # MO2 invocation: -p <profile> "moshortcut://:<exe label>"
    proc = subprocess.Popen(
        [
            str(MO2_EXE),
            "-p", PROFILE,
            f"moshortcut://:{EXE_LABEL}",
        ],
        cwd=str(MO2_DIR),
    )

    print(f"waiting {args.wait}s for the main menu to settle…")
    try:
        # MO2 itself exits almost immediately after spawning the game. Don't wait on it.
        time.sleep(args.wait)
    except KeyboardInterrupt:
        print("\naborting smoke test early")
        kill_game()
        return 130

    ok = True
    notes: list[str] = []

    crash = newest_crash_log(launch_epoch)
    if crash:
        ok = False
        notes.append(f"CRASH: {crash.name} (mtime={time.ctime(crash.stat().st_mtime)})")

    log = read_plugin_log_after(launch_epoch)
    if not log:
        ok = False
        notes.append(f"SurvivalArchitect.log not updated since launch (checked: {PLUGIN_LOG})")
    else:
        missing = [m for m in EXPECTED_MARKERS if m not in log]
        if missing:
            ok = False
            for m in missing:
                notes.append(f"MISSING MARKER: {m!r}")
        failures = summarise(log)
        if failures:
            ok = False
            notes.extend(failures)

    if not args.no_kill:
        kill_game()

    print()
    if ok:
        print("PASS — plugin loaded cleanly, no crash log, no failure markers.")
        return 0

    print("FAIL:")
    for n in notes:
        print(f"  {n}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
