"""Print a Markdown table from the result lines of the page route probe.

Usage:
    python3 experiments/001-page-routes/summarize_routes.py RESULTS.jsonl [--baseline fill]

Reads lines that begin with {"page_route_probe": (any other line is skipped,
so a ctest log can be given as it is). "Held" is the route's private bytes at
the end of the idle sample less the baseline route's: what the route keeps
beyond what every run keeps (the window and the probe's two source pages).
"""

import argparse
import json
from pathlib import Path

RESULT_MARK: str = '{"page_route_probe":'


def read_results(path: Path) -> list[dict]:
    results: list[dict] = []
    text: str = path.read_text(encoding="utf-8", errors="replace")
    line: str = ""
    for line in text.splitlines():
        start: int = line.find(RESULT_MARK)
        if start < 0:
            continue
        # A ctest log also quotes the mark when it reports the pass
        # expression; such a line is not a result.
        try:
            record: dict = json.loads(line[start:])
        except json.JSONDecodeError:
            continue
        results.append(record["page_route_probe"])
    return results


def megabytes(count: float) -> str:
    text: str = "%.1f" % (count / 1_000_000.0)
    return text


def main() -> int:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("results", type=Path)
    parser.add_argument("--baseline", default="fill")
    arguments: argparse.Namespace = parser.parse_args()

    results: list[dict] = read_results(arguments.results)
    baseline_private: float = 0.0
    result: dict = {}
    for result in results:
        if result["route"] == arguments.baseline and "private_bytes" in result:
            baseline_private = float(result["private_bytes"])

    print("| Route | Presented | p50 ms | p95 ms | max ms | Update p50 ms | Host paint p50 ms "
          "| Paints per turn | Private MB | Held MB | Idle: CPU ms, paints, frames, ticks, live | Idle: megacycles, context switches |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    for result in results:
        if "failed" in result:
            print("| %s | failed: %s |" % (result["route"], result["failed"]))
            continue
        observed: dict = result["observed_ms"]
        update: dict = result["update_ms"]
        host: dict = result["host_present_ms"]
        idle: dict = result["idle"]
        turns: int = int(result["turns"])
        paints_per_turn: float = float(result["paint_passes_during_turns"]) / float(max(turns, 1))
        idle_cpu_ms: float = (float(idle["cpu_user_ns"]) + float(idle["cpu_kernel_ns"])) / 1_000_000.0
        live: str = "yes" if idle["live_surface_registered"] else "no"
        megacycles: float = float(idle.get("cycles", 0)) / 1_000_000.0
        switches: int = int(idle.get("context_switches", 0))
        print("| %s | %d/%d | %.1f | %.1f | %.1f | %.1f | %.1f | %.1f | %s | %s | %.1f, %d, %d, %d, %s | %.1f, %d |" % (
            result["route"], int(result["presented_turns"]), turns,
            observed["p50"], observed["p95"], observed["max"], update["p50"], host["p50"],
            paints_per_turn, megabytes(float(result["private_bytes"])),
            megabytes(float(result["private_bytes"]) - baseline_private),
            idle_cpu_ms, int(idle["paint_passes"]), int(idle["frames_presented"]),
            int(idle["active_surface_ticks"]), live, megacycles, switches))
    if len(results) > 0 and "renderer" in results[0]:
        first: dict = results[0]
        print()
        print("Window %g x %g logical at scale %g: %d x %d device pixels, %s MB per page. Renderer: %s." % (
            first["client_width"], first["client_height"], first["window_scale"],
            first["device_width"], first["device_height"], megabytes(float(first["page_bytes"])),
            first["renderer"]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
