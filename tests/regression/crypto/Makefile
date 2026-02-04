ROOT_DIR := $(realpath ../../..)
include $(ROOT_DIR)/config.mk

PROJECT := crypto

SRC_DIR := $(VORTEX_HOME)/tests/regression/$(PROJECT)

SRCS := $(SRC_DIR)/main.cpp

VX_SRCS := $(SRC_DIR)/kernel.cpp $(VORTEX_HOME)/kernel/src/vx_perf.c $(SRC_DIR)/start.S

OPTS ?= -n256

CONFIGS = -DL1_DISABLE -DLMEM_DISABLE -DNUM_CORES=1 -DNUM_WARPS=1 -DNUM_THREADS=1 -DNUM_CLUSTER=1 -DNUM_SOCKETS=1

ifeq ($(XLEN), 64)
	CONFIGS += -DXLEN_64
endif

include ../common.mk

VX_LDFLAGS = -Wl,-Bstatic,--gc-sections,-T,$(VORTEX_HOME)/kernel/scripts/link$(XLEN).ld,--defsym=STARTUP_ADDR=$(STARTUP_ADDR)

VX_CC  = $(RISCV_TOOLCHAIN_PATH)/bin/$(RISCV_PREFIX)-gcc
VX_CXX = $(RISCV_TOOLCHAIN_PATH)/bin/$(RISCV_PREFIX)-g++
VX_DP  = $(RISCV_TOOLCHAIN_PATH)/bin/$(RISCV_PREFIX)-objdump
VX_CP  = $(RISCV_TOOLCHAIN_PATH)/bin/$(RISCV_PREFIX)-objcopy
