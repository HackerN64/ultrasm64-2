#include "n64-util.h"
#include "n64-stdbool.h"

__attribute__((noinline))
static n64_bool n64_enable_interrupts() {
	register n64_bool status;
	asm volatile( ".set noat \n\t .align 16 \n\t mfc0 $1, $12 \n\t andi %0, $1, 1 \n\t ori  $1, $1, 1  \n\t mtc0 $1, $12" : "=r"( status ) :: "at" );
	return status;
}

__attribute__((noinline))
static n64_bool n64_disable_interrupts() {
	register n64_bool status;
	asm volatile( ".set noat \n\t .align 16 \n\t mfc0 $1, $12 \n\t andi %0, $1, 1 \n\t subu $1, $1, %0 \n\t mtc0 $1, $12" : "=r"( status ) :: "at" );
	return status;
}

n64_bool n64_set_interrupts( n64_bool enable ) {
	return enable ? n64_enable_interrupts() : n64_disable_interrupts();
}

void n64_await_pi_io() {
	while( *((volatile unsigned int*)0xa4600010u) & 0x3u );
}
