#!/bin/bash
qemu-system-i386 -S -s -boot a -fda a.img &
echo "At the gdb prompt enter \"target remote localhost:1234\""

ddd boot2.exe &


