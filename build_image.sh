#!/bin/bash

bximage -fd -size=1.44 -q a.img

mformat a:
dd if=boot1 of=a.img bs=1 count=512 conv=notrunc
mcopy -o boot2 a:BOOT2

