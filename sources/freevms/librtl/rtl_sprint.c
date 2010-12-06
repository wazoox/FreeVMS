/*
================================================================================
  FreeVMS (R)
  Copyright (C) 2010 Dr. BERTRAND Joël and all.

  This file is part of FreeVMS

  FreeVMS is free software; you can redistribute it and/or modify it
  under the terms of the CeCILL V2 License as published by the french
  CEA, CNRS and INRIA.
 
  FreeVMS is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the CeCILL V2 License
  for more details.
 
  You should have received a copy of the CeCILL License
  along with FreeVMS. If not, write to info@cecill.info.
================================================================================
*/

#include <freevms/freevms.h>

#define PUTCH(ch) do {          \
	if (size-- <= 0)            \
	    goto Done;              \
	*p++ = (ch);                \
	str_ptr++;                  \
	} while (0) 

#define PUTSTR(st) do {         \
    const char *s = (st);       \
    while (*s != '\0') {        \
        if (size-- <= 0)        \
            goto Done;          \
            *p++ = *s++;        \
			str_ptr++;          \
        }                       \
    } while (0)

#define PUTDEC(num) do {                                \
    uval = num;                                         \
    numdigits = 0;                                      \
    do {                                                \
        convert[numdigits++] = digits[uval % 10];       \
        uval /= 10;                                     \
    } while ( uval > 0 );                               \
    while (numdigits > 0)                               \
        PUTCH(convert[--numdigits]);                    \
    } while (0)

#define LEFTPAD(num) do {               \
    int cnt = (num);                    \
    if (!f_left && cnt > 0)             \
        while (cnt-- > 0)               \
        PUTCH(f_zero ? '0' : ' ');      \
    } while (0)

#define RIGHTPAD(num) do {              \
    int cnt = (num);                    \
    if (f_left && cnt > 0)              \
        while(cnt-- > 0)                \
            PUTCH( ' ' );               \
    } while (0)

/*
================================================================================
 Function vsnprintf (&str_desc, &fmt_desc, ap)

    Formated output conversion according to format string `fmt' and
    argument list `ap'.  The formated string is stored in `out'.  If
    the formated string is longer tha `outsize' characters, the
    string is truncated.

    The format conversion works as in printf(3), except for the
    conversions; e, E, g, and G which are not supported.
================================================================================
*/

void
rtl$sprint(struct vms$string *str_desc, struct vms$string *fmt_desc,
        void **arg)
{
    L4_ThreadId_t               tid;

    const char                  *digits;

    double                      fval;

    int                         base;
    int                         c;
    int                         f_alternate;
    int                         f_ldouble;
    int                         f_left;
    int                         f_long;
    int                         f_short;
    int                         f_sign;
    int                         f_space;
    int                         f_zero;
    int                         i;
    int                         numdigits;
    int                         numpr;
    int                         precision;
	int							sign;
    int                         signch;
    int                         someflag;
    int                         width;

    struct vms$string           *string_desc;

    unsigned char               convert[128];
    unsigned char               *p;

    unsigned long               uval;
    unsigned long               uval2;

    vms$pointer                 arg_ptr;
    vms$pointer                 fmt_ptr;
    vms$pointer                 size;
    vms$pointer                 str_ptr;
    vms$pointer                 string_ptr;

    size = str_desc->length;
    p = &(str_desc->c[0]);
    numpr = 0;
    arg_ptr = 0;

    /*
     * Sanity check on fmt string.
     */

    if (fmt_desc->length_trim == 0)
    {
        PUTSTR("(NULLIFIED FORMAT DESCRIPTOR)");
        goto Done;
    }

	fmt_ptr = 0;
	str_ptr = 0;

    while (size > 0)
    {
        /*
         * Copy characters until we encounter '%'.
         */

        while ((c = fmt_desc->c[fmt_ptr++]) != '%')
        {
            if (((size--) <= 0) || ((fmt_ptr - 1) == fmt_desc->length_trim))
            {
                goto Done;
            }

            str_desc->c[str_ptr++] = c;
            p++;
        }

		if ((fmt_ptr - 1) == fmt_desc->length_trim)
		{
			goto Done;
		}

        f_alternate = 0;
        f_left = 0;
        f_sign = 0;
        f_space = 0;
        f_zero = 0;
        someflag = 1;

        /*
         * Parse flags.
         */

        while(someflag)
        {
            switch(fmt_desc->c[fmt_ptr])
            {
                case '-': f_left = 1; fmt_ptr++; break;
                case '+': f_sign = 1; fmt_ptr++; break;
                case ' ': f_space = 1; fmt_ptr++; break;
                case '0': f_zero = 1; fmt_ptr++; break;
                case '#': f_alternate = 1; fmt_ptr++; break;
                default: someflag = 0; break;
            }
        }

		if ((fmt_ptr - 1) == fmt_desc->length_trim)
		{
			goto Done;
		}

        /*
         * Parse field width.
         */

        if ((c = fmt_desc->c[fmt_ptr]) == '*')
        {
            width = (*((int *) (arg[arg_ptr++])));
        }
        else if((c >= '0') && (c <= '9'))
        {
            width = 0;

            while(((c = fmt_desc->c[fmt_ptr++]) >= '0') && (c <= '9'))
            {
                width *= 10;
                width += c - '0';
            }

            fmt_ptr--;
        }
        else
        {
            width = -1;
        }

        /*
         * Parse precision.
         */

        if (fmt_desc->c[fmt_ptr] == '.')
        {
            if ((c = fmt_desc->c[++fmt_ptr]) == '*')
            {
                precision = (*((int *) (arg[arg_ptr++])));
                fmt_ptr++;
            }
            else if ((c >= '0') && (c <= '9'))
            {
                precision = 0;

                while(((c = fmt_desc->c[fmt_ptr++]) >= 0) && (c <= '9'))
                {
                    precision *= 10;
                    precision += c - '0';
                }

                fmt_ptr--;
            }
            else
            {
                precision = -1;
            }
        }
        else
        {
            precision = -1;
        }

        f_long = 0;
        f_short = 0;
        f_ldouble = 0;

        /*
         * Parse length modifier.
         */

        switch(fmt_desc->c[fmt_ptr])
        {
            case 'h': f_short= 1; fmt_ptr++; break;
            case 'l': f_long = 1, fmt_ptr++; break;
            case 'L': f_ldouble = 1; fmt_ptr++; break;
        }

        sign = 1;

        /*
         * Parse format conversion.
         */

        switch(c = fmt_desc->c[fmt_ptr++])
        {
            case 'b':
                uval = f_long ? (*((long *) (arg[arg_ptr++])))
                        : (*((int *) (arg[arg_ptr++])));
                base = 2;
                digits = "01";
                goto Print_unsigned;

            case 'o':
                uval = f_long ? (*((long *) (arg[arg_ptr++])))
                        : (*((int *) (arg[arg_ptr++])));
                base = 8;
                digits = "01234567";
                goto Print_unsigned;
                
            case 'p':
                precision = sizeof(long) * 2;
                width = precision;
                f_alternate = 1;

                if (sizeof(void *) == sizeof(long))
                {
                    f_long = 1;
                }

            case 'x':
                uval = f_long ? (*((long *) (arg[arg_ptr++])))
                        : (*((int *) (arg[arg_ptr++])));
                base = 16;
                digits = "0123456789abcdef";
                goto Print_unsigned;

            case 'X':
                uval = f_long ? (*((long *) (arg[arg_ptr++])))
                        : (*((int *) (arg[arg_ptr++])));
                base = 16;
                digits = "0123456789ABCDEF";
                goto Print_unsigned;

            case 'd':
            case 'i':
                uval = f_long ? (*((long *) (arg[arg_ptr++])))
                        : (*((int *) (arg[arg_ptr++])));
                base = 10;
                digits = "0123456789";
                goto Print_signed;

            case 'u':
                uval = f_long ? (*((long *) (arg[arg_ptr++])))
                        : (*((int *) (arg[arg_ptr++])));
                base = 10;
                digits = "0123456789";

            Print_unsigned:
                sign = 0;

            Print_signed:
                signch = 0;
                uval2 = uval;

                /*
                 * Check for sign character.
                 */

                if (sign)
                {
                    if (f_sign && ((long) uval >= 0))
                    {
                        signch = '+';
                    }
                    else if (f_space && ((long) uval >= 0))
                    {
                        signch = ' ';
                    }
                    else if ((long) uval < 0)
                    {
                        signch = '-';
                        uval = -((long) uval);
                    }
                }

                /*
                 * Create reversed number string.
                 */

                if ((sizeof(long) >= 8) && (base == 16))
                {
                    numdigits = 0;

                    do
                    {
                        convert[numdigits++] = digits[uval & 0xf];
                        uval /= 16;
                    } while(uval > 0);
                }
                else
                {
                    numdigits = 0;

                    do
                    {
                        convert[numdigits++] = digits[(unsigned int)
                                (uval % base)];
						uval /= base;
                    } while(uval > 0);
                }

                /*
                 * Calculate the actual size of the printed number.
                 */

                numpr = (numdigits > precision) ? numdigits : precision;

                if (signch)
                {
                    numpr++;
                }

                if (f_alternate)
                {
                    if (base == 8)
                    {
                        numpr++;
                    }
                    else if ((base == 16) || (base == 2))
                    {
                        numpr += 2;
                    }
                }

                /*
                 * Insert left padding.
                 */

                if ((!f_left) && (width > numpr))
                {
                    if (f_zero)
                    {
                        numpr = width;
                    }
                    else
                    {
                        for(i = width - numpr; i > 0; i--)
                        {
                            PUTCH(' ');
                        }
                    }
                }

                /*
                 * Insert sign character.
                 */

                if (signch)
                {
                    PUTCH(signch);
                    numpr--;
                }

                /*
                 * Insert number prefix.
                 */

                if (f_alternate)
                {
                    if (base == 2)
                    {
                        numpr -= 2;
                        PUTSTR("% ");
                    }
                    else if (base == 8)
                    {
                        numpr--;
                        PUTCH('o');
                    }
                    else if (base == 16)
                    {
                        numpr -= 2;
                        PUTSTR("$ ");
                    }
                }

                /*
                 * Insert zero padding.
                 */

                for(i = numpr - numdigits; i > 0; i--)
                {
                    PUTCH('0');
                }

                /*
                 * Insert number.
                 */

                while(numdigits > 0)
                {
                    PUTCH(convert[--numdigits]);
                }

                RIGHTPAD((width - numpr) - (signch ? 1 : 0));
                break;

            case 'f': 
            {
                fval = (*((double *) (arg[arg_ptr++])));

                if (precision == -1)
                {
                    precision = 6;
                }

                /*
                 * Check for sign character.
                 */

                if (f_sign && (fval >= 0.0))
                {
                    signch = '+';
                }
                else if (f_space && (fval >= 0.0))
                {
                    signch = ' ';
                }
                else if (fval < 0.0)
                {
                    signch = '-';
                    fval = -fval;
                }
                else
                {
                    signch = 0;
                }

                /*
                 * Get the integer part of the number.  If the floating
                 * point value is greater than the maximum value of an
                 * unsigned long, the result is undefined.
                 */

                uval = (unsigned long) fval;
                numdigits = 0;

                do
                {
                    convert[numdigits++] = '0' + (uval % 10);
                    uval /= 10;
                } while(uval > 0);

                /*
                 * Calculate the actual size of the printed number.
                 */

                numpr = numdigits + (signch ? 1 : 0);

                if (precision > 0)
                {
                    numpr += (1 + precision);
                }

                LEFTPAD(width - numpr);

                /*
                 * Insert sign character.
                 */

                if (signch)
                {
                    PUTCH(signch);
                }

                /*
                 * Insert integer number.
                 */

                while(numdigits > 0)
                {
                    PUTCH(convert[--numdigits]);
                }

                /*
                 * Insert precision.
                 */

                if (precision > 0)
                {
                    /*
                     * Truncate number to fractional part only.
                     */

                    while (fval >= 1.0)
                    {
                        fval -= (double) (unsigned long) fval;
                    }

                    PUTCH('.');

                    /*
                     * Insert precision digits.
                     */
                    while(precision-- > 0)
                    {
                        fval *= 10.0;
                        uval = (unsigned long) fval;
                        PUTCH( '0' + uval );
                        fval -= (double) (unsigned long) fval;
                    }
                }

                RIGHTPAD(width - numpr);
                break;
            }

            case 't':
                tid = (*((L4_ThreadId_t *) (arg[arg_ptr++])));

                if (L4_IsNilThread(tid))
                {
                    PUTSTR("NIL THREAD IDENTIFIER");
                    break;
                }

                digits = "0123456789";

                if (L4_IsGlobalId(tid))
                {
                    PUTSTR("GlobalThreadId=");
                    PUTDEC(tid.global.X.thread_no);
                    PUTSTR(", ver=");
                    PUTDEC(tid.global.X.version);
                } 
                else if (L4_IsLocalId(tid))
                {
                    PUTSTR("LocalThreadid=");
                    PUTDEC(tid.local.X.local_id);
                }
                else
                {
                    PUTSTR("MalformedThreadId=");
                    PUTDEC(tid.raw);
                }
                break;

            case 's':
                string_desc = ((struct vms$string *) (arg[arg_ptr++]));
                string_ptr = 0;

                if (width > 0)
                {
                    /*
                     * Calculate printed size.
                     */

                    numpr = string_desc->length_trim;

                    if ((precision >= 0) && (precision < numpr))
                    {
                        numpr = precision;
                    }

                    LEFTPAD(width - numpr);
                }

                /*
                 * Insert string.
                 */

                if (precision >= 0)
                {
                    while((precision-- > 0) &&
                            (string_ptr < string_desc->length_trim))
                    {
                        PUTCH(string_desc->c[string_ptr++]);
                    }
                }
                else
                {
                    while(string_ptr < string_desc->length_trim)
                    {
                        PUTCH(string_desc->c[string_ptr++]);
                    }
                }

                RIGHTPAD(width - numpr);
                break;

            case 'c':
                PUTCH((*((int *) (arg[arg_ptr++]))));
                break;

            case '%':
                PUTCH('%');
                break;

            default:
                PUTCH('%');
                PUTCH(c);
                break;
        }
    }

Done:

    /*
     * Return size of printed string.
     */

    str_desc->length_trim = p - &(str_desc->c[0]);
    return;
}
