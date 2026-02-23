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

vx_device_h device = nullptr;
vx_buffer_h key_buffer = nullptr;
vx_buffer_h in_buffer = nullptr;
vx_buffer_h out_buffer = nullptr;
vx_buffer_h iv_buffer = nullptr;
vx_buffer_h krnl_buffer = nullptr;
vx_buffer_h args_buffer = nullptr;
kernel_arg_t kernel_arg = {};

#define LEN_IN  64
#define LEN_OUT 64
#define LEN_IV  16
  
#define SIZE_KEY  (AES_KEYLEN * sizeof(uint8_t))
#define SIZE_IN   (LEN_IN  * sizeof(uint8_t))
#define SIZE_OUT  (LEN_OUT * sizeof(uint8_t))
#define SIZE_IV   (LEN_IV  * sizeof(uint8_t))

#if defined(AES256)
  uint8_t key_u8[AES_KEYLEN] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                      0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
  uint8_t in_u8[LEN_IN]   = { 0x60, 0x1e, 0xc3, 0x13, 0x77, 0x57, 0x89, 0xa5, 0xb7, 0xa7, 0xf5, 0x04, 0xbb, 0xf3, 0xd2, 0x28, 
                      0xf4, 0x43, 0xe3, 0xca, 0x4d, 0x62, 0xb5, 0x9a, 0xca, 0x84, 0xe9, 0x90, 0xca, 0xca, 0xf5, 0xc5, 
                      0x2b, 0x09, 0x30, 0xda, 0xa2, 0x3d, 0xe9, 0x4c, 0xe8, 0x70, 0x17, 0xba, 0x2d, 0x84, 0x98, 0x8d, 
                      0xdf, 0xc9, 0xc5, 0x8d, 0xb6, 0x7a, 0xad, 0xa6, 0x13, 0xc2, 0xdd, 0x08, 0x45, 0x79, 0x41, 0xa6 };
#elif defined(AES192)
  uint8_t key_u8[AES_KEYLEN] = { 0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5, 
                      0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b };
  uint8_t in_u8[LEN_IN]   = { 0x1a, 0xbc, 0x93, 0x24, 0x17, 0x52, 0x1c, 0xa2, 0x4f, 0x2b, 0x04, 0x59, 0xfe, 0x7e, 0x6e, 0x0b, 
                      0x09, 0x03, 0x39, 0xec, 0x0a, 0xa6, 0xfa, 0xef, 0xd5, 0xcc, 0xc2, 0xc6, 0xf4, 0xce, 0x8e, 0x94, 
                      0x1e, 0x36, 0xb2, 0x6b, 0xd1, 0xeb, 0xc6, 0x70, 0xd1, 0xbd, 0x1d, 0x66, 0x56, 0x20, 0xab, 0xf7, 
                      0x4f, 0x78, 0xa7, 0xf6, 0xd2, 0x98, 0x09, 0x58, 0x5a, 0x97, 0xda, 0xec, 0x58, 0xc6, 0xb0, 0x50 };
#elif defined(AES128)
  uint8_t key_u8[AES_KEYLEN] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  uint8_t in_u8[LEN_IN]   = { 0x87, 0x4d, 0x61, 0x91, 0xb6, 0x20, 0xe3, 0x26, 0x1b, 0xef, 0x68, 0x64, 0x99, 0x0d, 0xb6, 0xce,
                      0x98, 0x06, 0xf6, 0x6b, 0x79, 0x70, 0xfd, 0xff, 0x86, 0x17, 0x18, 0x7b, 0xb9, 0xff, 0xfd, 0xff,
                      0x5a, 0xe4, 0xdf, 0x3e, 0xdb, 0xd5, 0xd3, 0x5e, 0x5b, 0x4f, 0x09, 0x02, 0x0d, 0xb0, 0x3e, 0xab,
                      0x1e, 0x03, 0x1d, 0xda, 0x2f, 0xbe, 0x03, 0xd1, 0x79, 0x21, 0x70, 0xa0, 0xf3, 0x00, 0x9c, 0xee };
#endif
  uint8_t iv_u8[LEN_IV]   = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
  uint8_t out_u8[LEN_OUT] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                      0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
                      0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
                      0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };

static void show_usage() {
   std::cout << "Vortex Test for AES Counter Mode...." << std::endl;
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
    vx_mem_free(iv_buffer);
    vx_mem_free(in_buffer);
    vx_mem_free(out_buffer);
    vx_mem_free(key_buffer);
    vx_mem_free(krnl_buffer);
    vx_mem_free(args_buffer);
    vx_dev_close(device);
  }
}


int run_kernel_test(const kernel_arg_t& kernel_arg) {
  std::vector<uint8_t> key (key_u8, key_u8 + SIZE_KEY);
  std::vector<uint8_t> in  (in_u8, in_u8   + SIZE_IN ); 
  std::vector<uint8_t> iv  (iv_u8, iv_u8   + SIZE_IV );
  std::vector<uint8_t> out (SIZE_OUT);

  // Upload kernel binary
  std::cout << "Upload kernel binary" << std::endl;
  RT_CHECK(vx_upload_kernel_file(device, kernel_file, &krnl_buffer));

  // upload kernel argument
  std::cout << "upload kernel argument" << std::endl;
  RT_CHECK(vx_upload_bytes(device, &kernel_arg, sizeof(kernel_arg_t), &args_buffer));

  // upload source in_buffer
  std::cout << "upload source in_buffer" << std::endl;
  RT_CHECK(vx_copy_to_dev(in_buffer, in.data(), 0,  SIZE_IN));

  // upload source iv_buffer
  std::cout << "upload source iv_buffer" << std::endl;
  RT_CHECK(vx_copy_to_dev(iv_buffer, iv.data(), 0,  SIZE_IV));

  // upload source key_buffer
  std::cout << "upload source key_buffer" << std::endl; 
  RT_CHECK(vx_copy_to_dev(key_buffer, key.data(), 0,  SIZE_KEY));

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
  RT_CHECK(vx_copy_from_dev(out.data(), out_buffer, 0,  SIZE_OUT));
  auto t5 = std::chrono::high_resolution_clock::now();

  printf("CTR =");

  for (int i = 0; i < LEN_OUT; i++) {
    printf(" 0x%0x", out[i]);
  }
  printf("\n");
  
  int errors = 0;
  std::cout << "Verify Result" << std::endl;
  if (0 == memcmp((char *) out.data(), (char *) out_u8, SIZE_OUT)) {
      printf("SUCCESS!\n");
      errors = 0;
  } else {
      printf("FAILURE!\n");
      errors = 1;
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
  
  kernel_arg.size_in   = SIZE_IN;
  kernel_arg.block_dim = SIZE_IN / AES_BLOCKLEN;
  kernel_arg.grid_dim  = 1;

  // open device connection
  std::cout << "open device connection" << std::endl;
  RT_CHECK(vx_dev_open(&device));
  
  // allocate device memory
  std::cout << "allocate device memory" << std::endl;
  RT_CHECK(vx_mem_alloc(device,  SIZE_KEY, VX_MEM_READ, &key_buffer));
  RT_CHECK(vx_mem_address(key_buffer, (uint64_t*) &kernel_arg.key_addr));
  RT_CHECK(vx_mem_alloc(device,  SIZE_IN, VX_MEM_READ, &in_buffer));
  RT_CHECK(vx_mem_address(in_buffer, (uint64_t*) &kernel_arg.in_addr));
  RT_CHECK(vx_mem_alloc(device,  SIZE_IV, VX_MEM_READ, &iv_buffer));
  RT_CHECK(vx_mem_address(iv_buffer, (uint64_t*) &kernel_arg.iv_addr));
  RT_CHECK(vx_mem_alloc(device,  SIZE_OUT, VX_MEM_WRITE, &out_buffer));
  RT_CHECK(vx_mem_address(out_buffer, (uint64_t*) &kernel_arg.out_addr));

  
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
