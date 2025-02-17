#ifdef _LANGUAGE_ASSEMBLY
#undef _LANGUAGE_C
#endif

.macro glabel label
    .global \label
    .balign 4
    \label:
.endm

.macro gsymbol sym addr
.global \sym
.equ \sym, \addr
.endm

.macro .word32 x
    .word \x
.endm
