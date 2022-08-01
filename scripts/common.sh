# Helper functions
LOG () {
  echo "[LOG] $1" 
}

# global variables
BUSYBOX_VERSION=busybox-1.35.0
BUSYBOX=busybox-x86

TOP=`pwd`
DOWNLOADS=$TOP/downloads
BUILDS=$TOP/builds
TEMPLATES=$TOP/templates
INITRAMFS=$BUILDS/initramfs/x86-busybox
