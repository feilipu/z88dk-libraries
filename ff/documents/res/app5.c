/*----------------------------------------------------------------------/
/ Test if the file is contiguous                                        /
/----------------------------------------------------------------------*/

FRESULT test_contiguous_file (
    FIL* fp,    /* [IN]  Open file object to be checked */
    int* cont   /* [OUT] 1:Contiguous, 0:Fragmented or zero-length */
)
{
    DWORD clst, clsz, step;
    FSIZE_t fsz;
    FRESULT fr;


    *cont = 0;
    fr = f_lseek(fp, 0);
    if (fr != FR_OK) return fr;

#if FF_MAX_SS == FF_MIN_SS
    clsz = (DWORD)fp->obj.fs->csize * FF_MAX_SS;
#else
    clsz = (DWORD)fp->obj.fs->csize * fp->obj.fs->ssize;
#endif
    fsz = fp->obj.objsize;
    if (fsz > 0) {
        clst = fp->obj.sclust - 1;
        while (fsz) {
            step = (fsz >= clsz) ? clsz : (DWORD)fsz;
            fr = f_lseek(fp, f_tell(fp) + step);
            if (fr != FR_OK) return fr;
            if (clst + 1 != fp->clust) break;
            clst = fp->clust; fsz -= step;
        }
        if (fsz <= clsz) *cont = 1;
    }

    return FR_OK;
}
