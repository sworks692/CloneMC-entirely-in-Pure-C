# CloneMC source section map

The root `project2finalalpharecreation.c` is a unity driver that includes these current source sections in order. Compile the root file only.

- `00_core/` — constants, structs, globals, prototypes.
- `10_save/` — save/load, region/tile persistence.
- `20_entity/` — player, mobs, dropped items, mob rendering.
- `30_item/` — inventory, item use, crafting, furnace, redstone, special items.
- `40_world/` — terrain, biomes, infinite/finite world streaming, particles, scheduled ticks.
- `50_lighting/` — sky/block light arrays and brightness helpers.
- `60_ui/` — platform, resources, texture loading, GUI, audio/resource loading.
- `70_render/` — block rendering, chunk rendering, special entity rendering.

For V58 terrain work, start in:

```text
src/40_world/clonemc_world_generation_streaming.c
src/60_ui/clonemc_platform_ui_resources.c
```
