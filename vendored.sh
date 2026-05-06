#!/usr/bin/env bash

mkdir vendored
cd vendored
git submodule add https://github.com/libsdl-org/SDL
git submodule add https://github.com/libsdl-org/SDL_image
git submodule add https://github.com/libsdl-org/SDL_ttf

