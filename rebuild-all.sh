#!/usr/bin/env bash
# Rebuild package .lib products into the package tree, then install into z88dk.
# Max 2 concurrent zcc jobs.
#
# SDCC uses --max-allocs-per-node400000 (same as package readme Preparation).
# Products are written under:
#   <pkg>/<target>/lib/newlib/{sccz80,sdcc_ix,sdcc_iy}/<name>.lib
#
# ff: RW + RO (Z80 all clibs); rc2014 also ff_85 + ff_85_ro (sccz80).
# RO libs are written next to standard ff libs in the package tree.
#
# Phase 3: z88dk-lib installs each package (basename == package name, e.g. ff.lib).
# Extra products that z88dk-lib does not install (ff_ro, ff_85, ff_85_ro) are
# copied manually into the same install dirs, derived from ZCCCFG:
#   $ZCCCFG/../clibs/{sccz80,sdcc_ix,sdcc_iy}/lib/<target>/
set -euo pipefail

export PATH="/home/phillip/Z80/z88dk/bin:${PATH:-}"
export ZCCCFG="${ZCCCFG:-/home/phillip/Z80/z88dk/lib/config}"
if [[ ! -d "$ZCCCFG" ]]; then
  echo "ZCCCFG is not a directory: $ZCCCFG" >&2
  exit 1
fi
# Absolute install root for third-party newlib libs (mirrors z88dk-lib.c)
Z88DK_CLIBS="$(cd "$ZCCCFG/../clibs" && pwd)"

ROOT="$(cd "$(dirname "$0")" && pwd)"
LOG="$ROOT/rebuild-logs"
SUMMARY="$LOG/summary.txt"
STATUS="$LOG/STATUS"          # one-line live status: cat rebuild-logs/STATUS
MAXJOBS=2
mkdir -p "$LOG"
: >"$SUMMARY"
: >"$STATUS"

running=0
done_ok=0
done_fail=0
started=0
phase="init"

# Concise status for the terminal and for polling (echo/cat STATUS).
say() {
  local line
  line="$(date +%H:%M:%S)  $*"
  echo "$line" | tee -a "$SUMMARY"
  # single-line live snapshot (safe enough for concurrent writers)
  printf '%s\n' "$line" >"$STATUS"
}

phase_begin() {
  phase=$1
  say "PHASE $phase  (ok=$done_ok fail=$done_fail running=$running)"
}

reap() {
  local st=0
  wait -n || st=$?
  running=$((running - 1))
  if (( st != 0 )); then
    done_fail=$((done_fail + 1))
    say "FAIL  exit=$st  phase=$phase  ok=$done_ok fail=$done_fail run=$running  logs=$LOG"
    wait || true
    exit "$st"
  fi
  done_ok=$((done_ok + 1))
  say "OK    phase=$phase  ok=$done_ok fail=$done_fail run=$running"
}

spawn() {
  local name=$1
  shift
  local logf="$LOG/${name//\//_}.log"
  while (( running >= MAXJOBS )); do
    reap
  done
  started=$((started + 1))
  say "START #$started  $name  run=$((running + 1))/$MAXJOBS  phase=$phase"
  (
    set +e
    "$@" >"$logf" 2>&1
    st=$?
    if (( st == 0 )); then
      printf '%s\n' "$(date +%H:%M:%S)  DONE  $name" >>"$SUMMARY"
      printf '%s\n' "$(date +%H:%M:%S)  DONE  $name  (ok pending reap)" >"$STATUS"
    else
      printf '%s\n' "$(date +%H:%M:%S)  FAIL  $name  exit=$st  log=$logf" | tee -a "$SUMMARY"
      printf '%s\n' "$(date +%H:%M:%S)  FAIL  $name  exit=$st  log=$logf" >"$STATUS"
    fi
    exit "$st"
  ) &
  running=$((running + 1))
}

wait_all() {
  while (( running > 0 )); do
    reap
  done
}

clib_flags() {
  case "$1" in
    sccz80) echo "-clib=new -x -O2 --opt-code-speed=all --math32" ;;
    sdcc_ix) echo "-clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 --math32" ;;
    sdcc_iy) echo "-clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 --math32" ;;
    *) echo "bad clib $1" >&2; exit 1 ;;
  esac
}

# build_one package target clib srcdir lst outname [extra...]
# Writes: $ROOT/$pkg/$target/lib/newlib/$clib/${outname}.lib
build_one() {
  local pkg=$1 target=$2 clib=$3 srcdir=$4 lst=$5 outname=$6
  shift 6
  local flags dest tmp
  flags=$(clib_flags "$clib")
  dest="$ROOT/$pkg/$target/lib/newlib/$clib"
  mkdir -p "$dest"
  cd "$ROOT/$srcdir"
  tmp="$ROOT/$pkg/.build_${outname}_${target}_${clib}_$$"
  # shellcheck disable=SC2086
  zcc +"$target" $flags "$@" @"$lst" -o "$tmp"
  if [[ ! -f "${tmp}.lib" ]]; then
    echo "build_one: missing ${tmp}.lib for $pkg $target $clib" >&2
    exit 1
  fi
  mv -f "${tmp}.lib" "$dest/${outname}.lib"
  rm -f "$tmp" "${tmp}."* 2>/dev/null || true
}

say "BEGIN rebuild  root=$ROOT  maxjobs=$MAXJOBS  sdcc_allocs=400000"
say "      logs=$LOG  live=cat $STATUS"

# Phase 1 — RW libs
phase_begin "1/3 build RW"
for clib in sccz80 sdcc_ix sdcc_iy; do
  for t in rc2014 yaz180 scz180 hbios; do
    spawn "ff/$t/$clib" build_one ff "$t" "$clib" ff/source ff.lst ff
    spawn "time/$t/$clib" build_one time "$t" "$clib" time/source time.lst time
  done
  spawn "diskio_sd/scz180/$clib" build_one diskio_sd scz180 "$clib" diskio_sd/source diskio_sd.lst diskio_sd
  spawn "diskio_hbios/hbios/$clib" build_one diskio_hbios hbios "$clib" diskio_hbios/source diskio_hbios.lst diskio_hbios
  for t in yaz180 scz180; do
    spawn "freertos/$t/$clib" build_one freertos "$t" "$clib" freertos/source freertos.lst freertos
  done
  for t in cpm rc2014 yaz180; do
    spawn "3d/$t/$clib" build_one 3d "$t" "$clib" 3d/source 3d.lst 3d
    spawn "regis/$t/$clib" build_one regis "$t" "$clib" regis/source regis.lst regis
  done
  spawn "th02/yaz180/$clib" build_one th02 yaz180 "$clib" th02/source th02.lst th02
  spawn "ft80x/yaz180/$clib" build_one ft80x yaz180 "$clib" ft80x/source ft80x.lst ft80x
  spawn "i2c_lcd/yaz180/$clib" build_one i2c_lcd yaz180 "$clib" i2c_lcd/source i2c_lcd.lst i2c_lcd
done

# ff_85 RW (rc2014 sccz80 only)
spawn "ff/rc2014/sccz80/ff_85" bash -c '
  dest="'"$ROOT"'/ff/rc2014/lib/newlib/sccz80"
  mkdir -p "$dest"
  cd "'"$ROOT"'/ff/source"
  zcc +rc2014 -clib=new -m8085 -x -O2 --opt-code-speed=all -D__DISABLE_BUILTIN --math32 @ff.lst -o "'"$ROOT"'/ff/.build_ff_85_$$"
  mv -f "'"$ROOT"'/ff/.build_ff_85_$$.lib" "$dest/ff_85.lib"
'

wait_all
say "PHASE 1/3 done  ok=$done_ok"

# Phase 2 — RO (flip ffconf, build, restore)
phase_begin "2/3 build RO"
CONF="$ROOT/ff/source/ffconf.h"
cp -a "$CONF" "$CONF.bak_rebuild"
sed -i 's/#define FF_FS_READONLY  0/#define FF_FS_READONLY  1/g' "$CONF"

for clib in sccz80 sdcc_ix sdcc_iy; do
  for t in rc2014 yaz180 scz180 hbios; do
    spawn "ff_ro/$t/$clib" build_one ff "$t" "$clib" ff/source ff.lst ff_ro
  done
done

spawn "ff/rc2014/sccz80/ff_85_ro" bash -c '
  dest="'"$ROOT"'/ff/rc2014/lib/newlib/sccz80"
  mkdir -p "$dest"
  cd "'"$ROOT"'/ff/source"
  zcc +rc2014 -clib=new -m8085 -x -O2 --opt-code-speed=all -D__DISABLE_BUILTIN --math32 @ff.lst -o "'"$ROOT"'/ff/.build_ff_85_ro_$$"
  mv -f "'"$ROOT"'/ff/.build_ff_85_ro_$$.lib" "$dest/ff_85_ro.lib"
'

wait_all
mv -f "$CONF.bak_rebuild" "$CONF"
say "PHASE 2/3 done  ok=$done_ok  (ffconf restored)"

# ---------------------------------------------------------------------------
# Phase 3 — install into z88dk tree
# ---------------------------------------------------------------------------
# z88dk-lib must be run from the package repo root (looks for ./<pkg>/<target>/…).
# It only installs <pkg>.lib + <pkg>.h for the basename given. Remove first so
# overwrite is non-interactive.

install_pkg() {
  local target=$1
  shift
  local pkg
  for pkg in "$@"; do
    say "INSTALL +$target $pkg"
    # -r -f: remove without prompts if present
    z88dk-lib +"$target" -r -f "$pkg" >>"$LOG/install.log" 2>&1 || true
    if ! z88dk-lib +"$target" "$pkg" >>"$LOG/install.log" 2>&1; then
      say "FAIL  install +$target $pkg  (see $LOG/install.log)"
      exit 1
    fi
    say "OK    install +$target $pkg"
  done
}

: >"$LOG/install.log"
cd "$ROOT"

phase_begin "3/3 install"
say "      ZCCCFG=$ZCCCFG  clibs=$Z88DK_CLIBS"

install_pkg rc2014  ff time 3d regis
install_pkg yaz180  ff time freertos 3d regis th02 ft80x i2c_lcd
install_pkg scz180  ff time diskio_sd freertos
install_pkg hbios   ff time diskio_hbios
install_pkg cpm     3d regis

# Manual copy: extras that live beside ff.lib but are not z88dk-lib basenames
say "COPY  ff_ro / ff_85* extras → clibs"
for t in rc2014 yaz180 scz180 hbios; do
  for clib in sccz80 sdcc_ix sdcc_iy; do
    src="$ROOT/ff/$t/lib/newlib/$clib"
    dst="$Z88DK_CLIBS/$clib/lib/$t"
    mkdir -p "$dst"
    for f in ff_ro.lib ff_85.lib ff_85_ro.lib; do
      if [[ -f "$src/$f" ]]; then
        cp -f "$src/$f" "$dst/$f"
        say "COPY  $t/$clib/$f"
      fi
    done
  done
done

say "PHASE 3/3 done"
say "ALL OK  started=$started ok=$done_ok fail=$done_fail  $(date -Iseconds)"
