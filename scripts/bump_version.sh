#!/usr/bin/env sh
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
VERSION_FILE="$ROOT_DIR/VERSION"

if [ ! -f "$VERSION_FILE" ]; then
    printf '%s\n' "0.1.0" > "$VERSION_FILE"
fi

current_version="$(tr -d ' \t\r\n' < "$VERSION_FILE")"
IFS='.' read -r major minor patch <<EOF
$current_version
EOF

is_number() {
    case "$1" in
        ''|*[!0-9]*) return 1 ;;
        *) return 0 ;;
    esac
}

if ! is_number "${major:-}" || ! is_number "${minor:-}" || ! is_number "${patch:-}"; then
    echo "Invalid VERSION format: '$current_version' (expected MAJOR.MINOR.PATCH)" >&2
    exit 1
fi

patch=$((patch + 1))
new_version="$major.$minor.$patch"
printf '%s\n' "$new_version" > "$VERSION_FILE"

echo "Bumped VERSION to $new_version"
