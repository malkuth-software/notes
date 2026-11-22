#!/bin/sh
# Mirror this experiment and the bundled fonts to the M4 and run a command there.
# Usage: sh m4.sh '<command run in the mirror root>'
set -e
here=$(cd "$(dirname "$0")" && pwd)
host=$("$HOME/.local/bin/m4host")
dest=Developer/CodexBuilds/font-pack-study
rsync -a --delete --exclude=/build --exclude=/fonts --exclude=__pycache__ "$here/" "$host:$dest/"
rsync -a --delete --include='*.ttf' --include='*.otf' --exclude='*' "$here/../../assets/fonts/" "$host:$dest/fonts/"
ssh "$host" "cd $dest && mkdir -p build && export PATH=/opt/homebrew/opt/llvm@22/bin:/opt/homebrew/bin:/usr/bin:/bin && $1"
