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
# Phase 0: z88dk-lib install of time/diskio so compile-time headers exist
#   (ffsystem.c / ff.c need <lib/.../time.h> and diskio headers).
# Phase 3: z88dk-lib installs each package (basename == package name, e.g. ff.lib).
# Extra products that z88dk-lib does not install (ff_ro, ff_85, ff_85_ro) are
# copied manually into the same install dirs, derived from ZCCCFG:
#   $ZCCCFG/../clibs/{sccz80,sdcc_ix,sdcc_iy}/lib/<target>/
#
# Resume after crash / kill:
#   Default is to resume: skip jobs already recorded in rebuild-logs/DONE or
#   whose package-tree .lib product already exists (non-empty).
#   ./rebuild-all.sh --fresh     wipe DONE/checkpoint and rebuild everything
#   ./rebuild-all.sh --no-resume same as full rebuild of jobs but keep logs
#   ./rebuild-all.sh --status    print checkpoint / DONE counts and exit
#   Live: cat rebuild-logs/STATUS
#   Checkpoint: rebuild-logs/CHECKPOINT  (last completed phase marker)
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
DONE="$LOG/DONE"              # one completed job name per line
CHECKPOINT="$LOG/CHECKPOINT"  # last completed phase: 0 1 2 3
PIDFILE="$LOG/pid"
MAXJOBS=2
CONF="$ROOT/ff/source/ffconf.h"
CONF_BAK="$CONF.bak_rebuild"

RESUME=1
FRESH=0
STATUS_ONLY=0
for arg in "$@"; do
  case "$arg" in
    --fresh)
      FRESH=1
      RESUME=0
      ;;
    --no-resume)
      RESUME=0
      ;;
    --resume)
      RESUME=1
      ;;
    --status)
      STATUS_ONLY=1
      ;;
    -h|--help)
      sed -n '2,30p' "$0" | sed 's/^# \{0,1\}//'
      exit 0
      ;;
    *)
      echo "unknown option: $arg (try --help)" >&2
      exit 2
      ;;
  esac
done

mkdir -p "$LOG"

if (( STATUS_ONLY )); then
  echo "STATUS:     $(cat "$STATUS" 2>/dev/null || echo '(none)')"
  echo "CHECKPOINT: $(cat "$CHECKPOINT" 2>/dev/null || echo '(none)')"
  if [[ -f "$DONE" ]]; then
    echo "DONE jobs:  $(wc -l <"$DONE")"
  else
    echo "DONE jobs:  0 (no $DONE yet)"
  fi
  if [[ -f "$CONF_BAK" ]]; then
    echo "ffconf:     bak_rebuild present (RO build may have been interrupted)"
    grep -n "FF_FS_READONLY" "$CONF" | head -2 || true
  fi
  exit 0
fi

if (( FRESH )); then
  : >"$SUMMARY"
  : >"$STATUS"
  : >"$DONE"
  rm -f "$CHECKPOINT"
  # If a previous RO build left ffconf flipped, restore before anything else.
  if [[ -f "$CONF_BAK" ]]; then
    mv -f "$CONF_BAK" "$CONF"
    echo "$(date +%H:%M:%S)  restored ffconf.h from bak_rebuild (--fresh)" | tee -a "$SUMMARY"
  fi
elif (( ! RESUME )); then
  # --no-resume: rebuild all jobs, but keep old logs for forensics.
  : >"$DONE"
  rm -f "$CHECKPOINT"
  touch "$SUMMARY" "$STATUS"
  if [[ -f "$CONF_BAK" ]]; then
    mv -f "$CONF_BAK" "$CONF"
    echo "$(date +%H:%M:%S)  restored ffconf.h from bak_rebuild (--no-resume)" | tee -a "$SUMMARY"
  fi
else
  # Resume / continue: keep history; ensure tracking files exist.
  touch "$SUMMARY" "$DONE"
  # Seed DONE from prior summary lines if DONE is empty (first resume after
  # adding checkpoint support, or DONE lost but summary survived).
  if [[ ! -s "$DONE" && -s "$SUMMARY" ]]; then
    sed -n 's/.*DONE  \([^ ][^ ]*\).*/\1/p' "$SUMMARY" | sort -u >>"$DONE" || true
  fi
fi

running=0
done_ok=0
done_fail=0
started=0
skipped=0
phase="init"
ffconf_dirty=0   # 1 if we (or a prior run) left FF_FS_READONLY=1 with bak present

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
  say "PHASE $phase  (ok=$done_ok fail=$done_fail skip=$skipped running=$running)"
}

phase_done() {
  local n=$1
  printf '%s\n' "$n" >"$CHECKPOINT"
  say "PHASE $n/3 done  ok=$done_ok skip=$skipped"
}

# Map spawn job name → package-tree product path (empty if unknown).
product_path() {
  local name=$1
  local pkg target clib rest out
  # ff/rc2014/sccz80/ff_85  or  ff/rc2014/sccz80/ff_85_ro
  if [[ "$name" =~ ^(ff)/(rc2014)/(sccz80)/(ff_85_ro|ff_85)$ ]]; then
    echo "$ROOT/ff/rc2014/lib/newlib/sccz80/${BASH_REMATCH[4]}.lib"
    return 0
  fi
  # ff_ro/target/clib
  if [[ "$name" =~ ^(ff_ro)/([^/]+)/([^/]+)$ ]]; then
    echo "$ROOT/ff/${BASH_REMATCH[2]}/lib/newlib/${BASH_REMATCH[3]}/ff_ro.lib"
    return 0
  fi
  # pkg/target/clib  (standard)
  if [[ "$name" =~ ^([^/]+)/([^/]+)/([^/]+)$ ]]; then
    pkg=${BASH_REMATCH[1]}
    target=${BASH_REMATCH[2]}
    clib=${BASH_REMATCH[3]}
    # outname defaults to package basename (ff, time, diskio_sd, …)
    out=$pkg
    echo "$ROOT/$pkg/$target/lib/newlib/$clib/${out}.lib"
    return 0
  fi
  return 1
}

# Record a completed job (flock so concurrent children do not clobber).
mark_done() {
  local name=$1
  local lock="$DONE.lock"
  (
    flock 9
    if ! grep -Fxq "$name" "$DONE" 2>/dev/null; then
      printf '%s\n' "$name" >>"$DONE"
    fi
  ) 9>"$lock"
}

job_is_done() {
  local name=$1
  local p
  # --fresh / --no-resume: never skip (rebuild every job).
  if (( ! RESUME )); then
    return 1
  fi
  if grep -Fxq "$name" "$DONE" 2>/dev/null; then
    return 0
  fi
  p="$(product_path "$name" 2>/dev/null || true)"
  if [[ -n "${p:-}" && -s "$p" ]]; then
    # Product survived a crash; treat as complete and record it.
    mark_done "$name"
    return 0
  fi
  return 1
}

reap() {
  local st=0
  local last=""
  wait -n || st=$?
  running=$((running - 1))
  last="$(cat "$STATUS" 2>/dev/null || true)"
  if (( st != 0 )); then
    done_fail=$((done_fail + 1))
    say "FAIL  exit=$st  phase=$phase  ok=$done_ok fail=$done_fail run=$running"
    say "      hint: $last"
    say "      logs: $LOG  (see newest *.log)"
    say "      resume: re-run $0  (skips completed jobs in $DONE)"
    wait || true
    exit "$st"
  fi
  done_ok=$((done_ok + 1))
  say "OK    phase=$phase  ok=$done_ok fail=$done_fail skip=$skipped run=$running"
}

spawn() {
  local name=$1
  shift
  local logf="$LOG/${name//\//_}.log"

  if job_is_done "$name"; then
    skipped=$((skipped + 1))
    say "SKIP  $name  (already done)  skip=$skipped"
    return 0
  fi

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
      mark_done "$name"
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

# --- ffconf RO toggle (phase 2); restore on EXIT if interrupted ---
restore_ffconf() {
  if [[ -f "$CONF_BAK" ]]; then
    mv -f "$CONF_BAK" "$CONF"
    ffconf_dirty=0
    # best-effort log (may run during early abort)
    printf '%s\n' "$(date +%H:%M:%S)  restored ffconf.h from bak_rebuild" >>"$SUMMARY" 2>/dev/null || true
    printf '%s\n' "$(date +%H:%M:%S)  restored ffconf.h from bak_rebuild" >"$STATUS" 2>/dev/null || true
  fi
}

cleanup() {
  local st=$?
  restore_ffconf
  rm -f "$PIDFILE"
  exit "$st"
}
trap cleanup EXIT INT TERM

echo $$ >"$PIDFILE"

say "BEGIN rebuild  root=$ROOT  maxjobs=$MAXJOBS  sdcc_allocs=400000  resume=$RESUME"
say "      logs=$LOG  live=cat $STATUS  done=$DONE"
if (( RESUME )) && [[ -s "$DONE" || -f "$CHECKPOINT" ]]; then
  say "RESUME  checkpoint=$(cat "$CHECKPOINT" 2>/dev/null || echo none)  done_lines=$(wc -l <"$DONE" 2>/dev/null || echo 0)"
fi

# If a previous run died mid-RO, bak exists and conf is still RO — leave as-is
# until phase 2 finishes (or restore if phase 2 has nothing left to do).
if [[ -f "$CONF_BAK" ]]; then
  ffconf_dirty=1
  say "NOTE  ffconf.bak_rebuild present (interrupted RO build); will restore after phase 2"
fi

# ---------------------------------------------------------------------------
# Phase 0 — install existing package products so build-time headers exist.
# ---------------------------------------------------------------------------
last_cp="$(cat "$CHECKPOINT" 2>/dev/null || echo "")"
if (( RESUME )) && [[ -n "$last_cp" && "$last_cp" -ge 1 ]]; then
  say "SKIP  phase 0/3  (checkpoint=$last_cp ≥ 1; headers assumed installed)"
else
  phase_begin "0/3 prep headers"
  cd "$ROOT"
  : >"$LOG/install.log"
  install_pkg() {
    local target=$1
    shift
    local pkg
    for pkg in "$@"; do
      say "INSTALL +$target $pkg"
      z88dk-lib +"$target" -r -f "$pkg" >>"$LOG/install.log" 2>&1 || true
      if ! z88dk-lib +"$target" "$pkg" >>"$LOG/install.log" 2>&1; then
        say "FAIL  install +$target $pkg  (see $LOG/install.log)"
        exit 1
      fi
      say "OK    install +$target $pkg"
    done
  }

  # Headers needed as compile dependencies for later packages (time, diskio).
  install_pkg yaz180  time
  install_pkg scz180  time diskio_sd
  install_pkg hbios   time diskio_hbios
  install_pkg rc2014  time
  phase_done 0
fi

# Ensure install_pkg exists for phase 3 even if phase 0 was skipped.
if ! declare -F install_pkg >/dev/null; then
  install_pkg() {
    local target=$1
    shift
    local pkg
    for pkg in "$@"; do
      say "INSTALL +$target $pkg"
      z88dk-lib +"$target" -r -f "$pkg" >>"$LOG/install.log" 2>&1 || true
      if ! z88dk-lib +"$target" "$pkg" >>"$LOG/install.log" 2>&1; then
        say "FAIL  install +$target $pkg  (see $LOG/install.log)"
        exit 1
      fi
      say "OK    install +$target $pkg"
    done
  }
fi

# Phase 1 — RW libs
last_cp="$(cat "$CHECKPOINT" 2>/dev/null || echo "")"
if (( RESUME )) && [[ -n "$last_cp" && "$last_cp" -ge 1 ]]; then
  say "SKIP  phase 1/3 build RW  (checkpoint=$last_cp ≥ 1)"
else
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
  phase_done 1
fi

# Phase 2 — RO (flip ffconf, build, restore)
last_cp="$(cat "$CHECKPOINT" 2>/dev/null || echo "")"
if (( RESUME )) && [[ -n "$last_cp" && "$last_cp" -ge 2 ]]; then
  say "SKIP  phase 2/3 build RO  (checkpoint=$last_cp ≥ 2)"
  restore_ffconf
else
  phase_begin "2/3 build RO"

  # Prepare RO ffconf. If bak already exists we crashed mid-RO: conf should
  # already be RO — do not overwrite bak with the RO copy.
  if [[ -f "$CONF_BAK" ]]; then
    say "ffconf  using existing bak_rebuild (resume mid-RO)"
    ffconf_dirty=1
  else
    cp -a "$CONF" "$CONF_BAK"
    sed -i 's/#define FF_FS_READONLY  0/#define FF_FS_READONLY  1/g' "$CONF"
    ffconf_dirty=1
    say "ffconf  FF_FS_READONLY → 1"
  fi

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
  restore_ffconf
  say "ffconf  restored (RW)"
  phase_done 2
fi

# ---------------------------------------------------------------------------
# Phase 3 — install into z88dk tree (fresh products from phases 1–2)
# ---------------------------------------------------------------------------
last_cp="$(cat "$CHECKPOINT" 2>/dev/null || echo "")"
if (( RESUME )) && [[ -n "$last_cp" && "$last_cp" -ge 3 ]]; then
  say "SKIP  phase 3/3 install  (checkpoint=$last_cp ≥ 3; already complete)"
else
  cd "$ROOT"
  phase_begin "3/3 install"
  say "      ZCCCFG=$ZCCCFG  clibs=$Z88DK_CLIBS"
  : >>"$LOG/install.log"

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

  phase_done 3
fi

say "ALL OK  started=$started ok=$done_ok skip=$skipped fail=$done_fail  $(date -Iseconds)"
# Successful full run: clear trap side-effects already handled; leave DONE/CHECKPOINT
# for audit. Next --fresh starts clean.
trap - EXIT INT TERM
rm -f "$PIDFILE"
restore_ffconf
exit 0
