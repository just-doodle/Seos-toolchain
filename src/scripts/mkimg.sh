#!/bin/bash

dd if=/dev/zero of=$1 bs=1k count=100000 2> /dev/null
fdisk $1 << 'EOF'
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