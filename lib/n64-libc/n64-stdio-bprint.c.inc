#include "n64-stddef.h"

#include "n64-stdio-format.c.inc"

typedef enum {
	CSI_ALIGNMENT,
	CSI_SIGN,
	CSI_SPACE,
	CSI_ALTERNATE,
	CSI_ZEROPAD,
	CSI_WIDTH,
	CSI_PRECISION,
	CSI_LENGTH,
	CSI_FORMAT
} n64_csi;

#define PCS_VARIABLE 0xAAAAAAAAu
#define PCS_DEFAULT 0xBBBBBBBBu

static const char *parse_conversion_specifier( const char *format, n64_format_args *args ) {
	n64_bool isLong = false;

	args->width = 0u;
	args->dataSize = 4u;
	args->precision = PCS_DEFAULT;
	args->defaultPrecision = 1u;
	args->leftAligned = false;
	args->alternate = false;
	args->capitalize = false;
	args->padchar = ' ';
	args->poschar = '\0';

	n64_csi csi = CSI_ALIGNMENT;
	while( *format != '\0' ) {
		switch( csi ) {
			case CSI_ALIGNMENT:
				if( *format == '-' ) {
					args->leftAligned = true;
					format++;
				}
				csi = CSI_SIGN;
				__attribute__((fallthrough));
			case CSI_SIGN:
				if( *format == '+' ) {
					args->poschar = '+';
					format++;
				}
				csi = CSI_SPACE;
				__attribute__((fallthrough));
			case CSI_SPACE:
				if( *format == ' ' ) {
					if( args->poschar != '+' ) args->poschar = ' ';
					format++;
				}
				csi = CSI_ALTERNATE;
				__attribute__((fallthrough));
			case CSI_ALTERNATE:
				if( *format == '#' ) {
					args->alternate = true;
					format++;
				}
				csi = CSI_ZEROPAD;
				__attribute__((fallthrough));
			case CSI_ZEROPAD:
				if( *format == '0' ) {
					args->padchar = '0';
					format++;
				}
				csi = CSI_WIDTH;
				__attribute__((fallthrough));
			case CSI_WIDTH:
				if( *format == '-' ) {
					args->leftAligned = true;
					format++;
					continue;
				} else if( *format == '0' ) {
					format++;
					continue;
				} else if( *format == '*' ) {
					args->width = PCS_VARIABLE;
					format++;
				} else if( *format > '0' && *format <= '9' ) {
					int w = 0;
					while( *format >= '0' && *format <= '9' ) {
						w *= 10;
						if( w < 0 ) return NULL;
						w += (int)*format - (int)'0';
						format++;
					}
					args->width = (unsigned int)w;
				}
				csi = CSI_PRECISION;
				__attribute__((fallthrough));
			case CSI_PRECISION:
				if( *format == '.' ) {
					format++;
					if( *format == '-' ) {
						format++;
						while( *format >= '0' && *format <= '9' ) format++;
					} else if( *format == '*' ) {
						args->precision = PCS_VARIABLE;
						format++;
					} else if( *format >= '0' && *format <= '9' ) {
						int p = 0;
						while( *format >= '0' && *format <= '9' ) {
							p *= 10;
							if( p < 0 ) return NULL;
							p += (int)*format - (int)'0';
							format++;
						}
						args->precision = (unsigned int)p;
					} else {
						return NULL;
					}
				}
				csi = CSI_LENGTH;
				__attribute__((fallthrough));
			case CSI_LENGTH:
				switch( *format ) {
					case 'h':
						format++;
						if( *format == 'h' ) {
							args->dataSize = 1u;
							format++;
						} else {
							args->dataSize = 2u;
						}
						break;
					case 'l':
						isLong = true;
						args->dataSize = __SIZEOF_LONG__;
						format++;
						if( *format == 'l' ) {
							args->dataSize = 8u;
							format++;
						}
						break;
					case 'j':
					case 'L':
						isLong = true;
						args->dataSize = 8u;
						format++;
						break;
					case 'z':
						args->dataSize = __SIZEOF_SIZE_T__;
						format++;
						break;
					case 't':
						args->dataSize = __SIZEOF_PTRDIFF_T__;
						format++;
						break;
					default:
						break;
				}
				csi = CSI_FORMAT;
				__attribute__((fallthrough));
			case CSI_FORMAT:
				switch( *format ) {
					case 'c':
						args->type = 'c';
						args->dataSize = 1;
						break;
					case 's':
						args->type = 's';
						args->dataSize = 1;
						args->defaultPrecision = 0xFFFFFFFFu;
						break;
					case 'd':
					case 'i':
						args->type = 'd';
						if( args->precision != PCS_DEFAULT ) args->padchar = ' ';
						break;
					case 'o':
						args->type = 'o';
						args->poschar = '\0';
						if( args->precision != PCS_DEFAULT ) args->padchar = ' ';
						break;
					case 'X':
						args->capitalize = true;
						__attribute__((fallthrough));
					case 'x':
						args->type = 'x';
						args->poschar = '\0';
						if( args->precision != PCS_DEFAULT ) args->padchar = ' ';
						break;
					case 'B':
						args->capitalize = true;
						__attribute__((fallthrough));
					case 'b':
						args->type = 'b';
						args->poschar = '\0';
						if( args->precision != PCS_DEFAULT ) args->padchar = ' ';
						break;
					case 'u':
						args->type = 'u';
						args->poschar = '\0';
						if( args->precision != PCS_DEFAULT ) args->padchar = ' ';
						break;
					case 'F':
						args->capitalize = true;
						__attribute__((fallthrough));
					case 'f':
						args->type = 'f';
						args->dataSize = isLong ? 8u : 4u;
						args->defaultPrecision = 6u;
						break;
					case 'E':
						args->capitalize = true;
						__attribute__((fallthrough));
					case 'e':
						args->type = 'e';
						args->dataSize = isLong ? 8u : 4u;
						args->defaultPrecision = 6u;
						break;
					case 'A':
						args->capitalize = true;
						__attribute__((fallthrough));
					case 'a':
						args->type = 'a';
						args->dataSize = isLong ? 8u : 4u;
						args->defaultPrecision = 0xffffffffu;
						break;
					case 'G':
						args->capitalize = true;
						__attribute__((fallthrough));
					case 'g':
						args->type = 'g';
						args->dataSize = isLong ? 8u : 4u;
						args->defaultPrecision = 6u;
						break;
					case 'n':
						args->type = 'n';
						break;
					case 'p':
						args->type = 'p';
						args->dataSize = 4u;
						break;
					default:
						return NULL;
				}
				return ++format;
			default:
				return NULL;
		}
	}

	return NULL;
}

int n64_vbprintf( char *_buffer, unsigned int bufsz, n64_bool(*callback)(void*, unsigned int), void *state, const char *format, __builtin_va_list args ) {
	n64_bprint_buffer buff = {
		/* start */ _buffer,
		/* head */ _buffer,
		/* end */ _buffer + bufsz,
		callback,
		state,
		0u
	};

	while( *format != '\0' ) {
		if( *format != '%' ) {
			n64_putc( &buff, *format );
			format++;
			continue;
		}

		if( format[1] == '%' ) {
			n64_putc( &buff, '%' );
			format += 2;
			continue;
		}

		n64_format_args fmt;
		format = parse_conversion_specifier( ++format, &fmt );
		if( !format ) return -1;

		if( fmt.width == PCS_VARIABLE ) {
			const int w = __builtin_va_arg( args, int );
			if( w < 0 ) {
				fmt.leftAligned = true;
				fmt.width = (unsigned int)-w;
			} else {
				fmt.width = (unsigned int)w;
			}
		}

		if( fmt.precision == PCS_VARIABLE ) {
			const int p = __builtin_va_arg( args, int );
			fmt.precision = (p < 0) ? fmt.defaultPrecision : (unsigned int)p;
		} else if( fmt.precision == PCS_DEFAULT ) {
			fmt.precision = fmt.defaultPrecision;
		}

		switch( fmt.type ) {
			case 'c': {
				const char c = (char)__builtin_va_arg( args, int );
				n64_format_char( &buff, &fmt, c );
				break;
			}
			case 's': {
				const char *str = (const char*)__builtin_va_arg( args, const char* );
				n64_format_string( &buff, &fmt, str );
				break;
			}
			case 'd': {
				if( fmt.dataSize == 8 ) {
					const long long val = __builtin_va_arg( args, long long );
					n64_format_sdec64( &buff, &fmt, val );
				} else {
					int val = __builtin_va_arg( args, int );
					if( fmt.dataSize == 2 ) {
						val = (int)(short)val;
					} else if( fmt.dataSize == 1 ) {
						val = (int)(signed char)val;
					}
					n64_format_sdec32( &buff, &fmt, val );
				}
				break;
			}
			case 'u': {
				if( fmt.dataSize == 8 ) {
					const unsigned long long val = __builtin_va_arg( args, unsigned long long );
					n64_format_udec64( &buff, &fmt, val );
				} else {
					unsigned int val = __builtin_va_arg( args, unsigned int );
					if( fmt.dataSize == 2 ) {
						val &= 0xFFFFu;
					} else if( fmt.dataSize == 1 ) {
						val &= 0xFFu;
					}
					n64_format_udec32( &buff, &fmt, val );
				}
				break;
			}
			case 'o': {
				if( fmt.dataSize == 8 ) {
					const unsigned long long val = __builtin_va_arg( args, unsigned long long );
					n64_format_oct64( &buff, &fmt, val );
				} else {
					unsigned int val = __builtin_va_arg( args, unsigned int );
					if( fmt.dataSize == 2 ) {
						val &= 0xFFFFu;
					} else if( fmt.dataSize == 1 ) {
						val &= 0xFFu;
					}
					n64_format_oct32( &buff, &fmt, val );
				}
				break;
			}
			case 'x': {
				if( fmt.dataSize == 8 ) {
					const unsigned long long val = __builtin_va_arg( args, unsigned long long );
					n64_format_hex64( &buff, &fmt, val );
				} else {
					unsigned int val = __builtin_va_arg( args, unsigned int );
					if( fmt.dataSize == 2 ) {
						val &= 0xFFFFu;
					} else if( fmt.dataSize == 1 ) {
						val &= 0xFFu;
					}
					n64_format_hex32( &buff, &fmt, val );
				}
				break;
			}
			case 'b': {
				if( fmt.dataSize == 8 ) {
					const unsigned long long val = __builtin_va_arg( args, unsigned long long );
					n64_format_bin64( &buff, &fmt, val );
				} else {
					unsigned int val = __builtin_va_arg( args, unsigned int );
					if( fmt.dataSize == 2 ) {
						val &= 0xFFFFu;
					} else if( fmt.dataSize == 1 ) {
						val &= 0xFFu;
					}
					n64_format_bin32( &buff, &fmt, val );
				}
				break;
			}
			case 'f':
			case 'e':
			case 'g': {
				const double val = __builtin_va_arg( args, double );
				if( fmt.dataSize == 8 ) {
					n64_format_double( &buff, &fmt, val );
				} else {
					n64_format_float( &buff, &fmt, (float)val );
				}
				break;
			}
			case 'a': {
				const double val = __builtin_va_arg( args, double );
				if( fmt.dataSize == 8 ) {
					n64_format_hexdouble( &buff, &fmt, val );
				} else {
					n64_format_hexfloat( &buff, &fmt, (float)val );
				}
				break;
			}
			case 'p': {
				const void *ptr = __builtin_va_arg( args, const void* );
				n64_format_pointer( &buff, &fmt, ptr );
				break;
			}
			case 'n': {
				switch( fmt.dataSize ) {
					case 1: *__builtin_va_arg( args, signed char* ) = (signed char)buff.total; break;
					case 2: *__builtin_va_arg( args, short* ) = (short)buff.total; break;
					case 4: *__builtin_va_arg( args, int* ) = (int)buff.total; break;
					case 8: *__builtin_va_arg( args, long long* ) = (long long)buff.total; break;
					default: return -1;
				}
				break;
			}
			default:
				return -1;
		}
	}

	if( buff.start != buff.end ) {
		buff.callback( buff.userdata, (unsigned int)(buff.head - buff.start) );
	}
	return (int)buff.total;
}

int n64_bprintf( char *buffer, unsigned int bufsz, n64_bool(*callback)(void*, unsigned int), void *state, const char *format, ... ) {
	__builtin_va_list args;
	__builtin_va_start( args, format );
	register const int result = n64_vbprintf( buffer, bufsz, callback, state, format, args );
	__builtin_va_end( args );
	return result;
}
