#!/bin/sh

g++ main.cpp benchmark.cpp api.cpp globals.cpp io.cpp experimenter.cpp -o run -std=c++17 -g
