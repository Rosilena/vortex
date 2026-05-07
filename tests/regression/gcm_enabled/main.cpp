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
vx_buffer_h tag_buffer = nullptr;
vx_buffer_h iv_buffer = nullptr;
vx_buffer_h aad_buffer = nullptr;
vx_buffer_h krnl_buffer = nullptr;
vx_buffer_h args_buffer = nullptr;
kernel_arg_t kernel_arg = {};


#if defined(AES256) && (AES256 == 1)

#define LEN_IN 51
#define LEN_OUT 51
#define LEN_AAD 90
#define LEN_IV 12
#define LEN_TAG 16
#define AES_KEYLEN 32

uint8_t key_u8[AES_KEYLEN] = {
    0x26, 0xbf, 0x25, 0x5b, 0xee, 0x60, 0xef, 0x0f,
    0x65, 0x37, 0x69, 0xe7, 0x03, 0x4d, 0xb9, 0x5b,
    0x8c, 0x79, 0x17, 0x52, 0x75, 0x4e, 0x57, 0x5c,
    0x76, 0x10, 0x59, 0xe9, 0xee, 0x8d, 0xcf, 0x78
};

uint8_t iv_u8[LEN_IV] = {
    0xce, 0xcd, 0x97, 0xab, 0x07, 0xce, 0x57, 0xc1,
    0x61, 0x27, 0x44, 0xf5
};

uint8_t in_u8[LEN_IN] = {
    0x96, 0x98, 0x39, 0x17, 0xa0, 0x36, 0x65, 0x07,
    0x63, 0xac, 0xa2, 0xb4, 0xe9, 0x27, 0xd9, 0x5f,
    0xfc, 0x74, 0x33, 0x95, 0x19, 0xed, 0x40, 0xc4,
    0x33, 0x6d, 0xba, 0x91, 0xed, 0xfb, 0xf9, 0xad
};

uint8_t out_u8[LEN_OUT] = {
    0xe3, 0x4b, 0x15, 0x40, 0xa7, 0x69, 0xf7, 0x91,
    0x33, 0x31, 0xd6, 0x67, 0x96, 0xe0, 0x0b, 0xdc,
    0x3e, 0xe0, 0xf2, 0x58, 0xcf, 0x24, 0x4e, 0xb7,
    0x66, 0x33, 0x75, 0xcc, 0x5a, 0xd6, 0xc6, 0x58
};

uint8_t aad_u8[LEN_AAD] = {
    0xaf, 0xeb, 0xbe, 0x9f, 0x26, 0x0f, 0x8c, 0x11,
    0x8e, 0x52, 0xb8, 0x4d, 0x88, 0x80, 0xa3, 0x46,
    0x22, 0x67, 0x5f, 0xae, 0xf3, 0x34, 0xcd, 0xb4,
    0x1b, 0xe9, 0x38, 0x5b, 0x7d, 0x05, 0x9b, 0x79,
    0xc0, 0xf8, 0xa4, 0x32, 0xd2, 0x5f, 0x8b, 0x71,
    0xe7, 0x81, 0xb1, 0x77, 0xfc, 0xe4, 0xd4, 0xc5,
    0x7a, 0xc5, 0x73, 0x45, 0x43, 0xe8, 0x5d, 0x75,
    0x13, 0xf9, 0x63, 0x82, 0xff, 0x4b, 0x2d, 0x4b,
    0x95, 0xb2, 0xf1, 0xfd, 0xba, 0xf9, 0xe7, 0x8b,
    0xbd, 0x1d, 0xb1, 0x3a, 0x7d, 0xd2, 0x6e, 0x8a,
    0x4a, 0xc8, 0x3a, 0x3e, 0x8a, 0xb4, 0x2d, 0x1d,
    0x54, 0x5f
};

uint8_t tag_u8[LEN_TAG] = {
    0x38, 0x41, 0xf0, 0x2b, 0xeb, 0x7a, 0x7f, 0xca,
    0x7e, 0x57, 0x89, 0x22, 0xd0, 0xa2, 0xf8, 0x0c
};

#elif defined(AES192) && (AES192 == 1)

#define LEN_IN 32
#define LEN_OUT 32
#define LEN_AAD 90
#define LEN_IV 12
#define LEN_TAG 16
#define AES_KEYLEN 24

uint8_t key_u8[AES_KEYLEN] = {
    0x1b, 0x8f, 0xef, 0x01, 0xcf, 0x6e, 0xfa, 0xbc,
    0x2c, 0x7c, 0xff, 0x16, 0xff, 0x67, 0xa3, 0x30,
    0x13, 0xf0, 0x62, 0x9a, 0x0a, 0xd9, 0x36, 0x95
};

uint8_t iv_u8[LEN_IV] = {
    0xe9, 0x5e, 0xbb, 0x1e, 0x8d, 0x31, 0xd1, 0xac,
    0x41, 0x4a, 0xd5, 0xf1
};

uint8_t in_u8[LEN_IN] = {
    0xc0, 0xc9, 0x22, 0x56, 0x03, 0x2e, 0xd3, 0x11,
    0x3a, 0x79, 0x41, 0x0a, 0x8b, 0x0e, 0x25, 0x0e,
    0x9a, 0x97, 0xfb, 0x47, 0x29, 0x8d, 0x12, 0x29,
    0x32, 0xff, 0x94, 0xd8, 0xf9, 0x40, 0xbc, 0xf8
};

uint8_t out_u8[LEN_OUT] = {
    0x2c, 0x04, 0x3e, 0xc6, 0x9c, 0x25, 0x5c, 0x7e,
    0xaf, 0x7b, 0xef, 0x47, 0x9b, 0x74, 0x67, 0x4f,
    0xf2, 0x0c, 0x18, 0x5b, 0xcc, 0x6f, 0x94, 0x24,
    0x22, 0xc7, 0xb5, 0xbe, 0x19, 0x99, 0x68, 0x37
};

uint8_t aad_u8[LEN_AAD] = {
    0x9c, 0x1a, 0x35, 0x60, 0x40, 0xa1, 0x87, 0xb7,
    0xd8, 0x5c, 0x7b, 0xbd, 0xc2, 0x3d, 0x3e, 0x9c,
    0x63, 0x8f, 0x01, 0x4b, 0xd5, 0x62, 0x90, 0x88,
    0x75, 0x7b, 0x57, 0x05, 0xb4, 0xf2, 0x78, 0x33,
    0xb0, 0xa2, 0xb3, 0xfa, 0x4c, 0x9c, 0x43, 0xa7,
    0x7c, 0x69, 0xa3, 0xa2, 0x0a, 0xff, 0xd2, 0xac,
    0x4a, 0xa6, 0xfd, 0xf2, 0xe0, 0x7c, 0x0a, 0x8d,
    0xaf, 0x1a, 0x19, 0xc4, 0x9a, 0x6a, 0x69, 0xfb,
    0xf4, 0x25, 0x1b, 0x77, 0x98, 0x17, 0x31, 0x82,
    0xa5, 0x28, 0xe6, 0xeb, 0x94, 0x19, 0x28, 0xaf,
    0x99, 0x53, 0xbf, 0x59, 0x5b, 0xfb, 0x7b, 0xdf,
    0x5c, 0x3a
};

uint8_t tag_u8[LEN_TAG] = {
    0x6b, 0xce, 0x3a, 0xbe, 0xe7, 0xb7, 0xa6, 0x56,
    0x3a, 0xb7, 0x9f, 0x68, 0xa4, 0x19, 0x72, 0xf4
};

#else

#define LEN_IN 32
#define LEN_OUT 32
#define LEN_AAD 90
#define LEN_IV 12
#define LEN_TAG 16
#define AES_KEYLEN 16

uint8_t key_u8[AES_KEYLEN] = {
    0x8f, 0xbf, 0x7c, 0xa1, 0x2f, 0xd5, 0x25, 0xdd,
    0xe9, 0x1e, 0x62, 0x58, 0x73, 0xfe, 0x51, 0xc2
};

uint8_t iv_u8[LEN_IV] = {
    0x20, 0x0b, 0xea, 0x51, 0x7b, 0x97, 0x90, 0xa1,
    0xcf, 0xad, 0xaf, 0x5e
};

uint8_t in_u8[LEN_IN] = {
    0x39, 0xd3, 0xe6, 0x27, 0x7c, 0x4b, 0x49, 0x63,
    0x84, 0x0d, 0x16, 0x42, 0xe6, 0xfa, 0xae, 0x0a,
    0x5b, 0xe2, 0xda, 0x97, 0xf6, 0x1c, 0x4e, 0x55,
    0xbb, 0x57, 0xce, 0x02, 0x19, 0x03, 0xd4, 0xc4
};

uint8_t out_u8[LEN_OUT] = {
    0xfe, 0x67, 0x8e, 0xf7, 0x6f, 0x69, 0xac, 0x95,
    0xdb, 0x55, 0x3b, 0x6d, 0xad, 0xd5, 0xa0, 0x7a,
    0x9d, 0xc8, 0xe1, 0x51, 0xfe, 0x6a, 0x9f, 0xa3,
    0xa1, 0xcd, 0x62, 0x16, 0x36, 0xb8, 0x78, 0x68
};

uint8_t aad_u8[LEN_AAD] = {
    0xa4, 0x14, 0xc0, 0x7f, 0xe2, 0xe6, 0x0b, 0xec,
    0x9c, 0xcc, 0x40, 0x9e, 0x9e, 0x89, 0x9c, 0x6f,
    0xe6, 0x05, 0x80, 0xbb, 0x26, 0x07, 0xc8, 0x61,
    0xf7, 0xf0, 0x85, 0x23, 0xe6, 0x9c, 0xda, 0x1b,
    0x9c, 0x3a, 0x71, 0x1d, 0x1d, 0x9c, 0x35, 0x09,
    0x17, 0x71, 0xe4, 0xc9, 0x50, 0xb9, 0x99, 0x6d,
    0x0a, 0xd0, 0x4f, 0x2e, 0x00, 0xd1, 0xb3, 0x10,
    0x58, 0x53, 0x54, 0x2a, 0x96, 0xe0, 0x9f, 0xff,
    0xfc, 0x2e, 0xc8, 0x0f, 0x8c, 0xf8, 0x87, 0x28,
    0xf5, 0x94, 0xf0, 0xae, 0xb1, 0x4f, 0x98, 0xa6,
    0x88, 0x23, 0x4e, 0x8b, 0xfb, 0xf7, 0x03, 0x27,
    0xb3, 0x64
};

uint8_t tag_u8[LEN_TAG] = {
    0x7c, 0x86, 0x07, 0x74, 0xf8, 0x83, 0x32, 0xb9,
    0xa7, 0xce, 0x6b, 0xbd, 0x02, 0x72, 0xa7, 0x27
};
#endif

#define SIZE_KEY  (AES_KEYLEN)
#define SIZE_IN   (LEN_IN    )
#define SIZE_OUT  (LEN_OUT   )
#define SIZE_IV   (LEN_IV    )
#define SIZE_AAD  (LEN_AAD   )
#define SIZE_TAG  (LEN_TAG   )


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
    vx_mem_free(tag_buffer);
    vx_mem_free(krnl_buffer);
    vx_mem_free(args_buffer);
    vx_dev_close(device);
  }
}


int run_kernel_test(const kernel_arg_t& kernel_arg) {
  std::vector<uint8_t> key (key_u8, key_u8 + SIZE_KEY);
  std::vector<uint8_t> in  (in_u8, in_u8   + SIZE_IN ); 
  std::vector<uint8_t> iv  (iv_u8, iv_u8   + SIZE_IV );
  std::vector<uint8_t> aad (aad_u8, aad_u8 + SIZE_AAD);
  std::vector<uint8_t> out (out_u8, out_u8 + SIZE_OUT);
  std::vector<uint8_t> tag (SIZE_TAG);
  
  // Upload kernel binary
  std::cout << "Upload kernel binary" << std::endl;
  RT_CHECK(vx_upload_kernel_file(device, kernel_file, &krnl_buffer));

  // upload kernel argument
  std::cout << "upload kernel argument" << std::endl;
  RT_CHECK(vx_upload_bytes(device, &kernel_arg, sizeof(kernel_arg_t), &args_buffer));

  if (kernel_arg.enc_dec) {
    // Encrypt, upload plaintext in_buffer
    std::cout << "upload source in_buffer" << std::endl;
    RT_CHECK(vx_copy_to_dev(in_buffer, in.data(), 0,  SIZE_IN));
  } else {
    // Decrypt, upload cyphertext in_buffer
    std::cout << "upload source in_buffer" << std::endl;
    RT_CHECK(vx_copy_to_dev(out_buffer, out.data(), 0,  SIZE_OUT));
  }

  // upload source iv_buffer
  std::cout << "upload source iv_buffer" << std::endl;
  RT_CHECK(vx_copy_to_dev(iv_buffer, iv.data(), 0,  SIZE_IV));

  // upload source key_buffer
  std::cout << "upload source key_buffer" << std::endl; 
  RT_CHECK(vx_copy_to_dev(key_buffer, key.data(), 0,  SIZE_KEY));

  // upload source aad_buffer
  std::cout << "upload source aad_buffer" << std::endl; 
  vx_copy_to_dev(aad_buffer, aad.data(), 0,  SIZE_AAD);

  auto time_start = std::chrono::high_resolution_clock::now();
  
  // start device
  std::cout << "start execution";
  
  #if defined(AES256) && (AES256 == 1)
  std::cout << " AES256 GCM" << std::endl;
  #elif defined(AES192) && (AES192 == 1)
  std::cout << " AES192 GCM" << std::endl;
  #else
  std::cout << " AES128 GCM" << std::endl;
  #endif

  auto t2 = std::chrono::high_resolution_clock::now();
  RT_CHECK(vx_start(device, krnl_buffer, args_buffer));
  RT_CHECK(vx_ready_wait(device, VX_MAX_TIMEOUT));
  auto t3 = std::chrono::high_resolution_clock::now();

  // download destination buffer
  std::cout << "read destination buffer from local memory" << std::endl;
  auto t4 = std::chrono::high_resolution_clock::now();
  if (kernel_arg.enc_dec) {
    RT_CHECK(vx_copy_from_dev(out.data(), out_buffer, 0,  SIZE_OUT));
  } else {
    RT_CHECK(vx_copy_from_dev(in.data(), in_buffer, 0,  SIZE_IN));
  }
  RT_CHECK(vx_copy_from_dev(tag.data(), tag_buffer, 0,  SIZE_TAG));
  auto t5 = std::chrono::high_resolution_clock::now();

  printf("TAG =");

  for (int i = 0; i < LEN_TAG; i++) {
    printf(" 0x%0x", tag[i]);
  }
  printf("\n");

    printf("CT =");

  for (int i = 0; i < LEN_OUT; i++) {
    printf(" 0x%0x", out[i]);
  }
  printf("\n");
  
  int errors = 0;
  std::cout << "Verify Result" << std::endl;
  if (0 == memcmp((char *) tag.data(), (char *) tag_u8, SIZE_TAG)) {
      printf("SUCCESS TAG!\n");
      errors = 0;
  } else {
      printf("FAILURE TAG!\n");
      errors = 1;
  }

  if (kernel_arg.enc_dec) {
    if (0 == memcmp((char *) out.data(), (char *) out_u8, SIZE_OUT)) {
        printf("SUCCESS ENCRYPT!\n");
        errors = 0;
    } else {
        printf("FAILURE ENCRYPT!\n");
        errors = 1;
    }
  } else {
    if (0 == memcmp((char *) in.data(), (char *) in_u8, SIZE_IN)) {
        printf("SUCCESS DECRYPT!\n");
        errors = 0;
    } else {
        printf("FAILURE DECRYPT!\n");
        errors = 1;
    }
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
  
  kernel_arg.block_dim = NUM_WARPS * NUM_THREADS;
  kernel_arg.roundkeys = Nr;
  kernel_arg.grid_dim  = NUM_CORES;
  kernel_arg.enc_dec   = ENC_OR_DEC;
  kernel_arg.size_in   = SIZE_IN;
  kernel_arg.size_out  = SIZE_OUT;
  kernel_arg.size_iv   = SIZE_IV;
  kernel_arg.size_aad  = SIZE_AAD;
  
  std::cout << "Num Cores " << NUM_CORES << "Num Warps " << NUM_WARPS << " NUM THREADS " << NUM_THREADS << std::endl;
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
  vx_mem_alloc(device,  SIZE_AAD, VX_MEM_READ, &aad_buffer);
  vx_mem_address(aad_buffer, (uint64_t*) &kernel_arg.aad_addr);
  RT_CHECK(vx_mem_alloc(device,  SIZE_OUT, VX_MEM_WRITE, &out_buffer));
  RT_CHECK(vx_mem_address(out_buffer, (uint64_t*) &kernel_arg.out_addr));
  RT_CHECK(vx_mem_alloc(device,  SIZE_TAG, VX_MEM_WRITE, &tag_buffer));
  RT_CHECK(vx_mem_address(tag_buffer, (uint64_t*) &kernel_arg.tag_addr));
  
  std::cout << "run kernel test" << std::endl;
  run_kernel_test(kernel_arg);

  // cleanup
  std::cout << "cleanup" << std::endl;
  cleanup();

  std::cout << "Fine Test" << std::endl;

  return 0;
}
