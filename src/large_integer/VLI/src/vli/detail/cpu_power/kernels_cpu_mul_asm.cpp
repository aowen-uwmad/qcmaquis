/*
*Very Large Integer Library, License - Version 1.0 - May 3rd, 2012
*
*Timothee Ewart - University of Geneva, 
*Andreas Hehn - Swiss Federal Institute of technology Zurich.
*
*Permission is hereby granted, free of charge, to any person or organization
*obtaining a copy of the software and accompanying documentation covered by
*this license (the "Software") to use, reproduce, display, distribute,
*execute, and transmit the Software, and to prepare derivative works of the
*Software, and to permit third-parties to whom the Software is furnished to
*do so, all subject to the following:
*
*The copyright notices in the Software and this entire statement, including
*the above license grant, this restriction and the following disclaimer,
*must be included in all copies of the Software, in whole or in part, and
*all derivative works of the Software, unless such copies or derivative
*works are solely in the form of machine-executable object code generated by
*a source language processor.
*
*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
*SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
*FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
*ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*DEALINGS IN THE SOFTWARE.
*/

#include "vli/detail/cpu_power/kernel_implementation_macros.h"
#include <cassert>

// to check :  g++ -E -P -I /BOOST_PATH/include/ -I ../.. vli_number_cpu_function_hooks.hpp | sed  "s/n/;\\`echo -e '\n\r'`/g"  
namespace vli{
    namespace detail{
                     //new functions type : VLI<n*64> *= long int;
                     #define FUNCTION_mul_nbits_64bits(z, n, unused) \
                         void NAME_MUL_NBITS_64BITS(n)(unsigned long int* x, unsigned long int const* y){           \
                         asm(                                                                                       \
                             "xor   6,6,6 \n" \
                             "ld    "R(0)", 0(3)   \n" /* x arg */                                                     \
                             "ld    "R(1)", 0(4)   \n" /* y arg */                                                     \
                             "cmpi 0,0,"R(1)",0                  \n" /* rax is negative ? */             \
                             "bgt 0, "NAME_CONDITIONAL_MUL_NBITS_64BITS(n)"   \n" /* if statements begins */          \
                             "neg   "R(1)","R(1)"                              \n" /* negate the number */             \
                             "addi 6,6,1 \n" \
                             " "NAME_CONDITIONAL_MUL_NBITS_64BITS(n)" :     \n" /* putain de : */                   \
                             "mulld   "R(2)","R(0)","R(1)" \n" /*low part of the product */                           \
                             "mulhdu  "R(3)","R(0)","R(1)" \n" /*low part of the product */                           \
                             BOOST_PP_REPEAT(n, MUL_register, ~)               /* mul algo */                      \
                             "ld      "R(0)", "PPS(1,BOOST_PP_ADD(1,n))"(3) \n"                                       \
                             "mulld   5     ,"R(0)","R(1)" \n "                                                          \
                             "add  "R(BOOST_PP_ADD(3,n))", 5, "R(BOOST_PP_ADD(3,n))"  \n"  \
                             "cmpi 0,0,6,0                  \n" /* rax is negative ? */             \
                             "beq 0, "NAME_RES_CONDITIONAL_MUL_NBITS_64BITS(n)" \n" /* not equal ZF = 0, negate*/       \
                              BOOST_PP_REPEAT(BOOST_PP_ADD(n,2), NOT_register, ~) /* if for final sign */           \
                             "addic. 16,16, 1                 \n  " /* 2cm add 1 */                   \
                              BOOST_PP_REPEAT(BOOST_PP_ADD(n,1), ADC0_register, ~)/* propagate carry bit */         \
                             " "NAME_RES_CONDITIONAL_MUL_NBITS_64BITS(n)" : \n"   /* end final if */                \
                              BOOST_PP_REPEAT(BOOST_PP_ADD(n,2), STORE_register_r3mul,~)                                  \
                              : : :"r5","r6",BOOST_PP_REPEAT(BOOST_PP_ADD(n,4), CLOTHER_register, ~) "memory"   /* clother register*/      \
                             ); \
                         } \
 
                      BOOST_PP_REPEAT(7, FUNCTION_mul_nbits_64bits, ~) // 7 -> expand until 512 !

                      // remark same than x86, look cpu_x86 for details

                       #define BOOST_PP_LOCAL_MACRO(n) \
                          void NAME_MUL_NBITS_NBITS(BOOST_PP_SUB(n,2))(unsigned long int* x, unsigned long int const* y){ \
                         asm(                                                                                       \
                              BOOST_PP_REPEAT(n, MULNTON0, BOOST_PP_SUB(n,1))                                             \
                              BOOST_PP_REPEAT(n, STORE_register_r3mul2,BOOST_PP_ADD(n,2) )                     \
                              : : :"r5","r6","r14","r15",BOOST_PP_REPEAT(n, CLOTHER_register2,n ) "memory"   /* clother register*/      \
                             ); \
                         } \
                     
                       #define BOOST_PP_LOCAL_LIMITS (2, 8)

                       #include BOOST_PP_LOCAL_ITERATE() // the repetition, expand 128 -> 512


                       void mul128_64_64(unsigned long int* x/* %%rdi */, unsigned long int const* y/* %%rsi */, unsigned long int const* z/* %%rdx -> rcx */){
                          asm( 
                              "ld 14, 0(4)     \n"                   
                              "ld 15, 0(5)     \n"                   
                              "mulld  16,14,15 \n"  
                              "std 16 ,0(3)    \n"
                              "mulhdu 16,14,15 \n"  
                              "std 16 ,8(3)    \n"
                              : : :"r14","r15","r16"
                              );
                       }

                       void mul256_128_128(unsigned long int* x/* %%rdi */, unsigned long int const* y/* %%rsi */, unsigned long int const* z/* %%rdx -> rbx */){
   assert(false);
                       }

                      void mul384_192_192(unsigned long int* x/* %%rdi */, unsigned long int const* y/* %%rsi */, unsigned long int const* z/* %%rdx -> rbx */){
   assert(false);
                            }

                     void mul512_256_256(unsigned long int* x/* %%rdi */, unsigned long int const* y/* %%rsi */, unsigned long int const* z/* %%rdx -> rbx */){
   assert(false);
                     }
                    } // end namespace detail
             } // end namespace vli
