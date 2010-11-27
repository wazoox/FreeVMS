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

#include "freevms/freevms.h"
#include "./symbols.h"

static int
dbg$addrcmp(const char *s1, const char *s2)
{
    unsigned int        i;

    vms$pointer         v1;
    vms$pointer         v2;

    v1 = 0;
    for(i = 0; i < L4_SIZEOFWORD; i++)
    {
        if (s1[(L4_SIZEOFWORD - 1) - i] == '1')
        {
            v1 += ((vms$pointer) 1) << i;
        }
    }

    v2 = 0;
    for(i = 0; i < L4_SIZEOFWORD; i++)
    {
        if (s2[(L4_SIZEOFWORD - 1) - i] == '1')
        {
            v2 += ((vms$pointer) 1) << i;
        }
    }

    if (v1 < v2)
    {
        return(-1);
    }
    else if (v1 > v2)
    {
        return(1);
    }

    return(0);
}

static void
dbg$hex_to_bin(const char *reqaddr, char *bin_r)
{
    unsigned int            i;
    unsigned int            j;

    j = L4_SIZEOFWORD - 1;

    for(i = 0; i < (L4_SIZEOFWORD / 4); i++)
    {
        switch(reqaddr[((L4_SIZEOFWORD / 4) - 1) - i])
        {
            case '0':
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                break;

            case '1':
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                break;

            case '2':
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                break;

            case '3':
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                break;

            case '4':
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                break;

            case '5':
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                break;

            case '6':
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                break;

            case '7':
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                break;

            case '8':
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                break;

            case '9':
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                break;

            case 'a':
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                break;

            case 'b':
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                break;

            case 'c':
                bin_r[j--] = '0';
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                break;

            case 'd':
                bin_r[j--] = '1';
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                break;

            case 'e':
                bin_r[j--] = '0';
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                break;

            case 'f':
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                bin_r[j--] = '1';
                break;
        }
    }

    bin_r[L4_SIZEOFWORD] = 0;
    return;
}

const char *
dbg$symbol(vms$pointer address)
{
    const char              **function;

    char                    bin_1[L4_SIZEOFWORD + 1];
    char                    bin_2[L4_SIZEOFWORD + 1];
    char                    bin_r[L4_SIZEOFWORD + 1];
    char                    c;
    char                    reqaddr[(L4_SIZEOFWORD / 4) + 1];
    char                    value[L4_SIZEOFWORD / 4][4 + 1];

    unsigned int            i;
    unsigned int            j;

    for(i = 0; i < (L4_SIZEOFWORD / 4); i++)
    {
        for(j = 0; j < 4; j++)
        {
            if (address & (((vms$pointer) 1) << ((i * 4) + j)))
            {
                value[i][3 - j] = '1';
            }
            else
            {
                value[i][3 - j] = '0';
            }
        }

        value[i][4] = 0;
    }

    c = '-';

    for(i = 0; i < (L4_SIZEOFWORD / 4); i++)
    {
        switch(value[i][0])
        {
            case '0':
                switch(value[i][1])
                {
                    case '0':
                        switch(value[i][2])
                        {
                            case '0': // 000x
                                c = (value[i][3] == '0') ? '0' : '1';
                                break;

                            case '1': // 001x
                                c = (value[i][3] == '0') ? '2' : '3';
                                break;
                        }
                        break;

                    case '1':
                        switch(value[i][2])
                        {
                            case '0': // 010x
                                c = (value[i][3] == '0') ? '4' : '5';
                                break;

                            case '1': // 011x
                                c = (value[i][3] == '0') ? '6' : '7';
                                break;
                        }
                        break;
                }
                break;

            case '1':
                switch(value[i][1])
                {
                    case '0':
                        switch(value[i][2])
                        {
                            case '0': // 100x
                                c = (value[i][3] == '0') ? '8' : '9';
                                break;

                            case '1': // 101x
                                c = (value[i][3] == '0') ? 'a' : 'b';
                                break;
                        }
                        break;

                    case '1':
                        switch(value[i][2])
                        {
                            case '0': // 110x
                                c = (value[i][3] == '0') ? 'c' : 'd';
                                break;

                            case '1': // 111x
                                c = (value[i][3] == '0') ? 'e' : 'f';
                                break;
                        }
                        break;
                }
                break;
        }

        reqaddr[(L4_SIZEOFWORD / 4) - (i + 1)] = c;
    }

    reqaddr[L4_SIZEOFWORD / 4] = 0;
    function = symbols;

    dbg$hex_to_bin(reqaddr, bin_r);

    while((*function) != 0)
    {
        dbg$hex_to_bin((*function), bin_1);
        dbg$hex_to_bin((*(function + 2)), bin_2);

        if ((dbg$addrcmp(bin_1, bin_r) <= 0) &&
                (dbg$addrcmp(bin_r, bin_2) < 0))
        {
            return((*(++function)));
        }

        function += 2;
    }

    return((const char *) 0);
}

