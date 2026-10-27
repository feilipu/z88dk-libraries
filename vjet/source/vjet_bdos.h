/*
 * vjet_bdos.h
 *
 * One BDOS call for classic and for newlib.
 * Copyright (c) 2026 Phillip Stevens. MIT. See License.txt.
 */

#ifndef VJET_BDOS_H
#define VJET_BDOS_H

#ifdef __Z88DK_NEWLIB
#include <arch/cpm.h>
#define vjet_bdos(fn, arg) ((int)cpm_bdos_hl((unsigned int)(fn), (unsigned int)(arg)))
#else
#include <cpm.h>
#define vjet_bdos(fn, arg) bdos((int)(fn), (int)(arg))
#endif

#define VJET_BDOS_CONIN  1
#define VJET_BDOS_CONOUT 2
#define VJET_BDOS_DIRECT 6
#define VJET_BDOS_CONST  11
#define VJET_BDOS_OPEN   15
#define VJET_BDOS_CLOSE  16
#define VJET_BDOS_READ   20
#define VJET_BDOS_DMA    26

#endif
