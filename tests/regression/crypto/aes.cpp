#include "aes.h"
#include <vx_print.h>

aes_config_t aes_init(AES_SIZE size, uint64_t* round_keys, bool encrypt) {
      aes_config_t config;

      config.size = size;
      config.round_keys = round_keys;
      config.encrypt = encrypt;

      switch (size) {
            case AES128:
                  config.Nk = AES_128_NK;
                  config.Nr = AES_128_NR;
                  break;
            case AES192:
                  config.Nk = AES_192_NK;
                  config.Nr = AES_192_NR;
                  break;
            case AES256:
                  config.Nk = AES_256_NK;
                  config.Nr = AES_256_NR;
                  break;
            default:
                  // Invalid size, handle error
                  break;
      }
      
      if (config.encrypt) {
            key_expansion(&config);
      } else {
            inv_key_expansion(&config);
      }

      return config;
}

#define AES_KEYROUND_256_0(RND) \
      __asm__ (                                                                 \
            "aes64ks1i t0, %5," #RND "\n\t"                                     \
            "aes64ks2  %0, t0, %3 \n\t"                                         \
            "aes64ks2  %1, %4, %2"                                              \
            : "=r"(key[RND +1][0]), "=r"(key[RND+1][1])                         \
            : "r"(key[RND][1]), "r"(key[RND][0]), "0"(key[RND+1][0]), "r"(key[RND][3])            \
            : "t0"                                                              \
      );

#define AES_KEYROUND_256_1(RND)                     \
      __asm__ (                     \
            "aes64ks1i t0, %4, 0xA    \n\t"                     \
            "aes64ks2  %0, t0, %3     \n\t"                     \
            "aes64ks2  %1, %5, %2     \n\t"                     \
            : "=r"(key[RND + 1][2]), "=r"(key[RND + 1][3])                      \
            : "r"(key[RND][3]), "r"(key[RND][2]), "r"(key[RND + 1][1]), "0"(key[RND + 1][2])                        \
            : "t0"                      \
      );

#define AES_KEYROUND_192(RND) \
      __asm__ (                                                                                                     \
            "aes64ks1i t0, %3," #RND "\n\t"                                                                         \
            "aes64ks2  %0, t0, %5 \n\t"                                                                             \
            "aes64ks2  %1, %6, %4 \n\t"                                                                             \
            "aes64ks2  %2, %7, %3 \n\t"                                                                             \
            : "=r"(key[RND + 1][0]), "=r"(key[RND + 1][1]), "=r"(key[RND + 1][2])                                   \
            : "r"(key[RND][2]), "r"(key[RND][1]), "r"(key[RND][0]), "0"(key[RND + 1][0]), "1"(key[RND + 1][1])      \
            : "t0"                                                                                                  \
      );

#define AES_KEYROUND(RND) \
      __asm__ (                                                                 \
            "aes64ks1i t0, %2," #RND "\n\t"                                     \
            "aes64ks2  %0, t0, %3 \n\t"                                         \
            "aes64ks2  %1, %4, %2"                                              \
            : "=r"(key[RND +1][0]), "=r"(key[RND+1][1])                         \
            : "r"(key[RND][1]), "r"(key[RND][0]), "0"(key[RND+1][0])            \
            : "t0"                                                              \
      );

void aes_enc_double_round(state_t aes_state, const int &i, uint64_t (*round_keys)[2]) {
      __asm__ (
            "aes64esm t0, %0, %1\n\t"
            "aes64esm t1, %1, %0\n\t"
            "xor      t0, t0, %4\n\t"
            "xor      t1, t1, %5\n\t"
            "aes64esm %0, t0, t1\n\t"
            "aes64esm %1, t1, t0\n\t"
            : "=r"(aes_state[0]), "=r"(aes_state[1])
            : "0"(aes_state[0]), "1"(aes_state[1]), "r"(round_keys[2 * i + 1][0]), "r"(round_keys[2 * i + 1][1])
            : "t0", "t1"
      );
      aes_state[0] ^= round_keys[2 * (i + 1)][0];
      aes_state[1] ^= round_keys[2 * (i + 1)][1];
}

void aes_dec_double_round(state_t aes_state, const int &i, uint64_t (*round_keys)[2]) {
      __asm__ (
            "aes64dsm t0, %0, %1\n\t"
            "aes64dsm t1, %1, %0\n\t"
            "xor      t0, t0, %4\n\t"
            "xor      t1, t1, %5\n\t"
            "aes64dsm %0, t0, t1\n\t"
            "aes64dsm %1, t1, t0\n\t"
            : "=r"(aes_state[0]), "=r"(aes_state[1])
            : "0"(aes_state[0]), "1"(aes_state[1]), "r"(round_keys[2 * (i + 1) - 1][0]), "r"(round_keys[2 * (i + 1) - 1][1])
            : "t0", "t1"
      );
      aes_state[0] ^= round_keys[2 * i][0];
      aes_state[1] ^= round_keys[2 * i][1];
}

void aes_enc_round(state_t aes_state, const int &rnd, uint64_t (*round_keys)[2]) {
    __asm__ (
            "aes64esm t0, %0, %1\n\t"
            "aes64esm t1, %1, %0\n\t"
            "xor      %0, t0, %4\n\t"
            "xor      %1, t1, %5\n\t"
            : "=r"(aes_state[0]), "=r"(aes_state[1])
            : "0"(aes_state[0]), "1"(aes_state[1]), "r"(round_keys[rnd][0]), "r"(round_keys[rnd][1])
            : "t0", "t1"
    );
}

void aes_dec_round(state_t aes_state, const int &rnd, uint64_t (*round_keys)[2]) {
    __asm__ (
            "aes64dsm t0, %0, %1\n\t"
            "aes64dsm t1, %1, %0\n\t"
            "xor      %0, t0, %4\n\t"
            "xor      %1, t1, %5\n\t"
            : "=r"(aes_state[0]), "=r"(aes_state[1])
            : "0"(aes_state[0]), "1"(aes_state[1]), "r"(round_keys[rnd][0]), "r"(round_keys[rnd][1])
            : "t0", "t1"
    );
}

void aes_enc_final_round(state_t aes_state, const int &rnd, uint64_t (*round_keys)[2]) {
    __asm__ (
            "aes64es t0, %0, %1\n\t"
            "aes64es t1, %1, %0\n\t"
            "xor     %0, t0, %4\n\t"
            "xor     %1, t1, %5\n\t"
            : "=r"(aes_state[0]), "=r"(aes_state[1])
            : "0"(aes_state[0]), "1"(aes_state[1]), "r"(round_keys[rnd][0]), "r"(round_keys[rnd][1])
            : "t0", "t1"
    );
}

void aes_dec_final_round(state_t aes_state, const int &rnd, uint64_t (*round_keys)[2]) {
    __asm__ (
            "aes64ds t0, %0, %1\n\t"
            "aes64ds t1, %1, %0\n\t"
            "xor     %0, t0, %4\n\t"
            "xor     %1, t1, %5\n\t"
            : "=r"(aes_state[0]), "=r"(aes_state[1])
            : "0"(aes_state[0]), "1"(aes_state[1]), "r"(round_keys[rnd][0]), "r"(round_keys[rnd][1])
            : "t0", "t1"
    );
}

void key_expansion(aes_config_t* config) {

      if (config->size == AES256) {
            uint64_t (*key)[4] = (uint64_t(*)[4]) config->round_keys;

            AES_KEYROUND_256_0(0);
            AES_KEYROUND_256_1(0);
            AES_KEYROUND_256_0(1);
            AES_KEYROUND_256_1(1);
            AES_KEYROUND_256_0(2);
            AES_KEYROUND_256_1(2);
            AES_KEYROUND_256_0(3);
            AES_KEYROUND_256_1(3);
            AES_KEYROUND_256_0(4);
            AES_KEYROUND_256_1(4);
            AES_KEYROUND_256_0(5);
            AES_KEYROUND_256_1(5);
            AES_KEYROUND_256_0(6);
      } else if (config->size == AES192) {
            uint64_t (*key)[3] = (uint64_t(*)[3]) config->round_keys;

            AES_KEYROUND_192(0);
            AES_KEYROUND_192(1);
            AES_KEYROUND_192(2);
            AES_KEYROUND_192(3);
            AES_KEYROUND_192(4);
            AES_KEYROUND_192(5);
            AES_KEYROUND_192(6);
            AES_KEYROUND_192(7);
      } else {
            uint64_t (*key)[2] = (uint64_t(*)[2]) config->round_keys;

            AES_KEYROUND(0);
            AES_KEYROUND(1);
            AES_KEYROUND(2);
            AES_KEYROUND(3);
            AES_KEYROUND(4);
            AES_KEYROUND(5);
            AES_KEYROUND(6);
            AES_KEYROUND(7);
            AES_KEYROUND(8);
            AES_KEYROUND(9);
      }
}

void inv_key_expansion(aes_config_t* config) {
      key_expansion(config);

      for (int i = 2; i < config->Nr * 2; i++) {
                  uint64_t temp = config->round_keys[i];
                  __asm__ (
                        "aes64im %0, %1\n\t"
                        : "=r"(config->round_keys[i])
                        : "r"(temp)
                  );
      }
}

void cipher(aes_config_t* config, state_t aes_state) {
    uint64_t (*round_keys)[2] = (uint64_t(*)[2]) config->round_keys;

    //vx_print_aes_state(aes_state, 0);  
    //Add round key before starting the rounds.
    aes_state[0] ^= round_keys[0][0];
    aes_state[1] ^= round_keys[0][1];

    // vx_print_aes_state(aes_state, 0);

    //NR - 2 round
    for(int i = 0; i < (config->Nr - 1)/2; i++){
      aes_enc_double_round(aes_state, i, round_keys);
      //vx_print_aes_state(aes_state, 2 * (i + 1));  
    }

    // ROUND Nr - 1
    aes_enc_round(aes_state, config->Nr - 1, round_keys);
    //vx_print_aes_state(aes_state, config->Nr - 1); 

    //Final round
    aes_enc_final_round(aes_state, config->Nr, round_keys);
    //vx_print_aes_state(aes_state, config->Nr);
}

void decipher(aes_config_t* config, state_t aes_state) {
    uint64_t (*round_keys)[2] = (uint64_t(*)[2]) config->round_keys;

    //vx_print_aes_state(aes_state, 0);  
    //Add round key before starting the rounds.
    aes_state[0] ^= round_keys[config->Nr][0];
    aes_state[1] ^= round_keys[config->Nr][1];

    // vx_print_aes_state(aes_state, 0);

    //NR - 2 round
    for(int i = (config->Nr - 1)/2; i > 0; i--){
      aes_dec_double_round(aes_state, i, round_keys);
      //vx_print_aes_state(aes_state, 2 * (i + 1));  
    }

    // ROUND 1
    aes_dec_round(aes_state, 1, round_keys);
    //vx_print_aes_state(aes_state, config->Nr - 1); 

    //Final round
    aes_dec_final_round(aes_state, 0, round_keys);
    //vx_print_aes_state(aes_state, config->Nr);
}

void vx_print_aes_state(state_t aes_state, const int &rnd) {
      vx_printf("RND %d State ", rnd);

      for (int b = 0; b < 16; b++) {
            vx_printf("%02x", ((uint8_t*) aes_state)[b]);
            if( b == 7){
            vx_printf(" ");
            }
      }
      vx_printf("\n");
}

void vx_print_block(const uint8_t *b)
{
    for (int i = 0; i < 16; i++)
        vx_printf("%02x ", b[i]);
    vx_printf("\n");
}