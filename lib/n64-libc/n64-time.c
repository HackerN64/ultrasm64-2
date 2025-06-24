#include "n64-time.h"

#include "n64-util.h"
#include "n64-string.h"

static const n64_tm N64_UNIX_EPOCH_TM = {
	0, 0, 0, 1, 0, 70, 4, 0, -1
};

static const char *s_weekdayNames[7] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

static const char *s_monthNames[12] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

#define N64_SECONDS_IN_MINUTE 60ll
#define N64_SECONDS_IN_HOUR (60ll * N64_SECONDS_IN_MINUTE)
#define N64_SECONDS_IN_DAY (24ll * N64_SECONDS_IN_HOUR)
#define N64_SECONDS_IN_NON_LEAP_YEAR (365ll * N64_SECONDS_IN_DAY)

static unsigned int s_si_buffer[16] __attribute__((aligned(16)));
static unsigned int s_si_backup[16] __attribute__((aligned(16)));
static n64_bool s_exec_on_write_bug = false;
static unsigned int s_prev_dma_addr;

static const int s_yday_table[12] = {
	0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

typedef enum {
	N64_RTC_INIT_CALLED = 0x1,
	N64_RTC_NOT_WAITING = 0x2,
	N64_RTC_GOOD = 0x4,

	N64_RTC_READY = N64_RTC_INIT_CALLED | N64_RTC_NOT_WAITING
} n64_rtc_state;

static n64_rtc_state g_rtc_state = 0;
static n64_clock_t g_wait_start = 0u;
static n64_clock_t g_wait_end = 0u;

static inline void __attribute__((always_inline)) si_await_op() {
	// Wait for the SI operation we started to finish, then clear the status
	while( !(*((volatile unsigned int*)0xa4300008u) & *((volatile unsigned int*)0xa430000Cu) & 0x2u) );
	*((volatile unsigned int*)0xa4800018u) = 0u;
}

static inline void __attribute__((always_inline)) si_wait_safe( n64_bool yield ) {
	// If the SI is currently busy, wait until it isn't
	while( *((volatile unsigned int*)0xa4800018u) & 0x3 ) {
		n64_set_interrupts( yield );
		while( *((volatile unsigned int*)0xa4800018u) & 0x3 );
		n64_set_interrupts( false );
	}
}

static void n64_pif_save() {
	s_prev_dma_addr = *((volatile unsigned int*)0xa4800000u);

	// Save the state of PIF RAM to memory to restore it later.
	// The joybus is only executed when using an SI DMA read, and not when doing
	// a direct read via the memory-mapped address. Thus, this has no side effects.

	volatile unsigned int *si_pif_ram = (volatile unsigned int *)0xbfc007c0u;
	for( int i = 0; i < 16; i++ ) {
		s_si_backup[i] = si_pif_ram[i];
	}
}

static void n64_pif_restore() {
	if( s_exec_on_write_bug ) {
		// The emulator incorrectly executes the joybus on a DMA write instead of a read
		// Clear the command register so it doesn't execute again
		s_si_backup[15] &= 0xffffff00u;
	} else {
		// Re-parse the stored PIF RAM state
		s_si_backup[15] |= 1u;
	}

	__builtin_mips_cache( 0x19, &s_si_backup[0] );
	__builtin_mips_cache( 0x19, &s_si_backup[4] );
	__builtin_mips_cache( 0x19, &s_si_backup[8] );
	__builtin_mips_cache( 0x19, &s_si_backup[12] );

	*((volatile unsigned int*)0xa4800000u) = (unsigned int)s_si_backup & 0x1FFFFFFFu;
	asm volatile( "":::"memory" );
	*((volatile unsigned int*)0xa4800010u) = 0x1fc007c0u;
	asm volatile( "":::"memory" );
	si_await_op();
	
	*((volatile unsigned int*)0xa4800000u) = s_prev_dma_addr;
}

static void n64_joybus_dma_write() {
	__builtin_mips_cache( 0x19, &s_si_buffer[0] );
	__builtin_mips_cache( 0x19, &s_si_buffer[4] );
	__builtin_mips_cache( 0x19, &s_si_buffer[8] );
	__builtin_mips_cache( 0x19, &s_si_buffer[12] );

	*((volatile unsigned int*)0xa4800000u) = (unsigned int)s_si_buffer & 0x1FFFFFFFu;
	asm volatile( "":::"memory" );
	*((volatile unsigned int*)0xa4800010u) = 0x1fc007c0u;
	asm volatile( "":::"memory" );

	si_await_op();
}

static void n64_joybus_dma_read() {
	*((volatile unsigned int*)0xa4800000u) = (unsigned int)s_si_buffer & 0x1FFFFFFFu;
	asm volatile( "":::"memory" );
	*((volatile unsigned int*)0xa4800004u) = 0x1fc007c0u;
	asm volatile( "":::"memory" );

	si_await_op();

	__builtin_mips_cache( 0x11, &s_si_buffer[0] );
	__builtin_mips_cache( 0x11, &s_si_buffer[4] );
	__builtin_mips_cache( 0x11, &s_si_buffer[8] );
	__builtin_mips_cache( 0x11, &s_si_buffer[12] );
}

__attribute__((always_inline))
static inline void n64_joybus_exec() {
	n64_joybus_dma_write();
	n64_joybus_dma_read();
}

static inline unsigned char decode_rtc_byte( unsigned char x ) {
	return (((x & 0xF0) >> 4) * 10) + (x & 0x0F);
}

static inline n64_bool is_leap_year( long long year ) {
	if( year % 4ll != 0ll ) return false;
	if( year % 100ll == 0ll ) return (year % 400ll == 0ll) ? true : false;
	return false;
}

static n64_time_t year_to_unix_time( long long year ) {
	register long long leapYears = -478ll;
	leapYears += (year + 3ll) >> 2;
	leapYears -= ((year > 0ll) ? (year + 99ll) : year) / 100ll;
	leapYears += ((year > 0ll) ? (year + 399ll) : year) / 400ll;

	return (
		((year - 1970ll) * N64_SECONDS_IN_NON_LEAP_YEAR) +
		(leapYears * N64_SECONDS_IN_DAY)
	);
}

n64_time_t n64_to_unix_time( const n64_tm *time ) {
	int month = time->tm_mon % 12;
	if( month < 0 ) month += 12;

	long long days = (long long)s_yday_table[month] + (long long)time->tm_mday - 1ll;
	if( month >= 2 && is_leap_year( (long long)time->tm_year + 1900ll ) ) days++;
	return (
		year_to_unix_time( (long long)time->tm_year + 1900ll + (long long)(time->tm_mon / 12) ) +
		(days * N64_SECONDS_IN_DAY) +
		((long long)time->tm_hour * N64_SECONDS_IN_HOUR) +
		((long long)time->tm_min * N64_SECONDS_IN_MINUTE) +
		(long long)time->tm_sec
	);
}

__attribute__((always_inline))
static inline n64_bool is_emulator() {
	register const volatile unsigned int *const dpc = (const volatile unsigned int*)0xA4100000u;
	return !(dpc[5] | dpc[6] | dpc[7]);
}

n64_bool n64_rtc_init() {
	if( g_rtc_state & N64_RTC_INIT_CALLED ) {
		return g_rtc_state != N64_RTC_READY;
	}

	const n64_bool intr = n64_set_interrupts( false );
	if( g_rtc_state & N64_RTC_INIT_CALLED ) {
		n64_set_interrupts( intr );
		return g_rtc_state != N64_RTC_READY;
	}

	si_wait_safe( intr );
	g_rtc_state |= N64_RTC_INIT_CALLED;
	n64_pif_save();

	s_si_buffer[0] = 0u;
	s_si_buffer[1] = 0xff010306u;
	s_si_buffer[2] = 0xfffffffeu;
	n64_memset( &s_si_buffer[3], 0, 48u );
	s_si_buffer[15] = 1u;
	n64_joybus_dma_write();

	if(
		*((volatile unsigned char*)0xbfc007c6u) != 0x03 ||
		*((volatile unsigned char*)0xbfc007c8u) != 0xff
	) {
		// On hardware (and accurate emulators such as Ares), writing to the joybus merely causes it to parse the command.
		// The commands are not actually executed until a DMA read is performed. So only the final byte should have changed.
		s_exec_on_write_bug = true;
	}

	n64_joybus_dma_read();

	if( s_si_buffer[2] >> 8 != 0x001000u ) {
		n64_pif_restore();
		n64_set_interrupts( intr );
		return false;
	}

	g_rtc_state |= N64_RTC_GOOD;
	g_wait_start = n64_clock();
	g_wait_end = g_wait_start + (N64_CLOCKS_PER_SEC / 50u);

	s_si_buffer[0] = 0u;
	s_si_buffer[1] = 0x02090700u;
	s_si_buffer[2] = 0u;
	s_si_buffer[3] = 0u;
	s_si_buffer[4] = 0x00fe0000u;
	n64_memset( &s_si_buffer[5], 0, 40u );
	s_si_buffer[15] = 1u;
	n64_joybus_exec();

	s_si_buffer[1] = 0x0a010800u;
	s_si_buffer[2] = 0x03000000u;
	s_si_buffer[4] = 0x00fe0000u;
	s_si_buffer[15] = 1u;
	n64_joybus_exec();

	if( is_emulator() ) {
		g_rtc_state |= N64_RTC_NOT_WAITING;
	}

	n64_pif_restore();
	n64_set_interrupts( intr );
	return true;
}

n64_bool n64_rtc_ready( n64_bool *good ) {
	if( good ) *good = (g_rtc_state & N64_RTC_GOOD) >> 2;

	if( !(g_rtc_state & N64_RTC_INIT_CALLED) ) return false;
	if( !(g_rtc_state & N64_RTC_NOT_WAITING) ) {
		const n64_clock_t now = n64_clock();
		if( g_wait_end < g_wait_start ) {
			if( now >= g_wait_end && now < g_wait_start ) {
				g_rtc_state |= N64_RTC_NOT_WAITING;
				return true;
			} else {
				return false;
			}
		} else {
			if( now < g_wait_start || now >= g_wait_end ) {
				g_rtc_state |= N64_RTC_NOT_WAITING;
				return true;
			} else {
				return false;
			}
		}
	}
	
	return true;
}

n64_bool n64_rtc_read( n64_tm *arg ) {
	if( !(g_rtc_state & N64_RTC_READY) ) {
		n64_rtc_init();
		while( !n64_rtc_ready( NULL ) );
	}

	if( !(g_rtc_state & N64_RTC_GOOD) ) {
		n64_memcpy( arg, &N64_UNIX_EPOCH_TM, sizeof( n64_tm ) );
		return false;
	}

	const n64_bool intr = n64_set_interrupts( false );
	si_wait_safe( intr );
	n64_pif_save();

	s_si_buffer[0] = 0u;
	s_si_buffer[1] = 0x02090702u;
	s_si_buffer[2] = 0x00008001u;
	s_si_buffer[3] = 0x04017000u;
	s_si_buffer[4] = 0x80fe0000u;
	n64_memset( &s_si_buffer[5], 0, 40u );
	s_si_buffer[15] = 1u;
	n64_joybus_exec();

	const unsigned char *const data = (const unsigned char*)&s_si_buffer[2];
	arg->tm_sec = (int)decode_rtc_byte( data[0] );
	arg->tm_min = (int)decode_rtc_byte( data[1] );
	arg->tm_hour = (int)decode_rtc_byte( data[2] - 0x80 );
	arg->tm_mday = (int)decode_rtc_byte( data[3] );
	arg->tm_wday = (int)decode_rtc_byte( data[4] );
	arg->tm_mon = (int)decode_rtc_byte( data[5] ) - 1u;
	arg->tm_year = (100 * (int)decode_rtc_byte( data[7] )) + (int)decode_rtc_byte( data[6] );
	if( arg->tm_mon < 12 ) arg->tm_yday = s_yday_table[arg->tm_mon] + arg->tm_mday - 1;
	arg->tm_isdst = -1;

	if( arg->tm_mon >= 2 && is_leap_year( 1900ll + (long long)arg->tm_year ) ) {
		arg->tm_yday++;
	}

	n64_pif_restore();
	n64_set_interrupts( intr );
	return true;
}

n64_tm *n64_tm_add( n64_tm *time, n64_time_t seconds ) {
	seconds += n64_to_unix_time( time );
	return n64_gmtime_r( &seconds, time );
}

n64_time_t n64_time( n64_time_t *arg ) {
	n64_tm now;
	register const n64_time_t ts = n64_rtc_read( &now ) ? n64_to_unix_time( &now ) : (n64_time_t)-1;
	if( arg ) *arg = ts;
	return ts;
}

n64_time_t n64_mktime( n64_tm *time ) {
	const n64_time_t ts = n64_to_unix_time( time );
	n64_gmtime_r( &ts, time );
	return ts;
}

static inline void setMonthAndDay( long long year, int yday, int *mon, int *mday ) {
	if( yday < 31 ) {
		*mon = 0;
		*mday = yday + 1;
		return;
	} else if( yday < 59 ) {
		*mon = 1;
		*mday = yday - 30;
		return;
	} else if( yday == 59 ) {
		if( is_leap_year( year ) ) {
			*mon = 1;
			*mday = 29;
		} else {
			*mon = 2;
			*mday = 1;
		}
		return;
	}

	if( is_leap_year( year ) ) yday--;
	if( yday >= 334 ) {
		*mon = 11;
		*mday = yday - 333;
	} else for( int month = (yday + 4) >> 5;; month++ ) {
		if( yday < s_yday_table[month+1] ) {
			*mon = month;
			*mday = yday + 1 - s_yday_table[month];
			return;
		}
	}
}

n64_tm *n64_gmtime_r( const n64_time_t *timer, n64_tm *buf ) {
	buf->tm_isdst = 0;

	if( *timer >= 67768036191676800ll ) {
		// set to maximum valid n64_tm
		buf->tm_sec = 59;
		buf->tm_min = 59;
		buf->tm_hour = 23;
		buf->tm_mday = 31;
		buf->tm_mon = 11;
		buf->tm_year = 0x7FFFFFFF;
		buf->tm_wday = 3;
		buf->tm_yday = 364;
		return NULL;
	} else if( *timer < -67768040609721748ll ) {
		// set to minimum valid n64_tm
		buf->tm_sec = 0;
		buf->tm_min = 0;
		buf->tm_hour = 0;
		buf->tm_mday = 1;
		buf->tm_mon = 0;
		buf->tm_year = -0x80000000;
		buf->tm_wday = 4;
		buf->tm_yday = 0;
		return NULL;
	}

	long long yearGuess = 1970ll + (*timer / 31556952ll);
	long long yearTs = year_to_unix_time( yearGuess );
	if( *timer < yearTs ) {
		do {
			yearTs = year_to_unix_time( --yearGuess );
		} while( *timer < yearTs );
	} else if( yearGuess > 0x7fffffffll ) {
		yearGuess = 0x7fffffffll;
		yearTs = year_to_unix_time( yearGuess );
	} else {
		register const long long nextYearTs = year_to_unix_time( yearGuess + 1ll );
		if( *timer >= nextYearTs ) {
			yearGuess++;
			yearTs = nextYearTs;
		}
	}

	int ts = (int)(*timer - yearTs);
	buf->tm_year = (int)(yearGuess - 1900ll);
	buf->tm_yday = ts / (int)N64_SECONDS_IN_DAY;
	ts %= (int)N64_SECONDS_IN_DAY;
	setMonthAndDay( yearGuess, buf->tm_yday, &buf->tm_mon, &buf->tm_mday );
	buf->tm_hour = ts / (int)N64_SECONDS_IN_HOUR;
	ts %= (int)N64_SECONDS_IN_HOUR;
	buf->tm_min = ts / (int)N64_SECONDS_IN_MINUTE;
	buf->tm_sec = ts % (int)N64_SECONDS_IN_MINUTE;
	buf->tm_wday = (int)((4ll + (*timer / N64_SECONDS_IN_DAY)) % 7ll);
	if( buf->tm_wday < 0 ) buf->tm_wday += 7;

	return buf;
}

static inline n64_bool strftime_push_text( char *str, unsigned int *i, unsigned int count, const char *text ) {
	for( ; *text; (*i)++ ) {
		if( *i >= count - 1 ) return false;
		str[*i] = *(text++);
	}
	return true;
}

static inline n64_bool strftime_push_number( char *str, unsigned int *i, unsigned int count, long long n ) {
	if( n < 0ll ) {
		str[(*i)++] = '-';
		n = -n;
	}

	char buff[17];
	unsigned int j = 0u;
	do {
		buff[j++] = '0' + (char)(n % 10ll);
		n /= 10ll;
	} while( n );

	if( *i + j < *i || *i + j >= count ) return false;
	for( int k = (int)j - 1; k >= 0; k-- ) {
		str[(*i)++] = buff[k];
	}

	return true;
}

static inline int get_wby_days( int day, int wday ) {
	return day - ((day - wday + 382) % 7) + 3;
}

static long long get_wby_years( long long year, int day, int wday ) {
	if( get_wby_days( day, wday ) < 0 ) {
		return year - 1ll;
	} else if( get_wby_days( day - (is_leap_year( year ) ? 366ll : 365ll), wday ) > 0 ) {
		return year + 1ll;
	} else {
		return year;
	}
}

static int get_wby_weeks( long long year, int day, int wday ) {
	int days = get_wby_days( day, wday );
	if( days < 0 ) {
		days = get_wby_days( day + (is_leap_year( year ) ? 366ll : 365ll), wday );
	} else {
		const int days2 = get_wby_days( day - (is_leap_year( year ) ? 366ll : 365ll), wday );
		if( days2 > 0 ) days = days2;
	}

	return 1 + (days / 7);
}

unsigned int n64_strftime_internal( char *str, unsigned int count, const char *format, const n64_tm *tp ) {
	const long long year = (long long)tp->tm_year + 1900ll;
	for( unsigned int i = 0; i < count; format++ ) {
		if( !*format ) {
			str[i] = '\0';
			return i;
		} else if( *format != '%' ) {
			str[i++] = *format;
			continue;
		}

		format++;
		const n64_bool alt = (*format == '0');
		if( *format == 'E' || alt ) {
			format++;
		}

		switch( *format ) {
			case '%':
				str[i++] = '%';
				break;
			case 'n':
				str[i++] = '\n';
				break;
			case 't':
				str[i++] = '\t';
				break;
			case 'Y':
				if( !strftime_push_number( str, &i, count, year ) ) return 0;
				break;
			case 'y':
				if( i < count - 2 ) {
					register int yy = (int)(year % 100ll);
					if( yy < 0 ) yy += 100;
					str[i++] = '0' + (char)(yy / 10);
					str[i++] = '0' + (char)(yy % 10);
					break;
				} else return 0;
			case 'C':
				if( !strftime_push_number( str, &i, count, year / 100ll ) ) return 0;
				break;
			case 'G':
				if( !strftime_push_number( str, &i, count, get_wby_years(year, tp->tm_yday, tp->tm_wday ) ) ) return 0;
				break;
			case 'g':
				if( i < count - 2 ) {
					register int wyear = (int)(get_wby_years(year, tp->tm_yday, tp->tm_wday ) % 100ll);
					if( wyear < 0 ) wyear += 100;
					str[i++] = '0' + (char)(wyear / 10);
					str[i++] = '0' + (char)(wyear % 10);
					break;
				} else return 0;
				break;
			case 'b':
			case 'h':
				if( i >= count - 3 ) return 0;
				str[i++] = s_monthNames[tp->tm_mon][0];
				str[i++] = s_monthNames[tp->tm_mon][1];
				str[i++] = s_monthNames[tp->tm_mon][2];
				break;
			case 'B':
				if( !strftime_push_text( str, &i, count, s_monthNames[tp->tm_mon] ) ) return 0;
				break;
			case 'm':
				if( i >= count - 2 ) return 0;
				str[i++] = '0' + (char)((tp->tm_mon + 1) / 10);
				str[i++] = '0' + (char)((tp->tm_mon + 1) % 10);
				break;
			case 'U':
				if( i >= count - 2 ) return 0;
				str[i++] = '0' + (char)((tp->tm_yday - tp->tm_wday + 7) / 70);
				str[i++] = '0' + (char)(((tp->tm_yday - tp->tm_wday + 7) / 7) % 10);
				break;
			case 'W':
				if( i >= count - 2 ) return 0;
				str[i++] = '0' + (char)((tp->tm_yday - ((tp->tm_wday + 6) % 7) + 7) / 70);
				str[i++] = '0' + (char)(((tp->tm_yday - ((tp->tm_wday + 6) % 7) + 7) / 7) % 10);
				break;
			case 'V':
				if( i < count - 2 ) {
					register const int week = get_wby_weeks(year, tp->tm_yday, tp->tm_wday );
					str[i++] = '0' + (char)(week / 10);
					str[i++] = '0' + (char)(week % 10);
					break;
				} else return 0;
			case 'j':
				if( i >= count - 3 ) return 0;
				str[i++] = '0' + (char)(tp->tm_yday / 100);
				str[i++] = '0' + (char)((tp->tm_yday / 10) % 10);
				str[i++] = '0' + (char)(tp->tm_yday % 10);
				break;
			case 'd':
				if( i >= count - 2 ) return 0;
				str[i++] = '0' + (char)(tp->tm_mday / 10);
				str[i++] = '0' + (char)(tp->tm_mday % 10);
				break;
			case 'e':
				if( i >= count - 2 ) return 0;
				if( alt || tp->tm_mday > 9) {
					str[i++] = '0' + (char)(tp->tm_mday / 10);
					str[i++] = '0' + (char)(tp->tm_mday % 10);
				} else {
					str[i++] = ' ';
					str[i++] = '0' + (char)tp->tm_mday;
				}
				break;
			case 'a':
				if( i >= count - 3 ) return 0;
				str[i++] = s_weekdayNames[tp->tm_wday][0];
				str[i++] = s_weekdayNames[tp->tm_wday][1];
				str[i++] = s_weekdayNames[tp->tm_wday][2];
				break;
			case 'A':
				if( !strftime_push_text( str, &i, count, s_weekdayNames[tp->tm_wday] ) ) return 0;
				break;
			case 'w':
				str[i++] = '0' + (char)tp->tm_wday;
				break;
			case 'u':
				str[i++] = tp->tm_wday ? ('0' + (char)tp->tm_wday) : '7';
				break;
			case 'H':
				if( i >= count - 2 ) return 0;
				str[i++] = '0' + (char)(tp->tm_hour / 10);
				str[i++] = '0' + (char)(tp->tm_hour % 10);
				break;
			case 'I':
				if( i >= count - 2 ) return 0;
				str[i++] = '0' + (char)((1 + ((tp->tm_hour + 11) % 12)) / 10);
				str[i++] = '0' + (char)((1 + ((tp->tm_hour + 11) % 12)) % 10);
				break;
			case 'M':
				if( i >= count - 2 ) return 0;
				str[i++] = '0' + (char)(tp->tm_min / 10);
				str[i++] = '0' + (char)(tp->tm_min % 10);
				break;
			case 'S':
				if( i >= count - 2 ) return 0;
				str[i++] = '0' + (char)(tp->tm_sec / 10);
				str[i++] = '0' + (char)(tp->tm_sec % 10);
				break;
			case 'p':
				if( i >= count - 2 ) return 0;
				str[i++] = (tp->tm_hour < 12) ? 'A' : 'P';
				str[i++] = 'M';
				break;
			case 'c':
				if( i < count - 21 ) {
					const unsigned int j = n64_strftime_internal( &str[i], count - (unsigned int)i, "%a %b %e %H:%M:%S %Y", tp );
					if( !j ) return 0;
					i += j;
					break;
				} else return 0;
			case 'x':
			case 'D':
				if( i >= count - 8 ) return 0;
				i += n64_strftime_internal( &str[i], count - (unsigned int)i, "%m/%d/%y", tp );
				break;
			case 'X':
			case 'T':
				if( i >= count - 8 ) return 0;
				i += n64_strftime_internal( &str[i], count - (unsigned int)i, "%H:%M:%S", tp );
				break;
			case 'F':
				if( i < count - 7 ) {
					const unsigned int j = n64_strftime_internal( &str[i], count - (unsigned int)i, "%Y-%m-%d", tp );
					if( !j ) return 0;
					i += j;
					break;
				} else return 0;
			case 'r':
				if( i >= count - 11 ) return 0;
				i += n64_strftime_internal( &str[i], count - (unsigned int)i, "%I:%M:%S %p", tp );
				break;
			case 'R':
				if( i >= count - 5 ) return 0;
				i += n64_strftime_internal( &str[i], count - (unsigned int)i, "%H:%M", tp );
				break;
			default:
				return 0;
		}
	}

	return 0;
}

unsigned int n64_strftime( char *str, unsigned int count, const char *format, const n64_tm *tp ) {
	n64_tm time;
	n64_memcpy( &time, tp, sizeof( n64_tm ) );
	n64_mktime( &time );

	str[count - 1] = '\0';
	return n64_strftime_internal( str, count, format, &time );
}
