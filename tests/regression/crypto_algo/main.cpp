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
uint32_t size = 16; //dimensione totale
uint32_t grid_dim = 4; //numero di blocchi nella griglia
uint32_t block_dim = 4; //numero di thread in un blocco
test_type key = 0x0F0F0F01; //contatore crittografia

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

int test = -1;
test_type count = 0;

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
  
  test_type buf_size = size * sizeof(test_type);
  
  std::vector<test_type> h_src0(buf_size);
  std::vector<test_type> h_src1(buf_size); 
  std::vector<test_type> h_dst(buf_size);

  h_src0[0] = (test_type) 0x12345678; //First operand
  //h_src1[0] = (test_type) 0x87654321; //Second operand
  h_src0[1] = (test_type) 0x12345678; //First operand
  //h_src1[1] = (test_type) 0x87654321; //Second operand
  h_src0[2] = (test_type) 0x12345678; //First operand
  //h_src1[2] = (test_type) 0x87654321; //Second operand
  h_src0[3] = (test_type) 0x12345678; //First operand
  //h_src1[3] = (test_type) 0x87654321; //Second operand
  h_src0[4] = (test_type) 0x12345678; //First operand
  //h_src1[4] = (test_type) 0x87654321; //Second operand
  h_src0[5] = (test_type) 0x12345678; //First operand
  //h_src1[5] = (test_type) 0x87654321; //Second operand
  h_src0[6] = (test_type) 0x12345678; //First operand
  //h_src1[6] = (test_type) 0x87654321; //Second operand
  h_src0[7] = (test_type) 0x12345678; //First operand
  //h_src1[7] = (test_type) 0x87654321; //Second operand
    h_src0[8] = (test_type) 0x12345678; //First operand
  //h_src1[8] = (test_type) 0x87654321; //Second operand
  h_src0[9] = (test_type) 0x12345678; //First operand
  //h_src1[9] = (test_type) 0x87654321; //Second operand
  h_src0[10] = (test_type) 0x12345678; //First operand
  //h_src1[10] = (test_type) 0x87654321; //Second operand
  h_src0[11] = (test_type) 0x12345678; //First operand
  //h_src1[11] = (test_type) 0x87654321; //Second operand
    h_src0[12] = (test_type) 0x12345678; //First operand
  //h_src1[12] = (test_type) 0x87654321; //Second operand
  h_src0[13] = (test_type) 0x12345678; //First operand
  //h_src1[13] = (test_type) 0x87654321; //Second operand
  h_src0[14] = (test_type) 0x12345678; //First operand
  //h_src1[14] = (test_type) 0x87654321; //Second operand
  h_src0[15] = (test_type) 0x12345678; //First operand
  //h_src1[15] = (test_type) 0x87654321; //Second operand

  // Upload kernel binary
  std::cout << "Upload kernel binary" << std::endl;
  RT_CHECK(vx_upload_kernel_file(device, kernel_file, &krnl_buffer));

  // upload kernel argument
  std::cout << "upload kernel argument" << std::endl;
  RT_CHECK(vx_upload_bytes(device, &kernel_arg, sizeof(kernel_arg_t), &args_buffer));

  // upload source buffer0
  std::cout << "upload source buffer0" << std::endl;
  RT_CHECK(vx_copy_to_dev(src0_buffer, h_src0.data(), 0, buf_size));

  // upload source buffer1
  std::cout << "upload source buffer1" << std::endl; 
  RT_CHECK(vx_copy_to_dev(src1_buffer, h_src1.data(), 0, buf_size));

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
  for(uint32_t i = 0; i < size; i++ ){
        std::cout << "Thread "<< i << " : " << std::hex << h_dst[i] << std::endl;
  }

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
  uint32_t num_points = size; 
  test_type buf_size = num_points * sizeof(test_type);
  
  kernel_arg.num_points = num_points;
  kernel_arg.grid_dim = grid_dim;
  kernel_arg.block_dim = block_dim;
  kernel_arg.key = key;
  
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
