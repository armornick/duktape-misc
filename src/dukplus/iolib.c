/*
I/O Library for Duktape (based on stdio)
Based on the liolib.c file of the Lua 5.2.3 source.
*/

#include <stdio.h>
#include <duktape.h>


#if !defined(duk_checkmode)

/*
** Check whether 'mode' matches '[rwa]%+?b?'.
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#define duk_checkmode(mode) \
	(*mode != '\0' && strchr("rwa", *(mode++)) != NULL &&	\
	(*mode != '+' || ++mode) &&  /* skip if char is '+' */	\
	(*mode != 'b' || ++mode) &&  /* skip if char is 'b' */	\
	(*mode == '\0'))

#endif

