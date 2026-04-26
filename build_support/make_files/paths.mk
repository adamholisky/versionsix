BUILD_LOG = $(ROOT_DIR)/logs/build.log
MOUNT_DIR = $(ROOT_DIR)/build_support/img_mount_point
BUILD_NUMBER_FILE = $(ROOT_DIR)/build_support/build_number
KERNEL_BOOT_IMG = boot_drive.img
ASVFS_DATA_DRIVE = asvfs_drive.img
SHELL :=/bin/bash -O globstar
VPATH = $(shell find ./kernel -type d -printf "kernel/%P:")