#ifndef N64_STDLIB_N64_FENV_H_
#define N64_STDLIB_N64_FENV_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FE_DIVBYZERO 0x08
#define FE_INEXACT 0x01
#define FE_INVALID 0x10
#define FE_OVERFLOW 0x04
#define FE_UNDERFLOW 0x02
#define FE_ALL_EXCEPT 0x1F

#define FE_DOWNWARD 0x3
#define FE_TONEAREST 0x0
#define FE_TOWARDZERO 0x1
#define FE_UPWARD 0x2

typedef unsigned int n64_fenv_t;
typedef unsigned char n64_fexcept_t;

int n64_feclearexcept( int excepts );
int n64_fetestexcept( int excepts ) __attribute__((warn_unused_result));
int n64_feraiseexcept( int excepts );
int n64_fegetexceptflag( n64_fexcept_t *flagp, int excepts ) __attribute__((nonnull(1), access(write_only, 1)));
int n64_fesetexceptflag( const n64_fexcept_t *flagp, int excepts ) __attribute__((nonnull(1)));
int n64_fesetround( int round );
int n64_fegetround() __attribute__((warn_unused_result));
int n64_fegetenv( n64_fenv_t* envp ) __attribute__((nonnull(1), access(write_only, 1)));
int n64_fesetenv( const n64_fenv_t* envp ) __attribute__((nonnull(1)));
int n64_feholdexcept( n64_fenv_t* envp ) __attribute__((nonnull(1), access(write_only, 1)));
int n64_feupdateenv( const n64_fenv_t* envp ) __attribute__((nonnull(1)));

#ifdef __cplusplus
}
#endif

#endif
