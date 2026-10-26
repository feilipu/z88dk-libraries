/*
 * vjet_key.c
 *
 * Function 11 sees a key stored by function 2.
 * Function 1 takes that byte. Function 6 does not.
 *
 * Copyright (c) 2026 Phillip Stevens. MIT. See License.txt.
 */

#include "vjet_bdos.h"

int vjet_key(void)
{
    if ((vjet_bdos(VJET_BDOS_CONST, 0) & 0xFF) == 0)
        return 0;
    return vjet_bdos(VJET_BDOS_CONIN, 0) & 0xFF;
}
