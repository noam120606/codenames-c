#!/bin/sh
set -e

SERVER_IP=""
PORT=4242
FPS=60
INSTANCES=0
NO_BUILD=0
PIDS=""

print_help() {
	cat <<'EOF'
Usage: ./run_with_params.sh [options]

Options:
  -s, --server-ip IP   Address IP du serveur (fallback: 127.0.0.1)
  -n, --instances N    Nombre de clients (fallback: 1)
  -p, --port PORT      Port du serveur (default: 4242)
  -f, --fps FPS        Frames per second (default: 60)
	  --no-build       Skip compilation step
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
		-n|--instances)
			if [ "$#" -lt 2 ]; then
				echo "Missing value for $1" >&2
				exit 1
			fi
			INSTANCES="$2"
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
		-f|--fps)
			if [ "$#" -lt 2 ]; then
				echo "Missing value for $1" >&2
				exit 1
			fi
			FPS="$2"
			shift 2
			;;
		--no-build)
			NO_BUILD=1
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

if [ -z "$SERVER_IP" ]; then
	printf "Entrez l'adresse IP du serveur (par defaut: 127.0.0.1): "
	read -r SERVER_IP
fi
SERVER_IP="${SERVER_IP:-127.0.0.1}"

if [ "$INSTANCES" -le 0 ] 2>/dev/null; then
	printf "Nombre de clients a lancer (par defaut: 1): "
	read -r INSTANCES
fi
INSTANCES="${INSTANCES:-1}"

case "$INSTANCES" in
	''|*[!0-9]*)
		echo "Le nombre de clients doit etre un entier superieur ou egal a 1." >&2
		exit 1
		;;
esac

if [ "$INSTANCES" -lt 1 ]; then
	echo "Le nombre de clients doit etre un entier superieur ou egal a 1." >&2
	exit 1
fi

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$SCRIPT_DIR"

if [ "$NO_BUILD" -eq 0 ]; then
	echo "Compilation unique avant lancement multi-clients..."
	"$SCRIPT_DIR/run.sh" --server-ip "$SERVER_IP" --port "$PORT" --fps "$FPS" --build-only
elif [ ! -x ./build/client ]; then
	echo "No compiled client found at ./build/client. Run once without --no-build." >&2
	exit 1
fi

i=1
while [ "$i" -le "$INSTANCES" ]; do
	echo "Lancement du client $i/$INSTANCES..."
	"$SCRIPT_DIR/run.sh" --server-ip "$SERVER_IP" --port "$PORT" --fps "$FPS" --no-build >/dev/null 2>&1 &
	PIDS="$PIDS $!"
	i=$((i + 1))
done

echo "$INSTANCES client(s) lance(s)."

cleanup() {
	echo "Arret des clients..."
	for pid in $PIDS; do
		kill "$pid" >/dev/null 2>&1 || true
	done
}

trap cleanup INT TERM

for pid in $PIDS; do
	wait "$pid"
done