/*
 * vjet.h
 *
 * Send a VECTORJET list blob over the CP/M console.
 * Copyright (c) 2026 Phillip Stevens. MIT. See License.txt.
 */

#ifndef _VJET_H
#define _VJET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VJET_LIST_MAX  2000

#define VJET_OK     0
#define VJET_ERR    1
#define VJET_RELOC  2
#define VJET_FONT   3

/* Expected hexfont Hub address. 0 skips the check.
   A blob font of 0 also skips the check. */
extern void vjet_font(unsigned int hub);

/* Fix reloc words, then write $1C, the length, and the payload.
   size is the blob length in bytes.
   A short blob returns before the first byte.
   A bad link returns before the first byte.
   dest is the Hub address of the fill list. */
extern int vjet_send_list(unsigned char *blob, unsigned int dest, unsigned int size);

/* Read one CP/M file into buf. Returns the byte count, or -1.
   The count is a multiple of 128 and fits in a signed int.
   max above 32767 returns -1.
   A file that does not fit in max returns -1.
   A read error returns -1.
   After an open, the DMA address is 0080h. */
extern int vjet_load(char *name, unsigned char *buf, unsigned int max);

/* Write one $07. */
extern void vjet_graphics(void);

/* 0 when no key is stored. Otherwise the key. */
extern int vjet_key(void);

#ifdef __cplusplus
}
#endif

#endif
