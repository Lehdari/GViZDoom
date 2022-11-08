#!/bin/sh

cmake -S . -B build && \
cmake --build build -j --config Release && \
./build/gzdoom