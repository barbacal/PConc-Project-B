#!/bin/bash

clear;
make clean-all -C old-photo-pipeline-code/;
make all -C old-photo-pipeline-code/;
echo "Done run all test.";
