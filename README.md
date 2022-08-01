# Linux-image repository

## Description

This repository holds an pre-compiled image of the linux kernel with an embedded ramfs and is used by tyche as the default dom0 guest.

Our ramfs is based on busybox version 1.35.0. 

To correctly clone this repository use 
```
git clone --recursive {https/ssh link to the repo}
```

and to update submodules if you already cloned without submodules

```
git submodule update --init --recursive
```

## Map

For convenience, we pre-compiled the linux image and track it using `git lfs`.
However, we also provide the scripts to reproduce the entire build process: 

* `Makefile` has a single default target that calls our `scripts/build.sh` script to generate our linux image and copy it into the `images` folder. The secondary target `run-linux-embedded` runs the vmlinux image with qemu.
* `images` holds our compiled linux image with the embedded ramfs.
* `downloads` keeps track of the downloaded busybox version.
* `templates` contains:
  * `busybox-config` to configure and compile busybox.
  * `init_template` to generate our ramfs from the busybox build.
  * `tyche_linux_embedded_ramfs` to build the linux kernel with our embedded ramfs.
* `scripts/setup.sh` contains the bash script that downloads and builds busybox. It is never called and serves as a reference.
* `scripts/build.sh` is called by our makefile to recompile the linux kernel and update the `images/vmlinux` image. IT SHOULD NOT BE CALLED manually.
* `linux` is a submodule clone of the linux kernel set at version 5.10.


