#ifndef n64_alloca
#define n64_alloca( size ) __builtin_alloca_with_align( size, 64 )
#endif
