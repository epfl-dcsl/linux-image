#!/bin/bash

# Helper functions
source `pwd`/scripts/common.sh

LINUX_ROOT=$TOP/linux
IMAGES=images

# Crash on errors
set -e

build_linux_embedded() {
  LOG "Building embedded ramfs Linux"
  
  target=$BUILDS/linux-tyche-embedded
  # Copy the build template into the linux folder and replace the FS variable.
  cp $TEMPLATES/tyche_linux_embedded_ramfs $LINUX_ROOT/arch/x86/configs/tyche_linux_embedded_ramfs_defconfig
  sed -i "s|INIT_FS_ROOT|$BUILDS\/initramfs\/x86-busybox|g" $LINUX_ROOT/arch/x86/configs/tyche_linux_embedded_ramfs_defconfig

  make -C $LINUX_ROOT O=$target tyche_linux_embedded_ramfs_defconfig
  make -C $LINUX_ROOT O=$target -j `nproc`

  # cleanup
  rm $LINUX_ROOT/arch/x86/configs/tyche_linux_embedded_ramfs_defconfig

  LOG "Done building linux embedded in $target"
  LOG "Copying files"

  cp $target/vmlinux $IMAGES/ 
  cp $target/vmlinux-gdb.py $IMAGES/
  LOG "Done copying result"
}

build_linux_embedded
