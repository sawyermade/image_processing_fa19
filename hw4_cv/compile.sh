#!/bin/bash
[ ! -d ./output ] && mkdir output
[ -f ./hw4 ] && rm hw4
g++ hw4.cpp `pkg-config --cflags --libs opencv` -o hw4
