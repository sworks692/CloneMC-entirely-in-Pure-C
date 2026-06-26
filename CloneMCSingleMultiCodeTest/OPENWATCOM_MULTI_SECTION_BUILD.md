# CloneMC V74 Open Watcom build

Build command:

```bat
wmake -f Makefile.v74.wat
```

Output:

```text
CloneMC_V74_JavaTerrain_Zlib_Performance.exe
```

The project is still built as one Open Watcom translation unit by compiling
`project2finalalpharecreation.c`, which includes the split `src/` files in order.
The network source remains under `src/NETWORK/`.
