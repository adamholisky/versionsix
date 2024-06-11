
BUILD_LOG = $(ROOT_DIR)/build_support/logs/build.log
MOUNT_DIR = $(ROOT_DIR)/build_support/img_mount_point
MOUNT_IMG = vi_hd.img
SHELL :=/bin/bash -O globstar
VPATH = $(shell find ./ -type d -printf "%P:")