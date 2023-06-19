#!/bin/bash

rm sorhd sorhd.vdi ext2.vdi || true
make
VBoxManage convertdd sorhd sorhd.vdi --format VDI
vboxmanage internalcommands sethduuid ./sorhd.vdi 29a6336b-b33e-4c32-81e6-2951002bb4a4

VBoxManage convertdd ext2_hda.img ext2.vdi --format VDI
vboxmanage internalcommands sethduuid ./ext2.vdi 29a6336b-b33e-4c32-81e6-2951002bbeef

(killall vboxmanage && sleep 1) || true
vboxmanage startvm SEOS -E VBOX_GUI_DBG_ENABLED=true

# VBoxManage internalcommands converttoraw sorhd.vdi sorhd.raw