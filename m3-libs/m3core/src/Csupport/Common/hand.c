/* Copyright (C) 1992, Digital Equipment Corporation        */
/* All rights reserved.                                     */
/* See the file COPYRIGHT for a full description.           */
/*                                                          */
/* Last modified on Thu Feb  1 09:36:52 PST 1996 by heydon  */
/*      modified on Tue Jan 10 15:48:28 PST 1995 by kalsow  */
/*      modified on Tue Feb 11 15:18:40 PST 1992 by muller  */

#ifdef _MSC_VER
#pragma warning(disable:4255) /* () changed to (void) */
#pragma warning(disable:4505) /* unused static function removed */
#pragma warning(disable:4711) /* automatic inlining */
#undef _DLL
#ifndef _MT
#define _MT
#endif
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef long long int64;
typedef unsigned long long uint64;
#endif

#include <limits.h>
#include <string.h>
#include <stddef.h>

typedef int BOOL;

#ifdef __cplusplus
extern "C"
{           
#endif

#if !defined(_MSC_VER) && !defined(__stdcall)
#define __stdcall /* nothing */
#endif

#if (UINT_MAX <= 0xFFFF) || (UINT_MAX != 0xFFFFFFFF) || (UINT_MAX != 0xFFFFFFFFUL)
#error uint is not 32bits
#endif
typedef unsigned uint32;
typedef int int32;

/* There are problems passing int64 in K&R form! */
#if 1 /* defined(__STDC__) || defined(__cplusplus) || defined(_MSC_VER) */
#define ANSI(x) x
#define KR(x)
#else
#define ANSI(x)
#define KR(x) x
#endif

#ifdef _WIN32

/* return positive form of a negative value, avoiding overflow */
/* T should be an unsigned type */
#define M3_POS(T, a) (((T)-((a) + 1)) + 1)

int64 __stdcall m3_div64(int64 b, int64 a)
{
  typedef  int64 ST; /* signed type */
  typedef uint64 UT; /* unsigned type */
  int aneg = (a < 0);
  int bneg = (b < 0);
  if (aneg == bneg || a == 0 || b == 0)
    return (a / b);
  else
  {
    /* round negative result down by rounding positive result up
       unsigned math is much better defined, see gcc -Wstrict-overflow=4 */
    UT ua = (aneg ? M3_POS(UT, a) : (UT)a);
    UT ub = (bneg ? M3_POS(UT, b) : (UT)b);
    return -(ST)((ua + ub - 1) / ub);
  }
}

int64 __stdcall m3_mod64(int64 b, int64 a)
{
  typedef  int64 ST; /* signed type */
  typedef uint64 UT; /* unsigned type */
  int aneg = (a < 0);
  int bneg = (b < 0);
  if (aneg == bneg || a == 0 || b == 0)
    return (a % b);
  else
  {
    UT ua = (aneg ? M3_POS(UT, a) : (UT)a);
    UT ub = (bneg ? M3_POS(UT, b) : (UT)b);
    a = (ST)(ub - 1 - (ua + ub - 1) % ub);
    return (bneg ? -a : a);
  }
}

#endif

#define SET_GRAIN (sizeof (size_t) * 8)

void __stdcall set_union
    ANSI((                 size_t n_bits, size_t* c, size_t* b, size_t* a))
      KR((n_bits, c, b, a) size_t n_bits; size_t* c; size_t* b; size_t* a;)
{
  register size_t n_words = n_bits / SET_GRAIN;
  register size_t i;
  for (i = 0; i < n_words; i++) {
    a[i] = b[i] | c[i];
  }
}

void __stdcall set_intersection
    ANSI((                 size_t n_bits, size_t* c, size_t* b, size_t* a))
      KR((n_bits, c, b, a) size_t n_bits; size_t* c; size_t* b; size_t* a;)
{
  register size_t n_words = n_bits / SET_GRAIN;
  register size_t i;
  for (i = 0; i < n_words; i++) {
    a[i] = b[i] & c[i];
  }
}

void __stdcall set_difference
    ANSI((                 size_t n_bits, size_t* c, size_t* b, size_t* a))
      KR((n_bits, c, b, a) size_t n_bits; size_t* c; size_t* b; size_t* a;)
{
  register size_t n_words = n_bits / SET_GRAIN;
  register size_t i;
  for (i = 0; i < n_words; i++) {
    a[i] = b[i] & (~ c[i]);
  }
}

void __stdcall set_sym_difference
    ANSI((                 size_t n_bits, size_t* c, size_t* b, size_t* a))
      KR((n_bits, c, b, a) size_t n_bits; size_t* c; size_t* b; size_t* a;)
{
  register size_t n_words = n_bits / SET_GRAIN;
  register size_t i;
  for (i = 0; i < n_words; i++) {
    a[i] = b[i] ^ c[i];
  }
}

size_t __stdcall set_ge
    ANSI((              size_t n_bits, size_t* b, size_t* a))
      KR((n_bits, b, a) size_t n_bits; size_t* b; size_t* a;)
{
  register size_t n_words = n_bits / SET_GRAIN;
  register size_t i;
  for (i = 0; i < n_words; i++) {
    if ((~ a[i]) & b[i]) return 0;
  }
  return 1;
}

size_t __stdcall set_gt
    ANSI((              size_t n_bits, size_t* b, size_t* a))
      KR((n_bits, b, a) size_t n_bits; size_t* b; size_t* a;)
{
  register size_t n_words = n_bits / SET_GRAIN;
  register size_t i;
  register size_t eq = 0;
  for (i = 0; i < n_words; i++) {
    if ((~ a[i]) & b[i]) return 0;
    eq |=  (a[i] ^ b[i]);
  }
  return (eq != 0);
}

size_t __stdcall set_le
    ANSI((              size_t n_bits, size_t* b, size_t* a))
      KR((n_bits, b, a) size_t n_bits; size_t* b; size_t* a;)
{
  register size_t n_words = n_bits / SET_GRAIN;
  register size_t i;
  for (i = 0; i < n_words; i++) {
    if (a[i] & (~ b[i])) return 0;
  }
  return 1;
}

size_t __stdcall set_lt
    ANSI((              size_t n_bits, size_t* b, size_t* a))
      KR((n_bits, b, a) size_t n_bits; size_t* b; size_t* a;)
{
  register size_t n_words = n_bits / SET_GRAIN;
  register size_t i;
  register size_t eq = 0;
  for (i = 0; i < n_words; i++) {
    if (a[i] & (~ b[i])) return 0;
    eq |= (a[i] ^ b[i]);
  }
  return (eq != 0);
}

#define HIGH_BITS(a) ((~(size_t)0) << (a))
#define LOW_BITS(a)  ((~(size_t)0) >> (SET_GRAIN - (a) - 1))

void __stdcall set_range
    ANSI((       size_t b, size_t a, size_t* s))
    KR((b, a, s) size_t b; size_t a; size_t* s;)
{
  if (b < a) {
      /* no bits to set */
  } else {
    size_t a_word = a / SET_GRAIN;
    size_t b_word = b / SET_GRAIN;
    size_t i;
    size_t high_bits = HIGH_BITS(a % SET_GRAIN);
    size_t low_bits = LOW_BITS(b % SET_GRAIN);

    if (a_word == b_word) {
      s [a_word] |= (high_bits & low_bits);
    } else {
      s [a_word] |= high_bits;
      for (i = a_word + 1; i < b_word; ++i)
        s[i] = ~(size_t)0;
      s [b_word] |= low_bits;
    }
  }
}

#ifdef _WIN32

/* Several functions are "missing" here because we
 * call the C compiler helper functions directly.
 * e.g. multiply, unsigned div/mod, shift left/right.
 * Also some operations are generated inline, e.g. add, subtract, compare, and, or, xor, not, neg, abs.
 */

uint64 _rotl64(uint64 value, int shift);
uint64 _rotr64(uint64 value, int shift);
#pragma intrinsic(_rotl64)
#pragma intrinsic(_rotr64)
uint64 __stdcall  m3_rotate_left64(uint64 a, uint32 b)  { return _rotl64(a, (int)b); }
uint64 __stdcall m3_rotate_right64(uint64 a, uint32 b)  { return _rotr64(a, (int)b); }

uint64 __stdcall m3_rotate64(uint64 a, int b)
{
    b &= 63;
    if (b > 0)
        a = _rotl64(a, b);
    else if (b < 0)
        a = _rotr64(a, -b);
    return a;
}

#endif /* WIN32 */

#ifdef __cplusplus
} /* extern "C" */
#endif
