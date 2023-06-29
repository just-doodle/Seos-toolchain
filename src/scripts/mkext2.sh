#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# shellcheck source=/dev/null
source "$SCRIPT_DIR"/mkimg.sh ext2.img

# shellcheck source=/dev/null
source "$SCRIPT_DIR"/nbd.sh start ext2.img

sudo mkfs -t ext2 -i 1024 -b 1024 -F /dev/nbd0p1

# shellcheck source=/dev/null
source "$SCRIPT_DIR"/nbd.sh stop ext2.img