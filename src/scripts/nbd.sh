#!/bin/bash

case $1 in
    start)
        sudo modprobe nbd
        udevadm settle
        sudo qemu-nbd -c /dev/nbd0 $2 --format raw
	    udevadm settle
        ;;
    stop)
        sudo qemu-nbd -d /dev/nbd0
        sudo modprobe -r nbd || true
	    udevadm settle
        ;;
esac