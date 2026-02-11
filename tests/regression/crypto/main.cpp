#include <iostream>
#include <unistd.h>
#include <string.h>
#include <vortex.h>
#include <chrono>
#include <vector>
#include "common.h"

#define NONCE  0xdeadbeef

#define RT_CHECK(_expr)                                         \
   do {                                                         \
     int _ret = _expr;                                          \
     if (0 == _ret)                                             \
       break;                                                   \
     printf("Error: '%s' returned %d!\n", #_expr, (int)_ret);   \
	 cleanup();			                                              \
     exit(-1);                                                  \
   } while (false)

///////////////////////////////////////////////////////////////////////////////

const char* kernel_file = "kernel.vxbin";
int test = -1;
int num_instr = 47;
test_type count = 0;

vx_device_h device = nullptr;
vx_buffer_h src0_buffer = nullptr;
vx_buffer_h src1_buffer = nullptr;
vx_buffer_h dst_buffer = nullptr;
vx_buffer_h krnl_buffer = nullptr;
vx_buffer_h args_buffer = nullptr;
kernel_arg_t kernel_arg = {};

static void show_usage() {
   std::cout << "Vortex Test for Cryptographic Extensions...." << std::endl;
   std::cout << "Usage: [-t testno][-k: kernel][-n words][-h: help]" << std::endl;
}

static void parse_args(int argc, char **argv) {
  int c;
  while ((c = getopt(argc, argv, "n:t:k:h")) != -1) {
    switch (c) {
    case 'n':
      count = atoi(optarg);
      break;
    case 't':
      test = atoi(optarg);
      break;
    case 'k':
      kernel_file = optarg;
      break;
    case 'h':
      show_usage();
      exit(0);
      break;
    default:
      show_usage();
      exit(-1);
    }
  }
}

void cleanup() {
  if (device) {
    vx_mem_free(src0_buffer);
    vx_mem_free(src1_buffer);
    vx_mem_free(dst_buffer);
    vx_mem_free(krnl_buffer);
    vx_mem_free(args_buffer);
    vx_dev_close(device);
  }
}


int run_kernel_test(const kernel_arg_t& kernel_arg) {
  
  test_type buf_size = num_instr * sizeof(test_type);
  
  std::vector<test_type> h_src0(1);
  std::vector<test_type> h_src1(1); 
  std::vector<test_type> h_dst(num_instr);

  h_src0[0] = (test_type) 0xFFAA; //First operand
  h_src1[0] = (test_type) 0x1; //Second operand
  h_dst[0]  = (test_type) 0x0;

  // Upload kernel binary
  std::cout << "Upload kernel binary" << std::endl;
  RT_CHECK(vx_upload_kernel_file(device, kernel_file, &krnl_buffer));

  // upload kernel argument
  std::cout << "upload kernel argument" << std::endl;
  RT_CHECK(vx_upload_bytes(device, &kernel_arg, sizeof(kernel_arg_t), &args_buffer));

  // upload source buffer0
  std::cout << "upload source buffer0" << std::endl;
  RT_CHECK(vx_copy_to_dev(src0_buffer, h_src0.data(), 0, sizeof(test_type)));

  // upload source buffer1
  std::cout << "upload source buffer1" << std::endl; 
  RT_CHECK(vx_copy_to_dev(src1_buffer, h_src1.data(), 0, sizeof(test_type)));

  auto time_start = std::chrono::high_resolution_clock::now();

  
  // start device
  std::cout << "start execution" << std::endl;
  auto t2 = std::chrono::high_resolution_clock::now();
  RT_CHECK(vx_start(device, krnl_buffer, args_buffer));
  RT_CHECK(vx_ready_wait(device, VX_MAX_TIMEOUT));
  auto t3 = std::chrono::high_resolution_clock::now();

  // download destination buffer
  std::cout << "read destination buffer from local memory" << std::endl;
  auto t4 = std::chrono::high_resolution_clock::now();
  RT_CHECK(vx_copy_from_dev(h_dst.data(), dst_buffer, 0, buf_size));
  auto t5 = std::chrono::high_resolution_clock::now();

  // verify result
  int errors = 0;
  std::cout << "verify result" << std::endl;
  std::cout << "OPERANDS: " << std::hex << "\n";
  std::cout << "ROR = " << std::hex << h_dst[0] << std::endl;
  std::cout << "ROL = " << std::hex << h_dst[1] << std::endl;
  std::cout << "RORI  = " << std::hex << h_dst[2] << std::endl;
  std::cout << "PACK = " << std::hex << h_dst[3] << std::endl;
  std::cout << "PACKH = " << std::hex << h_dst[4] << std::endl;
  std::cout << "BREV8  = " << std::hex << h_dst[5] << std::endl;
  std::cout << "REV8  = " << std::hex << h_dst[6] << std::endl;
  std::cout << "CLMUL  = " << std::hex << h_dst[7] << std::endl;
  std::cout << "CLMULH  = " << std::hex << h_dst[8] << std::endl;
  std::cout << "XPERM8  = " << std::hex << h_dst[9] << std::endl;
  std::cout << "XPERM4  = " << std::hex << h_dst[10] << std::endl;
  std::cout << "SHA256SIG0  = " << std::hex << h_dst[11] << std::endl;
  std::cout << "SHA256SIG1  = " << std::hex << h_dst[12] << std::endl;
  std::cout << "SHA256SUM0  = " << std::hex << h_dst[13] << std::endl;
  std::cout << "SHA256SUM1  = " << std::hex << h_dst[14] << std::endl;

  #ifdef XLEN_64
    std:: cout << "RORW = " << std::hex << h_dst[15] << std::endl;
    std:: cout << "ROLW = " << std::hex << h_dst[16] << std::endl;
    std:: cout << "RORIW  = " << std::hex << h_dst[17] << std::endl;
    std:: cout << "PACKW  = " << std::hex << h_dst[18] << std::endl;
    std:: cout << "AES64DS  = " << std::hex << h_dst[19] << std::endl;
    std:: cout << "AES64DSM  = " << std::hex << h_dst[20] << std::endl;
    std:: cout << "AES64IM  = " << std::hex << h_dst[21] << std::endl;
    std:: cout << "AES64KS1I  = " << std::hex << h_dst[22] << std::endl;
    std:: cout << "AES64KS2  = " << std::hex << h_dst[23] << std::endl;
    std:: cout << "AES64ES  = " << std::hex << h_dst[24] << std::endl;
    std:: cout << "AES64ESM  = " << std::hex << h_dst[25] << std::endl;
    std:: cout << "SHA512SIG0  = " << std::hex << h_dst[26] << std::endl;
    std:: cout << "SHA512SIG1  = " << std::hex << h_dst[27] << std::endl;
    std:: cout << "SHA512SUM0  = " << std::hex << h_dst[28] << std::endl;
    std:: cout << "SHA512SUM1  = " << std::hex << h_dst[29] << std::endl;
  #else
    std::cout << "ZIP  = " << std::hex << h_dst[15] << std::endl;
    std::cout << "UNZIP  = " << std::hex << h_dst[16] << std::endl;
    std::cout << "AES32DSI  = " << std::hex << h_dst[17] << std::endl;
    std::cout << "AES32DSMI  = " << std::hex << h_dst[18] << std::endl;
    std::cout << "AES32ESI  = " << std::hex << h_dst[19] << std::endl;
    std::cout << "AES32ESMI = " << std::hex << h_dst[20] << std::endl;
    std::cout << "SHA512SIG0H  = " << std::hex << h_dst[21] << std::endl;
    std::cout << "SHA512SIG0L  = " << std::hex << h_dst[22] << std::endl;
    std::cout << "SHA512SIG1H  = " << std::hex << h_dst[23] << std::endl;
    std::cout << "SHA512SIG1L  = " << std::hex << h_dst[24] << std::endl;
    std::cout << "SHA512SUM0R  = " << std::hex << h_dst[25] << std::endl;
    std::cout << "SHA512SUM1R  = " << std::hex << h_dst[26] << std::endl;
  #endif

  auto time_end = std::chrono::high_resolution_clock::now();

  double elapsed;
  elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
  printf("execute time: %lg ms\n", elapsed);
  elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4).count();
  printf("download time: %lg ms\n", elapsed);
  elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
  printf("Total elapsed time: %lg ms\n", elapsed);

  return errors;
}

int main(int argc, char *argv[]) {
  // parse command arguments
  parse_args(argc, argv);

  if (count == 0) {
    count = 1;
  }

  // open device connection
  std::cout << "open device connection" << std::endl;
  RT_CHECK(vx_dev_open(&device));

  uint64_t num_cores;
  RT_CHECK(vx_dev_caps(device, VX_CAPS_NUM_CORES, &num_cores));

  test_type buf_size = num_instr * sizeof(test_type);
  
  // allocate device memory
  std::cout << "allocate device memory" << std::endl;
  RT_CHECK(vx_mem_alloc(device, buf_size, VX_MEM_READ, &src0_buffer));
  RT_CHECK(vx_mem_address(src0_buffer, (uint64_t*) &kernel_arg.src0_addr));
  RT_CHECK(vx_mem_alloc(device, buf_size, VX_MEM_READ, &src1_buffer));
  RT_CHECK(vx_mem_address(src1_buffer, (uint64_t*) &kernel_arg.src1_addr));
  RT_CHECK(vx_mem_alloc(device, buf_size, VX_MEM_WRITE, &dst_buffer));
  RT_CHECK(vx_mem_address(dst_buffer, (uint64_t*) &kernel_arg.dst_addr));

  
  std::cout << "dev_src0=0x" << std::hex << kernel_arg.src0_addr << std::dec << std::endl;
  std::cout << "dev_src1=0x" << std::hex << kernel_arg.src1_addr << std::dec << std::endl;
  std::cout << "dev_dst=0x" << std::hex << kernel_arg.dst_addr << std::endl;

  std::cout << "run kernel test" << std::endl;
  int errors = run_kernel_test(kernel_arg);

  // cleanup
  std::cout << "cleanup" << std::endl;
  cleanup();

  if (errors != 0) {
    std::cout << "Found " << std::dec << errors << " errors!" << std::endl;
    std::cout << "FAILED!" << std::endl;
    return errors;
  }

  std::cout << "Test PASSED" << std::endl;

  return 0;
}
