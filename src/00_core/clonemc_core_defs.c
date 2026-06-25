/* ============================================================
   CloneMC V51 section: CORE / TYPES / GLOBALS / PROTOTYPES
   Keep this file included through the root unity source unless you
   are doing a later full extern/prototype object-file refactor.
   ============================================================ */

/*
    CloneMC Cave Game + Menu + Survival + Texture Atlas Prototype
    -------------------------------------------------------------

    Open Watcom 2.0
    Win32 desktop app
    Classic OpenGL 1.1 immediate mode

    Compile with Open Watcom 2.x:
        OPENWATCOM_MULTI_SECTION_BUILD.md

    Manual compile/link command:
        wcc386 -bt=nt -dWIN32 -d_WINDOWS -i=src -fo=project2finalalpharecreation.obj project2finalalpharecreation.c

    Do not build with only `wcl386 project2finalalpharecreation.c`; that lets the driver
    create an invalid/default linker system line on some Open Watcom V2 installs.

    Required asset files:
        assets\terrain.tga
        assets\icons.tga

    Controls:
        Menu:
            Left click Play
            Left click Settings
            ESC quits from menu

        Game:
            W/S = forward/back
            A/D = strafe
            Mouse = look around
            Space = jump
            Left click = mine block and collect item
            Right click = place selected hotbar block
            1-9 = select hotbar slot
            E = open/close inventory
            ESC = close inventory or return to menu

    Notes:
        This is a Cave Game / early Minecraft-inspired prototype.
        It uses OpenGL 1.1 glBegin/glEnd rendering for compatibility.
        Uploaded texture atlases are loaded from TGA files.
*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#pragma library("opengl32.lib")
#pragma library("glu32.lib")
#pragma library("gdi32.lib")
#pragma library("user32.lib")
#pragma library("winmm.lib")

/* V40: early Open Watcom prototypes.
   These functions are defined later in this single translation unit, but
   Open Watcom otherwise assumes old K&R implicit externs at earlier call sites. */
int IsSignEditingV5(void);
int WorldGenV3_IsCrossPlantBlock(int block);
int BlockToItem(int block);
int GetLightLevel(int x, int y, int z);

#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX 0x0BA6
#endif
#ifndef GL_PROJECTION_MATRIX
#define GL_PROJECTION_MATRIX 0x0BA7
#endif


/* ------------------------------------------------------------ */
/* Win98/Win10 F11 fullscreen compatibility                     */
/* ------------------------------------------------------------ */
#ifndef VK_F11
#define VK_F11 0x7A
#endif
#ifndef GWL_STYLE
#define GWL_STYLE (-16)
#endif
#ifndef SWP_NOMOVE
#define SWP_NOMOVE 0x0002
#endif
#ifndef SWP_NOSIZE
#define SWP_NOSIZE 0x0001
#endif
#ifndef SWP_NOZORDER
#define SWP_NOZORDER 0x0004
#endif
#ifndef SWP_FRAMECHANGED
#define SWP_FRAMECHANGED 0x0020
#endif
#ifndef SWP_SHOWWINDOW
#define SWP_SHOWWINDOW 0x0040
#endif
#ifndef HWND_TOP
#define HWND_TOP ((HWND)0)
#endif
#ifndef SM_CXSCREEN
#define SM_CXSCREEN 0
#endif
#ifndef SM_CYSCREEN
#define SM_CYSCREEN 1
#endif
#ifndef WS_THICKFRAME
#define WS_THICKFRAME 0x00040000L
#endif
#ifndef WS_CAPTION
#define WS_CAPTION 0x00C00000L
#endif
#ifndef WS_MINIMIZEBOX
#define WS_MINIMIZEBOX 0x00020000L
#endif
#ifndef WS_MAXIMIZEBOX
#define WS_MAXIMIZEBOX 0x00010000L
#endif
#ifndef WS_SYSMENU
#define WS_SYSMENU 0x00080000L
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef VK_F2
#define VK_F2 0x71
#endif
#ifndef VK_HOME
#define VK_HOME 0x24
#endif
#ifndef VK_END
#define VK_END 0x23
#endif
#ifndef VK_OEM_2
#define VK_OEM_2 0xBF
#endif
#ifndef GL_PACK_ALIGNMENT
#define GL_PACK_ALIGNMENT 0x0D05
#endif
#ifndef GL_VENDOR
#define GL_VENDOR 0x1F00
#endif
#ifndef GL_RENDERER
#define GL_RENDERER 0x1F01
#endif
#ifndef GL_VERSION
#define GL_VERSION 0x1F02
#endif
#ifndef GL_EXTENSIONS
#define GL_EXTENSIONS 0x1F03
#endif
#ifndef GL_MAX_TEXTURE_SIZE
#define GL_MAX_TEXTURE_SIZE 0x0D33
#endif

/* ------------------------------------------------------------ */
/* Window settings                                              */
/* ------------------------------------------------------------ */

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

/* ------------------------------------------------------------ */
/* World settings                                               */
/* ------------------------------------------------------------ */

#define WORLD_X 128
#define WORLD_Y 128
#define WORLD_Z 128

#define CHUNK_SIZE 16
#define CHUNK_HEIGHT WORLD_Y
#define WORLD_CHUNKS_X (WORLD_X / CHUNK_SIZE)
#define WORLD_CHUNKS_Z (WORLD_Z / CHUNK_SIZE)

/* Finite world presets. The full terrain is deterministic and bounded;
   only a small render/cache window is kept in RAM for Win98-friendly memory use.
   Beyond the playable square, columns become shallow ocean so the border
   visually fades into an old Alpha-style endless sea. */
#define WORLD_SIZE_INFINITE 0
#define FINITE_WORLD_SIZE_SMALL 862
#define FINITE_WORLD_SIZE_LARGE 862
#define FINITE_WORLD_MAX_SIZE   862
#define FINITE_BORDER_OCEAN_WIDTH 48
#define FINITE_BORDER_CLAMP_PAD 3
#define RENDER_DISTANCE_MIN_CHUNKS 1
#define RENDER_DISTANCE_MAX_CHUNKS 6
#define DAY_LENGTH_SECONDS 1200.0
#define MAX_PARTICLES 160
#define MAX_WEATHER_PARTICLES 96
#define MAX_MOBS 16
#define MAX_MOB_PROJECTILES 6
#define MOB_SPAWN_RADIUS_MIN 28
#define MOB_SPAWN_RADIUS_MAX 42
#define MOB_SOFT_UPDATE_DISTANCE 10.0
#define MOB_DESPAWN_DISTANCE 96.0
#define MOB_RENDER_DISTANCE 18.0
#define MOB_ATTACK_DISTANCE 2.05
#define MOB_ATTACK_VERTICAL_DISTANCE 2.40
#define MOB_HEAR_IDLE_DISTANCE 32.0
#define MOB_HEAR_STEP_DISTANCE 18.0
#define MOB_HEAR_HURT_DISTANCE 48.0
#define MOB_PASSIVE_CAP 6
#define MOB_HOSTILE_CAP 5
#define MOB_WATER_CAP 0
#define MOB_SPAWN_ATTEMPTS 8
#define MOB_SOUND_ALIAS_COUNT 24
#define MAX_TILE_ENTITIES 384
#define MAX_SIMPLE_RECIPES 256
#define MAX_NET_EVENTS 64
#define PATH_GRID_RADIUS 8
#define PATH_GRID_SIZE 17
#define MOB_PATH_MAX_POINTS 16
#define MOB_TARGET_NONE 0
#define MOB_TARGET_PLAYER 1
#define MOB_TARGET_FLEE_PLAYER 2
#define MOB_TARGET_WANDER 3
#define MOB_AI_REPATH_NEAR_SECONDS 0.45
#define MOB_AI_REPATH_FAR_SECONDS 1.20
#define MOB_AI_STUCK_SECONDS 0.85
#define MOB_AI_CLOSE_NODE_DISTANCE 0.55
#define MOB_AI_WATER_DRAG 0.82
#define MOB_AI_LAVA_DRAG 0.52
#define MOB_AI_FALL_SAFE_DISTANCE 3.0
#define MOB_AI_AIR_SECONDS 15.0
#define MOB_AI_DROWN_INTERVAL 1.0
#define MOB_AI_PUSH_STRENGTH 0.45
#define MOB_AI_FIRE_TICK_SECONDS 1.0
#define MOB_AI_V24_TARGET_LOST_SECONDS 4.0
#define MOB_AI_V24_HURT_RESIST_SECONDS 0.45
#define MOB_AI_V24_MELEE_COOLDOWN 1.05
#define MOB_AI_V24_PATH_WATER_PENALTY 12
#define MOB_AI_V24_PATH_LAVA_PENALTY 200
#define MOB_AI_V24_PATH_DOOR_PENALTY 10
#define MOB_AI_V24_PATH_DROP_PENALTY 18
#define MOB_AI_V24_PATH_STEP_PENALTY 6
#define PARTICLE_V24_DIGGING 1
#define PARTICLE_V24_SMOKE 2
#define PARTICLE_V24_FLAME 3
#define PARTICLE_V24_BUBBLE 4
#define PARTICLE_V24_SPLASH 5
#define PARTICLE_V24_RAIN 6
#define PARTICLE_V24_SNOW 7
#define PARTICLE_V24_EXPLOSION 8
#define PARTICLE_V24_TORCH 9
#define STREAM_EDGE_MARGIN_BLOCKS (CHUNK_SIZE * 2)
#define FULL_GAMMA_BOOST 1.00f

#define WATER_LEVEL 5
#define RENDER_DISTANCE 40
#define NEAR_BLOCK_RENDER_DISTANCE 22
#define VERY_NEAR_BLOCK_RENDER_DISTANCE 12
#define FAR_COLUMN_RENDER_DISTANCE 48
#define MAX_DROPPED_ITEMS 48
#define MAX_PICKUP_FX_V38 16
#define ENTITY_ITEM_DESPAWN_SECONDS_V38 300.0
#define ENTITY_PICKUP_FX_TIME_V38 0.18

/* Java save/load conversion pass.
   Implements C equivalents for the responsibilities of NBTBase,
   NBTTagCompound, NBTTagList, NBTTagByteArray, CompressedStreamTools,
   RegionFile, RegionFileCache, RegionFileChunkBuffer, McRegionChunkLoader,
   SaveHandler, SaveFormatOld, SaveConverterMcRegion, WorldInfo, and
   ChunkLoader.  The files are intentionally compact, no-zlib, Win98-friendly
   binary files with Java-style tag names and safe tmp-to-final commits. */
#define CMC_SAVE_VERSION 3
#define CMC_REGION_VERSION 3
#define CMC_NBT_MAGIC "CNBT"
#define SAVE_INV_SLOT_BYTES 12
#define SAVE_MOB_RECORD_BYTES 192
#define SAVE_DROP_RECORD_BYTES 80
#define SAVE_SPECIAL_RECORD_BYTES_V44 136
#define SAVE_TILE_RECORD_BYTES (20 + 16 + 64 + (27 * SAVE_INV_SLOT_BYTES))
#define SAVE_CHUNK_BLOCK_COUNT (CHUNK_SIZE * WORLD_Y * CHUNK_SIZE)
#define SAVE_REGION_FILE_NAME "r.0.0.mcr"
#define CMC_REGION3_ENTRY_COUNT 1024
#define CMC_REGION3_OFFSET_TABLE 16L
#define CMC_REGION3_LENGTH_TABLE (CMC_REGION3_OFFSET_TABLE + (CMC_REGION3_ENTRY_COUNT * 4L))
#define CMC_REGION3_TIME_TABLE (CMC_REGION3_LENGTH_TABLE + (CMC_REGION3_ENTRY_COUNT * 4L))
#define CMC_REGION3_HEADER_BYTES (CMC_REGION3_TIME_TABLE + (CMC_REGION3_ENTRY_COUNT * 4L))
#define CMC_REGION3_COMPRESSION_NONE 0
#define CMC_REGION3_MAX_RECORD_BYTES 262144
#define CAMERA_FIRST_PERSON 0
#define CAMERA_THIRD_BACK 1
#define CAMERA_THIRD_FRONT 2
#define CLONEMC_VERSION_TEXT "CloneMC Beta-style C 0.9.0 V58 Terrain/Biome/Texture"

#define DEFAULT_WORLDGEN_SEED 173773
#define GEN_WATER_LEVEL 24
#define GEN_BASE_HEIGHT 34
#define GEN_DIRT_DEPTH 4
#define GEN_MAX_SURFACE (WORLD_Y - 8)
#define PLAYER_SPAWN_MIN_Y 20
#define PLAYER_SPAWN_MAX_Y 50

/*
    Extra passes inspired by Beta-style generation:
    base stone density, random-walker caves, liquid pass, surface texturing.
*/
#define GEN_CAVE_WALKERS 28
#define GEN_CAVE_STEPS 44
#define BETA_CHUNK_SIZE 16

#define CLOUD_HEIGHT 72.0f
#define CLOUD_CELL_SIZE 8.0f
#define CLOUD_RADIUS_CELLS 18
#define CLOUD_SCROLL_SPEED 0.75

/* ------------------------------------------------------------ */
/* Texture atlas settings                                       */
/* ------------------------------------------------------------ */

#define TILE_SIZE 16

/*
    EDITABLE TEXTURE COORDINATE MAP
    -------------------------------
    This project uses your uploaded 512 x 256 terrain.png/terrain.tga.
    It is NOT the plain vanilla Beta terrain.png layout, so direct atlas
    coordinates are used here instead of assuming Java's 16-column texture
    index layout.

    Java Block.java is still used for the block IDs and block names. The
    uploaded PNG is used for the actual tile location.

    To swap any block texture, edit only the matching TILE_*_COL and
    TILE_*_ROW pair below. Coordinates are tile positions, not pixels:
        col = horizontal tile number from left
        row = vertical tile number from top
*/

#define TERRAIN_ATLAS_WIDTH  512
#define TERRAIN_ATLAS_HEIGHT 256

#define ICONS_ATLAS_WIDTH  256
#define ICONS_ATLAS_HEIGHT 256

/* Main terrain blocks from uploaded terrain.png. */
#define TILE_STONE_COL       0
#define TILE_STONE_ROW       0
#define TILE_GRASS_SIDE_COL  3
#define TILE_GRASS_SIDE_ROW  0
#define TILE_GRASS_TOP_COL   2
#define TILE_GRASS_TOP_ROW   0
#define TILE_DIRT_COL        18
#define TILE_DIRT_ROW        1
#define TILE_PLANKS_COL      21
#define TILE_PLANKS_ROW      1
#define TILE_COBBLE_COL      1
#define TILE_COBBLE_ROW      5
#define TILE_BEDROCK_COL     0
#define TILE_BEDROCK_ROW     1
#define TILE_SAND_COL        5
#define TILE_SAND_ROW        1
#define TILE_GRAVEL_COL      26
#define TILE_GRAVEL_ROW      4

/* Tree / nature tiles. */
#define TILE_WOOD_SIDE_COL   3
#define TILE_WOOD_SIDE_ROW   3
#define TILE_WOOD_TOP_COL    4
#define TILE_WOOD_TOP_ROW    3
#define TILE_LEAVES_COL      8
#define TILE_LEAVES_ROW      5
#define TILE_WOOL_COL        15
#define TILE_WOOL_ROW        3

/* Ores from uploaded terrain.png. */
#define TILE_ORE_IRON_COL      0
#define TILE_ORE_IRON_ROW      4
#define TILE_ORE_COAL_COL      1
#define TILE_ORE_COAL_ROW      4
#define TILE_ORE_LAPIS_COL     2
#define TILE_ORE_LAPIS_ROW     4
#define TILE_ORE_DIAMOND_COL   3
#define TILE_ORE_DIAMOND_ROW   4
#define TILE_ORE_REDSTONE_COL  4
#define TILE_ORE_REDSTONE_ROW  4
#define TILE_ORE_GOLD_COL      28
#define TILE_ORE_GOLD_ROW      4

/* Crafting table tiles. */
#define TILE_WORKBENCH_TOP_COL   12
#define TILE_WORKBENCH_TOP_ROW   4
#define TILE_WORKBENCH_SIDE_COL  13
#define TILE_WORKBENCH_SIDE_ROW  4
#define TILE_WORKBENCH_FRONT_COL 14
#define TILE_WORKBENCH_FRONT_ROW 4

/* Additional craftable/block tiles used by the advanced save + recipe pass.
   These are intentionally centralized so swapping a texture is a one-line edit. */
#define TILE_TORCH_COL       25
#define TILE_TORCH_ROW       5
#define TILE_FURNACE_SIDE_COL 17
#define TILE_FURNACE_SIDE_ROW 4
#define TILE_FURNACE_FRONT_COL 15
#define TILE_FURNACE_FRONT_ROW 4
#define TILE_CHEST_COL       28
#define TILE_CHEST_ROW       5

/* Biome / special blocks. */
#define TILE_SNOW_COL        15
#define TILE_SNOW_ROW        3
#define TILE_ICE_COL         13
#define TILE_ICE_ROW         5
#define TILE_CACTUS_TOP_COL     17
#define TILE_CACTUS_TOP_ROW     5
#define TILE_CACTUS_SIDE_COL    15
#define TILE_CACTUS_SIDE_ROW    5
#define TILE_CACTUS_BOTTOM_COL  17
#define TILE_CACTUS_BOTTOM_ROW  5

/* Sandstone from uploaded terrain.png. */
#define TILE_SANDSTONE_SIDE_COL   8
#define TILE_SANDSTONE_SIDE_ROW   1
#define TILE_SANDSTONE_TOP_COL    9
#define TILE_SANDSTONE_TOP_ROW    1
#define TILE_SANDSTONE_BOTTOM_COL 10
#define TILE_SANDSTONE_BOTTOM_ROW 1
#define TILE_SANDSTONE_COL        TILE_SANDSTONE_SIDE_COL
#define TILE_SANDSTONE_ROW        TILE_SANDSTONE_SIDE_ROW

/* Water uses the bottom-left animated-water style tiles in this atlas. */
#define TILE_WATER_STILL_COL  0
#define TILE_WATER_STILL_ROW  13
#define TILE_WATER_FLOW_COL   1
#define TILE_WATER_FLOW_ROW   13
#define TILE_WATER_COL        TILE_WATER_STILL_COL
#define TILE_WATER_ROW        TILE_WATER_STILL_ROW

#define ICON_EMPTY_HEART_COL 3
#define ICON_EMPTY_HEART_ROW 0
#define ICON_FULL_HEART_COL  4
#define ICON_FULL_HEART_ROW  0
#define ICON_HALF_HEART_COL  5
#define ICON_HALF_HEART_ROW  0

/*
    EDITABLE WATER / HUD TUNING
    ---------------------------
    These values control the underwater screen overlay, darkening, and
    oxygen-bubble HUD.  Increase the alpha values for a stronger Beta-style
    underwater look, or set them lower if the view is too dark.
*/
#define WATER_OVERLAY_TILE_PIXELS 64
#define WATER_OVERLAY_ALPHA_HEAD  0.30f
#define WATER_OVERLAY_ALPHA_BODY  0.10f
#define WATER_DARKEN_ALPHA_HEAD   0.22f
#define WATER_DARKEN_ALPHA_BODY   0.08f
#define BUBBLE_ICON_SIZE          18

/* ------------------------------------------------------------ */
/* Block IDs                                                    */
/* ------------------------------------------------------------ */

#define BLOCK_AIR        0
#define BLOCK_STONE      1
#define BLOCK_GRASS      2
#define BLOCK_DIRT       3
#define BLOCK_COBBLESTONE 4
#define BLOCK_PLANKS     5
#define BLOCK_BEDROCK    7
#define BLOCK_BORDER     BLOCK_BEDROCK
#define BLOCK_WATER      9
#define BLOCK_SAND       12
#define BLOCK_GRAVEL     13
#define BLOCK_GOLD_ORE   14
#define BLOCK_IRON_ORE   15
#define BLOCK_COAL_ORE   16
#define BLOCK_WOOD       17
#define BLOCK_LEAVES     18
#define BLOCK_LAPIS_ORE  21
#define BLOCK_GLASS      20
#define BLOCK_SANDSTONE  24
#define BLOCK_WOOL       35
#define BLOCK_DIAMOND_ORE 56
#define BLOCK_TORCH     50
#define BLOCK_CHEST     54
#define BLOCK_WORKBENCH  58
#define BLOCK_FURNACE    61
#define BLOCK_LIT_FURNACE 62
#define BLOCK_REDSTONE_ORE 73
#define BLOCK_SNOW       78
#define BLOCK_ICE        79
#define BLOCK_CACTUS     81
#define BLOCK_REDSTONE_WIRE 55
#define BLOCK_MOB_SPAWNER 52
#define BLOCK_SIGN_POST 63
#define BLOCK_WOOD_DOOR 64
#define BLOCK_LADDER 65
#define BLOCK_RAIL 66
#define BLOCK_LEVER 69
#define BLOCK_STONE_PRESSURE_PLATE 70
#define BLOCK_WOOD_PRESSURE_PLATE 72
#define BLOCK_REDSTONE_TORCH_ON 76
#define BLOCK_STONE_BUTTON 77
#define BLOCK_NOTE 25
#define BLOCK_DISPENSER 23
#define BLOCK_PISTON 33
#define BLOCK_LIGHT      200


/* CLONEMC_JAVA_ASSET_COMPLETION_PASS: extra block IDs mapped from uploaded Java Block*.java files. */
#ifndef BLOCK_SAPLING
#define BLOCK_SAPLING 6
#endif
#ifndef BLOCK_LAVA
#define BLOCK_LAVA 10
#endif
#ifndef BLOCK_STATIONARY_LAVA
#define BLOCK_STATIONARY_LAVA 11
#endif
#ifndef BLOCK_WEB
#define BLOCK_WEB 30
#endif
#ifndef BLOCK_TALL_GRASS
#define BLOCK_TALL_GRASS 31
#endif
#ifndef BLOCK_DEAD_BUSH
#define BLOCK_DEAD_BUSH 32
#endif
#ifndef BLOCK_PISTON_STICKY
#define BLOCK_PISTON_STICKY 29
#endif
#ifndef BLOCK_PISTON_EXTENSION
#define BLOCK_PISTON_EXTENSION 34
#endif
#ifndef BLOCK_FLOWER_YELLOW
#define BLOCK_FLOWER_YELLOW 37
#endif
#ifndef BLOCK_FLOWER_RED
#define BLOCK_FLOWER_RED 38
#endif
#ifndef BLOCK_MUSHROOM_BROWN
#define BLOCK_MUSHROOM_BROWN 39
#endif
#ifndef BLOCK_MUSHROOM_RED
#define BLOCK_MUSHROOM_RED 40
#endif
#ifndef BLOCK_GOLD_BLOCK
#define BLOCK_GOLD_BLOCK 41
#endif
#ifndef BLOCK_IRON_BLOCK
#define BLOCK_IRON_BLOCK 42
#endif
#ifndef BLOCK_DOUBLE_STEP
#define BLOCK_DOUBLE_STEP 43
#endif
#ifndef BLOCK_STEP
#define BLOCK_STEP 44
#endif
#ifndef BLOCK_BRICK
#define BLOCK_BRICK 45
#endif
#ifndef BLOCK_TNT
#define BLOCK_TNT 46
#endif
#ifndef BLOCK_BOOKSHELF
#define BLOCK_BOOKSHELF 47
#endif
#ifndef BLOCK_MOSSY_COBBLESTONE
#define BLOCK_MOSSY_COBBLESTONE 48
#endif
#ifndef BLOCK_OBSIDIAN
#define BLOCK_OBSIDIAN 49
#endif
#ifndef BLOCK_FIRE
#define BLOCK_FIRE 51
#endif
#ifndef BLOCK_DIAMOND_BLOCK
#define BLOCK_DIAMOND_BLOCK 57
#endif
#ifndef BLOCK_CROPS
#define BLOCK_CROPS 59
#endif
#ifndef BLOCK_FARMLAND
#define BLOCK_FARMLAND 60
#endif
#ifndef BLOCK_FURNACE_LIT
#define BLOCK_FURNACE_LIT 62
#endif
#ifndef BLOCK_SIGN_WALL
#define BLOCK_SIGN_WALL 68
#endif
#ifndef BLOCK_REDSTONE_TORCH_OFF
#define BLOCK_REDSTONE_TORCH_OFF 75
#endif
#ifndef BLOCK_SNOW_BLOCK
#define BLOCK_SNOW_BLOCK 80
#endif
#ifndef BLOCK_CLAY
#define BLOCK_CLAY 82
#endif
#ifndef BLOCK_REED
#define BLOCK_REED 83
#endif
#ifndef BLOCK_JUKEBOX
#define BLOCK_JUKEBOX 84
#endif
#ifndef BLOCK_FENCE
#define BLOCK_FENCE 85
#endif
#ifndef BLOCK_PUMPKIN
#define BLOCK_PUMPKIN 86
#endif
#ifndef BLOCK_NETHERRACK
#define BLOCK_NETHERRACK 87
#endif
#ifndef BLOCK_SOULSAND
#define BLOCK_SOULSAND 88
#endif
#ifndef BLOCK_GLOWSTONE
#define BLOCK_GLOWSTONE 89
#endif
#ifndef BLOCK_PORTAL
#define BLOCK_PORTAL 90
#endif
#ifndef BLOCK_JACK_O_LANTERN
#define BLOCK_JACK_O_LANTERN 91
#endif
#ifndef BLOCK_CAKE
#define BLOCK_CAKE 92
#endif
#ifndef BLOCK_REPEATER_OFF
#define BLOCK_REPEATER_OFF 93
#endif
#ifndef BLOCK_REPEATER_ON
#define BLOCK_REPEATER_ON 94
#endif
#ifndef BLOCK_DETECTOR_RAIL
#define BLOCK_DETECTOR_RAIL 28
#endif
#ifndef BLOCK_LOCKED_CHEST
#define BLOCK_LOCKED_CHEST 95
#endif
#ifndef BLOCK_TRAPDOOR
#define BLOCK_TRAPDOOR 96
#ifndef BLOCK_BED
#define BLOCK_BED 26
#endif
#ifndef BLOCK_WOOD_STAIRS
#define BLOCK_WOOD_STAIRS 53
#endif
#ifndef BLOCK_COBBLESTONE_STAIRS
#define BLOCK_COBBLESTONE_STAIRS 67
#endif
#ifndef BLOCK_IRON_DOOR
#define BLOCK_IRON_DOOR 71
#endif
#ifndef BLOCK_WALL_SIGN
#define BLOCK_WALL_SIGN BLOCK_SIGN_WALL
#endif
#endif

#define BIOME_RAINFOREST       0
#define BIOME_FOREST           1
#define BIOME_SEASONAL_FOREST  2
#define BIOME_SHRUBLAND        3
#define BIOME_PLAINS           4
#define BIOME_SWAMPLAND        5
#define BIOME_TAIGA            6
#define BIOME_TUNDRA           7
#define BIOME_DESERT           8
#define BIOME_OCEAN            9

/* Block harvest tool constants are used by early ITEM sections before
   the full BlockDef registry section is included. Keep these values in
   sync with the registry constants in clonemc_platform_ui_resources.c. */
#ifndef BLOCK_HARVEST_NONE_V49
#define BLOCK_HARVEST_NONE_V49      0
#define BLOCK_HARVEST_PICKAXE_V49   1
#define BLOCK_HARVEST_SHOVEL_V49    2
#define BLOCK_HARVEST_AXE_V49       3
#define BLOCK_HARVEST_SHEARS_V49    4
#define BLOCK_HARVEST_SWORD_V49     5
#endif

/* ------------------------------------------------------------ */
/* Item IDs                                                     */
/* ------------------------------------------------------------ */

#define ITEM_NONE         0
#define ITEM_STONE        BLOCK_STONE
#define ITEM_GRASS        BLOCK_GRASS
#define ITEM_DIRT         BLOCK_DIRT
#define ITEM_COBBLESTONE  BLOCK_COBBLESTONE
#define ITEM_PLANKS       BLOCK_PLANKS
#define ITEM_WATER        9001
#define ITEM_LIGHT        9000
#define ITEM_SAND         BLOCK_SAND
#define ITEM_GRAVEL       BLOCK_GRAVEL
#define ITEM_GLASS        BLOCK_GLASS
#define ITEM_WOOD         BLOCK_WOOD
#define ITEM_LEAVES       BLOCK_LEAVES
#define ITEM_SANDSTONE    BLOCK_SANDSTONE
#define ITEM_WOOL         BLOCK_WOOL
#define ITEM_TORCH       BLOCK_TORCH
#define ITEM_CHEST       BLOCK_CHEST
#define ITEM_WORKBENCH    BLOCK_WORKBENCH
#define ITEM_FURNACE      BLOCK_FURNACE
#define ITEM_COAL_ORE     BLOCK_COAL_ORE
#define ITEM_IRON_ORE     BLOCK_IRON_ORE
#define ITEM_GOLD_ORE     BLOCK_GOLD_ORE
#define ITEM_DIAMOND_ORE  BLOCK_DIAMOND_ORE
#define ITEM_REDSTONE_ORE BLOCK_REDSTONE_ORE
#define ITEM_LAPIS_ORE    BLOCK_LAPIS_ORE
#define ITEM_COAL         263
#define ITEM_DIAMOND      264
#define ITEM_WOOD_SWORD   268
#define ITEM_WOOD_SHOVEL  269
#define ITEM_WOOD_PICKAXE 270
#define ITEM_WOOD_AXE     271
#define ITEM_STONE_SWORD  272
#define ITEM_STONE_SHOVEL 273
#define ITEM_STONE_PICKAXE 274
#define ITEM_STONE_AXE    275
#define ITEM_STICK        280
#define ITEM_BOWL         281
#define ITEM_STRING       287
#define ITEM_FEATHER      288
#define ITEM_GUNPOWDER    289
#define ITEM_ARROW        262
#define ITEM_PORK_RAW     319
#define ITEM_PORK_COOKED  320
#define ITEM_LEATHER      334
#define ITEM_REDSTONE     331
#define ITEM_SLIMEBALL    341
#define ITEM_DYE_POWDER   351
#define ITEM_LAPIS_DYE     1351  /* Internal stand-in for dyePowder damage 4; draws blue/lapis icon. */
#define ITEM_IRON_INGOT    265
#define ITEM_GOLD_INGOT    266
#define ITEM_BONE         352
#define ITEM_EGG          344
#define ITEM_FLINT        318
#define ITEM_SNOWBALL     332
#define ITEM_CACTUS       BLOCK_CACTUS
#define ITEM_ICE          BLOCK_ICE
#define ITEM_SNOW_BLOCK   BLOCK_SNOW
#define ITEM_IRON_SWORD   267
#define ITEM_IRON_SHOVEL  256
#define ITEM_IRON_PICKAXE 257
#define ITEM_IRON_AXE     258
#define ITEM_DIAMOND_SWORD   276
#define ITEM_DIAMOND_SHOVEL  277
#define ITEM_DIAMOND_PICKAXE 278
#define ITEM_DIAMOND_AXE     279
#define ITEM_GOLD_SWORD   283
#define ITEM_GOLD_SHOVEL  284
#define ITEM_GOLD_PICKAXE 285
#define ITEM_GOLD_AXE     286
#define ITEM_WOOD_HOE     290
#define ITEM_STONE_HOE    291
#define ITEM_IRON_HOE     292
#define ITEM_DIAMOND_HOE  293
#define ITEM_GOLD_HOE     294
#define ITEM_FLINT_STEEL  259
#define ITEM_BOW          261
#define ITEM_BUCKET       325
#define ITEM_WATER_BUCKET 326
#define ITEM_MILK_BUCKET  335
#define ITEM_SHEARS       359
#define ITEM_WOOD_DOOR    324
#define ITEM_SIGN         323
#define ITEM_BOAT         333
#define ITEM_LADDER       BLOCK_LADDER
#define ITEM_RAIL         BLOCK_RAIL
#define ITEM_MINECART     328
#define ITEM_REDSTONE_TORCH BLOCK_REDSTONE_TORCH_ON
#define ITEM_LEVER        BLOCK_LEVER
#define ITEM_BUTTON       BLOCK_STONE_BUTTON
#define ITEM_REDSTONE_WIRE BLOCK_REDSTONE_WIRE
#define ITEM_REPEATER     356
#define ITEM_FISHING_ROD  346
#define ITEM_MAP          358


/* Extra item IDs mapped from uploaded Java Item*.java and Recipes*.java files. */
#ifndef ITEM_LAVA_BUCKET
#define ITEM_LAVA_BUCKET 327
#endif
#ifndef ITEM_SADDLE
#define ITEM_SADDLE 329
#endif
#ifndef ITEM_IRON_DOOR
#define ITEM_IRON_DOOR 330
#endif
#ifndef ITEM_CLAY_BALL
#define ITEM_CLAY_BALL 337
#endif
#ifndef ITEM_REED
#define ITEM_REED 338
#endif
#ifndef ITEM_PAPER
#define ITEM_PAPER 339
#endif
#ifndef ITEM_BOOK
#define ITEM_BOOK 340
#endif
#ifndef ITEM_CHEST_MINECART
#define ITEM_CHEST_MINECART 342
#endif
#ifndef ITEM_FURNACE_MINECART
#define ITEM_FURNACE_MINECART 343
#endif
#ifndef ITEM_COMPASS
#define ITEM_COMPASS 345
#endif
#ifndef ITEM_CLOCK
#define ITEM_CLOCK 347
#endif
#ifndef ITEM_GLOWSTONE_DUST
#define ITEM_GLOWSTONE_DUST 348
#endif
#ifndef ITEM_FISH_RAW
#define ITEM_FISH_RAW 349
#endif
#ifndef ITEM_FISH_COOKED
#define ITEM_FISH_COOKED 350
#endif
#ifndef ITEM_BED
#define ITEM_BED 355
#endif
#ifndef ITEM_COOKIE
#define ITEM_COOKIE 357
#endif
#ifndef ITEM_CAKE
#define ITEM_CAKE 354
#endif
#ifndef ITEM_CHICKEN_RAW
#define ITEM_CHICKEN_RAW 365
#endif
#ifndef ITEM_CHICKEN_COOKED
#define ITEM_CHICKEN_COOKED 366
#endif
#ifndef ITEM_SEEDS
#define ITEM_SEEDS 295
#endif
#ifndef ITEM_WHEAT
#define ITEM_WHEAT 296
#endif
#ifndef ITEM_BREAD
#define ITEM_BREAD 297
#endif
#ifndef ITEM_LEATHER_HELMET
#define ITEM_LEATHER_HELMET 298
#endif
#ifndef ITEM_LEATHER_CHESTPLATE
#define ITEM_LEATHER_CHESTPLATE 299
#endif
#ifndef ITEM_LEATHER_LEGGINGS
#define ITEM_LEATHER_LEGGINGS 300
#endif
#ifndef ITEM_LEATHER_BOOTS
#define ITEM_LEATHER_BOOTS 301
#endif
#ifndef ITEM_CHAIN_HELMET
#define ITEM_CHAIN_HELMET 302
#endif
#ifndef ITEM_CHAIN_CHESTPLATE
#define ITEM_CHAIN_CHESTPLATE 303
#endif
#ifndef ITEM_CHAIN_LEGGINGS
#define ITEM_CHAIN_LEGGINGS 304
#endif
#ifndef ITEM_CHAIN_BOOTS
#define ITEM_CHAIN_BOOTS 305
#endif
#ifndef ITEM_IRON_HELMET
#define ITEM_IRON_HELMET 306
#endif
#ifndef ITEM_IRON_CHESTPLATE
#define ITEM_IRON_CHESTPLATE 307
#endif
#ifndef ITEM_IRON_LEGGINGS
#define ITEM_IRON_LEGGINGS 308
#endif
#ifndef ITEM_IRON_BOOTS
#define ITEM_IRON_BOOTS 309
#endif
#ifndef ITEM_DIAMOND_HELMET
#define ITEM_DIAMOND_HELMET 310
#endif
#ifndef ITEM_DIAMOND_CHESTPLATE
#define ITEM_DIAMOND_CHESTPLATE 311
#endif
#ifndef ITEM_DIAMOND_LEGGINGS
#define ITEM_DIAMOND_LEGGINGS 312
#endif
#ifndef ITEM_DIAMOND_BOOTS
#define ITEM_DIAMOND_BOOTS 313
#endif
#ifndef ITEM_GOLD_HELMET
#define ITEM_GOLD_HELMET 314
#endif
#ifndef ITEM_GOLD_CHESTPLATE
#define ITEM_GOLD_CHESTPLATE 315
#endif
#ifndef ITEM_GOLD_LEGGINGS
#define ITEM_GOLD_LEGGINGS 316
#endif
#ifndef ITEM_GOLD_BOOTS
#define ITEM_GOLD_BOOTS 317
#endif
#ifndef ITEM_APPLE
#define ITEM_APPLE 260
#ifndef ITEM_GOLDEN_APPLE
#define ITEM_GOLDEN_APPLE 322
#endif
#endif
#ifndef ITEM_MUSHROOM_STEW
#define ITEM_MUSHROOM_STEW 282
#endif
#ifndef ITEM_BRICK
#define ITEM_BRICK 336
#endif

#ifndef ITEM_SUGAR
#define ITEM_SUGAR 353
#endif

#ifndef ITEM_RECORD_13
#define ITEM_RECORD_13 2256
#endif
#ifndef ITEM_RECORD_CAT
#define ITEM_RECORD_CAT 2257
#endif


/* TILE_RECIPE_REDSTONE_V5: aliases for Java block/item classes that were present in
   the uploaded source but were still only partially active in the C build. */
#ifndef ITEM_FENCE
#define ITEM_FENCE BLOCK_FENCE
#endif
#ifndef ITEM_JUKEBOX
#define ITEM_JUKEBOX BLOCK_JUKEBOX
#endif
#ifndef ITEM_NOTE_BLOCK
#define ITEM_NOTE_BLOCK BLOCK_NOTE
#endif
#ifndef ITEM_DISPENSER
#define ITEM_DISPENSER BLOCK_DISPENSER
#endif
#ifndef ITEM_TNT
#define ITEM_TNT BLOCK_TNT
#endif
#ifndef ITEM_BOOKSHELF
#define ITEM_BOOKSHELF BLOCK_BOOKSHELF
#endif
#ifndef ITEM_BRICK_BLOCK
#define ITEM_BRICK_BLOCK BLOCK_BRICK
#endif
#ifndef ITEM_CLAY_BLOCK
#define ITEM_CLAY_BLOCK BLOCK_CLAY
#endif
#ifndef ITEM_GLOWSTONE_BLOCK
#define ITEM_GLOWSTONE_BLOCK BLOCK_GLOWSTONE
#endif
#ifndef ITEM_TRAPDOOR
#define ITEM_TRAPDOOR BLOCK_TRAPDOOR
#endif
#ifndef ITEM_PISTON
#define ITEM_PISTON BLOCK_PISTON
#endif
#ifndef ITEM_STICKY_PISTON
#define ITEM_STICKY_PISTON BLOCK_PISTON_STICKY
#endif
#ifndef ITEM_REDSTONE_REPEATER
#define ITEM_REDSTONE_REPEATER ITEM_REPEATER
#endif
#ifndef ITEM_PRESSURE_PLATE_STONE
#define ITEM_PRESSURE_PLATE_STONE BLOCK_STONE_PRESSURE_PLATE
#endif
#ifndef ITEM_PRESSURE_PLATE_WOOD
#define ITEM_PRESSURE_PLATE_WOOD BLOCK_WOOD_PRESSURE_PLATE
#endif
#ifndef ITEM_REDSTONE_LAMP_PLACEHOLDER
#define ITEM_REDSTONE_LAMP_PLACEHOLDER BLOCK_GLOWSTONE
#endif

/* ------------------------------------------------------------ */
/* Survival / inventory settings                                */
/* ------------------------------------------------------------ */

#define MAX_HEALTH 20
#define HEART_COUNT 10

#define HOTBAR_SLOTS 9
#define INVENTORY_SLOTS 27
#define MAX_STACK 64

/*
    Legacy Notchian-style grayscale lighting.
    Light values are 0 to 15, matching the classic Minecraft idea.
*/
#define LIGHT_MAX_LEVEL 15
#define LIGHT_SOURCE_LEVEL 14
#define MAX_LIGHT_NODES (WORLD_X * WORLD_Y * WORLD_Z)

/* ------------------------------------------------------------ */
/* Game states                                                  */
/* ------------------------------------------------------------ */

#define STATE_MENU          0
#define STATE_GAME          1
#define STATE_SETTINGS      2
#define STATE_WORLD_SELECT  3
#define STATE_CREATE_WORLD  4
#define STATE_OPTIONS       5
#define STATE_PAUSE         6
#define STATE_DEATH         7
#define STATE_RENAME_WORLD  8
#define STATE_VIDEO_SETTINGS 9
#define STATE_CONTROLS      10
#define STATE_ACHIEVEMENTS  11
#define STATE_STATS         12

#define STATE_MULTIPLAYER      13
#define STATE_CONNECTING       14
#define STATE_CONNECT_FAILED   15
#define STATE_DOWNLOAD_TERRAIN 16
#define STATE_SLEEP_MP         17
#define STATE_TEXTURE_PACKS    18
#define STATE_CONFLICT_WARNING 19
#define STATE_ERROR_SCREEN     20
#define STATE_YESNO            21

#define MAX_WORLD_SLOTS 12
#define WORLD_NAME_LEN 64
#define WORLD_SEED_LEN 64


/* ------------------------------------------------------------ */
/* Beta-style mobs                                              */
/* ------------------------------------------------------------ */

#define MOB_NONE      0
#define MOB_CHICKEN   1
#define MOB_COW       2
#define MOB_SHEEP     3
#define MOB_WOLF      4
#define MOB_SQUID     5
#define MOB_ZOMBIE    6
#define MOB_SKELETON  7
#define MOB_CREEPER   8
#define MOB_SPIDER    9
#define MOB_SLIME     10
#define MOB_PIG       11

/* Clean-room mob target set identified from uploaded source/resource names:
   chicken, cow, pig, sheep, wolf, squid, zombie, skeleton, creeper, spider, slime.
   Java behavior names were used as feature references; runtime remains C/OpenGL 1.1 for Open Watcom. */

/* ------------------------------------------------------------ */
/* Player settings                                              */
/* ------------------------------------------------------------ */

#define PI 3.1415926535

#define PLAYER_HEIGHT 1.70
#define PLAYER_RADIUS 0.25
#define EYE_HEIGHT    1.45

#define GRAVITY      18.0
#define JUMP_SPEED    7.0
#define MOVE_SPEED    5.0
#define MOUSE_SPEED   0.15

/* V22_PLAYER_CONTROLLER: Java EntityPlayer/MovementInput feel tuning.
   Fixed values are used so Open Watcom/Windows 98 builds stay deterministic. */
#define PLAYER_STEP_HEIGHT_V22       0.56
#define PLAYER_SWEEP_STEP_V22        0.03125
#define PLAYER_SNEAK_SPEED_SCALE_V22 0.32
#define PLAYER_GROUND_ACCEL_V22      18.0
#define PLAYER_AIR_ACCEL_V22          6.0
#define PLAYER_WATER_ACCEL_V22        5.0
#define PLAYER_GROUND_FRICTION_V22   12.0
#define PLAYER_AIR_FRICTION_V22       1.25
#define PLAYER_WATER_FRICTION_V22     5.2
#define PLAYER_WEB_SCALE_V22          0.18
#define PLAYER_LAVA_SCALE_V22         0.34
#define PLAYER_REACH_V22              5.0

/* V39_EXACT_ENTITY_MOVEMENT: Java Entity.java/EntityLiving.java inspired
   movement constants.  The collision core below uses AxisAlignedBB-style
   offset clipping instead of the old small-increment sweep. */
#define ENTITY_V39_EPSILON              0.0000001
#define ENTITY_V39_MAX_COLLISION_BOXES  768
#define ENTITY_V39_PLAYER_STEP_HEIGHT   0.50
#define ENTITY_V39_MOB_STEP_HEIGHT      0.50
#define ENTITY_V39_GRAVITY_PER_SECOND   18.0
#define ENTITY_V39_SAFE_FALL_DISTANCE    3.0
#define ENTITY_V39_WATER_DRAG            0.80
#define ENTITY_V39_LAVA_DRAG             0.50
#define ENTITY_V39_AIR_DRAG              0.91
#define ENTITY_V39_VERTICAL_DRAG         0.98

#define RAY_DISTANCE  6.0
#define RAY_STEP      0.10

#define HAND_SWING_TIME 0.28
#define HAND_USE_TIME   0.20

/*
    MP3 music files. These are low-bitrate MP3s, not WAV files.
    They are played through Win32 MCI/winmm.
*/
#define MUSIC_MENU_FILE  "assets\\music\\menu_oxygene.mp3"
#define MUSIC_GAME_FILE1 "assets\\music\\game_haggstrom.mp3"
#define MUSIC_GAME_FILE2 "assets\\music\\game_aria_math.mp3"
#define MUSIC_GAME_FILE3 "assets\\music\\game_track3.mp3"
#define MUSIC_GAME_FILE4 "assets\\music\\game_track4.mp3"
#define MUSIC_GAME_FILE5 "assets\\music\\game_track5.mp3"
#define MUSIC_GAME_FILE6 "assets\\music\\game_track6.mp3"
#define MUSIC_GAME_FILE7 "assets\\music\\game_track7.mp3"
#define MUSIC_GAME_FILE8 "assets\\music\\game_track8.mp3"
#define MUSIC_GAME_FILE9 "assets\\music\\calm1.mp3"
#define MUSIC_GAME_FILE10 "assets\\music\\calm2.mp3"
#define MUSIC_GAME_FILE11 "assets\\music\\calm3.mp3"
#define MUSIC_GAME_FILE12 "assets\\music\\hal1.mp3"
#define MUSIC_GAME_FILE13 "assets\\music\\piano1.mp3"
#define MUSIC_GAME_FILE14 "assets\\music\\hal2.mp3"
#define MUSIC_GAME_FILE15 "assets\\music\\hal3.mp3"
#define MUSIC_GAME_FILE16 "assets\\music\\hal4.mp3"
#define MUSIC_GAME_FILE17 "assets\\music\\flake.mp3"
#define GAME_MUSIC_MIN_SECONDS 45.0
#define GAME_MUSIC_COUNT 17
#define SOUND_PLAYER_HIT_FILE "assets\\sounds\\player_hit.mp3"
#define SOUND_UI_CLICK_FILE   "assets\\sounds\\ui_click.mp3"
#define SOUND_PICKUP_FILE     "assets\\sounds\\random\\pop.mp3"

/* V35_SOUND_SYSTEM_ACCURACY: Java SoundManager/SoundPool-style audio layer.
   MCI does volume reliably; pitch is attempted via speed where supported and
   otherwise approximated with Java-like randomized sound variants. */
#define SOUND_CHANNEL_COUNT_V35 48
#define SOUND_DEFAULT_RANGE_V35 16.0
#define SOUND_LOUD_RANGE_V35 48.0
#define SOUND_STEP_RANGE_V35 18.0
#define SOUND_MAX_VOLUME_MCI_V35 1000
#define SOUND_MUSIC_MIN_DELAY_V35 120.0
#define SOUND_MUSIC_RANDOM_DELAY_V35 360.0
#define SOUND_CAVE_MIN_DELAY_V35 20.0
#define SOUND_CAVE_RANDOM_DELAY_V35 55.0

#define SOUND_GROUP_STONE_V35  0
#define SOUND_GROUP_WOOD_V35   1
#define SOUND_GROUP_SAND_V35   2
#define SOUND_GROUP_SNOW_V35   3
#define SOUND_GROUP_CLOTH_V35  4
#define SOUND_GROUP_GRASS_V35  5
#define SOUND_GROUP_GRAVEL_V35 6
#define SOUND_GROUP_GLASS_V35  7
#define SOUND_GROUP_METAL_V35  8

/* ------------------------------------------------------------ */
/* Inventory slot structure                                     */
/* ------------------------------------------------------------ */

typedef struct InventorySlot {
    int item;
    int count;
    int damage;
} InventorySlot;

typedef struct LightNode {
    short x;
    short y;
    short z;
} LightNode;

typedef struct Particle {
    float x;
    float y;
    float z;
    float vx;
    float vy;
    float vz;
    float life;
    float maxLife;
    int block;
    int type;
    int variant;
    float size;
    float gravity;
    float age;
    int noClip;
} Particle;


typedef struct Mob {
    int active;
    int type;
    int health;
    int angry;
    int sheared;
    int burning;
    double x;
    double y;
    double z;
    double vx;
    double vy;
    double vz;
    double yaw;
    double thinkTimer;
    double soundTimer;
    double attackTimer;
    double burnTimer;
    double fleeTimer;
    double fuseTimer;
    double stepTimer;
    double spawnGraceTimer;
    double animWalk;
    double targetX;
    double targetZ;
    double pathTimer;
    double hurtTime;
    double deathTime;
    int deathDropsDone;
    double renderYawOffset;
    double prevX;
    double prevY;
    double prevZ;
    double prevYaw;
    double prevRenderYawOffset;
    double prevAnimWalk;
    int targetKind;
    int pathLength;
    int pathIndex;
    int pathNodeX[MOB_PATH_MAX_POINTS];
    int pathNodeY[MOB_PATH_MAX_POINTS];
    int pathNodeZ[MOB_PATH_MAX_POINTS];
    int lastPathGoalX;
    int lastPathGoalY;
    int lastPathGoalZ;
    int inWater;
    int inLava;
    int onGround;
    double pathRecalcTimer;
    double jumpDelay;
    double targetTimer;
    double despawnTimer;
    double stuckTimer;
    double lastMoveX;
    double lastMoveZ;
    int onLadder;
    int inWeb;
    int inFire;
    double fallDistance;
    double airTimer;
    double drownTimer;
    double fireTimer;
    double pushTimer;
    double hurtResistantTime;
    double attackCooldown;
    double targetLostTimer;
    double lineOfSightTimer;
    int lastTargetVisible;
    double pathFailTimer;
    double entityAge;
    double knockbackTimer;
    double knockbackX;
    double knockbackY;
    double knockbackZ;
    int persistent;
} Mob;

typedef struct DroppedItem {
    int active;
    int item;
    int count;
    int damage;
    int health;
    double x;
    double y;
    double z;
    double prevX;
    double prevY;
    double prevZ;
    double vx;
    double vy;
    double vz;
    double age;
    double spin;
    double hoverStart;
    double pickupDelay;
} DroppedItem;

typedef struct PickupFxV38 {
    int active;
    int item;
    int damage;
    int count;
    double sx;
    double sy;
    double sz;
    double age;
    double duration;
    double hoverStart;
} PickupFxV38;

/* MOB_MODEL_AI_V4: Java-style projectile entity used by skeleton-like mobs.
   Kept fixed-size for Open Watcom/Windows 98 compatibility. */
typedef struct MobProjectile {
    int active;
    int type;
    int shooterType;
    double x;
    double y;
    double z;
    double vx;
    double vy;
    double vz;
    double yaw;
    double pitch;
    double age;
    double life;
} MobProjectile;

typedef struct WorldSaveInfo {
    int exists;
    char name[WORLD_NAME_LEN];
    char seedText[WORLD_SEED_LEN];
    int seed;
    int worldSize;
    double playerGlobalX;
    double playerY;
    double playerGlobalZ;
    double worldTime;
} WorldSaveInfo;


typedef struct TileEntityState {
    int active;
    int type;
    int x;
    int y;
    int z;
    InventorySlot slots[27];
    double burnTime;
    double cookTime;
    char text[64];
    int power;
} TileEntityState;

typedef struct SimpleRecipe {
    int width;
    int height;
    int in[9];
    int outItem;
    int outCount;
    int outDamage;
    int shapeless;
} SimpleRecipe;

typedef struct LocalNetEvent {
    int active;
    int kind;
    int a;
    int b;
    int c;
    double timer;
} LocalNetEvent;

/* CLONEMC_RENDER_REVAMP_PATCH: cached chunk-batch, LOD mob, particle-culling, and fog-distance performance pass. */

/* ------------------------------------------------------------ */
/* Global variables                                             */
/* ------------------------------------------------------------ */

HINSTANCE g_hInst;
HWND g_hwnd;
HDC g_hdc;
HGLRC g_glrc;

int g_running = 1;
int g_state = STATE_MENU;
int g_optionsReturnState = STATE_MENU;

/* Runtime seed. Each saved world can use a different personal seed. */
int g_worldSeed = DEFAULT_WORLDGEN_SEED;
int g_worldSizeBlocks = WORLD_SIZE_INFINITE;
int g_createWorldSizeBlocks = WORLD_SIZE_INFINITE;

int g_windowWidth = WINDOW_WIDTH;
int g_windowHeight = WINDOW_HEIGHT;

/* F11 fullscreen state. Uses old SetWindowLong/SetWindowPos calls so it
   remains compatible with Open Watcom and Win98-era Win32 headers. */
int g_isFullscreen = 0;
RECT g_windowedRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
DWORD g_windowedStyle = WS_OVERLAPPEDWINDOW;

/* Water movement tuning based on the old entity water drag idea: movement is
   damped in water, gravity is small, and jump adds upward swim velocity. */
#define WATER_SWIM_UP_ACCEL       7.25
#define WATER_SWIM_UP_MIN_SPEED   1.15
#define WATER_SWIM_UP_MAX_SPEED   3.45
#define WATER_DOWN_MAX_SPEED      2.35
#define WATER_SURFACE_BUOYANCY    0.00
#define WATER_PASSIVE_SINK_ACCEL  1.85
#define WATER_HORIZONTAL_SCALE    0.62

/* Java-inspired lightweight liquid flow.  This keeps the old overlay-only
   underwater look, but lets water blocks behave more like a simple liquid:
   fall downward first, then spread sideways at the waterline.  The update is
   intentionally budgeted so Win98/Open Watcom builds do not lag. */
#define WATER_FLOW_INTERVAL_SECONDS 0.55
#define WATER_FLOW_RADIUS_BLOCKS    8
#define WATER_FLOW_SCAN_UP          5
#define WATER_FLOW_SCAN_DOWN        8
#define WATER_FLOW_MAX_CHANGES      12

/* V42_FLUID_FIRE_LIGHTING:
   Java reference path: BlockFluid.java, BlockFlowing.java,
   BlockStationary.java, BlockFire.java, EnumSkyBlock.java,
   MetadataChunkBlock.java, World.java and RenderBlocks.java.

   Fluid metadata follows the old Java idea:
   0 = source, 1..7 = horizontal decay, bit 3 set = falling fluid.
   Work is scanned/budgeted around the player so Win98/OpenGL 1.1 builds do
   not stall when a bucket is placed or fire spreads. */
#define FLUID_V42_FALLING_FLAG       8
#define FLUID_V42_LEVEL_MASK         7
#define FLUID_V42_MAX_DECAY          7
#define FLUID_V42_WATER_TICK         0.25
#define FLUID_V42_LAVA_TICK          1.50
#define FLUID_V42_FIRE_TICK          2.00
#define FLUID_V42_SCAN_RADIUS        18
#define FLUID_V42_SCAN_UP            10
#define FLUID_V42_SCAN_DOWN          18
#define FLUID_V42_WATER_BUDGET       64
#define FLUID_V42_LAVA_BUDGET        20
#define FLUID_V42_FIRE_BUDGET        32
#define FLUID_V42_LIGHT_BUDGET       4
#define LIGHT_V42_QUEUE_MAX          256

typedef struct LightDirtyV42 {
    short x;
    short y;
    short z;
    short radius;
} LightDirtyV42;


/* V48_REAL_LIGHTING_ARRAYS:
   Java reference path: EnumSkyBlock.java, MetadataChunkBlock.java,
   World.java and RenderBlocks.java.

   V48 replaces the old one-shot local relight behavior with a Java-style dirty
   light region queue: separate Sky and Block light passes, block opacity/source
   rules, daylight subtraction, and smooth RenderBlocks corner brightness. */
#define ENUM_SKY_BLOCK_SKY_V48       0
#define ENUM_SKY_BLOCK_BLOCK_V48     1
#define LIGHT_V48_QUEUE_MAX          512
#define LIGHT_V48_MAX_REGION_VOLUME  32768
#define LIGHT_V48_MAX_BOX_SPAN       34
#define LIGHT_V48_DAY_SUBTRACT_MAX   11

typedef struct MetadataChunkBlockV48 {
    unsigned char type;
    short minX;
    short minY;
    short minZ;
    short maxX;
    short maxY;
    short maxZ;
} MetadataChunkBlockV48;


/* V52_PRIORITY4_SCHEDULED_TICKS:
   Java reference path: World.scheduleBlockUpdate, NextTickListEntry,
   Block.updateTick, BlockFlowing/BlockFire/BlockCrops/BlockRedstone* update
   timing.  The old V42 code scanned a player-centered cube for every water,
   lava and fire update.  V52 keeps an explicit duplicate-suppressed tick queue
   so only blocks that actually need work are updated. */
#define SCHEDULED_TICK_V52_MAX              4096
#define SCHEDULED_TICK_V52_SECOND           0.05
#define SCHEDULED_TICK_V52_FRAME_BUDGET     256
#define SCHEDULED_TICK_V52_FRAME_BUDGET_SMALL 96
#define SCHEDULED_TICK_V52_SEED_BUDGET      160
#define SCHEDULED_TICK_V52_SEED_RADIUS      20
#define SCHEDULED_TICK_V52_KIND_NORMAL      0
#define SCHEDULED_TICK_V52_KIND_FLUID       1
#define SCHEDULED_TICK_V52_KIND_FIRE        2
#define SCHEDULED_TICK_V52_KIND_REDSTONE    3
#define SCHEDULED_TICK_V52_KIND_PLANT       4

typedef struct ScheduledTickV52 {
    unsigned char active;
    unsigned char kind;
    unsigned short seq;
    short x;
    short y;
    short z;
    short block;
    double dueTime;
} ScheduledTickV52;

GLuint texHeartFull = 0;
GLuint texHeartEmpty = 0;

GLuint texMobChicken = 0;
GLuint texMobCow = 0;
GLuint texMobSheep = 0;
GLuint texMobSheepFur = 0;
GLuint texMobWolf = 0;
GLuint texMobWolfAngry = 0;
GLuint texMobWolfTame = 0;
GLuint texMobSquid = 0;
GLuint texMobZombie = 0;
GLuint texMobSkeleton = 0;
GLuint texMobCreeper = 0;
GLuint texMobSpider = 0;
GLuint texMobSpiderEyes = 0;
GLuint texMobSlime = 0;
GLuint texMobPig = 0;
GLuint texMobPlayer = 0;


/* Extra texture handles loaded from folders that were present in the uploaded texture zip but not wired previously. */
GLuint texCompatTitleLogo = 0;
GLuint texCompatTitleBlack = 0;
GLuint texCompatTitleMojang = 0;
GLuint texCompatAchievementBg = 0;
GLuint texCompatAchievementIcons = 0;
GLuint texCompatArtKz = 0;
GLuint texCompatItemBoat = 0;
GLuint texCompatItemCart = 0;
GLuint texCompatItemDoor = 0;
GLuint texCompatItemSign = 0;
GLuint texCompatArmorLeather1 = 0;
GLuint texCompatArmorLeather2 = 0;
GLuint texCompatArmorChain1 = 0;
GLuint texCompatArmorChain2 = 0;
GLuint texCompatArmorIron1 = 0;
GLuint texCompatArmorIron2 = 0;
GLuint texCompatArmorGold1 = 0;
GLuint texCompatArmorGold2 = 0;
GLuint texCompatArmorDiamond1 = 0;
GLuint texCompatArmorDiamond2 = 0;

int playerHealth = MAX_HEALTH;
int playerPrevHealth = MAX_HEALTH;
double playerHeartsLife = 0.0;
int g_healthHudCounter = 0;

/* Player damage state.  Open Watcom needs these globals declared before TakeDamage(). */
double damageCooldown = 0.0;
double playerHurtFlash = 0.0;

/*
    The world is stored as a 3D array.

    world[x][y][z]

    x = east/west
    y = height
    z = north/south

    Every cell stores one block ID.
*/
int world[WORLD_X][WORLD_Y][WORLD_Z];

/*
    Legacy lighting arrays.
    skyLight stores sunlight, blockLight stores artificial light.
    Both are grayscale 0-15 values.
*/
unsigned char skyLight[WORLD_X][WORLD_Y][WORLD_Z];
unsigned char blockLight[WORLD_X][WORLD_Y][WORLD_Z];

/*
    Infinite world window state.
    world[][][] stores only the currently loaded chunk window.
    worldOriginBlockX/Z tells which global block coordinate local 0,0 maps to.
*/
int columnTop[WORLD_X][WORLD_Z];
int worldHeightMap[WORLD_X][WORLD_Z];
unsigned char biomeMap[WORLD_X][WORLD_Z];
int worldOriginBlockX = -(WORLD_X / 2);
int worldOriginBlockZ = -(WORLD_Z / 2);
int worldCenterChunkX = 0;
int worldCenterChunkZ = 0;

/* 20-minute day/night cycle and beta-style brightness/gamma. */
double g_worldTimeSeconds = 300.0;
float g_daySkyBrightness = 1.0f;
float g_dayNightBlend = 1.0f;
float g_gammaBoost = 1.0f;
int g_highGamma = 0;

/*
    Runtime performance options.  The old renderer was trying to draw too
    many block interiors every frame.  These values keep the feel of a Beta
    render-distance option but default to a fast 2-chunk visible radius.
*/
int g_renderDistanceChunks = 1;
/* V13B: user-selected render distance is never auto-reduced by the FPS governor.
   Low-lag mode still keeps mesh rebuilds, mob caps, particles, and weather cheap,
   but if the player chooses 6 chunks the renderer keeps the 6-chunk radius. */
int g_renderDistancePinnedV13B = 1;
int g_autoPerfMayLowerRenderDistanceV13B = 0;
double g_damageWobbleTimer = 0.0;
double g_damageWobbleStrength = 0.0;
double g_damageAttackedYaw = 0.0;

Particle particles[MAX_PARTICLES];
Mob mobs[MAX_MOBS];
MobProjectile mobProjectiles[MAX_MOB_PROJECTILES];
DroppedItem droppedItems[MAX_DROPPED_ITEMS];
PickupFxV38 pickupFxV38[MAX_PICKUP_FX_V38];
TileEntityState tileEntities[MAX_TILE_ENTITIES];
SimpleRecipe g_simpleRecipes[MAX_SIMPLE_RECIPES];
int g_simpleRecipeCount = 0;
LocalNetEvent g_localNetEvents[MAX_NET_EVENTS];
unsigned char g_redstonePower[WORLD_X][WORLD_Y][WORLD_Z];
unsigned char g_blockMeta[WORLD_X][WORLD_Y][WORLD_Z];
double g_weatherTimer = 0.0;
double g_rainStrength = 0.0;
double g_weatherParticleAccumV24 = 0.0;
float g_weatherWindX_V24 = 0.0f;
float g_weatherWindZ_V24 = 0.0f;
float g_vertexTintR = 1.0f;
float g_vertexTintG = 1.0f;
float g_vertexTintB = 1.0f;
int g_containerModeV5 = 0;
int g_activeTileIndexV5 = -1;
int g_signEditCursorV5 = 0;
double g_tileTickAccumV5 = 0.0;

typedef struct MinecartV5 {
    int active;
    double x;
    double y;
    double z;
    double vx;
    double vz;
} MinecartV5;


#define MAX_MINECARTS_V5 8
MinecartV5 g_minecartsV5[MAX_MINECARTS_V5];

/* ------------------------------------------------------------ */
/* Items/tools/combat/projectiles/vehicles V6                   */
/* ------------------------------------------------------------ */
#define MAX_SPECIAL_ENTITIES_V6 72
#define ENTITY_V6_NONE 0
#define ENTITY_V6_ARROW 1
#define ENTITY_V6_EGG 2
#define ENTITY_V6_SNOWBALL 3
#define ENTITY_V6_FIREBALL 4
#define ENTITY_V6_FISH_HOOK 5
#define ENTITY_V6_BOAT 6
#define ENTITY_V6_PAINTING 7
#define ENTITY_V6_TNT 8
#define ENTITY_V6_FALLING_BLOCK 9
#define ENTITY_V6_LIGHTNING 10

#define ARMOR_SLOT_HEAD_V6 0
#define ARMOR_SLOT_CHEST_V6 1
#define ARMOR_SLOT_LEGS_V6 2
#define ARMOR_SLOT_FEET_V6 3

#define TOOL_KIND_NONE_V6 0
#define TOOL_KIND_PICKAXE_V6 1
#define TOOL_KIND_AXE_V6 2
#define TOOL_KIND_SHOVEL_V6 3
#define TOOL_KIND_HOE_V6 4
#define TOOL_KIND_SWORD_V6 5
#define TOOL_KIND_BOW_V6 6
#define TOOL_KIND_SHEARS_V6 7

#define ITEM_PAINTING 321
#define ITEM_FIRE_CHARGE_PLACEHOLDER 9002

#ifndef ITEM_LAVA_BUCKET
#define ITEM_LAVA_BUCKET 327
#endif

typedef struct SpecialEntityV6 {
    int active;
    int type;
    int item;
    int block;
    int damage;
    int stuck;
    int meta;
    double x;
    double y;
    double z;
    double vx;
    double vy;
    double vz;
    double yaw;
    double pitch;
    double age;
    double life;
    double fuse;
} SpecialEntityV6;

SpecialEntityV6 g_specialEntitiesV6[MAX_SPECIAL_ENTITIES_V6];
InventorySlot g_armorSlotsV6[4];
int g_bowChargingV6 = 0;
double g_bowChargeV6 = 0.0;
int g_miningX_V6 = -9999;
int g_miningY_V6 = -9999;
int g_miningZ_V6 = -9999;
double g_miningProgressV6 = 0.0;

/* V11: held-left-mouse mining state.  Classic Java-style mining is not
   a one-click block deletion: holding attack advances damage over time,
   the block shows a crack texture, and releasing/cancelling resets the
   current block damage. */
int g_leftMouseMiningHeldV11 = 0;
double g_leftMouseMineAccumulatorV11 = 0.0;
double g_leftMouseMineFxTimerV11 = 0.0;
int g_leftMouseMineHadTargetV11 = 0;
double g_leftMouseContinuousAttackCooldownV28 = 0.0;
#define PLAYER_CONTINUOUS_ATTACK_DELAY_V28 0.44
#define PLAYER_CONTINUOUS_AIR_RETRY_V28 0.08
int g_fishingHookIndexV6 = -1;
int g_mapMadeV6 = 0;
int g_mapCenterXGlobalV6 = 0;
int g_mapCenterZGlobalV6 = 0;
double g_lightningTimerV6 = 18.0;


#define CONTAINER_NONE_V5 0
#define CONTAINER_CHEST_V5 1
#define CONTAINER_FURNACE_V5 2
#define CONTAINER_DISPENSER_V5 3
#define CONTAINER_SIGN_V5 4
#define CONTAINER_NOTE_V5 5

int g_cameraMode = CAMERA_FIRST_PERSON;
int g_currentFPS = 0;

/* Player walking/footstep sound state. */
double g_playerStepSoundTimer = 0.0;
int g_playerStepSoundPhase = 0;
int g_fpsFrameCounter = 0;
double g_fpsTimer = 0.0;
double g_mobUpdateAccumulator = 0.0;
/* V32_MOB_SMOOTH: Java-style partial-tick interpolation for mobs.
   The previous V31 build only ran mob AI roughly every 0.15 seconds, which
   made entity movement visibly jumpy.  Java keeps prev/current entity state and
   the renderer blends using partial ticks.  We do the same here while keeping
   the AI on a bounded 20 Hz fixed step for Windows 98-era machines. */
#define MOB_LOGIC_STEP_SECONDS 0.05
#define MOB_LOGIC_MAX_STEPS_PER_FRAME 4
double g_mobRenderPartialTicks = 0.0;
int g_nextMobSoundAlias = 0;
double g_mobSpawnTimer = 0.0;
float g_mobTintR = 1.0f;
float g_mobTintG = 1.0f;
float g_mobTintB = 1.0f;
double g_worldAutosaveTimer = 0.0;
double g_lastPickupSoundTime = -10.0;

/* V35_SOUND_SYSTEM_ACCURACY runtime state. */
double g_soundVolumeV35 = 1.0;
double g_musicVolumeV35 = 1.0;
int g_soundPlayedThisFrameV35 = 0;
int g_soundFailedThisFrameV35 = 0;
int g_soundVariantSeedV35 = 1;
double g_nextCaveAmbientTimeV35 = 35.0;
double g_waterAmbienceCooldownV35 = 0.0;
double g_lavaAmbienceCooldownV35 = 0.0;
double g_fireAmbienceCooldownV35 = 0.0;

/* V37_FULL_ITEM_USE: Java Item*.java style right-click/use state. */
double g_itemUseCooldownV37 = 0.0;
int g_lastFishCatchReadyV37 = 0;


/* Chunk mesh cache for OpenGL 1.1 display lists.
   The world is still the CloneMC finite 862x862 design, but the visible
   128x128 active window is split into 16x16 meshes.  Each chunk is compiled
   once and reused until blocks/light/streaming change, instead of re-emitting
   every block face every frame. */
GLuint terrainChunkLists[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
unsigned char terrainChunkDirty[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
int terrainChunkMeshOriginX = 2147483000;
int terrainChunkMeshOriginZ = 2147483000;
int g_chunkListsBuiltThisFrame = 0;
int g_breakSoundPhase = 0;

/* Runtime render budget. The renderer keeps
   world geometry in cached chunk batches and rebuilds only a small number of
   dirty batches per frame; these knobs let CloneMC do the same while staying
   OpenGL 1.1/Open Watcom friendly. */
int g_chunkMeshBuildBudget = 8;
int g_chunkMeshShellNear = WORLD_Y - 2;
int g_chunkMeshShellFar = WORLD_Y - 2;
int g_legacyPerformanceModeV13 = 1;
int g_fastWorldGenV13 = 1;
int g_skipWeatherRenderV13 = 1;


/* ------------------------------------------------------------ */
/* RENDER_PIPELINE_V8 globals                                   */
/* Java renderer-stack inspired pass: Tessellator, RenderManager,
   RenderGlobal/WorldRenderer sorting, translucent pass, selection
   outline, break-crack overlay, item/entity render dispatch, and
   weather/cloud helpers.  The code stays C89/OpenGL 1.1 friendly for
   Open Watcom and Windows 98/10. */
#define RENDER_V8_MAX_DISPATCH 12
#define RENDER_V8_MAX_SORTED_CHUNKS (WORLD_CHUNKS_X * WORLD_CHUNKS_Z)
#define RENDER_V8_TRANS_RADIUS 24
#define RENDER_V8_RAIN_COLUMNS 16

typedef void (*RendererV8SpecialRenderFn)(SpecialEntityV6 *e);

typedef struct RendererV8DispatchEntry {
    int type;
    RendererV8SpecialRenderFn renderFn;
} RendererV8DispatchEntry;

typedef struct RendererV8ChunkSortEntry {
    int cx;
    int cz;
    int distSq;
} RendererV8ChunkSortEntry;

typedef struct RendererV8Stats {
    long solidChunks;
    long transparentFaces;
    long specialBlocks;
    long renderedEntities;
    long weatherQuads;
    long tessVertices;
    long tessFlushes;
} RendererV8Stats;

RendererV8DispatchEntry g_renderDispatchV8[RENDER_V8_MAX_DISPATCH];
int g_renderDispatchCountV8 = 0;
RendererV8Stats g_rendererStatsV8;
int g_tessellatorActiveV8 = 0;
GLuint g_tessellatorTextureV8 = 0;
int g_rendererV8Initialized = 0;
int g_rendererV8SkipTranslucentInSolidPass = 1;
int g_rendererV8BreakingOverlayEnabled = 1;
int g_rendererV8SelectionOutlineEnabled = 1;
int g_mobFullModelDistanceBlocks = 20;
int g_particleCullDistanceBlocks = 56;

/* ------------------------------------------------------------ */
/* RENDER_PIPELINE_V33 globals                                  */
/* Java RenderGlobal/WorldRenderer/Tessellator/RenderSorter/    */
/* Frustrum inspired performance pass.  This remains original C  */
/* code for Open Watcom + OpenGL 1.1: display-list chunk caches, */
/* conservative frustum checks, bounded mesh rebuilds, far-to-   */
/* near translucent pass ordering, entity/particle throttles,    */
/* and HUD profiling.                                           */
#define RENDER_V33_MAX_CHUNK_QUEUE (WORLD_CHUNKS_X * WORLD_CHUNKS_Z)
#define RENDER_V33_NEAR_BUILD_RADIUS_CHUNKS 2
#define RENDER_V33_PARTICLE_BUDGET_HIGH 128
#define RENDER_V33_PARTICLE_BUDGET_LOW 48

typedef struct RendererV33ChunkEntry {
    int cx;
    int cz;
    int distSq;
    float dot;
} RendererV33ChunkEntry;

typedef struct RendererV33FrameProfile {
    long chunksConsidered;
    long chunksFrustumCulled;
    long chunksDrawn;
    long chunksBuilt;
    long chunksDeferred;
    long chunksDrawnStale;
    long chunksMissingNoBudget;
    long chunksFallbackDrawn;
    long chunksPlaneFrustumCulled;
    long transparentChunks;
    long transparentBlocks;
    long transparentListsBuilt;
    long transparentListsDrawn;
    long transparentListsDeferred;
    long specialChunksScanned;
    long specialBlocksDrawn;
    long mobsRendered;
    long mobsDistanceCulled;
    long mobsFrustumCulled;
    long particlesSpawned;
    long particlesSpawnSkipped;
    long particlesDrawn;
    long particlesDistanceCulled;
    long particlesFrustumCulled;
    long drawCalls;
    double meshBuildMs;
    double solidPassMs;
    double translucentPassMs;
    double specialPassMs;
} RendererV33FrameProfile;

RendererV33FrameProfile g_renderProfileV33;
RendererV33ChunkEntry g_renderOpaqueQueueV33[RENDER_V33_MAX_CHUNK_QUEUE];
RendererV33ChunkEntry g_renderTransQueueV33[RENDER_V33_MAX_CHUNK_QUEUE];
int g_renderOpaqueCountV33 = 0;
int g_renderTransCountV33 = 0;
unsigned short terrainChunkDirtyAgeV33[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
unsigned char terrainChunkWasVisibleV33[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
unsigned long g_rendererFrameIdV33 = 0;
int g_debugRenderProfileV33 = 1;

/* V41_FIX: RenderGlobal/WorldRenderer style adaptive chunk drawing.
   These knobs keep render distance above Tiny without making OpenGL 1.1 /
   Windows 98-era drivers compile every chunk display list in one frame. */
int g_renderV41SurfaceShellFarChunks = 1;
int g_renderV41FullMeshRadiusChunks = 2;
int g_renderV41FarShellDepth = 28;
int g_renderV41DisableAutoDistanceDrop = 1;

/* V47_RENDERER_CHUNK_PERFORMANCE: closer RenderGlobal/WorldRenderer pass.
   Solid and translucent chunk geometry are now independent cached display
   lists, built from dirty flags and queued nearest/farthest by render pass.
   This follows the Java split of skipRenderPass[0]/skipRenderPass[1] while
   keeping the C/OpenGL 1.1 code usable on Windows 98-era drivers. */
GLuint terrainChunkTransLists[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
unsigned char terrainChunkTransDirty[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
unsigned char terrainChunkSkipPassSolidV47[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
unsigned char terrainChunkSkipPassTransV47[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
unsigned char terrainChunkHasSpecialV47[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
unsigned long terrainChunkLastSolidBuildFrameV47[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
unsigned long terrainChunkLastTransBuildFrameV47[WORLD_CHUNKS_X][WORLD_CHUNKS_Z];
float g_frustumPlanesV47[6][4];
int g_frustumValidV47 = 0;
int g_renderV47UsePlaneFrustum = 1;
int g_renderV47MaxBuildMs = 7;
int g_renderV47TransBuildBudget = 2;
int g_renderV47FallbackShell = 1;
int g_renderV47DisableStaleAfterOriginShift = 1;
int g_renderV47InitialBuildBoostFrames = 0;
int g_renderV47SpecialChunkRadius = 3;


/*
    Player position.

    playerY is the player's feet.
    Camera/eye height is playerY + EYE_HEIGHT.
*/
double playerX = 32.0;
double playerY = 12.0;
double playerZ = 32.0;

double velocityX = 0.0;
double velocityY = 0.0;
double velocityZ = 0.0;
double lastVelocityY = 0.0;

int onGround = 0;

/* V22: runtime player controller/environment state. */
int keySneak = 0;
int g_playerSneakingV22 = 0;
int g_playerInWaterV22 = 0;
int g_playerHeadWaterV22 = 0;
int g_playerInLavaV22 = 0;
int g_playerOnLadderV22 = 0;
int g_playerInWebV22 = 0;
int g_playerInFireV22 = 0;
double g_playerFallDistanceV22 = 0.0;
double g_playerFireTickV22 = 0.0;
double g_playerLavaTickV22 = 0.0;
double g_playerUseCooldownV22 = 0.0;
int g_playerRidingMinecartV22 = -1;
double g_playerSleepTimerV22 = 0.0;

/*
    Camera direction.
*/
double yaw = 0.0;
double pitch = 0.0;

double cloudOffsetX = 0.0;
double cloudOffsetZ = 0.0;

/*
    First-person arm animation state.
*/
double handSwingTimer = 0.0;
double handSwingLength = 0.0;
double handUseTimer = 0.0;
double handUseLength = 0.0;
double handBob = 0.0;
int itemRendererHeldItemV38 = ITEM_NONE;
int itemRendererHeldDamageV38 = 0;
int itemRendererHeldSlotV38 = -1;
double itemRendererEquippedProgressV38 = 1.0;
double itemRendererPrevEquippedProgressV38 = 1.0;
double g_playerWalkAmount = 0.0;
double g_playerLastAnimX = 32.0;
double g_playerLastAnimZ = 32.0;

/*
    Event-based keyboard state.
    This avoids the jumpy/sticky feeling from relying only on GetAsyncKeyState.
*/
int keyForward = 0;
int keyBack = 0;
int keyLeft = 0;
int keyRight = 0;
int keyJump = 0;

/*
    Mouse capture state.
    Mouse is hidden and re-centered during gameplay, while the crosshair
    stays virtually fixed at screen center.
*/
int mouseLocked = 0;
int ignoreNextMouseDelta = 1;

/*
    Music state.
    mode: 0 none, 1 menu, 2 gameplay
*/
int musicMode = 0;
int currentGameSong = 0;
double gameMusicMinTimer = 0.0;

DWORD lastTime = 0;

/*
    Beta-style menu, world select, options, and create-world controls.
*/
RECT singleplayerButton;
RECT multiplayerButton;
RECT texturePackButton;
RECT optionsButton;
RECT quitButton;
RECT worldSlotButtons[MAX_WORLD_SLOTS];
RECT worldPlayButton;
RECT worldCreateButton;
RECT worldDeleteButton;
RECT worldBackButton;
RECT createNameField;
RECT createSeedField;
RECT createWorldSizeButton;
RECT createWorldButton;
RECT createCancelButton;
RECT optionsDoneButton;
RECT optionsRenderDistanceButton;
RECT pauseContinueButton;
RECT pauseOptionsButton;
RECT pauseExitButton;
RECT deathRespawnButton;
RECT deathTitleButton;

/* GUI_LIGHT_V7: converted GuiVideoSettings/GuiControls/GuiAchievements/GuiStats/GuiRenameWorld state. */
RECT worldRenameButton;
RECT optionsVideoButton;
RECT optionsControlsButton;
RECT optionsAchievementsButton;
RECT optionsStatsButton;
RECT optionsMusicSlider;
RECT optionsSoundSlider;
RECT videoRenderDistanceButton;
RECT videoBrightnessButton;
RECT videoSmoothLightingButton;
RECT videoCloudsButton;
RECT videoParticlesButton;
RECT videoFogButton;
RECT videoFullscreenButton;
RECT videoDoneButton;
RECT controlsButtons[8];
RECT controlsDoneButton;
RECT renameNameField;
RECT renameDoneButton;
RECT renameCancelButton;
RECT achievementsDoneButton;
RECT statsDoneButton;
char g_renameWorldName[WORLD_NAME_LEN] = "";
int g_videoSmoothLightingV7 = 1;
int g_videoCloudsV7 = 1;
int g_videoParticlesV7 = 1;
int g_videoFogV7 = 1;
int g_guiPendingControlV7 = -1;
WPARAM g_keyBindForwardV7 = 'W';
WPARAM g_keyBindBackV7 = 'S';
WPARAM g_keyBindLeftV7 = 'A';
WPARAM g_keyBindRightV7 = 'D';
WPARAM g_keyBindJumpV7 = VK_SPACE;
WPARAM g_keyBindInventoryV7 = 'E';
WPARAM g_keyBindDropV7 = 'Q';
WPARAM g_keyBindPerspectiveV7 = VK_F5;
unsigned long g_statsBlocksBrokenV7 = 0;
unsigned long g_statsBlocksPlacedV7 = 0;
unsigned long g_statsItemsCraftedV7 = 0;
unsigned long g_statsContainersOpenedV7 = 0;


/* GUI_PIPELINE_V9: Java-style GUI infrastructure converted into C89-compatible code.
   This pass covers GuiChat, multiplayer connection screens, texture packs, text fields,
   slot lists, loading screens, screenshots, scaled resolution, localization, and chat lines. */
#define GUIV9_MAX_TRANSLATIONS 512
#define GUIV9_MAX_TRANSLATION_KEY 72
#define GUIV9_MAX_TRANSLATION_VALUE 192
#define GUIV9_MAX_CHAT_LINES 80
#define GUIV9_CHAT_TEXT_LEN 192
#define GUIV9_MAX_HISTORY 32
#define GUIV9_MAX_PACKS 32
#define GUIV9_PACK_NAME_LEN 80
#define GUIV9_PACK_FILE_LEN 160
#define GUIV9_TEXT_LEN 160
#define GUIV9_MAX_LIST_ROWS 32
#define GUIV9_MAX_LINE_WRAP 8

typedef struct GuiV9Translation_s {
    char key[GUIV9_MAX_TRANSLATION_KEY];
    char value[GUIV9_MAX_TRANSLATION_VALUE];
} GuiV9Translation;

typedef struct GuiV9ChatLine_s {
    char text[GUIV9_CHAT_TEXT_LEN];
    unsigned long timeMs;
    int updateCounter;
} GuiV9ChatLine;

typedef struct GuiV9TextField_s {
    RECT rect;
    char text[GUIV9_TEXT_LEN];
    int maxLen;
    int focused;
    int cursor;
    int selectStart;
    int viewOffset;
    int enabled;
} GuiV9TextField;

typedef struct GuiV9SlotList_s {
    RECT rect;
    int itemHeight;
    int scroll;
    int selected;
    int count;
} GuiV9SlotList;

typedef struct GuiV9ScaledResolution_s {
    int scaleFactor;
    int scaledWidth;
    int scaledHeight;
    int guiLeft;
    int guiTop;
} GuiV9ScaledResolution;

typedef struct GuiV9TexturePack_s {
    char name[GUIV9_PACK_NAME_LEN];
    char file[GUIV9_PACK_FILE_LEN];
    int active;
    int incompatible;
} GuiV9TexturePack;

RECT guiV9BackButton;
RECT guiV9DoneButton;
RECT guiV9CancelButton;
RECT guiV9ConnectButton;
RECT guiV9DirectButton;
RECT guiV9RefreshButton;
RECT guiV9TextureOpenButton;
RECT guiV9YesButton;
RECT guiV9NoButton;
RECT guiV9SleepLeaveButton;
RECT guiV9ErrorBackButton;
RECT guiV9MultiplayerAddressFieldRect;
RECT guiV9ChatFieldRect;
RECT guiV9TextureSlotRect;

GuiV9ScaledResolution g_guiV9Scale;
GuiV9TextField g_guiV9ServerField;
GuiV9TextField g_guiV9ChatField;
GuiV9SlotList g_guiV9TextureSlots;
GuiV9Translation g_guiV9Translations[GUIV9_MAX_TRANSLATIONS];
GuiV9ChatLine g_guiV9ChatLines[GUIV9_MAX_CHAT_LINES];
GuiV9TexturePack g_guiV9TexturePacks[GUIV9_MAX_PACKS];
char g_guiV9ChatHistory[GUIV9_MAX_HISTORY][GUIV9_CHAT_TEXT_LEN];
char g_guiV9StatusMessage[GUIV9_CHAT_TEXT_LEN] = "";
char g_guiV9ErrorTitle[96] = "Error";
char g_guiV9ErrorMessage[GUIV9_CHAT_TEXT_LEN] = "";
char g_guiV9YesNoTitle[96] = "Confirm";
char g_guiV9YesNoLine1[GUIV9_CHAT_TEXT_LEN] = "";
char g_guiV9YesNoLine2[GUIV9_CHAT_TEXT_LEN] = "";
char g_guiV9ConnectAddress[128] = "localhost";
int g_guiV9TranslationCount = 0;
int g_guiV9TranslationLoaded = 0;
int g_guiV9ChatCount = 0;
int g_guiV9ChatOpen = 0;
int g_guiV9HistoryCount = 0;
int g_guiV9HistoryCursor = -1;
int g_guiV9TexturePackCount = 0;
int g_guiV9ActiveTexturePack = 0;
int g_guiV9YesNoAction = 0;
int g_guiV9YesNoArg = 0;
int g_guiV9OptionsReturnState = STATE_MENU;
int g_guiV9DownloadProgress = 0;
unsigned long g_guiV9ConnectStart = 0;
unsigned long g_guiV9LoadingStart = 0;

/* UI/HUD_V16: runtime state for the Java GuiMainMenu/GuiIngame/GuiContainer
   behavior port.  It is deliberately tiny/global because this project is a
   single C file and still targets Open Watcom + Win98-era systems. */
int g_guiV16LastState = -999;
unsigned long g_guiV16FadeStartMs = 0;
int g_guiV16LastInventoryScale = 2;
int g_guiV16LastHoveredButton = -1;

/* UI_CONTAINER_V21: Java InventoryPlayer/Container/GuiScreen cleanup. */
int g_guiV21WorldScroll = 0;
int g_guiV21WorldScrollMax = 0;
int g_guiV21TextSelectStart = -1;
int g_guiV21LastLoadingProgress = 0;
#define GUIV21_WORLD_VISIBLE_SLOTS 5


/* RESOURCE_PACK_V10: Java-inspired texture pack/resource manager.
   Converts the responsibilities of TexturePackBase, TexturePackDefault,
   TexturePackCustom, TexturePackList, TerrainTextureManager, RenderEngine,
   ImageBuffer, ThreadDownloadImage/Data/Resources, OpenGlCapsChecker, and
   GLAllocation into a Win98/OpenWatcom-friendly single-file C system.

   Runtime formats:
   - Loose pack folders are supported directly.
   - Stored/no-compression ZIP packs are supported without zlib.
   - Deflated ZIP entries are detected and skipped to preserve OpenWatcom/Win98
     portability; the loader falls back to default assets instead of crashing.
   - PNG files are kept as source/reference assets, while runtime textures use
     TGA because this C file already has a tiny OpenGL 1.1 TGA loader. */
#define RESOURCEV10_MAX_PATH 260
#define RESOURCEV10_MAX_ALIASES 8
#define RESOURCEV10_MAX_ANIM 32
#define RESOURCEV10_PACK_NONE 0
#define RESOURCEV10_PACK_FOLDER 1
#define RESOURCEV10_PACK_STORED_ZIP 2
#define RESOURCEV10_ZIP_LOCAL_SIG 0x04034b50UL

typedef struct ResourceV10Alias_s {
    const char *logical;
    const char *alias[RESOURCEV10_MAX_ALIASES];
} ResourceV10Alias;

typedef struct ResourceV10AnimatedTexture_s {
    int block;
    int baseCol;
    int baseRow;
    int frames;
    double speed;
    int frame;
    double timer;
} ResourceV10AnimatedTexture;

char g_resourceV10ActivePackPath[RESOURCEV10_MAX_PATH] = "assets";
char g_resourceV10ResolvedPath[RESOURCEV10_MAX_PATH] = "";
char g_resourceV10LastFallback[RESOURCEV10_MAX_PATH] = "";
char g_resourceV10Vendor[96] = "unknown";
char g_resourceV10Renderer[128] = "unknown";
char g_resourceV10Version[64] = "unknown";
int g_resourceV10PackType = RESOURCEV10_PACK_NONE;
int g_resourceV10MaxTextureSize = 0;
int g_resourceV10TexturesReloaded = 0;
int g_resourceV10DeflatedZipMisses = 0;
int g_resourceV10ResolvedFromPack = 0;
ResourceV10AnimatedTexture g_resourceV10Anim[RESOURCEV10_MAX_ANIM];
int g_resourceV10AnimCount = 0;
/* V54 TextureFX/EffectRenderer tuning.  These are lightweight counters used
   by the OpenGL 1.1 atlas-frame animation path.  They do not require shader
   support or runtime PNG decoding, so they stay compatible with Windows 98 and
   Open Watcom. */
double g_textureFxTimeV54 = 0.0;
int g_textureFxWaterFrameV54 = 0;
int g_textureFxLavaFrameV54 = 0;
int g_textureFxFireFrameV54 = 0;
int g_textureFxPortalFrameV54 = 0;

WorldSaveInfo worldSaves[MAX_WORLD_SLOTS];
int selectedWorldSlot = 0;
int currentWorldSlot = -1;
char newWorldName[WORLD_NAME_LEN] = "New World";
char newWorldSeedText[WORLD_SEED_LEN] = "";
int createInputField = 0;
int g_startFromSavedPosition = 0;
int g_regionTilesLoadedV34 = 0;
double g_startGlobalX = 0.0;
double g_startPlayerY = 80.0;
double g_startGlobalZ = 0.0;

/*
    Survival system.
*/
InventorySlot hotbar[HOTBAR_SLOTS];
InventorySlot inventory[INVENTORY_SLOTS];
InventorySlot g_dragSlot;
int g_draggingInventory = 0;

InventorySlot craftGrid[9];
InventorySlot craftResult;
int craftingOpen = 0;
int g_craftingTableX = 0;
int g_craftingTableY = 0;
int g_craftingTableZ = 0;
double g_playerAirTimer = 12.0;
double g_drownDamageTimer = 0.0;
double g_bubbleParticleTimer = 0.0;
double g_waterFlowAccumulator = 0.0;
double g_fluidWaterAccumulatorV42 = 0.0;
double g_fluidLavaAccumulatorV42 = 0.0;
double g_fireAccumulatorV42 = 0.0;
LightDirtyV42 g_lightDirtyQueueV42[LIGHT_V42_QUEUE_MAX];
int g_lightDirtyCountV42 = 0;
MetadataChunkBlockV48 g_lightQueueV48[LIGHT_V48_QUEUE_MAX];
int g_lightQueueCountV48 = 0;
long g_lightV48ScheduledSky = 0;
long g_lightV48ScheduledBlock = 0;
long g_lightV48ProcessedBoxes = 0;
long g_lightV48ChangedCells = 0;
long g_lightV48SkippedLarge = 0;
int g_skyLightSubtractedV48 = 0;
int g_fluidChangesThisTickV42 = 0;
int g_fireChangesThisTickV42 = 0;
ScheduledTickV52 g_scheduledTicksV52[SCHEDULED_TICK_V52_MAX];
int g_scheduledTickCountV52 = 0;
int g_scheduledTickWriteSeqV52 = 0;
double g_scheduledTickSeedTimerV52 = 0.0;
long g_scheduledTickScheduledV52 = 0;
long g_scheduledTickProcessedV52 = 0;
long g_scheduledTickDuplicatesV52 = 0;
long g_scheduledTickDroppedV52 = 0;
long g_scheduledTickSeededV52 = 0;

int selectedHotbarSlot = 0;
int inventoryOpen = 0;

/*
    Bitmap font display lists.
*/
GLuint fontBaseNormal = 0;
GLuint fontBaseTitle = 0;

/*
    Texture atlas IDs.
*/
GLuint texTerrain = 0;
GLuint texIcons = 0;

/* Imported Beta-style visual assets converted from uploaded PNG files. */
GLuint texBetaLogo = 0;
GLuint texBetaMenuBackground = 0;
GLuint texBetaGui = 0;
GLuint texBetaInventory = 0;
GLuint texBetaItems = 0;
GLuint texBetaCrafting = 0;
GLuint texBetaIcons = 0;
GLuint texBetaParticles = 0;
GLuint texBlockBreakV11 = 0;
GLuint texBetaSun = 0;
GLuint texBetaMoon = 0;
GLuint texBetaRain = 0;
GLuint texBetaSnow = 0;
GLuint texBetaShadow = 0;
GLuint texBetaVignette = 0;
GLuint texBetaWater = 0;
GLuint texBetaFont = 0;
int g_weatherMode = 0;
double g_weatherScroll = 0.0;

/* ------------------------------------------------------------ */
/* Function prototypes                                          */
/* ------------------------------------------------------------ */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ToggleFullscreen(void);
void ApplyWindowResizeState(void);

/* Open Watcom compile-fix forward declarations.
   These are intentionally kept near the top of project1new.c so calls added
   by the feature-gap systems do not become implicit int declarations. */
int GetLocalBiome(int x, int z);
int IsTileEntityBlock(int block);
void EnsureSaveDirectory(void);
void RebuildColumnTops(void);
void InvalidateAllTerrainChunkMeshes(void);
int IsInsideWorld(int x, int y, int z);
int GetBlock(int x, int y, int z);
void SetBlock(int x, int y, int z, int block);
int IsSolidBlock(int block);
float MobWidth(int type);
float MobHeight(int type);
int IsPassiveMobType(int type);
int IsHostileMobType(int type);
int IsDaylightForMobs(void);
int IsSkyOpenForSpawn(int bx, int by, int bz);
void SpawnMobEffectParticles(Mob *m, int block, int count);
double MobDistanceSquaredToPlayer(Mob *m);
void TakeDamage(int amount);
void SpawnWaterBubbleParticles(double x, double y, double z, int count);
void PlayMobHurtSoundNear(Mob *m);
void DropMobLoot(Mob *m);
void PlayOneShotMP3(const char *filename);
int SoundFileExistsV35(const char *filename);
int SoundResolveExistingPathV35(const char *filename, char *outPath);
double SoundRandUnitV35(void);
double SoundRandomRangeV35(double minValue, double maxValue);
void SoundApplyMciControlsV35(const char *aliasName, double volume, double pitch, int isMusic);
void PlaySoundFileExV35(const char *filename, double volume, double pitch, int isMusic);
void PlaySoundAtV35(const char *filename, double x, double y, double z, double volume, double pitch, double maxDistance);
void PlayRandomPathV35(const char *prefix, int minVariant, int maxVariant, const char *suffix, const char *fallback, double volume, double pitch);
void PlayRandomPathAtV35(const char *prefix, int minVariant, int maxVariant, const char *suffix, const char *fallback, double x, double y, double z, double volume, double pitch, double maxDistance);
void PlayBowSoundV35(double x, double y, double z);
void PlayDoorSoundV35(double x, double y, double z, int open);
void PlayChestSoundV35(double x, double y, double z, int open);
void PlayFurnaceSoundV35(double x, double y, double z);
void UpdateAmbientSoundsV35(double dt);
int WorldHash2D(int x, int z, int seed);
int WorldHash3D(int x, int y, int z, int seed);
void GetTileUVEx(int col, int row, int atlasWidth, int atlasHeight, float *u0, float *v0, float *u1, float *v1);
int InitWindow(HINSTANCE hInstance, int nCmdShow);
int InitOpenGL(void);
void LoadGameTextures(void);
void CreateFonts(void);
void DeleteFonts(void);
void DeleteGameTextures(void);
void ShutdownOpenGL(void);

void RendererV8_Init(void);
void RendererV33_ResetFrameProfile(void);
void RendererV33_BeginFrame(void);
int RendererV33_IsChunkInFrustum(int cx, int cz, int distSq, float *outDot);
void RendererV33_BuildChunkQueues(int pcx, int pcz, int renderChunks);
int RendererV33_GetMeshBuildBudget(void);
int RendererV33_CompareNearToFar(const void *a, const void *b);
int RendererV33_CompareFarToNear(const void *a, const void *b);
void RendererV33_DrawProfilerOverlay(void);
void RendererV47_UpdateFrustumFromOpenGL(void);
int RendererV47_IsAABBInFrustum(double minX, double minY, double minZ, double maxX, double maxY, double maxZ);
int RendererV47_IsChunkVisibleAABB(int cx, int cz);
int RendererV47_TimeBudgetAllows(DWORD startMs, int builds, int budget);
void BuildTerrainChunkTransMeshV47(int cx, int cz);
void RendererV47_DrawFallbackChunkShell(int cx, int cz);
int RendererV41_ShouldBuildFullChunk(int cx, int cz);
int RendererV41_GetColumnStartY(int cx, int cz, int topY);
void RenderFirstPersonPlayerArmV41(float swing, float usePush, float equip, float bobX, float bobY);
void InitFeatureGapSystems(void);

void JavaCompat_InitFeatureRegistry(void);
void JavaCompat_LoadLangAndSplashes(void);
void JavaCompat_UpdateFeatureSystems(double dt);
void JavaCompat_DrawSplashText(void);
int JavaCompat_GetFoodHealPoints(int item);
int JavaCompat_GetFuelBurnTicks(int item);
int JavaCompat_IsMappedExtraBlock(int block);
void UpdateFeatureGapSystems(double dt);
void InitRecipeBook(void);
int RecipeBookFindResult(void);
void SaveCurrentTileEntities(void);
int LoadCurrentTileEntities(void);
void SaveCurrentRegionLite(void);
int LoadCurrentRegionLite(void);
int SaveHandler_SaveWorldInfoV2(int slot);
int SaveHandler_LoadWorldInfoV2(int slot, WorldSaveInfo *info);
int SaveHandler_SaveCurrentWorldV2(void);
int SaveHandler_SavePlayerV2(void);
int SaveHandler_LoadPlayerV2(void);
int SaveHandler_SaveEntitiesV2(void);
int SaveHandler_LoadEntitiesV2(void);
int SaveHandler_SaveTileEntitiesV2(void);
int SaveHandler_LoadTileEntitiesV2(void);
int SaveHandler_SaveRegionWindowV2(void);
int SaveHandler_LoadRegionWindowV2(void);
void SaveHandler_DeleteWorldV2(int slot);
void SaveHandler_CreateWorldLayout(int slot);
int WorldIsInfiniteV55(void);

/* V58_PRIORITY3 terrain/worldgen and default texture repair. */
void WorldGenV58_ApplyTerrainSurfacePass(void);
void WorldGenV58_AddBiomeDecorationPass(void);
void WorldGenV58_ApplyFinalTerrainCleanupPass(void);
GLuint ResourceV58_LoadTerrainTexture(void);
int WorldSizeIsFiniteV55(int size);
int WorldGenV55_FindDroppedItemRestY(double x, double y, double z, double *outY);
void WorldGenV55_AddSurfaceVarietyPass(void);
double WorldGenV55_MountainRangeMask(int gx, int gz);
double GetPlayerGlobalX(void);
double GetPlayerGlobalZ(void);
int FloorDivInt(int a, int b);
void InitMobs(void);
int AddMob(int type, double x, double y, double z);
void InitDroppedItems(void);
int AddDroppedItem(int item, int count, double x, double y, double z, double vx, double vy, double vz);
int AddDroppedItemStackV38(int item, int count, int damage, double x, double y, double z, double vx, double vy, double vz);
int AddItemToInventoryWithDamageV38(int item, int count, int damage);
void InitPickupFxV38(void);
void SpawnPickupFxV38(DroppedItem *d);
void UpdatePickupFxV38(double dt);
void RenderPickupFxV38(void);
void ItemRendererV38_UpdateEquippedItem(double dt);
float ItemRendererV38_GetEquipProgress(void);
void EnsureTileEntityForBlock(int block, int x, int y, int z);
void RemoveTileEntityAt(int x, int y, int z);
void UpdateRedstoneAround(int x, int y, int z);
void UpdateWeatherAdvanced(double dt);
void SpawnWeatherParticles(void);
void MobPathSteer(Mob *m, double desiredX, double desiredZ, double *outX, double *outZ);
void MobInterp_SyncV32(Mob *m);
void MobInterp_BeginStepV32(Mob *m);
double MobInterp_LerpV32(double a, double b, double t);
double MobInterp_LerpAngleV32(double a, double b, double t);
void MobInterp_BuildRenderMobV32(Mob *src, Mob *dst, double alpha);
void MobAI_ResetV17(Mob *m);
void MobAI_RefreshFluidStateV17(Mob *m);
int MobAI_SelectTargetKindV17(Mob *m, double dist2);
int MobAI_AabbBlockedAtV17(int type, double x, double y, double z);
int MobAI_GetCachedPathSteerV17(Mob *m, double goalX, double goalY, double goalZ, double dt, double *outX, double *outZ);
void MobAI_ApplyFluidMotionV17(Mob *m, double dt);
void MobAI_TryJumpHelperV17(Mob *m, double dirX, double dirZ, int mobOnGround);
void MobAI_RefreshEntityStateV18(Mob *m);
void MobAI_ApplyLivingEnvironmentV18(Mob *m, double dt);
void MobAI_ResolveAxisSweepV18(Mob *m, double *newX, double *newY, double *newZ, int *mobOnGround);
void MobAI_ApplyEntityPushesV18(int index, Mob *m, double dt);
void MobAI_DamageSelfV18(Mob *m, int amount, int effectBlock);
int MobAI_FireImmuneV18(int type);
int MobAI_HasLineOfSightToPlayerV24(Mob *m);
int MobAI_SelectTargetKindV24(Mob *m, double dist2, double dt);
int MobAI_BuildPathV24(Mob *m, int goalX, int goalY, int goalZ);
int MobAI_GetCachedPathSteerV24(Mob *m, double goalX, double goalY, double goalZ, double dt, double *outX, double *outZ);
void MobAI_ResolveAxisSweepV24(Mob *m, double *newX, double *newY, double *newZ, int *mobOnGround);
void MobAI_ApplyEntityPushesV24(int index, Mob *m, double dt);
void MobAI_ApplyKnockbackV24(Mob *m, double srcX, double srcZ, double power, double up);
void MobAI_DamageSelfV24(Mob *m, int amount, int effectBlock, double srcX, double srcZ);
int MobAI_AttackPlayerV24(Mob *m, int amount, double cooldown);
int SpawnParticleV24(int ptype, double x, double y, double z, double vx, double vy, double vz, double life, double size, int block);
void SpawnSmokeParticlesV24(double x, double y, double z, int count);
void SpawnFlameParticlesV24(double x, double y, double z, int count);
void SpawnSplashParticlesV24(double x, double y, double z, int count);
void SpawnExplosionParticlesV24(double x, double y, double z, int count);
void KillMobRenderable(Mob *m);
int GetItemMaxDurability(int item);
void PlayItemPickupSound(void);
void PlayPickupSoundThrottled(double x, double y, double z);
void DrawDroppedBlockCube(int block);
void GetBlockTile(int block, int face, int *col, int *row);
void DrawBlock(int x, int y, int z, int block);
void GetBlockTextureTile(int block, int face, int *col, int *row);
void RenderBlockAtV19(int x, int y, int z, int block);
int BlockV43_ConnectsFence(int block);
int BlockV49_GetMaterial(int block);
int BlockV49_IsReplaceable(int block);
int BlockV49_BlocksMovement(int block);
int BlockV49_IsNormalCube(int block);
int BlockV49_IsSolidRenderBlock(int block);
int BlockV49_IsLadderLike(int block);
int BlockV49_IsWebLike(int block);
float BlockV49_GetSlipperiness(int block);
float BlockV49_GetHardness(int block);
int BlockV49_GetHarvestTool(int block);
int BlockV49_GetHarvestLevel(int block);
int BlockV49_GetPrimaryCollisionBounds(int block, int bx, int by, int bz, double *minX, double *minY, double *minZ, double *maxX, double *maxY, double *maxZ);
int BlockV49_ShouldSideBeRendered(int block, int neighbor);
void BlockRegistryV49_ApplyBlockDefaults(void);
int BlockV50_CanBlockStayAt(int block, int x, int y, int z);
int BlockV50_CanPlaceBlockAt(int block, int x, int y, int z, int hitX, int hitY, int hitZ);
void BlockV50_ApplyPlacementMetadata(int block, int x, int y, int z, int hitX, int hitY, int hitZ);
void BlockV50_OnNeighborBlockChange(int x, int y, int z, int changedX, int changedY, int changedZ);
void BlockV50_NotifyNeighborsOfChange(int x, int y, int z, int oldBlock, int newBlock);
int BlockV50_GetDropItemAt(int block, int x, int y, int z, int *countOut);
int BlockV50_IsSupportSolid(int block);
void BlockRegistryV50_ApplyPriority3Overrides(void);
void BlockRegistryV57_ApplyPriority2BlockBehavior(void);
int BlockV57_IsSupportSolid(int block);
int BlockV57_ShouldSideBeRendered(int block, int neighbor);
int BlockV57_IsBlockSelectable(int block);
int BlockV57_GetRenderLayer(int block);
int BlockV57_IsReplaceableAt(int block, int x, int y, int z);
int BlockV52_ShouldUseScheduledTick(int block);
int BlockV52_GetTickDelay(int block);
void ScheduleBlockUpdateV52(int x, int y, int z, int block, int delayTicks);
void ScheduleNeighborBlockUpdatesV52(int x, int y, int z);
void BlockV52_OnBlockChanged(int x, int y, int z, int oldBlock, int newBlock);
void ProcessScheduledBlockTicksV52(double dt, int maxTicks);
void SeedScheduledTicksNearPlayerV52(int maxSeeds);
int GetToolHarvestLevel(int item);
double GetToolMiningSpeed(int item, int block);
int ItemCombatV6_IsSword(int item);
int ItemCombatV6_IsPickaxe(int item);
int ItemCombatV6_IsAxe(int item);
int ItemCombatV6_IsShovel(int item);
int ItemCombatV6_IsHoe(int item);
int ItemCombatV6_IsTool(int item);
void DamageHeldTool(int amount);
void InitItemCombatV6(void);
void UpdateItemCombatV6(double dt);
void BeginHeldBlockMiningV11(void);
void CancelHeldBlockMiningV11(void);
void UpdateHeldBlockMiningV11(double dt);
void FinishMinedBlockV11(int bx, int by, int bz, int block);
void RenderItemCombatV6(void);
void RenderItemCombatV6_Cube(double x, double y, double z, double sx, double sy, double sz, float r, float g, float b);
int ItemCombatV6_TryUseSelectedItem(void);
int ItemCombatV6_GetHeldAttackDamage(void);
int ItemCombatV6_GetHeldAttackToolWear(void);
int ItemCombatV6_ApplyArmorReduction(int amount);
int ItemCombatV6_ShouldFinishMine(int block, int x, int y, int z);
int ItemCombatV6_CanHarvestBlock(int block, int item);
int SpawnSpecialEntityV6(int type, int item, int block, double x, double y, double z, double vx, double vy, double vz, double fuse);
void SpawnExplosionV6(double x, double y, double z, double radius, int fire);
int ItemSpecialV44_UpdateEntity(int index, SpecialEntityV6 *e, double dt);
void SaveSpecialToBytesV44(SpecialEntityV6 *e, unsigned char *p);
int LoadSpecialFromBytesV44(unsigned char *p, int bytes);
int MobAIV44_PlayerHoldsBreedingFood(Mob *m);
int MobAIV44_FindNearestMobType(int type, double x, double y, double z, double maxDist, int skip);
void MobAIV44_OnMobDamaged(Mob *m, double knockX, double knockZ);
void MobAIV44_ApplyAdvancedBehavior(int index, Mob *m, int targetKind, double dist2, double dt, double *targetX, double *targetZ, double *speed);
int MobAIV44_ShouldDespawn(Mob *m, double dist2, double dt);
void UpdateMinecartsV44(double dt);

/* V53 Priority 5/6: deeper EntityCreature/Pathfinder/SpawnerAnimals and RenderLiving helpers. */
int MobAIV53_SelectTargetKind(Mob *m, double dist2, double dt);
void MobAIV53_OnMobDamaged(Mob *m, double knockX, double knockZ);
void MobAIV53_ApplyAdvancedBehavior(int index, Mob *m, int targetKind, double dist2, double dt, double *targetX, double *targetZ, double *speed);
int MobAIV53_ShouldDespawn(Mob *m, double dist2, double dt);
void MobAIV53_RunSpawner(double dt);
int MobAIV53_GetAttackDamage(int type);
void DamageMob(int index, int amount, double knockX, double knockZ);
int MobAIV53_PathToGoal(Mob *m, double goalX, double goalY, double goalZ, double dt, double *outX, double *outZ);
void RenderMobModelJavaV4(Mob *m, GLuint tex, float alpha);
void RenderMobModelJavaV53(Mob *m, GLuint tex, float alpha);
void SpawnFallingBlockIfNeededV6(int x, int y, int z);
int SpawnSkeletonArrowProjectile(Mob *m);
void InitMobProjectiles(void);
void UpdateMobProjectiles(double dt);
void RenderMobProjectiles(void);
