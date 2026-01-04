ROOT_DIR := $(realpath ../..)
include $(ROOT_DIR)/config.mk

all:
	$(MAKE) -C blinky
	$(MAKE) -C conform
	$(MAKE) -C crypto
	$(MAKE) -C hello	
	$(MAKE) -C fibonacci
	$(MAKE) -C vecadd	

run-simx:
	$(MAKE) -C blinky run-simx
	$(MAKE) -C conform run-simx
	$(MAKE) -C crypto run-simx
	$(MAKE) -C hello run-simx
	$(MAKE) -C fibonacci run-simx	
	$(MAKE) -C vecadd run-simx

run-rtlsim:
	$(MAKE) -C blinky run-rtlsim
	$(MAKE) -C conform run-rtlsim
	$(MAKE) -C crypto run-rtlsim
	$(MAKE) -C hello run-rtlsim
	$(MAKE) -C fibonacci run-rtlsim
	$(MAKE) -C vecadd run-rtlsim
	
clean:
	$(MAKE) -C blinky clean
	$(MAKE) -C conform clean
	$(MAKE) -C crypto clean
	$(MAKE) -C hello clean
	$(MAKE) -C fibonacci clean
	$(MAKE) -C vecadd clean
