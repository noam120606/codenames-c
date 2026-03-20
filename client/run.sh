#!/bin/sh
set -e

PORT=4242
SERVER_IP="${SERVER_IP:-172.18.41.26}"

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$SCRIPT_DIR"

make clean
make
exec ./build/client -s "$SERVER_IP" -p "$PORT"    