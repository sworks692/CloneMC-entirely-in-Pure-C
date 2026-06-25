# Open Watcom build instructions for CloneMC V58

Keep the project outside `C:\WATCOM\binnt64`. Open Watcom tools live there, but the game should be compiled from its own folder.

Example:

```text
C:\CloneMC_V58\
```

Set environment:

```bat
set WATCOM=C:\WATCOM
set PATH=C:\WATCOM\binnt64;%PATH%
set INCLUDE=C:\WATCOM\h;C:\WATCOM\h\nt
set LIB=C:\WATCOM\lib386;C:\WATCOM\lib386\nt
```

Build:

```bat
cd /d C:\CloneMC_V58
wmake -f Makefile.v58.wat
```

Manual commands:

```bat
wcc386 -bt=nt -dWIN32 -d_WINDOWS -i=src -fo=project2finalalpharecreation.obj project2finalalpharecreation.c
wlink @clonemc_v58_nt_win.lnk
```

The linker file uses `system nt_win`, not `system win`.
