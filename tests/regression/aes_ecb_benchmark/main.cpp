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
   } while (false);

///////////////////////////////////////////////////////////////////////////////

const char* kernel_file = "kernel.vxbin";

vx_device_h device = nullptr;
vx_buffer_h key_buffer = nullptr;
vx_buffer_h in_buffer = nullptr;
vx_buffer_h out_buffer = nullptr;
vx_buffer_h krnl_buffer = nullptr;
vx_buffer_h args_buffer = nullptr;
kernel_arg_t kernel_arg = {};

uint8_t key_u8[AES_KEYLEN] = {0};

uint8_t in_u8[TEST_SIZE * AES_BLOCKLEN] = {0};

uint8_t out_u8[TEST_SIZE * AES_BLOCKLEN] = {0};

static void show_usage() {
  std::cout << "AES ECB Benchmark test" << std::endl;
  std::cout << "Usage: [-t testno][-k: kernel][-n words][-h: help]" << std::endl;
}

int test = -1;
int count = 0;

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
    vx_mem_free(in_buffer);
    vx_mem_free(out_buffer);
    vx_mem_free(key_buffer);
    vx_mem_free(krnl_buffer);
    vx_mem_free(args_buffer);
    vx_dev_close(device);
  }
}


int run_kernel_test(const kernel_arg_t& kernel_arg) {
  std::vector<uint8_t> key (key_u8, key_u8 + AES_KEYLEN);
  std::vector<uint8_t> in  (in_u8 , in_u8  + TEST_SIZE * AES_BLOCKLEN);
  std::vector<uint8_t> out (out_u8, out_u8 + TEST_SIZE * AES_BLOCKLEN);

  // Upload kernel binary
  std::cout << "Upload kernel binary" << std::endl;
  RT_CHECK(vx_upload_kernel_file(device, kernel_file, &krnl_buffer));

  // upload kernel argument
  std::cout << "upload kernel argument" << std::endl;
  RT_CHECK(vx_upload_bytes(device, &kernel_arg, sizeof(kernel_arg_t), &args_buffer));

  // Encrypt, upload plaintext in_buffer
  std::cout << "upload source in_buffer" << std::endl;
  RT_CHECK(vx_copy_to_dev(in_buffer, in.data(), 0,  TEST_SIZE * AES_BLOCKLEN));

  // upload source key_buffer
  std::cout << "upload source key_buffer" << std::endl; 
  RT_CHECK(vx_copy_to_dev(key_buffer, key.data(), 0, AES_KEYLEN));

  auto time_start = std::chrono::high_resolution_clock::now();
  
  // start device
  std::cout << "start execution ";
  
  #if defined(AES256) && (AES256 == 1)
  std::cout << "AES256 ECB Benchmark" << std::endl;
  #elif defined(AES192) && (AES192 == 1)
  std::cout << "AES192 ECB Benchmark" << std::endl;
  #else
  std::cout << "AES128 ECB Benchmark" << std::endl;
  #endif

  auto t2 = std::chrono::high_resolution_clock::now();
  RT_CHECK(vx_start(device, krnl_buffer, args_buffer));
  RT_CHECK(vx_ready_wait(device, VX_MAX_TIMEOUT));
  auto t3 = std::chrono::high_resolution_clock::now();

  // // download destination buffer
  // std::cout << "read destination buffer from local memory" << std::endl;
  // auto t4 = std::chrono::high_resolution_clock::now();
  // RT_CHECK(vx_copy_from_dev(out.data(), in_buffer, 0,  TEST_SIZE * AES_BLOCKLEN));
  // auto t5 = std::chrono::high_resolution_clock::now();

  // printf("CT =");

  // for (int i = 0; i < TEST_SIZE * AES_BLOCKLEN; i++) {
  //   printf(" 0x%0x", out[i]);
  // }
  // printf("\n");

  auto time_end = std::chrono::high_resolution_clock::now();

  double elapsed;
  elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
  printf("execute time: %lg ms\n", elapsed);
  elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
  printf("Total elapsed time: %lg ms\n", elapsed);

  return 0;
}

int main(int argc, char *argv[]) {
  // parse command arguments
  parse_args(argc, argv);

  if (count == 0) {
    count = 1;
  }
  
  kernel_arg.block_dim = 32;
  kernel_arg.roundkeys = Nr;
  kernel_arg.grid_dim  = 1;
  kernel_arg.size_in   = TEST_SIZE * (AES_BLOCKLEN);
  kernel_arg.size_out  = TEST_SIZE * (AES_BLOCKLEN);
  
  //std::cout << "Num Cores " << 4 << " Num Warps " << 4 << " NUM THREADS " << 8 << std::endl;
  // open device connection
  std::cout << "open device connection" << std::endl;
  RT_CHECK(vx_dev_open(&device));
  
  // allocate device memory
  std::cout << "allocate device memory" << std::endl;
  RT_CHECK(vx_mem_alloc(device,  AES_KEYLEN, VX_MEM_READ, &key_buffer));
  RT_CHECK(vx_mem_address(key_buffer, (uint64_t*) &kernel_arg.key_addr));
  RT_CHECK(vx_mem_alloc(device,  TEST_SIZE * AES_BLOCKLEN, VX_MEM_READ, &in_buffer));
  RT_CHECK(vx_mem_address(in_buffer, (uint64_t*) &kernel_arg.in_addr));
  RT_CHECK(vx_mem_alloc(device,  TEST_SIZE * AES_BLOCKLEN, VX_MEM_WRITE, &out_buffer));
  RT_CHECK(vx_mem_address(out_buffer, (uint64_t*) &kernel_arg.out_addr));
  
  std::cout << "run kernel test" << std::endl;
  run_kernel_test(kernel_arg);

  // cleanup
  std::cout << "cleanup" << std::endl;
  cleanup();

  std::cout << "Fine Test" << std::endl;

  return 0;
}
