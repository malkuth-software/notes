"""Run the page route probe once per route and keep each result line.

Usage:
    python3 experiments/001-page-routes/run_routes.py --probe .build/app/page_route_probe.exe \
        --output .build/routes/shadow-1060x680.jsonl [--turns 30] [--idle 10] [--width 1060 --height 680]

Each route opens the window, turns the page, idles and closes; keep the window
uncovered and the machine quiet meanwhile. The output file holds one JSON
object per line, in the order the routes ran. A route that printed no result
line is recorded as {"page_route_probe": {"route": name, "failed": text}}.
"""

import argparse
import json
from pathlib import Path
import subprocess

ROUTES: tuple[str, ...] = (
    "fill",
    "image-swap",
    "image-swap-nearest",
    "image-update",
    "image-patch",
    "image-strips",
    "image-pattern",
    "surface-painter",
    "surface-held",
    "surface-turn",
    "raster-canvas",
    "book-reading",
    "book-study",
)

RESULT_PREFIX: str = '{"page_route_probe":'


def run_route(probe: Path, route: str, arguments: argparse.Namespace) -> str:
    # An absolute path: process creation on Windows does not resolve a
    # relative one written with forward slashes.
    command: list[str] = [
        str(probe.resolve()),
        "--route", route,
        "--turns", str(arguments.turns),
        "--idle", str(arguments.idle),
        "--width", str(arguments.width),
        "--height", str(arguments.height),
    ]
    completed: subprocess.CompletedProcess[str] = subprocess.run(
        command, check=False, capture_output=True, text=True, encoding="utf-8", timeout=600)
    line: str = ""
    for line in completed.stdout.splitlines():
        if line.startswith(RESULT_PREFIX):
            return line
    failure: dict[str, dict[str, str]] = {
        "page_route_probe": {"route": route, "failed": completed.stderr.strip()}
    }
    text: str = json.dumps(failure)
    return text


def main() -> int:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--turns", type=int, default=30)
    parser.add_argument("--idle", type=int, default=10)
    parser.add_argument("--width", type=int, default=1060)
    parser.add_argument("--height", type=int, default=680)
    parser.add_argument("--route", action="append", default=[],
                        help="run only this route; may be given more than once")
    arguments: argparse.Namespace = parser.parse_args()

    routes: tuple[str, ...] = ROUTES
    if len(arguments.route) > 0:
        routes = tuple(arguments.route)
    arguments.output.parent.mkdir(parents=True, exist_ok=True)
    lines: list[str] = []
    route: str = ""
    for route in routes:
        line: str = run_route(arguments.probe, route, arguments)
        lines.append(line)
        print(line, flush=True)
    arguments.output.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
