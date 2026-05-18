#ifndef AES_H
#define AES_H

#include "aes_common.h"
#include <stdint.h>

aes_config_t aes_init(AES_SIZE size, uint64_t* round_keys, bool encrypt);
void key_expansion(aes_config_t* config);
void inv_key_expansion(aes_config_t* config);
void cipher(aes_config_t* config, state_t aes_state);
void decipher(aes_config_t* config, state_t aes_state);
void vx_print_aes_state(state_t aes_state, const int &rnd);
void vx_print_block(const uint8_t *b);

#endif // AES_H