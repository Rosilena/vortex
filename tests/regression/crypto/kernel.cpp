#include <vx_intrinsics.h>
#include "common.h"


int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      test_type* src0_ptr = (test_type*)arg->src0_addr;
      test_type* src1_ptr = (test_type*)arg->src1_addr;
	test_type* dst_ptr = (test_type*)arg->dst_addr;
      
      __asm__ (
            "ror %0, %1, %2"
            : "=r"(dst_ptr[0])
            : "r"(src0_ptr[0]), "r"(src1_ptr[0])
      );
    
      __asm__ (
             "rol %0, %1, %2"
             : "=r"(dst_ptr[1])
             : "r"(src0_ptr[1]), "r"(src1_ptr[1])
      );
      
        __asm__ (
              "rori %0, %1, %2"
              : "=r"(dst_ptr[2])
              : "r"(src0_ptr[2]), "i"(4)
        );

      #ifdef XLEN_64
          __asm__ (
                "rorw %0, %1, %2"
                : "=r"(dst_ptr[15])
                : "r"(src0_ptr[15]), "r"(src1_ptr[15])
          );

          __asm__ (
                "rolw %0, %1, %2"
                : "=r"(dst_ptr[16])
                : "r"(src0_ptr[16]), "r"(src1_ptr[16])
          );

          __asm__ (
                "roriw %0, %1, %2"
                : "=r"(dst_ptr[17])
                : "r"(src0_ptr[17]), "i"(6)
          );

      #endif

      __asm__ (
            "pack %0, %1, %2"
            : "=r"(dst_ptr[3])
            : "r"(src0_ptr[3]), "r"(src1_ptr[3])
      );
      
      __asm__ (
              "packh %0, %1, %2"
              : "=r"(dst_ptr[4])
              : "r"(src0_ptr[4]), "r"(src1_ptr[4])
        );

      #ifdef XLEN_64
          __asm__ (
                  "packw %0, %1, %2"
                  : "=r"(dst_ptr[18])
                  : "r"(src0_ptr[18]), "r"(src1_ptr[18])
            );

      #endif

        __asm__ (
              "brev8 %0, %1"
              : "=r"(dst_ptr[5])
              : "r"(src0_ptr[5])
        );


        __asm__ (
              "rev8 %0, %1"
              : "=r"(dst_ptr[6])
              : "r"(src0_ptr[6])
        );

      #ifdef XLEN_32
          __asm__ (
                  "zip %0, %1"
                  : "=r"(dst_ptr[15])
                  : "r"(src0_ptr[15])
            );

            __asm__ (
                  "unzip %0, %1"
                  : "=r"(dst_ptr[16])
                  : "r"(src0_ptr[16])
            );

      #endif


      __asm__ (
              "clmul %0, %1, %2"
              : "=r"(dst_ptr[7])
              : "r"(src0_ptr[7]), "r"(src1_ptr[7])
        );


        __asm__ (
              "clmulh %0, %1, %2"
              : "=r"(dst_ptr[8])
              : "r"(src0_ptr[8]), "r"(src1_ptr[8])
        );

        __asm__ (
              "xperm8 %0, %1, %2"
              : "=r"(dst_ptr[9])
              : "r"(src0_ptr[9]), "r"(src1_ptr[9])
        );

        __asm__ (
              "xperm4 %0, %1, %2"
              : "=r"(dst_ptr[10])
              : "r"(src0_ptr[10]), "r"(src1_ptr[10])
        );

      
      #ifdef XLEN_32
          __asm__ (
                "aes32dsi %0, %1, %2, %3"
                : "=r"(dst_ptr[17])
                : "r"(src0_ptr[17]), "r"(src1_ptr[17]), "i"(3)
          );

        __asm__ (
                "aes32dsmi %0, %1, %2, %3"
                : "=r"(dst_ptr[18])
                : "r"(src0_ptr[18]), "r"(src1_ptr[18]), "i"(2)
          );
      #endif

      #ifdef XLEN_64
        __asm__ (
                "aes64ds %0, %1, %2"
                : "=r"(dst_ptr[19])
                : "r"(src0_ptr[19]), "r"(src1_ptr[19])
          );

        __asm__ (
                "aes64dsm %0, %1, %2"
                : "=r"(dst_ptr[20])
                : "r"(src0_ptr[20]), "r"(src1_ptr[20])
          );

        __asm__ (
                "aes64im %0, %1"
                : "=r"(dst_ptr[21])
                : "r"(src0_ptr[21])
        );

        __asm__ (
                "aes64ks1i %0, %1, %2"
                : "=r"(dst_ptr[22])
                : "r"(src0_ptr[22]), "i"(5)
          );

        __asm__ (
                "aes64ks2 %0, %1, %2"
                : "=r"(dst_ptr[23])
                : "r"(src0_ptr[23]), "r"(src1_ptr[23])
          );

      #endif

      #ifdef XLEN_32
          __asm__ (
                "aes32esi %0, %1, %2, %3"
                : "=r"(dst_ptr[19])
                : "r"(src0_ptr[19]), "r"(src1_ptr[19]), "i"(2)
          );

        __asm__ (
                "aes32esmi %0, %1, %2, %3"
                : "=r"(dst_ptr[20])
                : "r"(src0_ptr[20]), "r"(src1_ptr[20]), "i"(3)
          );

      #endif

      #ifdef XLEN_64
        __asm__ (
                "aes64es %0, %1, %2"
                : "=r"(dst_ptr[24])
                : "r"(src0_ptr[24]), "r"(src1_ptr[24])
          );

        __asm__ (
                "aes64esm %0, %1, %2"
                : "=r"(dst_ptr[25])
                : "r"(src0_ptr[25]), "r"(src1_ptr[25])
          );
      #endif

      __asm__ (
                "sha256sig0 %0, %1"
                : "=r"(dst_ptr[11])
                : "r"(src0_ptr[11])
      );
      __asm__ (
                "sha256sig1 %0, %1"
                : "=r"(dst_ptr[12])
                : "r"(src0_ptr[12])
      );

      __asm__ (
                "sha256sum0 %0, %1"
                : "=r"(dst_ptr[13])
                : "r"(src0_ptr[13])
      );

      __asm__ (
                "sha256sum1 %0, %1"
                : "=r"(dst_ptr[14])
                : "r"(src0_ptr[14])
      );

      #ifdef XLEN_32
          __asm__ (
                "sha512sig0h %0, %1, %2"
                : "=r"(dst_ptr[21])
                : "r"(src0_ptr[21]), "r"(src1_ptr[21])
          );


          __asm__ (
                  "sha512sig0l %0, %1, %2"
                  : "=r"(dst_ptr[22])
                  : "r"(src0_ptr[22]), "r"(src1_ptr[22])
            );


            __asm__ (
                  "sha512sig1h %0, %1, %2"
                  : "=r"(dst_ptr[23])
                  : "r"(src0_ptr[23]), "r"(src1_ptr[23])
            );

          __asm__ (
                  "sha512sig1l %0, %1, %2"
                  : "=r"(dst_ptr[24])
                  : "r"(src0_ptr[24]), "r"(src1_ptr[24])
            );


            __asm__ (
                  "sha512sum0r %0, %1, %2"
                  : "=r"(dst_ptr[25])
                  : "r"(src0_ptr[25]), "r"(src1_ptr[25])
            );

          __asm__ (
                  "sha512sum1r %0, %1, %2"
                  : "=r"(dst_ptr[26])
                  : "r"(src0_ptr[26]), "r"(src1_ptr[26])
            );


        #endif

      #ifdef XLEN_64

          __asm__ (
                  "sha512sig0 %0, %1"
                  : "=r"(dst_ptr[26])
                  : "r"(src0_ptr[26])
            );

          __asm__ (
                  "sha512sig1 %0, %1"
                  : "=r"(dst_ptr[27])
                  : "r"(src0_ptr[27])
            );

          __asm__ (
                  "sha512sum0 %0, %1"
                  : "=r"(dst_ptr[28])
                  : "r"(src0_ptr[28])
            );
            
          __asm__ (
                  "sha512sum1 %0, %1"
                  : "=r"(dst_ptr[29])
                  : "r"(src0_ptr[29])
            );
      #endif
	return 0;
}
