#!/bin/bash

rsync --exclude .git/ --exclude tape/ --exclude **/build/ -avu --delete ~/remoteZicBox ~/localZicBox
