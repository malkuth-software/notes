#!/usr/bin/env python3
"""Scratch driver (hot-paths track, not committed): build the profile tree on
the M4 and record each workload with xctrace's Time Profiler.

  build [--tree NAME]                  configure and build the three workloads
  record WORKLOAD OUT [--tree NAME]    xctrace record one workload into OUT.trace
  time WORKLOAD [--tree NAME] [--repeat N]   run unsampled, print wall seconds
"""
from __future__ import annotations
import argparse
import subprocess
import sys
import time
from pathlib import Path

SOURCE: Path = Path(__file__).resolve().parents[1]
ROOT: Path = SOURCE.parent
sys.path.insert(0, str(SOURCE / 'tools'))
from toolkit_configure import host_options, prepare_host, toolkit_options  # noqa: E402
import profile_hot_paths  # noqa: E402

BUILD: Path = ROOT / '.build' / 'native-macos-arm64'


def build(tree_name: str) -> None:
    tree: Path = BUILD / tree_name
    (tree / 'CMakeCache.txt').unlink(missing_ok=True)
    prepare_host(SOURCE, True)
    configure: list[str] = ['cmake', '-S', str(SOURCE), '-B', str(tree), '-G', 'Ninja',
                            *toolkit_options(SOURCE, BUILD / (tree_name + '-sdk'), True, False),
                            *host_options('macos'), *profile_hot_paths.WORKLOAD_OPTIONS,
                            f'-DCMAKE_C_FLAGS={profile_hot_paths.PROFILE_FLAGS}',
                            # CMAKE_CXX_FLAGS would replace the toolchain's _INIT flags (the
                            # macOS 14 libc++ runtime); the Release flags carry the profile flags.
                            f'-DCMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG {profile_hot_paths.PROFILE_FLAGS}',
                            f'-DCMAKE_OBJCXX_FLAGS_RELEASE=-O3 -DNDEBUG {profile_hot_paths.PROFILE_FLAGS}']
    subprocess.run(configure, check=True, stdout=subprocess.DEVNULL)
    targets: list[str] = [w.target for w in profile_hot_paths.WORKLOADS]
    subprocess.run(['cmake', '--build', str(tree), '--parallel', '8', '--target', *targets], check=True)


def program(name: str, tree_name: str, output: Path) -> tuple[list[str], int]:
    for workload in profile_hot_paths.WORKLOADS:
        if workload.name == name:
            arguments: list[str] = [a.replace('{output}', str(output)).replace(
                '{fonts}', str(SOURCE / 'assets/fonts')) for a in workload.arguments]
            return [str(BUILD / tree_name / workload.target), *arguments], workload.runs
    raise SystemExit('unknown workload ' + name)


def record(name: str, out: Path, tree_name: str) -> None:
    out.parent.mkdir(parents=True, exist_ok=True)
    command: list[str]
    runs: int
    command, runs = program(name, tree_name, out.parent / (name + '-output'))
    trace: Path = out.with_suffix('.trace')
    subprocess.run(['rm', '-rf', str(trace)], check=True)
    if runs > 1:
        # Time Profiler follows only the launched process, so sample every
        # process while a shell loop runs the program; the summary filters by name.
        loop: str = f'i=0; while [ "$i" -lt {runs} ]; do "$0" "$@" > /dev/null || exit 1; i=$((i + 1)); done'
        runner = subprocess.Popen(['/bin/sh', '-c', loop, *command])
        subprocess.run(['xcrun', 'xctrace', 'record', '--template', 'Time Profiler', '--output', str(trace),
                        '--all-processes', '--time-limit', '15s'], check=True)
        runner.wait()
        return
    subprocess.run(['xcrun', 'xctrace', 'record', '--template', 'Time Profiler', '--output', str(trace),
                    '--target-stdout', '/dev/null', '--launch', '--', *command], check=True)


def timed(name: str, tree_name: str, repeat: int) -> None:
    command: list[str]
    runs: int
    output: Path = BUILD / tree_name / ('time-' + name)
    command, runs = program(name, tree_name, output)
    for attempt in range(repeat):
        started: float = time.perf_counter()
        for run in range(runs):
            subprocess.run(command, check=True, stdout=subprocess.DEVNULL)
        print(f'{name} attempt {attempt}: {time.perf_counter() - started:.3f} s', flush=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('command')
    parser.add_argument('workload', nargs='?')
    parser.add_argument('out', nargs='?')
    parser.add_argument('--tree', default='gui-forms-profile')
    parser.add_argument('--repeat', type=int, default=3)
    arguments = parser.parse_args()
    if arguments.command == 'build':
        build(arguments.tree)
    elif arguments.command == 'record':
        record(arguments.workload, Path(arguments.out).resolve(), arguments.tree)
    elif arguments.command == 'time':
        timed(arguments.workload, arguments.tree, arguments.repeat)
    else:
        raise SystemExit('unknown command')
    return 0


if __name__ == '__main__':
    sys.exit(main())
