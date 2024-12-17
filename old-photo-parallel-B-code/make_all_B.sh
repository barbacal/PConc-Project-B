#!/bin/bash

clear;
make clean-all -C old-photo-parallel-B-code/;
make all -C old-photo-parallel-B-code/;
echo "Done clean all and build";
