ROOT_DIR := $(realpath ../../..)
include $(ROOT_DIR)/config.mk

PROJECT := gcm_enabled

SRC_DIR := $(VORTEX_HOME)/tests/regression/$(PROJECT)

SRCS := $(SRC_DIR)/main.cpp

VX_SRCS := $(SRC_DIR)/kernel.cpp 

OPTS ?= -n64

ifeq ($(XLEN), 64)
	CONFIGS += -DXLEN_64
	else
	CONFIGS += -DXLEN_32
endif

include ../common.mk