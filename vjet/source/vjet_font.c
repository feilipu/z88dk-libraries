/*
 * vjet_font.c
 *
 * Copyright (c) 2026 Phillip Stevens. MIT. See License.txt.
 */

#include "vjet.h"

unsigned int vjet_font_hub = 0;

void vjet_font(unsigned int hub)
{
    vjet_font_hub = hub;
}
