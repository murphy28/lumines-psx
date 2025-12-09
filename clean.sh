#!/bin/bash

find src -name "*.o" -type f -delete
find src -name "*.map" -type f -delete
find src -name "*.dep" -type f -delete

find . -name "*.elf" -type f -delete
find . -name "*.ps-exe" -type f -delete


echo "All clean!"
