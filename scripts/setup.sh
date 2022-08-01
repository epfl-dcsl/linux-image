#!/bin/bash


 # This setup script is here for convenience and is never called by our makefile.
 # It keeps track of how each element required in order to build a linux kernel
 # with an embedded initramfs based on busybox is downloaded and compiled.

# Helper functions
source `pwd`/scripts/common.sh

mkdir -p $DOWNLOADS
mkdir -p $BUILDS

# Exit upon failure
set -e

download_busybox() {
  # Already have a busybox downloaded.
  if [ -d $BUILDS/$BUSYBOX ]; then
    LOG "$BUILDS/$BUSYBOX already exists. Delete if you want a fresh install."
  fi
  # Check if we have a local version, otherwise download. 
  if [ ! -f $DOWNLOADS/$BUSYBOX_VERSION.tar.bz2 ]; then
    LOG "Downloading busybox"
    cd $DOWNLOADS 
    curl https://busybox.net/downloads/$BUSYBOX_VERSION.tar.bz2 | tar xjf -
    cd $TOP
  fi
}

# Builds the busybox image. 
build_busybox() {
  target=$BUILDS/$BUSYBOX
  LOG "Building busybox in $target"
  if [ -d $target ]; then
    LOG "Busybox build already exists in $target. Remove it for fresh build"
    exit 1
  fi
  mkdir -pv $target
  cp $TEMPLATES/busybox-config $target/.config 
  make -C $DOWNLOADS/$BUSYBOX_VERSION O=$target
  make -C $target -j 4
  make -C $target install
  LOG "Done compiling busybox"
}

repack_ramfs() {
  target=$INITRAMFS 
  if [ ! -d $target ]; then
    LOG "Initramfs not found in $target"
    exit 1
  fi
  cd $INITRAMFS
  find . | cpio -H newc -o > ../initramfs.cpio
  cd ..
  cat initramfs.cpio | gzip > initramfs.igz
  cd $TOP
  LOG "Repacked initramfs"
}

build_initramfs() {
  LOG "Building initramfs"
  target=$INITRAMFS
  src=$BUILDS/$BUSYBOX
  if [ ! -d $src ]; then
    LOG "Now busybox build found. Please reinstall at $src"
    exit 1
  fi
  mkdir -pv $target
  mkdir -pv $target/{bin,dev,sbin,etc,proc,sys/kernel/debug,usr/{bin,sbin},lib,lib64,mnt/root,root} 
  cp -av $src/_install/* $target/
  
  LOG "Next command requires sudo access rights"
  sudo cp -av /dev/{null,console,tty} $target/dev/

  cp $TEMPLATES/init_template $target/init
  chmod +x $target/init
  LOG "Busybox initramfs created, about to repack"
  repack_ramfs
}

