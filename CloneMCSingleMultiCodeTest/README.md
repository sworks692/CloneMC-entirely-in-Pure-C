# CloneMC V74 Java Terrain / Zlib / Performance

This package fixes the V73 terrain problems where the world could become mostly
sand with extreme Y spikes/dips. It also includes the uploaded `zlib.dll` in
`DLLS/` for multiplayer chunk decompression and reduces server join lag by
batching chunk apply + throttling display-list mesh rebuilds.

Build:

```bat
wmake -f Makefile.v74.wat
```

Output:

```text
CloneMC_V74_JavaTerrain_Zlib_Performance.exe
```

Windows 98 note: the included uploaded zlib.dll is a PE32/i386 DLL, but it appears
to target Windows subsystem 6.0 and imports newer Kernel32 APIs. For real Win98
testing, replace `DLLS/zlib.dll` with a Win9x-compatible zlib DLL.
