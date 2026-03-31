#!/bin/sh
set -e

PORT=4242
SERVER_IP=""
INSTANCES=0
NO_BUILD=0
LAUNCHED=0
PIDS=""

print_help() {
	cat <<'EOF'
Usage: ./run_with_params.sh [options]

Options:
  -s, --server-ip IP   Address IP du serveur (fallback: 127.0.0.1)
  -n, --instances N    Nombre de clients (fallback: 1)
  -p, --port PORT      Port du serveur (default: 4242)
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
	"$SCRIPT_DIR/run.sh" --server-ip "$SERVER_IP" --port "$PORT" --build-only
elif [ ! -x ./build/client ]; then
	echo "No compiled client found at ./build/client. Run once without --no-build." >&2
	exit 1
fi

launch_in_terminal() {
	if command -v x-terminal-emulator >/dev/null 2>&1; then
		x-terminal-emulator -e "$SCRIPT_DIR/run.sh" --server-ip "$SERVER_IP" --port "$PORT" --no-build >/dev/null 2>&1 &
		return 0
	fi

	if command -v gnome-terminal >/dev/null 2>&1; then
		gnome-terminal -- "$SCRIPT_DIR/run.sh" --server-ip "$SERVER_IP" --port "$PORT" --no-build >/dev/null 2>&1 &
		return 0
	fi

	if command -v konsole >/dev/null 2>&1; then
		konsole -e "$SCRIPT_DIR/run.sh" --server-ip "$SERVER_IP" --port "$PORT" --no-build >/dev/null 2>&1 &
		return 0
	fi

	if command -v xfce4-terminal >/dev/null 2>&1; then
		xfce4-terminal -e "$SCRIPT_DIR/run.sh --server-ip $SERVER_IP --port $PORT --no-build" >/dev/null 2>&1 &
		return 0
	fi

	if command -v xterm >/dev/null 2>&1; then
		xterm -e "$SCRIPT_DIR/run.sh" --server-ip "$SERVER_IP" --port "$PORT" --no-build >/dev/null 2>&1 &
		return 0
	fi

	return 1
}

i=1
while [ "$i" -le "$INSTANCES" ]; do
	echo "Lancement du client $i/$INSTANCES..."
	if launch_in_terminal; then
		LAUNCHED=1
	else
		# Fallback pour les environnements sans terminal graphique: lancer en arrière-plan avec nohup
		nohup ./build/client -s "$SERVER_IP" -p "$PORT" >"$SCRIPT_DIR/build/client_${i}.log" 2>&1 &
		PIDS="$PIDS $!"
	fi
	i=$((i + 1))
done

echo "$INSTANCES client(s) lance(s)."

if [ "$LAUNCHED" -eq 0 ]; then
	echo "Aucun emulateur de terminal detecte: clients lances en arriere-plan (nohup)."
	echo "PIDs:$PIDS"
	echo "Logs: $SCRIPT_DIR/build/client_*.log"
fi