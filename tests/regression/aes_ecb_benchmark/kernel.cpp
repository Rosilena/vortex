#include <vx_intrinsics.h>
#include <vx_spawn.h>
#include <vx_print.h>
#include <string.h>
#include "common.h"
#include "aes-common.h"

uint64_t RK[Nr+1][Nk / 2] = {0};
uint8_t H[AES_BLOCKLEN]   = {0};
uint8_t J0[AES_BLOCKLEN]  = {0};

void print_aes_state(state_t aes_state, const int &rnd) {
      vx_printf("RND %d State ", rnd);

      for (int b = 0; b < 16; b++) {
            vx_printf("%02x", ((uint8_t*) aes_state)[b]);
            if( b == 7){
            vx_printf(" ");
            }
      }
      vx_printf("\n");
}

void print_block(const char *name, const uint8_t *b)
{
    vx_printf("%s: ", name);
    for (int i = 0; i < 16; i++)
        vx_printf("%02x ", b[i]);
    vx_printf("\n");
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

void aes_double_round(state_t aes_state, const int &i, uint64_t round_keys[Nr + 1][2]) {
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

void aes_round(state_t aes_state, const int &rnd, uint64_t round_keys[Nr + 1][2]) {
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

void aes_final_round(state_t aes_state, const int &rnd, uint64_t round_keys[Nr + 1][2]) {
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

void keyExpansion(uint64_t key[Nr+1][Nk / 2]) {

#if defined(AES256) && (AES256 == 1)
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
#elif (defined(AES192) && (AES192 == 1))
      AES_KEYROUND_192(0);
      AES_KEYROUND_192(1);
      AES_KEYROUND_192(2);
      AES_KEYROUND_192(3);
      AES_KEYROUND_192(4);
      AES_KEYROUND_192(5);
      AES_KEYROUND_192(6);
      AES_KEYROUND_192(7);
#else
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
#endif

    //   for(int i=0; i<Nr+1; i++) {
    //         uint64_t key_be_low  = AES_GET_BE64((uint8_t*) key, 16 * i);
    //         uint64_t key_be_high = AES_GET_BE64((uint8_t*) key, 16 * i + 8);

    //         vx_printf("key %d ", i);
    //         vx_printf(" 0x%016lx " "0x%016lx \n", key_be_low, key_be_high);
    //   }
}

void Cipher(state_t aes_state, void* rk_pointer) {
    uint64_t (*round_keys)[2] = (uint64_t(*)[2]) rk_pointer;

    //print_aes_state(aes_state, 0);  
    //Add round key before starting the rounds.
    aes_state[0] ^= round_keys[0][0];
    aes_state[1] ^= round_keys[0][1];

    // print_aes_state(aes_state, 0);

    //NR - 2 round
    for(int i = 0; i < (Nr - 1)/2; i++){
      aes_double_round(aes_state, i, round_keys);
      //print_aes_state(aes_state, 2 * (i + 1));  
    }

    // ROUND Nr - 1
    aes_round(aes_state, Nr - 1, round_keys);
    //print_aes_state(aes_state, Nr - 1); 

    //Final round
    aes_final_round(aes_state, Nr, round_keys);
    //print_aes_state(aes_state, Nr);
}

void inc32(uint8_t *block)
{
 	aes_uint val;
 	val = AES_GET_BE32(block + AES_BLOCKLEN - 4);
 	val++;
 	AES_PUT_BE32(block + AES_BLOCKLEN - 4, val);
}

void inc32(uint8_t *block, uint32_t amnt)
{
 	aes_uint val;
 	val = AES_GET_BE32(block + AES_BLOCKLEN - 4);
 	val += amnt;
 	AES_PUT_BE32(block + AES_BLOCKLEN - 4, val);
}

void aes128(void* rk_pointer, uint8_t* in, size_t length, size_t threadIdx, size_t workgroup_size)
{
      if (threadIdx >= length / AES_BLOCKLEN)
            return;

      //vx_printf("length %d", length);     
      //int k = 0;
      for (int i = 0; workgroup_size * i + threadIdx < length / AES_BLOCKLEN; i += 1) {
            int j = workgroup_size * i + threadIdx;
            //k++;
            //vx_printf("Executing block %d thread %d - length %d\n", j, threadIdx, length / AES_BLOCKLEN);
            //vx_printf("Executing block %d thread %d warp %d \n", j, threadIdx, vx_warp_id());
            Cipher((uint64_t*) &(in[j * AES_BLOCKLEN]), rk_pointer);
      }
    //vx_printf("Executed %d iterations on %d work threads\n", k, workgroup_size);
}

void kernel_body(kernel_arg_t* __UNIFORM__ arg) {
    uint64_t *pt            = (uint64_t*) arg->in_addr;
    uint64_t *ct            = (uint64_t*) arg->out_addr;
    uint8_t  *key_ptr       = (uint8_t* ) arg->key_addr;

    size_t index            = blockIdx.x * blockDim.x + threadIdx.x;
    size_t workgroup_size   = arg->grid_dim * arg->block_dim;
    
    //if (index >= workgroup_size) return;
    //vx_printf("Grid Dimension %d Block Dimension %d Core id: %d\n", arg->grid_dim, arg->block_dim, vx_core_id());
    
    //vx_printf("length %d", arg->size_in);
    aes128(RK, (uint8_t*) pt, arg->size_in, index, workgroup_size);
    
    return;
}


int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      //primo parametro: dimensione (1,2,3) sarebbe x,y e z
      //secondo parametro: dimensione griglia (in questo caso array) sarebbe numero di blocchi presenti nella griglia
      //terzo parametro: dimensione blocco sarebbe numero di thread per blocco

      uint64_t *key_first     = (uint64_t*)arg->key_addr;

      if (vx_core_id() == 0) {
            for (int i = 0; i < Nk / 2; i++) {
            RK[i / (Nk / 2)][i % (Nk / 2)] = key_first[i];
            //   vx_printf("key_first[%d] = 0x%016lx \n", i, key_first[i]);
            //   vx_printf("RK[%d][%d] = key_first[%d] \n", i / (Nk / 2), i % (Nk / 2), i);
            }

            keyExpansion(RK);

            //vx_printf("Grid Dimension %d Block Dimension %d Core id: %d\n", arg->grid_dim, arg->block_dim, vx_core_id());
      }

      return vx_spawn_threads(1, &arg->grid_dim, &arg->block_dim, (vx_kernel_func_cb)kernel_body, arg);
}
