#include "n64-math.h"

#include "n64-stdbool.h"
#include "n64-fenv.h"
#include "n64-util.h"

#ifdef __NO_TRAPPING_MATH__
	#define _FTRAP( ex ) 
#else
	#define _FTRAP( ex ) n64_feraiseexcept( ex )
#endif

typedef struct {
	double invc;
	double logc;
} __attribute__((aligned(16))) __n64_log2_table_entry;

static const __n64_log2_table_entry s_logTable[16] = {
	{ 0x1.661ec79f8f3bep+0, -0x1.efec65b963019p-2 },
	{ 0x1.571ed4aaf883dp+0, -0x1.b0b6832d4fca4p-2 },
	{ 0x1.49539f0f010bp+0, -0x1.7418b0a1fb77bp-2 },
	{ 0x1.3c995b0b80385p+0, -0x1.39de91a6dcf7bp-2 },
	{ 0x1.30d190c8864a5p+0, -0x1.01d9bf3f2b631p-2 },
	{ 0x1.25e227b0b8eap+0, -0x1.97c1d1b3b7afp-3 },
	{ 0x1.1bb4a4a1a343fp+0, -0x1.2f9e393af3c9fp-3 },
	{ 0x1.12358f08ae5bap+0, -0x1.960cbbf788d5cp-4 },
	{ 0x1.0953f419900a7p+0, -0x1.a6f9db6475fcep-5 },
	{ 0x1p+0, 0x0p+0 },
	{ 0x1.e608cfd9a47acp-1, 0x1.338ca9f24f53dp-4 },
	{ 0x1.ca4b31f026aap-1, 0x1.476a9543891bap-3 },
	{ 0x1.b2036576afce6p-1, 0x1.e840b4ac4e4d2p-3 },
	{ 0x1.9c2d163a1aa2dp-1, 0x1.40645f0c6651cp-2 },
	{ 0x1.886e6037841edp-1, 0x1.88e9c2c1b9ff8p-2 },
	{ 0x1.767dcf5534862p-1, 0x1.ce0a44eb17bccp-2 }
};

static const unsigned long long s_expTable[32] = {
	0x3ff0000000000000ull, 0x3fefd9b0d3158574ull, 0x3fefb5586cf9890full, 0x3fef9301d0125b51ull,
	0x3fef72b83c7d517bull, 0x3fef54873168b9aaull, 0x3fef387a6e756238ull, 0x3fef1e9df51fdee1ull,
	0x3fef06fe0a31b715ull, 0x3feef1a7373aa9cbull, 0x3feedea64c123422ull, 0x3feece086061892dull,
	0x3feebfdad5362a27ull, 0x3feeb42b569d4f82ull, 0x3feeab07dd485429ull, 0x3feea47eb03a5585ull,
	0x3feea09e667f3bcdull, 0x3fee9f75e8ec5f74ull, 0x3feea11473eb0187ull, 0x3feea589994cce13ull,
	0x3feeace5422aa0dbull, 0x3feeb737b0cdc5e5ull, 0x3feec49182a3f090ull, 0x3feed503b23e255dull,
	0x3feee89f995ad3adull, 0x3feeff76f2fb5e47ull, 0x3fef199bdd85529cull, 0x3fef3720dcef9069ull,
	0x3fef5818dcfba487ull, 0x3fef7c97337b9b5full, 0x3fefa4afa2a490daull, 0x3fefd0765b6e4540ull,
};

static double n64_log2_impl( unsigned int fbits ) {
	if( fbits == 0x3f800000u ) { // 1.f -> +0
		return 0.0;
	} else if( fbits - 0x00800000u >= 0x7F000000u ) {
		if( !(fbits << 1) ) { // 0.f -> -inf
			_FTRAP( FE_DIVBYZERO );
			return -N64_HUGE_VAL;
		} else if( fbits == 0x7F800000u ) { // +inf -> +inf
			return N64_HUGE_VAL;
		} else if( (fbits << 1) == 0xFF000000u ) { // nan -> nan
			return n64_bit_cast_itof( fbits );
		} else if( fbits >> 31 ) { // negative -> nan
			_FTRAP( FE_INVALID );
			return NAN;
		} else {
			register const float norm = n64_bit_cast_ftoi( fbits ) * 0x1p23f;
			fbits = n64_bit_cast_itof( norm ) - (23 << 23);
		}
	}

	register unsigned int top = fbits - 0x3F330000u;
	register const unsigned int i = (top >> 19) & 0xF;
	top &= 0xFF800000u;
	register const double z = n64_bit_cast_itof( fbits - top );

	register const double r = z * s_logTable[i].invc - 1.0;
	register const double y0 = s_logTable[i].logc + (double)((int)top >> 23);
	register const double r2 = r * r;

	register const double y = 0.288457581109214 * r - 0.36092606229713164;
	register const double p = 0.480898481472577 * r - 0.7213474675006291;
	register double q = 1.4426950408774342 * r + y0;
	q += p * r2;
	return y * (r2 * r2) + q;
}

static float n64_exp2_impl( double e, unsigned int signBias ) {
	double kd = e + 0x1.8p+47;
	const long long ki = n64_bit_cast_dtol( kd );
	n64_memory_barrier();
	kd -= 0x1.8p+47;
	n64_memory_barrier();

	register const double r = e - kd;
	register const double s = n64_bit_cast_ltod( s_expTable[ki & 0x1Fu] + ((ki + (unsigned long long)signBias) << 47) );
	
	register const double y = (
		__builtin_assoc_barrier( 0.05550361559341535 * r + 0.2402284522445722 ) *
		__builtin_assoc_barrier( r * r )
	) + __builtin_assoc_barrier( 0.6931471806916203 * r + 1.0 );
	
	return (float)(y * s);
}

float n64_expf( float arg ) {
#if __FINITE_MATH_ONLY__ != 1
	register const unsigned int bits = n64_bit_cast_ftoi( arg );
	if( ((bits >> 23) & 0xFFu) == 0xFFu ) {
		return (bits == 0xFF800000u) ? 0.f : arg;
	} else if( arg > 88.72283f ) {
		_FTRAP( FE_OVERFLOW );
		return N64_INFINITY;
	}
#endif
	if( arg < -103.27892f ) {
		_FTRAP( FE_UNDERFLOW );
		return 0.f;
	}
	return n64_exp2_impl( (double)arg * 1.4426950408889634, 0u );
}

float n64_exp2f( float arg ) {
#if __FINITE_MATH_ONLY__ != 1
	register const unsigned int bits = n64_bit_cast_ftoi( arg );
	if( ((bits >> 23) & 0xFFu) == 0xFFu ) {
		return (bits == 0xFF800000u) ? 0.f : arg;
	} else if( arg >= 128.f ) {
		_FTRAP( FE_OVERFLOW );
		return N64_INFINITY;
	}
#endif
	if( arg < -149.f ) {
		_FTRAP( FE_UNDERFLOW );
		return 0.f;
	}
	return n64_exp2_impl( arg, 0u );
}

float n64_logf( float arg ) {
	if( arg == 2.71828183f ) return 1.f;
	return (float)(n64_log2_impl( n64_bit_cast_ftoi( arg ) ) * 0.69314718055994531);
}

float n64_log10f( float arg ) {
	return (float)(n64_log2_impl( n64_bit_cast_ftoi( arg ) ) * 0.3010299956639812);
}

float n64_log2f( float arg ) {
	// Use a slightly faster approximation of log2 since we need less precision

	unsigned int fbits = n64_bit_cast_ftoi( arg );
	if( fbits == 0x3f800000u ) { // 1.f -> +0
		return 0.f;
	} else if( fbits - 0x00800000u >= 0x7F000000u ) {
		if( !(fbits << 1) ) { // 0.f -> -inf
			return -N64_INFINITY;
		} else if( fbits == 0x7F800000u ) { // +inf -> +inf
			return N64_INFINITY;
		} else if( (fbits << 1) == 0xFF000000u ) { // nan -> nan
			return arg;
		} else if( fbits >> 31 ) { // negative -> nan
			return NAN;
		} else {
			fbits = n64_bit_cast_itof( arg * 0x1p23f ) - (23 << 23);
		}
	}

	register unsigned int top = fbits - 0x3F330000u;
	register const unsigned int i = (top >> 19) & 0xF;
	top &= 0xFF800000u;
	register const double z = n64_bit_cast_itof( fbits - top );

	register const double r = z * s_logTable[i].invc - 1.0;
	register const double y0 = s_logTable[i].logc + (double)((int)top >> 23);
	register const double r2 = r * r;

	register const double y = 0.4811247078767291 * r - 0.7213476299867769;
	register const double p = 1.4426950186867042 * r + y0;
	return (float)((-0.36051725506874704 * r2 + y) * r2 + p);
}

float n64_powf( float base, float exponent ) {
	// an exponent of 0 or a base of 1 always results in 1, even if the other argument is NaN
	if( exponent == 0.f || exponent == -0.f || base == 1.f ) {
		return 1.f;
	}

	unsigned int ib = n64_bit_cast_ftoi( base );
	unsigned int ie = n64_bit_cast_ftoi( exponent );

#if __FINITE_MATH_ONLY__ != 1
	// if the base is -1 and the exponent is plus or minus infinity, return 1
	if( exponent == -1.f && !(ie << 9) && (((ie >> 23) & 0xFFu) == 0xFFu) ) {
		return 1.f;
	}
#endif

	n64_bool expIsInteger = true;
	n64_bool expIsOdd = false;
	if( !ib || (ib >> 31) ) { // only care about these values when the base is non-positive
		register const unsigned int iee = (ie >> 23) & 0xFFu;
		if( iee == 0xFFu || iee < 127u ) {
			expIsInteger = false;
		} else if( iee > 150u ) {
		} else if( ie & ((1u << (150u - iee)) - 1u) ) {
			expIsInteger = false;
		} else if( ie & (1 << (150u - iee)) ) {
			expIsOdd = true;
		}
	}

	if( !(ib << 1) ) {
		// base is +0 or -0
#if __FINITE_MATH_ONLY__ != 1
		if( ((ie >> 23) & 0xFFu) == 0xFFu ) {
			if( ie << 9 ) return base; // 0^NaN -> NaN
			if( ie >> 31 ) return N64_INFINITY; // 0^-inf -> +inf
			return 0.f; // 0^+inf -> +0
		}
#endif

		if( ie >> 31 ) {
			_FTRAP( FE_DIVBYZERO );
			if( expIsInteger && expIsOdd && (ib >> 31) ) {
				// -0 to the power of a negative odd integer -> -inf
				return -N64_INFINITY;
			}

			// 0 to the power of a finite even integer or a finite non-integer -> +inf
			return N64_INFINITY;
		} else {
			// -0 to the power of a positive odd integer is -0 (0 to any other finite power is +0)
			return (expIsInteger && expIsOdd) ? base : 0.f;
		}
	}

#if __FINITE_MATH_ONLY__ != 1
	if( ((ib >> 23) & 0xFFu) == 0xFFu ) {
		if( ib << 9 ) return base; // any base other than 1 to the power of NaN is NaN
		if( ib >> 31 ) {
			if( exponent > 0.f ) {
				// -inf to the power of a positive odd integer is -inf
				if( expIsInteger && expIsOdd ) {
					return -N64_INFINITY;
				}

				// -inf to any other positive power is +inf
				return N64_INFINITY;
			} else {
				// -inf to the power of a negative odd integer is -0
				if( expIsInteger && expIsOdd ) {
					return -0.f;
				}
				
				// -inf to any other negative power is +0
				return 0.f;
			}
		}

		// +inf to the power of any positive exponent is +inf
		// +inf to the power of any negative exponent is +0
		return (exponent > 0.f) ? N64_INFINITY : 0.f;
	}
#endif

	unsigned int signBias = 0u;
	if( ib >> 31 ) {
		// negative base
		if( !expIsInteger ) {
			// a negative finite number to the power of any non-integer finite number is NaN
			_FTRAP( FE_INVALID );
			return NAN;
		}

		if( expIsOdd ) signBias = 0x10000u;
		ib &= 0x7FFFFFFFu;
	} else if( exponent == -1.f ) {
		return 1.f / base;
	}

	if( ib < 0x00800000u ) {
		// subnormal base
		ib = n64_bit_cast_ftoi( base * 0x1p23f );
		ib &= 0x7FFFFFFFu;
		ib -= (23 << 23);
	}

	const double elogb = (double)exponent * n64_log2_impl( ib );
#if __FINITE_MATH_ONLY__ != 1
	if( (float)elogb >= 128.f ) {
		_FTRAP( FE_OVERFLOW );
		return signBias ? -N64_INFINITY : N64_INFINITY;
	}
#endif
	if( (float)elogb < -149.f ) {
		_FTRAP( FE_UNDERFLOW );
		return signBias ? -0.f : 0.f;
	}

	return n64_exp2_impl( elogb, signBias );
}

// x >= y >= 0
static double hypot_impl( double x, double y ) {
	double e1, e2;
	const double h = n64_sqrt( x * x + y * y );
	if( h <= 2.0 * y ) {
		register const double dy = h - y;
		e1 = x * (2.0 * dy - x);
		e2 = (dy - 2.0 * (x - y)) * dy;
	} else {
		register const double dx = h - x;
		e1 = 2.0 * dx  * (x - 2.0 * y);
		e2 = (4.0 * dx - y) * y + dx * dx;
	}

	return h - __builtin_assoc_barrier((e1 + e2) / (2.0 * h));
}

float n64_hypotf( float x, float y ) {
#if __FINITE_MATH_ONLY__ != 1
	register const unsigned int xi = n64_bit_cast_ftoi( x );
	register const unsigned int yi = n64_bit_cast_ftoi( y );
	if( (xi & 0x7F800000u) || (yi & 0x7F800000u) ) {
		register const unsigned int ynan = yi << 9;
		if(
			(!(xi << 9) && (!ynan || !(yi >> 31))) ||
			(!ynan && !(xi >> 31))
		) {
			return N64_INFINITY;
		}

		return x + y;
	}
#endif

	return (float)n64_sqrt( (double)x * (double)x + (double)y * (double)y );
}

typedef union {
	double f;
	struct {
		unsigned int hi;
		unsigned int lo;
	};
} __dbits_t;

double n64_hypot( double x, double y ) {
#if __FINITE_MATH_ONLY__ != 1
	const __dbits_t xi = { x };
	const __dbits_t yi = { y };
	if( (xi.hi & 0x7FF00000u) || (yi.hi & 0x7FF00000u) ) {
		register const unsigned int xnan = xi.lo || (xi.hi << 12);
		register const unsigned int ynan = yi.lo || (yi.hi << 12);
		if(
			(!xnan && (!ynan || !(yi.hi >> 31))) ||
			(!ynan && !(xi.hi >> 31))
		) {
			return N64_HUGE_VAL;
		}

		return x + y;
	}
#endif

	x = n64_fabs( x );
	y = n64_fabs( y );

	if( x < y ) {
		register const double temp = x;
		x = y;
		y = temp;
	}

	if( x > 0x1p+511 ) {
		if( y <= x * 0x1p-54 ) {
			return x + y;
		}

		return hypot_impl( x * 0x1p-600, y * 0x1p-600 ) * 0x1p+600;
	}

	if( y < 0x1p-459 ) {
		if( x >= y * 0x1p+54 ) {
			return x + y;
		}

		return hypot_impl( x * 0x1p+600, y * 0x1p+600 ) * 0x1p-600;
	}

	if( y <= x * 0x1p-54 ) {
		return x + y;
	}

	return hypot_impl( x, y );
}

float n64_cbrtf( float arg ) {
	register unsigned int bits = n64_bit_cast_ftoi( arg );
	if( ((bits >> 23) & 0xFFu) == 0xFFu || !(bits << 1) ) {
		return arg;
	}

	n64_bool negative = false;
	if( bits >> 31 ) {
		negative = true;
		bits &= 0x7FFFFFFFu;
	}

	if( bits < 0x00800000u ) {
		// subnormal base
		bits = n64_bit_cast_ftoi( arg * 0x1p23f );
		bits &= 0x7FFFFFFFu;
		bits -= (23 << 23);
	}

	register const float result = n64_exp2_impl( n64_log2_impl( bits ) / 3.0, 0u );
	return negative ? -result : result;
}

static const unsigned int s_4divPi[24] = {
	0x000000a2u, 0x0000a2f9u, 0x00a2f983u, 0xa2f9836eu,
	0xf9836e4eu, 0x836e4e44u, 0x6e4e4415u, 0x4e441529u,
	0x441529fcu, 0x1529fc27u, 0x29fc2757u, 0xfc2757d1u,
	0x2757d1f5u, 0x57d1f534u, 0xd1f534ddu, 0xf534ddc0u,
	0x34ddc0dbu, 0xddc0db62u, 0xc0db6295u, 0xdb629599u,
	0x6295993cu, 0x95993c43u, 0x993c4390u, 0x3c439041u
};

__attribute__((always_inline))
static inline unsigned int absTop12( float x ) {
	return (n64_bit_cast_ftoi( x ) >> 20) & 0x7FFu;
}

__attribute__((always_inline))
static inline double fastReduce( double x, unsigned int *quadrant ) {
	register const double r = x * 0.6366197723675814;
	*quadrant = n64_llround( r );
	return x - n64_round( r ) * 1.5707963267948966;
}

static inline double largeReduce( unsigned int xi, unsigned int *quadrant ) {
	const unsigned int *lut = &s_4divPi[(xi >> 26) & 0xFu];
	const unsigned int shift = (xi >> 23) & 0x7u;

	xi &= 0x00FFFFFFu;
	xi |= 0x00800000u;
	xi <<= shift;

	const unsigned long long hi = (unsigned long long)(xi * lut[0]) << 32;
	const unsigned long long lo = ((unsigned long long)xi * lut[8]) >> 32;
	unsigned long long result = (unsigned long long)xi * lut[4];
	result += hi | lo;

	*quadrant = ((unsigned int)(result >> 61) + 1u) >> 1;
	result -= (unsigned long long)*quadrant << 62;
  
	return (double)(long long)result * 3.4061215800865545e-19;
}

static const double s_cosPolyP[5] = {
	 1.0, -0.49999999725108224,  0.041666623324344516, -0.001388676379437604,  2.4390450703564542e-5
};

static const double s_cosPolyN[5] = {
	-1.0,  0.49999999725108224, -0.041666623324344516,  0.001388676379437604, -2.4390450703564542e-5
};

static float sinPoly( double x ) {
	register const double x2 = x * x;
	register const double x3 = x * x2;

	register const double s1 = 0.008332178146138854 + x2 * -1.9517298981385725e-4;
	register const double s0 = x + x3 * -0.16666654943701084;

	return (float)(s0 + (x3 * x2) * s1);
}

static float cosPoly( double x, unsigned int negatePoly ) {
	const double *p = negatePoly ? s_cosPolyN : s_cosPolyP;
	register const double x2 = x * x;

	register const double c2 = p[3] + x2 * p[4];
	register const double c1 = p[0] + x2 * p[1];

	register const double x4 = x2 * x2;
	register const double c0 = c1 + x4 * p[2];

	return (float)(c0 + __builtin_assoc_barrier( x4 * x2 ) * c2);
}

float n64_sinf( float arg ) {
	if( arg == 0.f ) return arg;

	const unsigned int top = absTop12( arg );
	if( top < 1012u ) {
		if( top < 920u ) {
			if( top < 8u ) _FTRAP( FE_UNDERFLOW );
			return arg;
		}

		return sinPoly( arg );
	} else if( top < 1071u ) {
		unsigned int q;
		double x = fastReduce( arg, &q );

		if( ((q & 3u) + 1u) & 2u ) {
			x = -x;
		}

		return (q & 1u) ? cosPoly( x, q & 2u ) : sinPoly( x );
	} else if( top < 2040u ) {
		unsigned int q;
		const unsigned int xi = n64_bit_cast_ftoi( arg );
		double x = largeReduce( xi, &q );

		const unsigned int s = q + (xi >> 31);
		if( ((s & 3u) + 1u) & 2u ) {
			x = -x;
		}

		return (q & 1u) ? cosPoly( x, s & 2u ) : sinPoly( x );
	} else {
		_FTRAP( FE_INVALID );
		return NAN;
	}
}

float n64_cosf( float arg ) {
	const unsigned int top = absTop12( arg );
	if( top < 1012u ) {
		if( top < 920u ) {
			return 1.f;
		}

		return cosPoly( arg, 0u );
	} else if( top < 1071u ) {
		unsigned int q;
		double x = fastReduce( arg, &q );

		if( ((q & 3u) + 1u) & 2u ) {
			x = -x;
		}

		return (q & 1u) ? sinPoly( x ) : cosPoly( x, q & 2u );
	} else if( top < 2040u ) {
		unsigned int q;
		const unsigned int xi = n64_bit_cast_ftoi( arg );
		double x = largeReduce( xi, &q );

		const unsigned int s = q + (xi >> 31);
		if( ((s & 3u) + 1u) & 2u ) {
			x = -x;
		}

		return (q & 1u) ? sinPoly( x ) : cosPoly( x, s & 2u );
	} else {
		_FTRAP( FE_INVALID );
		return NAN;
	}
}
