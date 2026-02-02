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
    cd ../../

    #SDL2 image
    git clone https://github.com/libsdl-org/SDL_image.git -b SDL2
    cd SDL_image
    mkdir build
    cd build
    ../configure
    make
    make install
    cd ../../

    #SDL2 ttf
    git clone https://github.com/libsdl-org/SDL_ttf.git -b SDL2
    cd SDL_ttf
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
