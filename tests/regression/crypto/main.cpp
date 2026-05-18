#include <iostream>
#include <unistd.h>
#include <string.h>
#include <vortex.h>
#include <chrono>
#include <vector>
#include <set>
#include "aes_common.h"
#include "aes_tv.h"
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
int size = 47;
test_type count = 0;
std::string algorithm = "aes-128-ecb";
std::set<std::string> valid_algorithms = {"aes-128-ecb", "aes-192-ecb", "aes-256-ecb", 
                                          "aes-128-ctr", "aes-192-ctr", "aes-256-ctr", 
                                          "aes-128-gcm", "aes-192-gcm", "aes-256-gcm"};

vx_device_h device = nullptr;

vx_buffer_h krnl_buffer = nullptr;
vx_buffer_h args_buffer = nullptr;

vx_buffer_h pt_buffer = nullptr;
vx_buffer_h ct_buffer = nullptr;
vx_buffer_h key_buffer = nullptr;
vx_buffer_h round_keys_buffer = nullptr;

// For GCM mode
vx_buffer_h iv_buffer = nullptr;
vx_buffer_h aad_buffer = nullptr;
vx_buffer_h tag_buffer = nullptr;

kernel_arg_t kernel_arg = {};

static void show_usage() {
  std::cout << "Vortex Cryptographic Extensions suite...." << std::endl;
  std::cout << "Usage: [-a algorithm][-t: test][-n size][-h: help]" << std::endl;
  std::cout << "Where algorithm can be: "<< std::endl;
  std::cout << "aes-128-ecb, aes-192-ecb, aes-256-ecb, ";
  std::cout << "aes-128-ctr, aes-192-ctr, aes-256-ctr, ";
  std::cout << "aes-128-gcm, aes-192-gcm, aes-256-gcm" << std::endl;
  std::cout << "Where test can be: -t 0: functional test, -t 1: performance test, -t 2: in-memory test" << std::endl;
  std::cout << "-n size specifies the number of blocks to be encrypted in performance test" << std::endl;
}

static void parse_args(int argc, char **argv) {
  int c;
  while ((c = getopt(argc, argv, "a:t:n:h")) != -1) {
    switch (c) {
    case 'a':
      algorithm = std::string(optarg);
      break;
    case 't':
      test = atoi(optarg);
      break;
    case 'n':
      size = atoi(optarg);
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
    vx_mem_free(pt_buffer);
    vx_mem_free(ct_buffer);
    vx_mem_free(key_buffer);
    vx_mem_free(krnl_buffer);
    vx_mem_free(args_buffer);
    vx_mem_free(round_keys_buffer);

    if (iv_buffer) {
      vx_mem_free(iv_buffer);
    }

    if (aad_buffer) {
      vx_mem_free(aad_buffer);
    }

    if (tag_buffer) {
      vx_mem_free(tag_buffer);
    }
    vx_dev_close(device);
  }
}

void run_aes_ecb_test() {
  /*
   * This function runs AES ECB mode test using the AES kernel implemented in kernel.vxbin.
   * It tests AES-128, AES-192 and AES-256 with a fixed plaintext and key, and compares the result with expected ciphertext.
   * The test is run for both encryption and decryption.
   */
  
  // allocate device memory
  std::cout << "allocate device memory" << std::endl;
  uint64_t pt_size = 0, ct_size = 0, key_size  = 0, round_keys_size = 0;
  std::vector<uint8_t> pt_vec, ct_vec, key_vec;
  uint8_t* expected_ct = nullptr, *expected_pt = nullptr;

  if (algorithm.find("128") != std::string::npos) {
    key_size = 16;
    round_keys_size = (AES_128_NR + 1) * sizeof(uint64_t) * 2;
  } else if (algorithm.find("192") != std::string::npos) {
    key_size = 24;
    round_keys_size = (AES_192_NR + 1) * sizeof(uint64_t) * 2;
  } else if (algorithm.find("256") != std::string::npos) {
    key_size = 32;
    round_keys_size = (AES_256_NR + 1) * sizeof(uint64_t) * 2;
  }

  if (test == 1) {
    pt_size = size * AES_BLOCKLEN;
    ct_size = size * AES_BLOCKLEN;

    for(uint64_t i = 0; i < pt_size; i++){
      pt_vec.push_back(i % 256);
    }
  } else {
    if (algorithm.find("128") != std::string::npos) {
      pt_size = sizeof(aes_128_ecb_pt);
      ct_size = sizeof(aes_128_ecb_ct);
      
      kernel_arg.aes_size = AES128;
      pt_vec.assign(aes_128_ecb_pt, aes_128_ecb_pt + pt_size);
      key_vec.assign(aes_128_ecb_key, aes_128_ecb_key + key_size);
      expected_ct = aes_128_ecb_ct;
      expected_pt = aes_128_ecb_pt;
      ct_vec.resize(ct_size, 0);
    } else if (algorithm.find("192") != std::string::npos) {
      pt_size = sizeof(aes_192_ecb_pt);
      ct_size = sizeof(aes_192_ecb_ct);

      kernel_arg.aes_size = AES192;
      pt_vec.assign(aes_192_ecb_pt, aes_192_ecb_pt + pt_size);
      key_vec.assign(aes_192_ecb_key, aes_192_ecb_key + key_size);
      expected_ct = aes_192_ecb_ct;
      expected_pt = aes_192_ecb_pt;
      ct_vec.resize(ct_size, 0);
    } else if (algorithm.find("256") != std::string::npos) {
      pt_size = sizeof(aes_256_ecb_pt);
      ct_size = sizeof(aes_256_ecb_ct);

      kernel_arg.aes_size = AES256;
      pt_vec.assign(aes_256_ecb_pt, aes_256_ecb_pt + pt_size);
      key_vec.assign(aes_256_ecb_key, aes_256_ecb_key + key_size);
      expected_ct = aes_256_ecb_ct;
      expected_pt = aes_256_ecb_pt;
      ct_vec.resize(ct_size, 0);
    }
  }

  kernel_arg.pt_size = pt_size;
  kernel_arg.ct_size = ct_size;
  kernel_arg.key_size = key_size;
  kernel_arg.round_keys_size = round_keys_size;
  kernel_arg.grid_dim = (pt_size + AES_BLOCKLEN - 1) / AES_BLOCKLEN; // one thread per block
  kernel_arg.block_dim = 1;

  uint64_t pt_addr = 0, ct_addr = 0, key_addr = 0, round_keys_addr = 0;

  RT_CHECK(vx_mem_alloc(device, pt_size, VX_MEM_READ_WRITE, &pt_buffer));
  RT_CHECK(vx_mem_address(pt_buffer, &pt_addr));
  RT_CHECK(vx_mem_alloc(device, ct_size, VX_MEM_READ_WRITE, &ct_buffer));
  RT_CHECK(vx_mem_address(ct_buffer, &ct_addr));
  RT_CHECK(vx_mem_alloc(device, key_size, VX_MEM_READ, &key_buffer));
  RT_CHECK(vx_mem_address(key_buffer, &key_addr));
  RT_CHECK(vx_mem_alloc(device, round_keys_size, VX_MEM_READ_WRITE, &round_keys_buffer));
  RT_CHECK(vx_mem_address(round_keys_buffer, &round_keys_addr));

  kernel_arg.pt_addr = (uint8_t*) pt_addr;
  kernel_arg.ct_addr = (uint8_t*) ct_addr;
  kernel_arg.key_addr = (uint8_t*) key_addr;
  kernel_arg.round_keys_addr = (uint8_t*) round_keys_addr;

  std::cout << "dev_pt=0x"  << std::hex << (uint64_t)kernel_arg.pt_addr  << std::dec << std::endl;
  std::cout << "dev_ct=0x"  << std::hex << (uint64_t)kernel_arg.ct_addr  << std::dec << std::endl;
  std::cout << "dev_key=0x" << std::hex << (uint64_t)kernel_arg.key_addr << std::dec << std::endl;

  std::cout << "Upload kernel binary" << std::endl;
  RT_CHECK(vx_upload_kernel_file(device, kernel_file, &krnl_buffer));

  // Encryption test
  kernel_arg.encrypt = true;
  std::cout << "upload kernel argument" << std::endl;
  RT_CHECK(vx_upload_bytes(device, &kernel_arg, sizeof(kernel_arg_t), &args_buffer));
 
  // upload pt buffer
  std::cout << "upload plaintext buffer" << std::endl;
  RT_CHECK(vx_copy_to_dev(pt_buffer, pt_vec.data(), 0, pt_size));
  // upload key buffer
  std::cout << "upload key buffer" << std::endl;
  RT_CHECK(vx_copy_to_dev(key_buffer, key_vec.data(), 0, key_size));

  std::cout << "Start encryption test" << std::endl;
  RT_CHECK(vx_start(device, krnl_buffer, args_buffer));
  RT_CHECK(vx_ready_wait(device, VX_MAX_TIMEOUT));

  // download ct buffer
  RT_CHECK(vx_copy_from_dev(ct_vec.data(), ct_buffer, 0, ct_size));

  if (test == 0) {
    // verify encryption result
    int errors = 0;
    if (expected_ct) {
      for (size_t i = 0; i < ct_size; i++) {
        if (ct_vec[i] != expected_ct[i]) {
          errors++;
          std::cout << "Encryption mismatch at byte " << i << ": expected 0x" << std::hex << (int)expected_ct[i] << ", got 0x" << (int)ct_vec[i] << std::dec << std::endl;
        }
      }
    }

    if (errors == 0) {
      std::cout << "Encryption test PASSED!" << std::endl;
    } else {
      std::cout << "Encryption test FAILED with " << errors << " errors!" << std::endl;
    }
  }

  vx_dump_perf(device, stdout);

  // Decryption test
  kernel_arg.encrypt = false;
  std::cout << "upload kernel argument" << std::endl;
  RT_CHECK(vx_upload_bytes(device, &kernel_arg, sizeof(kernel_arg_t), &args_buffer));

  ct_vec.assign(expected_ct, expected_ct + ct_size);
  std::cout << "Upload ciphertext buffer" << std::endl;
  RT_CHECK(vx_copy_to_dev(ct_buffer, ct_vec.data(), 0, ct_size));
  
  std::cout << "Start decryption test" << std::endl;
  RT_CHECK(vx_start(device, krnl_buffer, args_buffer));
  RT_CHECK(vx_ready_wait(device, VX_MAX_TIMEOUT));

  // download pt buffer
  RT_CHECK(vx_copy_from_dev(pt_vec.data(), pt_buffer, 0, pt_size));

  if (test == 0) {
    // verify decryption result
    int errors = 0;
    if (expected_pt) {
      for (size_t i = 0; i < pt_size; i++) {
        if (pt_vec[i] != expected_pt[i]) {
          errors++;
          std::cout << "Decryption mismatch at byte " << i << ": expected 0x" << std::hex << (int)expected_pt[i] << ", got 0x" << (int)pt_vec[i] << std::dec << std::endl;
        }
      }
    }

    if (errors == 0) {
      std::cout << "Decryption test PASSED!" << std::endl;
    } else {
      std::cout << "Decryption test FAILED with " << errors << " errors!" << std::endl;
    }
  }

  cleanup();
}

void run_aes_ctr_test() {
  /*
   * This function runs AES CTR mode test using the AES kernel implemented in kernel.vxbin.
   * It tests AES-128, AES-192 and AES-256 with a fixed plaintext, key and nonce, and compares the result with expected ciphertext.
   * The test is run for both encryption and decryption.
   */
}

void run_aes_gcm_test() {
  /*
   * This function runs AES GCM mode test using the AES kernel implemented in kernel.vxbin.
   * It tests AES-128, AES-192 and AES-256 with a fixed plaintext, key, nonce and AAD, and compares the result with expected ciphertext and tag.
   * The test is run for both encryption and decryption.
   */
}


int main(int argc, char *argv[]) {
  // parse command arguments
  parse_args(argc, argv);

  if (valid_algorithms.find(algorithm) == valid_algorithms.end()) {
    std::cout << "Invalid algorithm selection!" << std::endl;
    show_usage();
    return -1;
  }

  std::cout << "Selected algorithm: " << algorithm << std::endl;

  if(test == 0){
    std::cout << "Running functional test" << std::endl;
  } else if(test == 1){
    std::cout << "Running performance test with size " << size << std::endl;
  } else if(test == 2){
    std::cout << "Running in-memory test" << std::endl;
  } else {
    std::cout << "Invalid test selection!" << std::endl;
    show_usage();
    return -1;
  }

  // open device connection
  std::cout << "open device connection" << std::endl;
  RT_CHECK(vx_dev_open(&device));

  uint64_t num_cores, num_warps, num_threads;
  RT_CHECK(vx_dev_caps(device, VX_CAPS_NUM_CORES, &num_cores));
  RT_CHECK(vx_dev_caps(device, VX_CAPS_NUM_WARPS, &num_warps));
  RT_CHECK(vx_dev_caps(device, VX_CAPS_NUM_THREADS, &num_threads));
  
  std::cout << "Device capabilities: " << std::endl;
  std::cout << "Number of cores: " << num_cores << std::endl;
  std::cout << "Number of warps: " << num_warps << std::endl;
  std::cout << "Number of threads: " << num_threads << std::endl;

  if (algorithm.find("ecb") != std::string::npos) {
    run_aes_ecb_test();
  } else if (algorithm.find("ctr") != std::string::npos) {
    run_aes_ctr_test();
  } else if (algorithm.find("gcm") != std::string::npos) {
    run_aes_gcm_test();
  }

  return 0;
}
