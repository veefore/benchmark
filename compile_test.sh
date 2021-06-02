#!/bin/sh

g++ main.cpp benchmark.cpp api.cpp globals.cpp test.cpp -o run -std=c++17 -g
