FatFs Module Source Files R0.16 w/patch 2


FILES

  00readme.txt   This file.
  00history.txt  Revision history.
  ff.c           FatFs module (R0.16 + patch 1 + patch 2; z88dk LE macro-over-name).
  ffconf.h       Configuration file of FatFs module (package defaults).
  ff.h           Common include file for FatFs and application module.
  ffunicode.c    Optional Unicode utility functions.
  ffsystem.c     An example of optional O/S related functions.


  Low level disk I/O module is not included here because the FatFs
  module is only a generic file system layer and it does not depend on any specific
  storage device. In this package, disk I/O is supplied per target
  (in-tree z88dk drivers, or diskio_sd / diskio_hbios packages).

