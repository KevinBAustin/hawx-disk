#!/bin/bash
sudo apt update
sudo apt-get install -y git build-essential gdb-multiarch qemu-system-misc gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu 
mkdir -p /home/codespace/.config/gdb 
echo 'set auto-load safe-path /' > /home/codespace/.config/gdb/gdbinit