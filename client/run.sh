#!/bin/sh
set -e

PORT=4242
SERVER_IP="${SERVER_IP:-127.0.0.1}"
NO_BUILD=0
BUILD_ONLY=0

print_help() {
	cat <<'EOF'
Usage: ./run.sh [options]

Options:
  -s, --server-ip IP   Server IP address (default: 127.0.0.1)
  -p, --port PORT      Server port (default: 4242)
	  --no-build       Skip compilation
	  --build-only     Compile only, do not launch client
  -h, --help           Show this help
EOF
}

while [ "$#" -gt 0 ]; do
	case "$1" in
		-s|--server-ip)
			if [ "$#" -lt 2 ]; then
				echo "Missing value for $1" >&2
				exit 1
			fi
			SERVER_IP="$2"
			shift 2
			;;
		-p|--port)
			if [ "$#" -lt 2 ]; then
				echo "Missing value for $1" >&2
				exit 1
			fi
			PORT="$2"
			shift 2
			;;
		--no-build)
			NO_BUILD=1
			shift
			;;
		--build-only)
			BUILD_ONLY=1
			shift
			;;
		-h|--help)
			print_help
			exit 0
			;;
		*)
			echo "Unknown option: $1" >&2
			print_help >&2
			exit 1
			;;
	esac
done

if [ "$NO_BUILD" -eq 1 ] && [ "$BUILD_ONLY" -eq 1 ]; then
	echo "Cannot use --build-only with --no-build." >&2
	exit 1
fi

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$SCRIPT_DIR"

if [ "$NO_BUILD" -eq 0 ]; then
	make
elif [ ! -x ./build/client ]; then
	echo "No compiled client found at ./build/client. Run once without --no-build." >&2
	exit 1
fi

if [ "$BUILD_ONLY" -eq 1 ]; then
	echo "Client compiled successfully (build-only mode)."
	exit 0
fi

exec ./build/client -s "$SERVER_IP" -p "$PORT"