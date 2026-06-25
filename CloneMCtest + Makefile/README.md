# CloneMC V58 Priority 3 Terrain / Texture Default Fix

This build continues from V57 and focuses on terrain/world generation plus the default terrain texture not loading correctly.

## What changed

- Default terrain texture path now uses `assets/terrain.tga` first, matching Java-style texture pack expectations.
- Added `assets/terrain_v58_canonical.tga` as the canonical built-in terrain atlas.
- Added better terrain height generation with continental noise, ridge/mountain noise, coastline blending, valleys, and biome scaling.
- Added stronger biome surface rules for grass, sand, sandstone, gravel, clay, snow, ice, exposed stone, and mountain faces.
- Kept both infinite worlds and finite 862 x 862 worlds.
- Added biome decorations for reeds, cactus, flowers, tall grass, trees, clay, and pumpkins.

## Build

```text
wmake -f Makefile.v58.wat
```

Or manually:

```text
wcc386 -bt=nt -dWIN32 -d_WINDOWS -i=src -fo=project2finalalpharecreation.obj project2finalalpharecreation.c
wlink @clonemc_v58_nt_win.lnk
```

Run the EXE from this project folder so it can find `assets/`, `src/`, and `saves/`.
