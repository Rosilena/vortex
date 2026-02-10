#include <vx_intrinsics.h>
#include "common.h"


int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
	test_type* dst_ptr = (test_type*)arg->dst_addr;

  volatile test_type a = 0x1;
  volatile test_type b = 0x1;
  volatile test_type c = 0;
  
      __asm__ (
            "ror %0, %1, %2"
            : "=r"(c)
            : "r"(a), "r"(b)
      );

    dst_ptr[0] = c;
    #if 0
      __asm__ (
             "rol %0, %1, %2"
             : "=r"(c)
             : "r"(a), "r"(b)
      );

      dst_ptr[1] = c;
      
        __asm__ (
              "rori %0, %1, %2"
              : "=r"(c)
              : "r"(a), "i"(4)
        );

      dst_ptr[2] = c;

      #ifdef XLEN_64
          __asm__ (
                "rorw %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

          dst_ptr[15] = c;

          __asm__ (
                "rolw %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

          dst_ptr[16] = c;

          __asm__ (
                "roriw %0, %1, %2"
                : "=r"(c)
                : "r"(a), "i"(4)
          );

          dst_ptr[17] = c;
      #endif

      __asm__ (
            "pack %0, %1, %2"
            : "=r"(c)
            : "r"(a), "r"(b)
      );
      dst_ptr[3] = c;
      

      __asm__ (
              "packh %0, %1, %2"
              : "=r"(c)
              : "r"(a), "r"(b)
        );

      dst_ptr[4] = c;

      #ifdef XLEN_64
          __asm__ (
                  "packw %0, %1, %2"
                  : "=r"(c)
                  : "r"(a), "r"(b)
            );

          dst_ptr[18] = c;
      #endif

        __asm__ (
              "brev8 %0, %1"
              : "=r"(c)
              : "r"(a)
        );

      dst_ptr[5] = c;

        __asm__ (
              "rev8 %0, %1"
              : "=r"(c)
              : "r"(a)
        );

      dst_ptr[6] = c;

      #ifdef XLEN_32
          __asm__ (
                  "zip %0, %1"
                  : "=r"(c)
                  : "r"(a)
            );

          dst_ptr[15] = c;

            __asm__ (
                  "unzip %0, %1"
                  : "=r"(c)
                  : "r"(a)
            );

          dst_ptr[16] = c;
      #endif


      __asm__ (
              "clmul %0, %1, %2"
              : "=r"(c)
              : "r"(a), "r"(b)
        );

      dst_ptr[7] = c;

        __asm__ (
              "clmulh %0, %1, %2"
              : "=r"(c)
              : "r"(a), "r"(b)
        );

      dst_ptr[8] = c;

        __asm__ (
              "xperm8 %0, %1, %2"
              : "=r"(c)
              : "r"(a), "r"(b)
        );

      dst_ptr[9] = c;

        __asm__ (
              "xperm4 %0, %1, %2"
              : "=r"(c)
              : "r"(a), "r"(b)
        );

      dst_ptr[10] = c; 
      
      #ifdef XLEN_32
          __asm__ (
                "aes32dsi %0, %1, %2, %3"
                : "=r"(c)
                : "r"(a), "r"(b), "i"(3)
          );

        dst_ptr[17] = c;

        __asm__ (
                "aes32dsmi %0, %1, %2, %3"
                : "=r"(c)
                : "r"(a), "r"(b), "i"(2)
          );
          dst_ptr[18] = c;
      #endif

      #ifdef XLEN_64
        __asm__ (
                "aes64ds %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );
          dst_ptr[19] = c;

        __asm__ (
                "aes64dsm %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );
          dst_ptr[20] = c;

        __asm__ (
                "aes64im %0, %1"
                : "=r"(c)
                : "r"(a)
        );
        dst_ptr[21] = c;

        __asm__ (
                "aes64ks1i %0, %1, %2"
                : "=r"(c)
                : "r"(a), "i"(5)
          );
          dst_ptr[22] = c;

        __asm__ (
                "aes64ks2 %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

          dst_ptr[23] = c;
      #endif

      #ifdef XLEN_32
          __asm__ (
                "aes32esi %0, %1, %2, %3"
                : "=r"(c)
                : "r"(a), "r"(b), "i"(2)
          );

        dst_ptr[19] = c;

        __asm__ (
                "aes32esmi %0, %1, %2, %3"
                : "=r"(c)
                : "r"(a), "r"(b), "i"(3)
          );

        dst_ptr[20] = c;
      #endif

      #ifdef XLEN_64
        __asm__ (
                "aes64es %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );
          dst_ptr[24] = c;

        __asm__ (
                "aes64esm %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );
          dst_ptr[25] = c;
      #endif

      __asm__ (
                "sha256sig0 %0, %1"
                : "=r"(c)
                : "r"(a)
      );
      dst_ptr[11] = c;

      __asm__ (
                "sha256sig1 %0, %1"
                : "=r"(c)
                : "r"(a)
      );
      dst_ptr[12] = c;

      __asm__ (
                "sha256sum0 %0, %1"
                : "=r"(c)
                : "r"(a)
      );
      dst_ptr[13] = c;

      __asm__ (
                "sha256sum1 %0, %1"
                : "=r"(c)
                : "r"(a)
      );
      dst_ptr[14] = c;

      #ifdef XLEN_32
          __asm__ (
                "sha512sig0h %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

          dst_ptr[21] = c;

          __asm__ (
                  "sha512sig0l %0, %1, %2"
                  : "=r"(c)
                  : "r"(a), "r"(b)
            );

          dst_ptr[22] = c;

            __asm__ (
                  "sha512sig1h %0, %1, %2"
                  : "=r"(c)
                  : "r"(a), "r"(b)
            );

          dst_ptr[23] = c;

          __asm__ (
                  "sha512sig1l %0, %1, %2"
                  : "=r"(c)
                  : "r"(a), "r"(b)
            );

          dst_ptr[24] = c;

            __asm__ (
                  "sha512sum0r %0, %1, %2"
                  : "=r"(c)
                  : "r"(a), "r"(b)
            );

          dst_ptr[25] = c;

          __asm__ (
                  "sha512sum1r %0, %1, %2"
                  : "=r"(c)
                  : "r"(a), "r"(b)
            );

          dst_ptr[26] = c;

        #endif

      #ifdef XLEN_64

          __asm__ (
                  "sha512sig0 %0, %1"
                  : "=r"(c)
                  : "r"(a)
            );
            dst_ptr[26] = c;

          __asm__ (
                  "sha512sig1 %0, %1"
                  : "=r"(c)
                  : "r"(a)
            );
            dst_ptr[27] = c;

          __asm__ (
                  "sha512sum0 %0, %1"
                  : "=r"(c)
                  : "r"(a)
            );
            dst_ptr[28] = c;
            

          __asm__ (
                  "sha512sum1 %0, %1"
                  : "=r"(c)
                  : "r"(a)
            );
            dst_ptr[29] = c;
      #endif
  #endif
	
	return 0;
}
