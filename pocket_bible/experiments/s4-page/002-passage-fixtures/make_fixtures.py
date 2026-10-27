"""Cut the proving slice's three passages out of the pinned sources.

Usage:
    python3 experiments/002-passage-fixtures/make_fixtures.py --tool .build/core/page_extract_passage.exe \
        --sources <directory holding berean/ and stepbible/ as tools/fetch_sources.py leaves them> \
        --output assets/fixtures

The BSB's USFM files are read as unpacked from bsb_usfm.zip (unzip into
berean/extracted/). Every fixture is written by page_extract_passage; this
script only names the passages.
"""

import argparse
from pathlib import Path
import subprocess


def run_tool(tool: Path, arguments: list[str]) -> None:
    command: list[str] = [str(tool.resolve())] + arguments
    subprocess.run(command, check=True)


def main() -> int:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tool", type=Path, required=True)
    parser.add_argument("--sources", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    arguments: argparse.Namespace = parser.parse_args()

    usfm: Path = arguments.sources / "berean" / "extracted" / "bsb_usfm"
    archive: Path = arguments.sources / "berean" / "bsb_usfm.zip"
    step: Path = arguments.sources / "stepbible"
    output: Path = arguments.output
    output.mkdir(parents=True, exist_ok=True)

    run_tool(arguments.tool, ["usfm", "--file", str(usfm / "MRK.usfm"), "--archive", str(archive),
                              "--passage", "MRK 7:14-23", "--chapter", "7", "--first", "14", "--last", "23",
                              "--output", str(output / "mrk-7-14-23.bsb.fixture")])
    run_tool(arguments.tool, ["step", "--file", str(step / "TAGNT_Mat-Jhn.txt"),
                              "--passage", "MRK 7:14-23", "--prefix", "Mrk.7.", "--first", "14", "--last", "23",
                              "--output", str(output / "mrk-7-14-23.tagnt.fixture")])
    run_tool(arguments.tool, ["usfm", "--file", str(usfm / "GEN.usfm"), "--archive", str(archive),
                              "--passage", "GEN 1:1-13", "--chapter", "1", "--first", "1", "--last", "13",
                              "--output", str(output / "gen-1-1-13.bsb.fixture")])
    run_tool(arguments.tool, ["step", "--file", str(step / "TAHOT_Gen-Deu.txt"),
                              "--passage", "GEN 1:1-13", "--prefix", "Gen.1.", "--first", "1", "--last", "13",
                              "--output", str(output / "gen-1-1-13.tahot.fixture")])
    run_tool(arguments.tool, ["usfm", "--file", str(usfm / "PSA.usfm"), "--archive", str(archive),
                              "--passage", "PSA 87:1-7", "--chapter", "87", "--first", "1", "--last", "7",
                              "--output", str(output / "psa-87.bsb.fixture")])
    run_tool(arguments.tool, ["step", "--file", str(step / "TAHOT_Job-Sng.txt"),
                              "--passage", "PSA 87:1-7", "--prefix", "Psa.87.", "--first", "0", "--last", "7",
                              "--output", str(output / "psa-87.tahot.fixture")])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
