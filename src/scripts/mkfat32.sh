#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# shellcheck source=/dev/null
source "$SCRIPT_DIR"/mkimg.sh fat32.img

# shellcheck source=/dev/null
source "$SCRIPT_DIR"/nbd.sh start fat32.img

sudo mkfs.fat -F32 /dev/nbd0p1 -n SEOS

# shellcheck source=/dev/null
source "$SCRIPT_DIR"/nbd.sh stop fat32.img