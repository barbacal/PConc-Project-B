#!/bin/bash

clear;
make clean-all -C old-photo-parallel-A/;
make all -C old-photo-parallel-A/;
echo "Done run all test.";
