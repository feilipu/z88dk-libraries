/*-------------------------------------------------------------------------/
/  Unicode handling functions for FatFs R0.13+                             /
/--------------------------------------------------------------------------/
/
/ Copyright (C) 2017, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/-------------------------------------------------------------------------*/

#include "ff.h"         /* FatFs Public API */
#include "ffunicode.h"  /* FatFS Unicode */

/*------------------------------------------------------------------------*/
/* Code Conversion Tables                                                 */
/*------------------------------------------------------------------------*/

#if FF_USE_LFN

#if FF_CODE_PAGE == 0        /* Run-time code page configuration */
#define CODEPAGE CodePage
extern WORD CodePage;    /* Current code page */
extern const BYTE *ExCvt, *DbcTbl;    /* Pointer to current SBCS up-case table and DBCS code range table below */
extern const BYTE Ct437[];
extern const BYTE Ct720[];
extern const BYTE Ct737[];
extern const BYTE Ct771[];
extern const BYTE Ct775[];
extern const BYTE Ct850[];
extern const BYTE Ct852[];
extern const BYTE Ct855[];
extern const BYTE Ct857[];
extern const BYTE Ct860[];
extern const BYTE Ct861[];
extern const BYTE Ct862[];
extern const BYTE Ct863[];
extern const BYTE Ct864[];
extern const BYTE Ct865[];
extern const BYTE Ct866[];
extern const BYTE Ct869[];
extern const BYTE Dc932[];
extern const BYTE Dc936[];
extern const BYTE Dc949[];
extern const BYTE Dc950[];

extern const WORD cp_code[];
extern const WCHAR *const cp_table[];

#elif FF_CODE_PAGE < 900    /* static code page configuration (SBCS) */
#define CODEPAGE FF_CODE_PAGE
extern const BYTE ExCvt[];

#else                       /* static code page configuration (DBCS) */
#define CODEPAGE FF_CODE_PAGE
extern const BYTE DbcTbl[];

#endif


#if FF_CODE_PAGE == 932 || FF_CODE_PAGE == 0    /* Japanese */
extern const WCHAR uni2oem932[]         /* Unicode --> Shift_JIS pairs */
extern const WCHAR oem2uni932[];        /* Shift_JIS --> Unicode pairs */
#endif

#if FF_CODE_PAGE == 936 || FF_CODE_PAGE == 0    /* Simplified Chinese */
extern const WCHAR uni2oem936[];        /* Unicode --> GBK pairs */
extern const WCHAR oem2uni936[];        /* GBK --> Unicode pairs */

#endif

#if FF_CODE_PAGE == 949 || FF_CODE_PAGE == 0    /* Korean */
extern const WCHAR uni2oem949[];        /* Unicode --> Korean pairs */
extern const WCHAR oem2uni949[];        /* Korean --> Unicode pairs */
#endif

#if FF_CODE_PAGE == 950 || FF_CODE_PAGE == 0    /* Traditional Chinese */
extern const WCHAR uni2oem950[];       /* Unicode --> Big5 pairs */
extern const WCHAR oem2uni950[];       /* Big5 --> Unicode pairs */
#endif

#if FF_CODE_PAGE == 437 || FF_CODE_PAGE == 0
extern const WCHAR uc437[];            /*  CP437(U.S.) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 720 || FF_CODE_PAGE == 0
extern const WCHAR uc720[];            /*  CP720(Arabic) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 737 || FF_CODE_PAGE == 0
extern const WCHAR uc737[];            /*  CP737(Greek) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 771 || FF_CODE_PAGE == 0
extern const WCHAR uc771[];            /*  CP771(KBL) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 775 || FF_CODE_PAGE == 0
extern const WCHAR uc775[];            /*  CP775(Baltic) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 850 || FF_CODE_PAGE == 0
extern const WCHAR uc850[];            /*  CP850(Latin 1) to Unicode conversion table */

#endif
#if FF_CODE_PAGE == 852 || FF_CODE_PAGE == 0
extern const WCHAR uc852[];            /*  CP852(Latin 2) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 855 || FF_CODE_PAGE == 0
extern const WCHAR uc855[];            /*  CP855(Cyrillic) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 857 || FF_CODE_PAGE == 0
extern const WCHAR uc857[];            /*  CP857(Turkish) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 860 || FF_CODE_PAGE == 0
extern const WCHAR uc860[];            /*  CP860(Portuguese) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 861 || FF_CODE_PAGE == 0
extern const WCHAR uc861[];            /*  CP861(Icelandic) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 862 || FF_CODE_PAGE == 0
extern const WCHAR uc862[];            /*  CP862(Hebrew) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 863 || FF_CODE_PAGE == 0
extern const WCHAR uc863[];            /*  CP863(Canadian French) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 864 || FF_CODE_PAGE == 0
extern const WCHAR uc864[];            /*  CP864(Arabic) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 865 || FF_CODE_PAGE == 0
extern const WCHAR uc865[];            /*  CP865(Nordic) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 866 || FF_CODE_PAGE == 0
extern const WCHAR uc866[];            /*  CP866(Russian) to Unicode conversion table */
#endif

#if FF_CODE_PAGE == 869 || FF_CODE_PAGE == 0
extern const WCHAR uc869[];            /*  CP869(Greek 2) to Unicode conversion table */
#endif

#define MERGE2(a, b) a ## b
#define CVTBL(tbl, cp) MERGE2(tbl, cp)

/*------------------------------------------------------------------------*/
/* OEM <==> Unicode conversions for static code page configuration        */
/* SBCS fixed code page                                                   */
/*------------------------------------------------------------------------*/

#if FF_CODE_PAGE != 0 && FF_CODE_PAGE < 900
WCHAR ff_uni2oem (      /* Returns OEM code character, zero on error */
    WCHAR    uni,       /* Unicode character to be converted */
    WORD    cp          /* Code page for the conversion */
)
{
    WCHAR c = 0;
    const WCHAR *p = CVTBL(uc, FF_CODE_PAGE);


    if (uni < 0x80) {    /* ASCII char */
        c = uni;

    } else {            /* Non-ASCII char */
        if (cp == FF_CODE_PAGE) {    /* Is it a valid code page? */
            for (c = 0; c < 0x80 && uni != p[c]; c++) ;
            c = (c + 0x80) & 0xFF;
        }
    }
    return c;
}

WCHAR ff_oem2uni (      /* Returns Unicode character, zero on error */
    WCHAR    oem,       /* OEM code to be converted */
    WORD    cp          /* Code page for the conversion */
)
{
    WCHAR c = 0;
    const WCHAR *p = CVTBL(uc, FF_CODE_PAGE);


    if (oem < 0x80) {    /* ASCII char */
        c = oem;

    } else {            /* Extended char */
        if (cp == FF_CODE_PAGE) {    /* Is it a valid code page? */
            if (oem < 0x100) c = p[oem - 0x80];
        }
    }
    return c;
}

#endif



/*------------------------------------------------------------------------*/
/* OEM <==> Unicode conversions for static code page configuration        */
/* DBCS fixed code page                                                   */
/*------------------------------------------------------------------------*/

#if FF_CODE_PAGE != 0 && FF_CODE_PAGE >= 900
WCHAR ff_uni2oem (    /* Returns OEM code character, zero on error */
    WCHAR    uni,    /* Unicode character to be converted */
    WORD    cp        /* Code page for the conversion */
)
{
    const WCHAR *p;
    WCHAR c = 0;
    UINT i, n, li, hi;


    if (uni < 0x80) {    /* ASCII char */
        c = uni;

    } else {            /* Non-ASCII char */
        if (cp == FF_CODE_PAGE) {    /* Is it a valid code page? */
            p = CVTBL(uni2oem, FF_CODE_PAGE);
            hi = sizeof CVTBL(uni2oem, FF_CODE_PAGE) / 4 - 1;
            li = 0;
            for (n = 16; n; n--) {
                i = li + (hi - li) / 2;
                if (uni == p[i * 2]) break;
                if (uni > p[i * 2]) {
                    li = i;
                } else {
                    hi = i;
                }
            }
            if (n != 0) c = p[i * 2 + 1];
        }
    }
    return c;
}


WCHAR ff_oem2uni (    /* Returns Unicode character, zero on error */
    WCHAR    oem,    /* OEM code to be converted */
    WORD    cp        /* Code page for the conversion */
)
{
    const WCHAR *p;
    WCHAR c = 0;
    UINT i, n, li, hi;


    if (oem < 0x80) {    /* ASCII char */
        c = oem;

    } else {            /* Extended char */
        if (cp == FF_CODE_PAGE) {    /* Is it a valid code page? */
            p = CVTBL(oem2uni, FF_CODE_PAGE);
            hi = sizeof CVTBL(oem2uni, FF_CODE_PAGE) / 4 - 1;
            li = 0;
            for (n = 16; n; n--) {
                i = li + (hi - li) / 2;
                if (oem == p[i * 2]) break;
                if (oem > p[i * 2]) {
                    li = i;
                } else {
                    hi = i;
                }
            }
            if (n != 0) c = p[i * 2 + 1];
        }
    }
    return c;
}
#endif



/*------------------------------------------------------------------------*/
/* OEM <==> Unicode conversions for dynamic code page configuration       */
/*------------------------------------------------------------------------*/

#if FF_CODE_PAGE == 0

WCHAR ff_uni2oem (      /* Returns OEM code character, zero on error */
    WCHAR    uni,       /* Unicode character to be converted */
    WORD    cp          /* Code page for the conversion */
)
{
    const WCHAR *p;
    WCHAR c = 0;
    UINT i, n, li, hi;


    if (uni < 0x80) {    /* ASCII char */
        c = uni;

    } else {            /* Non-ASCII char */
        p = 0;
        if (cp < 900) {    /* SBCS */
            for (i = 0; cp_code[i] != 0 && cp_code[i] != cp; i++) ;        /* Get table */
            p = cp_table[i];
            if (p) {    /* Is it a valid CP ? */
                for (c = 0; c < 0x80 && uni != p[c]; c++) ;    /* Find OEM code in the table */
                c = (c + 0x80) & 0xFF;
            }
        } else {    /* DBCS */
            switch (cp) {
            case 932 : p = uni2oem932; hi = sizeof uni2oem932 / 4 - 1; break;
            case 936 : p = uni2oem936; hi = sizeof uni2oem936 / 4 - 1; break;
            case 949 : p = uni2oem949; hi = sizeof uni2oem949 / 4 - 1; break;
            case 950 : p = uni2oem950; hi = sizeof uni2oem950 / 4 - 1; break;
            }
            if (p) {    /* Is it a valid code page? */
                li = 0;
                for (n = 16; n; n--) {    /* Find OEM code */
                    i = li + (hi - li) / 2;
                    if (uni == p[i * 2]) break;
                    if (uni > p[i * 2]) {
                        li = i;
                    } else {
                        hi = i;
                    }
                }
                if (n != 0) c = p[i * 2 + 1];
            }
        }
    }
    return c;
}


WCHAR ff_oem2uni (    /* Returns Unicode character, zero on error */
    WCHAR    oem,    /* OEM code to be converted */
    WORD    cp        /* Code page for the conversion */
)
{
    const WCHAR *p;
    WCHAR c = 0;
    UINT i, n, li, hi;


    if (oem < 0x80) {    /* ASCII char */
        c = oem;

    } else {            /* Extended char */
        p = 0;
        if (cp < 900) {    /* SBCS */
            for (i = 0; cp_code[i] != 0 && cp_code[i] != cp; i++) ;        /* Get table */
            p = cp_table[i];
            if (p) {    /* Is it a valid CP ? */
                if (oem < 0x100) c = p[oem - 0x80];
            }
        } else {    /* DBCS */
            switch (cp) {
            case 932 : p = oem2uni932; hi = sizeof oem2uni932 / 4 - 1; break;
            case 936 : p = oem2uni936; hi = sizeof oem2uni936 / 4 - 1; break;
            case 949 : p = oem2uni949; hi = sizeof oem2uni949 / 4 - 1; break;
            case 950 : p = oem2uni950; hi = sizeof oem2uni950 / 4 - 1; break;
            }
            if (p) {
                li = 0;
                for (n = 16; n; n--) {
                    i = li + (hi - li) / 2;
                    if (oem == p[i * 2]) break;
                    if (oem > p[i * 2]) {
                        li = i;
                    } else {
                        hi = i;
                    }
                }
                if (n != 0) c = p[i * 2 + 1];
            }
        }
    }
    return c;
}
#endif



/*------------------------------------------------------------------------*/
/* Unicode up-case conversion                                             */
/*------------------------------------------------------------------------*/

WCHAR ff_wtoupper (    /* Returns up-converted character */
    WCHAR uni        /* Unicode character to be upper converted (BMP only) */
)
{
    /* Compressed upper conversion table */
    static const WCHAR cvt1[] = {    /* U+0000 - U+0FFF */
        /* Basic Latin */
        0x0061,0x031A,
        /* Latin-1 Supplement */
        0x00E0,0x0317,  0x00F8,0x0307,  0x00FF,0x0001,0x0178,
        /* Latin Extended-A */
        0x0100,0x0130,  0x0132,0x0106,  0x0139,0x0110,  0x014A,0x012E,  0x0179,0x0106,
        /* Latin Extended-B */
        0x0180,0x004D,0x0243,0x0181,0x0182,0x0182,0x0184,0x0184,0x0186,0x0187,0x0187,0x0189,0x018A,0x018B,0x018B,0x018D,0x018E,0x018F,0x0190,0x0191,0x0191,0x0193,0x0194,0x01F6,0x0196,0x0197,0x0198,0x0198,0x023D,0x019B,0x019C,0x019D,0x0220,0x019F,0x01A0,0x01A0,0x01A2,0x01A2,0x01A4,0x01A4,0x01A6,0x01A7,0x01A7,0x01A9,0x01AA,0x01AB,0x01AC,0x01AC,0x01AE,0x01AF,0x01AF,0x01B1,0x01B2,0x01B3,0x01B3,0x01B5,0x01B5,0x01B7,0x01B8,0x01B8,0x01BA,0x01BB,0x01BC,0x01BC,0x01BE,0x01F7,0x01C0,0x01C1,0x01C2,0x01C3,0x01C4,0x01C5,0x01C4,0x01C7,0x01C8,0x01C7,0x01CA,0x01CB,0x01CA,
        0x01CD,0x0110,  0x01DD,0x0001,0x018E,  0x01DE,0x0112,  0x01F3,0x0003,0x01F1,0x01F4,0x01F4,  0x01F8,0x0128,
        0x0222,0x0112,  0x023A,0x0009,0x2C65,0x023B,0x023B,0x023D,0x2C66,0x023F,0x0240,0x0241,0x0241,  0x0246,0x010A,
        /* IPA Extensions */
        0x0253,0x0040,0x0181,0x0186,0x0255,0x0189,0x018A,0x0258,0x018F,0x025A,0x0190,0x025C,0x025D,0x025E,0x025F,0x0193,0x0261,0x0262,0x0194,0x0264,0x0265,0x0266,0x0267,0x0197,0x0196,0x026A,0x2C62,0x026C,0x026D,0x026E,0x019C,0x0270,0x0271,0x019D,0x0273,0x0274,0x019F,0x0276,0x0277,0x0278,0x0279,0x027A,0x027B,0x027C,0x2C64,0x027E,0x027F,0x01A6,0x0281,0x0282,0x01A9,0x0284,0x0285,0x0286,0x0287,0x01AE,0x0244,0x01B1,0x01B2,0x0245,0x028D,0x028E,0x028F,0x0290,0x0291,0x01B7,
        /* Greek, Coptic */
        0x037B,0x0003,0x03FD,0x03FE,0x03FF,  0x03AC,0x0004,0x0386,0x0388,0x0389,0x038A,  0x03B1,0x0311,
        0x03C2,0x0002,0x03A3,0x03A3,  0x03C4,0x0308,  0x03CC,0x0003,0x038C,0x038E,0x038F,  0x03D8,0x0118,
        0x03F2,0x000A,0x03F9,0x03F3,0x03F4,0x03F5,0x03F6,0x03F7,0x03F7,0x03F9,0x03FA,0x03FA,
        /* Cyrillic */
        0x0430,0x0320,  0x0450,0x0710,  0x0460,0x0122,  0x048A,0x0136,  0x04C1,0x010E,  0x04CF,0x0001,0x04C0,  0x04D0,0x0144,
        /* Armenian */
        0x0561,0x0426,

        0x0000
    };
    static const WCHAR cvt2[] = {    /* U+1000 - U+FFFF */
        /* Phonetic Extensions */
        0x1D7D,0x0001,0x2C63,
        /* Latin Extended Additional */
        0x1E00,0x0196,  0x1EA0,0x015A,
        /* Greek Extended */
        0x1F00,0x0608,  0x1F10,0x0606,  0x1F20,0x0608,  0x1F30,0x0608,  0x1F40,0x0606,
        0x1F51,0x0007,0x1F59,0x1F52,0x1F5B,0x1F54,0x1F5D,0x1F56,0x1F5F,  0x1F60,0x0608,
        0x1F70,0x000E,0x1FBA,0x1FBB,0x1FC8,0x1FC9,0x1FCA,0x1FCB,0x1FDA,0x1FDB,0x1FF8,0x1FF9,0x1FEA,0x1FEB,0x1FFA,0x1FFB,
        0x1F80,0x0608,  0x1F90,0x0608,  0x1FA0,0x0608,  0x1FB0,0x0004,0x1FB8,0x1FB9,0x1FB2,0x1FBC,
        0x1FCC,0x0001,0x1FC3,  0x1FD0,0x0602,  0x1FE0,0x0602,  0x1FE5,0x0001,0x1FEC,  0x1FF3,0x0001,0x1FFC,
        /* Letterlike Symbols */
        0x214E,0x0001,0x2132,
        /* Number forms */
        0x2170,0x0210,  0x2184,0x0001,0x2183,
        /* Enclosed Alphanumerics */
        0x24D0,0x051A,  0x2C30,0x042F,
        /* Latin Extended-C */
        0x2C60,0x0102,  0x2C67,0x0106, 0x2C75,0x0102,
        /* Coptic */
        0x2C80,0x0164,
        /* Georgian Supplement */
        0x2D00,0x0826,
        /* Full-width */
        0xFF41,0x031A,

        0x0000
    };
    const WCHAR *p;
    WCHAR bc, nc, cmd;


    p = uni < 0x1000 ? cvt1 : cvt2;
    for (;;) {
        bc = *p++;                                /* Get block base */
        if (!bc || uni < bc) break;
        nc = *p++; cmd = nc >> 8; nc &= 0xFF;    /* Get processing command and block size */
        if (uni < bc + nc) {    /* In the block? */
            switch (cmd) {
            case 0:    uni = p[uni - bc]; break;        /* Table conversion */
            case 1:    uni -= (uni - bc) & 1; break;    /* Case pairs */
            case 2: uni -= 16; break;                /* Shift -16 */
            case 3:    uni -= 32; break;                /* Shift -32 */
            case 4:    uni -= 48; break;                /* Shift -48 */
            case 5:    uni -= 26; break;                /* Shift -26 */
            case 6:    uni += 8; break;                /* Shift +8 */
            case 7: uni -= 80; break;                /* Shift -80 */
            case 8:    uni -= 0x1C60; break;            /* Shift -0x1C60 */
            }
            break;
        }
        if (!cmd) p += nc;
    }

    return uni;
}

#endif /* #if _USE_LFN */
