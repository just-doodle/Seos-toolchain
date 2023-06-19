#!/bin/bash

rm sorhd.raw || true
VBoxManage internalcommands converttoraw sorhd.vdi sorhd.raw
./sorfs -e output sorhd.raw