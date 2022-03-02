#!/bin/bash

/home/user/qemu/build/qemu-system-aarch64 -M arm-generic-fdt \
  -serial mon:stdio \
  -device loader,file=./loader.img,addr=0x40000000,cpu-num=0 \
  -device loader,addr=0xfd1a0104,data=0x8000000e,data-len=4 \
  -hw-dtb /home/user/qemu-devicetrees/LATEST/SINGLE_ARCH/zcu102-arm.dtb \
  -m 4096 \
  -display none
