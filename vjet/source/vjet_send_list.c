/*
 * vjet_send_list.c
 *
 * Copy the payload, add dest to each reloc word, then write the frame.
 * A bad link returns before the first byte. The caller blob stays as it is.
 * $FF uses BDOS function 2. Every other byte uses function 6.
 *
 * Copyright (c) 2026 Phillip Stevens. MIT. See License.txt.
 */

#include "vjet.h"
#include "vjet_bdos.h"

extern unsigned int vjet_font_hub;

static unsigned char vjet_work[VJET_LIST_MAX];

static unsigned int rd16(unsigned char *p)
{
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static void wr16(unsigned char *p, unsigned int w)
{
    p[0] = (unsigned char)(w & 255);
    p[1] = (unsigned char)(w >> 8);
}

static void outb(unsigned int c)
{
    c &= 255;
    if (c == 0xFF)
        vjet_bdos(VJET_BDOS_CONOUT, 0xFF);
    else
        vjet_bdos(VJET_BDOS_DIRECT, c);
}

int vjet_send_list(unsigned char *blob, unsigned int dest)
{
    unsigned int len;
    unsigned int nrel;
    unsigned int font;
    unsigned int i;
    unsigned int off;
    unsigned int word;
    unsigned int prev;
    int have;

    if (blob == 0)
        return VJET_ERR;
    len = rd16(blob);
    nrel = rd16(blob + 2);
    font = rd16(blob + 4);
    if (len == 0 || len > VJET_LIST_MAX)
        return VJET_ERR;
    if (nrel > (len >> 1))
        return VJET_RELOC;
    if (nrel != 0 && (dest & 1) != 0)
        return VJET_RELOC;
    if (font != 0 && vjet_font_hub != 0 && font != vjet_font_hub)
        return VJET_FONT;

    for (i = 0; i < len; i++)
        vjet_work[i] = blob[6 + i];

    have = 0;
    prev = 0;
    for (i = 0; i < nrel; i++) {
        off = rd16(blob + 6 + len + (i << 1));
        if (off >= len || (unsigned int)(off + 1) >= len || (off & 1))
            return VJET_RELOC;
        if (have && off <= prev)
            return VJET_RELOC;
        word = rd16(vjet_work + off);
        if ((word & 1) || word < (unsigned int)(off + 8) || word >= len)
            return VJET_RELOC;
        if ((unsigned int)(word + 7) >= len)
            return VJET_RELOC;
        if ((unsigned int)(word + dest) < dest)
            return VJET_RELOC;
        wr16(vjet_work + off, (unsigned int)(word + dest));
        prev = off;
        have = 1;
    }

    outb(0x1C);
    outb(len & 255);
    outb(len >> 8);
    for (i = 0; i < len; i++)
        outb(vjet_work[i]);
    return VJET_OK;
}
