/*
 * vjet_load.c
 *
 * Read one CP/M file with the FCB. Classic and newlib share this path.
 * A full buffer reads one more record. More data returns -1.
 * End of file returns the count already stored.
 *
 * Copyright (c) 2026 Phillip Stevens. MIT. See License.txt.
 */

#include "vjet.h"
#include "vjet_bdos.h"

static unsigned char vjet_fcb[36];
static unsigned char vjet_dma[128];

static int up(int c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 'A';
    return c;
}

static int set_fcb(char *name)
{
    unsigned int i;
    unsigned int n;
    char *p;
    char *dot;
    int drive;
    int c;

    for (i = 0; i < 36; i++)
        vjet_fcb[i] = 0;
    for (i = 1; i < 12; i++)
        vjet_fcb[i] = ' ';

    p = name;
    if (p[0] != 0 && p[1] == ':') {
        drive = up((unsigned char)p[0]);
        if (drive < 'A' || drive > 'P')
            return -1;
        vjet_fcb[0] = (unsigned char)(drive - 'A' + 1);
        p += 2;
    }

    dot = 0;
    for (i = 0; p[i] != 0; i++) {
        if (p[i] == '.') {
            if (dot != 0)
                return -1;
            dot = p + i;
        }
    }

    n = 0;
    while (p[n] != 0 && p[n] != '.') {
        if (n >= 8)
            return -1;
        c = up((unsigned char)p[n]);
        vjet_fcb[1 + n] = (unsigned char)c;
        n++;
    }
    if (n == 0)
        return -1;

    if (dot != 0) {
        n = 0;
        dot++;
        while (dot[n] != 0) {
            if (n >= 3)
                return -1;
            c = up((unsigned char)dot[n]);
            vjet_fcb[9 + n] = (unsigned char)c;
            n++;
        }
    }
    return 0;
}

int vjet_load(char *name, unsigned char *buf, unsigned int max)
{
    unsigned int n;
    unsigned int i;
    int rc;

    if (name == 0 || buf == 0 || max < 128)
        return -1;
    if (set_fcb(name) != 0)
        return -1;

    rc = vjet_bdos(VJET_BDOS_OPEN, (unsigned int)vjet_fcb);
    if ((rc & 0xFF) == 0xFF)
        return -1;
    vjet_fcb[32] = 0;

    n = 0;
    while (n < max && (unsigned int)(max - n) >= 128) {
        vjet_bdos(VJET_BDOS_DMA, (unsigned int)vjet_dma);
        rc = vjet_bdos(VJET_BDOS_READ, (unsigned int)vjet_fcb);
        if ((rc & 0xFF) == 0xFF)
            return -1;
        if ((rc & 0xFF) != 0)
            break;
        for (i = 0; i < 128; i++)
            buf[n + i] = vjet_dma[i];
        n += 128;
    }
    if ((unsigned int)(max - n) < 128) {
        vjet_bdos(VJET_BDOS_DMA, (unsigned int)vjet_dma);
        rc = vjet_bdos(VJET_BDOS_READ, (unsigned int)vjet_fcb);
        if ((rc & 0xFF) == 0 || (rc & 0xFF) == 0xFF)
            return -1;
    }
    return (int)n;
}
