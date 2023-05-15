#!/bin/bash

rm sorhd sorhd.vdi || true
make
VBoxManage convertdd sorhd sorhd.vdi --format VDI
vboxmanage internalcommands sethduuid ./sorhd.vdi 29a6336b-b33e-4c32-81e6-2951002bb4a4

vboxmanage startvm SEOS -E VBOX_GUI_DBG_ENABLED=true