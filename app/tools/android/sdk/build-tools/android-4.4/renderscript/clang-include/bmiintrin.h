/*===---- bmiintrin.h - BMI intrinsics -------------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */

#if !defined __X86INTRIN_H && !defined __IMMINTRIN_H
#error "Never use <bmiintrin.h> directly; include <x86intrin.h> instead."
#endif

#ifndef __BMI__
# error "BMI instruction set not enabled"
#endif /* __BMI__ */

#ifndef __BMIINTRIN_H
#define __BMIINTRIN_H

static __inline__ unsigned short __attribute__((__always_inline__, __nodebug__))
__tzcnt_u16(unsigned short __X)
{
  return __builtin_ctzs(__X);
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
__andn_u32(unsigned int __X, unsigned int __Y)
{
  return ~__X & __Y;
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
__bextr_u32(unsigned int __X, unsigned int __Y)
{
  return __builtin_ia32_bextr_u32(__X, __Y);
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
__blsi_u32(unsigned int __X)
{
  return __X & -__X;
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
__blsmsk_u32(unsigned int __X)
{
  return __X ^ (__X - 1);
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
__blsr_u32(unsigned int __X)
{
  return __X & (__X - 1);
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
__tzcnt_u32(unsigned int __X)
{
  return __builtin_ctz(__X);
}

#ifdef __x86_64__
static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__))
__andn_u64 (unsigned long long __X, unsigned long long __Y)
{
  return ~__X & __Y;
}

static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__))
__bextr_u64(unsigned long long __X, unsigned long long __Y)
{
  return __builtin_ia32_bextr_u64(__X, __Y);
}

static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__))
__blsi_u64(unsigned long long __X)
{
  return __X & -__X;
}

static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__))
__blsmsk_u64(unsigned long long __X)
{
  return __X ^ (__X - 1);
}

static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__))
__blsr_u64(unsigned long long __X)
{
  return __X & (__X - 1);
}

static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__))
__tzcnt_u64(unsigned long long __X)
{
  return __builtin_ctzll(__X);
}
#endif

#endif /* __BMIINTRIN_H */
