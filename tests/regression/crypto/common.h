#ifndef _COMMON_H_
#define _COMMON_H_

#include <VX_config.h>
#include <stdint.h>
#include "aes_common.h"

#ifdef XLEN_64
  typedef uint64_t test_type;
#else
  typedef uint32_t test_type;
#endif

typedef struct {
  uint8_t*  pt_addr;
  uint64_t  pt_size;
  uint8_t*  ct_addr;
  uint64_t  ct_size;
  uint8_t*  key_addr;
  uint64_t  key_size;
  uint8_t*  round_keys_addr;
  uint64_t  round_keys_size;
  AES_SIZE  aes_size;
  bool      in_memory_test; 
  bool      encrypt; // true for encryption, false for decryption
  uint32_t  grid_dim;
  uint32_t  block_dim;
} kernel_arg_t;

#endif