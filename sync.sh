#!/bin/bash

rsync --exclude .git/ --exclude samples/tape_tmp/ --exclude **/build/ -avu --delete ~/remoteZicBox/ ~/localZicBox
