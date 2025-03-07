// assembler directives
.set noat      // allow manual use of $at
.set noreorder // don't insert nops after branches

#include "macros.inc"

.section .text, "ax"

glabel entry_point
#ifdef VERSION_CN
    // Get main segment bss address and size
    lui   $t0, %lo(_mainSegmentNoloadStartHi)
    ori   $t0, %lo(_mainSegmentNoloadStartLo)
    lui   $t1, %lo(_mainSegmentNoloadSizeHi)
    ori   $t1, %lo(_mainSegmentNoloadSizeLo)
.clear_bytes:
    // Clear bss section until they are zeroed out
    sw    $zero, ($t0) // Clear 4 bytes
    sw    $zero, 4($t0) // Clear the next 4 bytes
    addi  $t0, $t0, 8 // Increment the address of bytes to clear
    addi  $t1, $t1, -8 // Subtract 8 bytes from the amount remaining
    bnez  $t1, .clear_bytes // Continue clearing until clear_bytes is 0
    nop
    // Get init function and idle thread stack
    lui   $sp, %lo(gIdleThreadStackHi)
    ori   $sp, %lo(gIdleThreadStackLo)
    lui   $t2, %lo(main_funcHi)
    ori   $t2, %lo(main_funcLo)
    jr    $t2 // Jump to the init function
    nop
#else
    // Get main segment bss address and size
    lui   $t0, %hi(_mainSegmentNoloadStart)
    lui   $t1, %lo(_mainSegmentNoloadSizeHi)
    addiu $t0, %lo(_mainSegmentNoloadStart)
    ori   $t1, %lo(_mainSegmentNoloadSizeLo)
.clear_bytes:
    // Clear bss section until they are zeroed out
    addi  $t1, $t1, -8 // Subtract 8 bytes from the amount remaining
    sw    $zero, ($t0) // Clear 4 bytes
    sw    $zero, 4($t0) // Clear the next 4 bytes
    bnez  $t1, .clear_bytes // Continue clearing until clear_bytes is 0
    addi  $t0, $t0, 8 // Increment the address of bytes to clear
    // Get init function and idle thread stack
    lui   $t2, %hi(main_func)
    lui   $sp, %hi(gIdleThreadStack)
    addiu $t2, %lo(main_func)
    jr    $t2 // Jump to the init function
    addiu $sp, %lo(gIdleThreadStack)
#endif
    nop
    nop
    nop
    nop
    nop
    nop
