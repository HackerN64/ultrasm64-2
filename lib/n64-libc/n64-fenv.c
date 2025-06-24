#include "n64-fenv.h"

int n64_feclearexcept( int excepts ) {
	if( excepts & ~FE_ALL_EXCEPT ) return excepts;
	register unsigned int fcr31 = __builtin_mips_get_fcsr();
	fcr31 &= ~((unsigned int)excepts << 2);
	__builtin_mips_set_fcsr( fcr31 );
	return ((int)__builtin_mips_get_fcsr() >> 2) & excepts;
}

int n64_fetestexcept( int excepts ) {
	return (int)(__builtin_mips_get_fcsr() >> 2) & FE_ALL_EXCEPT & excepts;
}

int n64_feraiseexcept( int excepts ) {
	if( excepts & ~FE_ALL_EXCEPT ) return excepts;
	register unsigned int fcr31 = __builtin_mips_get_fcsr();
	fcr31 |= (unsigned int)excepts << 12;
	__builtin_mips_set_fcsr( fcr31 );
	return (((int)__builtin_mips_get_fcsr() >> 2) & excepts) ^ excepts;
}

int n64_fegetexceptflag( n64_fexcept_t *flagp, int excepts ) {
	if( excepts & ~FE_ALL_EXCEPT ) return excepts;
	*flagp = (n64_fexcept_t)((__builtin_mips_get_fcsr() >> 2) & (unsigned int)excepts);
	return 0;
}

int n64_fesetexceptflag( const n64_fexcept_t *flagp, int excepts ) {
	if( excepts & ~FE_ALL_EXCEPT ) return excepts;
	register unsigned int fcr31 = __builtin_mips_get_fcsr();
	fcr31 &= ~((unsigned int)excepts << 2);
	fcr31 |= ((unsigned int)*flagp & (unsigned int)excepts) << 2;
	__builtin_mips_set_fcsr( fcr31 );
	return (((int)__builtin_mips_get_fcsr() >> 2) & excepts) ^ ((int)*flagp & excepts);
}

int n64_fesetround( int round ) {
	if( round < 0 || round > 3 ) return round;
	register unsigned int fcr31 = __builtin_mips_get_fcsr() & ~3u;
	__builtin_mips_set_fcsr( fcr31 | (unsigned int)round );
	return (int)(__builtin_mips_get_fcsr() & 3u) ^ round;
}

int n64_fegetround() {
	return (unsigned int)__builtin_mips_get_fcsr() & 3u;
}

int n64_fegetenv( n64_fenv_t* envp ) {
	*envp = __builtin_mips_get_fcsr();
	return 0;
}

int n64_fesetenv( const n64_fenv_t* envp ) {
	register const unsigned int c = __builtin_mips_get_fcsr() & 0x00800000u;
	__builtin_mips_set_fcsr( (*envp & 0x01000FFFu) | c );
	return (int)((*envp ^ __builtin_mips_get_fcsr()) & 0x01000FFFu);
}

int n64_feholdexcept( n64_fenv_t* envp ) {
	*envp = (n64_fenv_t)__builtin_mips_get_fcsr() & 0x0103FFFFu;
	__builtin_mips_set_fcsr( *envp & 0x01800003u );
	return (int)(__builtin_mips_get_fcsr() & 0x0003FFFCu);
}

int n64_feupdateenv( const n64_fenv_t* envp ) {
	register const unsigned int e = __builtin_mips_get_fcsr() & 0x0083F000u;
	register const unsigned int c = e & 0x00800000u;
	__builtin_mips_set_fcsr( (*envp & 0x01000FFFu) | c );
	__builtin_mips_set_fcsr( (*envp & 0x01000FFFu) | e );
	return (int)((*envp ^ __builtin_mips_get_fcsr()) & 0x0103FFFFu);
}
