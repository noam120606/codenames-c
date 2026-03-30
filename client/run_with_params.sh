#!/bin/sh
set -e

PORT=4242
# Demander à l'utilisateur de saisir l'adresse IP du serveur
read -p "Entrez l'adresse IP du serveur (par défaut: 127.0.0.1): " SERVER_IP
SERVER_IP="${SERVER_IP:-127.0.0.1}"


SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$SCRIPT_DIR"

make
exec ./build/client -s "$SERVER_IP" -p "$PORT"