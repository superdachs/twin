/*
 *  getsizes.c  --  get sizeof() various data types and print it.
 *
 *  Copyright (C) 1999-2000 by Massimiliano Ghilardi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "autoconf.h"

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

/* This mess was adapted from the GNU getpagesize.h.  */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifndef HAVE_GETPAGESIZE

# ifdef _SC_PAGESIZE
#  define getpagesize() sysconf(_SC_PAGESIZE)
# else /* no _SC_PAGESIZE */
#  ifdef HAVE_ASM_PAGE_H
#   include <asm/page.h>
#  endif
#  ifdef HAVE_SYS_PARAM_H
#   include <sys/param.h>
#  endif
#  ifdef EXEC_PAGESIZE
#   define getpagesize() EXEC_PAGESIZE
#  else /* no EXEC_PAGESIZE */
#   ifdef NBPG
#    define getpagesize() NBPG * CLSIZE
#    ifndef CLSIZE
#     define CLSIZE 1
#    endif /* no CLSIZE */
#   else /* no NBPG */
#    ifdef NBPC
#     define getpagesize() NBPC
#    else /* no NBPC */
#     ifdef PAGE_SIZE
#      define getpagesize() PAGE_SIZE
#     else /* no PAGE_SIZE */
#      ifdef PAGESIZE      
#	define getpagesize() PAGESIZE
#      else
#	error cannot detect mmap() page size
#      endif /* no PAGESIZE */
#     endif /* no PAGE_SIZE */
#    endif /* no NBPC */
#   endif /* no NBPG */
#  endif /* no EXEC_PAGESIZE */
# endif /* no _SC_PAGESIZE */
#endif /* no HAVE_GETPAGESIZE */


#include <Tw/datatypes.h>

#ifdef HAVE_LONG_LONG
typedef unsigned long long ul;
# define UL "ull"
# define LX "%llX"
# define LX8 "%8llX"
#else
typedef unsigned long ul;
# define UL "ul"
# define LX "%lX"
# define LX8 "%8lX"
#endif

static int my_memcmp(byte *m1, byte *m2, uldat len) {
    int c = 0;
    while (len--) {
	if ((c = (int)*m1++ - (int)*m2++))
	    return c;
    }
    return c;
}

int main(void) {
    uldat endian_uldat = 0x04030201;
    byte *endian_str = (byte *)&endian_uldat;
    char *byte_order, *str_byte16, *str_byte32;
    
    char *names[] = { "uldat", "time_t", "frac_t", "topaque" };
    int sizes[] = { sizeof(uldat), sizeof(time_t), sizeof(frac_t), sizeof(size_t) };
    
    ul maxes[] = { MAXULDAT, MAXTIME_T, MAXFRAC_T, MAXU(size_t) };
    ul mins [] = {    0    , MINTIME_T, MINFRAC_T,      0       };
    
    int i, i_tlargest, i_tany;
    
    if (sizeof(byte) != 1 || sizeof(udat) < 2 || sizeof(uldat) < 4 ||
	sizeof(hwcol) != 1 || sizeof(time_t) < 4 || sizeof(frac_t) < 4) {
	
	fprintf(stderr, "Fatal: minimum requirements on data sizes not satisfied.\nSee scripts/getsizes.c\n");
	return 1;
    }
    if (sizeof(size_t) != sizeof(void *)) {
	fprintf(stderr, "Fatal: sizeof(size_t) != sizeof(void *)\nYour compiler and/or includes are seriously screwed up!\n");
	return 1;
    }
    
    if (sizeof(udat) == 2)
	str_byte16 = "udat";
    else if (sizeof(unsigned short) == 2)
	str_byte16 = "unsigned short";
    else {
	fprintf(stderr, "Fatal: could not find a 16-bit type to use as byte16.\nSee scripts/getsizes.c\n");
	return 1;
    }

    if (sizeof(udat) == 4)
	str_byte32 = "udat";
    else if (sizeof(uldat) == 4)
	str_byte32 = "uldat";
    else if (sizeof(unsigned int) == 4)
	str_byte32 = "unsigned int";
    else if (sizeof(unsigned long) == 4)
	str_byte32 = "unsigned long";
    else {
	fprintf(stderr, "Fatal: could not find a 32-bit type to use as byte32.\nSee scripts/getsizes.c\n");
	return 1;
    }

    for (i_tlargest = 0, i = 1; i < sizeof(sizes)/sizeof(sizes[0]) - 1/* skip size_t */ ; i++) {
	if (sizes[i_tlargest] < sizes[i])
	    i_tlargest = i;
    }
    for (i_tany = 0, i = 1; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
	if (sizes[i_tany] < sizes[i])
	    i_tany = i;
    }

    if (!my_memcmp(endian_str, "\1\2\3\4", 4))
	byte_order = "1234"; /* little endian */
    else if (!my_memcmp(endian_str + (sizeof(uldat) - 4), "\4\3\2\1", 4))
	byte_order = "4321"; /* big endian */
    else {
	fprintf(stderr, "Fatal: cannot determine byte order: not little endian, not big endian!\n"
		"       endianity test on data 0x04030201 returned 0x" LX8 "!\n",
		(ul)endian_str[0] | ((ul)endian_str[1]<<8) | ((ul)endian_str[2]<<16) | ((ul)endian_str[3]<<24));
	return 1;
    }
    
    /*
     * note about MINSBYTE, MINDAT, MINLDAT, MINTIME_T, MINFRAC_T:
     * if the type is unsigned, -(ul)0 == 0;
     * if the type is signed, extending to (ul) will fill higher bytes with 0xFF.
     * For example doing (ul)0x80000000 when (ul) is 8 bytes will give
     * 0xFFFFFFFF80000000, and -(ul)0x80000000 will give back the correct 0x80000000.
     */
    printf("\n"
	   "/*\n"
	   " * This file was automatically generated by 'scripts/Getsizes'. Do no edit!\n"
	   " */\n"
	   "\n"
	   "#ifndef _TW_DATASIZES_H\n"
	   "#define _TW_DATASIZES_H\n"
	   "\n"
	   "/* the biggest integer data type used by libTw. will be same as uldat or time_t or frac_t */\n"
	   "typedef %s	tlargest;\n"
	   "\n"
	   "/* an unsigned integer type as wide as (void *) */\n"
	   "typedef size_t	topaque;\n"
	   "\n"
	   "/* an integer type as wide as the bigger between tlargest and topaque (void *) */\n"
	   "typedef %s	tany;\n"
	   "\n"
	   "#define MAXTLARGEST    MAXSU(tlargest)\n"
	   "#define MAXTOPAQUE     MAXSU(topaque)\n"
	   "#define MAXTANY        MAXSU(tany)\n"
	   "\n"
	   "#define MINTLARGEST    MINSU(tlargest)\n"
	   "#define MINTANY        MINSU(tany)\n"
	   "\n"
	   "#define TW_SIZEOFBYTE         %d\n"
	   "#define TW_SIZEOFUDAT         %d\n"
	   "#define TW_SIZEOFULDAT        %d\n"
	   "#define TW_SIZEOFTIME_T       %d\n"
	   "#define TW_SIZEOFFRAC_T       %d\n"
	   "#define TW_SIZEOFTLARGEST     %d\n"
	   "#define TW_SIZEOFTOPAQUE      %d\n"
	   "#define TW_SIZEOFVOIDP        %d\n"
	   "#define TW_SIZEOFTANY         %d\n"
	   "\n"
	   "#define TW_MAXSBYTE    0x" LX "\n"
	   "#define TW_MAXBYTE     0x" LX "\n"
	   "#define TW_MAXDAT      0x" LX "\n"
	   "#define TW_MAXUDAT     0x" LX "\n"
	   "#define TW_MAXLDAT     0x" LX "\n"
	   "#define TW_MAXULDAT    0x" LX "\n"
	   "#define TW_MAXTIME_T   0x" LX "\n"
	   "#define TW_MAXFRAC_T   0x" LX "\n"
	   "#define TW_MAXTLARGEST 0x" LX "\n"
	   "#define TW_MAXTOPAQUE  0x" LX "\n"
	   "#define TW_MAXTANY     0x" LX "\n"
	   "\n"
	   "#define TW_MINSBYTE    0x" LX "\n"
	   "#define TW_MINDAT      0x" LX "\n"
	   "#define TW_MINLDAT     0x" LX "\n"
	   "#define TW_MINTIME_T   0x" LX "\n"
	   "#define TW_MINFRAC_T   0x" LX "\n"
	   "#define TW_MINTLARGEST 0x" LX "\n"
	   "#define TW_MINTANY     0x" LX "\n"
	   "\n"
	   "#define TW_BYTE16      %s\n"
	   "#define TW_BYTE32      %s\n"
	   "\n"
	   "#define TW_PAGE_SIZE        %d\n"
	   "\n"
	   "#define TW_BYTE_ORDER       %s\n"
	   "#define TW_LITTLE_ENDIAN    1234\n"
	   "#define TW_BIG_ENDIAN       4321\n"
	   "\n"
	   "#endif /* _TW_DATASIZES_H */\n",
	   names[i_tlargest], names[i_tany],
	   
	   sizeof(byte), sizeof(udat), sizeof(uldat),
	   sizeof(time_t), sizeof(frac_t),
	   sizes[i_tlargest], sizeof(size_t), sizeof(void *), sizes[i_tany],
	   

	   (ul)MAXSBYTE, (ul)MAXBYTE,
	   (ul)MAXDAT, (ul)MAXUDAT,
	   (ul)MAXLDAT, (ul)MAXULDAT,
	   (ul)MAXTIME_T, (ul)MAXFRAC_T,
	   (ul)maxes[i_tlargest], (ul)MAXU(size_t), (ul)maxes[i_tany],
	   
	   -(ul)MINSBYTE, -(ul)MINDAT, -(ul)MINLDAT,
	   -(ul)MINTIME_T, -(ul)MINFRAC_T,
	   -(ul)mins[i_tlargest], -(ul)mins[i_tany],
	   
	   str_byte16,
	   str_byte32,
	   (int)getpagesize(),
	   byte_order
	   );
    return 0;
}
