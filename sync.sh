#!/bin/bash

rsync --exclude .git/ --exclude samples/tape_tmp/ --exclude **/build/ --exclude pixel.arm --exclude **/libzicHost.* -avu --delete ~/remoteZicBox/ ~/localZicBox
