#!/bin/sh

g++ main.cpp benchmark.cpp api.cpp globals.cpp -o run -std=c++17 -g -pthread
