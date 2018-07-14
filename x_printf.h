/*
 * Copyright 2014-18 Andre M Maree / KSS Technologies (Pty) Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/*
 *	x_printf.h
 *
 *	Valid formatting characters
 *	!#'*+-%0.0-9ABC D EFGHI J KL M O PQRS T U VWXYZ
 *	|||||||||\_/ab|c|defgh|i|jk|lmn|opqr|s|t|uvwxy|z
 *	||||||||| | ||||||||||||||||||||||||||||||||||||
 *	||||||||| | |||||||||||||||||||||||||||||||||||*----> UNUSED
 *	||||||||| | ||||||||||||||||||||||||||||||||||*-----> DTZ
 *	||||||||| | |||||||||||||||||||||||||||||||||*------> UNUSED
 *	||||||||| | ||||||||||||||||||||||||||||||||*---> HEX UC/lc value
 *	||||||||| | |||||||||||||||||||||||||||||||*--------> HEXDUMP, word (u32) sized values UC/lc
 *	||||||||| | ||||||||||||||||||||||||||||||*---------> UNUSED
 *	||||||||| | |||||||||||||||||||||||||||||*------> UNSIGNED number (uint32_t)
 *	||||||||| | ||||||||||||||||||||||||||||*-----------> UNUSED
 *	||||||||| | |||||||||||||||||||||||||||*--------> Not implemented
 *	||||||||| | ||||||||||||||||||||||||||*-------------> TIME
 *	||||||||| | |||||||||||||||||||||||||*----------> STRING null terminated ascii
 *	||||||||| | ||||||||||||||||||||||||*-----------> Not implemented
 *	||||||||| | |||||||||||||||||||||||*----------------> UNUSED
 *	||||||||| | ||||||||||||||||||||||*-----------------> UNUSED
 *	||||||||| | |||||||||||||||||||||*--------------> POINTER U32 address with 0x prefix
 *	||||||||| | ||||||||||||||||||||*---------------> OCTAL value
 *	||||||||| | |||||||||||||||||||*--------------------> UNUSED
 *	||||||||| | ||||||||||||||||||*-----------------> Not implemented
 *	||||||||| | |||||||||||||||||*----------------------> MAC address UC/lc
 *	||||||||| | ||||||||||||||||*-------------------> LLONG modifier
 *	||||||||| | |||||||||||||||*--------------------> Not implemented
 *	||||||||| | ||||||||||||||*-------------------------> UNUSED (future SI unit scaling)
 *	||||||||| | |||||||||||||*----------------------> Not implemented
 *	||||||||| | ||||||||||||*---------------------------> BINARY U32 string
 *	||||||||| | |||||||||||*------------------------> INTEGER same as 'd
 *	||||||||| | ||||||||||*-----------------------------> IP address
 *	||||||||| | |||||||||*------------------------------> HEXDUMP, halfword (u16) sized values UC/lc
 *	||||||||| | ||||||||*---------------------------> FLOAT best suited format
 *	||||||||| | |||||||*----------------------------> FLOAT fixed format
 *	||||||||| | ||||||*-----------------------------> FLOAT exponential format
 *	||||||||| | |||||*------------------------------> DECIMAL signed formatted long
 *	||||||||| | ||||*-----------------------------------> DATE formatted
 *	||||||||| | |||*--------------------------------> CHAR single U8
 *	||||||||| | ||*---------------------------------> Not implemented
 *	||||||||| | |*--------------------------------------> HEXDUMP, byte (u8) sized values UC/lc
 *	||||||||| | *-----------------------------------> Not implemented!!
 *	||||||||| *--> field width specifiers
 *	||||||||*----> FLOAT fractional field size separator
 *	|||||||*-----> PAD0 enable flag
 *	||||||*------> format specifier flag
 *	|||||*-------> LEFT justification, HEXDUMP remove address info
 *	||||*--------> SIGN leading '+' or '-',	DTZ=Add full TZ info, HEXDUMP add ASCII info
 *	|||*---------> Minwid or precision variable provided
 *	||*----------> DECIMAL 3 digits group	DTZ "::." -> "hms"	DUMP use '|-+'
 *	|*-----------> DTZ=alt form (Sun, 10 Sep 2017 20:50:37 GMT) IP=ntohl() Hex=Reverse order
 *	*------------> DTZ=elapsed time,	DUMP=relative addr,
 *
 * Examples:
 * "%'03llz"			- print binary representation, optional separator, llong & field width modifiers
 * "%['!#+ll]{BbHhWw}"	- hexdump of memory area, USE 2 PARAMETERS FOR START AND LENGTH !!!!
 * 							MUST NOT specify "*" or "." or "*." or .*", this will screw up the parameter sequence
 * "%[-0]I"				- print IP address, justified left or right (pad 0 or ' ')
 * "%[']{Mm}"			- prints MAC address, optional ':' separator, upper/lower case
 * "%[!']D"				- POSIX [relative/altform] date (1 parameter, pointer to TSZ_t
 * "%[!']T"				- POSIX [relative/altform] time
 * "%[!']Z"				- POSIX [relative/altform] date, time & zone
 *
 */

#pragma once

#include	"x_definitions.h"
#include	"x_buffers.h"
#include	"x_sockets.h"
#include	"x_stdio.h"

#if		(ESP32_PLATFORM == 1)
	#include	<regex.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ###################### control functionality included in xprintf.c ##############################

#define	xpfSUPPORT_BINARY				1
#define	xpfSUPPORT_MAC_ADDR				1
#define	xpfSUPPORT_IP_ADDR				1
#define	xpfSUPPORT_HEXDUMP				1
#define	xpfSUPPORT_POINTER				1
#define	xpfSUPPORT_DATETIME				1
#define	xpfSUPPORT_IEEE754				1		// float point support in x_printf.c functions

#define	xpfMAXIMUM_DECIMALS				15
#define	xpfDEFAULT_DECIMALS				6

/* Specifically for the ESP-IDF we force output to x[v]printf(format, ...) to be treated same
 * as x[v]fprintf(stdout, format, ...) with locking enabled */
#define	xpfSUPPORT_ALIASES				1
#define	xpfSUPPORT_ALLOW_NUL			0

// ################################## x[snf]printf() related #######################################

#define	xpfMAX_DIGITS_FRAC_SEC			3

#if		(xpfMAX_DIGITS_FRAC_SEC == 0)
	#define	xpfTIME_FRAC_SEC_DIVISOR		(1000000)
#elif	(xpfMAX_DIGITS_FRAC_SEC == 1)
	#define	xpfTIME_FRAC_SEC_DIVISOR		(100000)
#elif	(xpfMAX_DIGITS_FRAC_SEC == 2)
	#define	xpfTIME_FRAC_SEC_DIVISOR		(10000)
#elif	(xpfMAX_DIGITS_FRAC_SEC == 3)
	#define	xpfTIME_FRAC_SEC_DIVISOR	(1000)
#elif	(xpfMAX_DIGITS_FRAC_SEC == 4)
	#define	xpfTIME_FRAC_SEC_DIVISOR		(100)
#elif	(xpfMAX_DIGITS_FRAC_SEC == 5)
	#define	xpfTIME_FRAC_SEC_DIVISOR		(10)
#elif	(xpfMAX_DIGITS_FRAC_SEC == 6)
	#define	xpfTIME_FRAC_SEC_DIVISOR		(1)
#else
	#error "Invalid values for xpfMAX_DIGITS_FRAC_SEC (must be 0 -> 6)"
#endif


#define	xpfMAX_LEN_FRAC_SEC				(xpfMAX_DIGITS_FRAC_SEC + 3)
#define	xpfMAX_LEN_TIME					(sizeof("12:34:56.") + xpfMAX_LEN_FRAC_SEC)
#define	xpfMAX_LEN_DATE					sizeof("2015-04-01T")
#define	xpfMAX_LEN_DTZ					(xpfMAX_LEN_DATE + xpfMAX_LEN_TIME + configTIME_MAX_LEN_TZINFO)

#define xpfMAX_LEN_X32					sizeof("+4,294,967,295")
#define xpfMAX_LEN_X64					sizeof("+18,446,744,073,709,551,615")

#define xpfMAX_LEN_F32					(sizeof("+4,294,967,295.") + xpfMAXIMUM_DECIMALS)
#define xpfMAX_LEN_F64					(sizeof("+18,446,744,073,709,551,615.") + xpfMAXIMUM_DECIMALS)

#define	xpfMAX_LEN_B32					sizeof("1010-1010|1010-1010 1010-1010|1010-1010")
#define	xpfMAX_LEN_B64					(xpfMAX_LEN_B32 * 2)

#define	xpfMAX_LEN_IP					sizeof("123.456.789.012")
#define	xpfMAX_LEN_MAC					sizeof("01:23:45:67:89:ab")

#define	xpfNULL_STRING					"'null'"

#define	xpfFLAGS_NONE					((xpf_t) 0ULL)

// Used in hexdump to determine size of each value conversion
#define	xpfSIZING_BYTE					0			// 8 bit byte
#define	xpfSIZING_SHORT					1			// 16 bit short
#define	xpfSIZING_WORD					2			// 32 bit word
#define	xpfSIZING_DWORD					3			// 64 bit long long word

#define	xpfFORMAT_0_G					0			// FLOAT 'G' or 'g' else NONE
#define	xpfFORMAT_1_F					1			// FLOAT 'F' or 'f' else COLON
#define	xpfFORMAT_2_E					2			// FLOAT 'E' or 'e'
#define	xpfFORMAT_3						3			// COMPLEX

/* For HEXDUMP functionality the size is as follows
 * ( 0x12345678 {32 x 3} [ 32 x Char]) = 142 plus some safety = 160 characters max.
 * When done in stages max size about 96 */
#define	xpfHEXDUMP_WIDTH				32			// number of bytes (as bytes/short/word/llong) in a single row

/* Maximum size is determined by bit width of maxlen and curlen fields below */
#define	xpfMAXLEN_BITS					16			// Number of bits in field(s)
#define	xpfMAXLEN_MAXVAL				((1 << xpfMAXLEN_BITS) - 1)

#define	xpfMINWID_BITS					16			// Number of bits in field(s)
#define	xpfMINWID_MAXVAL				((1 << xpfMINWID_BITS) - 1)
#define	xpfMINWID_MINVAL				0

#define	xpfPRECIS_BITS					16			// Number of bits in field(s)
#define	xpfPRECIS_MAXVAL				((1 << xpfPRECIS_BITS) - 1)
#define	xpfPRECIS_MINVAL				0

typedef	union xpf_u {
	struct {
		uint32_t	lengths ;							// maxlen & curlen ;
		uint32_t	limits ;							// minwid & precision
		uint32_t	flags ;								// rest of flags
	} ;
	struct {
	/* Be careful about changing the size of the next 4 fields since the 16bits each
	 * align directly with the uint32_t fields "limits and "flags" above. The "flags" field is
	 * used to reset the flags in the man vPrint routine */
		uint32_t	maxlen		: xpfMAXLEN_BITS ;		// max chars to output 0 = unlimited
		uint32_t	curlen		: xpfMAXLEN_BITS ;		// number of chars output so far
		uint32_t	minwid		: xpfMINWID_BITS ;		// minimum field width
		uint32_t	precision	: xpfPRECIS_BITS ;		// number of decimal digits or width of string
	// byte 0
		uint8_t		group 		: 1 ;					// 0 = disable, 1 = enable
		uint8_t		alt_form	: 1 ;					// '#'
		uint8_t		ljust		: 1 ;					// if "%-[0][1-9]{diouxX}" then justify LEFT ie pad on right
		uint8_t		Ucase		: 1 ;					// true = 'a' or false = 'A'
		uint8_t		pad0		: 1 ;					// true = pad with leading'0'
		uint8_t		llong		: 1 ;					// long long override flag
		uint8_t		radix		: 1 ;					// radix found flag
		uint8_t		abs_rel		: 1 ;					// relative address / elapsed time
	// byte 1
		uint8_t		nbase 		: 5 ;					// 2, 8, 10 or 16
		uint8_t		size		: 2 ;					// size of value ie byte / half / word / lword
		uint8_t		negvalue	: 1 ;					// if value < 0
	// byte 2
		uint8_t		form		: 2 ;					// format specifier in FLOAT & DUMP
		uint8_t		signed_val	: 1 ;					// true = content is signed value
		uint8_t		plus		: 1 ;					// true = force use of '+' or '-' signed
		uint8_t		arg_width	: 1 ;					// read arg for width
		uint8_t		arg_prec	: 1 ;					// read arg for precision
		uint8_t		yday_ok		: 1 ;					// day of year
		uint8_t		year_ok		: 1 ;					// year
	// byte 3
		uint8_t		mon_ok		: 1 ;					// month
		uint8_t		dow_ok		: 1 ;					// day of week
		uint8_t		mday_ok		: 1 ;					// day of month
		uint8_t		hour_ok		: 1 ;					// hours
		uint8_t		min_ok		: 1 ;					// minutes
		uint8_t		sec_ok		: 1 ;					// seconds
	// unused
		uint8_t		no_zone		: 1 ;					// 1=delay doing 'Z'
		uint8_t		spare		: 1 ;
	} ;
} xpf_t ;

typedef	struct xpc_s {
	int 	(*handler)(struct xpc_s * , int ) ;
	union {
		void *		pVoid ;
		char *		pStr ;
		FILE *		stream ;
		sock_ctx_t * psSock ;						// socket context pointer
		buf_t * 	psBuf ;
		int 		(*DevPutc)(int ) ;
	} ;
	xpf_t	f ;
} xpc_t ;

/*
 * Public function prototypes for extended functionality version of stdio supplied functions
 * These names MUST be used if any of the extended functionality is used in a format string
 */
int		xPrint(int (handler)(xpc_t *, int), void * pVoid, size_t BufSize, const char *format, va_list args) ;

int 	xvsnprintf(char * , size_t , const char * , va_list ) ;
int 	xsnprintf(char * , size_t , const char * , ...) ;
int 	xvsprintf(char * , const char * , va_list ) ;
int		xsprintf(char * , const char * , ...) ;

void	cprintf_lock(void) ;
void	cprintf_unlock(void) ;

int 	vcprintf(const char * format, va_list vArgs) ;
int 	cprintf(const char * format, ...) ;

int 	vdevprintf(int (* handler)(int ), const char * format, va_list vArgs) ;
int 	devprintf(int (* handler)(int), const char * format, ...) ;

int 	xvnprintf(size_t count, const char * format, va_list args) ;
int 	xvprintf(const char * , va_list ) ;
int 	xnprintf(size_t Count, const char * format, ...) ;
int		xprintf(const char * format, ...) ;

int		xvfprintf(FILE * , const char * , va_list ) ;
int		xfprintf(FILE * , const char * , ...) ;

/*
 * non standard additions
 */
int 	vsocprintf(sock_ctx_t * psSock, const char * format, va_list args) ;
int 	socprintf(sock_ctx_t * psSock, const char * format, ...) ;

int     vbufprintf(buf_t * psBuf, const char * , va_list) ;
int     bufprintf(buf_t * psBuf, const char * , ...) ;

// ############################## LOW LEVEL DIRECT formatted output ###############################

int32_t	_xprintf(const char * format, ...) ;

// ##################################### functional tests ##########################################

#ifdef __cplusplus
}
#endif
