#include "n64-assert.h"
#include "n64-stddef.h"

#include "n64-stdio-ryu.c.inc"

typedef unsigned char n64_smallbool;

n64_static_assert( sizeof( long long ) == 8 );
n64_static_assert( sizeof( double ) == 8 );
n64_static_assert( sizeof( float ) == 4 );
n64_static_assert( sizeof( int ) == 4 );

typedef struct {
	char *const start;
	char *head;
	char *end;
	n64_bool(*callback)(void*, unsigned int);
	void *const userdata;
	unsigned int total;
} n64_bprint_buffer;

static void n64_putc( n64_bprint_buffer *buffer, char c ) {
	buffer->total++;

	if( buffer->start == buffer->end ) return;
	if( buffer->head != buffer->end ) {
		*(buffer->head++) = c;
	} else if( buffer->callback( buffer->userdata, (unsigned int)(buffer->head - buffer->start) ) ) {
		buffer->head = buffer->start;
	} else {
		buffer->end = buffer->start;
	}
}

typedef struct {
	unsigned int width;
	unsigned int precision;
	unsigned int defaultPrecision;
	char type;
	unsigned char dataSize;
	n64_smallbool leftAligned;
	n64_smallbool alternate;
	n64_smallbool capitalize;
	char padchar;
	char poschar;
} n64_format_args;

static void n64_format_nan_or_inf(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	n64_bool isNegative,
	n64_bool isNaN
) {
	const unsigned int len = (isNegative || format->poschar != '\0') ? 4u : 3u;
	if( !format->leftAligned ) {
		for( unsigned int i = len; i < format->width; i++ ) {
			n64_putc( buffer, ' ' );
		}
	}

	if( isNegative ) {
		n64_putc( buffer, '-' );
	} else if( format->poschar != '\0' ) {
		n64_putc( buffer, format->poschar );
	}

	if( format->capitalize ) {
		if( isNaN ) {
			n64_putc( buffer, 'N' );
			n64_putc( buffer, 'A' );
			n64_putc( buffer, 'N' );
		} else {
			n64_putc( buffer, 'I' );
			n64_putc( buffer, 'N' );
			n64_putc( buffer, 'F' );
		}
	} else {
		if( isNaN ) {
			n64_putc( buffer, 'n' );
			n64_putc( buffer, 'a' );
			n64_putc( buffer, 'n' );
		} else {
			n64_putc( buffer, 'i' );
			n64_putc( buffer, 'n' );
			n64_putc( buffer, 'f' );
		}
	}

	if( format->leftAligned ) {
		for( unsigned int i = len; i < format->width; i++ ) {
			n64_putc( buffer, ' ' );
		}
	}
}

static const unsigned int s_u32_pow10[10] = {
	1u,
	10u,
	100u,
	1000u,
	10000u,
	100000u,
	1000000u,
	10000000u,
	100000000u,
	1000000000u
};

static const unsigned long long s_u64_pow10[18] = {
	1ull,
	10ull,
	100ull,
	1000ull,
	10000ull,
	100000ull,
	1000000ull,
	10000000ull,
	100000000ull,
	1000000000ull,
	10000000000ull,
	100000000000ull,
	1000000000000ull,
	10000000000000ull,
	100000000000000ull,
	1000000000000000ull,
	10000000000000000ull,
	100000000000000000ull,
};

static void n64_float_exp_helper(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	unsigned int precision,
	char *digitBuffer,
	int originalExponent,
	unsigned int digits,
	n64_bool negative,
	n64_bool trimTrailingZeros
) {
	if( trimTrailingZeros ) {
		if( digits > precision + 1u ) {
			digits = precision + 1u;
		}

		while( digits > 1u ) {
			if( digitBuffer[digits - 1u] != '0' ) break;
			digits--;
		}

		if( digits <= precision ) {
			precision = digits ? (digits - 1u) : 0u;
		}
	}

	if( !digits ) {
		digitBuffer[0] = '0';
		digits = 1u;
	}

	int e = originalExponent + (int)digits - 1;
	unsigned int len = precision ? (precision + 6u) : (format->alternate ? 6u : 5u);
	if( negative || format->poschar != '\0' ) len++;
	if( e >= 100 || e <= -100 ) len++;

	if( !format->leftAligned && format->padchar == ' ' ) {
		for( unsigned int i = len; i < format->width; i++ ) {
			n64_putc( buffer, ' ' );
		}
	}

	if( negative ) {
		n64_putc( buffer, '-' );
	} else if( format->poschar != '\0' ) {
		n64_putc( buffer, format->poschar );
	}

	if( !format->leftAligned && format->padchar == '0' ) {
		for( unsigned int i = len; i < format->width; i++ ) {
			n64_putc( buffer, '0' );
		}
	}

	n64_putc( buffer, digitBuffer[digits - 1] );
	if( precision || format->alternate ) n64_putc( buffer, '.' );
	for( int i = (int)digits - 2; i >= 0; i-- ) {
		n64_putc( buffer, digitBuffer[i] );
	}

	if( !trimTrailingZeros ) {
		for( unsigned int i = digits; i <= precision; i++ ) {
			n64_putc( buffer, '0' );
		}
	}

	n64_putc( buffer, format->capitalize ? 'E' : 'e' );
	if( e < 0 ) {
		n64_putc( buffer, '-' );
		e = -e;
	} else {
		n64_putc( buffer, '+' );
	}

	if( e >= 100 ) n64_putc( buffer, '0' + (char)(e / 100) );
	n64_putc( buffer, '0' + (char)((e / 10) % 10) );
	n64_putc( buffer, '0' + (char)(e % 10) );

	if( format->leftAligned ) {
		for( unsigned int i = len; i < format->width; i++ ) {
			n64_putc( buffer, ' ' );
		}
	}
}

static void n64_format_float_exp(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	ryu_floating_decimal_32 decimal,
	unsigned int precision,
	n64_bool negative,
	n64_bool trimTrailingZeros
) {
	char digitBuffer[9];
	unsigned int digits = decimalLength9( decimal.mantissa ) - 1u;
	if( digits > precision ) {
		decimal.mantissa /= s_u32_pow10[digits - precision - 1u];
		if( decimal.mantissa % 10u >= 5u ) {
			decimal.mantissa += 10u;
			if( decimal.mantissa >= s_u32_pow10[precision + 2u] ) {
				decimal.mantissa /= 10u;
				decimal.exponent++;
			}
		}
		decimal.mantissa /= 10u;
		decimal.exponent += (int)(digits - precision);
	}

	if( trimTrailingZeros ) {
		while( decimal.mantissa && !(decimal.mantissa % 10u) ) {
			decimal.mantissa /= 10u;
			decimal.exponent++;
		}
		precision = decimalLength9( decimal.mantissa ) - 1u;
	}

	digits = 0u;
	while( decimal.mantissa ) {
		digitBuffer[digits++] = '0' + (char)(decimal.mantissa % 10u);
		decimal.mantissa /= 10u;
	}

	n64_float_exp_helper( buffer, format, precision, digitBuffer, decimal.exponent, digits, negative, trimTrailingZeros );
}

static void n64_format_double_exp(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	ryu_floating_decimal_64 decimal,
	unsigned int precision,
	n64_bool negative,
	n64_bool trimTrailingZeros
) {
	char digitBuffer[17];
	unsigned int digits = decimalLength17( decimal.mantissa ) - 1u;
	if( digits > precision ) {
		decimal.mantissa /= s_u64_pow10[digits - precision - 1u];
		if( decimal.mantissa % 10ull >= 5ull ) {
			decimal.mantissa += 10ull;
			if( decimal.mantissa >= s_u64_pow10[precision + 2u] ) {
				decimal.mantissa /= 10ull;
				decimal.exponent++;
			}
		}
		decimal.mantissa /= 10ull;
		decimal.exponent += (int)(digits - precision);
	}

	if( trimTrailingZeros ) {
		while( decimal.mantissa && !(decimal.mantissa % 10ull) ) {
			decimal.mantissa /= 10ull;
			decimal.exponent++;
		}
		precision = decimalLength17( decimal.mantissa ) - 1u;
	}

	digits = 0u;
	while( decimal.mantissa ) {
		digitBuffer[digits++] = '0' + (char)(decimal.mantissa % 10u);
		decimal.mantissa /= 10u;
	}

	n64_float_exp_helper( buffer, format, precision, digitBuffer, decimal.exponent, digits, negative, trimTrailingZeros );
}

#define n64_format_fixed_generic( FTYPE, FBITS, FDECLEN, ITYPE, ISUFFIX ) \
static void n64_format_##FTYPE##_fixed(\
	n64_bprint_buffer *buffer,\
	const n64_format_args *format,\
	ryu_floating_decimal_##FBITS decimal,\
	unsigned int precision,\
	n64_bool negative,\
	n64_bool trimTrailingZeros\
) {\
	const unsigned int digits = decimalLength##FDECLEN( decimal.mantissa );\
	const int e = decimal.exponent + (int)digits - 1;\
\
	unsigned int integerDigits;\
	unsigned int decimalDigits;\
	unsigned int decimalLeadingZeros;\
	ITYPE integerPart;\
	ITYPE decimalPart;\
\
	if( e < 0 ) {\
		integerDigits = 1u;\
		decimalDigits = digits;\
		decimalLeadingZeros = (unsigned int)(-e) - 1u;\
		integerPart = 0##ISUFFIX;\
		decimalPart = decimal.mantissa;\
	} else if( decimal.exponent >= 0 ) {\
		integerDigits = digits;\
		decimalDigits = 0u;\
		decimalLeadingZeros = 0u;\
		integerPart = decimal.mantissa;\
		decimalPart = 0##ISUFFIX;\
	} else {\
		integerDigits = (unsigned int)e + 1u;\
		decimalDigits = digits - integerDigits;\
		decimalLeadingZeros = 0u;\
		integerPart = decimal.mantissa / s_u##FBITS##_pow10[decimalDigits];\
		decimalPart = decimal.mantissa % s_u##FBITS##_pow10[decimalDigits];\
	}\
\
	if( decimalLeadingZeros >= precision ) {\
		decimalLeadingZeros = precision;\
		decimalDigits = 0u;\
		decimalPart = 0##ISUFFIX;\
	} else if( decimalDigits + decimalLeadingZeros > precision ) {\
		const unsigned int newDigits = precision - decimalLeadingZeros;\
		decimalPart /= s_u##FBITS##_pow10[decimalDigits - newDigits];\
		decimalDigits = newDigits;\
	}\
\
	if( trimTrailingZeros ) {\
		while( decimalPart && !(decimalPart % 10##ISUFFIX) ) {\
			decimalPart /= 10##ISUFFIX;\
		}\
		decimalDigits = decimalPart ? decimalLength##FDECLEN( decimalPart ) : 0u;\
		precision = decimalDigits + decimalLeadingZeros;\
	}\
\
	unsigned int len = integerDigits + precision;\
	if( decimal.exponent > 0 ) len += (unsigned int)decimal.exponent;\
	if( negative || format->poschar != '\0' ) len++;\
	if( precision || format->alternate ) len++;\
\
	if( !format->leftAligned && format->padchar == ' ' ) {\
		for( unsigned int i = len; i < format->width; i++ ) {\
			n64_putc( buffer, ' ' );\
		}\
	}\
\
	if( negative ) {\
		n64_putc( buffer, '-' );\
	} else if( format->poschar != '\0' ) {\
		n64_putc( buffer, format->poschar );\
	}\
\
	if( !format->leftAligned && format->padchar == '0' ) {\
		for( unsigned int i = len; i < format->width; i++ ) {\
			n64_putc( buffer, '0' );\
		}\
	}\
\
	char tempBuffer[FDECLEN];\
	for( unsigned int i = 0u; i < integerDigits; i++ ) {\
		tempBuffer[i] = '0' + (char)(integerPart % 10##ISUFFIX);\
		integerPart /= 10##ISUFFIX;\
	}\
\
	for( int i = (int)integerDigits - 1; i >= 0; i-- ) {\
		n64_putc( buffer, tempBuffer[i] );\
	}\
\
	for( int i = 0; i < decimal.exponent; i++ ) {\
		n64_putc( buffer, '0' );\
	}\
\
	if( precision || format->alternate ) {\
		n64_putc( buffer, '.' );\
	}\
\
	for( unsigned int i = 0u; i < decimalLeadingZeros; i++ ) {\
		n64_putc( buffer, '0' );\
	}\
\
	for( unsigned int i = 0u; i < decimalDigits; i++ ) {\
		tempBuffer[i] = '0' + (char)(decimalPart % 10##ISUFFIX);\
		decimalPart /= 10##ISUFFIX;\
	}\
\
	for( int i = (int)decimalDigits - 1; i >= 0; i-- ) {\
		n64_putc( buffer, tempBuffer[i] );\
	}\
\
	if( !trimTrailingZeros ) {\
		for( unsigned int i = decimalDigits + decimalLeadingZeros; i < precision; i++ ) {\
			n64_putc( buffer, '0' );\
		}\
	}\
\
	if( format->leftAligned ) {\
		for( unsigned int i = len; i < format->width; i++ ) {\
			n64_putc( buffer, ' ' );\
		}\
	}\
}

n64_format_fixed_generic( float, 32, 9, unsigned int, u )
n64_format_fixed_generic( double, 64, 17, unsigned long long, ull )

static inline void n64_format_float(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	float value
) {
	unsigned int bits = n64_bit_cast_ftoi( value );
	const n64_bool sign = (n64_bool)(bits >> 31);
	const unsigned int e = (bits >> 23) & 0xffu;
	const unsigned int m = bits & 0x7fffffu;

	if( e == 0xffu ) {
		n64_format_nan_or_inf( buffer, format, sign, m != 0u );
		return;
	}

	const ryu_floating_decimal_32 decimal = ryu_float_to_decimal( m, e );
	switch( format->type ) {
		case 'e':
			n64_format_float_exp( buffer, format, decimal, format->precision, sign, false );
			break;
		case 'f':
			n64_format_float_fixed( buffer, format, decimal, format->precision, sign, false );
			break;
		default: {
			const int P = format->precision ? (int)format->precision : 1;
			const int exp = decimal.exponent + (int)decimalLength9( decimal.mantissa ) - 1;
			if( P > exp && exp >= -4 ) {
				n64_format_float_fixed( buffer, format, decimal, (unsigned int)(P - 1 - exp), sign, !format->alternate );
			} else {
				n64_format_float_exp( buffer, format, decimal, (unsigned int)(P - 1), sign, !format->alternate );
			}
			break;
		}
	}
}

static inline void n64_format_double(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	double value
) {
	unsigned long long bits = n64_bit_cast_dtol( value );
	const n64_bool sign = (n64_bool)(bits >> 63);
	const unsigned int e = (unsigned int)(bits >> 52) & 0x7ffu;
	const unsigned long long m = bits & 0x000fffffffffffffull;

	if( e == 0x7ffu ) {
		n64_format_nan_or_inf( buffer, format, sign, m != 0ull );
		return;
	}

	const ryu_floating_decimal_64 decimal = ryu_double_to_decimal( m, e );
	switch( format->type ) {
		case 'e':
			n64_format_double_exp( buffer, format, decimal, format->precision, sign, false );
			break;
		case 'f':
			n64_format_double_fixed( buffer, format, decimal, format->precision, sign, false );
			break;
		default: {
			const int P = format->precision ? (int)format->precision : 1;
			const int exp = decimal.exponent + (int)decimalLength17( decimal.mantissa ) - 1;
			if( P > exp && exp >= -4 ) {
				n64_format_double_fixed( buffer, format, decimal, (unsigned int)(P - 1 - exp), sign, !format->alternate );
			} else {
				n64_format_double_exp( buffer, format, decimal, (unsigned int)(P - 1), sign, !format->alternate );
			}
			break;
		}

	}
}

static inline void align_number(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	const char *prefix,
	unsigned int prefixLen,
	unsigned int minZeros,
	const char *reverseDigits,
	unsigned int numDigits
) {
	if( !format->leftAligned ) {
		if( format->padchar == '0' && prefixLen + numDigits < format->width ) {
			register const unsigned int Z = format->width - prefixLen - numDigits;
			if( minZeros < Z ) minZeros = Z;
		}

		for( unsigned int i = prefixLen + minZeros + numDigits; i < format->width; i++ ) {
			n64_putc( buffer, ' ' );
		}
	}

	for( unsigned int i = 0; i < prefixLen; i++ ) {
		n64_putc( buffer, prefix[i] );
	}

	for( unsigned int i = 0; i < minZeros; i++ ) {
		n64_putc( buffer, '0' );
	}

	for( int i = (int)numDigits - 1; i >= 0; i-- ) {
		n64_putc( buffer, reverseDigits[i] );
	}

	if( format->leftAligned ) {
		for( unsigned int i = prefixLen + minZeros + numDigits; i < format->width; i++ ) {
			n64_putc( buffer, ' ' );
		}
	}
}

static inline void n64_format_char(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	char value
) {
	if( !format->leftAligned ) {
		for( unsigned int i = 1; i < format->width; i++ ) n64_putc( buffer, ' ' );
	}

	n64_putc( buffer, value );

	if( format->leftAligned ) {
		for( unsigned int i = 1; i < format->width; i++ ) n64_putc( buffer, ' ' );
	}
}

static inline void n64_format_string(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	const char *value
) {
	if( !format->precision ) {
		for( unsigned int i = 0; i < format->width; i++ ) n64_putc( buffer, ' ' );
		return;
	}

	unsigned int i = 0;
	if( format->leftAligned || !format->width ) {
		while( value[i] != '\0' && i < format->precision ) n64_putc( buffer, value[i++] );
		while( i++ < format->width ) n64_putc( buffer, ' ' );
	} else {
		register const unsigned int maxpad = (format->precision > format->width) ? format->precision : format->width;
		while( value[i] != '\0' && i < maxpad ) i++;
		while( i++ < format->width ) n64_putc( buffer, ' ' );
		while( *value != '\0' ) n64_putc( buffer, *(value++) );
	}
}

static void n64_format_udec32(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	unsigned int value
) {
	char digitBuffer[10];
	unsigned int numDigits = 0u;
	while( value ) {
		digitBuffer[numDigits++] = '0' + (char)(value % 10u);
		value /= 10u;
	}

	unsigned int minZeros = 0u;
	if( format->precision > numDigits ) {
		minZeros = format->precision - numDigits;
	} else if( !numDigits && format->precision ) {
		minZeros = 1u;
	}

	align_number( buffer, format, &format->poschar, format->poschar ? 1u : 0u, minZeros, digitBuffer, numDigits );
}

static void n64_format_udec64(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	unsigned long long value
) {
	char digitBuffer[20];
	unsigned int numDigits = 0u;
	while( value ) {
		digitBuffer[numDigits++] = '0' + (char)(value % 10ull);
		value /= 10ull;
	}

	unsigned int minZeros = 0u;
	if( format->precision > numDigits ) {
		minZeros = format->precision - numDigits;
	} else if( !numDigits && format->precision ) {
		minZeros = 1u;
	}

	align_number( buffer, format, &format->poschar, format->poschar ? 1u : 0u, minZeros, digitBuffer, numDigits );
}

static void n64_format_sdec32(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	int value
) {
	if( value >= 0 ) {
		n64_format_udec32( buffer, format, (unsigned int)value );
		return;
	} else {
		char digitBuffer[10];
		unsigned int numDigits = 0u;
		while( value ) {
			digitBuffer[numDigits++] = '0' + (char)(-(value % 10));
			value /= 10;
		}

		unsigned int minZeros = 0u;
		if( format->precision > numDigits ) {
			minZeros = format->precision - numDigits;
		} else if( !numDigits && format->precision ) {
			minZeros = 1u;
		}

		align_number( buffer, format, "-", 1u, minZeros, digitBuffer, numDigits );
	}
}

static inline void n64_format_sdec64(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	long long value
) {
	if( value >= 0 ) {
		n64_format_udec64( buffer, format, (unsigned long long)value );
		return;
	} else {
		char digitBuffer[19];
		unsigned int numDigits = 0u;
		while( value ) {
			digitBuffer[numDigits++] = '0' + (char)(-(value % 10ll));
			value /= 10ll;
		}

		unsigned int minZeros = 0u;
		if( format->precision > numDigits ) {
			minZeros = format->precision - numDigits;
		} else if( !numDigits && format->precision ) {
			minZeros = 1u;
		}

		align_number( buffer, format, "-", 1u, minZeros, digitBuffer, numDigits );
	}
}

static void n64_format_oct32(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	unsigned int value
) {
	char digitBuffer[11];
	unsigned int numDigits = 0u;
	while( value ) {
		digitBuffer[numDigits++] = '0' + (char)(value & 0x7u);
		value >>= 3;
	}

	unsigned int minZeros = 0u;
	if( format->precision > numDigits ) {
		minZeros = format->precision - numDigits;
	} else if( format->alternate ) {
		minZeros = 1u;
	} else if( !numDigits && format->precision ) {
		minZeros = 1u;
	}

	align_number( buffer, format, NULL, 0u, minZeros, digitBuffer, numDigits );
}

static inline void n64_format_oct64(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	unsigned long value
) {
	char digitBuffer[22];
	unsigned int numDigits = 0u;
	while( value ) {
		digitBuffer[numDigits++] = '0' + (char)(value & 0x7u);
		value >>= 3;
	}

	unsigned int minZeros = 0u;
	if( format->precision > numDigits ) {
		minZeros = format->precision - numDigits;
	} else if( format->alternate ) {
		minZeros = 1u;
	} else if( !numDigits && format->precision ) {
		minZeros = 1u;
	}
	
	align_number( buffer, format, NULL, 0u, minZeros, digitBuffer, numDigits );
}

static void n64_format_hex32(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	unsigned int value
) {
	char digitBuffer[8];
	unsigned int numDigits = 0u;

	const unsigned int prefixLen = (format->alternate && value) ? 2u : 0u;
	const char hb = (format->capitalize) ? ('A' - (char)10) : ('a' - (char)10);
	while( value ) {
		register const unsigned int h = value & 0xFu;
		digitBuffer[numDigits++] = (h < 10u) ? ('0' + (char)h) : (hb + (char)h);
		value >>= 4;
	}

	const char *const prefix = (format->capitalize) ? "0X" : "0x";
	const unsigned int minZeros = (format->precision > numDigits) ? (format->precision - numDigits) : 0u;

	align_number( buffer, format, prefix, prefixLen, minZeros, digitBuffer, numDigits );
}

static void n64_format_hex64(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	unsigned long long value
) {
	char digitBuffer[16];
	unsigned int numDigits = 0u;

	const unsigned int prefixLen = (format->alternate && value) ? 2u : 0u;
	const char hb = (format->capitalize) ? ('A' - (char)10) : ('a' - (char)10);
	while( value ) {
		register const unsigned int h = value & 0xFu;
		digitBuffer[numDigits++] = (h < 10u) ? ('0' + (char)h) : (hb + (char)h);
		value >>= 4;
	}

	const char *const prefix = (format->capitalize) ? "0X" : "0x";
	const unsigned int minZeros = (format->precision > numDigits) ? (format->precision - numDigits) : 0u;

	align_number( buffer, format, prefix, prefixLen, minZeros, digitBuffer, numDigits );
}

static void n64_format_bin32(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	unsigned int value
) {
	char digitBuffer[32];
	unsigned int numDigits = 0u;

	const unsigned int prefixLen = (format->alternate && value) ? 2u : 0u;
	while( value ) {
		digitBuffer[numDigits++] = (value & 1u) ? '1' : '0';
		value >>= 1;
	}

	const char *const prefix = (format->capitalize) ? "0B" : "0b";
	const unsigned int minZeros = (format->precision > numDigits) ? (format->precision - numDigits) : 0u;
	
	align_number( buffer, format, prefix, prefixLen, minZeros, digitBuffer, numDigits );
}

static void n64_format_bin64(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	unsigned long long value
) {
	char digitBuffer[64];
	unsigned int numDigits = 0u;

	const unsigned int prefixLen = (format->alternate && value) ? 2u : 0u;
	while( value ) {
		digitBuffer[numDigits++] = (value & 1ull) ? '1' : '0';
		value >>= 1;
	}

	const char *const prefix = (format->capitalize) ? "0B" : "0b";
	const unsigned int minZeros = (format->precision > numDigits) ? (format->precision - numDigits) : 0u;

	align_number( buffer, format, prefix, prefixLen, minZeros, digitBuffer, numDigits );
}

__attribute__((always_inline))
static inline void n64_format_hexreal_helper(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	const char *mbuff,
	unsigned int mdigits,
	char firstDigit,
	int exp,
	n64_bool sign,
	n64_bool nonzeroMantissa
) {
	unsigned int precision = format->precision;
	if( precision == 0xFFFFFFFFu ) {
		precision = mdigits;
	} else if( mdigits > precision ) {
		mdigits = precision;
	}

	unsigned int len = precision + 6u;
	if( sign || format->poschar != '\0' ) len++;
	if( format->alternate || nonzeroMantissa ) len++;
	if( exp >= 1000 || exp <= -1000 ) { len += 3; }
	else if( exp >= 100 || exp <= -100 ) { len += 2; }
	else if( exp >= 10 || exp <= -10 ) len++;

	if( !format->leftAligned && format->padchar == ' ' ) {
		for( unsigned int i = len; i < format->width; i++ ) n64_putc( buffer, ' ' );
	}

	if( sign ) {
		n64_putc( buffer, '-' );
	} else if( format->poschar != '\0' ) {
		n64_putc( buffer, format->poschar );
	}

	n64_putc( buffer, '0' );
	n64_putc( buffer, format->capitalize ? 'X' : 'x' );

	if( !format->leftAligned && format->padchar == '0' ) {
		for( unsigned int i = len; i < format->width; i++ ) n64_putc( buffer, '0' );
	}

	n64_putc( buffer, firstDigit );

	if( format->alternate || nonzeroMantissa ) {
		n64_putc( buffer, '.' );
	}

	for( unsigned int i = 0u; i < mdigits; i++ ) {
		n64_putc( buffer, mbuff[i] );
	}

	for( unsigned int i = mdigits; i < precision; i++ ) {
		n64_putc( buffer, '0' );
	}

	n64_putc( buffer, format->capitalize ? 'P' : 'p' );
	if( exp < 0 ) {
		n64_putc( buffer, '-' );
		exp = -exp;
	} else {
		n64_putc( buffer, '+' );
	}

	char ebuff[4];
	int elen = 0u;
	do {
		ebuff[elen++] = '0' + (char)(exp % 10);
		exp /= 10;
	} while( exp );

	for( int i = elen - 1; i >= 0; i-- ) {
		n64_putc( buffer, ebuff[i] );
	}

	if( format->leftAligned ) {
		for( unsigned int i = len; i < format->width; i++ ) n64_putc( buffer, ' ' );
	}
}

static inline void n64_format_hexfloat(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	float value
) {
	const unsigned int bits = n64_bit_cast_ftoi( value );

	const n64_bool sign = (n64_bool)(bits >> 31);
	int exp = ((int)(bits >> 23) & 0xFF) - 127;
	unsigned int mantissa = (bits & 0x7FFFFF) << 1;
	const n64_bool nonzeroMantissa = (mantissa != 0u);

	if( exp == 128 ) {
		// NaN or Infinity
		n64_format_nan_or_inf( buffer, format, sign, nonzeroMantissa );
		return;
	}

	char firstDigit = (nonzeroMantissa || exp != -127) ? '1' : '0';
	if( mantissa && format->precision < 6u ) {
		// rounding
		mantissa += 0x8u << (20u - (format->precision << 2));
		if( mantissa >> 24 ) {
			firstDigit++;
			mantissa = 0u;
		}
	}

	if( exp == -127 ) {
		if( nonzeroMantissa ) {
			// subnormal
			exp++;
			firstDigit--;
		} else {
			// zero
			exp = 0;
			firstDigit = '0';
		}
	}

	unsigned int mdigits;
	char mbuff[6];

	if( mantissa ) {
		mdigits = 6u;
		while( !(mantissa & 0xFu) ) {
			mdigits--;
			mantissa >>= 4;
		}

		const char *const hexits = format->capitalize ? "0123456789ABCDEF" : "0123456789abcdef";
		for( int i = (int)mdigits - 1; i >= 0; i-- ) {
			mbuff[i] = hexits[mantissa & 0xFu];
			mantissa >>= 4;
		}
	} else {
		mdigits = 0u;
	}

	n64_format_hexreal_helper( buffer, format, mbuff, mdigits, firstDigit, exp, sign, nonzeroMantissa );
}

static inline void n64_format_hexdouble(
	n64_bprint_buffer *buffer,
	const n64_format_args *format,
	double value
) {
	const unsigned long long bits = n64_bit_cast_dtol( value );

	const n64_bool sign = (n64_bool)(bits >> 63);
	int exp = ((int)(bits >> 52) & 0x7FF) - 1023;
	unsigned long long mantissa = bits & 0x000FFFFFFFFFFFFFull;
	const n64_bool nonzeroMantissa = (mantissa != 0ull);

	if( exp == 1024 ) {
		// NaN or Infinity
		n64_format_nan_or_inf( buffer, format, sign, nonzeroMantissa );
		return;
	}

	char firstDigit = (nonzeroMantissa || exp != -1023) ? '1' : '0';
	if( mantissa && format->precision < 13u ) {
		// rounding
		mantissa += 0x8ull << (48u - (format->precision << 2));
		if( mantissa >> 52 ) {
			firstDigit++;
			mantissa = 0ull;
		}
	}

	if( exp == -1023 ) {
		if( nonzeroMantissa ) {
			// subnormal
			exp++;
			firstDigit--;
		} else {
			// zero
			exp = 0;
			firstDigit = '0';
		}
	}

	unsigned int mdigits;
	char mbuff[13];

	if( mantissa ) {
		mdigits = 13u;
		while( !(mantissa & 0xFull) ) {
			mdigits--;
			mantissa >>= 4;
		}

		const char *const hexits = format->capitalize ? "0123456789ABCDEF" : "0123456789abcdef";
		for( int i = (int)mdigits - 1; i >= 0; i-- ) {
			mbuff[i] = hexits[mantissa & 0xFull];
			mantissa >>= 4;
		}
	} else {
		mdigits = 0u;
	}

	n64_format_hexreal_helper( buffer, format, mbuff, mdigits, firstDigit, exp, sign, nonzeroMantissa );
}

static inline void n64_format_pointer(
	n64_bprint_buffer *buffer,
	n64_format_args *format,
	const void *value
) {
	if( value == NULL ) {
		if( !format->leftAligned ) {
			for( unsigned int i = 5; i < format->width; i++ ) n64_putc( buffer, ' ' );
		}

		n64_putc( buffer, '(' );
		n64_putc( buffer, 'n' );
		n64_putc( buffer, 'i' );
		n64_putc( buffer, 'l' );
		n64_putc( buffer, ')' );

		if( format->leftAligned ) {
			for( unsigned int i = 5; i < format->width; i++ ) n64_putc( buffer, ' ' );
		}

		return;
	}

	format->alternate = true;
	format->capitalize = false;
	n64_format_hex32( buffer, format, (unsigned int)value );
}
