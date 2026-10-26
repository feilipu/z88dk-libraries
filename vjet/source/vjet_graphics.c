/*
 * vjet_graphics.c
 *
 * Copyright (c) 2026 Phillip Stevens. MIT. See License.txt.
 */

#include "vjet_bdos.h"

void vjet_graphics(void)
{
    vjet_bdos(VJET_BDOS_DIRECT, 0x07);
}
