## VECTORJET send

The library sends a list blob to the UX Module. The console pair stays a 6850. One C89 source builds the Z80 library and the 8085 library.

The blob is a 6-byte header, the payload, then reloc words. `vjet_send_list` adds the destination Hub address to each reloc word. It then writes `$1C`, the length, and the payload.

`$FF` uses BDOS function 2. Every other byte uses function 6. Function 2 expands TAB `$09`. Lists contain `$09`. Function 6 treats `$FF` as a read.

`vjet_key` uses function 11, then function 1. The program exits on `q`. `vjet_graphics` writes one `$07`.

A blob font of 0 skips the font check. Call `vjet_font` with the image font when the blob stores one.

## Installation

`z88dk-lib` installs the Z80 library.

```bash
z88dk-lib +cpm vjet
z88dk-lib +rc2014 vjet
```

The 8085 library is `vjet_8085.lib` beside `vjet.lib`. Copy it into `lib/clibs/sccz80/lib/cpm/`. `z88dk-lib` does not install that extra name.

## Calls

| Call | Action |
|------|--------|
| `vjet_font(hub)` | Expected font address. 0 skips the check. |
| `vjet_send_list(blob, dest)` | Fix reloc words. Write the frame. |
| `vjet_load(name, buf, max)` | Read one CP/M file. The count is a multiple of 128. |
| `vjet_graphics()` | Write one `$07`. |
| `vjet_key()` | Return a stored key, or 0. |

`vjet_send_list` returns `VJET_OK`, `VJET_ERR`, `VJET_RELOC`, or `VJET_FONT`.

## Preparation

Build from `source/`. These lines do not link a math library.

```bash
zcc +cpm -clib=new -x -O2 --opt-code-speed=all @vjet.lst -o ../cpm/lib/newlib/sccz80/vjet
zcc +cpm -clib=sdcc_ix -x -SO3 --max-allocs-per-node400000 @vjet.lst -o ../cpm/lib/newlib/sdcc_ix/vjet
zcc +cpm -clib=sdcc_iy -x -SO3 --max-allocs-per-node400000 @vjet.lst -o ../cpm/lib/newlib/sdcc_iy/vjet
zcc +cpm -clib=8085 -x -O2 --opt-code-speed=all @vjet.lst -o ../cpm/lib/newlib/sccz80/vjet_8085
```

The package holds the sccz80 libraries. Run the sdcc lines when `z88dk-zsdcc` is on PATH. Create `lib/newlib/sdcc_ix/` and `lib/newlib/sdcc_iy/` first. This tree does not ship those libraries. The same Z80 lines with `+rc2014` and `+yaz180` write those trees. The 8085 library stays on `+cpm`.

## Demonstration

The white box is `demo/vjet_box.c`. `q` returns to the CCP. `-create-app` writes `VJETBOX.COM`. That file is the program. The Z80 new library also leaves an empty `vjetbox` next to `vjetbox_CODE.bin`.

```bash
zcc +cpm -clib=8085 -O2 --opt-code-speed=all -I../source -L../cpm/lib/newlib/sccz80 -lvjet_8085 vjet_box.c -o vjetbox -create-app
zcc +cpm -clib=new -O2 --opt-code-speed=all -I../source -L../cpm/lib/newlib/sccz80 -lvjet vjet_box.c -o vjetbox -create-app
```

## License

MIT. See `License.txt`.
