#!/bin/sh

cmake -S . -B build && \
cmake --build build -j16 --config Release && \
./build/gvizdoom_client
