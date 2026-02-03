#!/usr/bin/env bash
set -e

# =============================
# Configuration
# =============================
SDL_VERSION=2.30.0
SDL_IMAGE_VERSION=2.8.2
SDL_TTF_VERSION=2.22.0

ROOT_DIR="$(pwd)"
DEPS_DIR="$ROOT_DIR/client"
BUILD_DIR="$DEPS_DIR/build_SDL2"
INSTALL_DIR="$DEPS_DIR/SDL2"

# =============================
# Préparation des dossiers
# =============================
mkdir -p "$BUILD_DIR" "$INSTALL_DIR"
cd "$BUILD_DIR"

# =============================
# Fonction de téléchargement
# =============================
download () {
    if [ ! -f "$1" ]; then
        echo "Téléchargement de $1..."
        wget "$2"
    fi
}

# =============================
# Téléchargement
# =============================
download SDL2-$SDL_VERSION.tar.gz \
https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VERSION/SDL2-$SDL_VERSION.tar.gz

download SDL2_image-$SDL_IMAGE_VERSION.tar.gz \
https://github.com/libsdl-org/SDL_image/releases/download/release-$SDL_IMAGE_VERSION/SDL2_image-$SDL_IMAGE_VERSION.tar.gz

download SDL2_ttf-$SDL_TTF_VERSION.tar.gz \
https://github.com/libsdl-org/SDL_ttf/releases/download/release-$SDL_TTF_VERSION/SDL2_ttf-$SDL_TTF_VERSION.tar.gz

# =============================
# Extraction
# =============================
tar xzf SDL2-$SDL_VERSION.tar.gz
tar xzf SDL2_image-$SDL_IMAGE_VERSION.tar.gz
tar xzf SDL2_ttf-$SDL_TTF_VERSION.tar.gz

# =============================
# Compilation SDL2
# =============================
cd SDL2-$SDL_VERSION
./configure --prefix="$INSTALL_DIR"
make -j$(nproc)
make install
cd ..

# =============================
# Compilation SDL2_image
# =============================
cd SDL2_image-$SDL_IMAGE_VERSION
./configure \
  --prefix="$INSTALL_DIR" \
  --with-sdl-prefix="$INSTALL_DIR"
make -j$(nproc)
make install
cd ..

# =============================
# Compilation SDL2_ttf
# =============================
cd SDL2_ttf-$SDL_TTF_VERSION
./configure \
  --prefix="$INSTALL_DIR" \
  --with-sdl-prefix="$INSTALL_DIR"
make -j$(nproc)
make install
cd ..

rm -rf "$BUILD_DIR"

echo
echo "========================================"
echo " SDL2 installé localement avec succès ! "
echo "========================================"
