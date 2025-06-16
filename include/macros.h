#ifndef MACROS_H
#define MACROS_H

// Check for egcs compiler
#if defined (__GNUC__) && (__GNUC__ == 2) && (__GNUC_MINOR__ == 91)
#define EGCS_GCC
#endif

#ifndef _LANGUAGE_ASSEMBLY
#include "platform_info.h"

// Since we are using both compilers to match iQue, ignore these errors.
#ifndef EGCS_GCC

#if !defined(__sgi) && (!defined(NON_MATCHING) || !defined(AVOID_UB))
// asm-process isn't supported outside of IDO, and undefined behavior causes crashes.
#error Matching build is only possible on IDO; please build with NON_MATCHING=1.
#endif

#endif

#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof(arr[0]))

#define GLUE(a, b) a ## b
#define GLUE2(a, b) GLUE(a, b)

// Avoid compiler warnings for unused variables
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#ifdef VERSION_CN
#define UNUSED_CN UNUSED
#else
#define UNUSED_CN
#endif

// Avoid undefined behaviour for non-returning functions
#ifdef __GNUC__
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

// Static assertions
#ifdef __GNUC__
#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#define STATIC_ASSERT(cond, msg) typedef char GLUE2(static_assertion_failed, __LINE__)[(cond) ? 1 : -1]
#endif

// Align to 8-byte boundary for DMA requirements
#if defined(__GNUC__) && !defined(EGCS_GCC)
#define ALIGNED8 __attribute__((aligned(8)))
#else
#define ALIGNED8
#endif

// Align to 16-byte boundary for audio lib requirements
#if defined(__GNUC__) && !defined(EGCS_GCC)
#define ALIGNED16 __attribute__((aligned(16)))
#else
#define ALIGNED16
#endif

#ifndef NO_SEGMENTED_MEMORY
// convert a virtual address to physical.
#define VIRTUAL_TO_PHYSICAL(addr)   ((uintptr_t)(addr) & 0x1FFFFFFF)

// convert a physical address to virtual.
#define PHYSICAL_TO_VIRTUAL(addr)   ((uintptr_t)(addr) | 0x80000000)

// another way of converting virtual to physical
#define VIRTUAL_TO_PHYSICAL2(addr)  ((u8 *)(addr) - 0x80000000U)
#else
// no conversion needed other than cast
#define VIRTUAL_TO_PHYSICAL(addr)   ((uintptr_t)(addr))
#define PHYSICAL_TO_VIRTUAL(addr)   ((uintptr_t)(addr))
#define VIRTUAL_TO_PHYSICAL2(addr)  ((void *)(addr))
#endif

// Stubbed CN debug prints
#ifdef VERSION_CN
#define CN_DEBUG_PRINTF(args) osSyncPrintf args
#else
#define CN_DEBUG_PRINTF(args)
#endif

#ifdef VERSION_CN
#define FORCE_BSS __attribute__((nocommon)) __attribute__((section (".bss_cn")))
#else
#define FORCE_BSS
#endif

#endif

#endif // MACROS_H
