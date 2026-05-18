#ifndef AES_COMMON_H
#define AES_COMMON_H

#include <stdint.h>

#define AES_BLOCKLEN 16 // Block length in bytes - AES is 128b block only

#define AES_256_KEYLEN 32
#define AES_256_NK 8
#define AES_256_NR 14

#define AES_192_KEYLEN 24
#define AES_192_NK 6
#define AES_192_NR 12

#define AES_128_KEYLEN 16
#define AES_128_NK 4
#define AES_128_NR 10

typedef uint64_t state_t[2];

enum AES_SIZE {
    AES128 = 128,
    AES192 = 192,
    AES256 = 256
};

typedef struct aes_config {
    AES_SIZE size;
    uint64_t* round_keys; // Expanded round keys
    uint64_t Nk; // Number of 32-bit words comprising the Cipher Key
    uint64_t Nr; // Number of rounds, which is a function of Nk and Nb
    bool encrypt; // true for encryption, false for decryption  
} aes_config_t;

#endif // AES_COMMON_H