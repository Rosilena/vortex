This folder is used to generate a Vivado Project for the AMD Ultrascale ZCU102 board, 
where the Vortex GPU is istantiated in the processing logic part of the SoC.

The block diagram is composed of the Processing System part, a BRAM and BRAM Controller block (16MB total)
that are used as the main memory for the Vortex GPU.

The clock for the GPU is obtained from the processing logic and runs at 100MHz.

The maximum configuration size for the system that can fit on the device is:

2KByte ICache and DCcache
512B LMEM
2 Cores, 4 Threads, 4 Warps
RISC-V Cryprographic Extensions ON