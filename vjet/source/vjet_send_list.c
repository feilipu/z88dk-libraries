/*
 * vjet_send_list.c
 *
 * Copy the payload, add dest to each reloc word, then write the frame.
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

    if (blob == 0)
        return VJET_ERR;
    len = rd16(blob);
    nrel = rd16(blob + 2);
    font = rd16(blob + 4);
    if (len == 0 || len > VJET_LIST_MAX)
        return VJET_ERR;
    if (nrel > (len >> 1))
        return VJET_RELOC;
    if (font != 0 && vjet_font_hub != 0 && font != vjet_font_hub)
        return VJET_FONT;

    for (i = 0; i < len; i++)
        vjet_work[i] = blob[6 + i];

    for (i = 0; i < nrel; i++) {
        off = rd16(blob + 6 + len + (i << 1));
        if (off >= len || (unsigned int)(off + 1) >= len)
            return VJET_RELOC;
        word = rd16(vjet_work + off);
        wr16(vjet_work + off, (unsigned int)(word + dest));
    }

    outb(0x1C);
    outb(len & 255);
    outb(len >> 8);
    for (i = 0; i < len; i++)
        outb(vjet_work[i]);
    return VJET_OK;
}
