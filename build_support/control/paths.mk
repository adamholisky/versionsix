
BUILD_LOG = $(ROOT_DIR)/build_support/logs/build.log
MOUNT_DIR = $(ROOT_DIR)/build_support/img_mount_point
KERNEL_BOOT_IMG = boot_drive.img
ASVFS_DATA_DRIVE = asvfs_drive.img
SHELL :=/bin/bash -O globstar
VPATH = $(shell find ./kernel -type d -printf "kernel/%P:")