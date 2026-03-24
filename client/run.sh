#!/bin/sh
set -e

PORT=4242
SERVER_IP="${SERVER_IP:-127.0.0.1}"

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$SCRIPT_DIR"

make
exec ./build/client -s "$SERVER_IP" -p "$PORT"