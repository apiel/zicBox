#!/bin/bash

make && sudo dfu-util -a 0 -D build/rack.bin -s 0x08000000:leave