/*
 * vjet_box.c
 *
 * One white box, then $07. q sends $07 and returns.
 * The list addresses are the clean-point image: 17752 and 19752.
 *
 * Copyright (c) 2026 Phillip Stevens. MIT. See License.txt.
 */

#include "vjet.h"

#define DLIST1 17752

static unsigned char blob[] = {
    16, 0, 0, 0, 0, 0,
    0x00, 0x00, 0xFF, 0x02,
    40, 0, 200, 0,
    40, 0, 200, 0,
    0xFF, 0xFF,
    0x00, 0x00
};

int main(void)
{
    int key;

    vjet_font(17236);
    if (vjet_send_list(blob, DLIST1) != VJET_OK)
        return 1;
    vjet_graphics();
    do {
        key = vjet_key();
    } while (key != 'q');
    vjet_graphics();
    return 0;
}
