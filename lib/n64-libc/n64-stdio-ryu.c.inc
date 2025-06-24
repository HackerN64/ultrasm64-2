// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

// NOTICE:
// This file was adapted from the original Apache licensed code, whose sources
// may be found at:
// https://github.com/ulfjack/ryu/blob/1264a946ba66eab320e927bfd2362e0c8580c42f/ryu/d2s.c
// https://github.com/ulfjack/ryu/blob/1264a946ba66eab320e927bfd2362e0c8580c42f/ryu/f2s.c
// https://github.com/ulfjack/ryu/blob/1264a946ba66eab320e927bfd2362e0c8580c42f/ryu/d2s_small_table.h
// https://github.com/ulfjack/ryu/blob/1264a946ba66eab320e927bfd2362e0c8580c42f/ryu/f2s_full_table.h
// https://github.com/ulfjack/ryu/blob/1264a946ba66eab320e927bfd2362e0c8580c42f/ryu/d2s_intrinsics.h
// https://github.com/ulfjack/ryu/blob/1264a946ba66eab320e927bfd2362e0c8580c42f/ryu/f2s_intrinsics.h
// https://github.com/ulfjack/ryu/blob/1264a946ba66eab320e927bfd2362e0c8580c42f/ryu/common.h
//
// The files have been merged into this single file, all functions and debug
// code that were not necessary for this library were stripped out, and all
// explicit integer length types were swapped out for C keywords to remove the
// dependency on stdint.h.
// The f2d and d2d functions were also renamed to ryu_float_to_decimal and
// ryu_double_to_decimal for the sake of clarity.

#include "n64-stdbool.h"

#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_EXPONENT_BITS 11
#define DOUBLE_BIAS 1023

#define FLOAT_MANTISSA_BITS 23
#define FLOAT_EXPONENT_BITS 8
#define FLOAT_BIAS 127

#define FLOAT_POW5_INV_BITCOUNT 59
#define FLOAT_POW5_BITCOUNT 61

#define DOUBLE_POW5_INV_BITCOUNT 125
#define DOUBLE_POW5_BITCOUNT 125

#define POW5_TABLE_SIZE 26

#define FLOAT_POW5_INV_BITCOUNT 59
#define FLOAT_POW5_BITCOUNT 61

static unsigned long long FLOAT_POW5_INV_SPLIT[55] = {
  576460752303423489ull,  461168601842738791ull,  368934881474191033ull,  295147905179352826ull,
  472236648286964522ull,  377789318629571618ull,  302231454903657294ull,  483570327845851670ull,
  386856262276681336ull,  309485009821345069ull,  495176015714152110ull,  396140812571321688ull,
  316912650057057351ull,  507060240091291761ull,  405648192073033409ull,  324518553658426727ull,
  519229685853482763ull,  415383748682786211ull,  332306998946228969ull,  531691198313966350ull,
  425352958651173080ull,  340282366920938464ull,  544451787073501542ull,  435561429658801234ull,
  348449143727040987ull,  557518629963265579ull,  446014903970612463ull,  356811923176489971ull,
  570899077082383953ull,  456719261665907162ull,  365375409332725730ull,  292300327466180584ull,
  467680523945888934ull,  374144419156711148ull,  299315535325368918ull,  478904856520590269ull,
  383123885216472215ull,  306499108173177772ull,  490398573077084435ull,  392318858461667548ull,
  313855086769334039ull,  502168138830934462ull,  401734511064747569ull,  321387608851798056ull,
  514220174162876889ull,  411376139330301511ull,  329100911464241209ull,  526561458342785934ull,
  421249166674228747ull,  336999333339382998ull,  539198933343012796ull,  431359146674410237ull,
  345087317339528190ull,  552139707743245103ull,  441711766194596083ull
};
static const unsigned long long FLOAT_POW5_SPLIT[47] = {
  1152921504606846976ull, 1441151880758558720ull, 1801439850948198400ull, 2251799813685248000ull,
  1407374883553280000ull, 1759218604441600000ull, 2199023255552000000ull, 1374389534720000000ull,
  1717986918400000000ull, 2147483648000000000ull, 1342177280000000000ull, 1677721600000000000ull,
  2097152000000000000ull, 1310720000000000000ull, 1638400000000000000ull, 2048000000000000000ull,
  1280000000000000000ull, 1600000000000000000ull, 2000000000000000000ull, 1250000000000000000ull,
  1562500000000000000ull, 1953125000000000000ull, 1220703125000000000ull, 1525878906250000000ull,
  1907348632812500000ull, 1192092895507812500ull, 1490116119384765625ull, 1862645149230957031ull,
  1164153218269348144ull, 1455191522836685180ull, 1818989403545856475ull, 2273736754432320594ull,
  1421085471520200371ull, 1776356839400250464ull, 2220446049250313080ull, 1387778780781445675ull,
  1734723475976807094ull, 2168404344971008868ull, 1355252715606880542ull, 1694065894508600678ull,
  2117582368135750847ull, 1323488980084844279ull, 1654361225106055349ull, 2067951531382569187ull,
  1292469707114105741ull, 1615587133892632177ull, 2019483917365790221ull
};

static const unsigned long long DOUBLE_POW5_INV_SPLIT2[15][2] = {
  {                    1ull, 2305843009213693952ull },
  {  5955668970331000884ull, 1784059615882449851ull },
  {  8982663654677661702ull, 1380349269358112757ull },
  {  7286864317269821294ull, 2135987035920910082ull },
  {  7005857020398200553ull, 1652639921975621497ull },
  { 17965325103354776697ull, 1278668206209430417ull },
  {  8928596168509315048ull, 1978643211784836272ull },
  { 10075671573058298858ull, 1530901034580419511ull },
  {   597001226353042382ull, 1184477304306571148ull },
  {  1527430471115325346ull, 1832889850782397517ull },
  { 12533209867169019542ull, 1418129833677084982ull },
  {  5577825024675947042ull, 2194449627517475473ull },
  { 11006974540203867551ull, 1697873161311732311ull },
  { 10313493231639821582ull, 1313665730009899186ull },
  { 12701016819766672773ull, 2032799256770390445ull }
};
static const unsigned int POW5_INV_OFFSETS[19] = {
  0x54544554u, 0x04055545u, 0x10041000u, 0x00400414u, 0x40010000u, 0x41155555u,
  0x00000454u, 0x00010044u, 0x40000000u, 0x44000041u, 0x50454450u, 0x55550054u,
  0x51655554u, 0x40004000u, 0x01000001u, 0x00010500u, 0x51515411u, 0x05555554u,
  0x00000000u
};

static const unsigned long long DOUBLE_POW5_SPLIT2[13][2] = {
  {                    0ull, 1152921504606846976ull },
  {                    0ull, 1490116119384765625ull },
  {  1032610780636961552ull, 1925929944387235853ull },
  {  7910200175544436838ull, 1244603055572228341ull },
  { 16941905809032713930ull, 1608611746708759036ull },
  { 13024893955298202172ull, 2079081953128979843ull },
  {  6607496772837067824ull, 1343575221513417750ull },
  { 17332926989895652603ull, 1736530273035216783ull },
  { 13037379183483547984ull, 2244412773384604712ull },
  {  1605989338741628675ull, 1450417759929778918ull },
  {  9630225068416591280ull, 1874621017369538693ull },
  {   665883850346957067ull, 1211445438634777304ull },
  { 14931890668723713708ull, 1565756531257009982ull }
};
static const unsigned int POW5_OFFSETS[21] = {
  0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x40000000u, 0x59695995u,
  0x55545555u, 0x56555515u, 0x41150504u, 0x40555410u, 0x44555145u, 0x44504540u,
  0x45555550u, 0x40004000u, 0x96440440u, 0x55565565u, 0x54454045u, 0x40154151u,
  0x55559155u, 0x51405555u, 0x00000105u
};


static const unsigned long long DOUBLE_POW5_TABLE[POW5_TABLE_SIZE] = {
  1ull, 5ull, 25ull, 125ull, 625ull, 3125ull, 15625ull, 78125ull, 390625ull,
  1953125ull, 9765625ull, 48828125ull, 244140625ull, 1220703125ull, 6103515625ull,
  30517578125ull, 152587890625ull, 762939453125ull, 3814697265625ull,
  19073486328125ull, 95367431640625ull, 476837158203125ull,
  2384185791015625ull, 11920928955078125ull, 59604644775390625ull,
  298023223876953125ull //, 1490116119384765625ull
};

// Returns e == 0 ? 1 : [log_2(5^e)]; requires 0 <= e <= 3528.
static inline int log2pow5(const int e) {
  // This approximation works up to the point that the multiplication overflows at e = 3529.
  // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
  // than 2^9297.
  return (int) ((((unsigned int) e) * 1217359u) >> 19);
}

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 3528.
static inline int pow5bits(const int e) {
  // This approximation works up to the point that the multiplication overflows at e = 3529.
  // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
  // than 2^9297.
  return (int) (((((unsigned int) e) * 1217359u) >> 19) + 1u);
}

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 3528.
static inline int ceil_log2pow5(const int e) {
  return log2pow5(e) + 1;
}

// Returns floor(log_10(2^e)); requires 0 <= e <= 1650.
static inline unsigned int log10Pow2(const int e) {
  // The first value this approximation fails for is 2^1651 which is just greater than 10^297.
  return (((unsigned int) e) * 78913u) >> 18;
}

// Returns floor(log_10(5^e)); requires 0 <= e <= 2620.
static inline unsigned int log10Pow5(const int e) {
  // The first value this approximation fails for is 5^2621 which is just greater than 10^1832.
  return (((unsigned int) e) * 732923u) >> 20;
}

// Returns the number of decimal digits in v, which must not contain more than 9 digits.
static inline unsigned int decimalLength9(const unsigned int v) {
  // Function precondition: v is not a 10-digit number.
  // (f2s: 9 digits are sufficient for round-tripping.)
  // (d2fixed: We print 9-digit blocks.)
  if (v >= 100000000u) { return 9u; }
  if (v >= 10000000u) { return 8u; }
  if (v >= 1000000u) { return 7u; }
  if (v >= 100000u) { return 6u; }
  if (v >= 10000u) { return 5u; }
  if (v >= 1000u) { return 4u; }
  if (v >= 100u) { return 3u; }
  if (v >= 10u) { return 2u; }
  return 1;
}

static inline unsigned int decimalLength17(const unsigned long long v) {
  // This is slightly faster than a loop.
  // The average output length is 16.38 digits, so we check high-to-low.
  // Function precondition: v is not an 18, 19, or 20-digit number.
  // (17 digits are sufficient for round-tripping.)
  if (v >= 10000000000000000ull) { return 17u; }
  if (v >= 1000000000000000ull) { return 16u; }
  if (v >= 100000000000000ull) { return 15u; }
  if (v >= 10000000000000ull) { return 14u; }
  if (v >= 1000000000000ull) { return 13u; }
  if (v >= 100000000000ull) { return 12u; }
  if (v >= 10000000000ull) { return 11u; }
  if (v >= 1000000000ull) { return 10u; }
  if (v >= 100000000ull) { return 9u; }
  if (v >= 10000000ull) { return 8u; }
  if (v >= 1000000ull) { return 7u; }
  if (v >= 100000ull) { return 6u; }
  if (v >= 10000ull) { return 5u; }
  if (v >= 1000ull) { return 4u; }
  if (v >= 100ull) { return 3u; }
  if (v >= 10ull) { return 2u; }
  return 1;
}

static inline unsigned long long umul128(const unsigned long long a, const unsigned long long b, unsigned long long* const productHi) {
  const unsigned int aLo = (unsigned int)a;
  const unsigned int aHi = (unsigned int)(a >> 32);
  const unsigned int bLo = (unsigned int)b;
  const unsigned int bHi = (unsigned int)(b >> 32);

  const unsigned long long b00 = (unsigned long long)aLo * bLo;
  const unsigned long long b01 = (unsigned long long)aLo * bHi;
  const unsigned long long b10 = (unsigned long long)aHi * bLo;
  const unsigned long long b11 = (unsigned long long)aHi * bHi;

  const unsigned int b00Lo = (unsigned int)b00;
  const unsigned int b00Hi = (unsigned int)(b00 >> 32);

  const unsigned long long mid1 = b10 + b00Hi;
  const unsigned int mid1Lo = (unsigned int)(mid1);
  const unsigned int mid1Hi = (unsigned int)(mid1 >> 32);

  const unsigned long long mid2 = b01 + mid1Lo;
  const unsigned int mid2Lo = (unsigned int)(mid2);
  const unsigned int mid2Hi = (unsigned int)(mid2 >> 32);

  const unsigned long long pHi = b11 + mid1Hi + mid2Hi;
  const unsigned long long pLo = ((unsigned long long)mid2Lo << 32) | b00Lo;

  *productHi = pHi;
  return pLo;
}

static inline unsigned long long shiftright128(const unsigned long long lo, const unsigned long long hi, const unsigned int dist) {
  // We don't need to handle the case dist >= 64 here (see above).
  return (hi << (64u - dist)) | (lo >> dist);
}

// Computes 5^i in the form required by Ryu, and stores it in the given pointer.
static inline void double_computePow5(const unsigned int i, unsigned long long* const result) {
  const unsigned int base = i / POW5_TABLE_SIZE;
  const unsigned int base2 = base * POW5_TABLE_SIZE;
  const unsigned int offset = i - base2;
  const unsigned long long* const mul = DOUBLE_POW5_SPLIT2[base];
  if (offset == 0) {
    result[0] = mul[0];
    result[1] = mul[1];
    return;
  }
  const unsigned long long m = DOUBLE_POW5_TABLE[offset];
  unsigned long long high1;
  const unsigned long long low1 = umul128(m, mul[1], &high1);
  unsigned long long high0;
  const unsigned long long low0 = umul128(m, mul[0], &high0);
  const unsigned long long sum = high0 + low1;
  if (sum < high0) {
    ++high1; // overflow into high1
  }
  // high1 | sum | low0
  const unsigned int delta = pow5bits(i) - pow5bits(base2);
  result[0] = shiftright128(low0, sum, delta) + ((POW5_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
  result[1] = shiftright128(sum, high1, delta);
}

// Computes 5^-i in the form required by Ryu, and stores it in the given pointer.
static inline void double_computeInvPow5(const unsigned int i, unsigned long long* const result) {
  const unsigned int base = (i + POW5_TABLE_SIZE - 1) / POW5_TABLE_SIZE;
  const unsigned int base2 = base * POW5_TABLE_SIZE;
  const unsigned int offset = base2 - i;
  const unsigned long long* const mul = DOUBLE_POW5_INV_SPLIT2[base]; // 1/5^base2
  if (offset == 0) {
    result[0] = mul[0];
    result[1] = mul[1];
    return;
  }
  const unsigned long long m = DOUBLE_POW5_TABLE[offset];
  unsigned long long high1;
  const unsigned long long low1 = umul128(m, mul[1], &high1);
  unsigned long long high0;
  const unsigned long long low0 = umul128(m, mul[0] - 1, &high0);
  const unsigned long long sum = high0 + low1;
  if (sum < high0) {
    ++high1; // overflow into high1
  }
  // high1 | sum | low0
  const unsigned int delta = pow5bits(base2) - pow5bits(i);
  result[0] = shiftright128(low0, sum, delta) + 1 + ((POW5_INV_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3u);
  result[1] = shiftright128(sum, high1, delta);
}

#ifdef _ABIO32

// Returns the high 64 bits of the 128-bit product of a and b.
__attribute__((always_inline))
static inline unsigned long long umulh(const unsigned long long a, const unsigned long long b) {
  // Reuse the umul128 implementation.
  // Optimizers will likely eliminate the instructions used to compute the
  // low part of the product.
  unsigned long long hi;
  umul128(a, b, &hi);
  return hi;
}

// On 32-bit platforms, compilers typically generate calls to library
// functions for 64-bit divisions, even if the divisor is a constant.
//
// E.g.:
// https://bugs.llvm.org/show_bug.cgi?id=37932
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=17958
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=37443
//
// The functions here perform division-by-constant using multiplications
// in the same way as 64-bit compilers would do.
//
// NB:
// The multipliers and shift values are the ones generated by clang x64
// for expressions like x/5, x/10, etc.
__attribute__((always_inline))
static inline unsigned long long div5(const unsigned long long x) {
  return umulh(x, 0xCCCCCCCCCCCCCCCDull) >> 2;
}

__attribute__((always_inline))
static inline unsigned long long div10(const unsigned long long x) {
  return umulh(x, 0xCCCCCCCCCCCCCCCDull) >> 3;
}

__attribute__((always_inline))
static inline unsigned long long div100(const unsigned long long x) {
  return umulh(x >> 2, 0x28F5C28F5C28F5C3ull) >> 2;
}

__attribute__((always_inline))
static inline unsigned long long div1e8(const unsigned long long x) {
  return umulh(x, 0xABCC77118461CEFDull) >> 26;
}

__attribute__((always_inline))
static inline unsigned long long div1e9(const unsigned long long x) {
  return umulh(x >> 9, 0x44B82FA09B5A53ull) >> 11;
}

__attribute__((always_inline))
static inline unsigned int mod1e9(const unsigned long long x) {
  // Avoid 64-bit math as much as possible.
  // Returning (uint32_t) (x - 1000000000 * div1e9(x)) would
  // perform 32x64-bit multiplication and 64-bit subtraction.
  // x and 1000000000 * div1e9(x) are guaranteed to differ by
  // less than 10^9, so their highest 32 bits must be identical,
  // so we can truncate both sides to uint32_t before subtracting.
  // We can also simplify (uint32_t) (1000000000 * div1e9(x)).
  // We can truncate before multiplying instead of after, as multiplying
  // the highest 32 bits of div1e9(x) can't affect the lowest 32 bits.
  return ((unsigned int) x) - 1000000000u * ((unsigned int) div1e9(x));
}

#else

__attribute__((always_inline))
static inline unsigned long long div5(const unsigned long long x) {
  return x / 5ull;
}

__attribute__((always_inline))
static inline unsigned long long div10(const unsigned long long x) {
  return x / 10ull;
}

__attribute__((always_inline))
static inline unsigned long long div100(const unsigned long long x) {
  return x / 100ull;
}

__attribute__((always_inline))
static inline unsigned long long div1e8(const unsigned long long x) {
  return x / 100000000ull;
}

__attribute__((always_inline))
static inline unsigned long long div1e9(const unsigned long long x) {
  return x / 1000000000ull;
}

__attribute__((always_inline))
static inline unsigned int mod1e9(const unsigned long long x) {
  return (unsigned int) (x - 1000000000ull * div1e9(x));
}

#endif

static inline unsigned int pow5Factor(unsigned long long value) {
  const unsigned long long m_inv_5 = 14757395258967641293ull; // 5 * m_inv_5 = 1 (mod 2^64)
  const unsigned long long n_div_5 = 3689348814741910323ull;  // #{ n | n = 0 (mod 2^64) } = 2^64 / 5
  unsigned int count = 0u;
  for (;;) {
    value *= m_inv_5;
    if (value > n_div_5)
      break;
    ++count;
  }
  return count;
}

// Returns true if value is divisible by 5^p.
static inline n64_bool multipleOfPowerOf5(const unsigned long long value, const unsigned int p) {
  // I tried a case distinction on p, but there was no performance difference.
  return pow5Factor(value) >= p;
}

// Returns true if value is divisible by 2^p.
static inline n64_bool multipleOfPowerOf2(const unsigned long long value, const unsigned int p) {
  // __builtin_ctzll doesn't appear to be faster here.
  return (value & ((1ull << p) - 1)) == 0;
}

// We need a 64x128-bit multiplication and a subsequent 128-bit shift.
// Multiplication:
//   The 64-bit factor is variable and passed in, the 128-bit factor comes
//   from a lookup table. We know that the 64-bit factor only has 55
//   significant bits (i.e., the 9 topmost bits are zeros). The 128-bit
//   factor only has 124 significant bits (i.e., the 4 topmost bits are
//   zeros).
// Shift:
//   In principle, the multiplication result requires 55 + 124 = 179 bits to
//   represent. However, we then shift this value to the right by j, which is
//   at least j >= 115, so the result is guaranteed to fit into 179 - 115 = 64
//   bits. This means that we only need the topmost 64 significant bits of
//   the 64x128-bit multiplication.
//
// There are several ways to do this:
// 1. Best case: the compiler exposes a 128-bit type.
//    We perform two 64x64-bit multiplications, add the higher 64 bits of the
//    lower result to the higher result, and shift by j - 64 bits.
//
//    We explicitly cast from 64-bit to 128-bit, so the compiler can tell
//    that these are only 64-bit inputs, and can map these to the best
//    possible sequence of assembly instructions.
//    x64 machines happen to have matching assembly instructions for
//    64x64-bit multiplications and 128-bit shifts.
//
// 2. Second best case: the compiler exposes intrinsics for the x64 assembly
//    instructions mentioned in 1.
//
// 3. We only have 64x64 bit instructions that return the lower 64 bits of
//    the result, i.e., we have to use plain C.
//    Our inputs are less than the full width, so we have three options:
//    a. Ignore this fact and just implement the intrinsics manually.
//    b. Split both into 31-bit pieces, which guarantees no internal overflow,
//       but requires extra work upfront (unless we change the lookup table).
//    c. Split only the first factor into 31-bit pieces, which also guarantees
//       no internal overflow, but requires extra work since the intermediate
//       results are not perfectly aligned.
static inline unsigned long long mulShift64(const unsigned long long m, const unsigned* const mul, const int j) {
  // m is maximum 55 bits
  unsigned long long high1;                                   // 128
  const unsigned long long low1 = umul128(m, mul[1], &high1); // 64
  unsigned long long high0;                                   // 64
  umul128(m, mul[0], &high0);                       // 0
  const unsigned long long sum = high0 + low1;
  if (sum < high0) {
    ++high1; // overflow into high1
  }
  return shiftright128(sum, high1, j - 64);
}

// This is faster if we don't have a 64x64->128-bit multiplication.
static inline unsigned long long mulShiftAll64(unsigned long long m, const unsigned long long* const mul, const int j,
  unsigned long long* const vp, unsigned long long* const vm, const unsigned int mmShift) {
  m <<= 1;
  // m is maximum 55 bits
  unsigned long long tmp;
  const unsigned long long lo = umul128(m, mul[0], &tmp);
  unsigned long long hi;
  const unsigned long long mid = tmp + umul128(m, mul[1], &hi);
  hi += mid < tmp; // overflow into hi

  const unsigned long long lo2 = lo + mul[0];
  const unsigned long long mid2 = mid + mul[1] + (lo2 < lo);
  const unsigned long long hi2 = hi + (mid2 < mid);
  *vp = shiftright128(mid2, hi2, (unsigned int) (j - 64 - 1));

  if (mmShift == 1) {
    const unsigned long long lo3 = lo - mul[0];
    const unsigned long long mid3 = mid - mul[1] - (lo3 > lo);
    const unsigned long long hi3 = hi - (mid3 > mid);
    *vm = shiftright128(mid3, hi3, (unsigned int) (j - 64 - 1));
  } else {
    const unsigned long long lo3 = lo + lo;
    const unsigned long long mid3 = mid + mid + (lo3 < lo);
    const unsigned long long hi3 = hi + hi + (mid3 < mid);
    const unsigned long long lo4 = lo3 - mul[0];
    const unsigned long long mid4 = mid3 - mul[1] - (lo4 > lo3);
    const unsigned long long hi4 = hi3 - (mid4 > mid3);
    *vm = shiftright128(mid4, hi4, (unsigned int) (j - 64));
  }

  return shiftright128(mid, hi, (unsigned int) (j - 64 - 1));
}

static inline unsigned int pow5factor_32(unsigned int value) {
  unsigned int count = 0;
  for (;;) {
    const unsigned int q = value / 5;
    const unsigned int r = value % 5;
    if (r != 0) {
      break;
    }
    value = q;
    ++count;
  }
  return count;
}

// Returns true if value is divisible by 5^p.
static inline n64_bool multipleOfPowerOf5_32(const unsigned int value, const unsigned int p) {
  return pow5factor_32(value) >= p;
}

// Returns true if value is divisible by 2^p.
static inline n64_bool multipleOfPowerOf2_32(const unsigned int value, const unsigned int p) {
  // __builtin_ctz doesn't appear to be faster here.
  return (value & ((1u << p) - 1)) == 0;
}

// It seems to be slightly faster to avoid uint128_t here, although the
// generated code for uint128_t looks slightly nicer.
static inline unsigned int mulShift32(const unsigned int m, const unsigned long long factor, const int shift) {

  // The casts here help MSVC to avoid calls to the __allmul library
  // function.
  const unsigned int factorLo = (unsigned int)(factor);
  const unsigned int factorHi = (unsigned int)(factor >> 32);
  const unsigned long long bits0 = (unsigned long long)m * factorLo;
  const unsigned long long bits1 = (unsigned long long)m * factorHi;

#ifdef _ABIO32
  // On 32-bit platforms we can avoid a 64-bit shift-right since we only
  // need the upper 32 bits of the result and the shift value is > 32.
  const unsigned int bits0Hi = (unsigned int)(bits0 >> 32);
  unsigned int bits1Lo = (unsigned int)(bits1);
  unsigned int bits1Hi = (unsigned int)(bits1 >> 32);
  bits1Lo += bits0Hi;
  bits1Hi += (bits1Lo < bits0Hi);
  if (shift >= 64) {
    // s2f can call this with a shift value >= 64, which we have to handle.
    // This could now be slower than the !defined(RYU_32_BIT_PLATFORM) case.
    return (unsigned int)(bits1Hi >> (shift - 64));
  } else {
    const int s = shift - 32;
    return (bits1Hi << (32 - s)) | (bits1Lo >> s);
  }
#else
  const unsigned long long sum = (bits0 >> 32) + bits1;
  const unsigned long long shiftedSum = sum >> (shift - 32);
  return (unsigned int) shiftedSum;
#endif
}

static inline unsigned int mulPow5InvDivPow2(const unsigned int m, const unsigned int q, const int j) {
  return mulShift32(m, FLOAT_POW5_INV_SPLIT[q], j);
}

static inline unsigned int mulPow5divPow2(const unsigned int m, const unsigned int i, const int j) {
  return mulShift32(m, FLOAT_POW5_SPLIT[i], j);
}

// A floating decimal representing m * 10^e.
typedef struct {
  unsigned int mantissa;
  // Decimal exponent's range is -45 to 38
  // inclusive, and can fit in a short if needed.
  int exponent;
} ryu_floating_decimal_32;

// A floating decimal representing m * 10^e.
typedef struct {
  unsigned long long mantissa;
  // Decimal exponent's range is -324 to 308
  // inclusive, and can fit in a short if needed.
  int exponent;
} ryu_floating_decimal_64;

static ryu_floating_decimal_32 ryu_float_to_decimal(const unsigned int ieeeMantissa, const unsigned int ieeeExponent) {
  int e2;
  unsigned int m2;
  if (ieeeExponent == 0) {
    if (ieeeMantissa == 0u) {
      const ryu_floating_decimal_32 fd = { 0u, 0 };
      return fd;
    }
    // We subtract 2 so that the bounds computation has 2 additional bits.
    e2 = 1 - FLOAT_BIAS - FLOAT_MANTISSA_BITS - 2;
    m2 = ieeeMantissa;
  } else {
    e2 = (int) ieeeExponent - FLOAT_BIAS - FLOAT_MANTISSA_BITS - 2;
    m2 = (1u << FLOAT_MANTISSA_BITS) | ieeeMantissa;
  }
  const n64_bool even = (m2 & 1) == 0;
  const n64_bool acceptBounds = even;

  // Step 2: Determine the interval of valid decimal representations.
  const unsigned int mv = 4 * m2;
  const unsigned int mp = 4 * m2 + 2;
  // Implicit bool -> int conversion. True is 1, false is 0.
  const unsigned int mmShift = ieeeMantissa != 0 || ieeeExponent <= 1;
  const unsigned int mm = 4 * m2 - 1 - mmShift;

  // Step 3: Convert to a decimal power base using 64-bit arithmetic.
  unsigned int vr, vp, vm;
  int e10;
  n64_bool vmIsTrailingZeros = false;
  n64_bool vrIsTrailingZeros = false;
  unsigned char lastRemovedDigit = 0;
  if (e2 >= 0) {
    const unsigned int q = log10Pow2(e2);
    e10 = (int) q;
    const int k = FLOAT_POW5_INV_BITCOUNT + pow5bits((int) q) - 1;
    const int i = -e2 + (int) q + k;
    vr = mulPow5InvDivPow2(mv, q, i);
    vp = mulPow5InvDivPow2(mp, q, i);
    vm = mulPow5InvDivPow2(mm, q, i);
    if (q != 0 && (vp - 1) / 10 <= vm / 10) {
      // We need to know one removed digit even if we are not going to loop below. We could use
      // q = X - 1 above, except that would require 33 bits for the result, and we've found that
      // 32-bit arithmetic is faster even on 64-bit machines.
      const int l = FLOAT_POW5_INV_BITCOUNT + pow5bits((int) (q - 1)) - 1;
      lastRemovedDigit = (unsigned char) (mulPow5InvDivPow2(mv, q - 1, -e2 + (int) q - 1 + l) % 10);
    }
    if (q <= 9) {
      // The largest power of 5 that fits in 24 bits is 5^10, but q <= 9 seems to be safe as well.
      // Only one of mp, mv, and mm can be a multiple of 5, if any.
      if (mv % 5 == 0) {
        vrIsTrailingZeros = multipleOfPowerOf5_32(mv, q);
      } else if (acceptBounds) {
        vmIsTrailingZeros = multipleOfPowerOf5_32(mm, q);
      } else {
        vp -= multipleOfPowerOf5_32(mp, q);
      }
    }
  } else {
    const unsigned int q = log10Pow5(-e2);
    e10 = (int) q + e2;
    const int i = -e2 - (int) q;
    const int k = pow5bits(i) - FLOAT_POW5_BITCOUNT;
    int j = (int) q - k;
    vr = mulPow5divPow2(mv, (unsigned int) i, j);
    vp = mulPow5divPow2(mp, (unsigned int) i, j);
    vm = mulPow5divPow2(mm, (unsigned int) i, j);
    if (q != 0 && (vp - 1) / 10 <= vm / 10) {
      j = (int) q - 1 - (pow5bits(i + 1) - FLOAT_POW5_BITCOUNT);
      lastRemovedDigit = (unsigned char) (mulPow5divPow2(mv, (unsigned int) (i + 1), j) % 10);
    }
    if (q <= 1) {
      // {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q trailing 0 bits.
      // mv = 4 * m2, so it always has at least two trailing 0 bits.
      vrIsTrailingZeros = true;
      if (acceptBounds) {
        // mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
        vmIsTrailingZeros = mmShift == 1;
      } else {
        // mp = mv + 2, so it always has at least one trailing 0 bit.
        --vp;
      }
    } else if (q < 31) {
      vrIsTrailingZeros = multipleOfPowerOf2_32(mv, q - 1);
    }
  }

  // Step 4: Find the shortest decimal representation in the interval of valid representations.
  int removed = 0;
  unsigned int output;
  if (vmIsTrailingZeros || vrIsTrailingZeros) {
    // General case, which happens rarely (~4.0%).
    while (vp / 10 > vm / 10) {
      vmIsTrailingZeros &= vm % 10 == 0;
      vrIsTrailingZeros &= lastRemovedDigit == 0;
      lastRemovedDigit = (unsigned char) (vr % 10);
      vr /= 10;
      vp /= 10;
      vm /= 10;
      ++removed;
    }
    if (vmIsTrailingZeros) {
      while (vm % 10 == 0) {
        vrIsTrailingZeros &= lastRemovedDigit == 0;
        lastRemovedDigit = (unsigned char) (vr % 10);
        vr /= 10;
        vp /= 10;
        vm /= 10;
        ++removed;
      }
    }
    if (vrIsTrailingZeros && lastRemovedDigit == 5 && vr % 2 == 0) {
      // Round even if the exact number is .....50..0.
      lastRemovedDigit = 4;
    }
    // We need to take vr + 1 if vr is outside bounds or we need to round up.
    output = vr + ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || lastRemovedDigit >= 5);
  } else {
    // Specialized for the common case (~96.0%). Percentages below are relative to this.
    // Loop iterations below (approximately):
    // 0: 13.6%, 1: 70.7%, 2: 14.1%, 3: 1.39%, 4: 0.14%, 5+: 0.01%
    while (vp / 10 > vm / 10) {
      lastRemovedDigit = (unsigned char) (vr % 10);
      vr /= 10;
      vp /= 10;
      vm /= 10;
      ++removed;
    }
    // We need to take vr + 1 if vr is outside bounds or we need to round up.
    output = vr + (vr == vm || lastRemovedDigit >= 5);
  }
  const int exp = e10 + removed;

  ryu_floating_decimal_32 fd;
  fd.exponent = exp;
  fd.mantissa = output;
  return fd;
}

static ryu_floating_decimal_64 ryu_double_to_decimal(const unsigned long long ieeeMantissa, const unsigned int ieeeExponent) {
  int e2;
  unsigned long long m2;
  if (ieeeExponent == 0) {
    if (ieeeMantissa == 0ull) {
      const ryu_floating_decimal_64 fd = { 0ull, 0 };
      return fd;
    }
    // We subtract 2 so that the bounds computation has 2 additional bits.
    e2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
    m2 = ieeeMantissa;
  } else {
    e2 = (int) ieeeExponent - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
    m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
  }
  const n64_bool even = (m2 & 1) == 0;
  const n64_bool acceptBounds = even;

  // Step 2: Determine the interval of valid decimal representations.
  const unsigned long long mv = 4 * m2;
  // Implicit bool -> int conversion. True is 1, false is 0.
  const unsigned int mmShift = ieeeMantissa != 0 || ieeeExponent <= 1;
  // We would compute mp and mm like this:
  // uint64_t mp = 4 * m2 + 2;
  // uint64_t mm = mv - 1 - mmShift;

  // Step 3: Convert to a decimal power base using 128-bit arithmetic.
  unsigned long long vr, vp, vm;
  int e10;
  n64_bool vmIsTrailingZeros = false;
  n64_bool vrIsTrailingZeros = false;
  if (e2 >= 0) {
    // I tried special-casing q == 0, but there was no effect on performance.
    // This expression is slightly faster than max(0, log10Pow2(e2) - 1).
    const unsigned int q = log10Pow2(e2) - (e2 > 3);
    e10 = (int) q;
    const int k = DOUBLE_POW5_INV_BITCOUNT + pow5bits((int) q) - 1;
    const int i = -e2 + (int) q + k;
    unsigned long long pow5[2];
    double_computeInvPow5(q, pow5);
    vr = mulShiftAll64(m2, pow5, i, &vp, &vm, mmShift);
    if (q <= 21) {
      // This should use q <= 22, but I think 21 is also safe. Smaller values
      // may still be safe, but it's more difficult to reason about them.
      // Only one of mp, mv, and mm can be a multiple of 5, if any.
      const unsigned int mvMod5 = ((unsigned int) mv) - 5 * ((unsigned int) div5(mv));
      if (mvMod5 == 0) {
        vrIsTrailingZeros = multipleOfPowerOf5(mv, q);
      } else if (acceptBounds) {
        // Same as min(e2 + (~mm & 1), pow5Factor(mm)) >= q
        // <=> e2 + (~mm & 1) >= q && pow5Factor(mm) >= q
        // <=> true && pow5Factor(mm) >= q, since e2 >= q.
        vmIsTrailingZeros = multipleOfPowerOf5(mv - 1 - mmShift, q);
      } else {
        // Same as min(e2 + 1, pow5Factor(mp)) >= q.
        vp -= multipleOfPowerOf5(mv + 2, q);
      }
    }
  } else {
    // This expression is slightly faster than max(0, log10Pow5(-e2) - 1).
    const unsigned int q = log10Pow5(-e2) - (-e2 > 1);
    e10 = (int) q + e2;
    const int i = -e2 - (int) q;
    const int k = pow5bits(i) - DOUBLE_POW5_BITCOUNT;
    const int j = (int) q - k;
    unsigned long long pow5[2];
    double_computePow5(i, pow5);
    vr = mulShiftAll64(m2, pow5, j, &vp, &vm, mmShift);
    if (q <= 1) {
      // {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q trailing 0 bits.
      // mv = 4 * m2, so it always has at least two trailing 0 bits.
      vrIsTrailingZeros = true;
      if (acceptBounds) {
        // mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
        vmIsTrailingZeros = mmShift == 1;
      } else {
        // mp = mv + 2, so it always has at least one trailing 0 bit.
        --vp;
      }
    } else if (q < 63) {
      // We want to know if the full product has at least q trailing zeros.
      // We need to compute min(p2(mv), p5(mv) - e2) >= q
      // <=> p2(mv) >= q && p5(mv) - e2 >= q
      // <=> p2(mv) >= q (because -e2 >= q)
      vrIsTrailingZeros = multipleOfPowerOf2(mv, q);
    }
  }

  // Step 4: Find the shortest decimal representation in the interval of valid representations.
  int removed = 0;
  unsigned char lastRemovedDigit = 0;
  unsigned long long output;
  // On average, we remove ~2 digits.
  if (vmIsTrailingZeros || vrIsTrailingZeros) {
    // General case, which happens rarely (~0.7%).
    for (;;) {
      const unsigned long long vpDiv10 = div10(vp);
      const unsigned long long vmDiv10 = div10(vm);
      if (vpDiv10 <= vmDiv10) {
        break;
      }
      const unsigned int vmMod10 = ((unsigned int) vm) - 10 * ((unsigned int) vmDiv10);
      const unsigned long long vrDiv10 = div10(vr);
      const unsigned int vrMod10 = ((unsigned int) vr) - 10 * ((unsigned int) vrDiv10);
      vmIsTrailingZeros &= vmMod10 == 0;
      vrIsTrailingZeros &= lastRemovedDigit == 0;
      lastRemovedDigit = (unsigned char) vrMod10;
      vr = vrDiv10;
      vp = vpDiv10;
      vm = vmDiv10;
      ++removed;
    }
    if (vmIsTrailingZeros) {
      for (;;) {
        const unsigned long long vmDiv10 = div10(vm);
        const unsigned int vmMod10 = ((unsigned int) vm) - 10 * ((unsigned int) vmDiv10);
        if (vmMod10 != 0) {
          break;
        }
        const unsigned long long vpDiv10 = div10(vp);
        const unsigned long long vrDiv10 = div10(vr);
        const unsigned int vrMod10 = ((unsigned int) vr) - 10 * ((unsigned int) vrDiv10);
        vrIsTrailingZeros &= lastRemovedDigit == 0;
        lastRemovedDigit = (unsigned char) vrMod10;
        vr = vrDiv10;
        vp = vpDiv10;
        vm = vmDiv10;
        ++removed;
      }
    }
    if (vrIsTrailingZeros && lastRemovedDigit == 5 && vr % 2 == 0) {
      // Round even if the exact number is .....50..0.
      lastRemovedDigit = 4;
    }
    // We need to take vr + 1 if vr is outside bounds or we need to round up.
    output = vr + ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || lastRemovedDigit >= 5);
  } else {
    // Specialized for the common case (~99.3%). Percentages below are relative to this.
    n64_bool roundUp = false;
    const unsigned long long vpDiv100 = div100(vp);
    const unsigned long long vmDiv100 = div100(vm);
    if (vpDiv100 > vmDiv100) { // Optimization: remove two digits at a time (~86.2%).
      const unsigned long long vrDiv100 = div100(vr);
      const unsigned int vrMod100 = ((unsigned int) vr) - 100 * ((unsigned int) vrDiv100);
      roundUp = vrMod100 >= 50;
      vr = vrDiv100;
      vp = vpDiv100;
      vm = vmDiv100;
      removed += 2;
    }
    // Loop iterations below (approximately), without optimization above:
    // 0: 0.03%, 1: 13.8%, 2: 70.6%, 3: 14.0%, 4: 1.40%, 5: 0.14%, 6+: 0.02%
    // Loop iterations below (approximately), with optimization above:
    // 0: 70.6%, 1: 27.8%, 2: 1.40%, 3: 0.14%, 4+: 0.02%
    for (;;) {
      const unsigned long long vpDiv10 = div10(vp);
      const unsigned long long vmDiv10 = div10(vm);
      if (vpDiv10 <= vmDiv10) {
        break;
      }
      const unsigned long long vrDiv10 = div10(vr);
      const unsigned int vrMod10 = ((unsigned int) vr) - 10 * ((unsigned int) vrDiv10);
      roundUp = vrMod10 >= 5;
      vr = vrDiv10;
      vp = vpDiv10;
      vm = vmDiv10;
      ++removed;
    }
    // We need to take vr + 1 if vr is outside bounds or we need to round up.
    output = vr + (vr == vm || roundUp);
  }
  const int exp = e10 + removed;

  ryu_floating_decimal_64 fd;
  fd.exponent = exp;
  fd.mantissa = output;
  return fd;
}
