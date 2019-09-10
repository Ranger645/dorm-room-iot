#!/bin/bash

# Uploading all files in this directory to the server
rsync -r --exclude=server/config/. ./* pi@192.168.1.128:~/.
