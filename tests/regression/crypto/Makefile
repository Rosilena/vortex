ROOT_DIR := $(realpath ../../..)
include $(ROOT_DIR)/config.mk

PROJECT := crypto

SRC_DIR := $(VORTEX_HOME)/tests/regression/$(PROJECT)

SRCS := $(SRC_DIR)/main.cpp

VX_SRCS := $(SRC_DIR)/kernel.cpp $(SRC_DIR)/aes.cpp

OPTS ?= -a aes-128-ecb -t 0 -n 1000

ifeq ($(XLEN), 64)
	CONFIGS += -DXLEN_64
	else
	CONFIGS += -DXLEN_32
endif

include ../common.mk