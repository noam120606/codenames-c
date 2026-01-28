#!/bin/bash


# Installation SDL2
if [ ! -d "client/SDL" ]; then
    cd client
    git clone https://github.com/libsdl-org/SDL.git -b SDL2
    cd SDL
    mkdir build
    cd build
    ../configure
    make
    make install
    cd ../../../
fi

# Instalation cJSON


# Compilation des fichiers
make -C server
make -C client
