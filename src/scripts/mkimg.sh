#!/bin/bash

dd if=/dev/zero of=ext2_hda.img bs=1k count=100000 2> /dev/null
fdisk ext2_hda.img << 'EOF'
x
c
10
h
16
s
63
r
o
n
p
1

102400
a
n
p
2


w
EOF