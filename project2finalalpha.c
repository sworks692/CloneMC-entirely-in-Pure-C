/*
    CloneMC Cave Game + Menu + Survival + Texture Atlas Prototype
    -------------------------------------------------------------

    Open Watcom 2.0
    Win32 desktop app
    Classic OpenGL 1.1 immediate mode

    Compile:
        wcl386 -bt=nt -l=nt_win -fe=clonemc.exe project1.c opengl32.lib glu32.lib gdi32.lib user32.lib

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
#define FINITE_WORLD_SIZE_SMALL 862
#define FINITE_WORLD_SIZE_LARGE 862
#define FINITE_WORLD_MAX_SIZE   862
#define FINITE_BORDER_OCEAN_WIDTH 48
#define FINITE_BORDER_CLAMP_PAD 3
#define RENDER_DISTANCE_MIN_CHUNKS 1
#define RENDER_DISTANCE_MAX_CHUNKS 20
#define DAY_LENGTH_SECONDS 1200.0
#define MAX_PARTICLES 32
#define MAX_WEATHER_PARTICLES 2
#define MAX_MOBS 12
#define MOB_SPAWN_RADIUS_MIN 28
#define MOB_SPAWN_RADIUS_MAX 46
#define MOB_SOFT_UPDATE_DISTANCE 14.0
#define MOB_DESPAWN_DISTANCE 128.0
#define MOB_RENDER_DISTANCE 22.0
#define MOB_ATTACK_DISTANCE 2.05
#define MOB_ATTACK_VERTICAL_DISTANCE 2.40
#define MOB_HEAR_IDLE_DISTANCE 32.0
#define MOB_HEAR_STEP_DISTANCE 18.0
#define MOB_HEAR_HURT_DISTANCE 48.0
#define MOB_PASSIVE_CAP 3
#define MOB_HOSTILE_CAP 3
#define MOB_WATER_CAP 1
#define MOB_SPAWN_ATTEMPTS 1
#define MOB_SOUND_ALIAS_COUNT 24
#define MAX_TILE_ENTITIES 192
#define MAX_SIMPLE_RECIPES 96
#define MAX_NET_EVENTS 64
#define PATH_GRID_RADIUS 8
#define PATH_GRID_SIZE 17
#define STREAM_EDGE_MARGIN_BLOCKS (CHUNK_SIZE * 2)
#define FULL_GAMMA_BOOST 1.00f

#define WATER_LEVEL 5
#define RENDER_DISTANCE 40
#define NEAR_BLOCK_RENDER_DISTANCE 22
#define VERY_NEAR_BLOCK_RENDER_DISTANCE 12
#define FAR_COLUMN_RENDER_DISTANCE 48
#define MAX_DROPPED_ITEMS 96
#define CAMERA_FIRST_PERSON 0
#define CAMERA_THIRD_BACK 1
#define CAMERA_THIRD_FRONT 2
#define CLONEMC_VERSION_TEXT "CloneMC Beta-style C 0.8.1 Fast"

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
#define TILE_GRASS_SIDE_COL  1
#define TILE_GRASS_SIDE_ROW  0
#define TILE_GRASS_TOP_COL   2
#define TILE_GRASS_TOP_ROW   0
#define TILE_DIRT_COL        3
#define TILE_DIRT_ROW        0
#define TILE_PLANKS_COL      21
#define TILE_PLANKS_ROW      1
#define TILE_COBBLE_COL      1
#define TILE_COBBLE_ROW      5
#define TILE_BEDROCK_COL     0
#define TILE_BEDROCK_ROW     1
#define TILE_SAND_COL        5
#define TILE_SAND_ROW        1
#define TILE_GRAVEL_COL      2
#define TILE_GRAVEL_ROW      5

/* Tree / nature tiles. */
#define TILE_WOOD_SIDE_COL   3
#define TILE_WOOD_SIDE_ROW   3
#define TILE_WOOD_TOP_COL    4
#define TILE_WOOD_TOP_ROW    3
#define TILE_LEAVES_COL      0
#define TILE_LEAVES_ROW      5
#define TILE_WOOL_COL        22
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
#define TILE_ORE_GOLD_COL      31
#define TILE_ORE_GOLD_ROW      3

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
#define TILE_FURNACE_SIDE_COL 13
#define TILE_FURNACE_SIDE_ROW 4
#define TILE_FURNACE_FRONT_COL 14
#define TILE_FURNACE_FRONT_ROW 4
#define TILE_CHEST_COL       TILE_PLANKS_COL
#define TILE_CHEST_ROW       TILE_PLANKS_ROW

/* Biome / special blocks. */
#define TILE_SNOW_COL        22
#define TILE_SNOW_ROW        0
#define TILE_ICE_COL         13
#define TILE_ICE_ROW         5
#define TILE_CACTUS_TOP_COL     15
#define TILE_CACTUS_TOP_ROW     5
#define TILE_CACTUS_SIDE_COL    16
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
#define BLOCK_SANDSTONE  24
#define BLOCK_WOOL       35
#define BLOCK_DIAMOND_ORE 56
#define BLOCK_TORCH     50
#define BLOCK_CHEST     54
#define BLOCK_WORKBENCH  58
#define BLOCK_FURNACE    61
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

#define MAX_WORLD_SLOTS 5
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
#define GAME_MUSIC_MIN_SECONDS 45.0
#define GAME_MUSIC_COUNT 16
#define SOUND_PLAYER_HIT_FILE "assets\\sounds\\player_hit.mp3"
#define SOUND_UI_CLICK_FILE   "assets\\sounds\\ui_click.mp3"
#define SOUND_PICKUP_FILE     "assets\\sounds\\random\\pop.mp3"

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
    double prevZ;
} Mob;

typedef struct DroppedItem {
    int active;
    int item;
    int count;
    double x;
    double y;
    double z;
    double vx;
    double vy;
    double vz;
    double age;
    double spin;
} DroppedItem;

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
int g_worldSizeBlocks = FINITE_WORLD_SIZE_SMALL;
int g_createWorldSizeBlocks = FINITE_WORLD_SIZE_SMALL;

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
#define WATER_FLOW_INTERVAL_SECONDS 0.22
#define WATER_FLOW_RADIUS_BLOCKS    18
#define WATER_FLOW_SCAN_UP          10
#define WATER_FLOW_SCAN_DOWN        18
#define WATER_FLOW_MAX_CHANGES      44

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
int g_renderDistanceChunks = 2;
double g_damageWobbleTimer = 0.0;
double g_damageWobbleStrength = 0.0;
double g_damageAttackedYaw = 0.0;

Particle particles[MAX_PARTICLES];
Mob mobs[MAX_MOBS];
DroppedItem droppedItems[MAX_DROPPED_ITEMS];
TileEntityState tileEntities[MAX_TILE_ENTITIES];
SimpleRecipe g_simpleRecipes[MAX_SIMPLE_RECIPES];
int g_simpleRecipeCount = 0;
LocalNetEvent g_localNetEvents[MAX_NET_EVENTS];
unsigned char g_redstonePower[WORLD_X][WORLD_Y][WORLD_Z];
double g_weatherTimer = 0.0;
double g_rainStrength = 0.0;
float g_vertexTintR = 1.0f;
float g_vertexTintG = 1.0f;
float g_vertexTintB = 1.0f;
int g_cameraMode = CAMERA_FIRST_PERSON;
int g_currentFPS = 0;

/* Player walking/footstep sound state. */
double g_playerStepSoundTimer = 0.0;
int g_playerStepSoundPhase = 0;
int g_fpsFrameCounter = 0;
double g_fpsTimer = 0.0;
double g_mobUpdateAccumulator = 0.0;
int g_nextMobSoundAlias = 0;
double g_mobSpawnTimer = 0.0;
double g_worldAutosaveTimer = 0.0;
double g_lastPickupSoundTime = -10.0;


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
int g_chunkMeshBuildBudget = 2;
int g_chunkMeshShellNear = 28;
int g_chunkMeshShellFar = 10;
int g_mobFullModelDistanceBlocks = 20;
int g_particleCullDistanceBlocks = 56;

/*
    Player position.

    playerY is the player's feet.
    Camera/eye height is playerY + EYE_HEIGHT.
*/
double playerX = 32.0;
double playerY = 12.0;
double playerZ = 32.0;

double velocityY = 0.0;
double lastVelocityY = 0.0;

int onGround = 0;

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

WorldSaveInfo worldSaves[MAX_WORLD_SLOTS];
int selectedWorldSlot = 0;
int currentWorldSlot = -1;
char newWorldName[WORLD_NAME_LEN] = "New World";
char newWorldSeedText[WORLD_SEED_LEN] = "";
int createInputField = 0;
int g_startFromSavedPosition = 0;
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
int IsSolidBlock(int block);
void DropMobLoot(Mob *m);
void PlayOneShotMP3(const char *filename);
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

void InitFeatureGapSystems(void);
void UpdateFeatureGapSystems(double dt);
void InitRecipeBook(void);
int RecipeBookFindResult(void);
void SaveCurrentTileEntities(void);
int LoadCurrentTileEntities(void);
void SaveCurrentRegionLite(void);
int LoadCurrentRegionLite(void);
void EnsureTileEntityForBlock(int block, int x, int y, int z);
void RemoveTileEntityAt(int x, int y, int z);
void UpdateRedstoneAround(int x, int y, int z);
void UpdateWeatherAdvanced(double dt);
void SpawnWeatherParticles(void);
void MobPathSteer(Mob *m, double desiredX, double desiredZ, double *outX, double *outZ);
void KillMobRenderable(Mob *m);
int GetItemMaxDurability(int item);
int GetToolHarvestLevel(int item);
double GetToolMiningSpeed(int item, int block);
void DamageHeldTool(int amount);
void DrawBiomeTintOverlayForBlock(int block, int x, int z)
{
    /* PATCH_NO_GREEN_TINT:
       The earlier biome-color pass multiplied grass/leaves/water by green and
       swamp/desert hues.  That looked like a broken overlay with the uploaded
       terrain atlas.  Keep Java/Beta-style blocky light and face shading, but
       force texture color back to neutral white so the PNG/TGA pixels show with
       their original hue. */
    (void)block;
    (void)x;
    (void)z;
    g_vertexTintR = 1.0f;
    g_vertexTintG = 1.0f;
    g_vertexTintB = 1.0f;
}


int IsTileEntityBlock(int block)
{
    /* Java/Beta-style tile-entity blocks that carry extra state beyond
       a simple block id.  Keeping this in one helper prevents missing
       linker symbols and makes it easy to add/remove tile blocks later. */
    switch (block) {
    case BLOCK_CHEST:
    case BLOCK_FURNACE:
    case BLOCK_SIGN_POST:
    case BLOCK_DISPENSER:
    case BLOCK_MOB_SPAWNER:
    case BLOCK_NOTE:
    case BLOCK_PISTON:
        return 1;
    default:
        return 0;
    }
}

int FindTileEntityAt(int x, int y, int z)
{
    int i;
    for (i = 0; i < MAX_TILE_ENTITIES; i++) {
        if (tileEntities[i].active && tileEntities[i].x == x && tileEntities[i].y == y && tileEntities[i].z == z) {
            return i;
        }
    }
    return -1;
}

void EnsureTileEntityForBlock(int block, int x, int y, int z)
{
    int i;
    int idx;
    if (!IsTileEntityBlock(block)) { return; }
    idx = FindTileEntityAt(x, y, z);
    if (idx >= 0) { tileEntities[idx].type = block; return; }
    for (i = 0; i < MAX_TILE_ENTITIES; i++) {
        if (!tileEntities[i].active) {
            ZeroMemory(&tileEntities[i], sizeof(TileEntityState));
            tileEntities[i].active = 1;
            tileEntities[i].type = block;
            tileEntities[i].x = x;
            tileEntities[i].y = y;
            tileEntities[i].z = z;
            if (block == BLOCK_SIGN_POST) { strcpy(tileEntities[i].text, "CloneMC sign"); }
            return;
        }
    }
}

void RemoveTileEntityAt(int x, int y, int z)
{
    int idx;
    idx = FindTileEntityAt(x, y, z);
    if (idx >= 0) { ZeroMemory(&tileEntities[idx], sizeof(TileEntityState)); }
}

void SaveCurrentTileEntities(void)
{
    FILE *f;
    char path[128];
    int i;
    int j;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    wsprintf(path, "saves\\world%d_tiles.dat", currentWorldSlot);
    f = fopen(path, "w");
    if (!f) { return; }
    fprintf(f, "CLONEMC_TILEENTITIES_V1\n");
    for (i = 0; i < MAX_TILE_ENTITIES; i++) {
        if (!tileEntities[i].active) { continue; }
        fprintf(f, "tile %d %d %d %d %.3f %.3f %d %s\n",
                tileEntities[i].type, tileEntities[i].x, tileEntities[i].y, tileEntities[i].z,
                tileEntities[i].burnTime, tileEntities[i].cookTime, tileEntities[i].power,
                tileEntities[i].text);
        for (j = 0; j < 27; j++) {
            if (tileEntities[i].slots[j].item != ITEM_NONE && tileEntities[i].slots[j].count > 0) {
                fprintf(f, "slot %d %d %d %d\n", j, tileEntities[i].slots[j].item,
                        tileEntities[i].slots[j].count, tileEntities[i].slots[j].damage);
            }
        }
    }
    fclose(f);
}

int LoadCurrentTileEntities(void)
{
    FILE *f;
    char path[128];
    char key[64];
    char text[64];
    int type;
    int x;
    int y;
    int z;
    int power;
    int slot;
    int item;
    int count;
    int damage;
    int idx;
    double burn;
    double cook;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    wsprintf(path, "saves\\world%d_tiles.dat", currentWorldSlot);
    f = fopen(path, "r");
    if (!f) { return 0; }
    if (fscanf(f, "%63s", key) != 1 || strcmp(key, "CLONEMC_TILEENTITIES_V1") != 0) { fclose(f); return 0; }
    ZeroMemory(tileEntities, sizeof(tileEntities));
    idx = -1;
    while (fscanf(f, "%63s", key) == 1) {
        if (strcmp(key, "tile") == 0) {
            text[0] = 0;
            if (fscanf(f, "%d %d %d %d %lf %lf %d %63s", &type, &x, &y, &z, &burn, &cook, &power, text) >= 7) {
                EnsureTileEntityForBlock(type, x, y, z);
                idx = FindTileEntityAt(x, y, z);
                if (idx >= 0) {
                    tileEntities[idx].burnTime = burn;
                    tileEntities[idx].cookTime = cook;
                    tileEntities[idx].power = power;
                    if (text[0]) { strncpy(tileEntities[idx].text, text, 63); tileEntities[idx].text[63] = 0; }
                }
            }
        } else if (strcmp(key, "slot") == 0) {
            damage = 0;
            if (fscanf(f, "%d %d %d %d", &slot, &item, &count, &damage) >= 3) {
                if (idx >= 0 && slot >= 0 && slot < 27) {
                    tileEntities[idx].slots[slot].item = item;
                    tileEntities[idx].slots[slot].count = count;
                    tileEntities[idx].slots[slot].damage = damage;
                }
            }
        }
    }
    fclose(f);
    return 1;
}

void SaveCurrentRegionLite(void)
{
    FILE *f;
    char path[128];
    int x;
    int y;
    int z;
    unsigned short b;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    wsprintf(path, "saves\\world%d_region_lite.mcr", currentWorldSlot);
    f = fopen(path, "wb");
    if (!f) { return; }
    fwrite("CMCR", 1, 4, f);
    fwrite(&worldOriginBlockX, sizeof(int), 1, f);
    fwrite(&worldOriginBlockZ, sizeof(int), 1, f);
    fwrite(&g_worldSeed, sizeof(int), 1, f);
    fwrite(&g_worldTimeSeconds, sizeof(double), 1, f);
    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                b = (unsigned short)world[x][y][z];
                fwrite(&b, sizeof(unsigned short), 1, f);
            }
        }
    }
    fclose(f);
}

int LoadCurrentRegionLite(void)
{
    FILE *f;
    char path[128];
    char magic[4];
    int ox;
    int oz;
    int seed;
    int x;
    int y;
    int z;
    unsigned short b;
    double wt;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    wsprintf(path, "saves\\world%d_region_lite.mcr", currentWorldSlot);
    f = fopen(path, "rb");
    if (!f) { return 0; }
    if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "CMCR", 4) != 0) { fclose(f); return 0; }
    if (fread(&ox, sizeof(int), 1, f) != 1 || fread(&oz, sizeof(int), 1, f) != 1 ||
        fread(&seed, sizeof(int), 1, f) != 1 || fread(&wt, sizeof(double), 1, f) != 1) { fclose(f); return 0; }
    if (ox != worldOriginBlockX || oz != worldOriginBlockZ) { fclose(f); return 0; }
    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                if (fread(&b, sizeof(unsigned short), 1, f) != 1) { fclose(f); return 0; }
                world[x][y][z] = (int)b;
            }
        }
    }
    fclose(f);
    g_worldSeed = seed;
    g_worldTimeSeconds = wt;
    RebuildColumnTops();
    InvalidateAllTerrainChunkMeshes();
    return 1;
}

int IsMobPathWalkableAt(int type, int x, int y, int z)
{
    int floorBlock;
    int foot;
    int head;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    if (type == MOB_SQUID) {
        return GetBlock(x, y, z) == BLOCK_WATER || GetBlock(x, y + 1, z) == BLOCK_WATER;
    }
    foot = GetBlock(x, y, z);
    head = GetBlock(x, y + 1, z);
    floorBlock = GetBlock(x, y - 1, z);
    if (IsSolidBlock(foot) || IsSolidBlock(head)) { return 0; }
    if (!IsSolidBlock(floorBlock)) { return 0; }
    if (floorBlock == BLOCK_CACTUS) { return 0; }
    return 1;
}

int MobFindStepY(int type, int x, int y, int z)
{
    if (IsMobPathWalkableAt(type, x, y, z)) { return y; }
    if (IsMobPathWalkableAt(type, x, y + 1, z)) { return y + 1; }
    if (y > 2 && IsMobPathWalkableAt(type, x, y - 1, z)) { return y - 1; }
    return -9999;
}

void MobPathSteer(Mob *m, double desiredX, double desiredZ, double *outX, double *outZ)
{
    int startX;
    int startY;
    int startZ;
    int goalX;
    int goalZ;
    int grid[PATH_GRID_SIZE][PATH_GRID_SIZE];
    int parent[PATH_GRID_SIZE][PATH_GRID_SIZE];
    int qx[PATH_GRID_SIZE * PATH_GRID_SIZE];
    int qz[PATH_GRID_SIZE * PATH_GRID_SIZE];
    int head;
    int tail;
    int cx;
    int cz;
    int nx;
    int nz;
    int wx;
    int wz;
    int sy;
    int bestX;
    int bestZ;
    int bestScore;
    int score;
    int dir;
    int dirs[8][2];
    int stepX;
    int stepZ;
    double dx;
    double dz;
    double len;
    if (!m || !outX || !outZ) { return; }
    if (fabs(desiredX) < 0.0001 && fabs(desiredZ) < 0.0001) { return; }
    if (m->type == MOB_SQUID) { return; }

    dirs[0][0] = 1; dirs[0][1] = 0; dirs[1][0] = -1; dirs[1][1] = 0;
    dirs[2][0] = 0; dirs[2][1] = 1; dirs[3][0] = 0; dirs[3][1] = -1;
    dirs[4][0] = 1; dirs[4][1] = 1; dirs[5][0] = -1; dirs[5][1] = 1;
    dirs[6][0] = 1; dirs[6][1] = -1; dirs[7][0] = -1; dirs[7][1] = -1;

    startX = (int)floor(m->x);
    startY = (int)floor(m->y);
    startZ = (int)floor(m->z);
    goalX = startX + (int)(desiredX * 7.0);
    goalZ = startZ + (int)(desiredZ * 7.0);
    if (goalX < startX - PATH_GRID_RADIUS) { goalX = startX - PATH_GRID_RADIUS; }
    if (goalX > startX + PATH_GRID_RADIUS) { goalX = startX + PATH_GRID_RADIUS; }
    if (goalZ < startZ - PATH_GRID_RADIUS) { goalZ = startZ - PATH_GRID_RADIUS; }
    if (goalZ > startZ + PATH_GRID_RADIUS) { goalZ = startZ + PATH_GRID_RADIUS; }

    for (cx = 0; cx < PATH_GRID_SIZE; cx++) {
        for (cz = 0; cz < PATH_GRID_SIZE; cz++) { grid[cx][cz] = 0; parent[cx][cz] = -1; }
    }
    head = 0; tail = 0;
    qx[tail] = PATH_GRID_RADIUS; qz[tail] = PATH_GRID_RADIUS; tail++;
    grid[PATH_GRID_RADIUS][PATH_GRID_RADIUS] = 1;
    bestX = PATH_GRID_RADIUS; bestZ = PATH_GRID_RADIUS;
    bestScore = 999999;

    while (head < tail) {
        cx = qx[head]; cz = qz[head]; head++;
        wx = startX + cx - PATH_GRID_RADIUS;
        wz = startZ + cz - PATH_GRID_RADIUS;
        score = abs(wx - goalX) + abs(wz - goalZ) + grid[cx][cz] / 2;
        if (score < bestScore) { bestScore = score; bestX = cx; bestZ = cz; }
        if (wx == goalX && wz == goalZ) { bestX = cx; bestZ = cz; break; }
        for (dir = 0; dir < 8; dir++) {
            nx = cx + dirs[dir][0]; nz = cz + dirs[dir][1];
            if (nx < 0 || nz < 0 || nx >= PATH_GRID_SIZE || nz >= PATH_GRID_SIZE) { continue; }
            if (grid[nx][nz]) { continue; }
            wx = startX + nx - PATH_GRID_RADIUS;
            wz = startZ + nz - PATH_GRID_RADIUS;
            sy = MobFindStepY(m->type, wx, startY, wz);
            if (sy < -1000) { continue; }
            grid[nx][nz] = grid[cx][cz] + 1;
            parent[nx][nz] = cz * PATH_GRID_SIZE + cx;
            qx[tail] = nx; qz[tail] = nz; tail++;
        }
    }

    stepX = bestX; stepZ = bestZ;
    while (parent[stepX][stepZ] >= 0) {
        nx = parent[stepX][stepZ] % PATH_GRID_SIZE;
        nz = parent[stepX][stepZ] / PATH_GRID_SIZE;
        if (nx == PATH_GRID_RADIUS && nz == PATH_GRID_RADIUS) { break; }
        stepX = nx; stepZ = nz;
    }
    dx = (double)(stepX - PATH_GRID_RADIUS);
    dz = (double)(stepZ - PATH_GRID_RADIUS);
    len = sqrt(dx * dx + dz * dz);
    if (len > 0.001) {
        *outX = dx / len;
        *outZ = dz / len;
    }
}

void KillMobRenderable(Mob *m)
{
    if (!m) { return; }
    if (!m->deathDropsDone) {
        DropMobLoot(m);
        m->deathDropsDone = 1;
    }
    m->hurtTime = 0.45;
    if (m->deathTime <= 0.0) { m->deathTime = 0.70; }
}

int GetItemMaxDurability(int item)
{
    if (item == ITEM_WOOD_SWORD || item == ITEM_WOOD_SHOVEL || item == ITEM_WOOD_PICKAXE || item == ITEM_WOOD_AXE || item == ITEM_WOOD_HOE) { return 60; }
    if (item == ITEM_STONE_SWORD || item == ITEM_STONE_SHOVEL || item == ITEM_STONE_PICKAXE || item == ITEM_STONE_AXE || item == ITEM_STONE_HOE) { return 132; }
    if (item == ITEM_IRON_SWORD || item == ITEM_IRON_SHOVEL || item == ITEM_IRON_PICKAXE || item == ITEM_IRON_AXE || item == ITEM_IRON_HOE) { return 251; }
    if (item == ITEM_DIAMOND_SWORD || item == ITEM_DIAMOND_SHOVEL || item == ITEM_DIAMOND_PICKAXE || item == ITEM_DIAMOND_AXE || item == ITEM_DIAMOND_HOE) { return 1562; }
    if (item == ITEM_GOLD_SWORD || item == ITEM_GOLD_SHOVEL || item == ITEM_GOLD_PICKAXE || item == ITEM_GOLD_AXE || item == ITEM_GOLD_HOE) { return 33; }
    if (item == ITEM_BOW || item == ITEM_FISHING_ROD || item == ITEM_SHEARS || item == ITEM_FLINT_STEEL) { return 384; }
    return 0;
}

int GetToolHarvestLevel(int item)
{
    if (item == ITEM_WOOD_PICKAXE || item == ITEM_WOOD_AXE || item == ITEM_WOOD_SHOVEL || item == ITEM_WOOD_SWORD) { return 0; }
    if (item == ITEM_STONE_PICKAXE || item == ITEM_STONE_AXE || item == ITEM_STONE_SHOVEL || item == ITEM_STONE_SWORD) { return 1; }
    if (item == ITEM_IRON_PICKAXE || item == ITEM_IRON_AXE || item == ITEM_IRON_SHOVEL || item == ITEM_IRON_SWORD) { return 2; }
    if (item == ITEM_DIAMOND_PICKAXE || item == ITEM_DIAMOND_AXE || item == ITEM_DIAMOND_SHOVEL || item == ITEM_DIAMOND_SWORD) { return 3; }
    if (item == ITEM_GOLD_PICKAXE || item == ITEM_GOLD_AXE || item == ITEM_GOLD_SHOVEL || item == ITEM_GOLD_SWORD) { return 1; }
    return -1;
}

int IsPickaxeBlock(int block)
{
    return block == BLOCK_STONE || block == BLOCK_COBBLESTONE || block == BLOCK_COAL_ORE || block == BLOCK_IRON_ORE ||
           block == BLOCK_GOLD_ORE || block == BLOCK_DIAMOND_ORE || block == BLOCK_REDSTONE_ORE || block == BLOCK_LAPIS_ORE ||
           block == BLOCK_FURNACE;
}

int IsAxeBlock(int block)
{
    return block == BLOCK_WOOD || block == BLOCK_PLANKS || block == BLOCK_CHEST || block == BLOCK_WORKBENCH || block == BLOCK_SIGN_POST || block == BLOCK_WOOD_DOOR;
}

int IsShovelBlock(int block)
{
    return block == BLOCK_DIRT || block == BLOCK_GRASS || block == BLOCK_SAND || block == BLOCK_GRAVEL || block == BLOCK_SNOW;
}

double GetToolMiningSpeed(int item, int block)
{
    int level;
    double speed;
    level = GetToolHarvestLevel(item);
    speed = 1.0;
    if ((IsPickaxeBlock(block) && (item == ITEM_WOOD_PICKAXE || item == ITEM_STONE_PICKAXE || item == ITEM_IRON_PICKAXE || item == ITEM_DIAMOND_PICKAXE || item == ITEM_GOLD_PICKAXE)) ||
        (IsAxeBlock(block) && (item == ITEM_WOOD_AXE || item == ITEM_STONE_AXE || item == ITEM_IRON_AXE || item == ITEM_DIAMOND_AXE || item == ITEM_GOLD_AXE)) ||
        (IsShovelBlock(block) && (item == ITEM_WOOD_SHOVEL || item == ITEM_STONE_SHOVEL || item == ITEM_IRON_SHOVEL || item == ITEM_DIAMOND_SHOVEL || item == ITEM_GOLD_SHOVEL))) {
        if (level == 0) { speed = 2.0; }
        else if (level == 1) { speed = 4.0; }
        else if (level == 2) { speed = 6.0; }
        else if (level == 3) { speed = 8.0; }
    }
    if (item == ITEM_GOLD_PICKAXE || item == ITEM_GOLD_AXE || item == ITEM_GOLD_SHOVEL) { speed = 12.0; }
    return speed;
}

void DamageHeldTool(int amount)
{
    InventorySlot *slot;
    int maxDur;
    if (selectedHotbarSlot < 0 || selectedHotbarSlot >= HOTBAR_SLOTS) { return; }
    slot = &hotbar[selectedHotbarSlot];
    maxDur = GetItemMaxDurability(slot->item);
    if (maxDur <= 0) { return; }
    slot->damage += amount;
    if (slot->damage >= maxDur) {
        slot->item = ITEM_NONE;
        slot->count = 0;
        slot->damage = 0;
        PlayOneShotMP3("assets\\sounds\\random\\break.mp3");
    }
}

void RecipeClear(SimpleRecipe *r)
{
    int i;
    r->width = 0; r->height = 0; r->outItem = ITEM_NONE; r->outCount = 0; r->shapeless = 0;
    for (i = 0; i < 9; i++) { r->in[i] = ITEM_NONE; }
}

void RecipeAddShaped(int w, int h, int outItem, int outCount, int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
    SimpleRecipe *r;
    if (g_simpleRecipeCount >= MAX_SIMPLE_RECIPES) { return; }
    r = &g_simpleRecipes[g_simpleRecipeCount++];
    RecipeClear(r);
    r->width = w; r->height = h; r->outItem = outItem; r->outCount = outCount; r->shapeless = 0;
    r->in[0] = a0; r->in[1] = a1; r->in[2] = a2; r->in[3] = a3; r->in[4] = a4; r->in[5] = a5; r->in[6] = a6; r->in[7] = a7; r->in[8] = a8;
}

void RecipeAddShapeless(int outItem, int outCount, int a0, int a1, int a2, int a3)
{
    SimpleRecipe *r;
    if (g_simpleRecipeCount >= MAX_SIMPLE_RECIPES) { return; }
    r = &g_simpleRecipes[g_simpleRecipeCount++];
    RecipeClear(r);
    r->width = 2; r->height = 2; r->outItem = outItem; r->outCount = outCount; r->shapeless = 1;
    r->in[0] = a0; r->in[1] = a1; r->in[2] = a2; r->in[3] = a3;
}

void InitRecipeBook(void)
{
    if (g_simpleRecipeCount > 0) { return; }
    RecipeAddShapeless(ITEM_PLANKS, 4, ITEM_WOOD, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(1, 2, ITEM_STICK, 4, ITEM_PLANKS, ITEM_NONE, ITEM_NONE, ITEM_PLANKS, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(1, 2, ITEM_TORCH, 4, ITEM_COAL, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(2, 2, ITEM_WORKBENCH, 1, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(2, 2, ITEM_SANDSTONE, 1, ITEM_SAND, ITEM_SAND, ITEM_NONE, ITEM_SAND, ITEM_SAND, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(2, 2, ITEM_WOOL, 1, ITEM_STRING, ITEM_STRING, ITEM_NONE, ITEM_STRING, ITEM_STRING, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(2, 2, ITEM_SNOW_BLOCK, 1, ITEM_SNOWBALL, ITEM_SNOWBALL, ITEM_NONE, ITEM_SNOWBALL, ITEM_SNOWBALL, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);

    RecipeAddShaped(3, 3, ITEM_CHEST, 1, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS);
    RecipeAddShaped(3, 3, ITEM_FURNACE, 1, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_NONE, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_COBBLESTONE);
    RecipeAddShaped(3, 3, ITEM_WOOD_PICKAXE, 1, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_STONE_PICKAXE, 1, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_IRON_PICKAXE, 1, ITEM_IRON_INGOT, ITEM_IRON_INGOT, ITEM_IRON_INGOT, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_DIAMOND_PICKAXE, 1, ITEM_DIAMOND, ITEM_DIAMOND, ITEM_DIAMOND, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_GOLD_PICKAXE, 1, ITEM_GOLD_INGOT, ITEM_GOLD_INGOT, ITEM_GOLD_INGOT, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);

    RecipeAddShaped(3, 3, ITEM_WOOD_SWORD, 1, ITEM_NONE, ITEM_PLANKS, ITEM_NONE, ITEM_NONE, ITEM_PLANKS, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_STONE_SWORD, 1, ITEM_NONE, ITEM_COBBLESTONE, ITEM_NONE, ITEM_NONE, ITEM_COBBLESTONE, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_IRON_SWORD, 1, ITEM_NONE, ITEM_IRON_INGOT, ITEM_NONE, ITEM_NONE, ITEM_IRON_INGOT, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_DIAMOND_SWORD, 1, ITEM_NONE, ITEM_DIAMOND, ITEM_NONE, ITEM_NONE, ITEM_DIAMOND, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_GOLD_SWORD, 1, ITEM_NONE, ITEM_GOLD_INGOT, ITEM_NONE, ITEM_NONE, ITEM_GOLD_INGOT, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);

    RecipeAddShaped(3, 3, ITEM_WOOD_SHOVEL, 1, ITEM_NONE, ITEM_PLANKS, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_STONE_SHOVEL, 1, ITEM_NONE, ITEM_COBBLESTONE, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_IRON_SHOVEL, 1, ITEM_NONE, ITEM_IRON_INGOT, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_DIAMOND_SHOVEL, 1, ITEM_NONE, ITEM_DIAMOND, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);

    RecipeAddShaped(3, 3, ITEM_WOOD_AXE, 1, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_PLANKS, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_STONE_AXE, 1, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_NONE, ITEM_COBBLESTONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_IRON_AXE, 1, ITEM_IRON_INGOT, ITEM_IRON_INGOT, ITEM_NONE, ITEM_IRON_INGOT, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_DIAMOND_AXE, 1, ITEM_DIAMOND, ITEM_DIAMOND, ITEM_NONE, ITEM_DIAMOND, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE);

    RecipeAddShaped(3, 3, ITEM_BOWL, 4, ITEM_PLANKS, ITEM_NONE, ITEM_PLANKS, ITEM_NONE, ITEM_PLANKS, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_BOW, 1, ITEM_NONE, ITEM_STICK, ITEM_STRING, ITEM_STICK, ITEM_NONE, ITEM_STRING, ITEM_NONE, ITEM_STICK, ITEM_STRING);
    RecipeAddShaped(3, 3, ITEM_ARROW, 4, ITEM_NONE, ITEM_FLINT, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_FEATHER, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_BUCKET, 1, ITEM_IRON_INGOT, ITEM_NONE, ITEM_IRON_INGOT, ITEM_NONE, ITEM_IRON_INGOT, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(2, 2, ITEM_SHEARS, 1, ITEM_NONE, ITEM_IRON_INGOT, ITEM_IRON_INGOT, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(2, 2, ITEM_FLINT_STEEL, 1, ITEM_IRON_INGOT, ITEM_NONE, ITEM_NONE, ITEM_FLINT, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_LADDER, 3, ITEM_STICK, ITEM_NONE, ITEM_STICK, ITEM_STICK, ITEM_STICK, ITEM_STICK, ITEM_STICK, ITEM_NONE, ITEM_STICK);
    RecipeAddShaped(3, 3, ITEM_SIGN, 3, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_STICK, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_WOOD_DOOR, 1, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_BOAT, 1, ITEM_PLANKS, ITEM_NONE, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_RAIL, 16, ITEM_IRON_INGOT, ITEM_NONE, ITEM_IRON_INGOT, ITEM_IRON_INGOT, ITEM_STICK, ITEM_IRON_INGOT, ITEM_IRON_INGOT, ITEM_NONE, ITEM_IRON_INGOT);
    RecipeAddShaped(3, 3, ITEM_MINECART, 1, ITEM_IRON_INGOT, ITEM_NONE, ITEM_IRON_INGOT, ITEM_IRON_INGOT, ITEM_IRON_INGOT, ITEM_IRON_INGOT, ITEM_NONE, ITEM_NONE, ITEM_NONE);
}

int RecipeGridCellHasItem(int index)
{
    if (index < 0 || index >= 9) { return 0; }
    return (craftGrid[index].item != ITEM_NONE && craftGrid[index].count > 0);
}

int RecipeGridEffectiveWidth(void)
{
    return craftingOpen ? 3 : 2;
}

int RecipeGridEffectiveHeight(void)
{
    return craftingOpen ? 3 : 2;
}

int RecipeGridMatchesShapeless(SimpleRecipe *r)
{
    int wanted[9];
    int found[9];
    int wi;
    int fi;
    int i;
    int row;
    int col;
    int idx;
    int j;
    int ok;
    int gridW;
    int gridH;

    gridW = RecipeGridEffectiveWidth();
    gridH = RecipeGridEffectiveHeight();
    for (i = 0; i < 9; i++) { wanted[i] = ITEM_NONE; found[i] = ITEM_NONE; }

    wi = 0;
    for (i = 0; i < 9; i++) {
        if (r->in[i] != ITEM_NONE) {
            wanted[wi++] = r->in[i];
        }
    }

    fi = 0;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            idx = row * 3 + col;
            if (!craftingOpen && (row >= gridH || col >= gridW)) {
                if (RecipeGridCellHasItem(idx)) { return 0; }
            } else if (RecipeGridCellHasItem(idx)) {
                found[fi++] = craftGrid[idx].item;
            }
        }
    }

    if (wi != fi) { return 0; }
    for (i = 0; i < wi; i++) {
        ok = 0;
        for (j = 0; j < fi; j++) {
            if (found[j] == wanted[i]) {
                found[j] = ITEM_NONE;
                ok = 1;
                break;
            }
        }
        if (!ok) { return 0; }
    }
    return 1;
}

int RecipeGridMatchesAt(SimpleRecipe *r, int offX, int offY, int mirror)
{
    int row;
    int col;
    int idx;
    int localX;
    int localY;
    int wantX;
    int wanted;
    int actual;
    int gridW;
    int gridH;

    gridW = RecipeGridEffectiveWidth();
    gridH = RecipeGridEffectiveHeight();
    if (!craftingOpen && (r->width > 2 || r->height > 2)) { return 0; }
    if (offX < 0 || offY < 0 || offX + r->width > gridW || offY + r->height > gridH) { return 0; }

    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            idx = row * 3 + col;
            if (!craftingOpen && (row >= gridH || col >= gridW)) {
                if (RecipeGridCellHasItem(idx)) { return 0; }
                continue;
            }

            wanted = ITEM_NONE;
            if (row >= offY && row < offY + r->height &&
                col >= offX && col < offX + r->width) {
                localY = row - offY;
                localX = col - offX;
                wantX = mirror ? (r->width - 1 - localX) : localX;
                wanted = r->in[localY * 3 + wantX];
            }

            actual = ITEM_NONE;
            if (RecipeGridCellHasItem(idx)) { actual = craftGrid[idx].item; }
            if (actual != wanted) { return 0; }
        }
    }
    return 1;
}

int RecipeGridMatches(SimpleRecipe *r)
{
    int offX;
    int offY;
    int maxX;
    int maxY;
    int gridW;
    int gridH;

    if (!r) { return 0; }
    if (r->shapeless) { return RecipeGridMatchesShapeless(r); }
    if (!craftingOpen && (r->width > 2 || r->height > 2)) { return 0; }

    gridW = RecipeGridEffectiveWidth();
    gridH = RecipeGridEffectiveHeight();
    maxX = gridW - r->width;
    maxY = gridH - r->height;
    if (maxX < 0 || maxY < 0) { return 0; }

    for (offY = 0; offY <= maxY; offY++) {
        for (offX = 0; offX <= maxX; offX++) {
            if (RecipeGridMatchesAt(r, offX, offY, 0)) { return 1; }
            if (RecipeGridMatchesAt(r, offX, offY, 1)) { return 1; }
        }
    }
    return 0;
}

int RecipeBookFindResult(void)
{
    int i;
    InitRecipeBook();
    for (i = 0; i < g_simpleRecipeCount; i++) {
        if (RecipeGridMatches(&g_simpleRecipes[i])) {
            craftResult.item = g_simpleRecipes[i].outItem;
            craftResult.count = g_simpleRecipes[i].outCount;
            craftResult.damage = 0;
            return 1;
        }
    }
    return 0;
}

void UpdateRedstoneAround(int x, int y, int z)
{
    int dx;
    int dy;
    int dz;
    int nx;
    int ny;
    int nz;
    int block;
    int p;
    for (dx = -4; dx <= 4; dx++) {
        for (dy = -2; dy <= 2; dy++) {
            for (dz = -4; dz <= 4; dz++) {
                nx = x + dx; ny = y + dy; nz = z + dz;
                if (!IsInsideWorld(nx, ny, nz)) { continue; }
                block = GetBlock(nx, ny, nz);
                p = 0;
                if (block == BLOCK_LEVER || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_STONE_BUTTON || block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE) { p = 15; }
                else if (block == BLOCK_REDSTONE_WIRE) {
                    p = 0;
                    if (IsInsideWorld(nx + 1, ny, nz) && g_redstonePower[nx + 1][ny][nz] > p) { p = g_redstonePower[nx + 1][ny][nz] - 1; }
                    if (IsInsideWorld(nx - 1, ny, nz) && g_redstonePower[nx - 1][ny][nz] > p) { p = g_redstonePower[nx - 1][ny][nz] - 1; }
                    if (IsInsideWorld(nx, ny, nz + 1) && g_redstonePower[nx][ny][nz + 1] > p) { p = g_redstonePower[nx][ny][nz + 1] - 1; }
                    if (IsInsideWorld(nx, ny, nz - 1) && g_redstonePower[nx][ny][nz - 1] > p) { p = g_redstonePower[nx][ny][nz - 1] - 1; }
                    if (p < 0) { p = 0; }
                }
                g_redstonePower[nx][ny][nz] = (unsigned char)p;
            }
        }
    }
}

void UpdateWeatherAdvanced(double dt)
{
    int biome;
    g_weatherTimer -= dt;
    if (g_weatherTimer <= 0.0) {
        biome = GetLocalBiome((int)floor(playerX), (int)floor(playerZ));
        if (biome == BIOME_DESERT) { g_weatherMode = 0; g_weatherTimer = 90.0; }
        else {
            if ((WorldHash3D((int)g_worldTimeSeconds, g_worldSeed, biome, 9001) % 100) < 45) {
                g_weatherMode = (biome == BIOME_TUNDRA || biome == BIOME_TAIGA) ? 2 : 1;
                g_weatherTimer = 45.0 + (double)(WorldHash2D(biome, g_worldSeed, 9002) % 90);
            } else { g_weatherMode = 0; g_weatherTimer = 60.0; }
        }
    }
    if (g_weatherMode == 0) { g_rainStrength -= dt * 0.35; }
    else { g_rainStrength += dt * 0.35; }
    if (g_rainStrength < 0.0) { g_rainStrength = 0.0; }
    if (g_rainStrength > 1.0) { g_rainStrength = 1.0; }
    if (g_rainStrength > 0.05) { SpawnWeatherParticles(); }
}

void SpawnWeatherParticles(void)
{
    int i;
    int x;
    int z;
    int y;
    int count;
    int p;
    count = (int)(6.0 * g_rainStrength);
    if (count < 1) { count = 1; }
    for (p = 0; p < count; p++) {
        x = (int)floor(playerX) - 12 + (WorldHash3D(p, (int)g_worldTimeSeconds, g_worldSeed, 9100) % 25);
        z = (int)floor(playerZ) - 12 + (WorldHash3D(p, (int)g_worldTimeSeconds, g_worldSeed, 9110) % 25);
        y = WORLD_Y - 2;
        while (y > 1 && GetBlock(x, y - 1, z) == BLOCK_AIR) { y--; }
        for (i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life <= 0.0f) {
                particles[i].x = (float)x + 0.5f;
                particles[i].y = (float)(y + 5);
                particles[i].z = (float)z + 0.5f;
                particles[i].vx = 0.0f;
                particles[i].vy = g_weatherMode == 2 ? -0.06f : -0.18f;
                particles[i].vz = 0.0f;
                particles[i].life = g_weatherMode == 2 ? 1.2f : 0.55f;
                particles[i].maxLife = particles[i].life;
                particles[i].block = g_weatherMode == 2 ? BLOCK_SNOW : BLOCK_WATER;
                break;
            }
        }
    }
}

void InitFeatureGapSystems(void)
{
    InitRecipeBook();
}

void UpdateFeatureGapSystems(double dt)
{
    UpdateWeatherAdvanced(dt);
}

/* DrawBiomeTintOverlayForBlock implementation moved into feature-gap block above. */



int GetItemIconTile(int item, int *col, int *row);
/* PATCH_F11_MOB_GUI: forward item/block mapping so Open Watcom does not warn before first use. */
int ItemToBlock(int item);
void DrawTexturedQuad2D(GLuint texture, int atlasWidth, int atlasHeight, int col, int row, int x1, int y1, int x2, int y2);
void DrawImage2D(GLuint texture, int x1, int y1, int x2, int y2, float alpha);
void DrawImageCrop2D(GLuint texture, int atlasW, int atlasH, int sx, int sy, int sw, int sh, int x1, int y1, int x2, int y2, float alpha);
void DrawImage3DBillboard(GLuint texture, float cx, float cy, float cz, float width, float height, float alpha);
void DrawImportedBetaLogo(void);
void DrawDisabledButton2D(RECT r, const char *text);
void DrawBetaButtonPanel(RECT r, int hover, int disabled);
void DrawBetaVignette2D(void);
void DrawWaterOverlay2D(void);
void DrawOxygenBubbles(void);
void DrawWeather2D(void);
void DrawBetaMenuFooter(void);
void RenderSkyBodies(void);
void EnableBetaFog(void);
void DisableBetaFog(void);
void DrawMobShadow(Mob *m);
void DrawTerrainTile2D(int col, int row, int x1, int y1, int x2, int y2);
void DrawIconTile2D(int col, int row, int x1, int y1, int x2, int y2);

/*
    Custom full/empty heart textures.
    These are not atlas tiles. They are standalone TGA images:
        assets\heart_full.tga
        assets\heart_empty.tga
*/
void DrawHeartTexture(GLuint texture, int x, int y, int size);
void DrawHealthHearts(void);
void HealPlayer(int amount);

void MainLoop(void);

/* Better input, spawn, and music */
void ResetInputState(void);
void SetGameKey(WPARAM key, int down);
void HideGameCursor(void);
void ShowGameCursor(void);
void LockMouseForGame(void);
void UnlockMouseFromGame(void);
int IsSpawnSpaceClear(double sx, double sy, double sz);
int IsValidSpawnGround(int block);
int FindSafeSpawn(double *sx, double *sy, double *sz);

void StopAllMusic(void);
int PlayMusicFile(const char *filename, const char *aliasName, int repeat);
int IsMusicAliasPlaying(const char *aliasName);
void StartMenuMusic(void);
void StartGameMusic(void);
void UpdateMusic(double dt);

void StopMobSounds(void);
void PlayOneShotMP3(const char *filename);
void PlayUIClickSound(void);
void PlayPlayerHitSound(void);
void PlayPlayerStepSound(int blockBelow);

void PlayItemPickupSound(void);
int GetHeldHotbarItem(void);
void DrawHeldItemFirstPerson(int item);
void DrawHeldItemThirdPerson(int item);
void DrawHeldItemQuadLocal(int item, float size);
void DrawHeldBlockCubeLocal(int block, float scale);
void DrawMobSkinBoxPartRot(float px, float py, float pz, float cx, float cy, float cz, float sx, float sy, float sz, GLuint tex, float shade, float alpha, int tx, int ty, int pw, int ph, int pd, float rotX, float rotY, float rotZ);
void ReturnCraftingGridToInventory(void);
void DropCarriedInventoryStackToWorld(int oneOnly);
int IsPointInNormalInventoryPanel(int mx, int my);
int GetPlayerCraftingSlotAtPoint(int mx, int my, int *slotType);
void PlayerInventoryCraftingClick(int mx, int my);
void PlayerInventoryCraftingRightClick(int mx, int my);

void PlayMobIdleSound(int type, int angry);
void PlayMobHurtSound(int type);
void PlayMobIdleSoundNear(Mob *m);
void PlayMobHurtSoundNear(Mob *m);
void PlayMobStepSoundNear(Mob *m, int blockBelow);
double MobDistanceSquaredToPlayer(Mob *m);
int IsMobInsideLoadedWindow(Mob *m);
void RebaseMobsAfterWorldStream(int oldOriginX, int oldOriginZ);
int CountMobGroup(int hostileGroup);
int IsMobAggressiveNow(Mob *m);
int IsValidMobSpawnSpace(int type, int hostile, int x, int y, int z);
int FindUndergroundHostileSpawn(int *sx, int *sy, int *sz, int *typeOut);
int FindSurfaceMobSpawn(int hostile, int *sx, int *sy, int *sz, int *typeOut);
int PickPassiveMobType(int x, int z, int below);
int PickHostileMobType(int x, int y, int z, int underground);
void InitMobs(void);
void SpawnInitialMobs(void);
void SpawnVisibleStarterMobs(void);
int FindMobShowcaseSurface(int startX, int startZ, int *sx, int *sy, int *sz);
void UpdateMobs(double dt);
void RenderMobs(void);
int CountActiveMobs(void);
int CountMobType(int type);
int IsPassiveMobType(int type);
int IsHostileMobType(int type);
int IsNightHostile(int type);
int MobBurnsInDaylight(int type);
int IsDaylightForMobs(void);
int FindMobSpawnPoint(int hostile, int *sx, int *sy, int *sz, int *typeOut);
int AddMob(int type, double x, double y, double z);
void DamageMob(int index, int amount, double knockX, double knockZ);
int AttackMobRaycast(void);
void DropWoolNearMob(Mob *m);
void DropMobLoot(Mob *m);
void ExplodeCreeper(Mob *m);
void DrawMobBillboard(Mob *m, GLuint tex, float width, float height);

void DrawMobBoxPart(float cx, float cy, float cz, float sx, float sy, float sz, GLuint tex, float shade, float alpha);
void DrawMobSkinBoxPart(float cx, float cy, float cz, float sx, float sy, float sz, GLuint tex, float shade, float alpha, int tx, int ty, int pw, int ph, int pd);
void RenderMobModelTex(Mob *m, GLuint tex, float alpha);
GLuint GetMobTexture(Mob *m);
float MobWidth(int type);
float MobHeight(int type);

void EnterMenu(void);
void EnterGame(void);
void EnterSettings(void);
void EnterPauseMenu(void);
void EnterDeathScreen(void);
void RespawnPlayerAtWorldSpawn(void);
void EnterWorldSelect(void);
void EnterCreateWorld(void);
void EnterOptions(void);
void LayoutBetaMenus(void);
void SetRectXYWH(RECT *r, int x, int y, int w, int h);
int EstimateTextWidth(GLuint base, const char *text);
void DrawCenteredText2D(GLuint base, int x1, int y1, int x2, int y2, const char *text);
void DrawTextField2D(RECT r, const char *label, const char *value, int active);
void DrawWorldSelect(void);
void DrawCreateWorld(void);
void DrawOptions(void);
void DrawPauseMenu(void);
void DrawDeathScreen(void);
void HandleCreateWorldChar(WPARAM ch);
void LoadWorldList(void);
int LoadWorldSlot(int slot, WorldSaveInfo *info);
void SaveWorldSlotInfo(int slot);
void SaveCurrentWorld(void);
void GetInventorySavePath(int slot, char *path);
void GetDroppedItemsSavePath(int slot, char *path);
void GetBlockSavePath(int slot, char *path);
void SaveCurrentInventory(void);
int LoadCurrentInventory(void);
void SaveCurrentDroppedItems(void);
int LoadCurrentDroppedItems(void);
void SaveCurrentBlocks(void);
int LoadCurrentBlocks(void);
void DeleteWorldSlot(int slot);
void CreateWorldFromMenu(void);
void StartWorldSlot(int slot);
void StartNewWorldInSlot(int slot);
void GetWorldSavePath(int slot, char *path);
void GetMobSavePath(int slot, char *path);
void SaveCurrentMobs(void);
int LoadCurrentMobs(void);
void EnsureSaveDirectory(void);
void SanitizeWorldName(char *name);
int SeedFromText(const char *text);

void GameInit(void);
void GameUpdate(double dt);
void GameRender(void);

void Setup2D(void);

void DrawMenu(void);
void DrawSettings(void);
void DrawWorldSelect(void);
void DrawCreateWorld(void);
void DrawOptions(void);
void DrawPauseMenu(void);
void DrawDeathScreen(void);
void DrawDirtMenuBackground(void);
void DrawButton2D(RECT r, const char *text, int hover);
void DrawRect2D(int x1, int y1, int x2, int y2, float r, float g, float b);
int PointInRectInt(int x, int y, RECT r);


/* Finite world size / old-alpha ocean border helpers */
int IsGlobalInsideFiniteWorld(int gx, int gz);
int DistanceToFiniteWorldEdge(int gx, int gz);
int IsGlobalInBorderOcean(int gx, int gz);
void FillFiniteOceanColumn(int lx, int lz, int gx, int gz);
void ClampPlayerToFiniteWorld(void);
const char *WorldSizeLabel(int size);
void ChangeCreateWorldSize(void);
void RenderWorldBorderOceanIllusion(void);
void SetRenderDistanceFromMouse(int mouseX);

void GenerateWorld(void);
void GenerateWorldWindow(int centerChunkX, int centerChunkZ);
void UpdateInfiniteWorldStreaming(void);
void KeepPlayerSafeAfterStreaming(void);
int IsSkyOpenForSpawn(int bx, int by, int bz);
void ForceSpawnPad(int x, int y, int z);
int FloorDivInt(int a, int b);
int LocalToGlobalBlockX(int x);
int LocalToGlobalBlockZ(int z);
int GlobalToLocalBlockX(int gx);
int GlobalToLocalBlockZ(int gz);
double GetPlayerGlobalX(void);
double GetPlayerGlobalZ(void);
void RebuildColumnTops(void);
void RebuildColumnTopAt(int x, int z);
int FindHighestSolidOrWater(int x, int z);
int TerrainHeight(int x, int z);
int Hash2D(int x, int z);
void AddTree(int x, int y, int z);

int WorldHash2D(int x, int z, int seed);
int WorldHash3D(int x, int y, int z, int seed);
int ClampInt(int v, int minv, int maxv);
double ClampDouble(double v, double minv, double maxv);
double WorldFade(double t);
double WorldLerp(double a, double b, double t);
double WorldGrad2(int ix, int iz, double x, double z, int seed);
double WorldPerlin2D(double x, double z, int seed);
double WorldFractal2D(double x, double z, int seed, int octaves, double persistence);
double WorldGrad3(int ix, int iy, int iz, double x, double y, double z, int seed);
double WorldPerlin3D(double x, double y, double z, int seed);
double WorldFractal3D(double x, double y, double z, int seed, int octaves, double persistence);
double BetaMountainMask(int x, int z);
float BetaClimateTemperature(int gx, int gz);
float BetaClimateHumidity(int gx, int gz);
int GetBetaBiomeFromClimate(float temp, float humidity, int gx, int gz);
int GetBetaBiomeAt(int gx, int gz);
int GetBiomeAtGlobal(int gx, int gz);
const char *GetBetaBiomeName(int biome);
int BiomeTopBlock(int biome, int y);
int BiomeFillerBlock(int biome);
int BiomeTreeChance(int biome);
int GetLocalBiome(int x, int z);
int IsPointProbablyInView(double x, double z, double nearAlways, double dotLimit);
int IsColumnProbablyVisible(int x, int z, int distSq);
int GetTerrainRenderDistanceBlocks(void);
int GetNearTerrainRenderDistanceBlocks(void);
int GetVeryNearTerrainRenderDistanceBlocks(void);
int GetMobRenderDistanceBlocks(void);
const char *RenderDistanceLabel(void);
void ChangeRenderDistance(int delta);
void ApplyDamageCameraWobble(void);
void TriggerDamageCameraWobble(int amount);
double NormalizeAngle180(double a);
double ApproachAngleDeg(double from, double to, double maxStep);
double MobFaceYawFromDelta(double dx, double dz);
void MobApproachFacing(Mob *m, double dx, double dz, double dt, double rate);
void UpdatePlayerMovementAnimation(double dt);
int GetBlockyFontRows(char c, unsigned char rows[7]);
int BlockyCharWidth(GLuint base);
int BlockyPixelSize(GLuint base);
void DrawBlockyText2D(GLuint base, int x, int y, const char *text);

double BetaDensity3D(int x, int y, int z, int surfaceY);
void CarveSphere(int cx, int cy, int cz, int radius);
void CarveEllipsoid(int cx, int cy, int cz, int rx, int ry, int rz);
void AddRandomWalkerCaves(void);

void AddOrePass(void);
void AddOreVein(int cx, int cy, int cz, int oreBlock, int blocks, int radius);
int IsOreExposedToSky(int x, int y, int z);
int CanReplaceStoneWithOre(int x, int y, int z);
void SpawnWaterBubbleParticles(double x, double y, double z, int count);
int IsPlayerInWater(void);
int IsPlayerHeadUnderWater(void);
double GetPlayerWaterImmersion(void);
void UpdatePlayerWaterPhysics(double dt);
int CanWaterFlowIntoBlock(int block);
int IsWaterSpreadSupportBlock(int block);
void UpdateJavaStyleWaterFlow(double dt);
void OpenCraftingTable(int x, int y, int z);
void CloseCraftingTable(void);
void UpdateCraftingResult(void);
void ConsumeCraftingIngredients(void);
void CraftingMouseClick(int mx, int my);
void CraftingMouseRightClick(int mx, int my);
void CraftingSlotClick(InventorySlot *slot);
void CraftingSlotRightClick(InventorySlot *slot);
void DrawCraftingScreen(void);
int GetCraftingSlotAtPoint(int mx, int my, int *slotType);
void DrawItemIcon2D(int item, int x, int y, int size);
void DrawCenteredItemStack(int x, int y, InventorySlot slot, int selected);
int IsItemBlock(int item);
int AreCraftSlotsOnly(int item, int count);
void AddBetaCaveWorms(void);
void AddVerticalShaftsAndRooms(void);
void AddLiquidPass(void);
void AddSurfaceTexturePass(void);
int BetaTerrainHeight(int x, int z);
int IsBetaCave(int x, int y, int z, int surfaceY);
void AddBetaTree(int x, int y, int z);
void AddBetaCactus(int x, int y, int z, int height);
void AddBigBetaTree(int x, int y, int z);
void AddWorldFeatures(void);

void UpdateClouds(double dt);
void RenderClouds(void);
int CloudCellVisible(int cx, int cz);
void DrawCloudCell(float x0, float z0, float x1, float z1, float alpha);

void UpdateDayNightCycle(double dt);
void ApplyDayNightClearColor(void);
float ApplyGammaBoost(float v);
void InitParticles(void);
void SpawnBlockBreakParticles(int x, int y, int z, int block);
void SpawnMobEffectParticles(Mob *m, int block, int count);
void UpdateParticles(double dt);
void RenderParticles(void);
void GetParticleColorForBlock(int block, float *r, float *g, float *b);
void DrawBetaStatus2D(void);
void UpdateFPSCounter(double dt);

/* First-person rectangular-prism arm */
void StartHandSwing(void);
void StartHandUse(void);
void UpdatePlayerHandAnimation(double dt);
float GetHandProgress(double timer, double length);
void RenderPlayerHand(void);
void DrawDroppedBlockCube(int block);
void DrawTorchBlock(int x, int y, int z);

int GetHeldHotbarItem(void)
{
    if (selectedHotbarSlot < 0 || selectedHotbarSlot >= HOTBAR_SLOTS) {
        return ITEM_NONE;
    }
    if (hotbar[selectedHotbarSlot].count <= 0) {
        return ITEM_NONE;
    }
    return hotbar[selectedHotbarSlot].item;
}

void DrawHeldBlockCubeLocal(int block, float scale)
{
    if (block == BLOCK_AIR || !texTerrain) { return; }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPushMatrix();
    glScalef(scale, scale, scale);
    DrawDroppedBlockCube(block);
    glPopMatrix();
}

void DrawHeldItemQuadLocal(int item, float size)
{
    int col;
    int row;
    float u0;
    float v0;
    float u1;
    float v1;

    if (item == ITEM_NONE || !texBetaItems) { return; }
    if (!GetItemIconTile(item, &col, &row)) { return; }
    GetTileUVEx(col, row, ICONS_ATLAS_WIDTH, ICONS_ATLAS_HEIGHT, &u0, &v0, &u1, &v1);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texBetaItems);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(u0, v1); glVertex3f(-size, -size, 0.0f);
    glTexCoord2f(u1, v1); glVertex3f( size, -size, 0.0f);
    glTexCoord2f(u1, v0); glVertex3f( size,  size, 0.0f);
    glTexCoord2f(u0, v0); glVertex3f(-size,  size, 0.0f);
    glEnd();
}

void DrawHeldItemFirstPerson(int item)
{
    int block;

    if (item == ITEM_NONE) { return; }
    block = ItemToBlock(item);

    glPushMatrix();

    if (block != BLOCK_AIR) {
        /* Larger Java ItemRenderer-style in-hand block transform.  The block is
           close to the camera and covers most of the right hand instead of
           floating as a tiny cube beside it. */
        glTranslatef(-0.08f, 0.12f, -0.26f);
        glRotatef(18.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(48.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-8.0f, 0.0f, 0.0f, 1.0f);
        DrawHeldBlockCubeLocal(block, 0.50f);
    } else {
        /* Non-block items are drawn larger and closer so swords/tools/food
           occupy the hand like Java's first-person ItemRenderer. */
        glTranslatef(-0.05f, 0.16f, -0.24f);
        glRotatef(68.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-92.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(18.0f, 0.0f, 1.0f, 0.0f);
        DrawHeldItemQuadLocal(item, 0.50f);
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPopMatrix();
}

void DrawHeldItemThirdPerson(int item)
{
    int block;

    if (item == ITEM_NONE) { return; }
    block = ItemToBlock(item);

    glPushMatrix();
    /* Right hand anchor, matching RenderPlayer's postRender arm placement. */
    glTranslatef(-0.48f, 1.12f, -0.22f);

    if (block != BLOCK_AIR) {
        glTranslatef(0.0f, 0.16f, -0.16f);
        glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
        DrawHeldBlockCubeLocal(block, 0.20f);
    } else {
        glTranslatef(0.08f, 0.11f, -0.16f);
        glRotatef(60.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(20.0f, 0.0f, 0.0f, 1.0f);
        DrawHeldItemQuadLocal(item, 0.18f);
    }
    glPopMatrix();
}

void DrawPlayerArmPrism(void);
void DrawPlayerArmFace(float x0, float x1, float y0, float y1, float z0, float z1, int face);

int IsInsideWorld(int x, int y, int z);
int GetBlock(int x, int y, int z);
void SetBlock(int x, int y, int z, int block);
int IsSolidBlock(int block);

void UpdateMouseLook(void);
void CenterMouse(void);
void HandleGameInput(double dt);

int PlayerCollidesAt(double x, double y, double z);
int MovePlayerAxis(double dx, double dy, double dz);

void BreakBlockRaycast(void);
void PlaceBlockRaycast(void);

void SetupCamera(void);
void RenderWorld(void);
void DrawBlock(int x, int y, int z, int block);
void PlayBlockBreakSound(int block);
int GetBlockSoundGroup(int block);
void BuildTerrainChunkMesh(int cx, int cz);
int IsChunkProbablyVisible(int cx, int cz, int distSq);
void InvalidateTerrainChunkMeshAt(int x, int z);
void DeleteTerrainChunkMeshes(void);
void InvalidateAllTerrainChunkMeshes(void);

void DrawBlockFast(int x, int y, int z, int block)
{
    DrawBlock(x, y, z, block);
}

void RenderWorldBorderOceanIllusion(void)
{
    /* Finite-world border ocean. Intentionally lightweight. */
}

void DrawBlock(int x, int y, int z, int block);
void DrawFace(int x, int y, int z, int face, int block);
void DrawFaceFast(int x, int y, int z, int face, int block);
void DrawBlockFast(int x, int y, int z, int block);
int ShouldDrawFace(int nx, int ny, int nz, int block);
void SetBlockColorFallback(int block, int face);

/* Legacy grayscale lighting and AO */
void ComputeLegacyLighting(void);
void RecomputeLegacyLightingLocal(int cx, int cy, int cz, int radius);
void PropagateLightArrayLocal(unsigned char light[WORLD_X][WORLD_Y][WORLD_Z], LightNode *queue, int startTail, int minX, int maxX, int minY, int maxY, int minZ, int maxZ);
void ClearLightArrays(void);
int BlocksLightForLighting(int block);
int IsAOBlock(int block);
int GetBlockEmission(int block);
void PropagateLightArray(unsigned char light[WORLD_X][WORLD_Y][WORLD_Z], LightNode *queue, int startTail);
int PushLightNode(LightNode *queue, int *tail, int x, int y, int z);
int GetLegacyLightLevel(int x, int y, int z);
float ClampLightFloat(float v);
float LegacyLevelToBrightness(int level);
float GetLegacyFaceBrightness(int x, int y, int z, int face, int block);
float GetLegacyFaceShade(int face);
float VertexAOFromBlocks(int s1x, int s1y, int s1z, int s2x, int s2y, int s2z, int cx, int cy, int cz);
void ComputeFaceAO(int x, int y, int z, int face, float *a0, float *a1, float *a2, float *a3);
void EmitLitVertex(float u, float v, float x, float y, float z, float brightness);

void DrawCrosshair(void);

void InitSurvival(void);
int BlockToItem(int block);
int GetBlockDropItemAt(int block, int x, int y, int z, int *countOut);
int ItemToBlock(int item);
int AddItemToSlot(InventorySlot *slot, int item, int count);
int AddItemToInventory(int item, int count);
int RemoveItemFromSelectedHotbar(int count);
void SelectHotbarSlot(int slot);

void TakeDamage(int amount);

void DrawSurvivalUI(void);
void InitDroppedItems(void);
int AddDroppedItem(int item, int count, double x, double y, double z, double vx, double vy, double vz);
void DropSelectedItem(void);
void UpdateDroppedItems(double dt);
void RenderDroppedItems(void);
void DrawDroppedItem(DroppedItem *d);
void DrawDroppedBlockCube(int block);
void DrawTorchBlock(int x, int y, int z);
void DrawPlayerThirdPerson(void);
void DrawBuildingTerrainScreen(const char *message);
void DrawHearts(void);
void DrawHotbar(void);
void DrawInventoryScreen(void);
void DrawInventorySlot(int x, int y, InventorySlot slot, int selected);
int GetInventorySlotAtPoint(int mx, int my, int *isHotbar);
void DrawCarriedInventoryStack(void);
void HandleInventoryClick(int mx, int my);
void HandleInventoryRightClick(int mx, int my);
void InventoryMouseRightClick(int mx, int my);

void DrawHealthHearts(void)
{
    DrawHearts();
}

void DrawHearts(void);
void DrawHotbar(void);
void DrawInventoryScreen(void);

void DrawInventorySlot(int x, int y, InventorySlot slot, int selected);



/* ------------------------------------------------------------ */
/* Main program                                                 */
/* ------------------------------------------------------------ */

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    if (!InitWindow(hInstance, nCmdShow)) {
        MessageBox(NULL, "Failed to create window.", "Error", MB_OK);
        return 0;
    }

    if (!InitOpenGL()) {
        MessageBox(NULL, "Failed to initialize OpenGL.", "Error", MB_OK);
        return 0;
    }

    LoadGameTextures();
    CreateFonts();

    LoadWorldList();
    EnterMenu();

    MainLoop();

    DeleteFonts();
    DeleteGameTextures();
    ShutdownOpenGL();

    return 0;
}

/* ------------------------------------------------------------ */
/* Win32 window creation                                        */
/* ------------------------------------------------------------ */

int InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASS wc;

    g_hInst = hInstance;

    ZeroMemory(&wc, sizeof(wc));

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "CloneMCWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        return 0;
    }

    g_hwnd = CreateWindow(
        "CloneMCWindow",
        "CloneMC",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!g_hwnd) {
        return 0;
    }

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    return 1;
}


/* PATCH_F11_MOB_GUI_FULLSCREEN_IMPL
   F11 fullscreen/windowed toggle.
   Uses only old Win32 calls available to Open Watcom/Win98 headers. */
void ApplyWindowResizeState(void)
{
    if (g_windowWidth <= 0) { g_windowWidth = WINDOW_WIDTH; }
    if (g_windowHeight <= 0) { g_windowHeight = WINDOW_HEIGHT; }
    glViewport(0, 0, g_windowWidth, g_windowHeight);
    LayoutBetaMenus();
}

void ToggleFullscreen(void)
{
    int sw;
    int sh;
    int ww;
    int wh;

    if (!g_hwnd) {
        return;
    }

    if (!g_isFullscreen) {
        g_windowedStyle = (DWORD)GetWindowLong(g_hwnd, GWL_STYLE);
        GetWindowRect(g_hwnd, &g_windowedRect);

        sw = GetSystemMetrics(SM_CXSCREEN);
        sh = GetSystemMetrics(SM_CYSCREEN);
        if (sw <= 0) { sw = WINDOW_WIDTH; }
        if (sh <= 0) { sh = WINDOW_HEIGHT; }

        SetWindowLong(g_hwnd, GWL_STYLE,
                      (LONG)(g_windowedStyle & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU)));
        SetWindowPos(g_hwnd, HWND_TOP, 0, 0, sw, sh,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        g_windowWidth = sw;
        g_windowHeight = sh;
        g_isFullscreen = 1;
    } else {
        SetWindowLong(g_hwnd, GWL_STYLE, (LONG)g_windowedStyle);
        ww = g_windowedRect.right - g_windowedRect.left;
        wh = g_windowedRect.bottom - g_windowedRect.top;
        if (ww <= 0) { ww = WINDOW_WIDTH; }
        if (wh <= 0) { wh = WINDOW_HEIGHT; }
        SetWindowPos(g_hwnd, HWND_TOP,
                     g_windowedRect.left, g_windowedRect.top, ww, wh,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        g_windowWidth = ww;
        g_windowHeight = wh;
        g_isFullscreen = 0;
    }

    ApplyWindowResizeState();
    if (g_state == STATE_GAME && !inventoryOpen) {
        LockMouseForGame();
    }
}

/* ------------------------------------------------------------ */
/* OpenGL initialization                                        */
/* ------------------------------------------------------------ */

int InitOpenGL(void)
{
    PIXELFORMATDESCRIPTOR pfd;
    int pixelFormat;

    g_hdc = GetDC(g_hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL |
                  PFD_DOUBLEBUFFER;

    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    pixelFormat = ChoosePixelFormat(g_hdc, &pfd);

    if (!pixelFormat) {
        return 0;
    }

    if (!SetPixelFormat(g_hdc, pixelFormat, &pfd)) {
        return 0;
    }

    g_glrc = wglCreateContext(g_hdc);

    if (!g_glrc) {
        return 0;
    }

    if (!wglMakeCurrent(g_hdc, g_glrc)) {
        return 0;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.45f, 0.70f, 1.0f, 1.0f);

    return 1;
}

void ShutdownOpenGL(void)
{
    StopAllMusic();
    StopMobSounds();
    UnlockMouseFromGame();

    if (g_glrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_glrc);
        g_glrc = NULL;
    }

    if (g_hwnd && g_hdc) {
        ReleaseDC(g_hwnd, g_hdc);
        g_hdc = NULL;
    }
}


/* ------------------------------------------------------------ */
/* Bitmap font setup                                            */
/* ------------------------------------------------------------ */

void CreateFonts(void)
{
    HFONT normalFont;
    HFONT titleFont;
    HFONT oldFont;

    fontBaseNormal = glGenLists(96);
    fontBaseTitle = glGenLists(96);

    normalFont = CreateFont(
        22, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "Arial"
    );

    oldFont = (HFONT)SelectObject(g_hdc, normalFont);
    wglUseFontBitmaps(g_hdc, 32, 96, fontBaseNormal);
    SelectObject(g_hdc, oldFont);
    DeleteObject(normalFont);

    titleFont = CreateFont(
        64, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "Arial"
    );

    oldFont = (HFONT)SelectObject(g_hdc, titleFont);
    wglUseFontBitmaps(g_hdc, 32, 96, fontBaseTitle);
    SelectObject(g_hdc, oldFont);
    DeleteObject(titleFont);
}

void DeleteFonts(void)
{
    if (fontBaseNormal) {
        glDeleteLists(fontBaseNormal, 96);
        fontBaseNormal = 0;
    }

    if (fontBaseTitle) {
        glDeleteLists(fontBaseTitle, 96);
        fontBaseTitle = 0;
    }
}

void DrawText2D(GLuint base, int x, int y, const char *text)
{
    int len;

    if (!text) {
        return;
    }

    len = (int)strlen(text);
    glRasterPos2i(x, y);
    glListBase(base - 32);
    glCallLists(len, GL_UNSIGNED_BYTE, text);
}



/* ------------------------------------------------------------ */
/* Beta-style pixel font                                        */
/* ------------------------------------------------------------ */

int BlockyScaleForBase(GLuint base)
{
    if (base == fontBaseTitle) {
        return 7;
    }
    return 2;
}

int BlockyCharWidth(GLuint base)
{
    return 8 * BlockyScaleForBase(base);
}

int BlockyGlyphRow(char c, int row)
{
    static const int sp[7] = {0,0,0,0,0,0,0};
    static const int qn[7] = {14,17,1,2,4,0,4};
    static const int ex[7] = {4,4,4,4,4,0,4};
    static const int dt[7] = {0,0,0,0,0,0,4};
    static const int co[7] = {0,4,4,0,4,4,0};
    static const int da[7] = {0,0,0,31,0,0,0};
    static const int sl[7] = {1,1,2,4,8,16,16};
    static const int us[7] = {0,0,0,0,0,0,31};
    static const int pl[7] = {0,4,4,31,4,4,0};
    static const int cm[7] = {0,0,0,0,0,4,8};
    static const int ap[7] = {4,4,8,0,0,0,0};
    static const int n0[7] = {14,17,19,21,25,17,14};
    static const int n1[7] = {4,12,4,4,4,4,14};
    static const int n2[7] = {14,17,1,2,4,8,31};
    static const int n3[7] = {30,1,1,14,1,1,30};
    static const int n4[7] = {2,6,10,18,31,2,2};
    static const int n5[7] = {31,16,16,30,1,1,30};
    static const int n6[7] = {6,8,16,30,17,17,14};
    static const int n7[7] = {31,1,2,4,8,8,8};
    static const int n8[7] = {14,17,17,14,17,17,14};
    static const int n9[7] = {14,17,17,15,1,2,12};
    static const int A[7] = {14,17,17,31,17,17,17};
    static const int B[7] = {30,17,17,30,17,17,30};
    static const int C[7] = {14,17,16,16,16,17,14};
    static const int D[7] = {30,17,17,17,17,17,30};
    static const int E[7] = {31,16,16,30,16,16,31};
    static const int F[7] = {31,16,16,30,16,16,16};
    static const int G[7] = {14,17,16,23,17,17,14};
    static const int H[7] = {17,17,17,31,17,17,17};
    static const int I[7] = {14,4,4,4,4,4,14};
    static const int J[7] = {7,2,2,2,18,18,12};
    static const int K[7] = {17,18,20,24,20,18,17};
    static const int L[7] = {16,16,16,16,16,16,31};
    static const int M[7] = {17,27,21,21,17,17,17};
    static const int N[7] = {17,25,21,19,17,17,17};
    static const int O[7] = {14,17,17,17,17,17,14};
    static const int P[7] = {30,17,17,30,16,16,16};
    static const int Q[7] = {14,17,17,17,21,18,13};
    static const int R[7] = {30,17,17,30,20,18,17};
    static const int S[7] = {15,16,16,14,1,1,30};
    static const int T[7] = {31,4,4,4,4,4,4};
    static const int U[7] = {17,17,17,17,17,17,14};
    static const int V[7] = {17,17,17,17,17,10,4};
    static const int W[7] = {17,17,17,21,21,21,10};
    static const int X[7] = {17,17,10,4,10,17,17};
    static const int Y[7] = {17,17,10,4,4,4,4};
    static const int Z[7] = {31,1,2,4,8,16,31};
    const int *p;

    if (row < 0 || row >= 7) {
        return 0;
    }

    if (c >= 'a' && c <= 'z') {
        c = (char)(c - 'a' + 'A');
    }

    p = qn;
    switch (c) {
    case ' ': p = sp; break;
    case '?': p = qn; break;
    case '!': p = ex; break;
    case '.': p = dt; break;
    case ':': p = co; break;
    case '-': p = da; break;
    case '/': p = sl; break;
    case '_': p = us; break;
    case '+': p = pl; break;
    case ',': p = cm; break;
    case '\'': p = ap; break;
    case '0': p = n0; break;
    case '1': p = n1; break;
    case '2': p = n2; break;
    case '3': p = n3; break;
    case '4': p = n4; break;
    case '5': p = n5; break;
    case '6': p = n6; break;
    case '7': p = n7; break;
    case '8': p = n8; break;
    case '9': p = n9; break;
    case 'A': p = A; break;
    case 'B': p = B; break;
    case 'C': p = C; break;
    case 'D': p = D; break;
    case 'E': p = E; break;
    case 'F': p = F; break;
    case 'G': p = G; break;
    case 'H': p = H; break;
    case 'I': p = I; break;
    case 'J': p = J; break;
    case 'K': p = K; break;
    case 'L': p = L; break;
    case 'M': p = M; break;
    case 'N': p = N; break;
    case 'O': p = O; break;
    case 'P': p = P; break;
    case 'Q': p = Q; break;
    case 'R': p = R; break;
    case 'S': p = S; break;
    case 'T': p = T; break;
    case 'U': p = U; break;
    case 'V': p = V; break;
    case 'W': p = W; break;
    case 'X': p = X; break;
    case 'Y': p = Y; break;
    case 'Z': p = Z; break;
    default: p = qn; break;
    }

    return p[row];
}



void DrawBetaFontGlyph2D(int ch, int x1, int y1, int x2, int y2)
{
    int sx;
    int sy;
    float u0;
    float v0;
    float u1;
    float v1;

    if (!texBetaFont) {
        return;
    }

    ch = ch & 255;
    sx = (ch & 15) * 8;
    sy = ((ch >> 4) & 15) * 8;

    u0 = (float)sx / 128.0f;
    v0 = (float)sy / 128.0f;
    u1 = (float)(sx + 8) / 128.0f;
    v1 = (float)(sy + 8) / 128.0f;

    glTexCoord2f(u0, v0); glVertex2i(x1, y1);
    glTexCoord2f(u1, v0); glVertex2i(x2, y1);
    glTexCoord2f(u1, v1); glVertex2i(x2, y2);
    glTexCoord2f(u0, v1); glVertex2i(x1, y2);
}

void DrawBetaFontText2D(GLuint base, int x, int y, const char *text)
{
    int scale;
    int i;
    int px;
    int top;

    if (!text || !texBetaFont) {
        return;
    }

    scale = BlockyScaleForBase(base);
    px = x;
    top = y - 8 * scale;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texBetaFont);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    for (i = 0; text[i] != '\0'; i++) {
        DrawBetaFontGlyph2D((int)((unsigned char)text[i]),
                            px, top, px + 8 * scale, top + 8 * scale);
        px += 8 * scale;
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void DrawBlockyText2D(GLuint base, int x, int y, const char *text)
{
    int scale;
    int i;
    int row;
    int col;
    int mask;
    int px;
    int top;
    int x0;
    int y0;

    if (!text) {
        return;
    }

    if (texBetaFont) {
        DrawBetaFontText2D(base, x, y, text);
        return;
    }

    scale = BlockyScaleForBase(base);
    px = x;
    top = y - 7 * scale;

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    for (i = 0; text[i] != '\0'; i++) {
        for (row = 0; row < 7; row++) {
            mask = BlockyGlyphRow(text[i], row);
            for (col = 0; col < 5; col++) {
                if (mask & (1 << (4 - col))) {
                    x0 = px + col * scale;
                    y0 = top + row * scale;
                    glVertex2i(x0, y0);
                    glVertex2i(x0 + scale, y0);
                    glVertex2i(x0 + scale, y0 + scale);
                    glVertex2i(x0, y0 + scale);
                }
            }
        }
        px += 8 * scale;
    }

    glEnd();
}

/* ------------------------------------------------------------ */
/* Texture loading and atlas helpers                            */
/* ------------------------------------------------------------ */

GLuint LoadTGATexture(const char *filename)
{
    FILE *file;
    unsigned char header[18];
    int width;
    int height;
    int bpp;
    int bytesPerPixel;
    int imageSize;
    unsigned char *rawData;
    unsigned char *rgbaData;
    int x;
    int y;
    int srcIndex;
    int dstIndex;
    int srcY;
    int originTop;
    GLuint texID;

    file = fopen(filename, "rb");

    if (!file) {
        MessageBox(NULL, filename, "Could not open TGA texture", MB_OK);
        return 0;
    }

    if (fread(header, 1, 18, file) != 18) {
        fclose(file);
        MessageBox(NULL, filename, "Invalid TGA header", MB_OK);
        return 0;
    }

    /*
        TGA image type 2 = uncompressed true-color.
    */
    if (header[2] != 2) {
        fclose(file);
        MessageBox(NULL, filename, "Only uncompressed TGA files are supported", MB_OK);
        return 0;
    }

    width = header[12] | (header[13] << 8);
    height = header[14] | (header[15] << 8);
    bpp = header[16];

    if (bpp != 24 && bpp != 32) {
        fclose(file);
        MessageBox(NULL, filename, "Only 24-bit or 32-bit TGA supported", MB_OK);
        return 0;
    }

    bytesPerPixel = bpp / 8;
    imageSize = width * height * bytesPerPixel;

    rawData = (unsigned char *)malloc(imageSize);

    if (!rawData) {
        fclose(file);
        MessageBox(NULL, filename, "Could not allocate TGA memory", MB_OK);
        return 0;
    }

    fread(rawData, 1, imageSize, file);
    fclose(file);

    rgbaData = (unsigned char *)malloc(width * height * 4);

    if (!rgbaData) {
        free(rawData);
        MessageBox(NULL, filename, "Could not allocate RGBA memory", MB_OK);
        return 0;
    }

    /*
        TGA stores pixels as BGR/BGRA.
        OpenGL wants RGBA.
        This also converts bottom-left TGA origin to top-left atlas rows.
    */
    originTop = (header[17] & 0x20) != 0;

    for (y = 0; y < height; y++) {
        if (originTop) {
            srcY = y;
        } else {
            srcY = height - 1 - y;
        }

        for (x = 0; x < width; x++) {
            srcIndex = (srcY * width + x) * bytesPerPixel;
            dstIndex = (y * width + x) * 4;

            rgbaData[dstIndex + 0] = rawData[srcIndex + 2];
            rgbaData[dstIndex + 1] = rawData[srcIndex + 1];
            rgbaData[dstIndex + 2] = rawData[srcIndex + 0];

            if (bytesPerPixel == 4) {
                rgbaData[dstIndex + 3] = rawData[srcIndex + 3];
            } else {
                rgbaData[dstIndex + 3] = 255;
            }
        }
    }

    free(rawData);

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /*
        GL_CLAMP works on OpenGL 1.1.
    */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        rgbaData
    );

    free(rgbaData);

    return texID;
}

void LoadGameTextures(void)
{
    texTerrain = LoadTGATexture("assets/terrain.tga");

    /*
        Keep icons.tga loaded for any older UI/icon functions.
        The new hearts do not use this atlas.
    */
    texIcons = LoadTGATexture("assets/icons.tga");

    /*
        These two files are created from your uploaded heart PNGs.
        Full heart = health present.
        Empty heart = missing health.
    */
    texHeartFull = LoadTGATexture("assets/heart_full.tga");
    texHeartEmpty = LoadTGATexture("assets/heart_empty.tga");

    texBetaLogo = LoadTGATexture("assets/beta/mclogo.tga");
    texBetaMenuBackground = LoadTGATexture("assets/beta/menu_background.tga");
    texBetaGui = LoadTGATexture("assets/beta/gui.tga");
    texBetaInventory = LoadTGATexture("assets/beta/inventory.tga");
    texBetaCrafting = LoadTGATexture("assets/beta/crafting.tga");
    texBetaItems = LoadTGATexture("assets/beta/items.tga");
    texBetaIcons = LoadTGATexture("assets/beta/icons.tga");
    texBetaParticles = LoadTGATexture("assets/beta/particles.tga");
    texBetaSun = LoadTGATexture("assets/beta/sun.tga");
    texBetaMoon = LoadTGATexture("assets/beta/moon.tga");
    texBetaRain = LoadTGATexture("assets/beta/rain.tga");
    texBetaSnow = LoadTGATexture("assets/beta/snow.tga");
    texBetaShadow = LoadTGATexture("assets/beta/shadow.tga");
    texBetaVignette = LoadTGATexture("assets/beta/vignette.tga");
    texBetaWater = LoadTGATexture("assets/beta/water.tga");
    texBetaFont = LoadTGATexture("assets/beta/font_default.tga");


    texMobChicken = LoadTGATexture("assets/mobs/chicken.tga");
    texMobCow = LoadTGATexture("assets/mobs/cow.tga");
    texMobSheep = LoadTGATexture("assets/mobs/sheep.tga");
    texMobSheepFur = LoadTGATexture("assets/mobs/sheep_fur.tga");
    texMobWolf = LoadTGATexture("assets/mobs/wolf.tga");
    texMobWolfAngry = LoadTGATexture("assets/mobs/wolf_angry.tga");
    texMobWolfTame = LoadTGATexture("assets/mobs/wolf_tame.tga");
    texMobSquid = LoadTGATexture("assets/mobs/squid.tga");
    texMobZombie = LoadTGATexture("assets/mobs/zombie.tga");
    texMobSkeleton = LoadTGATexture("assets/mobs/skeleton.tga");
    texMobCreeper = LoadTGATexture("assets/mobs/creeper.tga");
    texMobSpider = LoadTGATexture("assets/mobs/spider.tga");
    texMobSpiderEyes = LoadTGATexture("assets/mobs/spider_eyes.tga");
    texMobSlime = LoadTGATexture("assets/mobs/slime.tga");
    texMobPig = LoadTGATexture("assets/mobs/pig.tga");
    texMobPlayer = LoadTGATexture("assets/mobs/player.tga");

    glEnable(GL_TEXTURE_2D);
}


void DeleteGameTextures(void)
{
    DeleteTerrainChunkMeshes();

    if (texTerrain) {
        glDeleteTextures(1, &texTerrain);
        texTerrain = 0;
    }

    if (texIcons) {
        glDeleteTextures(1, &texIcons);
        texIcons = 0;
    }

    if (texBetaLogo) { glDeleteTextures(1, &texBetaLogo); texBetaLogo = 0; }
    if (texBetaMenuBackground) { glDeleteTextures(1, &texBetaMenuBackground); texBetaMenuBackground = 0; }
    if (texBetaGui) { glDeleteTextures(1, &texBetaGui); texBetaGui = 0; }
    if (texBetaInventory) { glDeleteTextures(1, &texBetaInventory); texBetaInventory = 0; }
    if (texBetaCrafting) { glDeleteTextures(1, &texBetaCrafting); texBetaCrafting = 0; }
    if (texBetaItems) { glDeleteTextures(1, &texBetaItems); texBetaItems = 0; }
    if (texBetaIcons) { glDeleteTextures(1, &texBetaIcons); texBetaIcons = 0; }
    if (texBetaParticles) { glDeleteTextures(1, &texBetaParticles); texBetaParticles = 0; }
    if (texBetaSun) { glDeleteTextures(1, &texBetaSun); texBetaSun = 0; }
    if (texBetaMoon) { glDeleteTextures(1, &texBetaMoon); texBetaMoon = 0; }
    if (texBetaRain) { glDeleteTextures(1, &texBetaRain); texBetaRain = 0; }
    if (texBetaSnow) { glDeleteTextures(1, &texBetaSnow); texBetaSnow = 0; }
    if (texBetaShadow) { glDeleteTextures(1, &texBetaShadow); texBetaShadow = 0; }
    if (texBetaVignette) { glDeleteTextures(1, &texBetaVignette); texBetaVignette = 0; }
    if (texBetaWater) { glDeleteTextures(1, &texBetaWater); texBetaWater = 0; }
    if (texBetaFont) { glDeleteTextures(1, &texBetaFont); texBetaFont = 0; }

    if (texHeartFull) {
        glDeleteTextures(1, &texHeartFull);
        texHeartFull = 0;
    }

    if (texHeartEmpty) {
        glDeleteTextures(1, &texHeartEmpty);
        texHeartEmpty = 0;
    }


    if (texMobChicken) { glDeleteTextures(1, &texMobChicken); texMobChicken = 0; }
    if (texMobCow) { glDeleteTextures(1, &texMobCow); texMobCow = 0; }
    if (texMobSheep) { glDeleteTextures(1, &texMobSheep); texMobSheep = 0; }
    if (texMobSheepFur) { glDeleteTextures(1, &texMobSheepFur); texMobSheepFur = 0; }
    if (texMobWolf) { glDeleteTextures(1, &texMobWolf); texMobWolf = 0; }
    if (texMobWolfAngry) { glDeleteTextures(1, &texMobWolfAngry); texMobWolfAngry = 0; }
    if (texMobWolfTame) { glDeleteTextures(1, &texMobWolfTame); texMobWolfTame = 0; }
    if (texMobSquid) { glDeleteTextures(1, &texMobSquid); texMobSquid = 0; }
    if (texMobZombie) { glDeleteTextures(1, &texMobZombie); texMobZombie = 0; }
    if (texMobSkeleton) { glDeleteTextures(1, &texMobSkeleton); texMobSkeleton = 0; }
    if (texMobCreeper) { glDeleteTextures(1, &texMobCreeper); texMobCreeper = 0; }
    if (texMobSpider) { glDeleteTextures(1, &texMobSpider); texMobSpider = 0; }
    if (texMobSpiderEyes) { glDeleteTextures(1, &texMobSpiderEyes); texMobSpiderEyes = 0; }
    if (texMobSlime) { glDeleteTextures(1, &texMobSlime); texMobSlime = 0; }
    if (texMobPig) { glDeleteTextures(1, &texMobPig); texMobPig = 0; }
    if (texMobPlayer) { glDeleteTextures(1, &texMobPlayer); texMobPlayer = 0; }
}


void GetTileUVEx(int col, int row, int atlasWidth, int atlasHeight, float *u0, float *v0, float *u1, float *v1)
{
    /*
        Converts tile coordinates into OpenGL UV coordinates.

        col,row are measured from the top-left of the atlas.
        pad avoids texture bleeding from neighboring tiles.
    */
    float pad;

    pad = 0.5f;

    *u0 = ((float)(col * TILE_SIZE) + pad) / (float)atlasWidth;
    *v0 = ((float)(row * TILE_SIZE) + pad) / (float)atlasHeight;

    *u1 = ((float)((col + 1) * TILE_SIZE) - pad) / (float)atlasWidth;
    *v1 = ((float)((row + 1) * TILE_SIZE) - pad) / (float)atlasHeight;
}

void GetTerrainTileUV(int col, int row, float *u0, float *v0, float *u1, float *v1)
{
    GetTileUVEx(col, row, TERRAIN_ATLAS_WIDTH, TERRAIN_ATLAS_HEIGHT, u0, v0, u1, v1);
}

void GetIconTileUV(int col, int row, float *u0, float *v0, float *u1, float *v1)
{
    GetTileUVEx(col, row, ICONS_ATLAS_WIDTH, ICONS_ATLAS_HEIGHT, u0, v0, u1, v1);
}

void GetBlockTile(int block, int face, int *col, int *row)
{
    if (block == BLOCK_GRASS) {
        if (face == 0) { *col = TILE_GRASS_TOP_COL; *row = TILE_GRASS_TOP_ROW; }
        else if (face == 1) { *col = TILE_DIRT_COL; *row = TILE_DIRT_ROW; }
        else { *col = TILE_GRASS_SIDE_COL; *row = TILE_GRASS_SIDE_ROW; }
        return;
    }

    if (block == BLOCK_DIRT) { *col = TILE_DIRT_COL; *row = TILE_DIRT_ROW; return; }
    if (block == BLOCK_STONE) { *col = TILE_STONE_COL; *row = TILE_STONE_ROW; return; }
    if (block == BLOCK_COBBLESTONE) { *col = TILE_COBBLE_COL; *row = TILE_COBBLE_ROW; return; }
    if (block == BLOCK_BORDER || block == BLOCK_BEDROCK) { *col = TILE_BEDROCK_COL; *row = TILE_BEDROCK_ROW; return; }
    if (block == BLOCK_PLANKS) { *col = TILE_PLANKS_COL; *row = TILE_PLANKS_ROW; return; }
    if (block == BLOCK_SAND) { *col = TILE_SAND_COL; *row = TILE_SAND_ROW; return; }
    if (block == BLOCK_GRAVEL) { *col = TILE_GRAVEL_COL; *row = TILE_GRAVEL_ROW; return; }
    if (block == BLOCK_LEAVES) { *col = TILE_LEAVES_COL; *row = TILE_LEAVES_ROW; return; }
    if (block == BLOCK_WOOL) { *col = TILE_WOOL_COL; *row = TILE_WOOL_ROW; return; }
    if (block == BLOCK_SNOW) { *col = TILE_SNOW_COL; *row = TILE_SNOW_ROW; return; }
    if (block == BLOCK_ICE) { *col = TILE_ICE_COL; *row = TILE_ICE_ROW; return; }

    if (block == BLOCK_WATER) {
        if (face == 0 || face == 1) { *col = TILE_WATER_STILL_COL; *row = TILE_WATER_STILL_ROW; }
        else { *col = TILE_WATER_FLOW_COL; *row = TILE_WATER_FLOW_ROW; }
        return;
    }

    if (block == BLOCK_WOOD) {
        if (face == 0 || face == 1) { *col = TILE_WOOD_TOP_COL; *row = TILE_WOOD_TOP_ROW; }
        else { *col = TILE_WOOD_SIDE_COL; *row = TILE_WOOD_SIDE_ROW; }
        return;
    }

    if (block == BLOCK_CACTUS) {
        if (face == 0) { *col = TILE_CACTUS_TOP_COL; *row = TILE_CACTUS_TOP_ROW; }
        else if (face == 1) { *col = TILE_CACTUS_BOTTOM_COL; *row = TILE_CACTUS_BOTTOM_ROW; }
        else { *col = TILE_CACTUS_SIDE_COL; *row = TILE_CACTUS_SIDE_ROW; }
        return;
    }

    if (block == BLOCK_SANDSTONE) {
        if (face == 0) { *col = TILE_SANDSTONE_TOP_COL; *row = TILE_SANDSTONE_TOP_ROW; }
        else if (face == 1) { *col = TILE_SANDSTONE_BOTTOM_COL; *row = TILE_SANDSTONE_BOTTOM_ROW; }
        else { *col = TILE_SANDSTONE_SIDE_COL; *row = TILE_SANDSTONE_SIDE_ROW; }
        return;
    }

    if (block == BLOCK_TORCH) { *col = TILE_TORCH_COL; *row = TILE_TORCH_ROW; return; }

    if (block == BLOCK_CHEST) { *col = TILE_CHEST_COL; *row = TILE_CHEST_ROW; return; }

    if (block == BLOCK_FURNACE) {
        if (face == 2 || face == 4) { *col = TILE_FURNACE_FRONT_COL; *row = TILE_FURNACE_FRONT_ROW; }
        else { *col = TILE_FURNACE_SIDE_COL; *row = TILE_FURNACE_SIDE_ROW; }
        return;
    }

    if (block == BLOCK_WORKBENCH) {
        if (face == 0) { *col = TILE_WORKBENCH_TOP_COL; *row = TILE_WORKBENCH_TOP_ROW; }
        else if (face == 1) { *col = TILE_PLANKS_COL; *row = TILE_PLANKS_ROW; }
        else if (face == 2 || face == 4) { *col = TILE_WORKBENCH_FRONT_COL; *row = TILE_WORKBENCH_FRONT_ROW; }
        else { *col = TILE_WORKBENCH_SIDE_COL; *row = TILE_WORKBENCH_SIDE_ROW; }
        return;
    }

    if (block == BLOCK_COAL_ORE) { *col = TILE_ORE_COAL_COL; *row = TILE_ORE_COAL_ROW; return; }
    if (block == BLOCK_IRON_ORE) { *col = TILE_ORE_IRON_COL; *row = TILE_ORE_IRON_ROW; return; }
    if (block == BLOCK_GOLD_ORE) { *col = TILE_ORE_GOLD_COL; *row = TILE_ORE_GOLD_ROW; return; }
    if (block == BLOCK_DIAMOND_ORE) { *col = TILE_ORE_DIAMOND_COL; *row = TILE_ORE_DIAMOND_ROW; return; }
    if (block == BLOCK_REDSTONE_ORE) { *col = TILE_ORE_REDSTONE_COL; *row = TILE_ORE_REDSTONE_ROW; return; }
    if (block == BLOCK_LAPIS_ORE) { *col = TILE_ORE_LAPIS_COL; *row = TILE_ORE_LAPIS_ROW; return; }

    if (block == BLOCK_LIGHT) { *col = TILE_WOOD_TOP_COL; *row = TILE_WOOD_TOP_ROW; return; }

    *col = TILE_DIRT_COL;
    *row = TILE_DIRT_ROW;
}


void DrawTexturedQuad2D(GLuint texture, int atlasWidth, int atlasHeight, int col, int row, int x1, int y1, int x2, int y2)
{
    float u0;
    float v0;
    float u1;
    float v1;

    GetTileUVEx(col, row, atlasWidth, atlasHeight, &u0, &v0, &u1, &v1);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    glTexCoord2f(u0, v0); glVertex2i(x1, y1);
    glTexCoord2f(u1, v0); glVertex2i(x2, y1);
    glTexCoord2f(u1, v1); glVertex2i(x2, y2);
    glTexCoord2f(u0, v1); glVertex2i(x1, y2);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void DrawImage2D(GLuint texture, int x1, int y1, int x2, int y2, float alpha)
{
    if (!texture) {
        return;
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(x1, y1);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(x2, y1);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(x2, y2);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(x1, y2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}


void DrawImageCrop2D(GLuint texture, int atlasW, int atlasH,
                     int sx, int sy, int sw, int sh,
                     int x1, int y1, int x2, int y2, float alpha)
{
    float u0;
    float v0;
    float u1;
    float v1;

    if (!texture) {
        return;
    }

    if (atlasW <= 0 || atlasH <= 0 || sw <= 0 || sh <= 0) {
        return;
    }

    u0 = (float)sx / (float)atlasW;
    v0 = (float)sy / (float)atlasH;
    u1 = (float)(sx + sw) / (float)atlasW;
    v1 = (float)(sy + sh) / (float)atlasH;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(u0, v0); glVertex2i(x1, y1);
    glTexCoord2f(u1, v0); glVertex2i(x2, y1);
    glTexCoord2f(u1, v1); glVertex2i(x2, y2);
    glTexCoord2f(u0, v1); glVertex2i(x1, y2);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void DrawImage3DBillboard(GLuint texture, float cx, float cy, float cz, float width, float height, float alpha)
{
    float halfW;
    float halfH;
    float rightX;
    float rightZ;
    float yawRad;
    if (!texture) {
        return;
    }
    halfW = width * 0.5f;
    halfH = height * 0.5f;
    yawRad = (float)(yaw * PI / 180.0);
    rightX = cos(yawRad) * halfW;
    rightZ = sin(yawRad) * halfW;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(cx - rightX, cy - halfH, cz - rightZ);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(cx + rightX, cy - halfH, cz + rightZ);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(cx + rightX, cy + halfH, cz + rightZ);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(cx - rightX, cy + halfH, cz - rightZ);
    glEnd();
}

void DrawTerrainTile2D(int col, int row, int x1, int y1, int x2, int y2)
{
    DrawTexturedQuad2D(texTerrain, TERRAIN_ATLAS_WIDTH, TERRAIN_ATLAS_HEIGHT, col, row, x1, y1, x2, y2);
}

void DrawIconTile2D(int col, int row, int x1, int y1, int x2, int y2)
{
    DrawTexturedQuad2D(texIcons, ICONS_ATLAS_WIDTH, ICONS_ATLAS_HEIGHT, col, row, x1, y1, x2, y2);
}

/* ------------------------------------------------------------ */
/* Main loop                                                    */
/* ------------------------------------------------------------ */

void MainLoop(void)
{
    MSG msg;
    DWORD now;
    double dt;

    lastTime = GetTickCount();

    while (g_running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_running = 0;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        now = GetTickCount();
        dt = (double)(now - lastTime) / 1000.0;
        lastTime = now;

        if (dt > 0.05) {
            dt = 0.05;
        }

        UpdateFPSCounter(dt);

        /*
            Music is updated for both menu and gameplay.
        */
        UpdateMusic(dt);
        UpdateFeatureGapSystems(dt);

        if (g_state == STATE_GAME) {
            GameUpdate(dt);
        }

        GameRender();

        Sleep(1);
    }
}

/* ------------------------------------------------------------ */
/* Input, mouse capture, safe spawn, and MP3 music              */
/* ------------------------------------------------------------ */

void ResetInputState(void)
{
    keyForward = 0;
    keyBack = 0;
    keyLeft = 0;
    keyRight = 0;
    keyJump = 0;
}

void SetGameKey(WPARAM key, int down)
{
    if (key == 'W') {
        keyForward = down;
    } else if (key == 'S') {
        keyBack = down;
    } else if (key == 'A') {
        keyLeft = down;
    } else if (key == 'D') {
        keyRight = down;
    } else if (key == VK_SPACE) {
        keyJump = down;
    }
}

void HideGameCursor(void)
{
    /*
        Force cursor hidden even if Windows' internal show-count is not zero.
    */
    while (ShowCursor(FALSE) >= 0) {
    }
}

void ShowGameCursor(void)
{
    /*
        Force cursor visible again when leaving gameplay/inventory.
    */
    while (ShowCursor(TRUE) < 0) {
    }
}

void LockMouseForGame(void)
{
    if (!g_hwnd) {
        return;
    }

    mouseLocked = 1;
    SetCapture(g_hwnd);
    HideGameCursor();
    CenterMouse();
}

void UnlockMouseFromGame(void)
{
    mouseLocked = 0;
    ReleaseCapture();
    ShowGameCursor();
}

int IsSkyOpenForSpawn(int bx, int by, int bz)
{
    int y;
    int block;

    if (bx < 2 || bx >= WORLD_X - 2 || bz < 2 || bz >= WORLD_Z - 2) {
        return 0;
    }

    for (y = by + 2; y < WORLD_Y - 1; y++) {
        block = GetBlock(bx, y, bz);

        /* Leaves are allowed so spawning near a tree does not fail. */
        if (block != BLOCK_AIR && block != BLOCK_LEAVES) {
            return 0;
        }
    }

    return 1;
}

void ForceSpawnPad(int x, int y, int z)
{
    int dx;
    int dz;
    int yy;
    int px;
    int pz;
    int groundBlock;

    if (y < PLAYER_SPAWN_MIN_Y) {
        y = PLAYER_SPAWN_MIN_Y;
    }

    if (y > PLAYER_SPAWN_MAX_Y) {
        y = PLAYER_SPAWN_MAX_Y;
    }

    if (y + 4 >= WORLD_Y) {
        y = WORLD_Y - 6;
    }

    for (dx = -2; dx <= 2; dx++) {
        for (dz = -2; dz <= 2; dz++) {
            px = x + dx;
            pz = z + dz;

            if (!IsInsideWorld(px, y - 3, pz)) {
                continue;
            }

            if (dx == -2 || dx == 2 || dz == -2 || dz == 2) {
                groundBlock = BLOCK_COBBLESTONE;
            } else if ((dx == 0 && dz == 0) || (WorldHash3D(px, y, pz, g_worldSeed + 28001) & 1)) {
                groundBlock = BLOCK_GRASS;
            } else {
                groundBlock = BLOCK_DIRT;
            }

            world[px][y - 1][pz] = groundBlock;
            world[px][y - 2][pz] = BLOCK_DIRT;
            world[px][y - 3][pz] = BLOCK_STONE;

            for (yy = y; yy <= y + 4 && yy < WORLD_Y; yy++) {
                world[px][yy][pz] = BLOCK_AIR;
            }

            RebuildColumnTopAt(px, pz);
        }
    }
}



int IsSpawnSpaceClear(double sx, double sy, double sz)
{
    int bx;
    int by;
    int bz;
    int feet;
    int head;
    int above;
    int ground;

    bx = (int)floor(sx);
    by = (int)floor(sy);
    bz = (int)floor(sz);

    if (by < PLAYER_SPAWN_MIN_Y || by > PLAYER_SPAWN_MAX_Y || by + 3 >= WORLD_Y) {
        return 0;
    }

    if (bx < 3 || bz < 3 || bx >= WORLD_X - 3 || bz >= WORLD_Z - 3) {
        return 0;
    }

    ground = GetBlock(bx, by - 1, bz);

    if (!IsValidSpawnGround(ground)) {
        return 0;
    }

    feet = GetBlock(bx, by, bz);
    head = GetBlock(bx, by + 1, bz);
    above = GetBlock(bx, by + 2, bz);

    if (feet != BLOCK_AIR || head != BLOCK_AIR || above != BLOCK_AIR) {
        return 0;
    }

    if (!IsSkyOpenForSpawn(bx, by, bz)) {
        return 0;
    }

    if (PlayerCollidesAt(sx, sy, sz)) {
        return 0;
    }

    return 1;
}



int FindSafeSpawn(double *sx, double *sy, double *sz)
{
    int centerX;
    int centerZ;
    int radius;
    int x;
    int y;
    int z;
    int maxRadius;
    int bestX;
    int bestY;
    int bestZ;
    int candidateY;

    centerX = WORLD_X / 2;
    centerZ = WORLD_Z / 2;

    if (WORLD_X < WORLD_Z) {
        maxRadius = WORLD_X / 2 - 4;
    } else {
        maxRadius = WORLD_Z / 2 - 4;
    }

    if (maxRadius < 8) {
        maxRadius = 8;
    }

    bestX = centerX;
    bestZ = centerZ;
    bestY = 36 + (WorldHash2D(worldOriginBlockX, worldOriginBlockZ, g_worldSeed + 28100) % 11);

    if (bestY < PLAYER_SPAWN_MIN_Y) { bestY = PLAYER_SPAWN_MIN_Y; }
    if (bestY > PLAYER_SPAWN_MAX_Y) { bestY = PLAYER_SPAWN_MAX_Y; }

    for (radius = 0; radius < maxRadius; radius++) {
        for (x = centerX - radius; x <= centerX + radius; x++) {
            for (z = centerZ - radius; z <= centerZ + radius; z++) {
                if (x < 4 || z < 4 || x >= WORLD_X - 4 || z >= WORLD_Z - 4) {
                    continue;
                }

                if (abs(x - centerX) != radius && abs(z - centerZ) != radius) {
                    continue;
                }

                for (y = PLAYER_SPAWN_MAX_Y; y >= PLAYER_SPAWN_MIN_Y; y--) {
                    if (!IsValidSpawnGround(GetBlock(x, y - 1, z))) {
                        continue;
                    }

                    if (IsSpawnSpaceClear((double)x + 0.5, (double)y, (double)z + 0.5)) {
                        *sx = (double)x + 0.5;
                        *sy = (double)y;
                        *sz = (double)z + 0.5;
                        return 1;
                    }
                }
            }
        }
    }

    candidateY = bestY;
    ForceSpawnPad(bestX, candidateY, bestZ);

    *sx = (double)bestX + 0.5;
    *sy = (double)candidateY;
    *sz = (double)bestZ + 0.5;

    return 1;
}



void StopAllMusic(void)
{
    StopMobSounds();

    mciSendString("stop menuMusic", NULL, 0, NULL);
    mciSendString("close menuMusic", NULL, 0, NULL);

    mciSendString("stop gameMusic", NULL, 0, NULL);
    mciSendString("close gameMusic", NULL, 0, NULL);

    musicMode = 0;
}

int PlayMusicFile(const char *filename, const char *aliasName, int repeat)
{
    char cmd[512];
    MCIERROR err;

    sprintf(cmd, "close %s", aliasName);
    mciSendString(cmd, NULL, 0, NULL);

    /*
        type mpegvideo is the MCI type commonly used for MP3 playback.
        Files stay as MP3, not WAV.
    */
    sprintf(cmd, "open \"%s\" type mpegvideo alias %s", filename, aliasName);
    err = mciSendString(cmd, NULL, 0, NULL);

    if (err != 0) {
        return 0;
    }

    if (repeat) {
        sprintf(cmd, "play %s repeat", aliasName);
    } else {
        sprintf(cmd, "play %s", aliasName);
    }

    err = mciSendString(cmd, NULL, 0, NULL);

    if (err != 0) {
        sprintf(cmd, "close %s", aliasName);
        mciSendString(cmd, NULL, 0, NULL);
        return 0;
    }

    return 1;
}

int IsMusicAliasPlaying(const char *aliasName)
{
    char cmd[128];
    char status[64];
    MCIERROR err;

    status[0] = '\0';

    sprintf(cmd, "status %s mode", aliasName);
    err = mciSendString(cmd, status, sizeof(status), NULL);

    if (err != 0) {
        return 0;
    }

    if (strcmp(status, "playing") == 0) {
        return 1;
    }

    return 0;
}

void StartMenuMusic(void)
{
    if (musicMode == 1 && IsMusicAliasPlaying("menuMusic")) {
        return;
    }

    StopAllMusic();

    /*
        Menu uses Oxygene only.
    */
    if (PlayMusicFile(MUSIC_MENU_FILE, "menuMusic", 1)) {
        musicMode = 1;
    }
}

void StartGameMusic(void)
{
    int nextSong;
    const char *songs[GAME_MUSIC_COUNT];

    songs[0] = MUSIC_GAME_FILE1;
    songs[1] = MUSIC_GAME_FILE2;
    songs[2] = MUSIC_GAME_FILE3;
    songs[3] = MUSIC_GAME_FILE4;
    songs[4] = MUSIC_GAME_FILE5;
    songs[5] = MUSIC_GAME_FILE6;
    songs[6] = MUSIC_GAME_FILE7;
    songs[7] = MUSIC_GAME_FILE8;
    songs[8] = MUSIC_GAME_FILE9;
    songs[9] = MUSIC_GAME_FILE10;
    songs[10] = MUSIC_GAME_FILE11;
    songs[11] = MUSIC_GAME_FILE12;
    songs[12] = MUSIC_GAME_FILE13;
    songs[13] = MUSIC_GAME_FILE14;
    songs[14] = MUSIC_GAME_FILE15;
    songs[15] = MUSIC_GAME_FILE16;

    StopAllMusic();

    nextSong = (int)((GetTickCount() ^ WorldHash2D((int)playerX, (int)playerZ, g_worldSeed + currentGameSong * 97)) % GAME_MUSIC_COUNT);

    if (GAME_MUSIC_COUNT > 1 && nextSong == currentGameSong) {
        nextSong = (nextSong + 1 + (int)(GetTickCount() & 3)) % GAME_MUSIC_COUNT;
    }

    currentGameSong = nextSong;

    if (!PlayMusicFile(songs[currentGameSong], "gameMusic", 0)) {
        if (currentGameSong != 0) {
            PlayMusicFile(MUSIC_GAME_FILE1, "gameMusic", 0);
            currentGameSong = 0;
        }
    }

    musicMode = 2;
    gameMusicMinTimer = GAME_MUSIC_MIN_SECONDS;
}

void UpdateMusic(double dt)
{
    if (g_state == STATE_MENU || g_state == STATE_SETTINGS ||
        g_state == STATE_WORLD_SELECT || g_state == STATE_CREATE_WORLD ||
        g_state == STATE_OPTIONS) {
        if (musicMode != 1 || !IsMusicAliasPlaying("menuMusic")) {
            StartMenuMusic();
        }

        return;
    }

    if (g_state == STATE_GAME) {
        if (musicMode != 2) {
            StartGameMusic();
            return;
        }

        if (gameMusicMinTimer > 0.0) {
            gameMusicMinTimer -= dt;
        }

        /*
            If the current song has ended and at least a minute has passed,
            choose another gameplay song.
        */
        if (gameMusicMinTimer <= 0.0 && !IsMusicAliasPlaying("gameMusic")) {
            StartGameMusic();
        }
    }
}



/* ------------------------------------------------------------ */
/* Window messages                                              */
/* ------------------------------------------------------------ */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int mx;
    int my;
    int i;

    switch (msg) {
    case WM_SIZE:
        g_windowWidth = LOWORD(lParam);
        g_windowHeight = HIWORD(lParam);

        if (g_windowWidth <= 0) {
            g_windowWidth = 1;
        }
        if (g_windowHeight <= 0) {
            g_windowHeight = 1;
        }

        LayoutBetaMenus();

        return 0;

    case WM_SETFOCUS:
        if (g_state == STATE_GAME && !inventoryOpen) {
            LockMouseForGame();
        }

        return 0;

    case WM_KILLFOCUS:
        ResetInputState();
        UnlockMouseFromGame();
        SaveCurrentWorld();
        return 0;

    case WM_CLOSE:
        SaveCurrentWorld();
        g_running = 0;
        StopAllMusic();
        PostQuitMessage(0);
        return 0;

    case WM_DESTROY:
        SaveCurrentWorld();
        g_running = 0;
        StopAllMusic();
        PostQuitMessage(0);
        return 0;

    case WM_CHAR:
        if (g_state == STATE_CREATE_WORLD) {
            HandleCreateWorldChar(wParam);
            return 0;
        }

        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_F11) {
            ToggleFullscreen();
            return 0;
        }

        if (g_state == STATE_DEATH) {
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                RespawnPlayerAtWorldSpawn();
                return 0;
            }
            if (wParam == VK_ESCAPE) {
                SaveCurrentWorld();
                EnterMenu();
                return 0;
            }
            return 0;
        }

        if (g_state == STATE_GAME) {
            SetGameKey(wParam, 1);
        }

        if (wParam == VK_ESCAPE) {
            if (g_state == STATE_GAME) {
                if (inventoryOpen) {
                    if (craftingOpen) { CloseCraftingTable(); }
                    else { ReturnCraftingGridToInventory(); }
                    if (g_draggingInventory) { DropCarriedInventoryStackToWorld(0); }
                    inventoryOpen = 0;
                    LockMouseForGame();
                } else {
                    SaveCurrentWorld();
                    EnterPauseMenu();
                }
            } else if (g_state == STATE_PAUSE) {
                EnterGame();
            } else if (g_state == STATE_WORLD_SELECT) {
                EnterMenu();
            } else if (g_state == STATE_CREATE_WORLD) {
                EnterWorldSelect();
            } else if (g_state == STATE_OPTIONS || g_state == STATE_SETTINGS) {
                if (g_optionsReturnState == STATE_PAUSE) {
                    EnterPauseMenu();
                } else {
                    EnterMenu();
                }
            } else {
                SaveCurrentWorld();
                g_running = 0;
                StopAllMusic();
                PostQuitMessage(0);
            }

            return 0;
        }

        if (g_state == STATE_CREATE_WORLD) {
            if (wParam == VK_TAB) {
                createInputField = 1 - createInputField;
                return 0;
            }
        }

        if (g_state == STATE_GAME) {
            if (wParam == 'E') {
                if (craftingOpen) {
                    CloseCraftingTable();
                    if (g_draggingInventory) { DropCarriedInventoryStackToWorld(0); }
                    inventoryOpen = 0;
                }
                else {
                    if (inventoryOpen) {
                        ReturnCraftingGridToInventory();
                        if (g_draggingInventory) { DropCarriedInventoryStackToWorld(0); }
                    }
                    inventoryOpen = !inventoryOpen;
                }

                if (inventoryOpen) {
                    UnlockMouseFromGame();
                } else {
                    LockMouseForGame();
                }

                return 0;
            }

            if (wParam >= '1' && wParam <= '9') {
                SelectHotbarSlot((int)(wParam - '1'));
                return 0;
            }

            if (wParam == 'Q') {
                DropSelectedItem();
                return 0;
            }

            if (wParam == VK_F5) {
                g_cameraMode++;
                if (g_cameraMode > CAMERA_THIRD_FRONT) {
                    g_cameraMode = CAMERA_FIRST_PERSON;
                }
                return 0;
            }

            /* Quick render-distance cycle while testing performance. */
            if (wParam == 'R') {
                ChangeRenderDistance(1);
                return 0;
            }
        }

        return 0;

    case WM_KEYUP:
        if (g_state == STATE_GAME) {
            SetGameKey(wParam, 0);
        }

        return 0;

    case WM_LBUTTONDOWN:
        mx = LOWORD(lParam);
        my = HIWORD(lParam);

        if (g_state != STATE_GAME || inventoryOpen) {
            PlayUIClickSound();
        }

        if (g_state == STATE_MENU) {
            if (PointInRectInt(mx, my, singleplayerButton)) {
                EnterWorldSelect();
            } else if (PointInRectInt(mx, my, optionsButton)) {
                EnterOptions();
            } else if (PointInRectInt(mx, my, quitButton)) {
                SaveCurrentWorld();
                g_running = 0;
                StopAllMusic();
                PostQuitMessage(0);
            }
        } else if (g_state == STATE_WORLD_SELECT) {
            for (i = 0; i < MAX_WORLD_SLOTS; i++) {
                if (PointInRectInt(mx, my, worldSlotButtons[i])) {
                    selectedWorldSlot = i;
                    return 0;
                }
            }

            if (PointInRectInt(mx, my, worldPlayButton)) {
                StartWorldSlot(selectedWorldSlot);
            } else if (PointInRectInt(mx, my, worldCreateButton)) {
                EnterCreateWorld();
            } else if (PointInRectInt(mx, my, worldDeleteButton)) {
                DeleteWorldSlot(selectedWorldSlot);
            } else if (PointInRectInt(mx, my, worldBackButton)) {
                EnterMenu();
            }
        } else if (g_state == STATE_CREATE_WORLD) {
            if (PointInRectInt(mx, my, createNameField)) {
                createInputField = 0;
            } else if (PointInRectInt(mx, my, createSeedField)) {
                createInputField = 1;
            } else if (PointInRectInt(mx, my, createWorldSizeButton)) {
                g_createWorldSizeBlocks = FINITE_WORLD_SIZE_SMALL;
            } else if (PointInRectInt(mx, my, createWorldButton)) {
                CreateWorldFromMenu();
            } else if (PointInRectInt(mx, my, createCancelButton)) {
                EnterWorldSelect();
            }
        } else if (g_state == STATE_OPTIONS || g_state == STATE_SETTINGS) {
            if (PointInRectInt(mx, my, optionsRenderDistanceButton)) {
                SetRenderDistanceFromMouse(mx);
            } else if (PointInRectInt(mx, my, optionsDoneButton)) {
                if (g_optionsReturnState == STATE_PAUSE) {
                    EnterPauseMenu();
                } else {
                    EnterMenu();
                }
            }
        } else if (g_state == STATE_PAUSE) {
            if (PointInRectInt(mx, my, pauseContinueButton)) {
                EnterGame();
            } else if (PointInRectInt(mx, my, pauseOptionsButton)) {
                EnterOptions();
            } else if (PointInRectInt(mx, my, pauseExitButton)) {
                SaveCurrentWorld();
                EnterMenu();
            }
        } else if (g_state == STATE_DEATH) {
            if (PointInRectInt(mx, my, deathRespawnButton)) {
                RespawnPlayerAtWorldSpawn();
            } else if (PointInRectInt(mx, my, deathTitleButton)) {
                SaveCurrentWorld();
                EnterMenu();
            }
        } else if (g_state == STATE_GAME) {
            if (inventoryOpen) {
                if (craftingOpen) { CraftingMouseClick(mx, my); } else { HandleInventoryClick(mx, my); }
            } else {
                BreakBlockRaycast();
            }
        }

        return 0;

    case WM_RBUTTONDOWN:
        if (g_state == STATE_GAME) {
            if (inventoryOpen) {
                if (craftingOpen) {
                    CraftingMouseRightClick(LOWORD(lParam), HIWORD(lParam));
                } else {
                    HandleInventoryRightClick(LOWORD(lParam), HIWORD(lParam));
                }
            } else {
                PlaceBlockRaycast();
            }
        }

        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/* ------------------------------------------------------------ */
/* State switching                                              */
/* ------------------------------------------------------------ */

void LayoutBetaMenus(void)
{
    int centerX;
    int startY;
    int i;

    centerX = g_windowWidth / 2;

    SetRectXYWH(&singleplayerButton, centerX - 150, 240, 300, 44);
    SetRectXYWH(&multiplayerButton,  centerX - 150, 292, 300, 44);
    SetRectXYWH(&texturePackButton,  centerX - 150, 344, 300, 44);
    SetRectXYWH(&optionsButton,      centerX - 150, 405, 145, 44);
    SetRectXYWH(&quitButton,         centerX +   5, 405, 145, 44);

    startY = 150;
    for (i = 0; i < MAX_WORLD_SLOTS; i++) {
        SetRectXYWH(&worldSlotButtons[i], centerX - 260, startY + i * 54, 520, 48);
    }

    SetRectXYWH(&worldPlayButton,    centerX - 260, 455, 170, 42);
    SetRectXYWH(&worldCreateButton,  centerX -  85, 455, 170, 42);
    SetRectXYWH(&worldDeleteButton,  centerX +  90, 455, 170, 42);
    SetRectXYWH(&worldBackButton,    centerX -  85, 505, 170, 42);

    SetRectXYWH(&createNameField,    centerX - 210, 220, 420, 44);
    SetRectXYWH(&createSeedField,    centerX - 210, 300, 420, 44);
    SetRectXYWH(&createWorldSizeButton, centerX - 210, 370, 420, 42);
    SetRectXYWH(&createWorldButton,  centerX - 210, 435, 200, 42);
    SetRectXYWH(&createCancelButton, centerX +  10, 435, 200, 42);

    SetRectXYWH(&optionsRenderDistanceButton, centerX + 5, 263, 250, 38);
    SetRectXYWH(&optionsDoneButton,  centerX - 100, 430, 200, 42);

    SetRectXYWH(&pauseContinueButton, centerX - 150, 210, 300, 44);
    SetRectXYWH(&pauseOptionsButton,  centerX - 150, 264, 300, 44);
    SetRectXYWH(&pauseExitButton,     centerX - 150, 318, 300, 44);

    /* GuiGameOver-style button placement. */
    SetRectXYWH(&deathRespawnButton,  centerX - 100, g_windowHeight / 4 + 72, 200, 40);
    SetRectXYWH(&deathTitleButton,    centerX - 100, g_windowHeight / 4 + 120, 200, 40);
}

void SetRectXYWH(RECT *r, int x, int y, int w, int h)
{
    r->left = x;
    r->top = y;
    r->right = x + w;
    r->bottom = y + h;
}

void EnterMenu(void)
{
    g_state = STATE_MENU;
    inventoryOpen = 0;

    ResetInputState();
    UnlockMouseFromGame();
    StartMenuMusic();
    LayoutBetaMenus();
}

void EnterWorldSelect(void)
{
    g_state = STATE_WORLD_SELECT;
    inventoryOpen = 0;

    ResetInputState();
    UnlockMouseFromGame();
    StartMenuMusic();
    LoadWorldList();
    LayoutBetaMenus();
}

void EnterCreateWorld(void)
{
    int i;

    g_state = STATE_CREATE_WORLD;
    createInputField = 0;

    strcpy(newWorldName, "New World");
    newWorldSeedText[0] = '\0';

    for (i = 0; i < MAX_WORLD_SLOTS; i++) {
        if (!worldSaves[i].exists) {
            selectedWorldSlot = i;
            break;
        }
    }

    ResetInputState();
    UnlockMouseFromGame();
    StartMenuMusic();
    LayoutBetaMenus();
}

void EnterOptions(void)
{
    g_optionsReturnState = g_state;
    g_state = STATE_OPTIONS;
    ResetInputState();
    UnlockMouseFromGame();
    StartMenuMusic();
    LayoutBetaMenus();
}


void EnterSettings(void)
{
    EnterOptions();
}

void EnterPauseMenu(void)
{
    g_state = STATE_PAUSE;
    inventoryOpen = 0;

    ResetInputState();
    UnlockMouseFromGame();
    LayoutBetaMenus();
}

void RespawnPlayerAtWorldSpawn(void)
{
    double sx;
    double sy;
    double sz;

    if (FindSafeSpawn(&sx, &sy, &sz)) {
        playerX = sx;
        playerY = sy;
        playerZ = sz;
    } else {
        playerX = WORLD_X / 2;
        playerZ = WORLD_Z / 2;
        playerY = 36.0;
        ForceSpawnPad((int)playerX, (int)playerY, (int)playerZ);
    }

    playerHealth = MAX_HEALTH;
    playerPrevHealth = MAX_HEALTH;
    playerHeartsLife = 20.0;
    damageCooldown = 1.0;
    playerHurtFlash = 0.0;
    g_damageWobbleTimer = 0.0;
    velocityY = 0.0;
    lastVelocityY = 0.0;
    onGround = 1;
    g_playerAirTimer = 12.0;
    g_drownDamageTimer = 1.0;
    inventoryOpen = 0;
    craftingOpen = 0;
    ReturnCraftingGridToInventory();
    if (g_draggingInventory) { DropCarriedInventoryStackToWorld(0); }
    g_draggingInventory = 0;

    if (currentWorldSlot >= 0) { SaveCurrentWorld(); }
    EnterGame();
}

void EnterDeathScreen(void)
{
    g_state = STATE_DEATH;
    inventoryOpen = 0;
    craftingOpen = 0;
    ReturnCraftingGridToInventory();
    if (g_draggingInventory) { DropCarriedInventoryStackToWorld(0); }
    g_draggingInventory = 0;
    ResetInputState();
    UnlockMouseFromGame();
    playerHealth = 0;
    playerPrevHealth = 0;
    playerHeartsLife = 20.0;
    damageCooldown = 0.0;
    playerHurtFlash = 0.0;
    velocityY = 0.0;
    lastVelocityY = 0.0;
    LayoutBetaMenus();
}

void EnterGame(void)
{
    g_state = STATE_GAME;
    inventoryOpen = 0;

    ResetInputState();
    LockMouseForGame();
    StartGameMusic();
}


/* ------------------------------------------------------------ */
/* Beta-style save/load and seed handling                       */
/* ------------------------------------------------------------ */

void EnsureSaveDirectory(void)
{
    CreateDirectory("saves", NULL);
}

void GetWorldSavePath(int slot, char *path)
{
    sprintf(path, "saves\\World%d.dat", slot + 1);
}

void GetMobSavePath(int slot, char *path)
{
    sprintf(path, "saves\\World%d_mobs.dat", slot + 1);
}

void GetInventorySavePath(int slot, char *path)
{
    sprintf(path, "saves\\World%d_inventory.dat", slot + 1);
}

void GetDroppedItemsSavePath(int slot, char *path)
{
    sprintf(path, "saves\\World%d_drops.dat", slot + 1);
}

void GetBlockSavePath(int slot, char *path)
{
    sprintf(path, "saves\\World%d_blocks.rle", slot + 1);
}

void SanitizeWorldName(char *name)
{
    int i;

    if (!name || name[0] == '\0') {
        strcpy(name, "New World");
        return;
    }

    for (i = 0; name[i] != '\0'; i++) {
        if (name[i] == '\\' || name[i] == '/' || name[i] == ':' ||
            name[i] == '*' || name[i] == '?' || name[i] == '"' ||
            name[i] == '<' || name[i] == '>' || name[i] == '|') {
            name[i] = '_';
        }
    }
}

int SeedFromText(const char *text)
{
    unsigned int hash;
    int i;
    int sign;
    int value;
    int hasDigit;

    if (!text || text[0] == '\0') {
        return (int)(GetTickCount() ^ 0x5F3759DFu);
    }

    sign = 1;
    i = 0;
    value = 0;
    hasDigit = 0;

    if (text[0] == '-') {
        sign = -1;
        i = 1;
    }

    for (; text[i] != '\0'; i++) {
        if (text[i] < '0' || text[i] > '9') {
            hasDigit = 0;
            break;
        }

        hasDigit = 1;
        value = value * 10 + (text[i] - '0');
    }

    if (hasDigit) {
        return value * sign;
    }

    hash = 2166136261u;

    for (i = 0; text[i] != '\0'; i++) {
        hash ^= (unsigned char)text[i];
        hash *= 16777619u;
    }

    return (int)(hash & 0x7fffffff);
}

int LoadWorldSlot(int slot, WorldSaveInfo *info)
{
    FILE *f;
    char path[128];
    char line[256];
    char *p;

    if (!info) {
        return 0;
    }

    ZeroMemory(info, sizeof(WorldSaveInfo));
    info->seed = DEFAULT_WORLDGEN_SEED;
    info->worldSize = FINITE_WORLD_SIZE_SMALL;
    strcpy(info->seedText, "173773");
    info->playerGlobalX = 0.5;
    info->playerY = 80.0;
    info->playerGlobalZ = 0.5;
    info->worldTime = 300.0;

    GetWorldSavePath(slot, path);
    f = fopen(path, "r");

    if (!f) {
        return 0;
    }

    while (fgets(line, sizeof(line), f)) {
        p = strchr(line, '\n');
        if (p) {
            *p = '\0';
        }

        if (strncmp(line, "name=", 5) == 0) {
            strncpy(info->name, line + 5, WORLD_NAME_LEN - 1);
            info->name[WORLD_NAME_LEN - 1] = '\0';
        } else if (strncmp(line, "seedText=", 9) == 0) {
            strncpy(info->seedText, line + 9, WORLD_SEED_LEN - 1);
            info->seedText[WORLD_SEED_LEN - 1] = '\0';
        } else if (strncmp(line, "seed=", 5) == 0) {
            info->seed = atoi(line + 5);
        } else if (strncmp(line, "worldSize=", 10) == 0) {
            info->worldSize = atoi(line + 10);
            info->worldSize = FINITE_WORLD_SIZE_SMALL;
        } else if (strncmp(line, "playerGlobalX=", 14) == 0) {
            info->playerGlobalX = atof(line + 14);
        } else if (strncmp(line, "playerY=", 8) == 0) {
            info->playerY = atof(line + 8);
        } else if (strncmp(line, "playerGlobalZ=", 14) == 0) {
            info->playerGlobalZ = atof(line + 14);
        } else if (strncmp(line, "worldTime=", 10) == 0) {
            info->worldTime = atof(line + 10);
        }
    }

    fclose(f);

    if (info->name[0] == '\0') {
        sprintf(info->name, "World %d", slot + 1);
    }

    if (info->seedText[0] == '\0') {
        sprintf(info->seedText, "%d", info->seed);
    }

    info->exists = 1;
    return 1;
}

void LoadWorldList(void)
{
    int i;

    EnsureSaveDirectory();

    for (i = 0; i < MAX_WORLD_SLOTS; i++) {
        LoadWorldSlot(i, &worldSaves[i]);
    }

    if (selectedWorldSlot < 0) {
        selectedWorldSlot = 0;
    }

    if (selectedWorldSlot >= MAX_WORLD_SLOTS) {
        selectedWorldSlot = MAX_WORLD_SLOTS - 1;
    }
}

void SaveWorldSlotInfo(int slot)
{
    FILE *f;
    char path[128];
    WorldSaveInfo *info;

    if (slot < 0 || slot >= MAX_WORLD_SLOTS) {
        return;
    }

    info = &worldSaves[slot];

    if (!info->exists) {
        return;
    }

    EnsureSaveDirectory();
    GetWorldSavePath(slot, path);

    f = fopen(path, "w");

    if (!f) {
        return;
    }

    fprintf(f, "name=%s\n", info->name);
    fprintf(f, "seedText=%s\n", info->seedText);
    fprintf(f, "seed=%d\n", info->seed);
    fprintf(f, "worldSize=%d\n", info->worldSize);
    fprintf(f, "playerGlobalX=%.3f\n", info->playerGlobalX);
    fprintf(f, "playerY=%.3f\n", info->playerY);
    fprintf(f, "playerGlobalZ=%.3f\n", info->playerGlobalZ);
    fprintf(f, "worldTime=%.3f\n", info->worldTime);

    fclose(f);
}


void SaveCurrentMobs(void)
{
    FILE *f;
    char path[128];
    int i;
    double gx;
    double gz;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) {
        return;
    }

    EnsureSaveDirectory();
    GetMobSavePath(currentWorldSlot, path);
    f = fopen(path, "w");

    if (!f) {
        return;
    }

    fprintf(f, "# CloneMC mob save: type globalX y globalZ health angry sheared fuseTimer\n");

    for (i = 0; i < MAX_MOBS; i++) {
        if (!mobs[i].active) {
            continue;
        }

        gx = (double)worldOriginBlockX + mobs[i].x;
        gz = (double)worldOriginBlockZ + mobs[i].z;

        fprintf(f, "%d %.3f %.3f %.3f %d %d %d %.3f\n",
                mobs[i].type,
                gx,
                mobs[i].y,
                gz,
                mobs[i].health,
                mobs[i].angry,
                mobs[i].sheared,
                mobs[i].fuseTimer);
    }

    fclose(f);
}

int LoadCurrentMobs(void)
{
    FILE *f;
    char path[128];
    char line[256];
    int type;
    int health;
    int angry;
    int sheared;
    int idx;
    double gx;
    double gy;
    double gz;
    double fuse;
    double lx;
    double lz;
    int loaded;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) {
        return 0;
    }

    GetMobSavePath(currentWorldSlot, path);
    f = fopen(path, "r");

    if (!f) {
        return 0;
    }

    InitMobs();
    loaded = 0;

    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#') {
            continue;
        }

        if (sscanf(line, "%d %lf %lf %lf %d %d %d %lf",
                   &type, &gx, &gy, &gz, &health, &angry, &sheared, &fuse) != 8) {
            continue;
        }

        lx = gx - (double)worldOriginBlockX;
        lz = gz - (double)worldOriginBlockZ;

        if (lx < 1.0 || lz < 1.0 || lx > (double)(WORLD_X - 2) || lz > (double)(WORLD_Z - 2)) {
            continue;
        }

        idx = AddMob(type, lx, gy, lz);

        if (idx >= 0) {
            mobs[idx].health = health;
            mobs[idx].angry = angry;
            mobs[idx].sheared = sheared;
            mobs[idx].fuseTimer = fuse;
            mobs[idx].spawnGraceTimer = 0.5;
            loaded++;
        }
    }

    fclose(f);
    return loaded > 0;
}


/* ------------------------------------------------------------ */
/* Advanced Beta-style save components                           */
/* ------------------------------------------------------------ */

void SaveCurrentInventory(void)
{
    FILE *f;
    char path[128];
    int i;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    GetInventorySavePath(currentWorldSlot, path);
    f = fopen(path, "w");
    if (!f) { return; }

    fprintf(f, "CLONEMC_INVENTORY_V3\n");
    fprintf(f, "health %d\n", playerHealth);
    fprintf(f, "prevHealth %d\n", playerPrevHealth);
    fprintf(f, "heartsLife %.3f\n", playerHeartsLife);
    fprintf(f, "selected %d\n", selectedHotbarSlot);
    fprintf(f, "yaw %.6f\n", yaw);
    fprintf(f, "pitch %.6f\n", pitch);
    fprintf(f, "air %.3f\n", g_playerAirTimer);
    fprintf(f, "hotbar %d\n", HOTBAR_SLOTS);
    for (i = 0; i < HOTBAR_SLOTS; i++) {
        fprintf(f, "%d %d %d\n", hotbar[i].item, hotbar[i].count, hotbar[i].damage);
    }
    fprintf(f, "inventory %d\n", INVENTORY_SLOTS);
    for (i = 0; i < INVENTORY_SLOTS; i++) {
        fprintf(f, "%d %d %d\n", inventory[i].item, inventory[i].count, inventory[i].damage);
    }

    fclose(f);
}

int LoadCurrentInventory(void)
{
    FILE *f;
    char path[128];
    char key[64];
    int i;
    int count;
    int item;
    int stack;
    int damage;
    int invVersion;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    GetInventorySavePath(currentWorldSlot, path);
    f = fopen(path, "r");
    if (!f) { return 0; }

    if (fscanf(f, "%63s", key) != 1) { fclose(f); return 0; }
    invVersion = 2;
    if (strcmp(key, "CLONEMC_INVENTORY_V3") == 0) { invVersion = 3; }
    else if (strcmp(key, "CLONEMC_INVENTORY_V2") != 0) { fclose(f); return 0; }

    while (fscanf(f, "%63s", key) == 1) {
        if (strcmp(key, "health") == 0) {
            fscanf(f, "%d", &playerHealth);
            if (playerHealth < 0) { playerHealth = 0; }
            if (playerHealth > MAX_HEALTH) { playerHealth = MAX_HEALTH; }
        } else if (strcmp(key, "prevHealth") == 0) {
            fscanf(f, "%d", &playerPrevHealth);
        } else if (strcmp(key, "heartsLife") == 0) {
            fscanf(f, "%lf", &playerHeartsLife);
        } else if (strcmp(key, "selected") == 0) {
            fscanf(f, "%d", &selectedHotbarSlot);
            if (selectedHotbarSlot < 0) { selectedHotbarSlot = 0; }
            if (selectedHotbarSlot >= HOTBAR_SLOTS) { selectedHotbarSlot = HOTBAR_SLOTS - 1; }
        } else if (strcmp(key, "yaw") == 0) {
            fscanf(f, "%lf", &yaw);
        } else if (strcmp(key, "pitch") == 0) {
            fscanf(f, "%lf", &pitch);
        } else if (strcmp(key, "air") == 0) {
            fscanf(f, "%lf", &g_playerAirTimer);
        } else if (strcmp(key, "hotbar") == 0) {
            fscanf(f, "%d", &count);
            for (i = 0; i < HOTBAR_SLOTS && i < count; i++) {
                damage = 0;
                if (invVersion >= 3) {
                    if (fscanf(f, "%d %d %d", &item, &stack, &damage) != 3) { break; }
                } else {
                    if (fscanf(f, "%d %d", &item, &stack) != 2) { break; }
                }
                hotbar[i].item = item;
                hotbar[i].count = stack;
                hotbar[i].damage = damage;
                if (hotbar[i].count <= 0) { hotbar[i].item = ITEM_NONE; hotbar[i].count = 0; }
                if (hotbar[i].count > MAX_STACK) { hotbar[i].count = MAX_STACK; }
            }
            for (; i < count; i++) { if (invVersion >= 3) { fscanf(f, "%d %d %d", &item, &stack, &damage); } else { fscanf(f, "%d %d", &item, &stack); } }
        } else if (strcmp(key, "inventory") == 0) {
            fscanf(f, "%d", &count);
            for (i = 0; i < INVENTORY_SLOTS && i < count; i++) {
                damage = 0;
                if (invVersion >= 3) {
                    if (fscanf(f, "%d %d %d", &item, &stack, &damage) != 3) { break; }
                } else {
                    if (fscanf(f, "%d %d", &item, &stack) != 2) { break; }
                }
                inventory[i].item = item;
                inventory[i].count = stack;
                inventory[i].damage = damage;
                if (inventory[i].count <= 0) { inventory[i].item = ITEM_NONE; inventory[i].count = 0; }
                if (inventory[i].count > MAX_STACK) { inventory[i].count = MAX_STACK; }
            }
            for (; i < count; i++) { if (invVersion >= 3) { fscanf(f, "%d %d %d", &item, &stack, &damage); } else { fscanf(f, "%d %d", &item, &stack); } }
        }
    }

    fclose(f);
    return 1;
}

void SaveCurrentDroppedItems(void)
{
    FILE *f;
    char path[128];
    int i;
    double gx;
    double gz;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    GetDroppedItemsSavePath(currentWorldSlot, path);
    f = fopen(path, "w");
    if (!f) { return; }

    fprintf(f, "CLONEMC_DROPS_V2\n");
    for (i = 0; i < MAX_DROPPED_ITEMS; i++) {
        if (!droppedItems[i].active) { continue; }
        gx = (double)worldOriginBlockX + droppedItems[i].x;
        gz = (double)worldOriginBlockZ + droppedItems[i].z;
        fprintf(f, "%d %d %.4f %.4f %.4f %.5f %.5f %.5f %.3f %.3f\n",
                droppedItems[i].item, droppedItems[i].count,
                gx, droppedItems[i].y, gz,
                droppedItems[i].vx, droppedItems[i].vy, droppedItems[i].vz,
                droppedItems[i].age, droppedItems[i].spin);
    }
    fclose(f);
}

int LoadCurrentDroppedItems(void)
{
    FILE *f;
    char path[128];
    char magic[64];
    int item;
    int count;
    int loaded;
    double gx;
    double gy;
    double gz;
    double vx;
    double vy;
    double vz;
    double age;
    double spin;
    int idx;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    GetDroppedItemsSavePath(currentWorldSlot, path);
    f = fopen(path, "r");
    if (!f) { return 0; }
    if (fscanf(f, "%63s", magic) != 1 || strcmp(magic, "CLONEMC_DROPS_V2") != 0) {
        fclose(f); return 0;
    }

    InitDroppedItems();
    loaded = 0;
    while (fscanf(f, "%d %d %lf %lf %lf %lf %lf %lf %lf %lf",
                  &item, &count, &gx, &gy, &gz, &vx, &vy, &vz, &age, &spin) == 10) {
        idx = AddDroppedItem(item, count, gx - (double)worldOriginBlockX, gy, gz - (double)worldOriginBlockZ, vx, vy, vz);
        if (idx >= 0) {
            droppedItems[idx].age = age;
            droppedItems[idx].spin = spin;
            loaded++;
        }
    }
    fclose(f);
    return loaded > 0;
}

void SaveCurrentBlocks(void)
{
    FILE *f;
    char path[128];
    int x;
    int y;
    int z;
    int current;
    int last;
    int run;
    int first;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    GetBlockSavePath(currentWorldSlot, path);
    f = fopen(path, "w");
    if (!f) { return; }

    fprintf(f, "CLONEMC_BLOCKS_RLE_V2\n");
    fprintf(f, "origin %d %d size %d %d %d\n", worldOriginBlockX, worldOriginBlockZ, WORLD_X, WORLD_Y, WORLD_Z);

    first = 1;
    last = 0;
    run = 0;
    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                current = world[x][y][z];
                if (first) {
                    first = 0;
                    last = current;
                    run = 1;
                } else if (current == last && run < 32000) {
                    run++;
                } else {
                    fprintf(f, "%d %d\n", last, run);
                    last = current;
                    run = 1;
                }
            }
        }
    }
    if (!first) { fprintf(f, "%d %d\n", last, run); }
    fclose(f);
}

int LoadCurrentBlocks(void)
{
    FILE *f;
    char path[128];
    char magic[64];
    char key1[32];
    char key2[32];
    int savedOriginX;
    int savedOriginZ;
    int sx;
    int sy;
    int sz;
    int block;
    int run;
    int total;
    int n;
    int x;
    int y;
    int z;
    int index;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    GetBlockSavePath(currentWorldSlot, path);
    f = fopen(path, "r");
    if (!f) { return 0; }
    if (fscanf(f, "%63s", magic) != 1 || strcmp(magic, "CLONEMC_BLOCKS_RLE_V2") != 0) {
        fclose(f); return 0;
    }
    if (fscanf(f, "%31s %d %d %31s %d %d %d", key1, &savedOriginX, &savedOriginZ, key2, &sx, &sy, &sz) != 7) {
        fclose(f); return 0;
    }
    if (savedOriginX != worldOriginBlockX || savedOriginZ != worldOriginBlockZ ||
        sx != WORLD_X || sy != WORLD_Y || sz != WORLD_Z) {
        fclose(f); return 0;
    }

    total = WORLD_X * WORLD_Y * WORLD_Z;
    index = 0;
    while (index < total && fscanf(f, "%d %d", &block, &run) == 2) {
        for (n = 0; n < run && index < total; n++, index++) {
            z = index % WORLD_Z;
            y = (index / WORLD_Z) % WORLD_Y;
            x = index / (WORLD_Z * WORLD_Y);
            world[x][y][z] = block;
        }
    }

    fclose(f);
    RebuildColumnTops();
    InvalidateAllTerrainChunkMeshes();
    return index == total;
}

void SaveCurrentWorld(void)
{
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) {
        return;
    }

    if (!worldSaves[currentWorldSlot].exists) {
        return;
    }

    worldSaves[currentWorldSlot].seed = g_worldSeed;
    worldSaves[currentWorldSlot].worldSize = g_worldSizeBlocks;
    worldSaves[currentWorldSlot].playerGlobalX = GetPlayerGlobalX();
    worldSaves[currentWorldSlot].playerY = playerY;
    worldSaves[currentWorldSlot].playerGlobalZ = GetPlayerGlobalZ();
    worldSaves[currentWorldSlot].worldTime = g_worldTimeSeconds;

    SaveWorldSlotInfo(currentWorldSlot);
    SaveCurrentInventory();
    SaveCurrentDroppedItems();
    SaveCurrentBlocks();
    SaveCurrentMobs();
    SaveCurrentTileEntities();
    SaveCurrentRegionLite();
}

void DeleteWorldSlot(int slot)
{
    char path[128];

    if (slot < 0 || slot >= MAX_WORLD_SLOTS) {
        return;
    }

    GetWorldSavePath(slot, path);
    remove(path);
    GetMobSavePath(slot, path);
    remove(path);
    GetInventorySavePath(slot, path);
    remove(path);
    GetDroppedItemsSavePath(slot, path);
    remove(path);
    GetBlockSavePath(slot, path);
    remove(path);

    ZeroMemory(&worldSaves[slot], sizeof(WorldSaveInfo));

    if (currentWorldSlot == slot) {
        currentWorldSlot = -1;
    }

    LoadWorldList();
}

void StartNewWorldInSlot(int slot)
{
    WorldSaveInfo *info;

    if (slot < 0 || slot >= MAX_WORLD_SLOTS) {
        return;
    }

    info = &worldSaves[slot];
    ZeroMemory(info, sizeof(WorldSaveInfo));

    strncpy(info->name, newWorldName, WORLD_NAME_LEN - 1);
    info->name[WORLD_NAME_LEN - 1] = '\0';
    SanitizeWorldName(info->name);

    if (newWorldSeedText[0] == '\0') {
        sprintf(info->seedText, "%lu", (unsigned long)GetTickCount());
    } else {
        strncpy(info->seedText, newWorldSeedText, WORLD_SEED_LEN - 1);
        info->seedText[WORLD_SEED_LEN - 1] = '\0';
    }

    info->seed = SeedFromText(info->seedText);
    info->worldSize = FINITE_WORLD_SIZE_SMALL;
    info->playerGlobalX = 0.5;
    info->playerY = 80.0;
    info->playerGlobalZ = 0.5;
    info->worldTime = 300.0;
    info->exists = 1;

    selectedWorldSlot = slot;
    currentWorldSlot = slot;
    g_worldSeed = info->seed;
    g_worldSizeBlocks = FINITE_WORLD_SIZE_SMALL;
    g_worldSizeBlocks = FINITE_WORLD_SIZE_SMALL;
    g_worldAutosaveTimer = 0.0;
    g_startFromSavedPosition = 0;

    SaveWorldSlotInfo(slot);
    DrawBuildingTerrainScreen("Building terrain");
    {
        char mobPath[128];
        GetMobSavePath(slot, mobPath);
        remove(mobPath);
        GetInventorySavePath(slot, mobPath);
        remove(mobPath);
        GetDroppedItemsSavePath(slot, mobPath);
        remove(mobPath);
        GetBlockSavePath(slot, mobPath);
        remove(mobPath);
    }

    GameInit();
    EnterGame();
}

void CreateWorldFromMenu(void)
{
    int slot;
    int i;

    slot = selectedWorldSlot;

    if (slot < 0 || slot >= MAX_WORLD_SLOTS || worldSaves[slot].exists) {
        slot = -1;

        for (i = 0; i < MAX_WORLD_SLOTS; i++) {
            if (!worldSaves[i].exists) {
                slot = i;
                break;
            }
        }
    }

    if (slot < 0) {
        MessageBox(g_hwnd, "Delete a world first. All five save slots are full.", "No Empty Save Slot", MB_OK);
        return;
    }

    StartNewWorldInSlot(slot);
}

void StartWorldSlot(int slot)
{
    WorldSaveInfo info;

    if (slot < 0 || slot >= MAX_WORLD_SLOTS) {
        return;
    }

    if (!LoadWorldSlot(slot, &info)) {
        return;
    }

    worldSaves[slot] = info;
    selectedWorldSlot = slot;
    currentWorldSlot = slot;

    g_worldSeed = info.seed;
    g_worldSizeBlocks = info.worldSize;
    g_worldSizeBlocks = FINITE_WORLD_SIZE_SMALL;
    g_worldAutosaveTimer = 0.0;
    g_startFromSavedPosition = 1;
    g_startGlobalX = info.playerGlobalX;
    g_startPlayerY = info.playerY;
    g_startGlobalZ = info.playerGlobalZ;
    g_worldTimeSeconds = info.worldTime;

    DrawBuildingTerrainScreen("Loading world");
    GameInit();
    LoadCurrentMobs();
    EnterGame();
}

void HandleCreateWorldChar(WPARAM ch)
{
    char *target;
    int maxLen;
    int len;

    if (createInputField == 0) {
        target = newWorldName;
        maxLen = WORLD_NAME_LEN;
    } else {
        target = newWorldSeedText;
        maxLen = WORLD_SEED_LEN;
    }

    len = (int)strlen(target);

    if (ch == 8) {
        if (len > 0) {
            target[len - 1] = '\0';
        }

        return;
    }

    if (ch == 13) {
        CreateWorldFromMenu();
        return;
    }

    if (ch < 32 || ch > 126) {
        return;
    }

    if (len >= maxLen - 1) {
        return;
    }

    target[len] = (char)ch;
    target[len + 1] = '\0';
}

/* ------------------------------------------------------------ */
/* Game initialization                                          */
/* ------------------------------------------------------------ */

void GameInit(void)
{
    double sx;
    double sy;
    double sz;
    int loadingExisting;

    loadingExisting = g_startFromSavedPosition;

    GenerateWorld();

    if (loadingExisting) {
        if (!LoadCurrentRegionLite()) {
            LoadCurrentBlocks();
        }
    }

    /*
        Compute skylight and block light after world generation.
    */
    ComputeLegacyLighting();

    if (g_startFromSavedPosition) {
        playerX = g_startGlobalX - (double)worldOriginBlockX;
        playerY = g_startPlayerY;
        playerZ = g_startGlobalZ - (double)worldOriginBlockZ;

        if (!IsSpawnSpaceClear(playerX, playerY, playerZ)) {
            if (FindSafeSpawn(&sx, &sy, &sz)) {
                playerX = sx;
                playerY = sy;
                playerZ = sz;
            } else {
                playerX = WORLD_X / 2;
                playerZ = WORLD_Z / 2;
                playerY = WORLD_Y - 8;
            }
        }

        g_startFromSavedPosition = 0;
    } else {
        /*
            Find a real safe spawn instead of blindly using terrain height.
            This prevents spawning inside cliffs, under overhangs, or in water.
        */
        if (FindSafeSpawn(&sx, &sy, &sz)) {
            playerX = sx;
            playerY = sy;
            playerZ = sz;
        } else {
            /*
                Last-resort fallback: high in the air at world center.
            */
            playerX = WORLD_X / 2;
            playerZ = WORLD_Z / 2;
            playerY = WORLD_Y - 8;
        }
    }

    velocityY = 0.0;
    lastVelocityY = 0.0;

    if (!loadingExisting) {
        yaw = 0.0;
        pitch = 0.0;
    }

    g_playerLastAnimX = playerX;
    g_playerLastAnimZ = playerZ;
    handBob = 0.0;

    InitSurvival();
    if (loadingExisting) {
        LoadCurrentInventory();
    }
    InitDroppedItems();
    if (loadingExisting) {
        LoadCurrentDroppedItems();
    }
    InitParticles();
    InitMobs();
    InitFeatureGapSystems();
    if (loadingExisting) {
        LoadCurrentMobs();
        LoadCurrentTileEntities();
    } else {
        SpawnInitialMobs();
    }
}



/* ------------------------------------------------------------ */
/* Game update                                                  */
/* ------------------------------------------------------------ */

void GameUpdate(double dt)
{
    UpdateDayNightCycle(dt);
    UpdateClouds(dt);
    UpdateParticles(dt);
    UpdateDroppedItems(dt);

    /* Performance: mob AI is intentionally updated at 20 Hz instead of every rendered frame. */
    g_mobUpdateAccumulator += dt;
    if (g_mobUpdateAccumulator >= 0.15) {
        UpdateMobs(g_mobUpdateAccumulator);
        g_mobUpdateAccumulator = 0.0;
    }

    UpdatePlayerHandAnimation(dt);

    if (g_damageWobbleTimer > 0.0) {
        g_damageWobbleTimer -= dt;
        if (g_damageWobbleTimer < 0.0) {
            g_damageWobbleTimer = 0.0;
        }
    }

    /* Update invulnerability and hurt flash timers. */
    if (damageCooldown > 0.0) {
        damageCooldown -= dt;
        if (damageCooldown < 0.0) {
            damageCooldown = 0.0;
        }
    }

    if (playerHurtFlash > 0.0) {
        playerHurtFlash -= dt;
        if (playerHurtFlash < 0.0) {
            playerHurtFlash = 0.0;
        }
    }

    if (playerHeartsLife > 0.0) {
        playerHeartsLife -= dt * 20.0;
        if (playerHeartsLife < 0.0) { playerHeartsLife = 0.0; }
    }
    g_healthHudCounter++;

    if (!inventoryOpen) {
        UpdateMouseLook();
        HandleGameInput(dt);
    }

    UpdatePlayerWaterPhysics(dt);
    UpdateJavaStyleWaterFlow(dt);

    if (GetPlayerWaterImmersion() > 0.0) {
        /* Water drag/sink/swim is handled in UpdatePlayerWaterPhysics(). */
    } else {
        velocityY -= GRAVITY * dt;
    }

    onGround = 0;

    lastVelocityY = velocityY;

    if (MovePlayerAxis(0.0, velocityY * dt, 0.0)) {
        if (velocityY < 0.0) {
            onGround = 1;

            /*
                Simple fall damage.
                Only hurts when falling fast.
            */
            if (!IsPlayerInWater() && lastVelocityY < -11.0) {
                TakeDamage((int)((-lastVelocityY - 10.0) * 1.5));
            }
        }

        velocityY = 0.0;
    }

    ClampPlayerToFiniteWorld();

    UpdatePlayerMovementAnimation(dt);

    UpdateInfiniteWorldStreaming();

    if (currentWorldSlot >= 0) {
        g_worldAutosaveTimer += dt;
        if (g_worldAutosaveTimer >= 45.0) {
            g_worldAutosaveTimer = 0.0;
            SaveCurrentWorld();
        }
    }
}

/* ------------------------------------------------------------ */
/* Main renderer                                                */
/* ------------------------------------------------------------ */

void GameRender(void)
{
    glViewport(0, 0, g_windowWidth, g_windowHeight);

    ApplyDayNightClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_state == STATE_MENU) {
        DrawMenu();
    } else if (g_state == STATE_WORLD_SELECT) {
        DrawWorldSelect();
    } else if (g_state == STATE_CREATE_WORLD) {
        DrawCreateWorld();
    } else if (g_state == STATE_OPTIONS) {
        DrawOptions();
    } else if (g_state == STATE_SETTINGS) {
        DrawSettings();
    } else if (g_state == STATE_GAME || g_state == STATE_PAUSE || g_state == STATE_DEATH) {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(
            70.0,
            (double)g_windowWidth / (double)g_windowHeight,
            0.05,
            100.0
        );

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        SetupCamera();
        RenderSkyBodies();
        EnableBetaFog();
        RenderWorld();
        RenderDroppedItems();
        RenderMobs();
        if (g_cameraMode != CAMERA_FIRST_PERSON) {
            DrawPlayerThirdPerson();
        }
        RenderClouds();
        RenderParticles();
        DisableBetaFog();
        if (g_cameraMode == CAMERA_FIRST_PERSON) {
            RenderPlayerHand();
        }

        if (g_state != STATE_DEATH) {
            DrawCrosshair();
            DrawSurvivalUI();
            DrawWeather2D();
            DrawBetaVignette2D();
            DrawBetaStatus2D();
        }

        if (g_state == STATE_PAUSE) {
            DrawPauseMenu();
        }
        if (g_state == STATE_DEATH) {
            DrawDeathScreen();
        }
    }

    SwapBuffers(g_hdc);
}

/* ------------------------------------------------------------ */
/* 2D setup and menu drawing                                    */
/* ------------------------------------------------------------ */

void Setup2D(void)
{
    /* GUI/menu render reset.  Several 3D render paths enable blending,
       depth, culling, and tinted glColor values; if those leak into the
       menus, the buttons/backgrounds smear or draw with wrong colors. */
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, g_windowWidth, g_windowHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int EstimateTextWidth(GLuint base, const char *text)
{
    int len;

    if (!text) {
        return 0;
    }

    len = (int)strlen(text);

    if (base == fontBaseTitle) {
        return len * 36;
    }

    return len * 12;
}


void DrawCenteredText2D(GLuint base, int x1, int y1, int x2, int y2, const char *text)
{
    int w;
    int x;
    int y;

    w = EstimateTextWidth(base, text);
    x = x1 + ((x2 - x1) - w) / 2;

    if (base == fontBaseTitle) {
        y = y1 + ((y2 - y1) / 2) + 23;
    } else {
        y = y1 + ((y2 - y1) / 2) + 8;
    }

    DrawText2D(base, x, y, text);
}

void DrawImportedBetaLogo(void)
{
    int y;

    y = 74;

    glColor3f(0.04f, 0.04f, 0.04f);
    DrawCenteredText2D(fontBaseTitle, 0, y + 4, g_windowWidth, y + 92 + 4, "CLONEMC");

    glColor3f(0.86f, 0.86f, 0.86f);
    DrawCenteredText2D(fontBaseTitle, 0, y, g_windowWidth, y + 92, "CLONEMC");
}


void DrawBetaMenuFooter(void)
{
    /* Core Beta-style menu screens only: no extra build/debug notes. */
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}


void DrawBetaButtonPanel(RECT r, int hover, int disabled)
{
    int sy;

    if (texBetaGui) {
        if (disabled) {
            sy = 46;
        } else if (hover) {
            sy = 86;
        } else {
            sy = 66;
        }

        DrawImageCrop2D(texBetaGui, 256, 256, 0, sy, 200, 20,
                        r.left, r.top, r.right, r.bottom, 1.0f);
        return;
    }

    DrawRect2D(r.left, r.top, r.right, r.bottom, 0.04f, 0.04f, 0.04f);
    if (disabled) {
        DrawRect2D(r.left + 3, r.top + 3, r.right - 3, r.bottom - 3, 0.22f, 0.22f, 0.22f);
    } else if (hover) {
        DrawRect2D(r.left + 3, r.top + 3, r.right - 3, r.bottom - 3, 0.66f, 0.66f, 0.66f);
    } else {
        DrawRect2D(r.left + 3, r.top + 3, r.right - 3, r.bottom - 3, 0.36f, 0.36f, 0.36f);
    }
}

void DrawDisabledButton2D(RECT r, const char *text)
{
    DrawBetaButtonPanel(r, 0, 1);

    if (text && text[0] != '\0') {
        glColor3f(0.18f, 0.18f, 0.18f);
        DrawCenteredText2D(fontBaseNormal, r.left + 2, r.top + 2, r.right + 2, r.bottom + 2, text);
        glColor3f(0.55f, 0.55f, 0.55f);
        DrawCenteredText2D(fontBaseNormal, r.left, r.top, r.right, r.bottom, text);
    }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_TEXTURE_2D);
}


void DrawBetaVignette2D(void)
{
    int w;
    int h;
    int band;

    w = g_windowWidth;
    h = g_windowHeight;
    band = 96;

    if (w <= 0 || h <= 0) {
        return;
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);

    /* top fade */
    glColor4f(0.0f, 0.0f, 0.0f, 0.28f); glVertex2i(0, 0); glVertex2i(w, 0);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);  glVertex2i(w, band); glVertex2i(0, band);

    /* bottom fade */
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);  glVertex2i(0, h - band); glVertex2i(w, h - band);
    glColor4f(0.0f, 0.0f, 0.0f, 0.24f); glVertex2i(w, h); glVertex2i(0, h);

    /* left fade */
    glColor4f(0.0f, 0.0f, 0.0f, 0.22f); glVertex2i(0, 0); glVertex2i(0, h);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);  glVertex2i(band, h); glVertex2i(band, 0);

    /* right fade */
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);  glVertex2i(w - band, 0); glVertex2i(w - band, h);
    glColor4f(0.0f, 0.0f, 0.0f, 0.22f); glVertex2i(w, h); glVertex2i(w, 0);

    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void DrawWaterOverlay2D(void)
{
    int x;
    int y;
    int tile;
    int sx;
    int sy;
    float darkAlpha;
    float overlayAlpha;

    if (g_state != STATE_GAME) {
        return;
    }

    if (IsPlayerHeadUnderWater()) {
        darkAlpha = WATER_DARKEN_ALPHA_HEAD;
        overlayAlpha = WATER_OVERLAY_ALPHA_HEAD;
    } else if (IsPlayerInWater()) {
        darkAlpha = WATER_DARKEN_ALPHA_BODY;
        overlayAlpha = WATER_OVERLAY_ALPHA_BODY;
    } else {
        return;
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.03f, 0.18f, darkAlpha);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(g_windowWidth, 0);
    glVertex2i(g_windowWidth, g_windowHeight);
    glVertex2i(0, g_windowHeight);
    glEnd();

    if (texBetaWater) {
        tile = WATER_OVERLAY_TILE_PIXELS;
        sx = (int)(g_worldTimeSeconds * 18.0) & 63;
        sy = (int)(g_worldTimeSeconds * 11.0) & 63;
        for (y = -tile; y < g_windowHeight + tile; y += tile) {
            for (x = -tile; x < g_windowWidth + tile; x += tile) {
                DrawImageCrop2D(texBetaWater, 16, 16, 0, 0, 16, 16,
                                x + sx, y + sy, x + sx + tile, y + sy + tile,
                                overlayAlpha);
            }
        }
    }

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    g_vertexTintR = 1.0f;
    g_vertexTintG = 1.0f;
    g_vertexTintB = 1.0f;
    glEnable(GL_DEPTH_TEST);
}

void DrawOxygenBubbles(void)
{
    int air;
    int full;
    int partial;
    int icons;
    int x;
    int y;
    int i;
    int sx;

    if (!IsPlayerInWater()) {
        return;
    }

    air = (int)((g_playerAirTimer / 12.0) * 300.0);
    if (air < 0) { air = 0; }
    if (air > 300) { air = 300; }

    full = (int)ceil(((double)(air - 2) * 10.0) / 300.0);
    icons = (int)ceil(((double)air * 10.0) / 300.0);
    if (full < 0) { full = 0; }
    if (full > 10) { full = 10; }
    if (icons < 0) { icons = 0; }
    if (icons > 10) { icons = 10; }
    partial = icons - full;
    if (partial < 0) { partial = 0; }

    x = g_windowWidth / 2 - 91 * 2;
    y = g_windowHeight - 104;

    for (i = 0; i < icons; i++) {
        sx = (i < full) ? 16 : 25;
        DrawImageCrop2D(texBetaIcons, 256, 256, sx, 18, 9, 9,
                        x + i * BUBBLE_ICON_SIZE, y,
                        x + i * BUBBLE_ICON_SIZE + BUBBLE_ICON_SIZE,
                        y + BUBBLE_ICON_SIZE, 1.0f);
    }
}




void DrawMenu(void)
{
    POINT mouse;
    int hoverSingle;
    int hoverMulti;
    int hoverPack;
    int hoverOptions;
    int hoverQuit;
    Setup2D();
    LayoutBetaMenus();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    hoverSingle = PointInRectInt(mouse.x, mouse.y, singleplayerButton);
    hoverMulti = PointInRectInt(mouse.x, mouse.y, multiplayerButton);
    hoverPack = PointInRectInt(mouse.x, mouse.y, texturePackButton);
    hoverOptions = PointInRectInt(mouse.x, mouse.y, optionsButton);
    hoverQuit = PointInRectInt(mouse.x, mouse.y, quitButton);
    DrawDirtMenuBackground();
    DrawImportedBetaLogo();
    DrawButton2D(singleplayerButton, "Singleplayer", hoverSingle);
    DrawDisabledButton2D(multiplayerButton, "Multiplayer");
    DrawDisabledButton2D(texturePackButton, "Mods and Texture Packs");
    DrawButton2D(optionsButton, "Options...", hoverOptions);
    DrawButton2D(quitButton, "Quit Game", hoverQuit);
    DrawBetaMenuFooter();
    DrawBetaVignette2D();
}

void DrawWorldSelect(void)
{
    POINT mouse;
    char line[128];
    int i;
    int hover;

    Setup2D();
    LayoutBetaMenus();

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);

    DrawDirtMenuBackground();

    glColor3f(0.05f, 0.05f, 0.05f);
    DrawCenteredText2D(fontBaseTitle, 0, 55 + 4, g_windowWidth, 120 + 4, "Select World");

    glColor3f(0.85f, 0.85f, 0.85f);
    DrawCenteredText2D(fontBaseTitle, 0, 55, g_windowWidth, 120, "Select World");

    for (i = 0; i < MAX_WORLD_SLOTS; i++) {
        hover = PointInRectInt(mouse.x, mouse.y, worldSlotButtons[i]);

        if (i == selectedWorldSlot) {
            DrawRect2D(worldSlotButtons[i].left - 3, worldSlotButtons[i].top - 3,
                       worldSlotButtons[i].right + 3, worldSlotButtons[i].bottom + 3,
                       0.80f, 0.80f, 0.80f);
        }

        DrawButton2D(worldSlotButtons[i], "", hover);

        if (worldSaves[i].exists) {
            sprintf(line, "%s", worldSaves[i].name);
            glColor3f(1.0f, 1.0f, 1.0f);
            DrawText2D(fontBaseNormal, worldSlotButtons[i].left + 18, worldSlotButtons[i].top + 21, line);
        } else {
            glColor3f(0.65f, 0.65f, 0.65f);
            DrawText2D(fontBaseNormal, worldSlotButtons[i].left + 18, worldSlotButtons[i].top + 31, "Empty World Slot");
        }
    }

    DrawButton2D(worldPlayButton, "Play Selected", PointInRectInt(mouse.x, mouse.y, worldPlayButton));
    DrawButton2D(worldCreateButton, "Create New World", PointInRectInt(mouse.x, mouse.y, worldCreateButton));
    DrawButton2D(worldDeleteButton, "Delete", PointInRectInt(mouse.x, mouse.y, worldDeleteButton));
    DrawButton2D(worldBackButton, "Cancel", PointInRectInt(mouse.x, mouse.y, worldBackButton));
}

void DrawTextField2D(RECT r, const char *label, const char *value, int active)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText2D(fontBaseNormal, r.left, r.top - 10, label);

    DrawRect2D(r.left - 2, r.top - 2, r.right + 2, r.bottom + 2, 0.05f, 0.05f, 0.05f);

    if (active) {
        DrawRect2D(r.left, r.top, r.right, r.bottom, 0.22f, 0.22f, 0.22f);
    } else {
        DrawRect2D(r.left, r.top, r.right, r.bottom, 0.12f, 0.12f, 0.12f);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText2D(fontBaseNormal, r.left + 10, r.top + 30, value);

    if (active && ((GetTickCount() / 350) & 1)) {
        int cursorX;
        cursorX = r.left + 12 + EstimateTextWidth(fontBaseNormal, value);
        DrawText2D(fontBaseNormal, cursorX, r.top + 30, "_");
    }
}

void DrawCreateWorld(void)
{
    POINT mouse;

    Setup2D();
    LayoutBetaMenus();

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);

    DrawDirtMenuBackground();

    glColor3f(0.05f, 0.05f, 0.05f);
    DrawCenteredText2D(fontBaseTitle, 0, 65 + 4, g_windowWidth, 130 + 4, "Create New World");

    glColor3f(0.85f, 0.85f, 0.85f);
    DrawCenteredText2D(fontBaseTitle, 0, 65, g_windowWidth, 130, "Create New World");

    {
        char sizeLabel[96];
        DrawTextField2D(createNameField, "World Name", newWorldName, createInputField == 0);
        DrawTextField2D(createSeedField, "Seed for the World Generator", newWorldSeedText, createInputField == 1);
        sprintf(sizeLabel, "World Size: 862 x 862");
        glColor3f(0.80f, 0.80f, 0.80f);
        DrawCenteredText2D(fontBaseNormal, createWorldSizeButton.left, createWorldSizeButton.top,
                           createWorldSizeButton.right, createWorldSizeButton.bottom, sizeLabel);
    }


    DrawButton2D(createWorldButton, "Create New World", PointInRectInt(mouse.x, mouse.y, createWorldButton));
    DrawButton2D(createCancelButton, "Cancel", PointInRectInt(mouse.x, mouse.y, createCancelButton));
}

void DrawOptions(void)
{
    POINT mouse;
    RECT r;
    int centerX;
    int rowY;
    char label[96];
    Setup2D();
    LayoutBetaMenus();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    DrawImportedBetaLogo();
    centerX = g_windowWidth / 2;
    rowY = 215;
    SetRectXYWH(&r, centerX - 255, rowY, 250, 38);
    DrawDisabledButton2D(r, "Music: ON");
    SetRectXYWH(&r, centerX + 5, rowY, 250, 38);
    DrawDisabledButton2D(r, "Sound: ON");
    rowY += 48;
    SetRectXYWH(&r, centerX - 255, rowY, 250, 38);
    DrawDisabledButton2D(r, "Difficulty: Normal");

    /* Click this as a real render-distance slider: 1 through 20 chunks. */
    SetRectXYWH(&optionsRenderDistanceButton, centerX + 5, rowY, 250, 38);
    sprintf(label, "Render Distance: %s", RenderDistanceLabel());
    DrawButton2D(optionsRenderDistanceButton, label,
                 PointInRectInt(mouse.x, mouse.y, optionsRenderDistanceButton));
    DrawRect2D(optionsRenderDistanceButton.left + 18,
               optionsRenderDistanceButton.bottom - 10,
               optionsRenderDistanceButton.right - 18,
               optionsRenderDistanceButton.bottom - 6,
               0.10f, 0.10f, 0.10f);
    DrawRect2D(optionsRenderDistanceButton.left + 18,
               optionsRenderDistanceButton.bottom - 10,
               optionsRenderDistanceButton.left + 18 +
               ((g_renderDistanceChunks - 1) *
                (optionsRenderDistanceButton.right - optionsRenderDistanceButton.left - 36)) /
               (RENDER_DISTANCE_MAX_CHUNKS - 1),
               optionsRenderDistanceButton.bottom - 6,
               0.70f, 0.70f, 0.70f);

    rowY += 48;
    SetRectXYWH(&r, centerX - 255, rowY, 250, 38);
    DrawDisabledButton2D(r, "View Bobbing: ON");
    SetRectXYWH(&r, centerX + 5, rowY, 250, 38);
    DrawDisabledButton2D(r, "Fast Terrain Skin: ON");
    rowY += 48;
    SetRectXYWH(&r, centerX - 255, rowY, 250, 38);
    DrawDisabledButton2D(r, "Brightness: Normal");
    SetRectXYWH(&r, centerX + 5, rowY, 250, 38);
    DrawDisabledButton2D(r, "Particles: Minimal");
    DrawButton2D(optionsDoneButton, "Done", PointInRectInt(mouse.x, mouse.y, optionsDoneButton));
    DrawBetaMenuFooter();
    DrawBetaVignette2D();
}

void DrawSettings(void)
{
    DrawOptions();
}

void DrawPauseMenu(void)
{
    POINT mouse;
    char label[96];

    Setup2D();
    LayoutBetaMenus();

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(g_windowWidth, 0);
    glVertex2i(g_windowWidth, g_windowHeight);
    glVertex2i(0, g_windowHeight);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseTitle, 0, 110, g_windowWidth, 170, "Game menu");

    DrawButton2D(pauseContinueButton, "Continue", PointInRectInt(mouse.x, mouse.y, pauseContinueButton));
    DrawButton2D(pauseOptionsButton, "Options...", PointInRectInt(mouse.x, mouse.y, pauseOptionsButton));
    DrawButton2D(pauseExitButton, "Save and quit", PointInRectInt(mouse.x, mouse.y, pauseExitButton));

}

void DrawDirtMenuBackground(void)
{
    int x;
    int y;
    Setup2D();
    for (y = 0; y < g_windowHeight; y += 32) {
        for (x = 0; x < g_windowWidth; x += 32) {
            if (texTerrain) {
                /* Verified dirt tile gives the classic tiled menu background and
                   avoids corrupted gui/background.png color/state issues. */
                DrawTerrainTile2D(TILE_DIRT_COL, TILE_DIRT_ROW, x, y, x + 32, y + 32);
            } else if (texBetaMenuBackground) {
                DrawImage2D(texBetaMenuBackground, x, y, x + 32, y + 32, 1.0f);
            } else {
                DrawRect2D(x, y, x + 32, y + 32, 0.28f, 0.18f, 0.10f);
            }
        }
    }
}


void DrawDeathScreen(void)
{
    POINT mouse;

    Setup2D();
    LayoutBetaMenus();

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);

    /* Converted GuiGameOver look: red/black gradient over the frozen world. */
    glBegin(GL_QUADS);
    glColor4f(0.31f, 0.00f, 0.00f, 0.38f);
    glVertex2i(0, 0);
    glVertex2i(g_windowWidth, 0);
    glColor4f(0.08f, 0.00f, 0.00f, 0.78f);
    glVertex2i(g_windowWidth, g_windowHeight);
    glVertex2i(0, g_windowHeight);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseTitle, 0, 58, g_windowWidth, 118, "Game over!");

    glColor3f(1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseNormal, 0, 155, g_windowWidth, 185, "Score: 0");

    DrawButton2D(deathRespawnButton, "Respawn", PointInRectInt(mouse.x, mouse.y, deathRespawnButton));
    DrawButton2D(deathTitleButton, "Title menu", PointInRectInt(mouse.x, mouse.y, deathTitleButton));

    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawButton2D(RECT r, const char *text, int hover)
{
    DrawBetaButtonPanel(r, hover, 0);

    if (text && text[0] != '\0') {
        glColor3f(0.08f, 0.08f, 0.08f);
        DrawCenteredText2D(fontBaseNormal, r.left + 2, r.top + 2, r.right + 2, r.bottom + 2, text);
        if (hover) {
            glColor3f(1.0f, 1.0f, 0.55f);
        } else {
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        DrawCenteredText2D(fontBaseNormal, r.left, r.top, r.right, r.bottom, text);
    }
}

void DrawRect2D(int x1, int y1, int x2, int y2, float r, float g, float b)
{
    glDisable(GL_TEXTURE_2D);

    glColor3f(r, g, b);

    glBegin(GL_QUADS);
    glVertex2i(x1, y1);
    glVertex2i(x2, y1);
    glVertex2i(x2, y2);
    glVertex2i(x1, y2);
    glEnd();
}

int PointInRectInt(int x, int y, RECT r)
{
    if (x >= r.left && x <= r.right &&
        y >= r.top && y <= r.bottom) {
        return 1;
    }

    return 0;
}

void DrawWeather2D(void)
{
    int i;
    int x;
    int y;
    int len;
    int mode;
    GLuint tex;
    mode = g_weatherMode;
    if (mode == 0) {
        return;
    }
    tex = texBetaRain;
    if (mode == 2) {
        tex = texBetaSnow;
    }
    Setup2D();
    if (tex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 0.34f);
        for (i = 0; i < MAX_WEATHER_PARTICLES; i++) {
            x = (WorldHash2D(i, 17, g_worldSeed + 12345) % (g_windowWidth + 160)) - 80;
            y = (WorldHash2D(i, 23, g_worldSeed + 22345) % (g_windowHeight + 160)) - 80;
            y += (int)g_weatherScroll;
            y = y % (g_windowHeight + 160);
            y -= 80;
            len = 26;
            if (mode == 2) {
                len = 10;
            }
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2i(x, y);
            glTexCoord2f(1.0f, 0.0f); glVertex2i(x + 8, y);
            glTexCoord2f(1.0f, 1.0f); glVertex2i(x + 8, y + len);
            glTexCoord2f(0.0f, 1.0f); glVertex2i(x, y + len);
            glEnd();
        }
        glDisable(GL_TEXTURE_2D);
    }
}

void RenderSkyBodies(void)
{
    float f;
    float ang;
    float sx;
    float sy;
    float sz;
    float mx;
    float my;
    float mz;
    float alphaSun;
    float alphaMoon;
    f = (float)(g_worldTimeSeconds / DAY_LENGTH_SECONDS);
    ang = f * 6.2831853f - 1.5707963f;
    sx = (float)playerX + (float)cos(ang) * 42.0f;
    sy = (float)playerY + 36.0f + (float)sin(ang) * 32.0f;
    sz = (float)playerZ - 56.0f;
    mx = (float)playerX - (float)cos(ang) * 42.0f;
    my = (float)playerY + 36.0f - (float)sin(ang) * 32.0f;
    mz = (float)playerZ - 56.0f;
    alphaSun = 0.35f + g_dayNightBlend * 0.65f;
    alphaMoon = 1.0f - g_dayNightBlend * 0.45f;
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    DrawImage3DBillboard(texBetaSun, sx, sy, sz, 9.0f, 9.0f, alphaSun);
    DrawImage3DBillboard(texBetaMoon, mx, my, mz, 8.0f, 8.0f, alphaMoon);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void EnableBetaFog(void)
{
#ifdef GL_FOG
    GLfloat fogColor[4];

    if (g_state == STATE_GAME && IsPlayerHeadUnderWater()) {
        fogColor[0] = 0.02f;
        fogColor[1] = 0.04f;
        fogColor[2] = 0.24f;
        fogColor[3] = 1.0f;
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_EXP2);
        glFogf(GL_FOG_DENSITY, 0.105f);
        glFogfv(GL_FOG_COLOR, fogColor);
        return;
    }

    fogColor[0] = 0.03f + 0.42f * g_dayNightBlend;
    fogColor[1] = 0.05f + 0.65f * g_dayNightBlend;
    fogColor[2] = 0.12f + 0.88f * g_dayNightBlend;
    fogColor[3] = 1.0f;
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, (float)(g_renderDistanceChunks * CHUNK_SIZE) * 0.62f);
    glFogf(GL_FOG_END,   (float)(g_renderDistanceChunks * CHUNK_SIZE) * 0.95f);
    glFogfv(GL_FOG_COLOR, fogColor);
#endif
}


void DisableBetaFog(void)
{
#ifdef GL_FOG
    glDisable(GL_FOG);
#endif
}

/* ------------------------------------------------------------ */
/* World generation                                             */
/* ------------------------------------------------------------ */


int IsGlobalInsideFiniteWorld(int gx, int gz)
{
    int half;

    half = g_worldSizeBlocks / 2;

    if (gx < -half || gx >= half) {
        return 0;
    }

    if (gz < -half || gz >= half) {
        return 0;
    }

    return 1;
}

int DistanceToFiniteWorldEdge(int gx, int gz)
{
    int half;
    int dx1;
    int dx2;
    int dz1;
    int dz2;
    int d;

    half = g_worldSizeBlocks / 2;
    dx1 = gx + half;
    dx2 = half - 1 - gx;
    dz1 = gz + half;
    dz2 = half - 1 - gz;

    d = dx1;
    if (dx2 < d) { d = dx2; }
    if (dz1 < d) { d = dz1; }
    if (dz2 < d) { d = dz2; }

    return d;
}

int IsGlobalInBorderOcean(int gx, int gz)
{
    if (!IsGlobalInsideFiniteWorld(gx, gz)) {
        return 1;
    }

    if (DistanceToFiniteWorldEdge(gx, gz) < FINITE_BORDER_OCEAN_WIDTH) {
        return 1;
    }

    return 0;
}

void FillFiniteOceanColumn(int lx, int lz, int gx, int gz)
{
    int y;
    int h;
    int ripple;

    ripple = WorldHash2D(gx, gz, g_worldSeed + 6060) % 3;
    h = GEN_WATER_LEVEL - 9 + ripple;
    if (h < 3) { h = 3; }

    biomeMap[lx][lz] = (unsigned char)BIOME_OCEAN;
    worldHeightMap[lx][lz] = h;

    for (y = 0; y < WORLD_Y; y++) {
        if (y == 0) {
            world[lx][y][lz] = BLOCK_BORDER;
        } else if (y <= h - 3) {
            world[lx][y][lz] = BLOCK_SANDSTONE;
        } else if (y <= h) {
            world[lx][y][lz] = BLOCK_SAND;
        } else if (y <= GEN_WATER_LEVEL) {
            world[lx][y][lz] = BLOCK_WATER;
        } else {
            world[lx][y][lz] = BLOCK_AIR;
        }
    }
}

void ClampPlayerToFiniteWorld(void)
{
    double gx;
    double gz;
    double minv;
    double maxv;

    minv = -(double)(g_worldSizeBlocks / 2) + (double)FINITE_BORDER_CLAMP_PAD;
    maxv =  (double)(g_worldSizeBlocks / 2) - (double)(FINITE_BORDER_CLAMP_PAD + 1);

    gx = GetPlayerGlobalX();
    gz = GetPlayerGlobalZ();

    if (gx < minv) { gx = minv; }
    if (gx > maxv) { gx = maxv; }
    if (gz < minv) { gz = minv; }
    if (gz > maxv) { gz = maxv; }

    playerX = gx - (double)worldOriginBlockX;
    playerZ = gz - (double)worldOriginBlockZ;
}

const char *WorldSizeLabel(int size)
{
    return "862 x 862";
}

void ChangeCreateWorldSize(void)
{
    g_createWorldSizeBlocks = FINITE_WORLD_SIZE_SMALL;
}




void GenerateWorld(void)
{
    int startChunkX;
    int startChunkZ;

    /*
        Finite-world bootstrap.
        The playable terrain is bounded to 862x862 blocks.
        This program still keeps only a 128x128 block window in memory
        for Win98-friendly RAM use, but generation outside the chosen
        square becomes shallow ocean rather than endless new land.
    */
    startChunkX = 0;
    startChunkZ = 0;

    if (g_startFromSavedPosition) {
        startChunkX = FloorDivInt((int)floor(g_startGlobalX), CHUNK_SIZE);
        startChunkZ = FloorDivInt((int)floor(g_startGlobalZ), CHUNK_SIZE);
    }

    worldCenterChunkX = startChunkX;
    worldCenterChunkZ = startChunkZ;

    GenerateWorldWindow(worldCenterChunkX, worldCenterChunkZ);
}

void GenerateWorldWindow(int centerChunkX, int centerChunkZ)
{
    int x;
    int y;
    int z;
    int gx;
    int gz;
    int h;
    double density;

    worldCenterChunkX = centerChunkX;
    worldCenterChunkZ = centerChunkZ;

    worldOriginBlockX = (worldCenterChunkX - WORLD_CHUNKS_X / 2) * CHUNK_SIZE;
    worldOriginBlockZ = (worldCenterChunkZ - WORLD_CHUNKS_Z / 2) * CHUNK_SIZE;

    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                world[x][y][z] = BLOCK_AIR;
            }
        }
    }

    /*
        Pass 1: deterministic global 3D density chunks.
        Optimization: only the terrain transition band uses expensive 3D noise.
        Far below the surface is guaranteed stone, and far above is air.
        This keeps Beta-style cliffs/overhangs but removes most per-block
        Perlin calls during chunk streaming.
    */
    for (x = 0; x < WORLD_X; x++) {
        gx = LocalToGlobalBlockX(x);

        for (z = 0; z < WORLD_Z; z++) {
            gz = LocalToGlobalBlockZ(z);
            if (IsGlobalInBorderOcean(gx, gz)) {
                FillFiniteOceanColumn(x, z, gx, gz);
                continue;
            }

            biomeMap[x][z] = (unsigned char)GetBetaBiomeAt(gx, gz);
            h = BetaTerrainHeight(gx, gz);
            if (biomeMap[x][z] == BIOME_OCEAN && h > GEN_WATER_LEVEL - 4) {
                h = GEN_WATER_LEVEL - 4;
            }
            worldHeightMap[x][z] = h;

            for (y = 0; y < WORLD_Y; y++) {
                if (y == 0) {
                    world[x][y][z] = BLOCK_BORDER;
                } else if (y < h - 26) {
                    world[x][y][z] = BLOCK_STONE;
                } else if (y > h + 22) {
                    world[x][y][z] = BLOCK_AIR;
                } else {
                    density = BetaDensity3D(gx, y, gz, h);

                    if (density > 0.0) {
                        world[x][y][z] = BLOCK_STONE;
                    } else {
                        world[x][y][z] = BLOCK_AIR;
                    }
                }
            }
        }
    }

    AddRandomWalkerCaves();
    AddOrePass();
    AddLiquidPass();
    AddSurfaceTexturePass();
    AddWorldFeatures();
    RebuildColumnTops();
    InvalidateAllTerrainChunkMeshes();
}

int FloorDivInt(int a, int b)
{
    int q;
    int r;

    q = a / b;
    r = a % b;

    if (r != 0 && ((r < 0 && b > 0) || (r > 0 && b < 0))) {
        q--;
    }

    return q;
}

int LocalToGlobalBlockX(int x)
{
    return worldOriginBlockX + x;
}

int LocalToGlobalBlockZ(int z)
{
    return worldOriginBlockZ + z;
}

int GlobalToLocalBlockX(int gx)
{
    return gx - worldOriginBlockX;
}

int GlobalToLocalBlockZ(int gz)
{
    return gz - worldOriginBlockZ;
}

double GetPlayerGlobalX(void)
{
    return (double)worldOriginBlockX + playerX;
}

double GetPlayerGlobalZ(void)
{
    return (double)worldOriginBlockZ + playerZ;
}

void KeepPlayerSafeAfterStreaming(void)
{
    int i;
    double testY;

    if (playerY < GEN_WATER_LEVEL + 3) {
        playerY = GEN_WATER_LEVEL + 3;
    }

    if (playerY > WORLD_Y - 6) {
        playerY = WORLD_Y - 6;
    }

    if (!PlayerCollidesAt(playerX, playerY, playerZ)) {
        return;
    }

    /* If a streamed chunk places terrain at the player, lift upward safely. */
    for (i = 0; i < WORLD_Y; i++) {
        testY = playerY + (double)i;

        if (testY >= WORLD_Y - 5) {
            break;
        }

        if (!PlayerCollidesAt(playerX, testY, playerZ)) {
            playerY = testY;
            velocityY = 0.0;
            return;
        }
    }

    /* Last-resort recovery. */
    playerY = WORLD_Y - 8;
    velocityY = 0.0;
}

void UpdateInfiniteWorldStreaming(void)
{
    double globalX;
    double globalZ;
    int blockX;
    int blockZ;
    int newCenterChunkX;
    int newCenterChunkZ;
    int margin;
    int oldOriginX;
    int oldOriginZ;
    int shouldStream;

    margin = STREAM_EDGE_MARGIN_BLOCKS;
    shouldStream = 0;

    /*
        Previous version regenerated the whole world window whenever the
        player crossed ANY 16-block chunk boundary.  Starting near global 0,
        a tiny step into negative X/Z could trigger full regen immediately.

        This safer version streams only when the player approaches the loaded
        window edge, leaving two chunks of padding.  That removes the movement
        crash/freeze and behaves more like a real chunk cache.
    */
    if (playerX < (double)margin || playerX > (double)(WORLD_X - margin) ||
        playerZ < (double)margin || playerZ > (double)(WORLD_Z - margin)) {
        shouldStream = 1;
    }

    if (!shouldStream) {
        return;
    }

    globalX = GetPlayerGlobalX();
    globalZ = GetPlayerGlobalZ();

    blockX = (int)floor(globalX);
    blockZ = (int)floor(globalZ);

    newCenterChunkX = FloorDivInt(blockX, CHUNK_SIZE);
    newCenterChunkZ = FloorDivInt(blockZ, CHUNK_SIZE);

    if (newCenterChunkX == worldCenterChunkX &&
        newCenterChunkZ == worldCenterChunkZ) {
        return;
    }

    oldOriginX = worldOriginBlockX;
    oldOriginZ = worldOriginBlockZ;

    GenerateWorldWindow(newCenterChunkX, newCenterChunkZ);

    playerX = globalX - (double)worldOriginBlockX;
    playerZ = globalZ - (double)worldOriginBlockZ;

    KeepPlayerSafeAfterStreaming();

    /* New chunks get cheap skylight values without expensive full flood-fill. */
    ComputeLegacyLighting();

    /* Keep existing mobs by rebasing their local positions to the new world window. */
    RebaseMobsAfterWorldStream(oldOriginX, oldOriginZ);
}

int FindHighestSolidOrWater(int x, int z)
{
    int y;

    for (y = WORLD_Y - 1; y >= 0; y--) {
        if (world[x][y][z] != BLOCK_AIR) {
            return y;
        }
    }

    return 0;
}

void RebuildColumnTopAt(int x, int z)
{
    if (x < 0 || x >= WORLD_X || z < 0 || z >= WORLD_Z) {
        return;
    }

    columnTop[x][z] = FindHighestSolidOrWater(x, z);
}

void RebuildColumnTops(void)
{
    int x;
    int z;

    for (x = 0; x < WORLD_X; x++) {
        for (z = 0; z < WORLD_Z; z++) {
            RebuildColumnTopAt(x, z);
        }
    }
}


int Hash2D(int x, int z)
{
    return WorldHash2D(LocalToGlobalBlockX(x), LocalToGlobalBlockZ(z), g_worldSeed);
}


int TerrainHeight(int x, int z)
{
    return BetaTerrainHeight(LocalToGlobalBlockX(x), LocalToGlobalBlockZ(z));
}


void AddTree(int x, int y, int z)
{
    AddBetaTree(x, y, z);
}

/* ------------------------------------------------------------ */
/* Beta-inspired fractal Perlin-style terrain and clouds         */
/* ------------------------------------------------------------ */

int WorldHash2D(int x, int z, int seed)
{
    unsigned int n;

    n = (unsigned int)(x * 374761393u + z * 668265263u + seed * 1442695041u);
    n = (n ^ (n >> 13)) * 1274126177u;
    n = n ^ (n >> 16);

    return (int)(n & 0x7fffffff);
}

int WorldHash3D(int x, int y, int z, int seed)
{
    unsigned int n;

    n = (unsigned int)(x * 374761393u + y * 1442695041u +
                       z * 668265263u + seed * 1274126177u);
    n = (n ^ (n >> 13)) * 1274126177u;
    n = n ^ (n >> 16);

    return (int)(n & 0x7fffffff);
}

double WorldFade(double t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double WorldLerp(double a, double b, double t)
{
    return a + (b - a) * t;
}

double WorldGrad2(int ix, int iz, double x, double z, int seed)
{
    int h;
    double gx;
    double gz;
    double dx;
    double dz;

    h = WorldHash2D(ix, iz, seed) & 7;

    if (h == 0) {
        gx = 1.0; gz = 0.0;
    } else if (h == 1) {
        gx = -1.0; gz = 0.0;
    } else if (h == 2) {
        gx = 0.0; gz = 1.0;
    } else if (h == 3) {
        gx = 0.0; gz = -1.0;
    } else if (h == 4) {
        gx = 0.707; gz = 0.707;
    } else if (h == 5) {
        gx = -0.707; gz = 0.707;
    } else if (h == 6) {
        gx = 0.707; gz = -0.707;
    } else {
        gx = -0.707; gz = -0.707;
    }

    dx = x - (double)ix;
    dz = z - (double)iz;

    return gx * dx + gz * dz;
}

double WorldPerlin2D(double x, double z, int seed)
{
    int x0;
    int x1;
    int z0;
    int z1;
    double sx;
    double sz;
    double n00;
    double n10;
    double n01;
    double n11;
    double ix0;
    double ix1;

    x0 = (int)floor(x);
    z0 = (int)floor(z);
    x1 = x0 + 1;
    z1 = z0 + 1;

    sx = WorldFade(x - (double)x0);
    sz = WorldFade(z - (double)z0);

    n00 = WorldGrad2(x0, z0, x, z, seed);
    n10 = WorldGrad2(x1, z0, x, z, seed);
    n01 = WorldGrad2(x0, z1, x, z, seed);
    n11 = WorldGrad2(x1, z1, x, z, seed);

    ix0 = WorldLerp(n00, n10, sx);
    ix1 = WorldLerp(n01, n11, sx);

    return WorldLerp(ix0, ix1, sz);
}

double WorldFractal2D(double x, double z, int seed, int octaves, double persistence)
{
    int i;
    double total;
    double freq;
    double amp;
    double maxValue;

    total = 0.0;
    freq = 1.0;
    amp = 1.0;
    maxValue = 0.0;

    for (i = 0; i < octaves; i++) {
        total += WorldPerlin2D(x * freq, z * freq, seed + i * 101) * amp;
        maxValue += amp;
        amp *= persistence;
        freq *= 2.0;
    }

    if (maxValue == 0.0) {
        return 0.0;
    }

    return total / maxValue;
}

double WorldGrad3(int ix, int iy, int iz, double x, double y, double z, int seed)
{
    int h;
    double dx;
    double dy;
    double dz;
    double u;
    double v;

    h = WorldHash3D(ix, iy, iz, seed) & 15;

    dx = x - (double)ix;
    dy = y - (double)iy;
    dz = z - (double)iz;

    if (h < 8) {
        u = dx;
    } else {
        u = dy;
    }

    if (h < 4) {
        v = dy;
    } else if (h == 12 || h == 14) {
        v = dx;
    } else {
        v = dz;
    }

    if (h & 1) {
        u = -u;
    }

    if (h & 2) {
        v = -v;
    }

    return u + v;
}

double WorldPerlin3D(double x, double y, double z, int seed)
{
    int x0;
    int x1;
    int y0;
    int y1;
    int z0;
    int z1;

    double sx;
    double sy;
    double sz;

    double n000;
    double n100;
    double n010;
    double n110;
    double n001;
    double n101;
    double n011;
    double n111;

    double ix00;
    double ix10;
    double ix01;
    double ix11;
    double iy0;
    double iy1;

    x0 = (int)floor(x);
    y0 = (int)floor(y);
    z0 = (int)floor(z);

    x1 = x0 + 1;
    y1 = y0 + 1;
    z1 = z0 + 1;

    sx = WorldFade(x - (double)x0);
    sy = WorldFade(y - (double)y0);
    sz = WorldFade(z - (double)z0);

    n000 = WorldGrad3(x0, y0, z0, x, y, z, seed);
    n100 = WorldGrad3(x1, y0, z0, x, y, z, seed);
    n010 = WorldGrad3(x0, y1, z0, x, y, z, seed);
    n110 = WorldGrad3(x1, y1, z0, x, y, z, seed);

    n001 = WorldGrad3(x0, y0, z1, x, y, z, seed);
    n101 = WorldGrad3(x1, y0, z1, x, y, z, seed);
    n011 = WorldGrad3(x0, y1, z1, x, y, z, seed);
    n111 = WorldGrad3(x1, y1, z1, x, y, z, seed);

    ix00 = WorldLerp(n000, n100, sx);
    ix10 = WorldLerp(n010, n110, sx);
    ix01 = WorldLerp(n001, n101, sx);
    ix11 = WorldLerp(n011, n111, sx);

    iy0 = WorldLerp(ix00, ix10, sy);
    iy1 = WorldLerp(ix01, ix11, sy);

    return WorldLerp(iy0, iy1, sz);
}

double WorldFractal3D(double x, double y, double z, int seed, int octaves, double persistence)
{
    int i;
    double total;
    double freq;
    double amp;
    double maxValue;

    total = 0.0;
    freq = 1.0;
    amp = 1.0;
    maxValue = 0.0;

    for (i = 0; i < octaves; i++) {
        total += WorldPerlin3D(x * freq, y * freq, z * freq, seed + i * 131) * amp;
        maxValue += amp;
        amp *= persistence;
        freq *= 2.0;
    }

    if (maxValue == 0.0) {
        return 0.0;
    }

    return total / maxValue;
}

int ClampInt(int v, int minv, int maxv)
{
    if (v < minv) {
        return minv;
    }

    if (v > maxv) {
        return maxv;
    }

    return v;
}


double ClampDouble(double v, double minv, double maxv)
{
    if (v < minv) {
        return minv;
    }

    if (v > maxv) {
        return maxv;
    }

    return v;
}




/* ------------------------------------------------------------ */
/* Beta-style climate biome helpers and cheap view culling       */
/* ------------------------------------------------------------ */


int GetBetaBiomeAt(int gx, int gz)
{
    double temp;
    double humidity;
    double ocean;
    double mountain;
    double detail;

    /* Big, readable biome regions. The low-frequency climate fields are
       close to the old Java climate map idea, while the small detail pass
       prevents perfectly straight borders. */
    temp = WorldFractal2D((double)gx * 0.0028, (double)gz * 0.0028,
                          g_worldSeed + 6100, 4, 0.56);
    humidity = WorldFractal2D((double)gx * 0.0031, (double)gz * 0.0031,
                              g_worldSeed + 6200, 4, 0.56);
    ocean = WorldFractal2D((double)gx * 0.0020, (double)gz * 0.0020,
                           g_worldSeed + 6300, 4, 0.54);
    detail = WorldFractal2D((double)gx * 0.014, (double)gz * 0.014,
                            g_worldSeed + 6310, 2, 0.50) * 0.10;
    mountain = BetaMountainMask(gx, gz);

    temp = ClampDouble((temp + 1.0) * 0.5 + detail, 0.0, 1.0);
    humidity = ClampDouble((humidity + 1.0) * 0.5 - detail * 0.6, 0.0, 1.0);
    ocean = ClampDouble((ocean + 1.0) * 0.5, 0.0, 1.0);

    if (ocean < 0.20) {
        return BIOME_OCEAN;
    }
    if (temp > 0.72 && humidity < 0.36) {
        return BIOME_DESERT;
    }
    if (temp < 0.22) {
        return BIOME_TUNDRA;
    }
    if (temp < 0.36) {
        return BIOME_TAIGA;
    }
    if (humidity > 0.78 && temp > 0.58) {
        return BIOME_RAINFOREST;
    }
    if (humidity > 0.70 && temp > 0.38) {
        return BIOME_SWAMPLAND;
    }
    if (humidity > 0.55) {
        if (mountain > 0.52) { return BIOME_SEASONAL_FOREST; }
        return BIOME_FOREST;
    }
    if (humidity < 0.33) {
        return BIOME_SHRUBLAND;
    }
    return BIOME_PLAINS;
}


int GetBiomeAtGlobal(int gx, int gz)
{
    return GetBetaBiomeAt(gx, gz);
}

int GetLocalBiome(int x, int z)
{
    if (x >= 0 && x < WORLD_X && z >= 0 && z < WORLD_Z) {
        return (int)biomeMap[x][z];
    }

    return GetBetaBiomeAt(LocalToGlobalBlockX(x), LocalToGlobalBlockZ(z));
}

int BiomeTopBlock(int biome, int y)
{
    if (biome == BIOME_DESERT) {
        return BLOCK_SAND;
    }

    if (biome == BIOME_TUNDRA || biome == BIOME_TAIGA) {
        if (y > GEN_WATER_LEVEL) {
            return BLOCK_SNOW;
        }
    }

    if (biome == BIOME_OCEAN) {
        return BLOCK_SAND;
    }

    return BLOCK_GRASS;
}

int BiomeFillerBlock(int biome)
{
    if (biome == BIOME_DESERT || biome == BIOME_OCEAN) {
        return BLOCK_SAND;
    }

    if (biome == BIOME_TUNDRA || biome == BIOME_TAIGA) {
        return BLOCK_DIRT;
    }

    return BLOCK_DIRT;
}

int IsPointProbablyInView(double x, double z, double nearAlways, double dotLimit)
{
    double dx;
    double dz;
    double distSq;
    double yawRad;
    double forwardX;
    double forwardZ;
    double dot;

    dx = x - playerX;
    dz = z - playerZ;
    distSq = dx * dx + dz * dz;

    /* Anything very close is always rendered to avoid pop-in around the player. */
    if (distSq <= nearAlways * nearAlways) {
        return 1;
    }

    if (distSq <= 0.0001) {
        return 1;
    }

    yawRad = yaw * PI / 180.0;
    forwardX = -sin(yawRad);
    forwardZ = -cos(yawRad);
    dot = (dx * forwardX + dz * forwardZ) / sqrt(distSq);

    if (dot >= dotLimit) {
        return 1;
    }

    return 0;
}

int IsColumnProbablyVisible(int x, int z, int distSq)
{
    double cx;
    double cz;
    int veryNear;
    int nearD;

    veryNear = GetVeryNearTerrainRenderDistanceBlocks();
    nearD = GetNearTerrainRenderDistanceBlocks();

    if (distSq < veryNear * veryNear) {
        return 1;
    }

    cx = (double)x + 0.5;
    cz = (double)z + 0.5;

    if (distSq < nearD * nearD) {
        return IsPointProbablyInView(cx, cz, (double)veryNear + 2.0, -0.50);
    }

    /* Far terrain is culled harder; this is the main FPS win. */
    return IsPointProbablyInView(cx, cz, (double)nearD + 2.0, 0.02);
}


int GetTerrainRenderDistanceBlocks(void)
{
    int d;
    d = g_renderDistanceChunks * CHUNK_SIZE;
    if (d < 24) { d = 24; }
    if (d > RENDER_DISTANCE_MAX_CHUNKS * CHUNK_SIZE) {
        d = RENDER_DISTANCE_MAX_CHUNKS * CHUNK_SIZE;
    }
    return d;
}

int GetNearTerrainRenderDistanceBlocks(void)
{
    int d;
    d = g_renderDistanceChunks * 8 + 8;
    if (d < 14) { d = 14; }
    if (d > 56) { d = 56; }
    return d;
}

int GetVeryNearTerrainRenderDistanceBlocks(void)
{
    int d;
    d = g_renderDistanceChunks * 3 + 6;
    if (d < 10) { d = 10; }
    if (d > 24) { d = 24; }
    return d;
}

int GetMobRenderDistanceBlocks(void)
{
    int d;
    d = g_renderDistanceChunks * CHUNK_SIZE;
    if (d < 18) { d = 18; }
    if (d > 64) { d = 64; }
    return d;
}

const char *RenderDistanceLabel(void)
{
    static char label[32];

    if (g_renderDistanceChunks <= 1) { return "Tiny 1 chunk"; }
    if (g_renderDistanceChunks == 2) { return "Short 2 chunks"; }
    if (g_renderDistanceChunks == 4) { return "Normal 4 chunks"; }
    if (g_renderDistanceChunks >= RENDER_DISTANCE_MAX_CHUNKS) { return "Far 20 chunks"; }

    sprintf(label, "%d chunks", g_renderDistanceChunks);
    return label;
}

void ChangeRenderDistance(int delta)
{
    g_renderDistanceChunks += delta;
    if (g_renderDistanceChunks > RENDER_DISTANCE_MAX_CHUNKS) {
        g_renderDistanceChunks = RENDER_DISTANCE_MIN_CHUNKS;
    }
    if (g_renderDistanceChunks < RENDER_DISTANCE_MIN_CHUNKS) {
        g_renderDistanceChunks = RENDER_DISTANCE_MAX_CHUNKS;
    }
}

void SetRenderDistanceFromMouse(int mouseX)
{
    int x0;
    int x1;
    int usable;
    int value;

    x0 = optionsRenderDistanceButton.left + 18;
    x1 = optionsRenderDistanceButton.right - 18;
    usable = x1 - x0;

    if (usable <= 0) {
        ChangeRenderDistance(1);
        return;
    }

    if (mouseX < x0) { mouseX = x0; }
    if (mouseX > x1) { mouseX = x1; }

    value = 1 + ((mouseX - x0) * (RENDER_DISTANCE_MAX_CHUNKS - 1) + usable / 2) / usable;

    if (value < RENDER_DISTANCE_MIN_CHUNKS) { value = RENDER_DISTANCE_MIN_CHUNKS; }
    if (value > RENDER_DISTANCE_MAX_CHUNKS) { value = RENDER_DISTANCE_MAX_CHUNKS; }

    g_renderDistanceChunks = value;
}


double NormalizeAngle180(double a)
{
    while (a < -180.0) { a += 360.0; }
    while (a > 180.0) { a -= 360.0; }
    return a;
}

double ApproachAngleDeg(double from, double to, double maxStep)
{
    double diff;
    diff = NormalizeAngle180(to - from);
    if (diff > maxStep) { diff = maxStep; }
    if (diff < -maxStep) { diff = -maxStep; }
    return NormalizeAngle180(from + diff);
}


/* PATCH_F11_MOB_GUI_MOB_FACING_HELPERS
   Java EntityLiving.faceEntity uses atan2(z, x) - 90 degrees.  The local C
   renderer faces its cuboids down -Z at yaw 0, so this helper converts a world
   X/Z movement vector into the same readable facing direction and smooths it. */
double MobFaceYawFromDelta(double dx, double dz)
{
    if (fabs(dx) < 0.0001 && fabs(dz) < 0.0001) {
        return 0.0;
    }
    return NormalizeAngle180(atan2(dx, -dz) * 180.0 / PI);
}

void MobApproachFacing(Mob *m, double dx, double dz, double dt, double rate)
{
    double desired;
    if (!m) { return; }
    if (fabs(dx) < 0.0001 && fabs(dz) < 0.0001) { return; }
    if (dt <= 0.0) { dt = 0.016; }
    desired = MobFaceYawFromDelta(dx, dz);
    m->yaw = ApproachAngleDeg(m->yaw, desired, rate * dt);
}

void UpdatePlayerMovementAnimation(double dt)
{
    double dx;
    double dz;
    double dist;
    double speed;
    int bx;
    int by;
    int bz;
    int blockBelow;

    if (dt <= 0.0) { return; }
    dx = playerX - g_playerLastAnimX;
    dz = playerZ - g_playerLastAnimZ;
    dist = sqrt(dx * dx + dz * dz);
    speed = dist / dt;

    if (onGround && speed > 0.035) {
        g_playerWalkAmount = speed / MOVE_SPEED;
        if (g_playerWalkAmount > 1.0) { g_playerWalkAmount = 1.0; }
        handBob += dist * 8.0;
        if (g_playerStepSoundTimer <= 0.0) {
            bx = (int)floor(playerX);
            by = (int)floor(playerY - 0.15);
            bz = (int)floor(playerZ);
            blockBelow = GetBlock(bx, by, bz);
            PlayPlayerStepSound(blockBelow);
            g_playerStepSoundTimer = 0.42;
        }
    } else {
        g_playerWalkAmount *= 0.70;
        if (g_playerWalkAmount < 0.01) { g_playerWalkAmount = 0.0; }
    }

    if (g_playerStepSoundTimer > 0.0) {
        g_playerStepSoundTimer -= dt;
        if (g_playerStepSoundTimer < 0.0) { g_playerStepSoundTimer = 0.0; }
    }

    g_playerLastAnimX = playerX;
    g_playerLastAnimZ = playerZ;
}

void ApplyDamageCameraWobble(void)
{
    double hurtLeft;
    double maxHurt;
    double f;
    double roll;

    if (g_damageWobbleTimer <= 0.0) {
        return;
    }

    /* Converted from EntityRenderer.hurtCameraEffect: use the remaining
       hurt time over max hurt time, then sin(f^4*pi) to roll the camera. */
    maxHurt = 0.50;
    hurtLeft = g_damageWobbleTimer;
    if (hurtLeft < 0.0) { hurtLeft = 0.0; }
    if (hurtLeft > maxHurt) { hurtLeft = maxHurt; }

    f = hurtLeft / maxHurt;
    f = sin(f * f * f * f * PI);
    roll = -f * 18.0 * g_damageWobbleStrength;

    glRotatef((float)(-g_damageAttackedYaw), 0.0f, 1.0f, 0.0f);
    glRotatef((float)roll, 0.0f, 0.0f, 1.0f);
    glRotatef((float)(g_damageAttackedYaw), 0.0f, 1.0f, 0.0f);
}

void TriggerDamageCameraWobble(int amount)
{
    if (amount < 1) { amount = 1; }
    g_damageWobbleTimer = 0.50;
    g_damageWobbleStrength = 0.85 + (double)amount * 0.06;
    if (g_damageWobbleStrength > 1.25) { g_damageWobbleStrength = 1.25; }
    g_damageAttackedYaw = yaw + 180.0;
}


/*
    Mountain mask decides where the huge Beta-like ranges appear.
    Low values make normal rolling land.
    High values make jagged mega mountain regions.
*/
double BetaMountainMask(int x, int z)
{
    double continent;
    double ridges;
    double mask;

    continent = WorldFractal2D((double)x * 0.010,
                               (double)z * 0.010,
                               g_worldSeed + 500,
                               3,
                               0.55);

    ridges = fabs(WorldFractal2D((double)x * 0.021,
                                 (double)z * 0.021,
                                 g_worldSeed + 530,
                                 2,
                                 0.52));

    mask = (continent + 0.35) * 0.75 + ridges * 0.65;
    mask = (mask - 0.18) / 0.82;
    mask = ClampDouble(mask, 0.0, 1.0);

    /* Smoothstep so mountains blend into nearby terrain. */
    mask = mask * mask * (3.0 - 2.0 * mask);

    return mask;
}


double BetaDensity3D(int x, int y, int z, int surfaceY)
{
    double vertical;
    double main3d;
    double rough3d;
    double overhang3d;
    double brokenY;
    double shelf;
    double mountainMask;
    double density;

    mountainMask = BetaMountainMask(x, z);

    /*
        Positive density becomes stone.
        Negative density becomes air.
        The vertical value is the basic solid-under-surface rule.
    */
    vertical = ((double)surfaceY - (double)y) / 7.0;

    main3d = WorldFractal3D((double)x * 0.024,
                            (double)y * 0.030,
                            (double)z * 0.024,
                            g_worldSeed + 1200,
                            3,
                            0.55);

    rough3d = WorldFractal3D((double)x * 0.070,
                             (double)y * 0.075,
                             (double)z * 0.070,
                             g_worldSeed + 1300,
                             2,
                             0.50);

    /*
        Slower Y scale makes broad horizontal layers and overhangs.
    */
    overhang3d = WorldFractal3D((double)x * 0.038,
                                (double)y * 0.014,
                                (double)z * 0.038,
                                g_worldSeed + 1400,
                                2,
                                0.57);

    /*
        Intentional old-Beta-style vertical weirdness.
        This is not copied Minecraft source code. It imitates the broken,
        discontinuous 3D-noise feeling that creates shelves and arches.
    */
    brokenY = sin((double)y * 0.48 +
                  WorldFractal2D((double)x * 0.030,
                                 (double)z * 0.030,
                                 g_worldSeed + 1450,
                                 3,
                                 0.50) * 4.0);

    shelf = fabs(WorldFractal2D((double)x * 0.028,
                                (double)z * 0.028,
                                g_worldSeed + 1500,
                                4,
                                0.50));

    density = vertical;
    density += main3d * (1.00 + mountainMask * 0.95);
    density += rough3d * 0.36;
    density += overhang3d * (0.80 + mountainMask * 0.90);
    density += brokenY * mountainMask * 0.42;

    if (shelf > 0.15 && y < surfaceY + 24) {
        density += shelf * (0.80 + mountainMask * 1.25);
    }

    /* Keep the very bottom mostly solid. */
    if (y < 5) {
        density += 8.0;
    }

    /* Very top of the world should fade back toward air. */
    if (y > WORLD_Y - 10) {
        density -= ((double)y - (double)(WORLD_Y - 10)) * 0.55;
    }

    return density;
}


void CarveSphere(int cx, int cy, int cz, int radius)
{
    int x;
    int y;
    int z;
    int dx;
    int dy;
    int dz;
    int r2;
    int d2;

    r2 = radius * radius;

    for (dx = -radius; dx <= radius; dx++) {
        for (dy = -radius; dy <= radius; dy++) {
            for (dz = -radius; dz <= radius; dz++) {
                x = cx + dx;
                y = cy + dy;
                z = cz + dz;

                if (!IsInsideWorld(x, y, z)) {
                    continue;
                }

                d2 = dx * dx + dy * dy + dz * dz;

                if (d2 <= r2 && y > 2 && y < WORLD_Y - 4) {
                    if (world[x][y][z] != BLOCK_BORDER) {
                        world[x][y][z] = BLOCK_AIR;
                    }
                }
            }
        }
    }
}

void CarveEllipsoid(int cx, int cy, int cz, int rx, int ry, int rz)
{
    int x;
    int y;
    int z;
    int dx;
    int dy;
    int dz;
    double nx;
    double ny;
    double nz;
    double d;

    if (rx < 1) {
        rx = 1;
    }

    if (ry < 1) {
        ry = 1;
    }

    if (rz < 1) {
        rz = 1;
    }

    for (dx = -rx; dx <= rx; dx++) {
        for (dy = -ry; dy <= ry; dy++) {
            for (dz = -rz; dz <= rz; dz++) {
                x = cx + dx;
                y = cy + dy;
                z = cz + dz;

                if (!IsInsideWorld(x, y, z)) {
                    continue;
                }

                nx = (double)dx / (double)rx;
                ny = (double)dy / (double)ry;
                nz = (double)dz / (double)rz;
                d = nx * nx + ny * ny + nz * nz;

                if (d <= 1.0 && y > 2 && y < WORLD_Y - 4) {
                    if (world[x][y][z] != BLOCK_BORDER) {
                        world[x][y][z] = BLOCK_AIR;
                    }
                }
            }
        }
    }
}


void AddRandomWalkerCaves(void)
{
    /*
        Keep this public function name because GenerateWorld already calls it.
        Internally it now uses smoother Beta-style Perlin worms, plus rooms
        and vertical shafts.
    */
    AddBetaCaveWorms();
    AddVerticalShaftsAndRooms();
}


void AddBetaCaveWorms(void)
{
    int chunkX;
    int chunkZ;
    int minChunkX;
    int maxChunkX;
    int minChunkZ;
    int maxChunkZ;
    int worm;
    int wormCount;
    int step;
    int steps;
    int branch;
    int cx;
    int cy;
    int cz;
    int lx;
    int lz;
    int h;
    int rx;
    int ry;
    int rz;
    int borderCut;
    int bstep;
    int bx;
    int by;
    int bz;
    double px;
    double py;
    double pz;
    double yawAngle;
    double pitchAngle;
    double yawVelocity;
    double pitchVelocity;
    double radiusBase;
    double radiusWave;
    double turnNoise;
    double branchYaw;
    double bpx;
    double bpy;
    double bpz;

    minChunkX = FloorDivInt(worldOriginBlockX, CHUNK_SIZE);
    maxChunkX = FloorDivInt(worldOriginBlockX + WORLD_X - 1, CHUNK_SIZE);
    minChunkZ = FloorDivInt(worldOriginBlockZ, CHUNK_SIZE);
    maxChunkZ = FloorDivInt(worldOriginBlockZ + WORLD_Z - 1, CHUNK_SIZE);

    for (chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
        for (chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
            wormCount = 1 + (WorldHash2D(chunkX, chunkZ, g_worldSeed + 1999) % 3);

            for (worm = 0; worm < wormCount; worm++) {
                px = (double)(chunkX * CHUNK_SIZE + 2 +
                     (WorldHash3D(chunkX, worm, chunkZ, g_worldSeed + 2000) % 12));
                pz = (double)(chunkZ * CHUNK_SIZE + 2 +
                     (WorldHash3D(chunkX, worm, chunkZ, g_worldSeed + 2001) % 12));

                h = BetaTerrainHeight((int)px, (int)pz);
                py = 7.0 + (double)(WorldHash3D(chunkX, worm, chunkZ, g_worldSeed + 2002) % 46);

                if (py > (double)h - 5.0) {
                    py = (double)h - 9.0;
                }

                if (py < 6.0) {
                    py = 6.0;
                }

                yawAngle = ((double)(WorldHash3D(chunkX, worm, chunkZ, g_worldSeed + 2003) % 6283)) / 1000.0;
                pitchAngle = (((double)(WorldHash3D(chunkX, worm, chunkZ, g_worldSeed + 2004) % 1200)) / 1000.0) - 0.6;
                yawVelocity = 0.0;
                pitchVelocity = 0.0;
                radiusBase = 1.4 + (double)(WorldHash3D(chunkX, worm, chunkZ, g_worldSeed + 2005) % 22) / 10.0;
                steps = GEN_CAVE_STEPS + (WorldHash3D(chunkX, worm, chunkZ, g_worldSeed + 2006) % 32);

                for (step = 0; step < steps; step++) {
                    cx = (int)floor(px);
                    cy = (int)floor(py);
                    cz = (int)floor(pz);

                    lx = GlobalToLocalBlockX(cx);
                    lz = GlobalToLocalBlockZ(cz);

                    if (cy < 4 || cy >= WORLD_Y - 4) {
                        break;
                    }

                    if (px < (double)(worldOriginBlockX - 16) ||
                        pz < (double)(worldOriginBlockZ - 16) ||
                        px > (double)(worldOriginBlockX + WORLD_X + 16) ||
                        pz > (double)(worldOriginBlockZ + WORLD_Z + 16)) {
                        break;
                    }

                    /*
                        Beta-like chunk border quirk.
                        Some worms stop exactly at global 16x16 chunk borders,
                        leaving those classic abrupt cave cutoffs.
                    */
                    borderCut = 0;

                    if ((cx & (CHUNK_SIZE - 1)) == 0 ||
                        (cz & (CHUNK_SIZE - 1)) == 0) {
                        if ((WorldHash3D(cx, cy, cz, g_worldSeed + worm + step) % 100) < 9) {
                            borderCut = 1;
                        }
                    }

                    if (borderCut) {
                        break;
                    }

                    radiusWave = sin((double)step * 3.1415926535 / (double)steps);
                    rx = 1 + (int)(radiusBase + radiusWave * 2.2);
                    ry = 1 + (int)(radiusBase * 0.65 + radiusWave * 1.25);
                    rz = 1 + (int)(radiusBase + radiusWave * 2.2);

                    if (IsInsideWorld(lx, cy, lz)) {
                        CarveEllipsoid(lx, cy, lz, rx, ry, rz);

                        if ((WorldHash3D(cx, cy, cz, g_worldSeed + 2600 + step) % 1000) < 24) {
                            CarveEllipsoid(lx, cy, lz, rx + 3, ry + 2, rz + 3);
                        }
                    }

                    branch = WorldHash3D(cx, cy, cz, g_worldSeed + 2700 + step) % 1000;

                    if (branch < 18) {
                        branchYaw = yawAngle + 1.10;

                        if (branch & 1) {
                            branchYaw = yawAngle - 1.10;
                        }

                        bpx = px;
                        bpy = py;
                        bpz = pz;

                        for (bstep = 0; bstep < 14; bstep++) {
                            bpx += cos(branchYaw) * 0.90;
                            bpz += sin(branchYaw) * 0.90;
                            bpy += sin(pitchAngle * 0.65) * 0.45;

                            bx = GlobalToLocalBlockX((int)floor(bpx));
                            by = (int)floor(bpy);
                            bz = GlobalToLocalBlockZ((int)floor(bpz));

                            if (IsInsideWorld(bx, by, bz)) {
                                CarveEllipsoid(bx, by, bz, 2, 1, 2);
                            }
                        }
                    }

                    turnNoise = WorldFractal3D(px * 0.030,
                                               py * 0.050,
                                               pz * 0.030,
                                               g_worldSeed + 2800 + worm,
                                               3,
                                               0.55);

                    yawVelocity = yawVelocity * 0.72 + turnNoise * 0.36;
                    pitchVelocity = pitchVelocity * 0.70 +
                                    WorldFractal3D(px * 0.025,
                                                   py * 0.060,
                                                   pz * 0.025,
                                                   g_worldSeed + 2900 + worm,
                                                   2,
                                                   0.50) * 0.18;

                    yawAngle += yawVelocity;
                    pitchAngle += pitchVelocity;
                    pitchAngle = ClampDouble(pitchAngle, -0.72, 0.72);

                    px += cos(yawAngle) * cos(pitchAngle) * 1.15;
                    pz += sin(yawAngle) * cos(pitchAngle) * 1.15;
                    py += sin(pitchAngle) * 0.95;

                    if (py < 5.0) {
                        py = 5.0;
                        pitchAngle = fabs(pitchAngle);
                    }

                    if (py > (double)(WORLD_Y - 8)) {
                        py = (double)(WORLD_Y - 8);
                        pitchAngle = -fabs(pitchAngle);
                    }
                }
            }
        }
    }
}


void AddVerticalShaftsAndRooms(void)
{
    int chunkX;
    int chunkZ;
    int minChunkX;
    int maxChunkX;
    int minChunkZ;
    int maxChunkZ;
    int gx;
    int gz;
    int lx;
    int lz;
    int y;
    int h;
    int sy;
    int roomChance;
    int shaftChance;

    minChunkX = FloorDivInt(worldOriginBlockX, CHUNK_SIZE);
    maxChunkX = FloorDivInt(worldOriginBlockX + WORLD_X - 1, CHUNK_SIZE);
    minChunkZ = FloorDivInt(worldOriginBlockZ, CHUNK_SIZE);
    maxChunkZ = FloorDivInt(worldOriginBlockZ + WORLD_Z - 1, CHUNK_SIZE);

    for (chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
        for (chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
            if ((WorldHash2D(chunkX, chunkZ, g_worldSeed + 3100) % 100) > 46) {
                continue;
            }

            gx = chunkX * CHUNK_SIZE + 3 +
                 (WorldHash2D(chunkX, chunkZ, g_worldSeed + 3101) % 10);
            gz = chunkZ * CHUNK_SIZE + 3 +
                 (WorldHash2D(chunkX, chunkZ, g_worldSeed + 3102) % 10);

            lx = GlobalToLocalBlockX(gx);
            lz = GlobalToLocalBlockZ(gz);

            if (lx < 3 || lx >= WORLD_X - 3 || lz < 3 || lz >= WORLD_Z - 3) {
                continue;
            }

            h = BetaTerrainHeight(gx, gz);
            y = 7 + (WorldHash2D(gx, gz, g_worldSeed + 3103) % 44);

            if (y > h - 6) {
                y = h - 10;
            }

            if (y < 6) {
                y = 6;
            }

            roomChance = WorldHash2D(gx, gz, g_worldSeed + 3104) % 100;

            if (roomChance < 70) {
                CarveEllipsoid(lx, y, lz, 5, 3, 5);
            } else {
                CarveEllipsoid(lx, y, lz, 7, 4, 7);
            }

            shaftChance = WorldHash2D(gx, gz, g_worldSeed + 3105) % 100;

            if (shaftChance < 40) {
                for (sy = y; sy < h + 4 && sy < WORLD_Y - 4; sy++) {
                    CarveEllipsoid(lx, sy, lz, 2, 1, 2);
                }
            }
        }
    }
}


void AddLiquidPass(void)
{
    int x;
    int y;
    int z;

    for (x = 0; x < WORLD_X; x++) {
        for (z = 0; z < WORLD_Z; z++) {
            for (y = 1; y <= GEN_WATER_LEVEL; y++) {
                if (world[x][y][z] == BLOCK_AIR) {
                    world[x][y][z] = BLOCK_WATER;
                }
            }
        }
    }
}

void AddSurfaceTexturePass(void)
{
    int x;
    int y;
    int z;
    int topFound;
    int depth;
    int biome;
    int topBlock;
    int fillerBlock;

    for (x = 0; x < WORLD_X; x++) {
        for (z = 0; z < WORLD_Z; z++) {
            topFound = 0;
            depth = 0;
            biome = GetLocalBiome(x, z);

            for (y = WORLD_Y - 2; y >= 1; y--) {
                if (world[x][y][z] == BLOCK_STONE) {
                    if (!topFound) {
                        topFound = 1;
                        depth = 0;
                        topBlock = BiomeTopBlock(biome, y);
                        fillerBlock = BiomeFillerBlock(biome);

                        if (biome == BIOME_OCEAN || y <= GEN_WATER_LEVEL + 1) {
                            world[x][y][z] = fillerBlock;
                        } else {
                            world[x][y][z] = topBlock;
                        }
                    } else if (depth < GEN_DIRT_DEPTH) {
                        fillerBlock = BiomeFillerBlock(biome);
                        if (biome == BIOME_DESERT && depth >= 2) {
                            world[x][y][z] = BLOCK_SANDSTONE;
                        } else {
                            world[x][y][z] = fillerBlock;
                        }
                        depth++;
                    }
                } else if (world[x][y][z] == BLOCK_AIR || world[x][y][z] == BLOCK_WATER) {
                    topFound = 0;
                    depth = 0;
                }
            }
        }
    }
}



int BetaTerrainHeight(int x, int z)
{
    double low;
    double high;
    double selector;
    double mountainMask;
    double ridge;
    double detail;
    double valley;
    double heightValue;
    double climateScale;
    int biome;
    int h;

    biome = GetBetaBiomeAt(x, z);

    low = WorldFractal2D((double)x * 0.013, (double)z * 0.013, g_worldSeed + 100, 4, 0.55);
    high = WorldFractal2D((double)x * 0.008, (double)z * 0.008, g_worldSeed + 200, 5, 0.57);
    selector = WorldFractal2D((double)x * 0.0042, (double)z * 0.0042, g_worldSeed + 300, 3, 0.52);
    mountainMask = BetaMountainMask(x, z);
    ridge = 1.0 - fabs(WorldFractal2D((double)x * 0.021, (double)z * 0.021, g_worldSeed + 400, 4, 0.50));
    detail = WorldFractal2D((double)x * 0.066, (double)z * 0.066, g_worldSeed + 450, 2, 0.48);
    valley = WorldFractal2D((double)x * 0.0065, (double)z * 0.0065, g_worldSeed + 460, 2, 0.52);

    heightValue = (selector > -0.05) ? high : low;

    climateScale = 1.0;
    if (biome == BIOME_PLAINS || biome == BIOME_SHRUBLAND) { climateScale = 0.52; }
    if (biome == BIOME_DESERT) { climateScale = 0.68; }
    if (biome == BIOME_SWAMPLAND) { climateScale = 0.28; }
    if (biome == BIOME_TUNDRA) { climateScale = 0.50; }
    if (biome == BIOME_TAIGA) { climateScale = 0.78; }
    if (biome == BIOME_FOREST) { climateScale = 0.92; }
    if (biome == BIOME_SEASONAL_FOREST) { climateScale = 1.08; }
    if (biome == BIOME_RAINFOREST) { climateScale = 1.18; }
    if (biome == BIOME_OCEAN) { climateScale = 0.20; }

    heightValue = heightValue * 18.0 * climateScale;
    heightValue += mountainMask * 34.0 * climateScale;
    heightValue += mountainMask * ridge * 28.0 * climateScale;
    if (valley < -0.28 && biome != BIOME_OCEAN && biome != BIOME_SWAMPLAND) {
        heightValue -= (-valley - 0.28) * 16.0;
    }
    heightValue += detail * 4.5;

    h = GEN_BASE_HEIGHT + (int)heightValue;
    if (biome == BIOME_DESERT) { h = h - 1 + (int)(detail * 2.0); }
    if (biome == BIOME_SWAMPLAND) { h = GEN_WATER_LEVEL + 1 + (int)(detail * 2.0); }
    if (biome == BIOME_OCEAN) { h = GEN_WATER_LEVEL - 9 + (int)(detail * 2.0); }
    if (biome == BIOME_TUNDRA) { h -= 2; }
    h = ClampInt(h, 5, GEN_MAX_SURFACE);
    return h;
}



int IsBetaCave(int x, int y, int z, int surfaceY)
{
    double n;

    if (y < 5 || y > surfaceY - 3) {
        return 0;
    }

    n = WorldFractal3D((double)x * 0.050,
                       (double)y * 0.070,
                       (double)z * 0.050,
                       g_worldSeed + 700,
                       3,
                       0.55);

    if (n > 0.24) {
        return 1;
    }

    return 0;
}


void AddWorldFeatures(void)
{
    int x;
    int z;
    int gx;
    int gz;
    int h;
    int r;
    int biome;
    double forest;

    for (x = 4; x < WORLD_X - 4; x++) {
        gx = LocalToGlobalBlockX(x);

        for (z = 4; z < WORLD_Z - 4; z++) {
            gz = LocalToGlobalBlockZ(z);
            h = BetaTerrainHeight(gx, gz);

            if (h < GEN_WATER_LEVEL + 1 || h + 8 >= WORLD_Y) {
                continue;
            }

            biome = GetBiomeAtGlobal(gx, gz);
            r = WorldHash2D(gx, gz, g_worldSeed + 950) % 1000;

            if (biome == BIOME_DESERT) {
                if (world[x][h][z] == BLOCK_SAND && r < 10) {
                    AddBetaCactus(x, h + 1, z, 2 + (WorldHash2D(gx, gz, g_worldSeed + 996) % 3));
                }
                continue;
            }

            if (world[x][h][z] != BLOCK_GRASS) {
                continue;
            }

            forest = WorldFractal2D((double)gx * 0.018,
                                    (double)gz * 0.018,
                                    g_worldSeed + 900,
                                    3,
                                    0.55);

            if (forest > -0.12 && r < 16) {
                AddBetaTree(x, h + 1, z);
            } else if (forest > 0.12 && r >= 16 && r < 20) {
                AddBigBetaTree(x, h + 1, z);
            }
        }
    }
}



void AddBetaTree(int x, int y, int z)
{
    int i;
    int lx;
    int ly;
    int lz;
    int dist;

    if (!IsInsideWorld(x, y + 5, z)) {
        return;
    }

    for (i = 0; i < 4; i++) {
        SetBlock(x, y + i, z, BLOCK_WOOD);
    }

    for (ly = 2; ly <= 5; ly++) {
        for (lx = -2; lx <= 2; lx++) {
            for (lz = -2; lz <= 2; lz++) {
                dist = abs(lx) + abs(lz);

                if (ly == 5 && dist > 1) {
                    continue;
                }

                if (dist <= 3) {
                    if (GetBlock(x + lx, y + ly, z + lz) == BLOCK_AIR) {
                        SetBlock(x + lx, y + ly, z + lz, BLOCK_LEAVES);
                    }
                }
            }
        }
    }
}

void AddBigBetaTree(int x, int y, int z)
{
    int i;
    int lx;
    int ly;
    int lz;
    int dist;

    if (!IsInsideWorld(x, y + 8, z)) {
        return;
    }

    for (i = 0; i < 6; i++) {
        SetBlock(x, y + i, z, BLOCK_WOOD);
    }

    for (ly = 3; ly <= 7; ly++) {
        for (lx = -3; lx <= 3; lx++) {
            for (lz = -3; lz <= 3; lz++) {
                dist = abs(lx) + abs(lz);

                if (ly >= 6 && dist > 2) {
                    continue;
                }

                if (dist <= 4) {
                    if (GetBlock(x + lx, y + ly, z + lz) == BLOCK_AIR) {
                        SetBlock(x + lx, y + ly, z + lz, BLOCK_LEAVES);
                    }
                }
            }
        }
    }
}

void AddBetaCactus(int x, int y, int z, int height)
{
    int i;

    if (height < 1) { height = 1; }
    if (height > 4) { height = 4; }
    if (!IsInsideWorld(x, y - 1, z)) { return; }
    if (GetBlock(x, y - 1, z) != BLOCK_SAND) { return; }

    for (i = 0; i < height; i++) {
        if (!IsInsideWorld(x, y + i, z)) { return; }
        if (GetBlock(x, y + i, z) != BLOCK_AIR) { return; }
        if (GetBlock(x + 1, y + i, z) != BLOCK_AIR) { return; }
        if (GetBlock(x - 1, y + i, z) != BLOCK_AIR) { return; }
        if (GetBlock(x, y + i, z + 1) != BLOCK_AIR) { return; }
        if (GetBlock(x, y + i, z - 1) != BLOCK_AIR) { return; }
    }

    for (i = 0; i < height; i++) {
        SetBlock(x, y + i, z, BLOCK_CACTUS);
    }
}



/* ------------------------------------------------------------ */
/* Finite-world sky, neutral brightness, and particles                     */
/* ------------------------------------------------------------ */

float ApplyGammaBoost(float v)
{
    /* Neutral brightness, matching the older pre-gamma option behavior. */
    return ClampLightFloat(v);
}


void UpdateDayNightCycle(double dt)
{
    double f;
    double sun;

    g_worldTimeSeconds += dt;

    while (g_worldTimeSeconds >= DAY_LENGTH_SECONDS) {
        g_worldTimeSeconds -= DAY_LENGTH_SECONDS;
    }

    while (g_worldTimeSeconds < 0.0) {
        g_worldTimeSeconds += DAY_LENGTH_SECONDS;
    }

    f = g_worldTimeSeconds / DAY_LENGTH_SECONDS;

    /* 20-minute Minecraft-like cycle: sunrise -> noon -> sunset -> night. */
    sun = sin(f * 6.28318530718);
    sun = (sun + 0.22) / 1.22;
    sun = ClampDouble(sun, 0.0, 1.0);
    sun = sun * sun * (3.0 - 2.0 * sun);

    g_dayNightBlend = (float)sun;
    g_daySkyBrightness = 0.18f + (float)sun * 0.82f;

    g_weatherScroll += dt * 140.0;
    if (g_weatherScroll > 100000.0) {
        g_weatherScroll = 0.0;
    }

    if ((WorldHash2D((int)(g_worldTimeSeconds / 45.0), worldCenterChunkX + worldCenterChunkZ, g_worldSeed + 9100) % 17) == 0) {
        if (g_daySkyBrightness < 0.42f) {
            g_weatherMode = 1;
        } else if ((WorldHash2D(worldCenterChunkX, worldCenterChunkZ, g_worldSeed + 9200) & 3) == 0) {
            g_weatherMode = 2;
        } else {
            g_weatherMode = 1;
        }
    } else {
        g_weatherMode = 0;
    }
}

void ApplyDayNightClearColor(void)
{
    float t;
    float r;
    float g;
    float b;

    if (g_state == STATE_GAME && IsPlayerHeadUnderWater()) {
        glClearColor(0.015f, 0.025f, 0.18f, 1.0f);
        return;
    }

    t = g_dayNightBlend;

    r = 0.03f + 0.42f * t;
    g = 0.05f + 0.65f * t;
    b = 0.12f + 0.88f * t;

    glClearColor(r, g, b, 1.0f);
}


void InitParticles(void)
{
    int i;

    for (i = 0; i < MAX_PARTICLES; i++) {
        particles[i].life = 0.0f;
        particles[i].maxLife = 0.0f;
    }
}

void GetParticleColorForBlock(int block, float *r, float *g, float *b)
{
    if (block == BLOCK_GRASS) {
        *r = 0.36f; *g = 0.62f; *b = 0.28f;
    } else if (block == BLOCK_DIRT) {
        *r = 0.43f; *g = 0.28f; *b = 0.15f;
    } else if (block == BLOCK_STONE) {
        *r = 0.48f; *g = 0.48f; *b = 0.48f;
    } else if (block == BLOCK_WOOD) {
        *r = 0.45f; *g = 0.30f; *b = 0.17f;
    } else if (block == BLOCK_LEAVES) {
        *r = 0.18f; *g = 0.48f; *b = 0.16f;
    } else if (block == BLOCK_SAND || block == BLOCK_SANDSTONE) {
        *r = 0.76f; *g = 0.68f; *b = 0.42f;
    } else if (block == BLOCK_SNOW || block == BLOCK_ICE) {
        *r = 0.82f; *g = 0.90f; *b = 0.95f;
    } else if (block == BLOCK_CACTUS) {
        *r = 0.20f; *g = 0.52f; *b = 0.20f;
    } else {
        *r = 0.70f; *g = 0.70f; *b = 0.70f;
    }
}

void SpawnBlockBreakParticles(int x, int y, int z, int block)
{
    int i;
    int p;
    int h;
    float rx;
    float ry;
    float rz;

    p = 0;

    for (i = 0; i < MAX_PARTICLES && p < 18; i++) {
        if (particles[i].life > 0.0f) {
            continue;
        }

        h = WorldHash3D(LocalToGlobalBlockX(x) + p,
                        y + p * 3,
                        LocalToGlobalBlockZ(z) - p,
                        g_worldSeed + 7000);

        rx = (float)((h & 255) - 128) / 128.0f;
        ry = (float)(((h >> 8) & 255)) / 255.0f;
        rz = (float)(((h >> 16) & 255) - 128) / 128.0f;

        particles[i].x = (float)x + 0.50f + rx * 0.35f;
        particles[i].y = (float)y + 0.50f + ry * 0.30f;
        particles[i].z = (float)z + 0.50f + rz * 0.35f;

        particles[i].vx = rx * 1.20f;
        particles[i].vy = 1.10f + ry * 1.40f;
        particles[i].vz = rz * 1.20f;

        particles[i].life = 0.55f;
        particles[i].maxLife = 0.55f;
        particles[i].block = block;

        p++;
    }
}


void SpawnMobEffectParticles(Mob *m, int block, int count)
{
    int i;
    int made;
    int h;
    float rx;
    float ry;
    float rz;

    if (!m) {
        return;
    }

    made = 0;

    for (i = 0; i < MAX_PARTICLES && made < count; i++) {
        if (particles[i].life > 0.0f) {
            continue;
        }

        h = WorldHash3D((int)(m->x * 16.0) + made,
                        (int)(m->y * 16.0) + made * 5,
                        (int)(m->z * 16.0) - made * 7,
                        g_worldSeed + 7100 + m->type * 13);

        rx = (float)((h & 255) - 128) / 128.0f;
        ry = (float)(((h >> 8) & 255)) / 255.0f;
        rz = (float)(((h >> 16) & 255) - 128) / 128.0f;

        particles[i].x = (float)m->x + rx * 0.35f;
        particles[i].y = (float)m->y + 0.75f + ry * (float)MobHeight(m->type) * 0.45f;
        particles[i].z = (float)m->z + rz * 0.35f;

        particles[i].vx = rx * 0.90f;
        particles[i].vy = 0.70f + ry * 0.90f;
        particles[i].vz = rz * 0.90f;

        particles[i].life = 0.40f;
        particles[i].maxLife = 0.40f;
        particles[i].block = block;

        made++;
    }
}

void UpdateParticles(double dt)
{
    int i;
    float fdt;

    fdt = (float)dt;

    for (i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0.0f) {
            continue;
        }

        particles[i].life -= fdt;

        if (particles[i].life <= 0.0f) {
            particles[i].life = 0.0f;
            continue;
        }

        particles[i].vy -= 6.0f * fdt;
        particles[i].x += particles[i].vx * fdt;
        particles[i].y += particles[i].vy * fdt;
        particles[i].z += particles[i].vz * fdt;
    }
}

void RenderParticles(void)
{
    int i;
    int col;
    int row;
    int shard;
    float r;
    float g;
    float b;
    float a;
    float size;
    float yawRad;
    float rx;
    float rz;
    float ux;
    float uy;
    float uz;
    float u0;
    float v0;
    float u1;
    float v1;
    float du;
    float dv;

    yawRad = (float)(yaw * PI / 180.0);
    rx = (float)cos(yawRad) * 0.08f;
    rz = (float)-sin(yawRad) * 0.08f;
    ux = 0.0f;
    uy = 0.08f;
    uz = 0.0f;

    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (texTerrain) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texTerrain);

        glBegin(GL_QUADS);
        for (i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life <= 0.0f) {
                continue;
            }
            if ((particles[i].x - (float)playerX) * (particles[i].x - (float)playerX) +
                (particles[i].z - (float)playerZ) * (particles[i].z - (float)playerZ) >
                (float)(g_particleCullDistanceBlocks * g_particleCullDistanceBlocks)) {
                continue;
            }
            if (!IsPointProbablyInView((double)particles[i].x, (double)particles[i].z, 8.0, -0.25)) {
                continue;
            }

            GetBlockTile(particles[i].block, 0, &col, &row);
            GetTileUVEx(col, row, TERRAIN_ATLAS_WIDTH, TERRAIN_ATLAS_HEIGHT, &u0, &v0, &u1, &v1);
            du = (u1 - u0) * 0.25f;
            dv = (v1 - v0) * 0.25f;
            shard = (i + particles[i].block * 3) & 15;
            u0 = u0 + du * (float)(shard & 3);
            v0 = v0 + dv * (float)((shard >> 2) & 3);
            u1 = u0 + du;
            v1 = v0 + dv;

            GetParticleColorForBlock(particles[i].block, &r, &g, &b);
            a = particles[i].life / particles[i].maxLife;
            size = 0.70f + 0.30f * a;

            glColor4f(r, g, b, a);
            glTexCoord2f(u0, v0); glVertex3f(particles[i].x - rx * size - ux, particles[i].y - uy * size, particles[i].z - rz * size - uz);
            glTexCoord2f(u1, v0); glVertex3f(particles[i].x + rx * size - ux, particles[i].y - uy * size, particles[i].z + rz * size - uz);
            glTexCoord2f(u1, v1); glVertex3f(particles[i].x + rx * size + ux, particles[i].y + uy * size, particles[i].z + rz * size + uz);
            glTexCoord2f(u0, v1); glVertex3f(particles[i].x - rx * size + ux, particles[i].y + uy * size, particles[i].z - rz * size + uz);
        }
        glEnd();
    } else {
        glDisable(GL_TEXTURE_2D);
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        for (i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life <= 0.0f) {
                continue;
            }
            GetParticleColorForBlock(particles[i].block, &r, &g, &b);
            a = particles[i].life / particles[i].maxLife;
            glColor4f(r, g, b, a);
            glVertex3f(particles[i].x, particles[i].y, particles[i].z);
        }
        glEnd();
        glPointSize(1.0f);
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_CULL_FACE);
}




/* ------------------------------------------------------------ */
/* Dropped items, third-person player, loading screen, status UI */


void InvalidateAllTerrainChunkMeshes(void)
{
    int cx;
    int cz;

    for (cx = 0; cx < WORLD_CHUNKS_X; cx++) {
        for (cz = 0; cz < WORLD_CHUNKS_Z; cz++) {
            terrainChunkDirty[cx][cz] = 1;
        }
    }

    terrainChunkMeshOriginX = worldOriginBlockX;
    terrainChunkMeshOriginZ = worldOriginBlockZ;
}

void DeleteTerrainChunkMeshes(void)
{
    int cx;
    int cz;

    for (cx = 0; cx < WORLD_CHUNKS_X; cx++) {
        for (cz = 0; cz < WORLD_CHUNKS_Z; cz++) {
            if (terrainChunkLists[cx][cz]) {
                glDeleteLists(terrainChunkLists[cx][cz], 1);
                terrainChunkLists[cx][cz] = 0;
            }
            terrainChunkDirty[cx][cz] = 1;
        }
    }
}

void InvalidateTerrainChunkMeshAt(int x, int z)
{
    int cx;
    int cz;

    if (x < 0 || x >= WORLD_X || z < 0 || z >= WORLD_Z) {
        return;
    }

    cx = x / CHUNK_SIZE;
    cz = z / CHUNK_SIZE;

    if (cx >= 0 && cx < WORLD_CHUNKS_X && cz >= 0 && cz < WORLD_CHUNKS_Z) {
        terrainChunkDirty[cx][cz] = 1;
    }

    /* Neighbour chunks need rebuilding too when a border block changes,
       otherwise faces at the seam can be stale. */
    if ((x & (CHUNK_SIZE - 1)) == 0 && cx > 0) {
        terrainChunkDirty[cx - 1][cz] = 1;
    }
    if ((x & (CHUNK_SIZE - 1)) == CHUNK_SIZE - 1 && cx + 1 < WORLD_CHUNKS_X) {
        terrainChunkDirty[cx + 1][cz] = 1;
    }
    if ((z & (CHUNK_SIZE - 1)) == 0 && cz > 0) {
        terrainChunkDirty[cx][cz - 1] = 1;
    }
    if ((z & (CHUNK_SIZE - 1)) == CHUNK_SIZE - 1 && cz + 1 < WORLD_CHUNKS_Z) {
        terrainChunkDirty[cx][cz + 1] = 1;
    }
}

int IsChunkProbablyVisible(int cx, int cz, int distSq)
{
    int x0;
    int z0;
    int x1;
    int z1;
    int cxm;
    int czm;

    x0 = cx * CHUNK_SIZE;
    z0 = cz * CHUNK_SIZE;
    x1 = x0 + CHUNK_SIZE - 1;
    z1 = z0 + CHUNK_SIZE - 1;
    cxm = x0 + CHUNK_SIZE / 2;
    czm = z0 + CHUNK_SIZE / 2;

    if (distSq < (CHUNK_SIZE * CHUNK_SIZE * 5)) {
        return 1;
    }

    if (IsPointProbablyInView((double)cxm, (double)czm, 22.0, -0.22)) {
        return 1;
    }
    if (IsPointProbablyInView((double)x0, (double)z0, 22.0, -0.22)) {
        return 1;
    }
    if (IsPointProbablyInView((double)x1, (double)z0, 22.0, -0.22)) {
        return 1;
    }
    if (IsPointProbablyInView((double)x0, (double)z1, 22.0, -0.22)) {
        return 1;
    }
    if (IsPointProbablyInView((double)x1, (double)z1, 22.0, -0.22)) {
        return 1;
    }

    return 0;
}

void BuildTerrainChunkMesh(int cx, int cz)
{
    int x;
    int y;
    int z;
    int minX;
    int maxX;
    int minZ;
    int maxZ;
    int topY;
    int startY;
    int block;

    if (cx < 0 || cx >= WORLD_CHUNKS_X || cz < 0 || cz >= WORLD_CHUNKS_Z) {
        return;
    }

    if (!terrainChunkLists[cx][cz]) {
        terrainChunkLists[cx][cz] = glGenLists(1);
        if (!terrainChunkLists[cx][cz]) {
            return;
        }
    }

    minX = cx * CHUNK_SIZE;
    maxX = minX + CHUNK_SIZE - 1;
    minZ = cz * CHUNK_SIZE;
    maxZ = minZ + CHUNK_SIZE - 1;

    if (maxX >= WORLD_X) { maxX = WORLD_X - 1; }
    if (maxZ >= WORLD_Z) { maxZ = WORLD_Z - 1; }

    glNewList(terrainChunkLists[cx][cz], GL_COMPILE);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);

    for (x = minX; x <= maxX; x++) {
        for (z = minZ; z <= maxZ; z++) {
            topY = columnTop[x][z];
            if (topY < 0) {
                continue;
            }
            if (topY >= WORLD_Y) {
                topY = WORLD_Y - 1;
            }

            /* Render a visible shell rather than all buried stone.
               This is the same performance principle as chunk mesh generation:
               only exposed faces are emitted, and deep solid interiors are ignored. */
            /* Meshing rule: cache only visible shell geometry.
               Near chunks keep deeper cliff/cave shell; far chunks keep a thin skin. */
            {
                int chunkCenterX;
                int chunkCenterZ;
                int ddx;
                int ddz;
                int shellDepth;
                chunkCenterX = minX + CHUNK_SIZE / 2;
                chunkCenterZ = minZ + CHUNK_SIZE / 2;
                ddx = chunkCenterX - (int)playerX;
                ddz = chunkCenterZ - (int)playerZ;
                if (ddx * ddx + ddz * ddz > (CHUNK_SIZE * CHUNK_SIZE * 9)) {
                    shellDepth = g_chunkMeshShellFar;
                } else {
                    shellDepth = g_chunkMeshShellNear;
                }
                startY = topY - shellDepth;
            }
            if (startY < 1) {
                startY = 1;
            }

            for (y = startY; y <= topY; y++) {
                block = world[x][y][z];
                if (block == BLOCK_AIR) {
                    continue;
                }

                if (ShouldDrawFace(x, y + 1, z, block) ||
                    ShouldDrawFace(x, y - 1, z, block) ||
                    ShouldDrawFace(x, y, z - 1, block) ||
                    ShouldDrawFace(x, y, z + 1, block) ||
                    ShouldDrawFace(x - 1, y, z, block) ||
                    ShouldDrawFace(x + 1, y, z, block)) {
                    DrawBlock(x, y, z, block);
                }
            }
        }
    }

    glEndList();
    terrainChunkDirty[cx][cz] = 0;
}

int GetBlockSoundGroup(int block)
{
    if (block == BLOCK_STONE || block == BLOCK_BORDER || block == BLOCK_SANDSTONE) {
        return 0; /* stone */
    }
    if (block == BLOCK_WOOD) {
        return 1; /* wood */
    }
    if (block == BLOCK_SAND) {
        return 2; /* sand */
    }
    if (block == BLOCK_SNOW || block == BLOCK_ICE) {
        return 3; /* snow */
    }
    if (block == BLOCK_LEAVES || block == BLOCK_WOOL) {
        return 4; /* cloth/soft */
    }
    if (block == BLOCK_DIRT || block == BLOCK_GRASS) {
        return 5; /* grass */
    }
    return 0;
}

void PlayBlockBreakSound(int block)
{
    char path[160];
    int group;
    int variant;

    group = GetBlockSoundGroup(block);
    g_breakSoundPhase++;
    variant = 1 + (g_breakSoundPhase & 3);

    if (group == 1) {
        wsprintf(path, "assets\\sounds\\dig\\dig_wood%d.mp3", variant);
    } else if (group == 2) {
        wsprintf(path, "assets\\sounds\\dig\\dig_sand%d.mp3", variant);
    } else if (group == 3) {
        wsprintf(path, "assets\\sounds\\dig\\dig_snow%d.mp3", variant);
    } else if (group == 4) {
        wsprintf(path, "assets\\sounds\\dig\\dig_cloth%d.mp3", variant);
    } else if (group == 5) {
        wsprintf(path, "assets\\sounds\\dig\\dig_grass%d.mp3", variant);
    } else {
        wsprintf(path, "assets\\sounds\\dig\\dig_stone%d.mp3", variant);
    }

    PlayOneShotMP3(path);
}


/* ------------------------------------------------------------ */

void UpdateFPSCounter(double dt)
{
    g_fpsFrameCounter++;
    g_fpsTimer += dt;

    if (g_fpsTimer >= 0.50) {
        if (g_fpsTimer > 0.0001) {
            g_currentFPS = (int)((double)g_fpsFrameCounter / g_fpsTimer + 0.5);
        } else {
            g_currentFPS = 0;
        }

        if (g_currentFPS > 0 && g_currentFPS < 28) {
            g_chunkMeshBuildBudget = 1;
            g_particleCullDistanceBlocks = 40;
            g_mobFullModelDistanceBlocks = 14;
        } else if (g_currentFPS > 0 && g_currentFPS < 45) {
            g_chunkMeshBuildBudget = 2;
            g_particleCullDistanceBlocks = 48;
            g_mobFullModelDistanceBlocks = 18;
        } else {
            g_chunkMeshBuildBudget = 4;
            g_particleCullDistanceBlocks = 64;
            g_mobFullModelDistanceBlocks = 22;
        }

        g_fpsFrameCounter = 0;
        g_fpsTimer = 0.0;
    }
}

void InitDroppedItems(void)
{
    int i;

    for (i = 0; i < MAX_DROPPED_ITEMS; i++) {
        droppedItems[i].active = 0;
        droppedItems[i].item = ITEM_NONE;
        droppedItems[i].count = 0;
        droppedItems[i].x = 0.0;
        droppedItems[i].y = 0.0;
        droppedItems[i].z = 0.0;
        droppedItems[i].vx = 0.0;
        droppedItems[i].vy = 0.0;
        droppedItems[i].vz = 0.0;
        droppedItems[i].age = 0.0;
        droppedItems[i].spin = 0.0;
    }
}

int AddDroppedItem(int item, int count, double x, double y, double z,
                   double vx, double vy, double vz)
{
    int i;

    if (item == ITEM_NONE || count <= 0) {
        return -1;
    }

    for (i = 0; i < MAX_DROPPED_ITEMS; i++) {
        if (!droppedItems[i].active) {
            droppedItems[i].active = 1;
            droppedItems[i].item = item;
            droppedItems[i].count = count;
            droppedItems[i].x = x;
            droppedItems[i].y = y;
            droppedItems[i].z = z;
            droppedItems[i].vx = vx;
            droppedItems[i].vy = vy;
            droppedItems[i].vz = vz;
            droppedItems[i].age = 0.0;
            droppedItems[i].spin = 0.0;
            return i;
        }
    }

    return -1;
}

void DropSelectedItem(void)
{
    InventorySlot *slot;
    double yawRad;
    double dx;
    double dz;
    int item;

    if (selectedHotbarSlot < 0 || selectedHotbarSlot >= HOTBAR_SLOTS) {
        return;
    }

    slot = &hotbar[selectedHotbarSlot];

    if (slot->item == ITEM_NONE || slot->count <= 0) {
        return;
    }

    item = slot->item;
    yawRad = yaw * PI / 180.0;
    dx = -sin(yawRad);
    dz = -cos(yawRad);

    AddDroppedItem(item,
                   1,
                   playerX + dx * 0.65,
                   playerY + EYE_HEIGHT - 0.35,
                   playerZ + dz * 0.65,
                   dx * 3.0,
                   2.4,
                   dz * 3.0);

    slot->count--;
    if (slot->count <= 0) {
        slot->item = ITEM_NONE;
        slot->count = 0;
    }

    StartHandUse();
}

void UpdateDroppedItems(double dt)
{
    int i;
    int bx;
    int by;
    int bz;
    int below;
    double dx;
    double dy;
    double dz;
    double d2;

    for (i = 0; i < MAX_DROPPED_ITEMS; i++) {
        if (!droppedItems[i].active) {
            continue;
        }

        droppedItems[i].age += dt;
        droppedItems[i].spin += dt * 180.0;

        droppedItems[i].vy -= 9.0 * dt;
        if (droppedItems[i].vy < -12.0) {
            droppedItems[i].vy = -12.0;
        }

        droppedItems[i].x += droppedItems[i].vx * dt;
        droppedItems[i].y += droppedItems[i].vy * dt;
        droppedItems[i].z += droppedItems[i].vz * dt;

        droppedItems[i].vx *= 0.985;
        droppedItems[i].vz *= 0.985;

        bx = (int)floor(droppedItems[i].x);
        by = (int)floor(droppedItems[i].y - 0.10);
        bz = (int)floor(droppedItems[i].z);
        below = GetBlock(bx, by, bz);

        if (IsSolidBlock(below)) {
            droppedItems[i].y = (double)by + 1.12;
            if (droppedItems[i].vy < 0.0) {
                /* Java EntityItem uses a half-strength bounce on ground contact. */
                droppedItems[i].vy = -droppedItems[i].vy * 0.50;
            }
            /* Java-like ground friction: block slipperiness * 0.98, approximated. */
            droppedItems[i].vx *= 0.588;
            droppedItems[i].vz *= 0.588;
        }

        if (droppedItems[i].y < 1.0 || droppedItems[i].age > 300.0) {
            droppedItems[i].active = 0;
            continue;
        }

        dx = droppedItems[i].x - playerX;
        dy = droppedItems[i].y - (playerY + 0.85);
        dz = droppedItems[i].z - playerZ;
        d2 = dx * dx + dy * dy + dz * dz;

        if (droppedItems[i].age > 0.35 && d2 < 1.65) {
            if (AddItemToInventory(droppedItems[i].item, droppedItems[i].count)) {
                PlayItemPickupSound();
                droppedItems[i].active = 0;
            }
        }
    }
}

void DrawDroppedBlockFaceLocal(float x0, float x1, float y0, float y1, float z0, float z1, int face, int block)
{
    int col;
    int row;
    float u0;
    float v0;
    float u1;
    float v1;
    float shade;

    GetBlockTile(block, face, &col, &row);
    GetTerrainTileUV(col, row, &u0, &v0, &u1, &v1);

    shade = 1.0f;
    if (face == 1) { shade = 0.55f; }
    else if (face == 2 || face == 3) { shade = 0.78f; }
    else if (face == 4 || face == 5) { shade = 0.68f; }
    glColor4f(shade, shade, shade, 1.0f);

    if (face == 0) {
        glTexCoord2f(u0, v0); glVertex3f(x0, y1, z0);
        glTexCoord2f(u0, v1); glVertex3f(x0, y1, z1);
        glTexCoord2f(u1, v1); glVertex3f(x1, y1, z1);
        glTexCoord2f(u1, v0); glVertex3f(x1, y1, z0);
    } else if (face == 1) {
        glTexCoord2f(u0, v0); glVertex3f(x0, y0, z1);
        glTexCoord2f(u0, v1); glVertex3f(x0, y0, z0);
        glTexCoord2f(u1, v1); glVertex3f(x1, y0, z0);
        glTexCoord2f(u1, v0); glVertex3f(x1, y0, z1);
    } else if (face == 2) {
        glTexCoord2f(u0, v0); glVertex3f(x1, y0, z0);
        glTexCoord2f(u1, v0); glVertex3f(x0, y0, z0);
        glTexCoord2f(u1, v1); glVertex3f(x0, y1, z0);
        glTexCoord2f(u0, v1); glVertex3f(x1, y1, z0);
    } else if (face == 3) {
        glTexCoord2f(u0, v0); glVertex3f(x0, y0, z1);
        glTexCoord2f(u1, v0); glVertex3f(x1, y0, z1);
        glTexCoord2f(u1, v1); glVertex3f(x1, y1, z1);
        glTexCoord2f(u0, v1); glVertex3f(x0, y1, z1);
    } else if (face == 4) {
        glTexCoord2f(u0, v0); glVertex3f(x0, y0, z0);
        glTexCoord2f(u1, v0); glVertex3f(x0, y0, z1);
        glTexCoord2f(u1, v1); glVertex3f(x0, y1, z1);
        glTexCoord2f(u0, v1); glVertex3f(x0, y1, z0);
    } else if (face == 5) {
        glTexCoord2f(u0, v0); glVertex3f(x1, y0, z1);
        glTexCoord2f(u1, v0); glVertex3f(x1, y0, z0);
        glTexCoord2f(u1, v1); glVertex3f(x1, y1, z0);
        glTexCoord2f(u0, v1); glVertex3f(x1, y1, z1);
    }
}

void DrawDroppedBlockCube(int block)
{
    float s;
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;
    int face;

    s = 0.50f;
    x0 = -s;
    x1 = s;
    y0 = -s;
    y1 = s;
    z0 = -s;
    z1 = s;

    glBegin(GL_QUADS);
    for (face = 0; face < 6; face++) {
        DrawDroppedBlockFaceLocal(x0, x1, y0, y1, z0, z1, face, block);
    }
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawDroppedItemShadow(DroppedItem *d)
{
    int sy;
    float topY;

    if (!texBetaShadow) {
        return;
    }

    for (sy = (int)floor(d->y); sy >= 1; sy--) {
        if (IsSolidBlock(GetBlock((int)floor(d->x), sy - 1, (int)floor(d->z)))) {
            topY = (float)sy + 0.012f;
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texBetaShadow);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 1.0f, 1.0f, 0.32f);
            glDisable(GL_CULL_FACE);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex3f((float)d->x - 0.35f, topY, (float)d->z - 0.35f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f((float)d->x + 0.35f, topY, (float)d->z - 0.35f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f((float)d->x + 0.35f, topY, (float)d->z + 0.35f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f((float)d->x - 0.35f, topY, (float)d->z + 0.35f);
            glEnd();
            break;
        }
    }
}

void DrawDroppedItem(DroppedItem *d)
{
    int block;
    int col;
    int row;
    int copies;
    int copy;
    float u0;
    float v0;
    float u1;
    float v1;
    float size;
    float bob;
    float yawRad;
    float f3;
    float offX;
    float offY;
    float offZ;
    float rightX;
    float rightZ;

    if (!d || !d->active) {
        return;
    }

    copies = 1;
    if (d->count > 1) { copies = 2; }
    if (d->count > 5) { copies = 3; }
    if (d->count > 20) { copies = 4; }

    /* Java RenderItem bob/rotation: sin(age/10 + phase), age/20 * degrees. */
    bob = (float)(sin(d->age * 2.0 + d->spin * 0.01) * 0.10 + 0.10);
    f3 = (float)(d->spin);

    DrawDroppedItemShadow(d);

    block = ItemToBlock(d->item);
    if (block != BLOCK_AIR && texTerrain) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texTerrain);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);

        glPushMatrix();
        glTranslatef((float)d->x, (float)d->y + bob, (float)d->z);
        glRotatef(f3, 0.0f, 1.0f, 0.0f);
        glScalef(0.25f, 0.25f, 0.25f);

        for (copy = 0; copy < copies; copy++) {
            glPushMatrix();
            if (copy > 0) {
                offX = (float)((((copy * 37) % 11) - 5) * 0.045);
                offY = (float)((((copy * 19) % 7) - 3) * 0.045);
                offZ = (float)((((copy * 23) % 11) - 5) * 0.045);
                glTranslatef(offX / 0.25f, offY / 0.25f, offZ / 0.25f);
            }
            DrawDroppedBlockCube(block);
            glPopMatrix();
        }

        glPopMatrix();
        glEnable(GL_CULL_FACE);
        return;
    }

    if (!texBetaItems || !GetItemIconTile(d->item, &col, &row)) {
        return;
    }

    GetTileUVEx(col, row, ICONS_ATLAS_WIDTH, ICONS_ATLAS_HEIGHT, &u0, &v0, &u1, &v1);

    size = 0.26f;
    yawRad = (float)(yaw * PI / 180.0);
    rightX = (float)cos(yawRad) * size;
    rightZ = (float)-sin(yawRad) * size;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texBetaItems);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);

    glPushMatrix();
    glTranslatef((float)d->x, (float)d->y + bob, (float)d->z);
    glRotatef(f3, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
    for (copy = 0; copy < copies; copy++) {
        offX = (float)(copy - (copies - 1) * 0.5f) * 0.065f;
        offY = (float)((copy & 1) ? 0.035f : -0.010f);
        glTexCoord2f(u0, v1); glVertex3f(-rightX + offX, -size + offY, -rightZ);
        glTexCoord2f(u1, v1); glVertex3f( rightX + offX, -size + offY,  rightZ);
        glTexCoord2f(u1, v0); glVertex3f( rightX + offX,  size + offY,  rightZ);
        glTexCoord2f(u0, v0); glVertex3f(-rightX + offX,  size + offY, -rightZ);
    }
    glEnd();

    glPopMatrix();
    glEnable(GL_CULL_FACE);
}


void RenderDroppedItems(void)
{
    int i;
    double dx;
    double dz;
    double maxDist2;

    maxDist2 = 36.0 * 36.0;

    for (i = 0; i < MAX_DROPPED_ITEMS; i++) {
        if (!droppedItems[i].active) {
            continue;
        }

        dx = droppedItems[i].x - playerX;
        dz = droppedItems[i].z - playerZ;
        if (dx * dx + dz * dz > maxDist2) {
            continue;
        }

        DrawDroppedItem(&droppedItems[i]);
    }
}

static void DrawSolidPlayerBox(float cx, float cy, float cz,
                               float sx, float sy, float sz,
                               float r, float g, float b)
{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;

    x0 = cx - sx * 0.5f;
    x1 = cx + sx * 0.5f;
    y0 = cy - sy * 0.5f;
    y1 = cy + sy * 0.5f;
    z0 = cz - sz * 0.5f;
    z1 = cz + sz * 0.5f;

    glDisable(GL_TEXTURE_2D);
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0); glVertex3f(x0, y1, z0);
    glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1); glVertex3f(x1, y1, z1);
    glVertex3f(x0, y0, z1); glVertex3f(x0, y0, z0); glVertex3f(x0, y1, z0); glVertex3f(x0, y1, z1);
    glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1); glVertex3f(x1, y1, z0);
    glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0); glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
    glVertex3f(x0, y0, z1); glVertex3f(x1, y0, z1); glVertex3f(x1, y0, z0); glVertex3f(x0, y0, z0);
    glEnd();
}

void DrawPlayerThirdPerson(void)
{
    float walk;
    float armSwing;
    float legSwing;
    float shade;
    GLuint tex;
    int heldItem;

    walk = (float)handBob;
    /* Java ModelBiped equivalent: arms use cos(walk*0.6662+PI), legs use the
       opposite cosine pair.  Multiplication by g_playerWalkAmount returns the
       limbs to neutral when standing still. */
    armSwing = (float)cos((double)walk * 0.6662 + PI) * 57.29578f * 1.0f * (float)g_playerWalkAmount * 0.50f;
    legSwing = (float)cos((double)walk * 0.6662) * 57.29578f * 1.4f * (float)g_playerWalkAmount;
    shade = ApplyGammaBoost(0.65f + g_daySkyBrightness * 0.42f);
    tex = texMobPlayer;
    heldItem = GetHeldHotbarItem();

    glPushMatrix();
    glTranslatef((float)playerX, (float)playerY, (float)playerZ);
    glRotatef((float)-yaw, 0.0f, 1.0f, 0.0f);
    glDisable(GL_CULL_FACE);

    if (tex) {
        DrawMobSkinBoxPartRot(0.0f, 1.74f, 0.0f, 0.0f, 0.0f, 0.0f, 0.50f, 0.50f, 0.50f, tex, shade, 1.0f, 0, 0, 8, 8, 8, (float)-pitch, 0.0f, 0.0f);
        DrawMobSkinBoxPart(0.0f, 1.12f, 0.0f, 0.50f, 0.75f, 0.25f, tex, shade, 1.0f, 16, 16, 8, 12, 4);
        DrawMobSkinBoxPartRot(-0.38f, 1.45f, 0.0f, 0.0f, -0.36f, 0.0f, 0.25f, 0.75f, 0.25f, tex, shade * 0.93f, 1.0f, 40, 16, 4, 12, 4, armSwing, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.38f, 1.45f, 0.0f, 0.0f, -0.36f, 0.0f, 0.25f, 0.75f, 0.25f, tex, shade * 0.93f, 1.0f, 40, 16, 4, 12, 4, -armSwing, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot(-0.13f, 0.75f, 0.0f, 0.0f, -0.36f, 0.0f, 0.25f, 0.75f, 0.25f, tex, shade * 0.84f, 1.0f, 0, 16, 4, 12, 4, -legSwing, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.13f, 0.75f, 0.0f, 0.0f, -0.36f, 0.0f, 0.25f, 0.75f, 0.25f, tex, shade * 0.84f, 1.0f, 0, 16, 4, 12, 4, legSwing, 0.0f, 0.0f);
        DrawHeldItemThirdPerson(heldItem);
    } else {
        DrawSolidPlayerBox(0.0f, 1.42f, 0.0f, 0.55f, 0.75f, 0.28f, 0.10f, 0.32f, 0.75f);
        DrawSolidPlayerBox(0.0f, 1.95f, 0.0f, 0.48f, 0.48f, 0.48f, 0.78f, 0.56f, 0.42f);
        DrawSolidPlayerBox(-0.43f, 1.40f, 0.0f, 0.20f, 0.70f, 0.20f, 0.78f, 0.56f, 0.42f);
        DrawSolidPlayerBox( 0.43f, 1.40f, 0.0f, 0.20f, 0.70f, 0.20f, 0.78f, 0.56f, 0.42f);
        DrawSolidPlayerBox(-0.16f, 0.62f, 0.0f, 0.22f, 0.82f, 0.22f, 0.18f, 0.18f, 0.75f);
        DrawSolidPlayerBox( 0.16f, 0.62f, 0.0f, 0.22f, 0.82f, 0.22f, 0.18f, 0.18f, 0.75f);
    }
    glEnable(GL_CULL_FACE);
    glPopMatrix();
}

void DrawBuildingTerrainScreen(const char *message)
{
    int x;
    int y;
    char line[160];

    glViewport(0, 0, g_windowWidth, g_windowHeight);
    glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Setup2D();

    for (y = 0; y < g_windowHeight; y += 32) {
        for (x = 0; x < g_windowWidth; x += 32) {
            if (texBetaMenuBackground) {
                DrawImage2D(texBetaMenuBackground, x, y, x + 32, y + 32, 1.0f);
            } else if (texTerrain) {
                DrawTerrainTile2D(TILE_DIRT_COL, TILE_DIRT_ROW, x, y, x + 32, y + 32);
            } else {
                DrawRect2D(x, y, x + 32, y + 32, 0.22f, 0.15f, 0.08f);
            }
        }
    }

    DrawRect2D(g_windowWidth / 2 - 190, g_windowHeight / 2 - 42,
               g_windowWidth / 2 + 190, g_windowHeight / 2 + 42,
               0.05f, 0.05f, 0.05f);
    DrawRect2D(g_windowWidth / 2 - 186, g_windowHeight / 2 - 38,
               g_windowWidth / 2 + 186, g_windowHeight / 2 + 38,
               0.28f, 0.28f, 0.28f);

    glColor3f(1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseNormal,
                       g_windowWidth / 2 - 180,
                       g_windowHeight / 2 - 24,
                       g_windowWidth / 2 + 180,
                       g_windowHeight / 2 + 6,
                       message ? message : "Building terrain");

    sprintf(line, "%s  |  Open Watcom Win32/OpenGL build", CLONEMC_VERSION_TEXT);
    DrawCenteredText2D(fontBaseNormal,
                       g_windowWidth / 2 - 240,
                       g_windowHeight / 2 + 10,
                       g_windowWidth / 2 + 240,
                       g_windowHeight / 2 + 38,
                       line);

    SwapBuffers(g_hdc);
}

void DrawBetaStatus2D(void)
{
    char text[192];
    double gx;
    double gy;
    double gz;

    gx = GetPlayerGlobalX();
    gy = playerY;
    gz = GetPlayerGlobalZ();

    sprintf(text,
            "%s  FPS:%d  XYZ: %.1f / %.1f / %.1f",
            CLONEMC_VERSION_TEXT,
            g_currentFPS,
            gx,
            gy,
            gz);

    Setup2D();
    DrawRect2D(8, 8, 620, 36, 0.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText2D(fontBaseNormal, 16, 30, text);
}




void UpdateClouds(double dt)
{
    cloudOffsetX += dt * CLOUD_SCROLL_SPEED;
    cloudOffsetZ += dt * CLOUD_SCROLL_SPEED * 0.18;
}

int CloudCellVisible(int cx, int cz)
{
    double n1;
    double n2;
    int holes;

    n1 = WorldFractal2D((double)cx * 0.130, (double)cz * 0.130,
                        g_worldSeed + 5000, 4, 0.55);
    n2 = WorldFractal2D((double)cx * 0.330, (double)cz * 0.330,
                        g_worldSeed + 5100, 2, 0.50);
    holes = WorldHash2D(cx, cz, g_worldSeed + 5200) % 100;

    if (n1 + n2 * 0.35 > 0.06 && holes > 18) {
        return 1;
    }

    return 0;
}

void RenderClouds(void)
{
    int centerX;
    int centerZ;
    int cx;
    int cz;
    int gx;
    int gz;
    float x0;
    float x1;
    float z0;
    float z1;
    float alpha;

    centerX = (int)floor((GetPlayerGlobalX() + cloudOffsetX) / CLOUD_CELL_SIZE);
    centerZ = (int)floor((GetPlayerGlobalZ() + cloudOffsetZ) / CLOUD_CELL_SIZE);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);

    alpha = 0.72f;

    for (gx = -CLOUD_RADIUS_CELLS; gx <= CLOUD_RADIUS_CELLS; gx++) {
        for (gz = -CLOUD_RADIUS_CELLS; gz <= CLOUD_RADIUS_CELLS; gz++) {
            cx = centerX + gx;
            cz = centerZ + gz;

            if (CloudCellVisible(cx, cz)) {
                x0 = (float)((double)cx * CLOUD_CELL_SIZE - cloudOffsetX - (double)worldOriginBlockX);
                z0 = (float)((double)cz * CLOUD_CELL_SIZE - cloudOffsetZ - (double)worldOriginBlockZ);
                x1 = x0 + CLOUD_CELL_SIZE;
                z1 = z0 + CLOUD_CELL_SIZE;

                DrawCloudCell(x0, z0, x1, z1, alpha);
            }
        }
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void DrawCloudCell(float x0, float z0, float x1, float z1, float alpha)
{
    float y;

    y = CLOUD_HEIGHT;

    glColor4f(1.0f, 1.0f, 1.0f, alpha);

    glBegin(GL_QUADS);
    glVertex3f(x0, y, z0);
    glVertex3f(x1, y, z0);
    glVertex3f(x1, y, z1);
    glVertex3f(x0, y, z1);
    glEnd();
}


/* ------------------------------------------------------------ */
/* Beta-style mob system                                        */
/* ------------------------------------------------------------ */

void StopMobSounds(void)
{
    int i;
    char aliasName[32];
    char cmd[128];

    for (i = 0; i < MOB_SOUND_ALIAS_COUNT; i++) {
        wsprintf(aliasName, "mobSnd%d", i);
        wsprintf(cmd, "stop %s", aliasName);
        mciSendString(cmd, NULL, 0, NULL);
        wsprintf(cmd, "close %s", aliasName);
        mciSendString(cmd, NULL, 0, NULL);
    }
}

void PlayOneShotMP3(const char *filename)
{
    char aliasName[32];
    char cmd[512];
    MCIERROR err;

    if (!filename || filename[0] == '\0') {
        return;
    }

    wsprintf(aliasName, "mobSnd%d", g_nextMobSoundAlias);
    g_nextMobSoundAlias++;

    if (g_nextMobSoundAlias >= MOB_SOUND_ALIAS_COUNT) {
        g_nextMobSoundAlias = 0;
    }

    wsprintf(cmd, "stop %s", aliasName);
    mciSendString(cmd, NULL, 0, NULL);
    wsprintf(cmd, "close %s", aliasName);
    mciSendString(cmd, NULL, 0, NULL);

    wsprintf(cmd, "open \"%s\" type mpegvideo alias %s", filename, aliasName);
    err = mciSendString(cmd, NULL, 0, NULL);

    if (err != 0) {
        return;
    }

    wsprintf(cmd, "play %s", aliasName);
    mciSendString(cmd, NULL, 0, NULL);
}


void PlayUIClickSound(void)
{
    PlayOneShotMP3("assets\\sounds\\ui_click.mp3");
}

void PlayPlayerHitSound(void)
{
    PlayOneShotMP3("assets\\sounds\\player_hit.mp3");
}

void PlayItemPickupSound(void)
{
    /* Java plays random.pop when EntityItem joins the inventory.
       This Open Watcom build uses an MP3 copy named assets\sounds\random\pop.mp3. */
    if (g_worldTimeSeconds - g_lastPickupSoundTime < 0.04) {
        return;
    }
    g_lastPickupSoundTime = g_worldTimeSeconds;
    PlayOneShotMP3(SOUND_PICKUP_FILE);
}

void PlayPlayerStepSound(int blockBelow)
{
    char path[160];
    int group;
    int variant;

    g_playerStepSoundPhase++;
    variant = 1 + (g_playerStepSoundPhase & 3);
    group = GetBlockSoundGroup(blockBelow);

    if (group == 1) {
        wsprintf(path, "assets\\sounds\\player\\step_wood%d.mp3", variant);
    } else if (group == 2) {
        wsprintf(path, "assets\\sounds\\player\\step_sand%d.mp3", variant);
    } else if (group == 3) {
        wsprintf(path, "assets\\sounds\\player\\step_snow%d.mp3", variant);
    } else if (group == 4) {
        wsprintf(path, "assets\\sounds\\player\\step_cloth%d.mp3", variant);
    } else if (group == 5) {
        wsprintf(path, "assets\\sounds\\player\\step_grass%d.mp3", variant);
    } else {
        wsprintf(path, "assets\\sounds\\player\\step_stone%d.mp3", variant);
    }

    PlayOneShotMP3(path);
}


void PlayMobIdleSound(int type, int angry)
{
    if (type == MOB_CHICKEN) {
        PlayOneShotMP3("assets\\sounds\\mob\\chicken\\say1.mp3");
    } else if (type == MOB_COW) {
        PlayOneShotMP3("assets\\sounds\\mob\\cow\\say1.mp3");
    } else if (type == MOB_SHEEP) {
        PlayOneShotMP3("assets\\sounds\\mob\\sheep\\say1.mp3");
    } else if (type == MOB_PIG) {
        PlayOneShotMP3("assets\\sounds\\mob\\pig\\say1.mp3");
    } else if (type == MOB_WOLF) {
        if (angry) {
            PlayOneShotMP3("assets\\sounds\\mob\\wolf\\growl1.mp3");
        } else {
            PlayOneShotMP3("assets\\sounds\\mob\\wolf\\bark1.mp3");
        }
    } else if (type == MOB_ZOMBIE) {
        PlayOneShotMP3("assets\\sounds\\mob\\zombie\\say1.mp3");
    } else if (type == MOB_SKELETON) {
        PlayOneShotMP3("assets\\sounds\\mob\\skeleton\\say1.mp3");
    } else if (type == MOB_CREEPER) {
        PlayOneShotMP3("assets\\sounds\\mob\\creeper\\say1.mp3");
    } else if (type == MOB_SPIDER) {
        PlayOneShotMP3("assets\\sounds\\mob\\spider\\say1.mp3");
    } else if (type == MOB_SLIME) {
        PlayOneShotMP3("assets\\sounds\\mob\\slime\\small1.mp3");
    }
}

void PlayMobHurtSound(int type)
{
    if (type == MOB_CHICKEN) {
        PlayOneShotMP3("assets\\sounds\\mob\\chicken\\hurt1.mp3");
    } else if (type == MOB_COW) {
        PlayOneShotMP3("assets\\sounds\\mob\\cow\\hurt1.mp3");
    } else if (type == MOB_SHEEP) {
        PlayOneShotMP3("assets\\sounds\\mob\\sheep\\shear.mp3");
    } else if (type == MOB_PIG) {
        PlayOneShotMP3("assets\\sounds\\mob\\pig\\say1.mp3");
    } else if (type == MOB_WOLF) {
        PlayOneShotMP3("assets\\sounds\\mob\\wolf\\hurt1.mp3");
    } else if (type == MOB_ZOMBIE) {
        PlayOneShotMP3("assets\\sounds\\mob\\zombie\\hurt1.mp3");
    } else if (type == MOB_SKELETON) {
        PlayOneShotMP3("assets\\sounds\\mob\\skeleton\\hurt1.mp3");
    } else if (type == MOB_CREEPER) {
        PlayOneShotMP3("assets\\sounds\\mob\\creeper\\death.mp3");
    } else if (type == MOB_SPIDER) {
        PlayOneShotMP3("assets\\sounds\\mob\\spider\\death.mp3");
    } else if (type == MOB_SLIME) {
        PlayOneShotMP3("assets\\sounds\\mob\\slime\\small1.mp3");
    }
}

double MobDistanceSquaredToPlayer(Mob *m)
{
    double dx;
    double dz;

    if (!m) {
        return 999999.0;
    }

    dx = m->x - playerX;
    dz = m->z - playerZ;

    return dx * dx + dz * dz;
}


int IsMobInsideLoadedWindow(Mob *m)
{
    if (!m) {
        return 0;
    }

    if (m->x < 1.0 || m->z < 1.0 ||
        m->x > (double)(WORLD_X - 2) ||
        m->z > (double)(WORLD_Z - 2)) {
        return 0;
    }

    if (m->y < 1.0 || m->y > (double)(WORLD_Y - 2)) {
        return 0;
    }

    return 1;
}

void RebaseMobsAfterWorldStream(int oldOriginX, int oldOriginZ)
{
    int i;
    double gx;
    double gz;
    double pgx;
    double pgz;
    double dx;
    double dz;

    pgx = GetPlayerGlobalX();
    pgz = GetPlayerGlobalZ();

    for (i = 0; i < MAX_MOBS; i++) {
        if (!mobs[i].active) {
            continue;
        }

        gx = (double)oldOriginX + mobs[i].x;
        gz = (double)oldOriginZ + mobs[i].z;
        mobs[i].x = gx - (double)worldOriginBlockX;
        mobs[i].z = gz - (double)worldOriginBlockZ;

        dx = gx - pgx;
        dz = gz - pgz;

        if ((dx * dx + dz * dz) > MOB_DESPAWN_DISTANCE * MOB_DESPAWN_DISTANCE) {
            mobs[i].active = 0;
        }
    }
}

void PlayMobIdleSoundNear(Mob *m)
{
    double maxDist;

    if (!m) {
        return;
    }

    maxDist = MOB_HEAR_IDLE_DISTANCE;

    if (m->type == MOB_ZOMBIE || m->type == MOB_SKELETON ||
        m->type == MOB_CREEPER || m->type == MOB_SPIDER) {
        maxDist = 36.0;
    }

    if (MobDistanceSquaredToPlayer(m) <= maxDist * maxDist) {
        PlayMobIdleSound(m->type, m->angry);
    }
}

void PlayMobHurtSoundNear(Mob *m)
{
    if (!m) {
        return;
    }

    if (MobDistanceSquaredToPlayer(m) <= MOB_HEAR_HURT_DISTANCE * MOB_HEAR_HURT_DISTANCE) {
        PlayMobHurtSound(m->type);
    }
}

void PlayMobStepSoundNear(Mob *m, int blockBelow)
{
    int phase;

    if (!m) {
        return;
    }

    if (blockBelow == BLOCK_WATER || m->type == MOB_SQUID) {
        return;
    }

    if (MobDistanceSquaredToPlayer(m) > MOB_HEAR_STEP_DISTANCE * MOB_HEAR_STEP_DISTANCE) {
        return;
    }

    phase = (WorldHash3D((int)m->x, (int)m->y, (int)m->z, g_worldSeed + (int)(g_worldTimeSeconds * 8.0)) % 3) + 1;

    if (m->type == MOB_CHICKEN) {
        if (phase == 1) { PlayOneShotMP3("assets\\sounds\\mob\\chicken\\step1.mp3"); }
        else { PlayOneShotMP3("assets\\sounds\\mob\\chicken\\step2.mp3"); }
    } else if (m->type == MOB_COW) {
        if (phase == 1) { PlayOneShotMP3("assets\\sounds\\mob\\cow\\step1.mp3"); }
        else if (phase == 2) { PlayOneShotMP3("assets\\sounds\\mob\\cow\\step2.mp3"); }
        else { PlayOneShotMP3("assets\\sounds\\mob\\cow\\step3.mp3"); }
    } else if (m->type == MOB_SHEEP) {
        if (phase == 1) { PlayOneShotMP3("assets\\sounds\\mob\\sheep\\step1.mp3"); }
        else if (phase == 2) { PlayOneShotMP3("assets\\sounds\\mob\\sheep\\step2.mp3"); }
        else { PlayOneShotMP3("assets\\sounds\\mob\\sheep\\step3.mp3"); }
    } else if (m->type == MOB_PIG) {
        if (phase == 1) { PlayOneShotMP3("assets\\sounds\\mob\\pig\\step1.mp3"); }
        else if (phase == 2) { PlayOneShotMP3("assets\\sounds\\mob\\pig\\step2.mp3"); }
        else { PlayOneShotMP3("assets\\sounds\\mob\\pig\\step3.mp3"); }
    } else if (m->type == MOB_WOLF) {
        if (phase == 1) { PlayOneShotMP3("assets\\sounds\\mob\\wolf\\step1.mp3"); }
        else if (phase == 2) { PlayOneShotMP3("assets\\sounds\\mob\\wolf\\step2.mp3"); }
        else { PlayOneShotMP3("assets\\sounds\\mob\\wolf\\step3.mp3"); }
    } else if (m->type == MOB_ZOMBIE || m->type == MOB_CREEPER) {
        if (phase == 1) { PlayOneShotMP3("assets\\sounds\\mob\\zombie\\step1.mp3"); }
        else if (phase == 2) { PlayOneShotMP3("assets\\sounds\\mob\\zombie\\step2.mp3"); }
        else { PlayOneShotMP3("assets\\sounds\\mob\\zombie\\step3.mp3"); }
    } else if (m->type == MOB_SKELETON) {
        if (phase == 1) { PlayOneShotMP3("assets\\sounds\\mob\\skeleton\\step1.mp3"); }
        else if (phase == 2) { PlayOneShotMP3("assets\\sounds\\mob\\skeleton\\step2.mp3"); }
        else { PlayOneShotMP3("assets\\sounds\\mob\\skeleton\\step3.mp3"); }
    } else if (m->type == MOB_SPIDER) {
        if (phase == 1) { PlayOneShotMP3("assets\\sounds\\mob\\spider\\step1.mp3"); }
        else if (phase == 2) { PlayOneShotMP3("assets\\sounds\\mob\\spider\\step2.mp3"); }
        else { PlayOneShotMP3("assets\\sounds\\mob\\spider\\step3.mp3"); }
    } else if (m->type == MOB_SLIME) {
        PlayOneShotMP3("assets\\sounds\\mob\\slime\\small1.mp3");
    }
}

int CountMobGroup(int hostileGroup)
{
    int i;
    int count;

    count = 0;

    for (i = 0; i < MAX_MOBS; i++) {
        if (!mobs[i].active) {
            continue;
        }

        if (hostileGroup == 1 && IsHostileMobType(mobs[i].type)) {
            count++;
        } else if (hostileGroup == 0 && IsPassiveMobType(mobs[i].type) && mobs[i].type != MOB_SQUID) {
            count++;
        } else if (hostileGroup == 2 && mobs[i].type == MOB_SQUID) {
            count++;
        }
    }

    return count;
}

int IsMobAggressiveNow(Mob *m)
{
    int bx;
    int by;
    int bz;
    int openSky;

    if (!m) {
        return 0;
    }

    if (m->angry) {
        return 1;
    }

    if (m->type == MOB_CREEPER || m->type == MOB_ZOMBIE ||
        m->type == MOB_SKELETON || m->type == MOB_SLIME) {
        return 1;
    }

    if (m->type == MOB_SPIDER) {
        bx = (int)floor(m->x);
        by = (int)floor(m->y);
        bz = (int)floor(m->z);
        openSky = IsSkyOpenForSpawn(bx, by, bz);

        if (!IsDaylightForMobs() || !openSky) {
            return 1;
        }
    }

    return 0;
}

int PickPassiveMobType(int x, int z, int below)
{
    int r;
    int gx;
    int gz;

    if (below == BLOCK_WATER) {
        return MOB_SQUID;
    }

    gx = LocalToGlobalBlockX(x);
    gz = LocalToGlobalBlockZ(z);
    r = WorldHash2D(gx, gz, g_worldSeed + 9100) % 100;

    if (r < 28) {
        return MOB_SHEEP;
    }
    if (r < 52) {
        return MOB_COW;
    }
    if (r < 75) {
        return MOB_PIG;
    }
    if (r < 94) {
        return MOB_CHICKEN;
    }

    return MOB_WOLF;
}

int PickHostileMobType(int x, int y, int z, int underground)
{
    int r;
    int gx;
    int gz;

    gx = LocalToGlobalBlockX(x);
    gz = LocalToGlobalBlockZ(z);
    r = WorldHash3D(gx, y, gz, g_worldSeed + 9300) % 100;

    if (underground && y < 38 && r > 91) {
        return MOB_SLIME;
    }

    if (r < 29) {
        return MOB_ZOMBIE;
    }
    if (r < 54) {
        return MOB_SKELETON;
    }
    if (r < 75) {
        return MOB_SPIDER;
    }
    if (r < 96) {
        return MOB_CREEPER;
    }

    return MOB_SLIME;
}

int IsValidMobSpawnSpace(int type, int hostile, int x, int y, int z)
{
    int below;
    int feet;
    int head;
    int openSky;
    int light;

    if (x < 3 || z < 3 || x >= WORLD_X - 3 || z >= WORLD_Z - 3) {
        return 0;
    }

    if (y < 3 || y >= WORLD_Y - 4) {
        return 0;
    }

    below = GetBlock(x, y - 1, z);
    feet = GetBlock(x, y, z);
    head = GetBlock(x, y + 1, z);
    openSky = IsSkyOpenForSpawn(x, y, z);
    light = GetLegacyLightLevel(x, y, z);

    if (type == MOB_SQUID) {
        if (feet != BLOCK_WATER) {
            return 0;
        }
        if (head != BLOCK_WATER && head != BLOCK_AIR) {
            return 0;
        }
        if (y < 4 || y > GEN_WATER_LEVEL + 2) {
            return 0;
        }
        return 1;
    }

    if (feet != BLOCK_AIR || head != BLOCK_AIR) {
        return 0;
    }

    if (!IsSolidBlock(below)) {
        return 0;
    }

    if (!hostile) {
        if (below != BLOCK_GRASS) {
            return 0;
        }
        if (!openSky) {
            return 0;
        }
        if (y < GEN_WATER_LEVEL + 1) {
            return 0;
        }
        return 1;
    }

    if (below == BLOCK_WATER || below == BLOCK_LEAVES) {
        return 0;
    }

    if (IsDaylightForMobs() && openSky) {
        return 0;
    }

    if (!openSky && light > 7) {
        return 0;
    }

    return 1;
}

int FindSurfaceMobSpawn(int hostile, int *sx, int *sy, int *sz, int *typeOut)
{
    int tries;
    int dx;
    int dz;
    int x;
    int z;
    int y;
    int h;
    int below;
    int type;
    double distX;
    double distZ;
    double dist2;

    for (tries = 0; tries < MOB_SPAWN_ATTEMPTS; tries++) {
        h = WorldHash3D((int)GetTickCount() + tries * 23,
                        (int)playerX + tries * 17,
                        (int)playerZ - tries * 31,
                        g_worldSeed + 8400);

        dx = (h % (MOB_SPAWN_RADIUS_MAX * 2 + 1)) - MOB_SPAWN_RADIUS_MAX;
        dz = ((h >> 8) % (MOB_SPAWN_RADIUS_MAX * 2 + 1)) - MOB_SPAWN_RADIUS_MAX;

        if (dx > -MOB_SPAWN_RADIUS_MIN && dx < MOB_SPAWN_RADIUS_MIN &&
            dz > -MOB_SPAWN_RADIUS_MIN && dz < MOB_SPAWN_RADIUS_MIN) {
            continue;
        }

        x = (int)floor(playerX) + dx;
        z = (int)floor(playerZ) + dz;

        if (x < 3 || z < 3 || x >= WORLD_X - 3 || z >= WORLD_Z - 3) {
            continue;
        }

        distX = (double)x + 0.5 - playerX;
        distZ = (double)z + 0.5 - playerZ;
        dist2 = distX * distX + distZ * distZ;

        if (dist2 < (double)(MOB_SPAWN_RADIUS_MIN * MOB_SPAWN_RADIUS_MIN)) {
            continue;
        }

        if (!hostile && (h & 7) == 0 && CountMobGroup(2) < MOB_WATER_CAP) {
            for (y = GEN_WATER_LEVEL; y >= 3; y--) {
                if (GetBlock(x, y, z) == BLOCK_WATER &&
                    IsValidMobSpawnSpace(MOB_SQUID, 0, x, y, z)) {
                    *sx = x;
                    *sy = y;
                    *sz = z;
                    *typeOut = MOB_SQUID;
                    return 1;
                }
            }
        }

        y = columnTop[x][z] + 1;
        if (y < 3 || y >= WORLD_Y - 3) {
            continue;
        }

        below = GetBlock(x, y - 1, z);

        if (hostile) {
            if (IsDaylightForMobs()) {
                continue;
            }
            type = PickHostileMobType(x, y, z, 0);
        } else {
            type = PickPassiveMobType(x, z, below);
        }

        if (!IsValidMobSpawnSpace(type, hostile, x, y, z)) {
            continue;
        }

        *sx = x;
        *sy = y;
        *sz = z;
        *typeOut = type;
        return 1;
    }

    return 0;
}

int FindUndergroundHostileSpawn(int *sx, int *sy, int *sz, int *typeOut)
{
    int tries;
    int dx;
    int dz;
    int x;
    int z;
    int y;
    int yy;
    int surface;
    int h;
    int type;
    double distX;
    double distZ;
    double dist2;

    for (tries = 0; tries < MOB_SPAWN_ATTEMPTS * 2; tries++) {
        h = WorldHash3D((int)GetTickCount() + tries * 37,
                        (int)playerX - tries * 19,
                        (int)playerZ + tries * 29,
                        g_worldSeed + 8700);

        dx = (h % (MOB_SPAWN_RADIUS_MAX * 2 + 1)) - MOB_SPAWN_RADIUS_MAX;
        dz = ((h >> 9) % (MOB_SPAWN_RADIUS_MAX * 2 + 1)) - MOB_SPAWN_RADIUS_MAX;

        if (dx > -MOB_SPAWN_RADIUS_MIN && dx < MOB_SPAWN_RADIUS_MIN &&
            dz > -MOB_SPAWN_RADIUS_MIN && dz < MOB_SPAWN_RADIUS_MIN) {
            continue;
        }

        x = (int)floor(playerX) + dx;
        z = (int)floor(playerZ) + dz;

        if (x < 3 || z < 3 || x >= WORLD_X - 3 || z >= WORLD_Z - 3) {
            continue;
        }

        distX = (double)x + 0.5 - playerX;
        distZ = (double)z + 0.5 - playerZ;
        dist2 = distX * distX + distZ * distZ;

        if (dist2 < (double)(MOB_SPAWN_RADIUS_MIN * MOB_SPAWN_RADIUS_MIN)) {
            continue;
        }

        surface = columnTop[x][z];
        if (surface < 14) {
            continue;
        }

        y = 5 + ((h >> 16) % (surface - 8));
        if (y > 56) {
            y = 56 - (tries % 8);
        }
        if (y < 5) {
            y = 5;
        }

        for (yy = y; yy > 4 && yy > y - 10; yy--) {
            type = PickHostileMobType(x, yy, z, 1);
            if (IsValidMobSpawnSpace(type, 1, x, yy, z)) {
                *sx = x;
                *sy = yy;
                *sz = z;
                *typeOut = type;
                return 1;
            }
        }

        for (yy = y; yy < surface - 2 && yy < y + 10 && yy < WORLD_Y - 4; yy++) {
            type = PickHostileMobType(x, yy, z, 1);
            if (IsValidMobSpawnSpace(type, 1, x, yy, z)) {
                *sx = x;
                *sy = yy;
                *sz = z;
                *typeOut = type;
                return 1;
            }
        }
    }

    return 0;
}


int IsPassiveMobType(int type)
{
    if (type == MOB_CHICKEN || type == MOB_COW || type == MOB_SHEEP ||
        type == MOB_WOLF || type == MOB_SQUID || type == MOB_PIG) {
        return 1;
    }

    return 0;
}

int IsHostileMobType(int type)
{
    if (type == MOB_ZOMBIE || type == MOB_SKELETON || type == MOB_CREEPER ||
        type == MOB_SPIDER || type == MOB_SLIME) {
        return 1;
    }

    return 0;
}

int IsDaylightForMobs(void)
{
    if (g_dayNightBlend > 0.55f) {
        return 1;
    }

    return 0;
}

int IsNightHostile(int type)
{
    if (type == MOB_ZOMBIE || type == MOB_SKELETON || type == MOB_CREEPER ||
        type == MOB_SLIME) {
        return 1;
    }

    if (type == MOB_SPIDER && !IsDaylightForMobs()) {
        return 1;
    }

    return 0;
}

int MobBurnsInDaylight(int type)
{
    if (type == MOB_ZOMBIE || type == MOB_SKELETON) {
        return 1;
    }

    return 0;
}

float MobWidth(int type)
{
    if (type == MOB_CHICKEN) {
        return 0.65f;
    }
    if (type == MOB_SLIME) {
        return 1.10f;
    }
    if (type == MOB_PIG) {
        return 0.95f;
    }
    if (type == MOB_SPIDER) {
        return 1.75f;
    }
    if (type == MOB_SQUID) {
        return 1.15f;
    }
    return 0.95f;
}

float MobHeight(int type)
{
    if (type == MOB_CHICKEN) {
        return 0.90f;
    }
    if (type == MOB_SPIDER) {
        return 0.90f;
    }
    if (type == MOB_SLIME) {
        return 1.05f;
    }
    if (type == MOB_CREEPER || type == MOB_ZOMBIE || type == MOB_SKELETON) {
        return 1.85f;
    }
    if (type == MOB_PIG) {
        return 1.05f;
    }
    if (type == MOB_SQUID) {
        return 1.20f;
    }
    return 1.35f;
}

GLuint GetMobTexture(Mob *m)
{
    if (!m) {
        return 0;
    }
    if (m->type == MOB_CHICKEN) {
        return texMobChicken;
    }
    if (m->type == MOB_COW) {
        return texMobCow;
    }
    if (m->type == MOB_SHEEP) {
        if (m->sheared) {
            return texMobSheep;
        }
        return texMobSheepFur;
    }
    if (m->type == MOB_WOLF) {
        if (m->angry) {
            return texMobWolfAngry;
        }
        return texMobWolf;
    }
    if (m->type == MOB_SQUID) {
        return texMobSquid;
    }
    if (m->type == MOB_PIG) {
        return texMobPig;
    }
    if (m->type == MOB_ZOMBIE) {
        return texMobZombie;
    }
    if (m->type == MOB_SKELETON) {
        return texMobSkeleton;
    }
    if (m->type == MOB_CREEPER) {
        return texMobCreeper;
    }
    if (m->type == MOB_SPIDER) {
        return texMobSpider;
    }
    if (m->type == MOB_SLIME) {
        return texMobSlime;
    }
    return 0;
}

void InitMobs(void)
{
    int i;
    for (i = 0; i < MAX_MOBS; i++) {
        mobs[i].active = 0;
    }
    g_mobSpawnTimer = 1.0;
}

int CountActiveMobs(void)
{
    int i;
    int count;
    count = 0;
    for (i = 0; i < MAX_MOBS; i++) {
        if (mobs[i].active) {
            count++;
        }
    }
    return count;
}

int CountMobType(int type)
{
    int i;
    int count;
    count = 0;
    for (i = 0; i < MAX_MOBS; i++) {
        if (mobs[i].active && mobs[i].type == type) {
            count++;
        }
    }
    return count;
}

int AddMob(int type, double x, double y, double z)
{
    int i;
    int health;
    for (i = 0; i < MAX_MOBS; i++) {
        if (!mobs[i].active) {
            health = 8;
            if (type == MOB_COW || type == MOB_WOLF || type == MOB_PIG) {
                health = 10;
            } else if (type == MOB_CREEPER || type == MOB_ZOMBIE ||
                       type == MOB_SKELETON || type == MOB_SPIDER) {
                health = 12;
            } else if (type == MOB_SLIME) {
                health = 6;
            }
            mobs[i].active = 1;
            mobs[i].type = type;
            mobs[i].health = health;
            mobs[i].angry = 0;
            mobs[i].sheared = 0;
            mobs[i].burning = 0;
            mobs[i].x = x;
            mobs[i].y = y;
            mobs[i].z = z;
            mobs[i].vx = 0.0;
            mobs[i].vy = 0.0;
            mobs[i].vz = 0.0;
            mobs[i].yaw = 0.0;
            mobs[i].thinkTimer = 0.0;
            mobs[i].soundTimer = 2.0 + (double)(WorldHash3D((int)x, (int)y, (int)z, g_worldSeed + type) % 500) / 100.0;
            mobs[i].attackTimer = 0.0;
            mobs[i].burnTimer = 0.0;
            mobs[i].fleeTimer = 0.0;
            mobs[i].fuseTimer = 0.0;
            mobs[i].stepTimer = 0.4;
            mobs[i].spawnGraceTimer = 1.0;
            mobs[i].animWalk = 0.0;
            mobs[i].targetX = 0.0;
            mobs[i].targetZ = 0.0;
            mobs[i].pathTimer = 0.0;
            return i;
        }
    }
    return -1;
}

int FindMobSpawnPoint(int hostile, int *sx, int *sy, int *sz, int *typeOut)
{
    int preferUnderground;
    int h;

    h = WorldHash3D((int)GetTickCount(), (int)playerX, (int)playerZ, g_worldSeed + 8850);
    preferUnderground = h & 1;

    if (hostile) {
        if (preferUnderground) {
            if (FindUndergroundHostileSpawn(sx, sy, sz, typeOut)) {
                return 1;
            }
            return FindSurfaceMobSpawn(1, sx, sy, sz, typeOut);
        }

        if (!IsDaylightForMobs()) {
            if (FindSurfaceMobSpawn(1, sx, sy, sz, typeOut)) {
                return 1;
            }
        }

        return FindUndergroundHostileSpawn(sx, sy, sz, typeOut);
    }

    return FindSurfaceMobSpawn(0, sx, sy, sz, typeOut);
}


void SpawnInitialMobs(void)
{
    int i;
    int x;
    int y;
    int z;
    int type;

    /* Initial population follows normal spawn rules: passives on grass,
       squids in water, and hostiles only at night or in dark cave pockets. */
    for (i = 0; i < 18; i++) {
        if (CountMobGroup(0) < MOB_PASSIVE_CAP) {
            if (FindMobSpawnPoint(0, &x, &y, &z, &type)) {
                AddMob(type, (double)x + 0.5, (double)y, (double)z + 0.5);
            }
        }
    }

    for (i = 0; i < 10; i++) {
        if (CountMobGroup(1) < MOB_HOSTILE_CAP) {
            if (FindMobSpawnPoint(1, &x, &y, &z, &type)) {
                AddMob(type, (double)x + 0.5, (double)y, (double)z + 0.5);
            }
        }
    }
}


int FindMobShowcaseSurface(int startX, int startZ, int *sx, int *sy, int *sz)
{
    int r;
    int ox;
    int oz;
    int x;
    int z;
    int y;
    int below;

    for (r = 0; r <= 8; r++) {
        for (ox = -r; ox <= r; ox++) {
            for (oz = -r; oz <= r; oz++) {
                if (abs(ox) != r && abs(oz) != r) {
                    continue;
                }

                x = startX + ox;
                z = startZ + oz;

                if (x < 4 || z < 4 || x >= WORLD_X - 4 || z >= WORLD_Z - 4) {
                    continue;
                }

                y = columnTop[x][z] + 1;

                if (y < GEN_WATER_LEVEL + 1 || y >= WORLD_Y - 4) {
                    continue;
                }

                if (GetBlock(x, y, z) != BLOCK_AIR ||
                    GetBlock(x, y + 1, z) != BLOCK_AIR) {
                    continue;
                }

                below = GetBlock(x, y - 1, z);
                if (below == BLOCK_GRASS || below == BLOCK_DIRT || below == BLOCK_STONE) {
                    *sx = x;
                    *sy = y;
                    *sz = z;
                    return 1;
                }
            }
        }
    }

    /* Last-resort visible test pad: do not let the game start with zero visible mobs. */
    x = startX;
    z = startZ;
    if (x < 5) { x = 5; }
    if (z < 5) { z = 5; }
    if (x > WORLD_X - 6) { x = WORLD_X - 6; }
    if (z > WORLD_Z - 6) { z = WORLD_Z - 6; }
    y = (int)floor(playerY);
    ForceSpawnPad(x, y, z);
    *sx = x;
    *sy = y;
    *sz = z;
    return 1;
}

void SpawnVisibleStarterMobs(void)
{
    int types[10];
    int ox[10];
    int oz[10];
    int i;
    int x;
    int y;
    int z;
    int idx;
    int px;
    int pz;

    /*
        This is a deliberate visual-test group.  Earlier builds sometimes spawned
        mobs beyond the short render distance or on failed terrain checks, so the
        player could boot the game and see no mobs at all.
    */
    types[0] = MOB_PIG;      ox[0] =  7; oz[0] =  0;
    types[1] = MOB_COW;      ox[1] = -7; oz[1] =  2;
    types[2] = MOB_SHEEP;    ox[2] =  4; oz[2] =  7;
    types[3] = MOB_CHICKEN;  ox[3] = -4; oz[3] = -7;
    types[4] = MOB_WOLF;     ox[4] =  9; oz[4] = -5;
    types[5] = MOB_CREEPER;  ox[5] = -10; oz[5] = -5;
    types[6] = MOB_SPIDER;   ox[6] =  0; oz[6] = 11;
    types[7] = MOB_SLIME;    ox[7] =  11; oz[7] =  5;
    types[8] = MOB_ZOMBIE;   ox[8] = -12; oz[8] =  6;
    types[9] = MOB_SKELETON; ox[9] =  12; oz[9] = -8;

    px = (int)floor(playerX);
    pz = (int)floor(playerZ);

    for (i = 0; i < 10; i++) {
        if (CountMobType(types[i]) > 0) {
            continue;
        }

        if (FindMobShowcaseSurface(px + ox[i], pz + oz[i], &x, &y, &z)) {
            idx = AddMob(types[i], (double)x + 0.5, (double)y, (double)z + 0.5);
            if (idx >= 0) {
                mobs[idx].soundTimer = 1.0 + (double)i * 0.25;
                mobs[idx].thinkTimer = 0.2;
                mobs[idx].fleeTimer = 0.0;
            }
        }
    }
}

void DropWoolNearMob(Mob *m)
{
    int count;

    if (!m) {
        return;
    }

    count = 1 + (WorldHash3D((int)m->x, (int)m->y, (int)m->z, g_worldSeed + 8200) % 2);
    AddDroppedItem(ITEM_WOOL, count,
                   m->x, m->y + 0.55, m->z,
                   0.05, 1.4, 0.03);
}

void DropMobLoot(Mob *m)
{
    int count;
    int hash;
    int porkItem;

    if (!m) {
        return;
    }

    hash = WorldHash3D((int)m->x, (int)m->y, (int)m->z, g_worldSeed + 8250);

    if (m->type == MOB_CHICKEN) {
        count = hash % 3;
        if (count > 0) { AddDroppedItem(ITEM_FEATHER, count, m->x, m->y + 0.45, m->z, 0.04, 1.2, 0.02); }
    } else if (m->type == MOB_COW) {
        count = hash % 3;
        if (count > 0) { AddDroppedItem(ITEM_LEATHER, count, m->x, m->y + 0.55, m->z, 0.04, 1.3, 0.02); }
    } else if (m->type == MOB_PIG) {
        count = hash % 3;
        porkItem = m->burning ? ITEM_PORK_COOKED : ITEM_PORK_RAW;
        if (count > 0) { AddDroppedItem(porkItem, count, m->x, m->y + 0.55, m->z, 0.04, 1.3, 0.02); }
    } else if (m->type == MOB_SHEEP) {
        if (!m->sheared) {
            DropWoolNearMob(m);
        }
    } else if (m->type == MOB_ZOMBIE) {
        count = hash % 3;
        if (count > 0) { AddDroppedItem(ITEM_FEATHER, count, m->x, m->y + 0.65, m->z, 0.04, 1.3, 0.02); }
    } else if (m->type == MOB_SKELETON) {
        count = hash % 3;
        if (count > 0) { AddDroppedItem(ITEM_ARROW, count, m->x, m->y + 0.65, m->z, 0.04, 1.3, 0.02); }
        count = (hash / 7) % 3;
        if (count > 0) { AddDroppedItem(ITEM_BONE, count, m->x, m->y + 0.65, m->z, -0.04, 1.2, 0.02); }
    } else if (m->type == MOB_CREEPER) {
        count = hash % 3;
        if (count > 0) { AddDroppedItem(ITEM_GUNPOWDER, count, m->x, m->y + 0.65, m->z, 0.04, 1.3, 0.02); }
    } else if (m->type == MOB_SPIDER) {
        count = hash % 3;
        if (count > 0) { AddDroppedItem(ITEM_STRING, count, m->x, m->y + 0.45, m->z, 0.04, 1.2, 0.02); }
    } else if (m->type == MOB_SLIME) {
        count = hash % 3;
        if (count > 0) { AddDroppedItem(ITEM_SLIMEBALL, count, m->x, m->y + 0.35, m->z, 0.04, 1.1, 0.02); }
    } else if (m->type == MOB_SQUID) {
        count = 1 + (hash % 3);
        AddDroppedItem(ITEM_DYE_POWDER, count, m->x, m->y + 0.45, m->z, 0.04, 1.1, 0.02);
    }
}




void ExplodeCreeper(Mob *m)
{
    int ix;
    int iy;
    int iz;
    int dx;
    int dy;
    int dz;
    int x;
    int y;
    int z;
    double d2;
    if (!m) {
        return;
    }
    ix = (int)floor(m->x);
    iy = (int)floor(m->y + 0.7);
    iz = (int)floor(m->z);
    for (dx = -3; dx <= 3; dx++) {
        for (dy = -2; dy <= 2; dy++) {
            for (dz = -3; dz <= 3; dz++) {
                x = ix + dx;
                y = iy + dy;
                z = iz + dz;
                if (!IsInsideWorld(x, y, z)) {
                    continue;
                }
                d2 = (double)(dx * dx + dy * dy + dz * dz);
                if (d2 <= 8.5 && GetBlock(x, y, z) != BLOCK_BORDER) {
                    SpawnBlockBreakParticles(x, y, z, GetBlock(x, y, z));
                    SetBlock(x, y, z, BLOCK_AIR);
                }
            }
        }
    }
    if ((playerX - m->x) * (playerX - m->x) +
        (playerZ - m->z) * (playerZ - m->z) < 16.0) {
        TakeDamage(8);
    }
    RecomputeLegacyLightingLocal(ix, iy, iz, 18);
}

void DamageMob(int index, int amount, double knockX, double knockZ)
{
    Mob *m;
    double len;
    if (index < 0 || index >= MAX_MOBS) {
        return;
    }
    m = &mobs[index];
    if (!m->active) {
        return;
    }
    m->health -= amount;
    m->hurtTime = 0.45;
    m->fleeTimer = 4.0;
    len = sqrt(knockX * knockX + knockZ * knockZ);
    if (len > 0.001) {
        m->vx += (knockX / len) * 4.0;
        m->vz += (knockZ / len) * 4.0;
    }
    if (m->type == MOB_SHEEP && !m->sheared) {
        m->sheared = 1;
        DropWoolNearMob(m);
        SpawnMobEffectParticles(m, BLOCK_WOOL, 16);
    }
    if (m->type == MOB_WOLF) {
        m->angry = 1;
        m->fleeTimer = 0.0;
    }
    PlayMobHurtSoundNear(m);
    SpawnMobEffectParticles(m, BLOCK_STONE, 10);
    if (m->health <= 0) {
        DropMobLoot(m);
        if (m->type == MOB_CREEPER) {
            PlayOneShotMP3("assets\\sounds\\mob\\creeper\\death.mp3");
        }
        m->active = 0;
    }
}

int AttackMobRaycast(void)
{
    double yawRad;
    double pitchRad;
    double dirX;
    double dirY;
    double dirZ;
    double startX;
    double startY;
    double startZ;
    double t;
    double px;
    double py;
    double pz;
    double dx;
    double dy;
    double dz;
    double radius;
    int i;
    yawRad = yaw * PI / 180.0;
    pitchRad = pitch * PI / 180.0;
    dirX = -sin(yawRad) * cos(pitchRad);
    dirY = sin(pitchRad);
    dirZ = -cos(yawRad) * cos(pitchRad);
    startX = playerX;
    startY = playerY + EYE_HEIGHT;
    startZ = playerZ;
    for (t = 0.0; t < RAY_DISTANCE; t += 0.18) {
        px = startX + dirX * t;
        py = startY + dirY * t;
        pz = startZ + dirZ * t;
        for (i = 0; i < MAX_MOBS; i++) {
            if (!mobs[i].active) {
                continue;
            }
            radius = (double)MobWidth(mobs[i].type) * 0.55;
            dx = px - mobs[i].x;
            dy = py - (mobs[i].y + (double)MobHeight(mobs[i].type) * 0.50);
            dz = pz - mobs[i].z;
            if (dx * dx + dy * dy + dz * dz < radius * radius) {
                DamageMob(i, 3, mobs[i].x - playerX, mobs[i].z - playerZ);
                return 1;
            }
        }
    }
    return 0;
}

void UpdateMobs(double dt)
{
    int i;
    int x;
    int y;
    int z;
    int type;
    int bx;
    int by;
    int bz;
    int footY;
    int blockBelow;
    int mobOnGround;
    int blocked;
    double dx;
    double dz;
    double dist2;
    double len;
    double speed;
    double wander;
    double targetX;
    double targetZ;
    double newX;
    double newY;
    double newZ;
    double horSpeed;
    double maxSpeed;
    double stepDelay;
    double avoidX;
    double avoidZ;
    int testX;
    int testY;
    int testZ;
    Mob *m;

    g_mobSpawnTimer -= dt;

    if (g_mobSpawnTimer <= 0.0) {
        g_mobSpawnTimer = 12.0;

        if (CountActiveMobs() < MAX_MOBS - 4) {
            if (IsDaylightForMobs() && CountMobGroup(0) < MOB_PASSIVE_CAP) {
                if (FindMobSpawnPoint(0, &x, &y, &z, &type)) {
                    AddMob(type, (double)x + 0.5, (double)y, (double)z + 0.5);
                }
            }

            if (CountMobGroup(1) < MOB_HOSTILE_CAP) {
                if (FindMobSpawnPoint(1, &x, &y, &z, &type)) {
                    AddMob(type, (double)x + 0.5, (double)y, (double)z + 0.5);
                }
            }
        }
    }

    for (i = 0; i < MAX_MOBS; i++) {
        if (!mobs[i].active) {
            continue;
        }

        m = &mobs[i];

        dist2 = MobDistanceSquaredToPlayer(m);

        if (dist2 > MOB_DESPAWN_DISTANCE * MOB_DESPAWN_DISTANCE) {
            m->active = 0;
            continue;
        }

        if (!IsMobInsideLoadedWindow(m)) {
            continue;
        }

        if (dist2 > (double)(GetMobRenderDistanceBlocks() - 8) * (double)(GetMobRenderDistanceBlocks() - 8)) {
            /* Keep mobs persistent around saved/loaded chunks, but do not run heavy AI/pathing far away. */
            continue;
        }

        if (m->attackTimer > 0.0) {
            m->attackTimer -= dt;
        }
        if (m->soundTimer > 0.0) {
            m->soundTimer -= dt;
        }
        if (m->thinkTimer > 0.0) {
            m->thinkTimer -= dt;
        }
        if (m->stepTimer > 0.0) {
            m->stepTimer -= dt;
        }
        if (m->spawnGraceTimer > 0.0) {
            m->spawnGraceTimer -= dt;
        }
        if (m->fleeTimer > 0.0) {
            m->fleeTimer -= dt;
        }
        if (m->hurtTime > 0.0) {
            m->hurtTime -= dt;
            if (m->hurtTime < 0.0) { m->hurtTime = 0.0; }
        }
        if (m->deathTime > 0.0) {
            m->deathTime -= dt;
            m->animWalk += dt * 8.0;
            if (m->deathTime <= 0.0) { m->active = 0; }
            continue;
        }

        bx = (int)floor(m->x);
        by = (int)floor(m->y);
        bz = (int)floor(m->z);

        if (MobBurnsInDaylight(m->type) && IsDaylightForMobs() &&
            IsSkyOpenForSpawn(bx, by, bz)) {
            m->burning = 1;
            m->burnTimer += dt;
            if (m->burnTimer > 1.0) {
                m->burnTimer = 0.0;
                m->health -= 2;
                SpawnMobEffectParticles(m, BLOCK_LIGHT, 5);
                if (m->health <= 0) {
                    KillMobRenderable(m);
                    continue;
                }
            }
        } else {
            m->burning = 0;
            m->burnTimer = 0.0;
        }

        if (m->soundTimer <= 0.0) {
            PlayMobIdleSoundNear(m);
            m->soundTimer = 5.0 + (double)(WorldHash3D(bx, by, bz, g_worldSeed + i) % 800) / 100.0;
            if (IsHostileMobType(m->type)) {
                m->soundTimer += 2.0;
            }
        }

        dx = playerX - m->x;
        dz = playerZ - m->z;
        dist2 = dx * dx + dz * dz;
        targetX = 0.0;
        targetZ = 0.0;
        speed = 1.05;

        mobOnGround = 0;
        blockBelow = GetBlock((int)floor(m->x), (int)floor(m->y - 0.06), (int)floor(m->z));
        if (IsSolidBlock(blockBelow)) {
            mobOnGround = 1;
        }

        /* PATCH_F11_MOB_GUI: face the player while targeting, even if the mob
           is pausing to attack or backing up. This avoids sideways sliding. */
        if (IsMobAggressiveNow(m) && dist2 > 0.0001) {
            MobApproachFacing(m, dx, dz, dt, 420.0);
        }

        if (m->type == MOB_SQUID) {
            speed = 0.65;
            if (GetBlock(bx, by, bz) == BLOCK_WATER || GetBlock(bx, by + 1, bz) == BLOCK_WATER) {
                targetX = sin(g_worldTimeSeconds * 0.7 + (double)i) * 0.45;
                targetZ = cos(g_worldTimeSeconds * 0.5 + (double)i) * 0.45;
                m->vy += sin(g_worldTimeSeconds + (double)i) * dt * 0.8;
                if (m->vy > 0.30) { m->vy = 0.30; }
                if (m->vy < -0.30) { m->vy = -0.30; }
            } else {
                m->vy -= GRAVITY * dt;
                m->fleeTimer = 2.0;
            }
        } else if (IsMobAggressiveNow(m)) {
            len = sqrt(dist2);

            if (len > 0.001) {
                targetX = dx / len;
                targetZ = dz / len;
            }

            speed = 1.85;

            if (m->type == MOB_ZOMBIE) {
                speed = 1.65;
            } else if (m->type == MOB_SPIDER) {
                speed = 2.55;
            } else if (m->type == MOB_CREEPER) {
                speed = 1.80;
            } else if (m->type == MOB_SLIME) {
                speed = 1.25;
            } else if (m->type == MOB_WOLF) {
                speed = 2.65;
            }

            if (m->type == MOB_SKELETON) {
                if (dist2 < 20.0 && len > 0.001) {
                    targetX = -dx / len;
                    targetZ = -dz / len;
                    speed = 1.35;
                } else if (dist2 < 144.0) {
                    targetX = 0.0;
                    targetZ = 0.0;
                    speed = 0.0;
                    if (m->attackTimer <= 0.0) {
                        TakeDamage(2);
                        PlayMobIdleSoundNear(m);
                        m->attackTimer = 1.60;
                    }
                }
            }

            if (m->type == MOB_CREEPER && dist2 < 9.5 && fabs((playerY + 0.8) - (m->y + 0.8)) < MOB_ATTACK_VERTICAL_DISTANCE) {
                m->fuseTimer += dt;
                speed = 0.25;
                if (((int)(m->fuseTimer * 8.0)) != ((int)((m->fuseTimer - dt) * 8.0))) {
                    SpawnMobEffectParticles(m, BLOCK_DIRT, 4);
                }
                if (m->fuseTimer > 1.6) {
                    ExplodeCreeper(m);
                    m->active = 0;
                    continue;
                }
            } else if (m->type == MOB_CREEPER) {
                m->fuseTimer = 0.0;
            }

            if (dist2 < MOB_ATTACK_DISTANCE * MOB_ATTACK_DISTANCE &&
                fabs((playerY + 0.8) - (m->y + 0.8)) < MOB_ATTACK_VERTICAL_DISTANCE &&
                m->attackTimer <= 0.0) {
                if (m->type == MOB_ZOMBIE || m->type == MOB_SPIDER ||
                    m->type == MOB_SLIME || m->type == MOB_WOLF) {
                    TakeDamage(2);
                    m->attackTimer = 1.0;
                }
            }
        } else if (m->fleeTimer > 0.0) {
            len = sqrt(dist2);
            if (len > 0.001) {
                targetX = -dx / len;
                targetZ = -dz / len;
            }
            speed = 2.25;
            if (m->type == MOB_CHICKEN) {
                speed = 2.00;
            }
        } else {
            if (m->thinkTimer <= 0.0) {
                wander = (double)(WorldHash3D(bx, by, bz, g_worldSeed + i * 31) % 6283) / 1000.0;
                m->vx = sin(wander) * 0.42;
                m->vz = cos(wander) * 0.42;
                m->thinkTimer = 1.5 + (double)(WorldHash3D(bz, by, bx, g_worldSeed + i * 17) % 300) / 100.0;
            }
        }

        if ((targetX != 0.0 || targetZ != 0.0) && m->type != MOB_SQUID) {
            testX = (int)floor(m->x + targetX * 0.85);
            testY = (int)floor(m->y);
            testZ = (int)floor(m->z + targetZ * 0.85);

            if (IsSolidBlock(GetBlock(testX, testY, testZ)) ||
                IsSolidBlock(GetBlock(testX, testY + 1, testZ))) {
                avoidX = -targetZ;
                avoidZ = targetX;

                testX = (int)floor(m->x + avoidX * 0.85);
                testZ = (int)floor(m->z + avoidZ * 0.85);

                if (!IsSolidBlock(GetBlock(testX, testY, testZ)) &&
                    !IsSolidBlock(GetBlock(testX, testY + 1, testZ))) {
                    targetX = avoidX;
                    targetZ = avoidZ;
                } else if (mobOnGround) {
                    m->vy = 4.2;
                }
            }
        }

        if (targetX != 0.0 || targetZ != 0.0) {
            MobPathSteer(m, targetX, targetZ, &targetX, &targetZ);
            MobApproachFacing(m, targetX, targetZ, dt, 520.0);
            m->vx += targetX * speed * dt * 3.0;
            m->vz += targetZ * speed * dt * 3.0;
        }

        if (m->type == MOB_SLIME && mobOnGround && m->thinkTimer <= 0.15) {
            m->vy = 4.4;
            PlayMobStepSoundNear(m, blockBelow);
        }

        m->vx *= 0.90;
        m->vz *= 0.90;

        maxSpeed = speed;
        if (maxSpeed < 0.25) {
            maxSpeed = 0.25;
        }

        if (m->vx > maxSpeed) { m->vx = maxSpeed; }
        if (m->vx < -maxSpeed) { m->vx = -maxSpeed; }
        if (m->vz > maxSpeed) { m->vz = maxSpeed; }
        if (m->vz < -maxSpeed) { m->vz = -maxSpeed; }

        if (m->type == MOB_CHICKEN && m->vy < -3.0) {
            m->vy = -3.0;
        }

        if (m->type != MOB_SQUID) {
            m->vy -= GRAVITY * dt;
        }

        newX = m->x + m->vx * dt;
        newY = m->y + m->vy * dt;
        newZ = m->z + m->vz * dt;

        if (newX < 2.0 || newX > WORLD_X - 3.0) {
            m->vx = -m->vx * 0.3;
            newX = m->x;
        }
        if (newZ < 2.0 || newZ > WORLD_Z - 3.0) {
            m->vz = -m->vz * 0.3;
            newZ = m->z;
        }

        x = (int)floor(newX);
        z = (int)floor(newZ);

        if (m->type != MOB_SQUID) {
            footY = (int)floor(newY - 0.06);
            if (m->vy <= 0.0 && IsSolidBlock(GetBlock(x, footY, z))) {
                newY = (double)footY + 1.0;
                m->vy = 0.0;
                mobOnGround = 1;
                blockBelow = GetBlock(x, footY, z);
            } else {
                mobOnGround = 0;
            }
        }

        y = (int)floor(newY);
        blocked = 0;

        if (m->type == MOB_SPIDER) {
            if (IsSolidBlock(GetBlock(x, y, z))) {
                blocked = 1;
            }
        } else if (m->type != MOB_SQUID) {
            if (IsSolidBlock(GetBlock(x, y, z)) ||
                IsSolidBlock(GetBlock(x, y + 1, z))) {
                blocked = 1;
            }
        }

        if (blocked) {
            if (mobOnGround && !IsSolidBlock(GetBlock(x, y + 2, z))) {
                newY = m->y + 0.50;
                m->vy = 4.6;
            } else if (m->type == MOB_SPIDER) {
                m->vy = 3.2;
                newX = m->x;
                newZ = m->z;
            } else {
                m->vx = -m->vx * 0.35;
                m->vz = -m->vz * 0.35;
                newX = m->x;
                newZ = m->z;
            }
        }

        if (m->type == MOB_SQUID) {
            if (GetBlock((int)floor(newX), (int)floor(newY), (int)floor(newZ)) != BLOCK_WATER &&
                GetBlock((int)floor(newX), (int)floor(newY + 1.0), (int)floor(newZ)) != BLOCK_WATER) {
                if (newY > GEN_WATER_LEVEL + 1) {
                    newY = GEN_WATER_LEVEL;
                }
            }
        }

        if (newY < 2.0) {
            newY = 2.0;
            m->vy = 0.0;
        }
        if (newY > WORLD_Y - 4) {
            newY = WORLD_Y - 4;
            m->vy = 0.0;
        }

        horSpeed = sqrt((newX - m->x) * (newX - m->x) + (newZ - m->z) * (newZ - m->z)) / (dt > 0.0001 ? dt : 0.0001);
        if (mobOnGround && horSpeed > 0.10 && m->type != MOB_SQUID) {
            if (m->stepTimer <= 0.0) {
                PlayMobStepSoundNear(m, blockBelow);
                stepDelay = 0.48;
                if (m->type == MOB_CHICKEN || m->type == MOB_SPIDER) { stepDelay = 0.28; }
                else if (m->type == MOB_ZOMBIE || m->type == MOB_SKELETON) { stepDelay = 0.55; }
                else if (m->type == MOB_WOLF) { stepDelay = 0.34; }
                else if (m->type == MOB_SLIME) { stepDelay = 0.90; }
                m->stepTimer = stepDelay;
            }
        }

        if (horSpeed > 0.025) {
            m->animWalk += horSpeed * dt * 5.6;
            MobApproachFacing(m, newX - m->x, newZ - m->z, dt, 520.0);
        } else {
            m->vx *= 0.50;
            m->vz *= 0.50;
        }

        m->prevX = m->x;
        m->prevZ = m->z;
        m->x = newX;
        m->y = newY;
        m->z = newZ;
    }
}


void DrawMobBillboard(Mob *m, GLuint tex, float width, float height)
{
    float cx;
    float cy;
    float cz;
    float halfW;
    float rightX;
    float rightZ;
    float yawRad;
    float bright;
    float pulse;

    if (!m || !tex) {
        return;
    }

    cx = (float)m->x;
    cy = (float)m->y;
    cz = (float)m->z;

    /* The uploaded mob PNGs are bitmap skin/model cards, so draw them larger. */
    halfW = width * 0.72f;
    if (m->type == MOB_CHICKEN) {
        halfW = 0.55f;
        height = 1.15f;
    } else if (m->type == MOB_SPIDER) {
        halfW = 1.10f;
        height = 0.90f;
    } else if (m->type == MOB_SLIME) {
        halfW = 0.80f;
        height = 1.05f;
    } else if (m->type == MOB_COW || m->type == MOB_SHEEP ||
               m->type == MOB_PIG || m->type == MOB_WOLF) {
        halfW = 0.92f;
        height = 1.35f;
    }

    yawRad = (float)(yaw * PI / 180.0);
    rightX = cos(yawRad) * halfW;
    rightZ = sin(yawRad) * halfW;

    bright = ApplyGammaBoost(0.68f + g_daySkyBrightness * 0.42f);
    if (bright > 1.0f) {
        bright = 1.0f;
    }

    if (m->burning) {
        pulse = (float)(0.75 + 0.25 * sin(g_worldTimeSeconds * 18.0));
        glColor4f(1.0f, pulse * 0.60f, pulse * 0.30f, 1.0f);
    } else {
        glColor4f(bright, bright, bright, 1.0f);
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    /* Existing TGA loader converts to top-left row order, so this UV orientation is upright. */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(cx - rightX, cy, cz - rightZ);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(cx + rightX, cy, cz + rightZ);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(cx + rightX, cy + height, cz + rightZ);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(cx - rightX, cy + height, cz - rightZ);
    glEnd();

    /* Cross-card second plane, like old sprite vegetation, so mobs remain visible from side angles. */
    rightX = -sin(yawRad) * halfW;
    rightZ =  cos(yawRad) * halfW;
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(cx - rightX, cy, cz - rightZ);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(cx + rightX, cy, cz + rightZ);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(cx + rightX, cy + height, cz + rightZ);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(cx - rightX, cy + height, cz - rightZ);
    glEnd();
}

void DrawMobShadow(Mob *m)
{
    int gx;
    int gz;
    int gy;
    float x;
    float z;
    float y;
    float size;
    if (!m) {
        return;
    }
    gx = (int)floor(m->x);
    gz = (int)floor(m->z);
    if (gx < 1 || gx >= WORLD_X - 1 || gz < 1 || gz >= WORLD_Z - 1) {
        return;
    }
    gy = (int)floor(m->y);
    while (gy > 1 && !IsSolidBlock(GetBlock(gx, gy - 1, gz))) {
        gy--;
    }
    y = (float)gy + 0.025f;
    x = (float)m->x;
    z = (float)m->z;
    size = MobWidth(m->type) * 0.60f;
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (texBetaShadow) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texBetaShadow);
        glColor4f(1.0f, 1.0f, 1.0f, 0.42f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.0f, 0.0f, 0.0f, 0.32f);
    }
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x - size, y, z - size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + size, y, z - size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + size, y, z + size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x - size, y, z + size);
    glEnd();
}



void MobSkinUV(int px, int py, int pw, int ph, float *u0, float *v0, float *u1, float *v1)
{
    float pad;

    pad = 0.30f;
    *u0 = ((float)px + pad) / 64.0f;
    *v0 = ((float)py + pad) / 32.0f;
    *u1 = ((float)(px + pw) - pad) / 64.0f;
    *v1 = ((float)(py + ph) - pad) / 32.0f;
}

void EmitMobSkinQuad(float x0, float y0, float z0,
                     float x1, float y1, float z1,
                     float x2, float y2, float z2,
                     float x3, float y3, float z3,
                     int px, int py, int pw, int ph)
{
    float u0;
    float v0;
    float u1;
    float v1;

    MobSkinUV(px, py, pw, ph, &u0, &v0, &u1, &v1);

    glTexCoord2f(u0, v1); glVertex3f(x0, y0, z0);
    glTexCoord2f(u1, v1); glVertex3f(x1, y1, z1);
    glTexCoord2f(u1, v0); glVertex3f(x2, y2, z2);
    glTexCoord2f(u0, v0); glVertex3f(x3, y3, z3);
}

void DrawMobSkinBoxPart(float cx, float cy, float cz,
                        float sx, float sy, float sz,
                        GLuint tex, float shade, float alpha,
                        int tx, int ty, int pw, int ph, int pd)
{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;

    if (!tex) {
        return;
    }

    x0 = cx - sx * 0.5f;
    x1 = cx + sx * 0.5f;
    y0 = cy - sy * 0.5f;
    y1 = cy + sy * 0.5f;
    z0 = cz - sz * 0.5f;
    z1 = cz + sz * 0.5f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor4f(shade, shade, shade, alpha);

    glBegin(GL_QUADS);

    EmitMobSkinQuad(x0, y0, z0, x1, y0, z0, x1, y1, z0, x0, y1, z0,
                    tx + pd, ty + pd, pw, ph);
    EmitMobSkinQuad(x1, y0, z1, x0, y0, z1, x0, y1, z1, x1, y1, z1,
                    tx + pd + pw + pd, ty + pd, pw, ph);
    EmitMobSkinQuad(x0, y0, z1, x0, y0, z0, x0, y1, z0, x0, y1, z1,
                    tx, ty + pd, pd, ph);
    EmitMobSkinQuad(x1, y0, z0, x1, y0, z1, x1, y1, z1, x1, y1, z0,
                    tx + pd + pw, ty + pd, pd, ph);
    EmitMobSkinQuad(x0, y1, z0, x1, y1, z0, x1, y1, z1, x0, y1, z1,
                    tx + pd, ty, pw, pd);
    EmitMobSkinQuad(x0, y0, z1, x1, y0, z1, x1, y0, z0, x0, y0, z0,
                    tx + pd + pw, ty, pw, pd);

    glEnd();
}


void DrawMobSkinBoxPartRot(float px, float py, float pz,
                           float cx, float cy, float cz,
                           float sx, float sy, float sz,
                           GLuint tex, float shade, float alpha,
                           int tx, int ty, int pw, int ph, int pd,
                           float rotX, float rotY, float rotZ)
{
    glPushMatrix();
    glTranslatef(px, py, pz);
    if (rotZ != 0.0f) { glRotatef(rotZ, 0.0f, 0.0f, 1.0f); }
    if (rotY != 0.0f) { glRotatef(rotY, 0.0f, 1.0f, 0.0f); }
    if (rotX != 0.0f) { glRotatef(rotX, 1.0f, 0.0f, 0.0f); }
    DrawMobSkinBoxPart(cx, cy, cz, sx, sy, sz, tex, shade, alpha, tx, ty, pw, ph, pd);
    glPopMatrix();
}

/* Clean-room blocky mob model renderer.
   The first mob pass used large billboard cards.  This pass replaces that
   with simple OpenGL 1.1 cuboid models so mobs read like voxel creatures
   while still compiling in Open Watcom and Win98-era OpenGL. */

void DrawMobBoxPart(float cx, float cy, float cz,
                    float sx, float sy, float sz,
                    GLuint tex, float shade, float alpha)
{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;

    if (!tex) {
        return;
    }

    x0 = cx - sx * 0.5f;
    x1 = cx + sx * 0.5f;
    y0 = cy - sy * 0.5f;
    y1 = cy + sy * 0.5f;
    z0 = cz - sz * 0.5f;
    z1 = cz + sz * 0.5f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor4f(shade, shade, shade, alpha);

    glBegin(GL_QUADS);

    /* Front */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y1, z0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y1, z0);

    /* Back */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x0, y1, z1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y1, z1);

    /* Left */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x0, y1, z0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y1, z1);

    /* Right */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y1, z0);

    /* Top */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y1, z1);

    /* Bottom */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z0);

    glEnd();
}

void RenderMobModelTex(Mob *m, GLuint tex, float alpha)
{
    float bright;
    float pulse;
    float shade;
    float walk;
    float bodyBob;
    float legDeg;
    float legDegOpp;
    float armDeg;
    float wingDeg;
    float scale;
    float side;
    float zpos;
    float yrot;
    float zrot;
    float walkFactor;
    int s;
    int t;

    if (!m || !tex) { return; }

    bright = ApplyGammaBoost(0.62f + g_daySkyBrightness * 0.48f);
    if (bright > 1.0f) { bright = 1.0f; }

    if (m->burning) {
        pulse = (float)(0.70 + 0.30 * sin(g_worldTimeSeconds * 20.0));
        shade = pulse;
    } else {
        shade = bright;
    }

    if (alpha > 1.0f) { alpha = 1.0f; }
    if (alpha < 0.05f) { alpha = 0.05f; }
    if (m->hurtTime > 0.0) {
        pulse = (float)(0.65 + 0.35 * sin(g_worldTimeSeconds * 48.0));
        shade = 1.0f;
        alpha = alpha * pulse;
    }
    if (m->deathTime > 0.0) {
        alpha = alpha * (float)(m->deathTime / 0.70);
        if (alpha < 0.10f) { alpha = 0.10f; }
    }

    /* Converted from Java model animation ideas: limb swing is cosine based,
       arms/legs rotate around the shoulder/hip instead of sliding through blocks. */
    walk = (float)m->animWalk;
    walkFactor = (float)(sqrt(m->vx * m->vx + m->vz * m->vz) / 1.45);
    if (walkFactor > 1.0f) { walkFactor = 1.0f; }
    if (walkFactor < 0.04f && m->type != MOB_SQUID) { walkFactor = 0.0f; }

    legDeg = (float)cos((double)walk * 0.6662) * 38.0f * walkFactor;
    legDegOpp = (float)cos((double)walk * 0.6662 + PI) * 38.0f * walkFactor;
    armDeg = (float)cos((double)walk * 0.6662 + PI) * 30.0f * walkFactor;
    wingDeg = (float)sin((double)walk * 1.7) * 35.0f * (walkFactor > 0.0f ? walkFactor : 0.15f);
    bodyBob = (float)(fabs(sin((double)walk)) * 0.025 * walkFactor);

    glPushMatrix();
    glTranslatef((float)m->x, (float)m->y, (float)m->z);
    if (m->deathTime > 0.0) {
        glRotatef((float)((1.0 - m->deathTime / 0.70) * 82.0), 0.0f, 0.0f, 1.0f);
    }
    /* Match the player renderer convention: model front is -Z, positive yaw
       turns the camera/player, so negate mob yaw for world-facing cuboids. */
    glRotatef((float)-m->yaw, 0.0f, 1.0f, 0.0f);
    glDisable(GL_CULL_FACE);

    if (m->type == MOB_ZOMBIE || m->type == MOB_SKELETON) {
        scale = 1.0f;
        if (m->type == MOB_SKELETON) { scale = 0.86f; }
        DrawMobSkinBoxPart(0.0f, 1.74f + bodyBob, 0.0f, 0.50f * scale, 0.50f, 0.50f * scale, tex, shade, alpha, 0, 0, 8, 8, 8);
        DrawMobSkinBoxPart(0.0f, 1.12f + bodyBob, 0.0f, 0.50f * scale, 0.75f, 0.25f * scale, tex, shade, alpha, 16, 16, 8, 12, 4);
        DrawMobSkinBoxPartRot(-0.38f * scale, 1.45f + bodyBob, 0.0f, 0.0f, -0.36f, 0.0f, 0.22f * scale, 0.74f, 0.22f * scale, tex, shade * 0.93f, alpha, 40, 16, 4, 12, 4, armDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.38f * scale, 1.45f + bodyBob, 0.0f, 0.0f, -0.36f, 0.0f, 0.22f * scale, 0.74f, 0.22f * scale, tex, shade * 0.93f, alpha, 40, 16, 4, 12, 4, -armDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot(-0.13f * scale, 0.75f, 0.0f, 0.0f, -0.36f, 0.0f, 0.22f * scale, 0.74f, 0.22f * scale, tex, shade * 0.84f, alpha, 0, 16, 4, 12, 4, -legDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.13f * scale, 0.75f, 0.0f, 0.0f, -0.36f, 0.0f, 0.22f * scale, 0.74f, 0.22f * scale, tex, shade * 0.84f, alpha, 0, 16, 4, 12, 4, legDeg, 0.0f, 0.0f);
    } else if (m->type == MOB_CREEPER) {
        DrawMobSkinBoxPart(0.0f, 1.60f + bodyBob, 0.0f, 0.64f, 0.64f, 0.64f, tex, shade, alpha, 0, 0, 8, 8, 8);
        DrawMobSkinBoxPart(0.0f, 0.96f + bodyBob, 0.0f, 0.56f, 0.86f, 0.30f, tex, shade, alpha, 16, 16, 8, 12, 4);
        DrawMobSkinBoxPartRot(-0.20f, 0.54f, -0.25f, 0.0f, -0.24f, 0.0f, 0.22f, 0.48f, 0.22f, tex, shade * 0.84f, alpha, 0, 16, 4, 6, 4, legDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.20f, 0.54f, -0.25f, 0.0f, -0.24f, 0.0f, 0.22f, 0.48f, 0.22f, tex, shade * 0.84f, alpha, 0, 16, 4, 6, 4, legDegOpp, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot(-0.20f, 0.54f,  0.25f, 0.0f, -0.24f, 0.0f, 0.22f, 0.48f, 0.22f, tex, shade * 0.84f, alpha, 0, 16, 4, 6, 4, legDegOpp, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.20f, 0.54f,  0.25f, 0.0f, -0.24f, 0.0f, 0.22f, 0.48f, 0.22f, tex, shade * 0.84f, alpha, 0, 16, 4, 6, 4, legDeg, 0.0f, 0.0f);
    } else if (m->type == MOB_PIG || m->type == MOB_COW || m->type == MOB_SHEEP || m->type == MOB_WOLF) {
        GLuint bodyTex;
        GLuint headTex;
        float bodyY;
        float bodyW;
        float bodyH;
        float bodyD;
        float headY;
        float headZ;
        float headW;
        float headH;
        float headD;
        float legH;
        float legW;

        bodyTex = tex;
        headTex = tex;
        bodyY = 0.70f + bodyBob;
        bodyW = 0.68f; bodyH = 0.52f; bodyD = 1.00f;
        headY = 0.98f + bodyBob; headZ = -0.50f; headW = 0.50f; headH = 0.50f; headD = 0.50f;
        legH = 0.50f; legW = 0.16f;

        if (m->type == MOB_COW) { bodyW = 0.84f; bodyH = 0.62f; bodyD = 1.15f; headY = 1.18f + bodyBob; headW = 0.55f; headH = 0.55f; headD = 0.42f; legH = 0.68f; legW = 0.18f; }
        if (m->type == MOB_SHEEP) { bodyW = 0.76f; bodyH = 0.62f; bodyD = 1.08f; headY = 1.10f + bodyBob; headW = 0.50f; headH = 0.50f; headD = 0.56f; legH = 0.62f; bodyTex = m->sheared ? texMobSheep : texMobSheepFur; headTex = bodyTex; }
        if (m->type == MOB_WOLF) { bodyW = 0.48f; bodyH = 0.44f; bodyD = 0.86f; headY = 1.05f + bodyBob; headZ = -0.60f; headW = 0.48f; headH = 0.42f; headD = 0.44f; legH = 0.55f; legW = 0.11f; }

        DrawMobSkinBoxPart(0.0f, bodyY, 0.10f, bodyW, bodyH, bodyD, bodyTex, shade, alpha, 28, 8, 10, 16, 8);
        DrawMobSkinBoxPart(0.0f, headY, headZ, headW, headH, headD, headTex, shade, alpha, 0, 0, 8, 8, 8);
        if (m->type == MOB_COW) {
            DrawMobSkinBoxPart(-0.24f, 1.55f + bodyBob, -0.62f, 0.07f, 0.22f, 0.07f, tex, shade * 0.95f, alpha, 22, 0, 1, 3, 1);
            DrawMobSkinBoxPart( 0.24f, 1.55f + bodyBob, -0.62f, 0.07f, 0.22f, 0.07f, tex, shade * 0.95f, alpha, 22, 0, 1, 3, 1);
        }
        DrawMobSkinBoxPartRot(-0.28f, 0.55f, -0.32f, 0.0f, -legH * 0.5f, 0.0f, legW, legH, legW, tex, shade * 0.82f, alpha, 0, 16, 4, 6, 4, legDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.28f, 0.55f, -0.32f, 0.0f, -legH * 0.5f, 0.0f, legW, legH, legW, tex, shade * 0.82f, alpha, 0, 16, 4, 6, 4, legDegOpp, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot(-0.28f, 0.55f,  0.42f, 0.0f, -legH * 0.5f, 0.0f, legW, legH, legW, tex, shade * 0.82f, alpha, 0, 16, 4, 6, 4, legDegOpp, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.28f, 0.55f,  0.42f, 0.0f, -legH * 0.5f, 0.0f, legW, legH, legW, tex, shade * 0.82f, alpha, 0, 16, 4, 6, 4, legDeg, 0.0f, 0.0f);
    } else if (m->type == MOB_CHICKEN) {
        DrawMobSkinBoxPart(0.0f, 0.58f + bodyBob, 0.05f, 0.38f, 0.50f, 0.38f, tex, shade, alpha, 0, 9, 6, 8, 6);
        DrawMobSkinBoxPart(0.0f, 1.00f + bodyBob, -0.20f, 0.25f, 0.38f, 0.20f, tex, shade, alpha, 0, 0, 4, 6, 3);
        DrawMobSkinBoxPart(0.0f, 0.96f + bodyBob, -0.38f, 0.25f, 0.12f, 0.14f, tex, shade, alpha, 14, 0, 4, 2, 2);
        DrawMobSkinBoxPart(0.0f, 0.82f + bodyBob, -0.35f, 0.12f, 0.12f, 0.12f, tex, shade, alpha, 14, 4, 2, 2, 2);
        DrawMobSkinBoxPartRot(-0.13f, 0.36f, 0.02f, 0.0f, -0.18f, 0.0f, 0.08f, 0.36f, 0.08f, tex, shade * 0.78f, alpha, 26, 0, 3, 5, 3, legDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.13f, 0.36f, 0.02f, 0.0f, -0.18f, 0.0f, 0.08f, 0.36f, 0.08f, tex, shade * 0.78f, alpha, 26, 0, 3, 5, 3, legDegOpp, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot(-0.32f, 0.72f, 0.08f, 0.0f, -0.15f, 0.0f, 0.08f, 0.30f, 0.38f, tex, shade * 0.92f, alpha, 24, 13, 1, 4, 6, 0.0f, 0.0f, wingDeg);
        DrawMobSkinBoxPartRot( 0.32f, 0.72f, 0.08f, 0.0f, -0.15f, 0.0f, 0.08f, 0.30f, 0.38f, tex, shade * 0.92f, alpha, 24, 13, 1, 4, 6, 0.0f, 0.0f, -wingDeg);
    } else if (m->type == MOB_SPIDER) {
        DrawMobSkinBoxPart(0.0f, 0.48f + bodyBob, 0.12f, 0.92f, 0.36f, 0.92f, tex, shade, alpha, 0, 12, 10, 8, 12);
        DrawMobSkinBoxPart(0.0f, 0.52f + bodyBob, -0.42f, 0.50f, 0.32f, 0.44f, tex, shade, alpha, 0, 0, 6, 6, 6);
        DrawMobSkinBoxPart(0.0f, 0.58f + bodyBob, -0.72f, 0.58f, 0.42f, 0.42f, tex, shade, alpha, 32, 4, 8, 8, 8);
        for (s = 0; s < 2; s++) {
            side = -1.0f; if (s == 1) { side = 1.0f; }
            for (t = 0; t < 4; t++) {
                zpos = -0.38f + (float)t * 0.28f;
                yrot = side * (55.0f - (float)t * 8.0f) + (float)sin((double)walk * 1.3324 + (double)t) * 18.0f * side;
                zrot = side * (34.0f + (float)(t % 2) * 10.0f);
                DrawMobSkinBoxPartRot(side * 0.36f, 0.36f, zpos, side * 0.42f, 0.0f, 0.0f, 0.84f, 0.10f, 0.10f, tex, shade * 0.78f, alpha, 18, 0, 16, 2, 2, 0.0f, yrot, zrot);
            }
        }
    } else if (m->type == MOB_SLIME) {
        scale = 1.0f + (float)fabs(sin((double)walk)) * 0.09f;
        DrawMobSkinBoxPart(0.0f, 0.55f + bodyBob, 0.0f, 1.05f * scale, 1.05f / scale, 1.05f * scale, tex, shade, 0.78f * alpha, 0, 0, 8, 8, 8);
        DrawMobSkinBoxPart(0.0f, 0.55f + bodyBob, 0.0f, 0.62f, 0.62f, 0.62f, tex, shade * 1.08f, 0.92f * alpha, 0, 16, 6, 6, 6);
        DrawMobSkinBoxPart(-0.18f, 0.70f + bodyBob, -0.34f, 0.13f, 0.13f, 0.06f, tex, shade * 0.55f, alpha, 32, 0, 2, 2, 2);
        DrawMobSkinBoxPart( 0.18f, 0.70f + bodyBob, -0.34f, 0.13f, 0.13f, 0.06f, tex, shade * 0.55f, alpha, 32, 4, 2, 2, 2);
    } else if (m->type == MOB_SQUID) {
        DrawMobSkinBoxPart(0.0f, 0.95f + bodyBob, 0.0f, 0.82f, 0.82f, 0.82f, tex, shade, alpha, 0, 0, 12, 16, 12);
        for (t = 0; t < 8; t++) {
            yrot = (float)t * 45.0f;
            DrawMobSkinBoxPartRot(0.0f, 0.48f + bodyBob, 0.0f, 0.0f, -0.36f, 0.34f, 0.10f, 0.72f, 0.10f, tex, shade * 0.82f, alpha, 48, 0, 2, 18, 2, (float)sin((double)walk + (double)t) * 16.0f, yrot, 0.0f);
        }
    } else {
        DrawMobSkinBoxPart(0.0f, 0.80f + bodyBob, 0.0f, 0.85f, 1.25f, 0.50f, tex, shade, alpha, 0, 0, 8, 8, 8);
    }

    glEnable(GL_CULL_FACE);
    glPopMatrix();
}

void RenderMobs(void)
{
    int i;
    GLuint tex;
    double dx;
    double dz;
    double dist2;
    double fullDist2;

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    fullDist2 = (double)g_mobFullModelDistanceBlocks * (double)g_mobFullModelDistanceBlocks;

    for (i = 0; i < MAX_MOBS; i++) {
        if (!mobs[i].active) { continue; }
        if (!IsMobInsideLoadedWindow(&mobs[i])) { continue; }

        dx = mobs[i].x - playerX;
        dz = mobs[i].z - playerZ;
        dist2 = dx * dx + dz * dz;

        if (dist2 > (double)GetMobRenderDistanceBlocks() * (double)GetMobRenderDistanceBlocks()) {
            continue;
        }
        if (!IsPointProbablyInView(mobs[i].x, mobs[i].z, 14.0, -0.10)) {
            continue;
        }

        tex = GetMobTexture(&mobs[i]);
        DrawMobShadow(&mobs[i]);

        /* Close mobs use the full cuboid model. Far mobs use a cheap billboard
           so distance rendering stays smooth on OpenGL 1.1/Win98-class systems. */
        if (dist2 > fullDist2) {
            DrawMobBillboard(&mobs[i], tex, 0.95f, 1.45f);
        } else {
            RenderMobModelTex(&mobs[i], tex, 1.0f);
            if (mobs[i].type == MOB_SPIDER && !IsDaylightForMobs()) {
                RenderMobModelTex(&mobs[i], texMobSpiderEyes, 0.88f);
            }
        }
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_CULL_FACE);
}

void StartHandSwing(void)
{
    handSwingLength = HAND_SWING_TIME;
    handSwingTimer = HAND_SWING_TIME;
}

void StartHandUse(void)
{
    handUseLength = HAND_USE_TIME;
    handUseTimer = HAND_USE_TIME;
}

void UpdatePlayerHandAnimation(double dt)
{
    if (handSwingTimer > 0.0) {
        handSwingTimer -= dt;
        if (handSwingTimer < 0.0) { handSwingTimer = 0.0; }
    }

    if (handUseTimer > 0.0) {
        handUseTimer -= dt;
        if (handUseTimer < 0.0) { handUseTimer = 0.0; }
    }
}

float GetHandProgress(double timer, double length)
{
    double p;

    if (length <= 0.0 || timer <= 0.0) {
        return 0.0f;
    }

    p = 1.0 - timer / length;

    if (p < 0.0) {
        p = 0.0;
    }

    if (p > 1.0) {
        p = 1.0;
    }

    return (float)p;
}

void RenderPlayerHand(void)
{
    float swingP;
    float useP;
    float swing;
    float usePush;
    float bobX;
    float bobY;
    int heldItem;

    swingP = GetHandProgress(handSwingTimer, handSwingLength);
    useP = GetHandProgress(handUseTimer, handUseLength);

    swing = (float)sin((double)swingP * PI);
    usePush = (float)sin((double)useP * PI);

    bobX = (float)(cos(handBob * 0.5) * 0.025);
    bobY = (float)(sin(handBob) * 0.045);
    heldItem = GetHeldHotbarItem();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    /* First-person transform is based on the Java ItemRenderer hand path:
       view bob + swing-root sin + use push, then the textured right arm. */
    glTranslatef(0.82f + bobX,
                 -0.66f + bobY - swing * 0.12f + usePush * 0.05f,
                 -1.10f + usePush * 0.20f);

    glRotatef(-18.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(30.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(8.0f, 0.0f, 0.0f, 1.0f);

    glRotatef(swing * 58.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(swing * -26.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(swing * 18.0f, 0.0f, 0.0f, 1.0f);

    glRotatef(usePush * -18.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(usePush * 12.0f, 0.0f, 1.0f, 0.0f);

    DrawPlayerArmPrism();
    DrawHeldItemFirstPerson(heldItem);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
}

void DrawPlayerArmPrism(void)
{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;

    /* First-person arm from player skin texture; fallback is colored cuboid. */
    if (texMobPlayer) {
        DrawMobSkinBoxPart(0.0f, 0.0f, 0.0f,
                           0.30f, 1.04f, 0.44f,
                           texMobPlayer, 1.0f, 1.0f,
                           40, 16, 4, 12, 4);
        return;
    }

    x0 = -0.14f;
    x1 =  0.16f;
    y0 = -0.54f;
    y1 =  0.18f;
    z0 = -0.14f;
    z1 =  0.14f;

    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 0);
    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 1);
    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 2);
    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 3);
    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 4);
    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 5);
}

void DrawPlayerArmFace(float x0, float x1, float y0, float y1, float z0, float z1, int face)
{
    float shade;

    if (face == 0) {
        shade = 1.00f;
    } else if (face == 1) {
        shade = 0.58f;
    } else if (face == 2 || face == 3) {
        shade = 0.82f;
    } else {
        shade = 0.70f;
    }

    glColor3f(0.82f * shade, 0.56f * shade, 0.43f * shade);

    glBegin(GL_QUADS);

    if (face == 0) {
        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z0);
    } else if (face == 1) {
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y0, z1);
    } else if (face == 2) {
        glVertex3f(x1, y0, z0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);
    } else if (face == 3) {
        glVertex3f(x0, y0, z1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x0, y1, z1);
    } else if (face == 4) {
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y1, z0);
    } else if (face == 5) {
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1);
    }

    glEnd();

    if (face == 5) {
        glColor3f(0.62f * shade, 0.38f * shade, 0.30f * shade);

        glBegin(GL_QUADS);
        glVertex3f(x1 + 0.002f, y0 + 0.12f, z0 + 0.03f);
        glVertex3f(x1 + 0.002f, y0 + 0.12f, z1 - 0.03f);
        glVertex3f(x1 + 0.002f, y0 + 0.34f, z1 - 0.03f);
        glVertex3f(x1 + 0.002f, y0 + 0.34f, z0 + 0.03f);
        glEnd();
    }
}




/* ------------------------------------------------------------ */
/* World helpers                                                */
/* ------------------------------------------------------------ */

int IsInsideWorld(int x, int y, int z)
{
    if (x < 0 || x >= WORLD_X) {
        return 0;
    }

    if (y < 0 || y >= WORLD_Y) {
        return 0;
    }

    if (z < 0 || z >= WORLD_Z) {
        return 0;
    }

    return 1;
}

int GetBlock(int x, int y, int z)
{
    if (!IsInsideWorld(x, y, z)) {
        return BLOCK_BORDER;
    }

    return world[x][y][z];
}

void SetBlock(int x, int y, int z, int block)
{
    if (!IsInsideWorld(x, y, z)) {
        return;
    }

    if (world[x][y][z] == BLOCK_BORDER) {
        return;
    }

    world[x][y][z] = block;
    RebuildColumnTopAt(x, z);
    InvalidateTerrainChunkMeshAt(x, z);
}

int IsSolidBlock(int block)
{
    if (block == BLOCK_AIR) {
        return 0;
    }

    if (block == BLOCK_WATER) {
        return 0;
    }

    if (block == BLOCK_TORCH || block == BLOCK_LIGHT) {
        return 0;
    }

    return 1;
}

/* ------------------------------------------------------------ */
/* Player movement                                              */
/* ------------------------------------------------------------ */

void CenterMouse(void)
{
    POINT p;

    p.x = g_windowWidth / 2;
    p.y = g_windowHeight / 2;

    ClientToScreen(g_hwnd, &p);
    SetCursorPos(p.x, p.y);

    ignoreNextMouseDelta = 1;
}


void UpdateMouseLook(void)
{
    POINT center;
    POINT mouse;
    int dx;
    int dy;

    if (!mouseLocked) {
        return;
    }

    if (GetForegroundWindow() != g_hwnd) {
        return;
    }

    center.x = g_windowWidth / 2;
    center.y = g_windowHeight / 2;

    ClientToScreen(g_hwnd, &center);

    GetCursorPos(&mouse);

    dx = mouse.x - center.x;
    dy = mouse.y - center.y;

    /*
        After forcing the mouse to the center, skip one frame so the
        teleport itself does not become camera movement.
    */
    if (ignoreNextMouseDelta) {
        ignoreNextMouseDelta = 0;
        SetCursorPos(center.x, center.y);
        return;
    }

    /*
        Delta-counting camera movement:
            yaw   = left/right mouse movement
            pitch = up/down mouse movement
    */
    yaw += (double)dx * MOUSE_SPEED;
    pitch -= (double)dy * MOUSE_SPEED;

    if (pitch > 89.0) {
        pitch = 89.0;
    }

    if (pitch < -89.0) {
        pitch = -89.0;
    }

    /*
        Lock physical cursor back to the screen center.
        The visual crosshair is the only cursor the player sees.
    */
    SetCursorPos(center.x, center.y);
}



void HandleGameInput(double dt)
{
    double speed;
    double yawRad;
    double forwardX;
    double forwardZ;
    double rightX;
    double rightZ;
    double moveX;
    double moveZ;
    double len;
    int inWater;

    inWater = IsPlayerInWater();
    speed = MOVE_SPEED * dt;
    if (inWater) {
        speed *= WATER_HORIZONTAL_SCALE;
    }

    yawRad = yaw * PI / 180.0;

    forwardX = -sin(yawRad);
    forwardZ = -cos(yawRad);

    rightX = cos(yawRad);
    rightZ = -sin(yawRad);

    moveX = 0.0;
    moveZ = 0.0;

    if (keyForward) {
        moveX += forwardX;
        moveZ += forwardZ;
    }

    if (keyBack) {
        moveX -= forwardX;
        moveZ -= forwardZ;
    }

    if (keyRight) {
        moveX += rightX;
        moveZ += rightZ;
    }

    if (keyLeft) {
        moveX -= rightX;
        moveZ -= rightZ;
    }

    len = sqrt(moveX * moveX + moveZ * moveZ);

    if (len > 0.0001) {
        moveX = (moveX / len) * speed;
        moveZ = (moveZ / len) * speed;
    }

    MovePlayerAxis(moveX, 0.0, 0.0);
    MovePlayerAxis(0.0, 0.0, moveZ);

    if (keyJump) {
        if (inWater) {
            /* Space swimming is handled by UpdatePlayerWaterPhysics() so the
               player rises smoothly through water without a double jump boost. */
            onGround = 0;
        } else if (onGround) {
            velocityY = JUMP_SPEED;
            onGround = 0;
        }
    }
}



/* ------------------------------------------------------------ */
/* Collision detection                                          */
/* ------------------------------------------------------------ */

int PlayerCollidesAt(double x, double y, double z)
{
    int minX;
    int maxX;
    int minY;
    int maxY;
    int minZ;
    int maxZ;

    int bx;
    int by;
    int bz;

    minX = (int)floor(x - PLAYER_RADIUS);
    maxX = (int)floor(x + PLAYER_RADIUS);

    minY = (int)floor(y);
    maxY = (int)floor(y + PLAYER_HEIGHT);

    minZ = (int)floor(z - PLAYER_RADIUS);
    maxZ = (int)floor(z + PLAYER_RADIUS);

    for (bx = minX; bx <= maxX; bx++) {
        for (by = minY; by <= maxY; by++) {
            for (bz = minZ; bz <= maxZ; bz++) {
                if (IsSolidBlock(GetBlock(bx, by, bz))) {
                    return 1;
                }
            }
        }
    }

    return 0;
}

int MovePlayerAxis(double dx, double dy, double dz)
{
    double newX;
    double newY;
    double newZ;

    newX = playerX + dx;
    newY = playerY + dy;
    newZ = playerZ + dz;

    if (!PlayerCollidesAt(newX, newY, newZ)) {
        playerX = newX;
        playerY = newY;
        playerZ = newZ;
        return 0;
    }

    return 1;
}

/* ------------------------------------------------------------ */
/* Block mining and placing                                     */
/* ------------------------------------------------------------ */

void BreakBlockRaycast(void)
{
    double yawRad;
    double pitchRad;
    double dirX;
    double dirY;
    double dirZ;
    double startX;
    double startY;
    double startZ;
    double t;
    double vx;
    double vz;
    int bx;
    int by;
    int bz;
    int block;
    int item;
    int dropCount;
    int hash;

    if (AttackMobRaycast()) {
        StartHandSwing();
        return;
    }

    yawRad = yaw * PI / 180.0;
    pitchRad = pitch * PI / 180.0;

    dirX = -sin(yawRad) * cos(pitchRad);
    dirY = sin(pitchRad);
    dirZ = -cos(yawRad) * cos(pitchRad);

    startX = playerX;
    startY = playerY + EYE_HEIGHT;
    startZ = playerZ;

    for (t = 0.0; t < RAY_DISTANCE; t += RAY_STEP) {
        bx = (int)floor(startX + dirX * t);
        by = (int)floor(startY + dirY * t);
        bz = (int)floor(startZ + dirZ * t);

        block = GetBlock(bx, by, bz);

        if (block != BLOCK_AIR &&
            block != BLOCK_BORDER &&
            block != BLOCK_WATER) {

            dropCount = 1;
            item = GetBlockDropItemAt(block, bx, by, bz, &dropCount);

            PlayBlockBreakSound(block);
            SpawnBlockBreakParticles(bx, by, bz, block);
            RemoveTileEntityAt(bx, by, bz);
            SetBlock(bx, by, bz, BLOCK_AIR);
            DamageHeldTool(1);
            UpdateRedstoneAround(bx, by, bz);

            if (item != ITEM_NONE && dropCount > 0) {
                hash = WorldHash3D(bx, by, bz, g_worldSeed + 55310);
                vx = (((hash & 255) / 255.0) - 0.5) * 1.4;
                vz = ((((hash >> 8) & 255) / 255.0) - 0.5) * 1.4;
                /* Java Block.dropBlockAsItem_do spawns inside the block with a small random offset. */
                AddDroppedItem(item, dropCount,
                               (double)bx + 0.25 + (double)((hash >> 16) & 255) / 510.0,
                               (double)by + 0.25 + (double)((hash >> 24) & 127) / 254.0,
                               (double)bz + 0.25 + (double)((hash >> 4) & 255) / 510.0,
                               vx, 1.85, vz);
            }

            RecomputeLegacyLightingLocal(bx, by, bz, 18);

            StartHandSwing();

            return;
        }
    }
}





void PlaceBlockRaycast(void)
{
    double yawRad;
    double pitchRad;
    double dirX;
    double dirY;
    double dirZ;
    double startX;
    double startY;
    double startZ;
    double t;

    int bx;
    int by;
    int bz;

    int lastAirX;
    int lastAirY;
    int lastAirZ;

    int block;
    int oldBlock;
    int placeBlock;

    InventorySlot *selectedSlot;

    selectedSlot = &hotbar[selectedHotbarSlot];

    yawRad = yaw * PI / 180.0;
    pitchRad = pitch * PI / 180.0;
    dirX = -sin(yawRad) * cos(pitchRad);
    dirY = sin(pitchRad);
    dirZ = -cos(yawRad) * cos(pitchRad);
    startX = playerX;
    startY = playerY + EYE_HEIGHT;
    startZ = playerZ;

    /* First handle interactive blocks.  This lets an empty hand open the
       crafting table when the crosshair points at one. */
    for (t = 0.0; t < RAY_DISTANCE; t += RAY_STEP) {
        bx = (int)floor(startX + dirX * t);
        by = (int)floor(startY + dirY * t);
        bz = (int)floor(startZ + dirZ * t);
        block = GetBlock(bx, by, bz);

        if (block == BLOCK_WORKBENCH) {
            OpenCraftingTable(bx, by, bz);
            return;
        }

        if (block != BLOCK_AIR && block != BLOCK_WATER) {
            break;
        }
    }

    if (selectedSlot->item == ITEM_NONE || selectedSlot->count <= 0) {
        return;
    }

    placeBlock = ItemToBlock(selectedSlot->item);

    if (placeBlock == BLOCK_AIR || placeBlock == BLOCK_BORDER) {
        return;
    }

    lastAirX = -1;
    lastAirY = -1;
    lastAirZ = -1;

    for (t = 0.0; t < RAY_DISTANCE; t += RAY_STEP) {
        bx = (int)floor(startX + dirX * t);
        by = (int)floor(startY + dirY * t);
        bz = (int)floor(startZ + dirZ * t);

        block = GetBlock(bx, by, bz);

        if (block == BLOCK_AIR || block == BLOCK_WATER) {
            lastAirX = bx;
            lastAirY = by;
            lastAirZ = bz;
        } else {
            if (lastAirX >= 0) {
                oldBlock = GetBlock(lastAirX, lastAirY, lastAirZ);

                SetBlock(lastAirX, lastAirY, lastAirZ, placeBlock);
                EnsureTileEntityForBlock(placeBlock, lastAirX, lastAirY, lastAirZ);
                UpdateRedstoneAround(lastAirX, lastAirY, lastAirZ);

                if (PlayerCollidesAt(playerX, playerY, playerZ)) {
                    SetBlock(lastAirX, lastAirY, lastAirZ, oldBlock);
                    return;
                }

                RemoveItemFromSelectedHotbar(1);
                RecomputeLegacyLightingLocal(lastAirX, lastAirY, lastAirZ, 18);
                StartHandUse();
            }

            return;
        }
    }
}


/* ------------------------------------------------------------ */
/* Camera and world rendering                                   */
/* ------------------------------------------------------------ */

void SetupCamera(void)
{
    double yawRad;
    double pitchRad;
    double dirX;
    double dirY;
    double dirZ;
    double eyeX;
    double eyeY;
    double eyeZ;
    double camX;
    double camY;
    double camZ;
    double dist;

    if (g_cameraMode == CAMERA_FIRST_PERSON) {
        ApplyDamageCameraWobble();
        glRotatef((float)-pitch, 1.0f, 0.0f, 0.0f);
        glRotatef((float)-yaw, 0.0f, 1.0f, 0.0f);

        glTranslatef(
            (float)-playerX,
            (float)-(playerY + EYE_HEIGHT),
            (float)-playerZ
        );
        return;
    }

    yawRad = yaw * PI / 180.0;
    pitchRad = pitch * PI / 180.0;
    dirX = -sin(yawRad) * cos(pitchRad);
    dirY = sin(pitchRad);
    dirZ = -cos(yawRad) * cos(pitchRad);

    eyeX = playerX;
    eyeY = playerY + EYE_HEIGHT;
    eyeZ = playerZ;
    dist = 5.0;

    if (g_cameraMode == CAMERA_THIRD_BACK) {
        camX = eyeX - dirX * dist;
        camY = eyeY - dirY * dist + 0.85;
        camZ = eyeZ - dirZ * dist;
    } else {
        camX = eyeX + dirX * dist;
        camY = eyeY + 0.60;
        camZ = eyeZ + dirZ * dist;
    }

    if (camY < playerY + 0.8) {
        camY = playerY + 0.8;
    }

    gluLookAt(camX, camY, camZ, eyeX, eyeY, eyeZ, 0.0, 1.0, 0.0);
}

void RenderWorld(void)
{
    int px;
    int pz;
    int pcx;
    int pcz;
    int cx;
    int cz;
    int dx;
    int dz;
    int distSq;
    int renderChunks;
    int renderChunkSq;
    int builds;
    int radius;
    int edge;

    px = (int)playerX;
    pz = (int)playerZ;
    pcx = px / CHUNK_SIZE;
    pcz = pz / CHUNK_SIZE;

    renderChunks = g_renderDistanceChunks;
    if (renderChunks < RENDER_DISTANCE_MIN_CHUNKS) {
        renderChunks = RENDER_DISTANCE_MIN_CHUNKS;
    }
    if (renderChunks > RENDER_DISTANCE_MAX_CHUNKS) {
        renderChunks = RENDER_DISTANCE_MAX_CHUNKS;
    }

    renderChunkSq = renderChunks * renderChunks;
    builds = 0;

    if (terrainChunkMeshOriginX != worldOriginBlockX ||
        terrainChunkMeshOriginZ != worldOriginBlockZ) {
        InvalidateAllTerrainChunkMeshes();
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glEnable(GL_CULL_FACE);

    /* Build/draw in rings around the player.  This avoids the old left-to-right
       scan that could spend a frame compiling far chunks while close chunks stayed empty. */
    for (radius = 0; radius <= renderChunks; radius++) {
        for (dx = -radius; dx <= radius; dx++) {
            for (edge = 0; edge < 2; edge++) {
                dz = edge ? radius : -radius;
                if (radius == 0 && edge == 1) { continue; }
                cx = pcx + dx;
                cz = pcz + dz;
                if (cx < 0 || cx >= WORLD_CHUNKS_X || cz < 0 || cz >= WORLD_CHUNKS_Z) { continue; }
                distSq = dx * dx + dz * dz;
                if (distSq > renderChunkSq) { continue; }
                if (!IsChunkProbablyVisible(cx, cz, distSq * CHUNK_SIZE * CHUNK_SIZE)) { continue; }
                if (terrainChunkDirty[cx][cz] || !terrainChunkLists[cx][cz]) {
                    if (builds < g_chunkMeshBuildBudget || distSq <= 1) {
                        BuildTerrainChunkMesh(cx, cz);
                        builds++;
                    } else {
                        continue;
                    }
                }
                if (terrainChunkLists[cx][cz]) { glCallList(terrainChunkLists[cx][cz]); }
            }
        }
        for (dz = -radius + 1; dz <= radius - 1; dz++) {
            for (edge = 0; edge < 2; edge++) {
                dx = edge ? radius : -radius;
                if (radius == 0) { continue; }
                cx = pcx + dx;
                cz = pcz + dz;
                if (cx < 0 || cx >= WORLD_CHUNKS_X || cz < 0 || cz >= WORLD_CHUNKS_Z) { continue; }
                distSq = dx * dx + dz * dz;
                if (distSq > renderChunkSq) { continue; }
                if (!IsChunkProbablyVisible(cx, cz, distSq * CHUNK_SIZE * CHUNK_SIZE)) { continue; }
                if (terrainChunkDirty[cx][cz] || !terrainChunkLists[cx][cz]) {
                    if (builds < g_chunkMeshBuildBudget || distSq <= 1) {
                        BuildTerrainChunkMesh(cx, cz);
                        builds++;
                    } else {
                        continue;
                    }
                }
                if (terrainChunkLists[cx][cz]) { glCallList(terrainChunkLists[cx][cz]); }
            }
        }
    }

    RenderWorldBorderOceanIllusion();
}


void DrawTorchBlock(int x, int y, int z)
{
    float u0;
    float v0;
    float u1;
    float v1;
    float cx;
    float zc;
    float y0;
    float y1;
    float w;

    if (!texTerrain) { return; }
    GetTerrainTileUV(TILE_TORCH_COL, TILE_TORCH_ROW, &u0, &v0, &u1, &v1);
    cx = (float)x + 0.5f;
    zc = (float)z + 0.5f;
    y0 = (float)y + 0.05f;
    y1 = (float)y + 0.85f;
    w = 0.18f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(u0, v1); glVertex3f(cx - w, y0, zc);
    glTexCoord2f(u1, v1); glVertex3f(cx + w, y0, zc);
    glTexCoord2f(u1, v0); glVertex3f(cx + w, y1, zc);
    glTexCoord2f(u0, v0); glVertex3f(cx - w, y1, zc);

    glTexCoord2f(u0, v1); glVertex3f(cx, y0, zc - w);
    glTexCoord2f(u1, v1); glVertex3f(cx, y0, zc + w);
    glTexCoord2f(u1, v0); glVertex3f(cx, y1, zc + w);
    glTexCoord2f(u0, v0); glVertex3f(cx, y1, zc - w);
    glEnd();
}

void DrawBlock(int x, int y, int z, int block)
{
    if (block == BLOCK_TORCH) {
        DrawTorchBlock(x, y, z);
        return;
    }

    if (ShouldDrawFace(x, y + 1, z, block)) {
        DrawFace(x, y, z, 0, block);
    }

    if (ShouldDrawFace(x, y - 1, z, block)) {
        DrawFace(x, y, z, 1, block);
    }

    if (ShouldDrawFace(x, y, z - 1, block)) {
        DrawFace(x, y, z, 2, block);
    }

    if (ShouldDrawFace(x, y, z + 1, block)) {
        DrawFace(x, y, z, 3, block);
    }

    if (ShouldDrawFace(x - 1, y, z, block)) {
        DrawFace(x, y, z, 4, block);
    }

    if (ShouldDrawFace(x + 1, y, z, block)) {
        DrawFace(x, y, z, 5, block);
    }
}

int ShouldDrawFace(int nx, int ny, int nz, int block)
{
    int neighbor;

    neighbor = GetBlock(nx, ny, nz);

    if (neighbor == BLOCK_AIR) {
        return 1;
    }

    if (block != BLOCK_WATER && neighbor == BLOCK_WATER) {
        return 1;
    }

    return 0;
}

void DrawFace(int x, int y, int z, int face, int block)
{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;

    int col;
    int row;

    float u0;
    float tv0;
    float u1;
    float tv1;

    float base;
    float faceShade;
    float ao0;
    float ao1;
    float ao2;
    float ao3;

    x0 = (float)x;
    x1 = (float)x + 1.0f;

    y0 = (float)y;
    y1 = (float)y + 1.0f;

    z0 = (float)z;
    z1 = (float)z + 1.0f;

    if (!texTerrain) {
        SetBlockColorFallback(block, face);

        glBegin(GL_QUADS);

        if (face == 0) {
            glVertex3f(x0, y1, z0);
            glVertex3f(x0, y1, z1);
            glVertex3f(x1, y1, z1);
            glVertex3f(x1, y1, z0);
        } else if (face == 1) {
            glVertex3f(x0, y0, z1);
            glVertex3f(x0, y0, z0);
            glVertex3f(x1, y0, z0);
            glVertex3f(x1, y0, z1);
        } else if (face == 2) {
            glVertex3f(x1, y0, z0);
            glVertex3f(x0, y0, z0);
            glVertex3f(x0, y1, z0);
            glVertex3f(x1, y1, z0);
        } else if (face == 3) {
            glVertex3f(x0, y0, z1);
            glVertex3f(x1, y0, z1);
            glVertex3f(x1, y1, z1);
            glVertex3f(x0, y1, z1);
        } else if (face == 4) {
            glVertex3f(x0, y0, z0);
            glVertex3f(x0, y0, z1);
            glVertex3f(x0, y1, z1);
            glVertex3f(x0, y1, z0);
        } else if (face == 5) {
            glVertex3f(x1, y0, z1);
            glVertex3f(x1, y0, z0);
            glVertex3f(x1, y1, z0);
            glVertex3f(x1, y1, z1);
        }

        glEnd();

        return;
    }

    GetBlockTile(block, face, &col, &row);
    GetTerrainTileUV(col, row, &u0, &tv0, &u1, &tv1);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);

    /*
        Classic grayscale lighting:
        - max of sky light and artificial block light
        - no yellow/orange tint for artificial light
        - simple face shade
        - vertex AO darkens corners
    */
    base = GetLegacyFaceBrightness(x, y, z, face, block);
    faceShade = GetLegacyFaceShade(face);
    ComputeFaceAO(x, y, z, face, &ao0, &ao1, &ao2, &ao3);
    DrawBiomeTintOverlayForBlock(block, x, z);

    glBegin(GL_QUADS);

    if (face == 0) {
        EmitLitVertex(u0, tv0, x0, y1, z0, base * faceShade * ao0);
        EmitLitVertex(u0, tv1, x0, y1, z1, base * faceShade * ao1);
        EmitLitVertex(u1, tv1, x1, y1, z1, base * faceShade * ao2);
        EmitLitVertex(u1, tv0, x1, y1, z0, base * faceShade * ao3);
    } else if (face == 1) {
        EmitLitVertex(u0, tv0, x0, y0, z1, base * faceShade * ao0);
        EmitLitVertex(u0, tv1, x0, y0, z0, base * faceShade * ao1);
        EmitLitVertex(u1, tv1, x1, y0, z0, base * faceShade * ao2);
        EmitLitVertex(u1, tv0, x1, y0, z1, base * faceShade * ao3);
    } else if (face == 2) {
        EmitLitVertex(u0, tv0, x1, y0, z0, base * faceShade * ao0);
        EmitLitVertex(u1, tv0, x0, y0, z0, base * faceShade * ao1);
        EmitLitVertex(u1, tv1, x0, y1, z0, base * faceShade * ao2);
        EmitLitVertex(u0, tv1, x1, y1, z0, base * faceShade * ao3);
    } else if (face == 3) {
        EmitLitVertex(u0, tv0, x0, y0, z1, base * faceShade * ao0);
        EmitLitVertex(u1, tv0, x1, y0, z1, base * faceShade * ao1);
        EmitLitVertex(u1, tv1, x1, y1, z1, base * faceShade * ao2);
        EmitLitVertex(u0, tv1, x0, y1, z1, base * faceShade * ao3);
    } else if (face == 4) {
        EmitLitVertex(u0, tv0, x0, y0, z0, base * faceShade * ao0);
        EmitLitVertex(u1, tv0, x0, y0, z1, base * faceShade * ao1);
        EmitLitVertex(u1, tv1, x0, y1, z1, base * faceShade * ao2);
        EmitLitVertex(u0, tv1, x0, y1, z0, base * faceShade * ao3);
    } else if (face == 5) {
        EmitLitVertex(u0, tv0, x1, y0, z1, base * faceShade * ao0);
        EmitLitVertex(u1, tv0, x1, y0, z0, base * faceShade * ao1);
        EmitLitVertex(u1, tv1, x1, y1, z0, base * faceShade * ao2);
        EmitLitVertex(u0, tv1, x1, y1, z1, base * faceShade * ao3);
    }

    glEnd();
    g_vertexTintR = 1.0f;
    g_vertexTintG = 1.0f;
    g_vertexTintB = 1.0f;

    /*
        Reset color so 2D UI/text does not inherit darkness.
    */
    glColor3f(1.0f, 1.0f, 1.0f);
}


void SetBlockColorFallback(int block, int face)
{
    float shade;

    shade = 1.0f;

    if (face == 1) {
        shade = 0.55f;
    } else if (face == 2 || face == 3) {
        shade = 0.75f;
    } else if (face == 4 || face == 5) {
        shade = 0.65f;
    }

    if (block == BLOCK_GRASS) {
        if (face == 0) {
            glColor3f(0.56f * shade, 0.56f * shade, 0.56f * shade);
        } else {
            glColor3f(0.40f * shade, 0.25f * shade, 0.10f * shade);
        }
    } else if (block == BLOCK_DIRT) {
        glColor3f(0.40f * shade, 0.25f * shade, 0.10f * shade);
    } else if (block == BLOCK_STONE) {
        glColor3f(0.45f * shade, 0.45f * shade, 0.45f * shade);
    } else if (block == BLOCK_WOOD) {
        glColor3f(0.38f * shade, 0.20f * shade, 0.08f * shade);
    } else if (block == BLOCK_LEAVES) {
        glColor3f(0.44f * shade, 0.44f * shade, 0.44f * shade);
    } else if (block == BLOCK_WATER) {
        glColor3f(0.32f * shade, 0.44f * shade, 0.70f * shade);
    } else if (block == BLOCK_BORDER) {
        glColor3f(0.05f * shade, 0.05f * shade, 0.05f * shade);
    } else if (block == BLOCK_LIGHT) {
        glColor3f(0.85f * shade, 0.85f * shade, 0.85f * shade);
    } else if (block == BLOCK_WOOL) {
        glColor3f(0.88f, 0.88f, 0.84f);
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
}

/* ------------------------------------------------------------ */
/* Legacy grayscale lighting + ambient occlusion                */
/* ------------------------------------------------------------ */

void ClearLightArrays(void)
{
    int x;
    int y;
    int z;

    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                skyLight[x][y][z] = 0;
                blockLight[x][y][z] = 0;
            }
        }
    }
}

int BlocksLightForLighting(int block)
{
    /*
        Opaque blocks stop light. This is a simple classic-style rule.
        Air, water, leaves, and the light block let light pass.
    */
    if (block == BLOCK_AIR) {
        return 0;
    }

    if (block == BLOCK_WATER) {
        return 0;
    }

    if (block == BLOCK_LEAVES) {
        return 0;
    }

    if (block == BLOCK_LIGHT || block == BLOCK_TORCH) {
        return 0;
    }

    return 1;
}

int IsAOBlock(int block)
{
    /*
        AO is based on nearby opaque blocks.
        Leaves and water do not create heavy black corners.
    */
    if (block == BLOCK_AIR || block == BLOCK_WATER ||
        block == BLOCK_LEAVES || block == BLOCK_LIGHT || block == BLOCK_TORCH) {
        return 0;
    }

    return 1;
}

int GetBlockEmission(int block)
{
    /*
        Grayscale-only artificial light.
        It is intentionally NOT yellow tinted.
    */
    if (block == BLOCK_LIGHT) {
        return LIGHT_SOURCE_LEVEL;
    }
    if (block == BLOCK_TORCH) {
        return 14;
    }

    return 0;
}

int PushLightNode(LightNode *queue, int *tail, int x, int y, int z)
{
    if (*tail >= MAX_LIGHT_NODES) {
        return 0;
    }

    queue[*tail].x = (short)x;
    queue[*tail].y = (short)y;
    queue[*tail].z = (short)z;

    *tail = *tail + 1;

    return 1;
}

void PropagateLightArray(unsigned char light[WORLD_X][WORLD_Y][WORLD_Z], LightNode *queue, int startTail)
{
    int head;
    int tail;
    int i;

    int x;
    int y;
    int z;
    int nx;
    int ny;
    int nz;
    int level;
    int nextLevel;

    int dx[6];
    int dy[6];
    int dz[6];

    dx[0] = 1;  dy[0] = 0;  dz[0] = 0;
    dx[1] = -1; dy[1] = 0;  dz[1] = 0;
    dx[2] = 0;  dy[2] = 1;  dz[2] = 0;
    dx[3] = 0;  dy[3] = -1; dz[3] = 0;
    dx[4] = 0;  dy[4] = 0;  dz[4] = 1;
    dx[5] = 0;  dy[5] = 0;  dz[5] = -1;

    head = 0;
    tail = startTail;

    while (head < tail) {
        x = queue[head].x;
        y = queue[head].y;
        z = queue[head].z;
        head++;

        if (!IsInsideWorld(x, y, z)) {
            continue;
        }

        level = light[x][y][z];

        if (level <= 1) {
            continue;
        }

        nextLevel = level - 1;

        for (i = 0; i < 6; i++) {
            nx = x + dx[i];
            ny = y + dy[i];
            nz = z + dz[i];

            if (!IsInsideWorld(nx, ny, nz)) {
                continue;
            }

            if (BlocksLightForLighting(GetBlock(nx, ny, nz))) {
                continue;
            }

            if ((int)light[nx][ny][nz] < nextLevel) {
                light[nx][ny][nz] = (unsigned char)nextLevel;
                PushLightNode(queue, &tail, nx, ny, nz);
            }
        }
    }
}

void ComputeLegacyLighting(void)
{
    int x;
    int y;
    int z;
    int block;
    int level;
    int bx;
    int by;
    int bz;
    int emission;
    int dx;
    int dy;
    int dz;
    int nx;
    int ny;
    int nz;
    int falloff;

    ClearLightArrays();

    /* Fast Beta-like lighting: vertical sky light plus small-radius block light.
       This avoids a full 128x128x128 flood fill every time the chunk window streams. */
    for (x = 0; x < WORLD_X; x++) {
        for (z = 0; z < WORLD_Z; z++) {
            level = LIGHT_MAX_LEVEL;
            for (y = WORLD_Y - 1; y >= 0; y--) {
                block = GetBlock(x, y, z);
                if (BlocksLightForLighting(block)) {
                    if (level > 0) {
                        level -= 2;
                    }
                    if (level < 0) {
                        level = 0;
                    }
                }
                skyLight[x][y][z] = (unsigned char)level;
            }
        }
    }

    for (bx = 0; bx < WORLD_X; bx++) {
        for (by = 0; by < WORLD_Y; by++) {
            for (bz = 0; bz < WORLD_Z; bz++) {
                emission = GetBlockEmission(GetBlock(bx, by, bz));
                if (emission <= 0) {
                    continue;
                }

                for (dx = -8; dx <= 8; dx++) {
                    for (dy = -8; dy <= 8; dy++) {
                        for (dz = -8; dz <= 8; dz++) {
                            nx = bx + dx;
                            ny = by + dy;
                            nz = bz + dz;
                            if (!IsInsideWorld(nx, ny, nz)) {
                                continue;
                            }
                            if (BlocksLightForLighting(GetBlock(nx, ny, nz))) {
                                continue;
                            }
                            falloff = abs(dx) + abs(dy) + abs(dz);
                            level = emission - falloff;
                            if (level > (int)blockLight[nx][ny][nz]) {
                                if (level < 0) { level = 0; }
                                blockLight[nx][ny][nz] = (unsigned char)level;
                            }
                        }
                    }
                }
            }
        }
    }
}

void PropagateLightArrayLocal(unsigned char light[WORLD_X][WORLD_Y][WORLD_Z], LightNode *queue, int startTail, int minX, int maxX, int minY, int maxY, int minZ, int maxZ)
{
    int head;
    int tail;
    int i;

    int x;
    int y;
    int z;
    int nx;
    int ny;
    int nz;
    int level;
    int nextLevel;

    int dx[6];
    int dy[6];
    int dz[6];

    dx[0] = 1;  dy[0] = 0;  dz[0] = 0;
    dx[1] = -1; dy[1] = 0;  dz[1] = 0;
    dx[2] = 0;  dy[2] = 1;  dz[2] = 0;
    dx[3] = 0;  dy[3] = -1; dz[3] = 0;
    dx[4] = 0;  dy[4] = 0;  dz[4] = 1;
    dx[5] = 0;  dy[5] = 0;  dz[5] = -1;

    head = 0;
    tail = startTail;

    while (head < tail) {
        x = queue[head].x;
        y = queue[head].y;
        z = queue[head].z;
        head++;

        if (!IsInsideWorld(x, y, z)) {
            continue;
        }

        level = light[x][y][z];

        if (level <= 1) {
            continue;
        }

        nextLevel = level - 1;

        for (i = 0; i < 6; i++) {
            nx = x + dx[i];
            ny = y + dy[i];
            nz = z + dz[i];

            if (nx < minX || nx > maxX ||
                ny < minY || ny > maxY ||
                nz < minZ || nz > maxZ) {
                continue;
            }

            if (!IsInsideWorld(nx, ny, nz)) {
                continue;
            }

            if (BlocksLightForLighting(GetBlock(nx, ny, nz))) {
                continue;
            }

            if ((int)light[nx][ny][nz] < nextLevel) {
                light[nx][ny][nz] = (unsigned char)nextLevel;

                if (tail < MAX_LIGHT_NODES) {
                    queue[tail].x = (short)nx;
                    queue[tail].y = (short)ny;
                    queue[tail].z = (short)nz;
                    tail++;
                }
            }
        }
    }
}

void RecomputeLegacyLightingLocal(int cx, int cy, int cz, int radius)
{
    /*
        Safe lag fix:
        do a local recompute request, but fall back to full recompute only
        if something is badly out of bounds. This function exists so linker
        errors disappear and block edit calls resolve correctly.
    */
    int minX;
    int maxX;
    int minY;
    int maxY;
    int minZ;
    int maxZ;

    int x;
    int y;
    int z;
    int block;
    int skyTail;
    int blockTail;
    int emission;

    LightNode *queue;

    minX = cx - radius;
    maxX = cx + radius;
    minY = cy - radius;
    maxY = cy + radius;
    minZ = cz - radius;
    maxZ = cz + radius;

    if (minX < 0) {
        minX = 0;
    }

    if (minY < 0) {
        minY = 0;
    }

    if (minZ < 0) {
        minZ = 0;
    }

    if (maxX >= WORLD_X) {
        maxX = WORLD_X - 1;
    }

    if (maxY >= WORLD_Y) {
        maxY = WORLD_Y - 1;
    }

    if (maxZ >= WORLD_Z) {
        maxZ = WORLD_Z - 1;
    }

    queue = (LightNode *)malloc(sizeof(LightNode) * MAX_LIGHT_NODES);

    if (!queue) {
        return;
    }

    for (x = minX; x <= maxX; x++) {
        for (y = minY; y <= maxY; y++) {
            for (z = minZ; z <= maxZ; z++) {
                skyLight[x][y][z] = 0;
                blockLight[x][y][z] = 0;
            }
        }
    }

    skyTail = 0;

    for (x = minX; x <= maxX; x++) {
        for (z = minZ; z <= maxZ; z++) {
            for (y = WORLD_Y - 1; y >= 0; y--) {
                block = GetBlock(x, y, z);

                if (BlocksLightForLighting(block)) {
                    break;
                }

                if (y >= minY && y <= maxY) {
                    skyLight[x][y][z] = LIGHT_MAX_LEVEL;
                    PushLightNode(queue, &skyTail, x, y, z);
                }
            }
        }
    }

    PropagateLightArrayLocal(skyLight, queue, skyTail, minX, maxX, minY, maxY, minZ, maxZ);

    blockTail = 0;

    for (x = minX; x <= maxX; x++) {
        for (y = minY; y <= maxY; y++) {
            for (z = minZ; z <= maxZ; z++) {
                emission = GetBlockEmission(GetBlock(x, y, z));

                if (emission > 0) {
                    blockLight[x][y][z] = (unsigned char)emission;
                    PushLightNode(queue, &blockTail, x, y, z);
                }
            }
        }
    }

    PropagateLightArrayLocal(blockLight, queue, blockTail, minX, maxX, minY, maxY, minZ, maxZ);

    free(queue);
}


int GetLegacyLightLevel(int x, int y, int z)
{
    int sky;
    int block;

    if (!IsInsideWorld(x, y, z)) {
        return LIGHT_MAX_LEVEL;
    }

    sky = (int)skyLight[x][y][z];
    block = (int)blockLight[x][y][z];

    if (block > sky) {
        return block;
    }

    return sky;
}

float ClampLightFloat(float v)
{
    if (v < 0.04f) {
        return 0.04f;
    }

    if (v > 1.0f) {
        return 1.0f;
    }

    return v;
}

float LegacyLevelToBrightness(int level)
{
    float f;

    if (level < 0) {
        level = 0;
    }

    if (level > LIGHT_MAX_LEVEL) {
        level = LIGHT_MAX_LEVEL;
    }

    /*
        Classic-looking grayscale brightness curve.
        No warm artificial-light tint.
    */
    f = (float)level / 15.0f;

    return ClampLightFloat(0.14f + f * f * 0.86f);
}

float GetLegacyFaceBrightness(int x, int y, int z, int face, int block)
{
    int sx;
    int sy;
    int sz;
    int sky;
    int blockLightLevel;
    float skyBrightness;
    float blockBrightness;
    float brightness;

    sx = x;
    sy = y;
    sz = z;

    if (face == 0) {
        sy = y + 1;
    } else if (face == 1) {
        sy = y - 1;
    } else if (face == 2) {
        sz = z - 1;
    } else if (face == 3) {
        sz = z + 1;
    } else if (face == 4) {
        sx = x - 1;
    } else if (face == 5) {
        sx = x + 1;
    }

    if (block == BLOCK_LIGHT) {
        return 1.0f;
    }

    if (!IsInsideWorld(sx, sy, sz)) {
        return ApplyGammaBoost(g_daySkyBrightness);
    }

    sky = (int)skyLight[sx][sy][sz];
    blockLightLevel = (int)blockLight[sx][sy][sz];

    skyBrightness = LegacyLevelToBrightness(sky) * g_daySkyBrightness;
    blockBrightness = LegacyLevelToBrightness(blockLightLevel);

    if (blockBrightness > skyBrightness) {
        brightness = blockBrightness;
    } else {
        brightness = skyBrightness;
    }

    return ApplyGammaBoost(brightness);
}

float GetLegacyFaceShade(int face)
{
    /*
        Old blocky face shading:
        top brightest, bottom darkest, sides medium.
    */
    if (face == 0) {
        return 1.00f;
    }

    if (face == 1) {
        return 0.48f;
    }

    if (face == 2 || face == 3) {
        return 0.76f;
    }

    return 0.68f;
}

float VertexAOFromBlocks(int s1x, int s1y, int s1z, int s2x, int s2y, int s2z, int cx, int cy, int cz)
{
    int side1;
    int side2;
    int corner;
    int total;

    side1 = IsAOBlock(GetBlock(s1x, s1y, s1z));
    side2 = IsAOBlock(GetBlock(s2x, s2y, s2z));
    corner = IsAOBlock(GetBlock(cx, cy, cz));

    if (side1 && side2) {
        return 0.55f;
    }

    total = side1 + side2 + corner;

    return 1.0f - (float)total * 0.15f;
}

void ComputeFaceAO(int x, int y, int z, int face, float *a0, float *a1, float *a2, float *a3)
{
    /*
        Four AO values match the four vertices emitted in DrawFace().
        It is a simplified version of the classic corner-darkening idea.
    */

    if (face == 0) {
        *a0 = VertexAOFromBlocks(x - 1, y + 1, z,     x, y + 1, z - 1, x - 1, y + 1, z - 1);
        *a1 = VertexAOFromBlocks(x - 1, y + 1, z,     x, y + 1, z + 1, x - 1, y + 1, z + 1);
        *a2 = VertexAOFromBlocks(x + 1, y + 1, z,     x, y + 1, z + 1, x + 1, y + 1, z + 1);
        *a3 = VertexAOFromBlocks(x + 1, y + 1, z,     x, y + 1, z - 1, x + 1, y + 1, z - 1);
    } else if (face == 1) {
        *a0 = VertexAOFromBlocks(x - 1, y - 1, z,     x, y - 1, z + 1, x - 1, y - 1, z + 1);
        *a1 = VertexAOFromBlocks(x - 1, y - 1, z,     x, y - 1, z - 1, x - 1, y - 1, z - 1);
        *a2 = VertexAOFromBlocks(x + 1, y - 1, z,     x, y - 1, z - 1, x + 1, y - 1, z - 1);
        *a3 = VertexAOFromBlocks(x + 1, y - 1, z,     x, y - 1, z + 1, x + 1, y - 1, z + 1);
    } else if (face == 2) {
        *a0 = VertexAOFromBlocks(x + 1, y, z - 1,     x, y - 1, z - 1, x + 1, y - 1, z - 1);
        *a1 = VertexAOFromBlocks(x - 1, y, z - 1,     x, y - 1, z - 1, x - 1, y - 1, z - 1);
        *a2 = VertexAOFromBlocks(x - 1, y, z - 1,     x, y + 1, z - 1, x - 1, y + 1, z - 1);
        *a3 = VertexAOFromBlocks(x + 1, y, z - 1,     x, y + 1, z - 1, x + 1, y + 1, z - 1);
    } else if (face == 3) {
        *a0 = VertexAOFromBlocks(x - 1, y, z + 1,     x, y - 1, z + 1, x - 1, y - 1, z + 1);
        *a1 = VertexAOFromBlocks(x + 1, y, z + 1,     x, y - 1, z + 1, x + 1, y - 1, z + 1);
        *a2 = VertexAOFromBlocks(x + 1, y, z + 1,     x, y + 1, z + 1, x + 1, y + 1, z + 1);
        *a3 = VertexAOFromBlocks(x - 1, y, z + 1,     x, y + 1, z + 1, x - 1, y + 1, z + 1);
    } else if (face == 4) {
        *a0 = VertexAOFromBlocks(x - 1, y, z - 1,     x - 1, y - 1, z, x - 1, y - 1, z - 1);
        *a1 = VertexAOFromBlocks(x - 1, y, z + 1,     x - 1, y - 1, z, x - 1, y - 1, z + 1);
        *a2 = VertexAOFromBlocks(x - 1, y, z + 1,     x - 1, y + 1, z, x - 1, y + 1, z + 1);
        *a3 = VertexAOFromBlocks(x - 1, y, z - 1,     x - 1, y + 1, z, x - 1, y + 1, z - 1);
    } else {
        *a0 = VertexAOFromBlocks(x + 1, y, z + 1,     x + 1, y - 1, z, x + 1, y - 1, z + 1);
        *a1 = VertexAOFromBlocks(x + 1, y, z - 1,     x + 1, y - 1, z, x + 1, y - 1, z - 1);
        *a2 = VertexAOFromBlocks(x + 1, y, z - 1,     x + 1, y + 1, z, x + 1, y + 1, z - 1);
        *a3 = VertexAOFromBlocks(x + 1, y, z + 1,     x + 1, y + 1, z, x + 1, y + 1, z + 1);
    }
}

void EmitLitVertex(float u, float v, float x, float y, float z, float brightness)
{
    brightness = ClampLightFloat(brightness);

    glColor3f(brightness * g_vertexTintR, brightness * g_vertexTintG, brightness * g_vertexTintB);
    glTexCoord2f(u, v);
    glVertex3f(x, y, z);
}


/* ------------------------------------------------------------ */
/* Crosshair                                                    */
/* ------------------------------------------------------------ */

void DrawCrosshair(void)
{
    int cx;
    int cy;

    Setup2D();

    cx = g_windowWidth / 2;
    cy = g_windowHeight / 2;

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_LINES);

    glVertex2i(cx - 8, cy);
    glVertex2i(cx + 8, cy);

    glVertex2i(cx, cy - 8);
    glVertex2i(cx, cy + 8);

    glEnd();
}

/* ------------------------------------------------------------ */
/* Survival system                                              */
/* ------------------------------------------------------------ */

void InitSurvival(void)
{
    int i;

    for (i = 0; i < HOTBAR_SLOTS; i++) {
        hotbar[i].item = ITEM_NONE;
        hotbar[i].count = 0;
    }

    for (i = 0; i < INVENTORY_SLOTS; i++) {
        inventory[i].item = ITEM_NONE;
        inventory[i].count = 0;
    }

    selectedHotbarSlot = 0;
    inventoryOpen = 0;
    playerHealth = MAX_HEALTH;

    /*
        Starter blocks for testing.
        Remove these if you want pure survival.
    */
    hotbar[0].item = ITEM_DIRT;
    hotbar[0].count = 32;

    hotbar[1].item = ITEM_COBBLESTONE;
    hotbar[1].count = 16;

    hotbar[2].item = ITEM_WOOD;
    hotbar[2].count = 8;

    /*
        Test grayscale artificial light source.
        Select slot 4 and right-click to place a light block.
    */
    hotbar[3].item = ITEM_LIGHT;
    hotbar[3].count = 16;

    hotbar[4].item = ITEM_WORKBENCH;
    hotbar[4].count = 1;
    hotbar[5].item = ITEM_PLANKS;
    hotbar[5].count = 16;
    hotbar[6].item = ITEM_STICK;
    hotbar[6].count = 8;
}

int BlockToItem(int block)
{
    /*
        EDITABLE BLOCK -> ITEM DROP MAP
        --------------------------------
        Java Block.java default is idDropped() == blockID, with special cases
        in BlockStone, BlockGrass, BlockOre, BlockGravel, BlockSnow, etc.
        This table keeps every currently generated/placeable block from falling
        back to the grass texture when dropped or rendered in the inventory.
    */
    if (block == BLOCK_GRASS) { return ITEM_GRASS; }
    if (block == BLOCK_DIRT) { return ITEM_DIRT; }
    if (block == BLOCK_STONE) { return ITEM_STONE; }
    if (block == BLOCK_COBBLESTONE) { return ITEM_COBBLESTONE; }
    if (block == BLOCK_PLANKS) { return ITEM_PLANKS; }
    if (block == BLOCK_WOOD) { return ITEM_WOOD; }
    if (block == BLOCK_LEAVES) { return ITEM_LEAVES; }
    if (block == BLOCK_WATER) { return ITEM_WATER; }
    if (block == BLOCK_WOOL) { return ITEM_WOOL; }
    if (block == BLOCK_SAND) { return ITEM_SAND; }
    if (block == BLOCK_GRAVEL) { return ITEM_GRAVEL; }
    if (block == BLOCK_SANDSTONE) { return ITEM_SANDSTONE; }
    if (block == BLOCK_CACTUS) { return ITEM_CACTUS; }
    if (block == BLOCK_SNOW) { return ITEM_SNOW_BLOCK; }
    if (block == BLOCK_ICE) { return ITEM_ICE; }
    if (block == BLOCK_LIGHT) { return ITEM_LIGHT; }
    if (block == BLOCK_TORCH) { return ITEM_TORCH; }
    if (block == BLOCK_CHEST) { return ITEM_CHEST; }
    if (block == BLOCK_WORKBENCH) { return ITEM_WORKBENCH; }
    if (block == BLOCK_FURNACE) { return ITEM_FURNACE; }
    if (block == BLOCK_COAL_ORE) { return ITEM_COAL_ORE; }
    if (block == BLOCK_IRON_ORE) { return ITEM_IRON_ORE; }
    if (block == BLOCK_GOLD_ORE) { return ITEM_GOLD_ORE; }
    if (block == BLOCK_DIAMOND_ORE) { return ITEM_DIAMOND_ORE; }
    if (block == BLOCK_REDSTONE_ORE) { return ITEM_REDSTONE_ORE; }
    if (block == BLOCK_LAPIS_ORE) { return ITEM_LAPIS_ORE; }

    return ITEM_NONE;
}

int GetBlockDropItemAt(int block, int x, int y, int z, int *countOut)
{
    int hash;

    if (countOut) {
        *countOut = 1;
    }

    hash = WorldHash3D(x, y, z, g_worldSeed + 46000);

    if (block == BLOCK_AIR || block == BLOCK_BORDER || block == BLOCK_WATER) {
        if (countOut) { *countOut = 0; }
        return ITEM_NONE;
    }

    /* Java BlockGrass.idDropped -> dirt. */
    if (block == BLOCK_GRASS) { return ITEM_DIRT; }

    /* Java BlockStone.idDropped -> cobblestone. */
    if (block == BLOCK_STONE) { return ITEM_COBBLESTONE; }

    /* Java BlockOre.idDropped special cases. */
    if (block == BLOCK_COAL_ORE) { return ITEM_COAL; }
    if (block == BLOCK_DIAMOND_ORE) { return ITEM_DIAMOND; }

    if (block == BLOCK_REDSTONE_ORE) {
        if (countOut) { *countOut = 4 + (hash & 1); }
        return ITEM_REDSTONE;
    }

    if (block == BLOCK_LAPIS_ORE) {
        if (countOut) { *countOut = 4 + (hash % 5); }
        return ITEM_LAPIS_DYE;
    }

    /* Java BlockGravel: 10 percent flint, otherwise gravel. */
    if (block == BLOCK_TORCH) { return ITEM_TORCH; }
    if (block == BLOCK_CHEST) { return ITEM_CHEST; }
    if (block == BLOCK_FURNACE) { return ITEM_FURNACE; }

    if (block == BLOCK_GRAVEL) {
        if ((hash % 10) == 0) { return ITEM_FLINT; }
        return ITEM_GRAVEL;
    }

    /* Snow layers / snow blocks use snowball icons in Java. */
    if (block == BLOCK_SNOW) {
        if (countOut) { *countOut = 1; }
        return ITEM_SNOWBALL;
    }

    /*
        Strict Beta ice normally drops nothing, but this clone keeps a visible
        ice drop so generated/mined blocks never show an incorrect fallback.
        To match strict Beta exactly, return ITEM_NONE and set count to 0 here.
    */
    if (block == BLOCK_ICE) { return ITEM_ICE; }

    return BlockToItem(block);
}

int ItemToBlock(int item)
{
    if (item == ITEM_GRASS) { return BLOCK_GRASS; }
    if (item == ITEM_DIRT) { return BLOCK_DIRT; }
    if (item == ITEM_STONE) { return BLOCK_STONE; }
    if (item == ITEM_COBBLESTONE) { return BLOCK_COBBLESTONE; }
    if (item == ITEM_PLANKS) { return BLOCK_PLANKS; }
    if (item == ITEM_WOOD) { return BLOCK_WOOD; }
    if (item == ITEM_LEAVES) { return BLOCK_LEAVES; }
    if (item == ITEM_WATER) { return BLOCK_WATER; }
    if (item == ITEM_LIGHT) { return BLOCK_LIGHT; }
    if (item == ITEM_TORCH) { return BLOCK_TORCH; }
    if (item == ITEM_CHEST) { return BLOCK_CHEST; }
    if (item == ITEM_FURNACE) { return BLOCK_FURNACE; }
    if (item == ITEM_WOOL) { return BLOCK_WOOL; }
    if (item == ITEM_SAND) { return BLOCK_SAND; }
    if (item == ITEM_GRAVEL) { return BLOCK_GRAVEL; }
    if (item == ITEM_SANDSTONE) { return BLOCK_SANDSTONE; }
    if (item == ITEM_CACTUS) { return BLOCK_CACTUS; }
    if (item == ITEM_ICE) { return BLOCK_ICE; }
    if (item == ITEM_SNOW_BLOCK) { return BLOCK_SNOW; }
    if (item == ITEM_WOOD_DOOR) { return BLOCK_WOOD_DOOR; }
    if (item == ITEM_SIGN) { return BLOCK_SIGN_POST; }
    if (item == ITEM_LADDER) { return BLOCK_LADDER; }
    if (item == ITEM_RAIL) { return BLOCK_RAIL; }
    if (item == ITEM_REDSTONE_TORCH) { return BLOCK_REDSTONE_TORCH_ON; }
    if (item == ITEM_LEVER) { return BLOCK_LEVER; }
    if (item == ITEM_BUTTON) { return BLOCK_STONE_BUTTON; }
    if (item == ITEM_REDSTONE_WIRE) { return BLOCK_REDSTONE_WIRE; }
    if (item == ITEM_WORKBENCH) { return BLOCK_WORKBENCH; }
    if (item == ITEM_COAL_ORE) { return BLOCK_COAL_ORE; }
    if (item == ITEM_IRON_ORE) { return BLOCK_IRON_ORE; }
    if (item == ITEM_GOLD_ORE) { return BLOCK_GOLD_ORE; }
    if (item == ITEM_DIAMOND_ORE) { return BLOCK_DIAMOND_ORE; }
    if (item == ITEM_REDSTONE_ORE) { return BLOCK_REDSTONE_ORE; }
    if (item == ITEM_LAPIS_ORE) { return BLOCK_LAPIS_ORE; }

    return BLOCK_AIR;
}


int AddItemToSlot(InventorySlot *slot, int item, int count)
{
    int space;
    int added;

    if (item == ITEM_NONE || count <= 0) {
        return count;
    }

    if (slot->item == ITEM_NONE || slot->count <= 0) {
        added = count;

        if (added > MAX_STACK) {
            added = MAX_STACK;
        }

        slot->item = item;
        slot->count = added;

        return count - added;
    }

    if (slot->item == item && slot->count < MAX_STACK) {
        space = MAX_STACK - slot->count;
        added = count;

        if (added > space) {
            added = space;
        }

        slot->count += added;

        return count - added;
    }

    return count;
}

int AddItemToInventory(int item, int count)
{
    int i;
    int remaining;

    remaining = count;

    for (i = 0; i < HOTBAR_SLOTS; i++) {
        remaining = AddItemToSlot(&hotbar[i], item, remaining);

        if (remaining <= 0) {
            return 1;
        }
    }

    for (i = 0; i < INVENTORY_SLOTS; i++) {
        remaining = AddItemToSlot(&inventory[i], item, remaining);

        if (remaining <= 0) {
            return 1;
        }
    }

    return 0;
}

int RemoveItemFromSelectedHotbar(int count)
{
    InventorySlot *slot;

    slot = &hotbar[selectedHotbarSlot];

    if (slot->item == ITEM_NONE || slot->count <= 0) {
        return 0;
    }

    if (slot->count < count) {
        return 0;
    }

    slot->count -= count;

    if (slot->count <= 0) {
        slot->item = ITEM_NONE;
        slot->count = 0;
    }

    return 1;
}

void SelectHotbarSlot(int slot)
{
    if (slot < 0) {
        return;
    }

    if (slot >= HOTBAR_SLOTS) {
        return;
    }

    selectedHotbarSlot = slot;
}

void TakeDamage(int amount)
{
    if (damageCooldown > 0.0) {
        return;
    }

    playerPrevHealth = playerHealth;
    playerHealth -= amount;
    playerHeartsLife = 20.0;
    damageCooldown = 0.75;
    playerHurtFlash = 0.35;
    TriggerDamageCameraWobble(amount);
    PlayPlayerHitSound();

    if (playerHealth <= 0) {
        playerHealth = 0;
        EnterDeathScreen();
    }
}



void HealPlayer(int amount)
{
    if (amount <= 0) {
        return;
    }

    playerPrevHealth = playerHealth;
    playerHealth += amount;
    playerHeartsLife = 10.0;

    if (playerHealth > MAX_HEALTH) {
        playerHealth = MAX_HEALTH;
    }
}




/* ------------------------------------------------------------ */
/* Final gameplay glue: ores, water, crafting, item atlas helpers */
/* ------------------------------------------------------------ */

int IsValidSpawnGround(int block)
{
    if (block == BLOCK_GRASS ||
        block == BLOCK_DIRT ||
        block == BLOCK_COBBLESTONE ||
        block == BLOCK_SAND ||
        block == BLOCK_SANDSTONE ||
        block == BLOCK_STONE ||
        block == BLOCK_SNOW) {
        return 1;
    }

    return 0;
}



float BetaClimateTemperature(int gx, int gz)
{
    double n;
    n = WorldFractal2D((double)gx * 0.0045, (double)gz * 0.0045,
                       g_worldSeed + 6100, 4, 0.55);
    n += WorldFractal2D((double)gx * 0.017, (double)gz * 0.017,
                        g_worldSeed + 6120, 2, 0.50) * 0.25;
    return (float)ClampDouble((n + 1.0) * 0.5, 0.0, 1.0);
}

float BetaClimateHumidity(int gx, int gz)
{
    double n;
    n = WorldFractal2D((double)gx * 0.0040, (double)gz * 0.0040,
                       g_worldSeed + 6200, 4, 0.55);
    n += WorldFractal2D((double)gx * 0.015, (double)gz * 0.015,
                        g_worldSeed + 6220, 2, 0.50) * 0.22;
    return (float)ClampDouble((n + 1.0) * 0.5, 0.0, 1.0);
}

int GetBetaBiomeFromClimate(float temp, float humidity, int gx, int gz)
{
    double ocean;
    ocean = WorldFractal2D((double)gx * 0.0032, (double)gz * 0.0032,
                           g_worldSeed + 6300, 3, 0.55);

    if (ocean < -0.48) { return BIOME_OCEAN; }
    if (temp < 0.22f && humidity < 0.55f) { return BIOME_TUNDRA; }
    if (temp < 0.38f) { return BIOME_TAIGA; }
    if (temp > 0.78f && humidity < 0.34f) { return BIOME_DESERT; }
    if (humidity > 0.78f && temp > 0.62f) { return BIOME_RAINFOREST; }
    if (humidity > 0.70f && temp > 0.45f) { return BIOME_SWAMPLAND; }
    if (humidity > 0.56f && temp > 0.50f) { return BIOME_FOREST; }
    if (humidity > 0.42f) { return BIOME_SEASONAL_FOREST; }
    if (humidity > 0.26f) { return BIOME_SHRUBLAND; }
    return BIOME_PLAINS;
}

int BiomeTreeChance(int biome)
{
    if (biome == BIOME_RAINFOREST) { return 20; }
    if (biome == BIOME_FOREST) { return 14; }
    if (biome == BIOME_SEASONAL_FOREST) { return 10; }
    if (biome == BIOME_TAIGA) { return 12; }
    if (biome == BIOME_SWAMPLAND) { return 7; }
    if (biome == BIOME_SHRUBLAND) { return 4; }
    if (biome == BIOME_DESERT || biome == BIOME_TUNDRA || biome == BIOME_OCEAN) { return 0; }
    return 2;
}

int IsItemBlock(int item)
{
    return ItemToBlock(item) != BLOCK_AIR;
}

int AreCraftSlotsOnly(int item, int count)
{
    int i;
    if (item == ITEM_NONE || count <= 0) { return 0; }
    for (i = 0; i < 9; i++) {
        if (craftGrid[i].item != item || craftGrid[i].count < count) {
            return 0;
        }
    }
    return 1;
}

void DrawFaceFast(int x, int y, int z, int face, int block)
{
    DrawFace(x, y, z, face, block);
}

int BlockyPixelSize(GLuint base)
{
    (void)base;
    return 2;
}

int GetBlockyFontRows(char c, unsigned char rows[7])
{
    int i;
    (void)c;
    for (i = 0; i < 7; i++) {
        rows[i] = 0;
    }
    return 0;
}

int IsOreExposedToSky(int x, int y, int z)
{
    int yy;
    int block;

    if (!IsInsideWorld(x, y, z)) {
        return 1;
    }

    for (yy = y + 1; yy < WORLD_Y - 1; yy++) {
        block = GetBlock(x, yy, z);
        if (block != BLOCK_AIR && block != BLOCK_LEAVES && block != BLOCK_WATER) {
            return 0;
        }
    }

    return 1;
}

int CanReplaceStoneWithOre(int x, int y, int z)
{
    if (!IsInsideWorld(x, y, z)) {
        return 0;
    }

    if (y <= 2 || y >= WORLD_Y - 3) {
        return 0;
    }

    if (world[x][y][z] != BLOCK_STONE) {
        return 0;
    }

    if (IsOreExposedToSky(x, y, z)) {
        return 0;
    }

    return 1;
}

void AddOreVein(int cx, int cy, int cz, int oreBlock, int blocks, int radius)
{
    int i;
    int dx;
    int dy;
    int dz;
    int x;
    int y;
    int z;
    int tries;

    if (radius < 1) { radius = 1; }
    tries = blocks * 6;

    for (i = 0; i < tries && blocks > 0; i++) {
        dx = (WorldHash3D(cx, cy, cz, g_worldSeed + 7000 + i) % (radius * 2 + 1)) - radius;
        dy = (WorldHash3D(cx, cy, cz, g_worldSeed + 7100 + i) % (radius * 2 + 1)) - radius;
        dz = (WorldHash3D(cx, cy, cz, g_worldSeed + 7200 + i) % (radius * 2 + 1)) - radius;
        x = cx + dx;
        y = cy + dy;
        z = cz + dz;

        if (CanReplaceStoneWithOre(x, y, z)) {
            world[x][y][z] = oreBlock;
            blocks--;
        }
    }
}

void AddOrePass(void)
{
    int chunkX;
    int chunkZ;
    int minChunkX;
    int maxChunkX;
    int minChunkZ;
    int maxChunkZ;
    int i;
    int gx;
    int gy;
    int gz;
    int lx;
    int lz;
    int pass;
    int ore;
    int attempts;
    int minY;
    int maxY;
    int veinBlocks;
    int radius;

    minChunkX = FloorDivInt(worldOriginBlockX, CHUNK_SIZE);
    maxChunkX = FloorDivInt(worldOriginBlockX + WORLD_X - 1, CHUNK_SIZE);
    minChunkZ = FloorDivInt(worldOriginBlockZ, CHUNK_SIZE);
    maxChunkZ = FloorDivInt(worldOriginBlockZ + WORLD_Z - 1, CHUNK_SIZE);

    for (chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
        for (chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
            for (pass = 0; pass < 6; pass++) {
                if (pass == 0) { ore = BLOCK_COAL_ORE; attempts = 20; minY = 5; maxY = 76; veinBlocks = 10; radius = 3; }
                else if (pass == 1) { ore = BLOCK_IRON_ORE; attempts = 20; minY = 5; maxY = 64; veinBlocks = 8; radius = 2; }
                else if (pass == 2) { ore = BLOCK_GOLD_ORE; attempts = 2; minY = 5; maxY = 32; veinBlocks = 7; radius = 2; }
                else if (pass == 3) { ore = BLOCK_REDSTONE_ORE; attempts = 8; minY = 4; maxY = 20; veinBlocks = 7; radius = 2; }
                else if (pass == 4) { ore = BLOCK_DIAMOND_ORE; attempts = 1; minY = 4; maxY = 20; veinBlocks = 6; radius = 2; }
                else { ore = BLOCK_LAPIS_ORE; attempts = 1; minY = 4; maxY = 24; veinBlocks = 6; radius = 2; }

                for (i = 0; i < attempts; i++) {
                    gx = chunkX * CHUNK_SIZE + (WorldHash3D(chunkX, i, chunkZ, g_worldSeed + 7300 + pass * 100) & 15);
                    gz = chunkZ * CHUNK_SIZE + (WorldHash3D(chunkX, i, chunkZ, g_worldSeed + 7310 + pass * 100) & 15);
                    gy = minY + (WorldHash3D(chunkX, i, chunkZ, g_worldSeed + 7320 + pass * 100) % (maxY - minY + 1));
                    lx = GlobalToLocalBlockX(gx);
                    lz = GlobalToLocalBlockZ(gz);
                    if (!IsInsideWorld(lx, gy, lz)) { continue; }
                    AddOreVein(lx, gy, lz, ore, veinBlocks, radius);
                }
            }
        }
    }
}




int CanWaterFlowIntoBlock(int block)
{
    /* Java BlockFluid can replace air-like spaces.  In this compact C port we
       keep it conservative so water does not erase real blocks or inventory
       structures: it flows into air only, while water remains pass-through. */
    if (block == BLOCK_AIR) { return 1; }
    return 0;
}

int IsWaterSpreadSupportBlock(int block)
{
    if (block == BLOCK_AIR) { return 0; }
    if (block == BLOCK_WATER) { return 1; }
    if (block == BLOCK_BORDER) { return 0; }
    return IsSolidBlock(block);
}

void UpdateJavaStyleWaterFlow(double dt)
{
    int px;
    int pz;
    int x0;
    int x1;
    int z0;
    int z1;
    int y0;
    int y1;
    int x;
    int y;
    int z;
    int dir;
    int nx;
    int nz;
    int below;
    int side;
    int support;
    int changes;
    int h;
    int order;
    static const int dx[4] = { 1, -1, 0, 0 };
    static const int dz[4] = { 0, 0, 1, -1 };

    if (g_state != STATE_GAME) { return; }

    g_waterFlowAccumulator += dt;
    if (g_waterFlowAccumulator < WATER_FLOW_INTERVAL_SECONDS) { return; }
    g_waterFlowAccumulator = 0.0;

    px = (int)floor(playerX);
    pz = (int)floor(playerZ);

    x0 = px - WATER_FLOW_RADIUS_BLOCKS;
    x1 = px + WATER_FLOW_RADIUS_BLOCKS;
    z0 = pz - WATER_FLOW_RADIUS_BLOCKS;
    z1 = pz + WATER_FLOW_RADIUS_BLOCKS;
    y0 = (int)floor(playerY) - WATER_FLOW_SCAN_DOWN;
    y1 = (int)floor(playerY) + WATER_FLOW_SCAN_UP;

    if (x0 < 1) { x0 = 1; }
    if (z0 < 1) { z0 = 1; }
    if (x1 > WORLD_X - 2) { x1 = WORLD_X - 2; }
    if (z1 > WORLD_Z - 2) { z1 = WORLD_Z - 2; }
    if (y0 < 1) { y0 = 1; }
    if (y1 > WORLD_Y - 2) { y1 = WORLD_Y - 2; }

    changes = 0;

    /* Downward pass first, matching the old Java liquid priority: a liquid
       falls before it spreads sideways.  Scanning bottom-up avoids creating an
       entire vertical column in one frame. */
    for (y = y0; y <= y1 && changes < WATER_FLOW_MAX_CHANGES; y++) {
        for (x = x0; x <= x1 && changes < WATER_FLOW_MAX_CHANGES; x++) {
            for (z = z0; z <= z1 && changes < WATER_FLOW_MAX_CHANGES; z++) {
                if (GetBlock(x, y, z) != BLOCK_WATER) { continue; }
                below = GetBlock(x, y - 1, z);
                if (CanWaterFlowIntoBlock(below)) {
                    SetBlock(x, y - 1, z, BLOCK_WATER);
                    RecomputeLegacyLightingLocal(x, y - 1, z, 6);
                    changes++;
                }
            }
        }
    }

    if (changes >= WATER_FLOW_MAX_CHANGES) { return; }

    /* Sideways pass.  Water spreads at/under sea level and in supported pools,
       but it does not turn every high placed bucket into an infinite ocean. */
    for (y = y0; y <= y1 && changes < WATER_FLOW_MAX_CHANGES; y++) {
        for (x = x0; x <= x1 && changes < WATER_FLOW_MAX_CHANGES; x++) {
            for (z = z0; z <= z1 && changes < WATER_FLOW_MAX_CHANGES; z++) {
                if (GetBlock(x, y, z) != BLOCK_WATER) { continue; }
                if (y > GEN_WATER_LEVEL + 2) { continue; }
                support = GetBlock(x, y - 1, z);
                if (!IsWaterSpreadSupportBlock(support)) { continue; }

                h = WorldHash3D(x, y, z, g_worldSeed + 75019);
                order = h & 3;
                for (dir = 0; dir < 4 && changes < WATER_FLOW_MAX_CHANGES; dir++) {
                    nx = x + dx[(dir + order) & 3];
                    nz = z + dz[(dir + order) & 3];
                    side = GetBlock(nx, y, nz);
                    if (!CanWaterFlowIntoBlock(side)) { continue; }
                    if (!IsWaterSpreadSupportBlock(GetBlock(nx, y - 1, nz))) { continue; }

                    /* Stagger the edges so water looks like it creeps outward
                       instead of all four sides appearing on the same tick. */
                    if (((h >> (dir * 3)) & 1) == 0 && changes > 0) { continue; }
                    SetBlock(nx, y, nz, BLOCK_WATER);
                    RecomputeLegacyLightingLocal(nx, y, nz, 6);
                    changes++;
                }
            }
        }
    }
}


void SpawnWaterBubbleParticles(double x, double y, double z, int count)
{
    int i;
    int p;
    p = 0;
    for (i = 0; i < MAX_PARTICLES && p < count; i++) {
        if (particles[i].life > 0.0f) { continue; }
        particles[i].x = (float)(x + ((WorldHash2D(i, p, g_worldSeed + 8100) % 100) - 50) / 120.0);
        particles[i].y = (float)(y + ((WorldHash2D(i, p, g_worldSeed + 8110) % 100) / 120.0));
        particles[i].z = (float)(z + ((WorldHash2D(i, p, g_worldSeed + 8120) % 100) - 50) / 120.0);
        particles[i].vx = ((float)((WorldHash2D(i, p, g_worldSeed + 8130) % 100) - 50)) / 700.0f;
        particles[i].vy = 0.020f + ((float)(WorldHash2D(i, p, g_worldSeed + 8140) % 100)) / 2500.0f;
        particles[i].vz = ((float)((WorldHash2D(i, p, g_worldSeed + 8150) % 100) - 50)) / 700.0f;
        particles[i].life = 0.55f;
        particles[i].maxLife = 0.55f;
        particles[i].block = BLOCK_WATER;
        p++;
    }
}


int IsPlayerInWater(void)
{
    int bx;
    int byFeet;
    int byBody;
    int bz;

    bx = (int)floor(playerX);
    byFeet = (int)floor(playerY + 0.12);
    byBody = (int)floor(playerY + 1.05);
    bz = (int)floor(playerZ);

    if (GetBlock(bx, byFeet, bz) == BLOCK_WATER) { return 1; }
    if (GetBlock(bx, byBody, bz) == BLOCK_WATER) { return 1; }
    if (GetBlock(bx, byBody + 1, bz) == BLOCK_WATER) { return 1; }
    return 0;
}



int IsPlayerHeadUnderWater(void)
{
    int bx;
    int by;
    int bz;
    bx = (int)floor(playerX);
    by = (int)floor(playerY + EYE_HEIGHT);
    bz = (int)floor(playerZ);
    return GetBlock(bx, by, bz) == BLOCK_WATER;
}

double GetPlayerWaterImmersion(void)
{
    int bx;
    int bz;
    double amount;

    bx = (int)floor(playerX);
    bz = (int)floor(playerZ);
    amount = 0.0;

    if (GetBlock(bx, (int)floor(playerY + 0.10), bz) == BLOCK_WATER) { amount += 0.34; }
    if (GetBlock(bx, (int)floor(playerY + 0.95), bz) == BLOCK_WATER) { amount += 0.33; }
    if (GetBlock(bx, (int)floor(playerY + EYE_HEIGHT), bz) == BLOCK_WATER) { amount += 0.33; }

    return ClampDouble(amount, 0.0, 1.0);
}



void UpdatePlayerWaterPhysics(double dt)
{
    double immersion;
    double damping;

    immersion = GetPlayerWaterImmersion();
    if (immersion > 0.0) {
        /* PATCH_WATER_SINK_SPACE:
           Converted from the old Entity water idea: water damps motion, but it
           should not automatically float the player to the surface.  The player
           now slowly sinks unless Space/jump is held, which then swims upward.
           The screen overlay remains separate and is drawn by DrawWaterOverlay2D(). */
        damping = 1.0 - ClampDouble(dt * (2.4 + immersion * 2.1), 0.0, 0.38);
        velocityY *= damping;

        if (keyJump) {
            velocityY += WATER_SWIM_UP_ACCEL * dt;
            if (velocityY < WATER_SWIM_UP_MIN_SPEED) {
                velocityY = WATER_SWIM_UP_MIN_SPEED;
            }
            if (velocityY > WATER_SWIM_UP_MAX_SPEED) {
                velocityY = WATER_SWIM_UP_MAX_SPEED;
            }
            onGround = 0;
        } else {
            velocityY -= WATER_PASSIVE_SINK_ACCEL * immersion * dt;
        }

        if (velocityY < -WATER_DOWN_MAX_SPEED) {
            velocityY = -WATER_DOWN_MAX_SPEED;
        }

        if (IsPlayerHeadUnderWater()) {
            g_playerAirTimer -= dt;
            if (((int)(g_playerAirTimer * 4.0)) != ((int)((g_playerAirTimer + dt) * 4.0))) {
                SpawnWaterBubbleParticles(playerX, playerY + EYE_HEIGHT - 0.20, playerZ, 3);
            }
            if (g_playerAirTimer <= 0.0) {
                g_drownDamageTimer -= dt;
                if (g_drownDamageTimer <= 0.0) {
                    TakeDamage(2);
                    SpawnWaterBubbleParticles(playerX, playerY + EYE_HEIGHT - 0.20, playerZ, 9);
                    g_drownDamageTimer = 1.0;
                }
            } else {
                g_drownDamageTimer = 1.0;
            }
        } else {
            g_playerAirTimer = 12.0;
            g_drownDamageTimer = 1.0;
        }
    } else {
        g_playerAirTimer = 12.0;
        g_drownDamageTimer = 1.0;
    }
}


int GetItemIconTile(int item, int *col, int *row)
{
    /*
        EDITABLE ITEM ICON MAP
        ----------------------
        These coordinates come from Java Item.java setIconCoord(x, y)
        and draw from assets/beta/items.tga, copied from gui/items.png.
    */
    *col = 0;
    *row = 0;

    if (item == ITEM_STICK) { *col = 5; *row = 3; return 1; }
    if (item == ITEM_BOWL) { *col = 7; *row = 4; return 1; }
    if (item == ITEM_COAL) { *col = 7; *row = 0; return 1; }
    if (item == ITEM_DIAMOND) { *col = 7; *row = 3; return 1; }
    if (item == ITEM_IRON_INGOT) { *col = 7; *row = 1; return 1; }
    if (item == ITEM_GOLD_INGOT) { *col = 7; *row = 2; return 1; }
    if (item == ITEM_WOOD_SWORD) { *col = 0; *row = 4; return 1; }
    if (item == ITEM_WOOD_SHOVEL) { *col = 0; *row = 5; return 1; }
    if (item == ITEM_WOOD_PICKAXE) { *col = 0; *row = 6; return 1; }
    if (item == ITEM_WOOD_AXE) { *col = 0; *row = 7; return 1; }
    if (item == ITEM_STONE_SWORD) { *col = 1; *row = 4; return 1; }
    if (item == ITEM_STONE_SHOVEL) { *col = 1; *row = 5; return 1; }
    if (item == ITEM_STONE_PICKAXE) { *col = 1; *row = 6; return 1; }
    if (item == ITEM_STONE_AXE) { *col = 1; *row = 7; return 1; }
    if (item == ITEM_STRING) { *col = 8; *row = 0; return 1; }
    if (item == ITEM_FEATHER) { *col = 8; *row = 1; return 1; }
    if (item == ITEM_GUNPOWDER) { *col = 8; *row = 2; return 1; }
    if (item == ITEM_REDSTONE) { *col = 8; *row = 3; return 1; }
    if (item == ITEM_ARROW) { *col = 5; *row = 2; return 1; }
    if (item == ITEM_PORK_RAW) { *col = 7; *row = 5; return 1; }
    if (item == ITEM_PORK_COOKED) { *col = 8; *row = 5; return 1; }
    if (item == ITEM_LEATHER) { *col = 7; *row = 6; return 1; }
    if (item == ITEM_SLIMEBALL) { *col = 14; *row = 1; return 1; }
    if (item == ITEM_DYE_POWDER) { *col = 14; *row = 4; return 1; }
    if (item == ITEM_LAPIS_DYE) { *col = 14; *row = 8; return 1; }
    if (item == ITEM_BONE) { *col = 12; *row = 1; return 1; }
    if (item == ITEM_EGG) { *col = 12; *row = 0; return 1; }
    if (item == ITEM_FLINT) { *col = 6; *row = 0; return 1; }
    if (item == ITEM_SNOWBALL) { *col = 14; *row = 0; return 1; }
    if (item == ITEM_IRON_SWORD) { *col = 2; *row = 4; return 1; }
    if (item == ITEM_IRON_SHOVEL) { *col = 2; *row = 5; return 1; }
    if (item == ITEM_IRON_PICKAXE) { *col = 2; *row = 6; return 1; }
    if (item == ITEM_IRON_AXE) { *col = 2; *row = 7; return 1; }
    if (item == ITEM_DIAMOND_SWORD) { *col = 3; *row = 4; return 1; }
    if (item == ITEM_DIAMOND_SHOVEL) { *col = 3; *row = 5; return 1; }
    if (item == ITEM_DIAMOND_PICKAXE) { *col = 3; *row = 6; return 1; }
    if (item == ITEM_DIAMOND_AXE) { *col = 3; *row = 7; return 1; }
    if (item == ITEM_GOLD_SWORD) { *col = 4; *row = 4; return 1; }
    if (item == ITEM_GOLD_SHOVEL) { *col = 4; *row = 5; return 1; }
    if (item == ITEM_GOLD_PICKAXE) { *col = 4; *row = 6; return 1; }
    if (item == ITEM_GOLD_AXE) { *col = 4; *row = 7; return 1; }
    if (item == ITEM_WOOD_HOE) { *col = 0; *row = 8; return 1; }
    if (item == ITEM_STONE_HOE) { *col = 1; *row = 8; return 1; }
    if (item == ITEM_IRON_HOE) { *col = 2; *row = 8; return 1; }
    if (item == ITEM_DIAMOND_HOE) { *col = 3; *row = 8; return 1; }
    if (item == ITEM_GOLD_HOE) { *col = 4; *row = 8; return 1; }
    if (item == ITEM_FLINT_STEEL) { *col = 5; *row = 0; return 1; }
    if (item == ITEM_BOW) { *col = 5; *row = 1; return 1; }
    if (item == ITEM_BUCKET) { *col = 10; *row = 4; return 1; }
    if (item == ITEM_WATER_BUCKET) { *col = 11; *row = 4; return 1; }
    if (item == ITEM_MILK_BUCKET) { *col = 13; *row = 4; return 1; }
    if (item == ITEM_WOOD_DOOR) { *col = 11; *row = 2; return 1; }
    if (item == ITEM_SIGN) { *col = 10; *row = 2; return 1; }
    if (item == ITEM_BOAT) { *col = 8; *row = 4; return 1; }
    if (item == ITEM_MINECART) { *col = 7; *row = 8; return 1; }
    if (item == ITEM_SHEARS) { *col = 13; *row = 5; return 1; }
    if (item == ITEM_FISHING_ROD) { *col = 5; *row = 5; return 1; }
    if (item == ITEM_MAP) { *col = 12; *row = 3; return 1; }

    return 0;
}


void DrawItemIcon2D(int item, int x, int y, int size)
{
    int block;
    int col;
    int row;
    int pad;

    block = ItemToBlock(item);
    pad = size >= 30 ? 1 : 0;

    if (block != BLOCK_AIR) {
        if (block == BLOCK_WOOD) { GetBlockTile(block, 2, &col, &row); }
        else { GetBlockTile(block, 0, &col, &row); }
        DrawTerrainTile2D(col, row, x + pad, y + pad, x + size - pad, y + size - pad);
        return;
    }

    if (texBetaItems && GetItemIconTile(item, &col, &row)) {
        /* Draw at exact slot-centered bounds.  The half-pixel UV pad in
           GetTileUVEx keeps neighboring icons from bleeding, which makes
           inventory/hotbar icons clearer when scaled by 2x. */
        DrawTexturedQuad2D(texBetaItems, ICONS_ATLAS_WIDTH, ICONS_ATLAS_HEIGHT,
                           col, row, x + pad, y + pad, x + size - pad, y + size - pad);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        return;
    }

    DrawRect2D(x + 3, y + 3, x + size - 3, y + size - 3, 0.75f, 0.65f, 0.35f);
}




void DrawCenteredItemStack(int x, int y, InventorySlot slot, int selected)
{
    char text[16];
    int iconSize;
    int iconX;
    int iconY;

    if (selected) {
        DrawRect2D(x, y, x + 36, y + 36, 1.0f, 1.0f, 1.0f);
    }
    if (slot.item == ITEM_NONE || slot.count <= 0) { return; }

    iconSize = 32;
    iconX = x + (36 - iconSize) / 2;
    iconY = y + (36 - iconSize) / 2;
    DrawItemIcon2D(slot.item, iconX, iconY, iconSize);

    if (slot.count > 1) {
        wsprintf(text, "%d", slot.count);
        glColor3f(1.0f, 1.0f, 1.0f);
        DrawText2D(fontBaseNormal, x + 21, y + 34, text);
    }
}


void OpenCraftingTable(int x, int y, int z)
{
    int i;
    ReturnCraftingGridToInventory();
    craftingOpen = 1;
    inventoryOpen = 1;
    g_craftingTableX = x;
    g_craftingTableY = y;
    g_craftingTableZ = z;
    for (i = 0; i < 9; i++) {
        craftGrid[i].item = ITEM_NONE;
        craftGrid[i].count = 0;
    }
    craftResult.item = ITEM_NONE;
    craftResult.count = 0;
    UnlockMouseFromGame();
}

void CloseCraftingTable(void)
{
    int i;
    for (i = 0; i < 9; i++) {
        if (craftGrid[i].item != ITEM_NONE && craftGrid[i].count > 0) {
            AddItemToInventory(craftGrid[i].item, craftGrid[i].count);
        }
        craftGrid[i].item = ITEM_NONE;
        craftGrid[i].count = 0;
    }
    craftResult.item = ITEM_NONE;
    craftResult.count = 0;
    craftingOpen = 0;
}

void UpdateCraftingResult(void)
{
    int i;
    int filledAll;
    int filled2;
    int onlyItem;
    int s0;
    int s1;
    int s2;
    int s3;
    int s4;
    int s5;
    int s6;
    int s7;
    int s8;

    craftResult.item = ITEM_NONE;
    craftResult.count = 0;

    filledAll = 0;
    filled2 = 0;
    onlyItem = ITEM_NONE;
    for (i = 0; i < 9; i++) {
        if (craftGrid[i].item != ITEM_NONE && craftGrid[i].count > 0) {
            filledAll++;
            onlyItem = craftGrid[i].item;
            if (i == 0 || i == 1 || i == 3 || i == 4) { filled2++; }
        }
    }

    s0 = craftGrid[0].item; s1 = craftGrid[1].item; s2 = craftGrid[2].item;
    s3 = craftGrid[3].item; s4 = craftGrid[4].item; s5 = craftGrid[5].item;
    s6 = craftGrid[6].item; s7 = craftGrid[7].item; s8 = craftGrid[8].item;

    if (RecipeBookFindResult()) { return; }

    /* 2x2 inventory recipes from CraftingManager/RecipesCrafting style data. */
    if (filledAll == 1 && onlyItem == ITEM_WOOD) { craftResult.item = ITEM_PLANKS; craftResult.count = 4; return; }

    if (filledAll == 2 &&
        ((s0 == ITEM_PLANKS && s3 == ITEM_PLANKS) ||
         (s1 == ITEM_PLANKS && s4 == ITEM_PLANKS))) {
        craftResult.item = ITEM_STICK; craftResult.count = 4; return;
    }

    if (filledAll == 2 &&
        ((s0 == ITEM_COAL && s3 == ITEM_STICK) ||
         (s1 == ITEM_COAL && s4 == ITEM_STICK))) {
        craftResult.item = ITEM_TORCH; craftResult.count = 4; return;
    }

    if (filledAll == 4 && filled2 == 4 &&
        s0 == ITEM_PLANKS && s1 == ITEM_PLANKS && s3 == ITEM_PLANKS && s4 == ITEM_PLANKS) {
        craftResult.item = ITEM_WORKBENCH; craftResult.count = 1; return;
    }

    if (filledAll == 4 && filled2 == 4 &&
        s0 == ITEM_SAND && s1 == ITEM_SAND && s3 == ITEM_SAND && s4 == ITEM_SAND) {
        craftResult.item = ITEM_SANDSTONE; craftResult.count = 1; return;
    }

    if (filledAll == 4 && filled2 == 4 &&
        s0 == ITEM_STRING && s1 == ITEM_STRING && s3 == ITEM_STRING && s4 == ITEM_STRING) {
        craftResult.item = ITEM_WOOL; craftResult.count = 1; return;
    }

    if (filledAll == 4 && filled2 == 4 &&
        s0 == ITEM_SNOWBALL && s1 == ITEM_SNOWBALL && s3 == ITEM_SNOWBALL && s4 == ITEM_SNOWBALL) {
        craftResult.item = ITEM_SNOW_BLOCK; craftResult.count = 1; return;
    }

    /* Full 3x3 workbench recipes. */
    if (!craftingOpen) { return; }

    if (filledAll == 8 &&
        s0 == ITEM_PLANKS && s1 == ITEM_PLANKS && s2 == ITEM_PLANKS &&
        s3 == ITEM_PLANKS && s5 == ITEM_PLANKS &&
        s6 == ITEM_PLANKS && s7 == ITEM_PLANKS && s8 == ITEM_PLANKS) {
        craftResult.item = ITEM_CHEST; craftResult.count = 1; return;
    }

    if (filledAll == 8 &&
        s0 == ITEM_COBBLESTONE && s1 == ITEM_COBBLESTONE && s2 == ITEM_COBBLESTONE &&
        s3 == ITEM_COBBLESTONE && s5 == ITEM_COBBLESTONE &&
        s6 == ITEM_COBBLESTONE && s7 == ITEM_COBBLESTONE && s8 == ITEM_COBBLESTONE) {
        craftResult.item = ITEM_FURNACE; craftResult.count = 1; return;
    }

    if (filledAll == 5 && s0 == ITEM_PLANKS && s1 == ITEM_PLANKS && s2 == ITEM_PLANKS &&
        s4 == ITEM_STICK && s7 == ITEM_STICK) { craftResult.item = ITEM_WOOD_PICKAXE; craftResult.count = 1; return; }
    if (filledAll == 5 && s0 == ITEM_COBBLESTONE && s1 == ITEM_COBBLESTONE && s2 == ITEM_COBBLESTONE &&
        s4 == ITEM_STICK && s7 == ITEM_STICK) { craftResult.item = ITEM_STONE_PICKAXE; craftResult.count = 1; return; }

    if (filledAll == 3 && s1 == ITEM_PLANKS && s4 == ITEM_PLANKS && s7 == ITEM_STICK) { craftResult.item = ITEM_WOOD_SWORD; craftResult.count = 1; return; }
    if (filledAll == 3 && s1 == ITEM_COBBLESTONE && s4 == ITEM_COBBLESTONE && s7 == ITEM_STICK) { craftResult.item = ITEM_STONE_SWORD; craftResult.count = 1; return; }

    if (filledAll == 3 && s1 == ITEM_PLANKS && s4 == ITEM_STICK && s7 == ITEM_STICK) { craftResult.item = ITEM_WOOD_SHOVEL; craftResult.count = 1; return; }
    if (filledAll == 3 && s1 == ITEM_COBBLESTONE && s4 == ITEM_STICK && s7 == ITEM_STICK) { craftResult.item = ITEM_STONE_SHOVEL; craftResult.count = 1; return; }

    if (filledAll == 5 && s0 == ITEM_PLANKS && s1 == ITEM_PLANKS && s3 == ITEM_PLANKS && s4 == ITEM_STICK && s7 == ITEM_STICK) { craftResult.item = ITEM_WOOD_AXE; craftResult.count = 1; return; }
    if (filledAll == 5 && s0 == ITEM_COBBLESTONE && s1 == ITEM_COBBLESTONE && s3 == ITEM_COBBLESTONE && s4 == ITEM_STICK && s7 == ITEM_STICK) { craftResult.item = ITEM_STONE_AXE; craftResult.count = 1; return; }

    if (filledAll == 3 && s0 == ITEM_PLANKS && s2 == ITEM_PLANKS && s4 == ITEM_PLANKS) { craftResult.item = ITEM_BOWL; craftResult.count = 4; return; }
}



void ConsumeCraftingIngredients(void)
{
    int i;
    for (i = 0; i < 9; i++) {
        if (craftGrid[i].item != ITEM_NONE && craftGrid[i].count > 0) {
            craftGrid[i].count--;
            if (craftGrid[i].count <= 0) {
                craftGrid[i].item = ITEM_NONE;
                craftGrid[i].count = 0;
            }
        }
    }
}

int GetCraftingSlotAtPoint(int mx, int my, int *slotType)
{
    int panelW;
    int panelH;
    int panelX;
    int panelY;
    int scale;
    int slot;
    int row;
    int col;
    int x;
    int y;

    scale = 2;
    panelW = 176 * scale;
    panelH = 166 * scale;
    panelX = g_windowWidth / 2 - panelW / 2;
    panelY = g_windowHeight / 2 - panelH / 2;
    if (slotType) { *slotType = -1; }

    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            x = panelX + (30 + col * 18) * scale;
            y = panelY + (17 + row * 18) * scale;
            if (mx >= x && mx < x + 18 * scale && my >= y && my < y + 18 * scale) {
                if (slotType) { *slotType = 0; }
                return row * 3 + col;
            }
        }
    }

    x = panelX + 124 * scale;
    y = panelY + 35 * scale;
    if (mx >= x && mx < x + 18 * scale && my >= y && my < y + 18 * scale) {
        if (slotType) { *slotType = 1; }
        return 0;
    }

    slot = GetInventorySlotAtPoint(mx, my, slotType);
    if (slot >= 0) {
        if (slotType && *slotType == 1) { *slotType = 3; }
        else if (slotType) { *slotType = 2; }
        return slot;
    }

    return -1;
}

void CraftingSlotClick(InventorySlot *slot)
{
    InventorySlot temp;
    int space;
    int moveCount;

    if (!slot) { return; }

    if (!g_draggingInventory) {
        if (slot->item != ITEM_NONE && slot->count > 0) {
            g_dragSlot = *slot;
            slot->item = ITEM_NONE;
            slot->count = 0;
            g_draggingInventory = 1;
        }
        return;
    }

    if (slot->item == ITEM_NONE || slot->count <= 0) {
        *slot = g_dragSlot;
        g_dragSlot.item = ITEM_NONE;
        g_dragSlot.count = 0;
        g_draggingInventory = 0;
        return;
    }

    if (slot->item == g_dragSlot.item && slot->count < MAX_STACK) {
        space = MAX_STACK - slot->count;
        moveCount = g_dragSlot.count;
        if (moveCount > space) { moveCount = space; }
        slot->count += moveCount;
        g_dragSlot.count -= moveCount;
        if (g_dragSlot.count <= 0) {
            g_dragSlot.item = ITEM_NONE;
            g_dragSlot.count = 0;
            g_draggingInventory = 0;
        }
        return;
    }

    temp = *slot;
    *slot = g_dragSlot;
    g_dragSlot = temp;
    g_draggingInventory = 1;
}

void CraftingSlotRightClick(InventorySlot *slot)
{
    int take;

    if (!slot) { return; }

    if (!g_draggingInventory) {
        if (slot->item != ITEM_NONE && slot->count > 0) {
            take = (slot->count + 1) / 2;
            g_dragSlot.item = slot->item;
            g_dragSlot.count = take;
            slot->count -= take;
            if (slot->count <= 0) {
                slot->item = ITEM_NONE;
                slot->count = 0;
            }
            g_draggingInventory = 1;
        }
        return;
    }

    if (g_dragSlot.item == ITEM_NONE || g_dragSlot.count <= 0) {
        g_draggingInventory = 0;
        return;
    }

    if (slot->item == ITEM_NONE || slot->count <= 0) {
        slot->item = g_dragSlot.item;
        slot->count = 1;
        g_dragSlot.count--;
    } else if (slot->item == g_dragSlot.item && slot->count < MAX_STACK) {
        slot->count++;
        g_dragSlot.count--;
    }

    if (g_dragSlot.count <= 0) {
        g_dragSlot.item = ITEM_NONE;
        g_dragSlot.count = 0;
        g_draggingInventory = 0;
    }
}


void CraftingMouseClick(int mx, int my)
{
    int type;
    int index;
    InventorySlot *slot;

    UpdateCraftingResult();
    index = GetCraftingSlotAtPoint(mx, my, &type);
    if (index < 0) {
        DropCarriedInventoryStackToWorld(0);
        UpdateCraftingResult();
        return;
    }

    if (type == 0) {
        CraftingSlotClick(&craftGrid[index]);
        UpdateCraftingResult();
        return;
    }

    if (type == 1) {
        if (craftResult.item == ITEM_NONE || craftResult.count <= 0) { return; }
        if (!g_draggingInventory || g_dragSlot.item == ITEM_NONE || g_dragSlot.count <= 0) {
            g_dragSlot = craftResult;
            g_draggingInventory = 1;
            ConsumeCraftingIngredients();
            UpdateCraftingResult();
        } else if (g_dragSlot.item == craftResult.item && g_dragSlot.count + craftResult.count <= MAX_STACK) {
            g_dragSlot.count += craftResult.count;
            ConsumeCraftingIngredients();
            UpdateCraftingResult();
        }
        return;
    }

    if (type == 2) {
        if (index < 0 || index >= INVENTORY_SLOTS) { return; }
        slot = &inventory[index];
        CraftingSlotClick(slot);
        UpdateCraftingResult();
        return;
    }

    if (type == 3) {
        if (index < 0 || index >= HOTBAR_SLOTS) { return; }
        slot = &hotbar[index];
        CraftingSlotClick(slot);
        UpdateCraftingResult();
        return;
    }
}

void CraftingMouseRightClick(int mx, int my)
{
    int type;
    int index;
    InventorySlot *slot;

    UpdateCraftingResult();
    index = GetCraftingSlotAtPoint(mx, my, &type);
    if (index < 0) {
        DropCarriedInventoryStackToWorld(1);
        UpdateCraftingResult();
        return;
    }

    if (type == 0) {
        CraftingSlotRightClick(&craftGrid[index]);
        UpdateCraftingResult();
        return;
    }

    if (type == 1) {
        CraftingMouseClick(mx, my);
        return;
    }

    if (type == 2) {
        if (index < 0 || index >= INVENTORY_SLOTS) { return; }
        slot = &inventory[index];
        CraftingSlotRightClick(slot);
        UpdateCraftingResult();
        return;
    }

    if (type == 3) {
        if (index < 0 || index >= HOTBAR_SLOTS) { return; }
        slot = &hotbar[index];
        CraftingSlotRightClick(slot);
        UpdateCraftingResult();
        return;
    }
}



void DrawSlotHoverFrame(int x, int y, int mx, int my)
{
    if (mx >= x && mx < x + 36 && my >= y && my < y + 36) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glColor4f(1.0f, 1.0f, 1.0f, 0.22f);
        glBegin(GL_QUADS);
        glVertex2i(x, y); glVertex2i(x + 36, y);
        glVertex2i(x + 36, y + 36); glVertex2i(x, y + 36);
        glEnd();
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void DrawCraftingScreen(void)
{
    int panelX;
    int panelY;
    int panelW;
    int panelH;
    int scale;
    int row;
    int col;
    int x;
    int y;
    int idx;
    POINT mouse;

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    UpdateCraftingResult();
    scale = 2;
    panelW = 176 * scale;
    panelH = 166 * scale;
    panelX = g_windowWidth / 2 - panelW / 2;
    panelY = g_windowHeight / 2 - panelH / 2;

    if (texBetaCrafting) {
        DrawImageCrop2D(texBetaCrafting, 256, 256, 0, 0, 176, 166,
                        panelX, panelY, panelX + panelW, panelY + panelH, 1.0f);
    } else {
        DrawRect2D(panelX, panelY, panelX + panelW, panelY + panelH, 0.18f, 0.18f, 0.18f);
    }

    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            idx = row * 3 + col;
            x = panelX + (30 + col * 18) * scale;
            y = panelY + (17 + row * 18) * scale;
            DrawSlotHoverFrame(x, y, mouse.x, mouse.y);
            DrawCenteredItemStack(x, y, craftGrid[idx], 0);
        }
    }

    x = panelX + 124 * scale;
    y = panelY + 35 * scale;
    DrawSlotHoverFrame(x, y, mouse.x, mouse.y);
    DrawCenteredItemStack(x, y, craftResult, 0);

    for (row = 0; row < 3; row++) {
        for (col = 0; col < 9; col++) {
            idx = row * 9 + col;
            x = panelX + (8 + col * 18) * scale;
            y = panelY + (84 + row * 18) * scale;
            DrawSlotHoverFrame(x, y, mouse.x, mouse.y);
            DrawCenteredItemStack(x, y, inventory[idx], 0);
        }
    }

    for (col = 0; col < HOTBAR_SLOTS; col++) {
        x = panelX + (8 + col * 18) * scale;
        y = panelY + 142 * scale;
        DrawSlotHoverFrame(x, y, mouse.x, mouse.y);
        DrawCenteredItemStack(x, y, hotbar[col], col == selectedHotbarSlot);
    }

    DrawCarriedInventoryStack();
}



#ifndef BETA_INV_GUI_W
#define BETA_INV_GUI_W 176
#define BETA_INV_GUI_H 166
#define BETA_INV_SCALE 2
#define BETA_INV_SLOT 18
#define BETA_INV_SLOT_DRAW 36
#define BETA_INV_MAIN_X 8
#define BETA_INV_MAIN_Y 84
#define BETA_INV_HOTBAR_X 8
#define BETA_INV_HOTBAR_Y 142
#endif

void ReturnCraftingGridToInventory(void)
{
    int i;
    for (i = 0; i < 9; i++) {
        if (craftGrid[i].item != ITEM_NONE && craftGrid[i].count > 0) {
            AddItemToInventory(craftGrid[i].item, craftGrid[i].count);
        }
        craftGrid[i].item = ITEM_NONE;
        craftGrid[i].count = 0;
    }
    craftResult.item = ITEM_NONE;
    craftResult.count = 0;
}

void DropCarriedInventoryStackToWorld(int oneOnly)
{
    double yawRad;
    double dx;
    double dz;
    int count;

    if (!g_draggingInventory || g_dragSlot.item == ITEM_NONE || g_dragSlot.count <= 0) {
        return;
    }

    count = g_dragSlot.count;
    if (oneOnly && count > 1) {
        count = 1;
    }

    yawRad = yaw * PI / 180.0;
    dx = -sin(yawRad);
    dz = -cos(yawRad);
    AddDroppedItem(g_dragSlot.item,
                   count,
                   playerX + dx * 0.70,
                   playerY + EYE_HEIGHT - 0.35,
                   playerZ + dz * 0.70,
                   dx * 2.5,
                   2.0,
                   dz * 2.5);
    g_dragSlot.count -= count;
    if (g_dragSlot.count <= 0) {
        g_dragSlot.item = ITEM_NONE;
        g_dragSlot.count = 0;
        g_draggingInventory = 0;
    }
    StartHandUse();
}

int IsPointInNormalInventoryPanel(int mx, int my)
{
    int panelW;
    int panelH;
    int panelX;
    int panelY;
    panelW = BETA_INV_GUI_W * BETA_INV_SCALE;
    panelH = BETA_INV_GUI_H * BETA_INV_SCALE;
    panelX = g_windowWidth / 2 - panelW / 2;
    panelY = g_windowHeight / 2 - panelH / 2;
    if (mx >= panelX && mx < panelX + panelW && my >= panelY && my < panelY + panelH) {
        return 1;
    }
    return 0;
}

int GetPlayerCraftingSlotAtPoint(int mx, int my, int *slotType)
{
    int panelW;
    int panelH;
    int panelX;
    int panelY;
    int scale;
    int slotSize;
    int x;
    int y;
    int row;
    int col;

    scale = BETA_INV_SCALE;
    slotSize = BETA_INV_SLOT * scale;
    panelW = BETA_INV_GUI_W * scale;
    panelH = BETA_INV_GUI_H * scale;
    panelX = g_windowWidth / 2 - panelW / 2;
    panelY = g_windowHeight / 2 - panelH / 2;
    if (slotType) { *slotType = -1; }

    for (row = 0; row < 2; row++) {
        for (col = 0; col < 2; col++) {
            x = panelX + (88 + col * 18) * scale;
            y = panelY + (26 + row * 18) * scale;
            if (mx >= x && mx < x + slotSize && my >= y && my < y + slotSize) {
                if (slotType) { *slotType = 0; }
                return row * 3 + col;
            }
        }
    }

    x = panelX + 144 * scale;
    y = panelY + 36 * scale;
    if (mx >= x && mx < x + slotSize && my >= y && my < y + slotSize) {
        if (slotType) { *slotType = 1; }
        return 0;
    }

    return -1;
}

void PlayerInventoryCraftingClick(int mx, int my)
{
    int type;
    int index;
    int isHotbar;
    InventorySlot *slot;

    UpdateCraftingResult();
    index = GetPlayerCraftingSlotAtPoint(mx, my, &type);
    if (index >= 0) {
        if (type == 0) {
            CraftingSlotClick(&craftGrid[index]);
            UpdateCraftingResult();
            return;
        }
        if (type == 1) {
            if (craftResult.item == ITEM_NONE || craftResult.count <= 0) { return; }
            if (!g_draggingInventory || g_dragSlot.item == ITEM_NONE || g_dragSlot.count <= 0) {
                g_dragSlot = craftResult;
                g_draggingInventory = 1;
                ConsumeCraftingIngredients();
                UpdateCraftingResult();
            } else if (g_dragSlot.item == craftResult.item && g_dragSlot.count + craftResult.count <= MAX_STACK) {
                g_dragSlot.count += craftResult.count;
                ConsumeCraftingIngredients();
                UpdateCraftingResult();
            }
            return;
        }
    }

    index = GetInventorySlotAtPoint(mx, my, &isHotbar);
    if (index >= 0) {
        if (isHotbar) {
            if (index >= HOTBAR_SLOTS) { return; }
            slot = &hotbar[index];
        } else {
            if (index >= INVENTORY_SLOTS) { return; }
            slot = &inventory[index];
        }
        CraftingSlotClick(slot);
        UpdateCraftingResult();
        return;
    }

    if (!IsPointInNormalInventoryPanel(mx, my)) {
        DropCarriedInventoryStackToWorld(0);
        UpdateCraftingResult();
    }
}

void PlayerInventoryCraftingRightClick(int mx, int my)
{
    int type;
    int index;
    int isHotbar;
    InventorySlot *slot;

    UpdateCraftingResult();
    index = GetPlayerCraftingSlotAtPoint(mx, my, &type);
    if (index >= 0) {
        if (type == 0) {
            CraftingSlotRightClick(&craftGrid[index]);
            UpdateCraftingResult();
            return;
        }
        if (type == 1) {
            PlayerInventoryCraftingClick(mx, my);
            return;
        }
    }

    index = GetInventorySlotAtPoint(mx, my, &isHotbar);
    if (index >= 0) {
        if (isHotbar) {
            if (index >= HOTBAR_SLOTS) { return; }
            slot = &hotbar[index];
        } else {
            if (index >= INVENTORY_SLOTS) { return; }
            slot = &inventory[index];
        }
        CraftingSlotRightClick(slot);
        UpdateCraftingResult();
        return;
    }

    if (!IsPointInNormalInventoryPanel(mx, my)) {
        DropCarriedInventoryStackToWorld(1);
        UpdateCraftingResult();
    }
}

/* ------------------------------------------------------------ */
/* Survival UI                                                  */
/* ------------------------------------------------------------ */

void DrawSurvivalUI(void)
{
    Setup2D();

    /* Overlay only: keep the water screen texture/tint when the player is in
       water, but keep HUD/icons drawn above it like the Java screen path. */
    DrawWaterOverlay2D();
    DrawHearts();
    DrawOxygenBubbles();
    DrawHotbar();
    if (inventoryOpen) {
        if (craftingOpen) { DrawCraftingScreen(); } else { DrawInventoryScreen(); }
    }
}



void DrawHeartTexture(GLuint texture, int x, int y, int size)
{
    /*
        Draw one standalone TGA heart in 2D screen space.

        The heart textures are 32x32 RGBA TGA files.
        Alpha transparency lets the dark screenshot background disappear.
    */
    if (!texture) {
        return;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texture);

    /*
        White keeps the original red/dark-red colors of the PNG heart.
    */
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f); glVertex2i(x,        y);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(x + size, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(x + size, y + size);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(x,        y + size);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void DrawHearts(void)
{
    int i;
    int x;
    int y;
    int sx;
    int baseX;
    int health;
    int prev;
    int lowJitter;
    int heartSize;
    int spacing;
    int flash;

    health = playerHealth;
    prev = playerPrevHealth;
    if (health < 0) { health = 0; }
    if (health > MAX_HEALTH) { health = MAX_HEALTH; }
    if (prev < 0) { prev = 0; }
    if (prev > MAX_HEALTH) { prev = MAX_HEALTH; }

    /* PATCH_F11_MOB_GUI: bigger Java-style heart HUD, matched to the scaled
       2x hotbar so hearts do not look tiny beside the GUI. */
    heartSize = 20;
    spacing = 18;
    baseX = g_windowWidth / 2 - 91 * 2;
    y = g_windowHeight - 88;
    flash = ((int)(playerHeartsLife / 3.0)) & 1;
    if (playerHeartsLife < 10.0) { flash = 0; }

    for (i = 0; i < HEART_COUNT; i++) {
        x = baseX + i * spacing;
        lowJitter = 0;
        if (health <= 4) { lowJitter = (WorldHash3D(i, g_healthHudCounter, 0, 0) & 1); }

        sx = 16 + flash * 9;
        DrawImageCrop2D(texBetaIcons, 256, 256, sx, 0, 9, 9,
                        x, y + lowJitter, x + heartSize, y + lowJitter + heartSize, 1.0f);

        if (flash) {
            if (i * 2 + 1 < prev) {
                DrawImageCrop2D(texBetaIcons, 256, 256, 70, 0, 9, 9,
                                x, y + lowJitter, x + heartSize, y + lowJitter + heartSize, 1.0f);
            } else if (i * 2 + 1 == prev) {
                DrawImageCrop2D(texBetaIcons, 256, 256, 79, 0, 9, 9,
                                x, y + lowJitter, x + heartSize, y + lowJitter + heartSize, 1.0f);
            }
        }

        if (i * 2 + 1 < health) {
            DrawImageCrop2D(texBetaIcons, 256, 256, 52, 0, 9, 9,
                            x, y + lowJitter, x + heartSize, y + lowJitter + heartSize, 1.0f);
        } else if (i * 2 + 1 == health) {
            DrawImageCrop2D(texBetaIcons, 256, 256, 61, 0, 9, 9,
                            x, y + lowJitter, x + heartSize, y + lowJitter + heartSize, 1.0f);
        }
    }
}





/* ------------------------------------------------------------ */
/* Clickable inventory slot mapping and drag/drop                */
/* ------------------------------------------------------------ */

#ifndef BETA_INV_GUI_W
#define BETA_INV_GUI_W 176
#define BETA_INV_GUI_H 166
#define BETA_INV_SCALE 2
#define BETA_INV_SLOT 18
#define BETA_INV_SLOT_DRAW 36
#define BETA_INV_MAIN_X 8
#define BETA_INV_MAIN_Y 84
#define BETA_INV_HOTBAR_X 8
#define BETA_INV_HOTBAR_Y 142
#endif

int GetInventorySlotAtPoint(int mx, int my, int *isHotbar)
{
    int panelW;
    int panelH;
    int panelX;
    int panelY;
    int slotSize;
    int left;
    int top;
    int row;
    int col;

    panelW = BETA_INV_GUI_W * BETA_INV_SCALE;
    panelH = BETA_INV_GUI_H * BETA_INV_SCALE;
    panelX = g_windowWidth / 2 - panelW / 2;
    panelY = g_windowHeight / 2 - panelH / 2;
    slotSize = BETA_INV_SLOT * BETA_INV_SCALE;

    if (isHotbar) {
        *isHotbar = 0;
    }

    /*
        Only the normal storage slots and hotbar slots are clickable.
        Armor, crafting grid, crafting output, and player preview areas are
        intentionally ignored so blocks cannot be placed into those UI tiles.
    */
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 9; col++) {
            left = panelX + (BETA_INV_MAIN_X + col * BETA_INV_SLOT) * BETA_INV_SCALE;
            top  = panelY + (BETA_INV_MAIN_Y + row * BETA_INV_SLOT) * BETA_INV_SCALE;

            if (mx >= left && mx < left + slotSize &&
                my >= top  && my < top  + slotSize) {
                if (isHotbar) {
                    *isHotbar = 0;
                }
                return row * 9 + col;
            }
        }
    }

    for (col = 0; col < HOTBAR_SLOTS; col++) {
        left = panelX + (BETA_INV_HOTBAR_X + col * BETA_INV_SLOT) * BETA_INV_SCALE;
        top  = panelY + BETA_INV_HOTBAR_Y * BETA_INV_SCALE;

        if (mx >= left && mx < left + slotSize &&
            my >= top  && my < top  + slotSize) {
            if (isHotbar) {
                *isHotbar = 1;
            }
            return col;
        }
    }

    return -1;
}

void InventoryMouseClick(int mx, int my)
{
    int isHotbar;
    int index;
    InventorySlot *slot;
    InventorySlot temp;
    int space;
    int moveCount;

    index = GetInventorySlotAtPoint(mx, my, &isHotbar);
    if (index < 0) {
        return;
    }

    if (isHotbar) {
        if (index >= HOTBAR_SLOTS) {
            return;
        }
        slot = &hotbar[index];
    } else {
        if (index >= INVENTORY_SLOTS) {
            return;
        }
        slot = &inventory[index];
    }

    if (!g_draggingInventory) {
        if (slot->item != ITEM_NONE && slot->count > 0) {
            g_dragSlot = *slot;
            slot->item = ITEM_NONE;
            slot->count = 0;
            g_draggingInventory = 1;
        }
        return;
    }

    if (slot->item == ITEM_NONE || slot->count <= 0) {
        *slot = g_dragSlot;
        g_dragSlot.item = ITEM_NONE;
        g_dragSlot.count = 0;
        g_draggingInventory = 0;
        return;
    }

    if (slot->item == g_dragSlot.item && slot->count < MAX_STACK) {
        space = MAX_STACK - slot->count;
        moveCount = g_dragSlot.count;
        if (moveCount > space) {
            moveCount = space;
        }
        slot->count += moveCount;
        g_dragSlot.count -= moveCount;
        if (g_dragSlot.count <= 0) {
            g_dragSlot.item = ITEM_NONE;
            g_dragSlot.count = 0;
            g_draggingInventory = 0;
        }
        return;
    }

    temp = *slot;
    *slot = g_dragSlot;
    g_dragSlot = temp;
    g_draggingInventory = 1;
}

void InventoryMouseRightClick(int mx, int my)
{
    int isHotbar;
    int index;
    InventorySlot *slot;

    index = GetInventorySlotAtPoint(mx, my, &isHotbar);
    if (index < 0) {
        return;
    }

    if (isHotbar) {
        if (index >= HOTBAR_SLOTS) { return; }
        slot = &hotbar[index];
    } else {
        if (index >= INVENTORY_SLOTS) { return; }
        slot = &inventory[index];
    }

    CraftingSlotRightClick(slot);
}


void DrawCarriedInventoryStack(void)
{
    POINT mp;

    if (!g_draggingInventory || g_dragSlot.item == ITEM_NONE || g_dragSlot.count <= 0) {
        return;
    }

    GetCursorPos(&mp);
    ScreenToClient(g_hwnd, &mp);
    DrawInventorySlot(mp.x - 18, mp.y - 18, g_dragSlot, 1);
}


void HandleInventoryClick(int mx, int my)
{
    PlayerInventoryCraftingClick(mx, my);
}

void HandleInventoryRightClick(int mx, int my)
{
    PlayerInventoryCraftingRightClick(mx, my);
}


void DrawHotbar(void)
{
    int i;
    int slotSize;
    int startX;
    int y;

    slotSize = 44;
    startX = g_windowWidth / 2 - (HOTBAR_SLOTS * slotSize) / 2;
    y = g_windowHeight - 55;

    if (texBetaGui) {
        DrawImageCrop2D(texBetaGui, 256, 256, 0, 0, 182, 22,
                        startX - 2, y - 2, startX + HOTBAR_SLOTS * slotSize + 2, y + slotSize + 2, 1.0f);
        DrawImageCrop2D(texBetaGui, 256, 256, 0, 22, 24, 24,
                        startX + selectedHotbarSlot * slotSize - 4, y - 4,
                        startX + selectedHotbarSlot * slotSize + 48, y + 48, 1.0f);
    }

    for (i = 0; i < HOTBAR_SLOTS; i++) {
        DrawInventorySlot(
            startX + i * slotSize,
            y,
            hotbar[i],
            i == selectedHotbarSlot
        );
    }
}

void DrawInventoryScreen(void)
{
    int panelX;
    int panelY;
    int panelW;
    int panelH;
    int row;
    int col;
    int index;
    int slotSize;
    int slotX;
    int slotY;
    POINT mouse;

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    panelW = BETA_INV_GUI_W * BETA_INV_SCALE;
    panelH = BETA_INV_GUI_H * BETA_INV_SCALE;
    panelX = g_windowWidth / 2 - panelW / 2;
    panelY = g_windowHeight / 2 - panelH / 2;
    slotSize = BETA_INV_SLOT * BETA_INV_SCALE;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (texBetaInventory) {
        DrawImageCrop2D(texBetaInventory, 256, 256, 0, 0, BETA_INV_GUI_W, BETA_INV_GUI_H,
                        panelX, panelY, panelX + panelW, panelY + panelH, 1.0f);
    } else {
        DrawRect2D(panelX, panelY, panelX + panelW, panelY + panelH, 0.12f, 0.12f, 0.12f);
        DrawRect2D(panelX + 4, panelY + 4, panelX + panelW - 4, panelY + panelH - 4, 0.25f, 0.25f, 0.25f);
    }

    UpdateCraftingResult();

    /* Draw the built-in 2x2 inventory crafting grid and its result slot. */
    slotX = panelX + 88 * BETA_INV_SCALE;
    slotY = panelY + 26 * BETA_INV_SCALE;
    DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
    DrawCenteredItemStack(slotX, slotY, craftGrid[0], 0);
    slotX = panelX + 106 * BETA_INV_SCALE;
    slotY = panelY + 26 * BETA_INV_SCALE;
    DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
    DrawCenteredItemStack(slotX, slotY, craftGrid[1], 0);
    slotX = panelX + 88 * BETA_INV_SCALE;
    slotY = panelY + 44 * BETA_INV_SCALE;
    DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
    DrawCenteredItemStack(slotX, slotY, craftGrid[3], 0);
    slotX = panelX + 106 * BETA_INV_SCALE;
    slotY = panelY + 44 * BETA_INV_SCALE;
    DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
    DrawCenteredItemStack(slotX, slotY, craftGrid[4], 0);
    slotX = panelX + 144 * BETA_INV_SCALE;
    slotY = panelY + 36 * BETA_INV_SCALE;
    DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
    DrawCenteredItemStack(slotX, slotY, craftResult, 0);

    /*
        Draw items only in the valid dark slot squares from the inventory GUI:
        3x9 storage rows and 1x9 hotbar. Armor and crafting tiles stay empty.
    */
    index = 0;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 9; col++) {
            slotX = panelX + (BETA_INV_MAIN_X + col * BETA_INV_SLOT) * BETA_INV_SCALE;
            slotY = panelY + (BETA_INV_MAIN_Y + row * BETA_INV_SLOT) * BETA_INV_SCALE;
            DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
            DrawInventorySlot(slotX, slotY, inventory[index], 0);
            index++;
        }
    }

    for (col = 0; col < HOTBAR_SLOTS; col++) {
        slotX = panelX + (BETA_INV_HOTBAR_X + col * BETA_INV_SLOT) * BETA_INV_SCALE;
        slotY = panelY + BETA_INV_HOTBAR_Y * BETA_INV_SCALE;
        DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
        DrawInventorySlot(slotX, slotY, hotbar[col], col == selectedHotbarSlot);
    }

    DrawCarriedInventoryStack();
}



void DrawInventorySlot(int x, int y, InventorySlot slot, int selected)
{
    char countText[16];
    int iconSize;
    int iconX;
    int iconY;

    if (texBetaGui) {
        if (selected) {
            DrawImageCrop2D(texBetaGui, 256, 256, 0, 22, 24, 24,
                            x - 4, y - 4, x + 46, y + 46, 1.0f);
        }
    } else {
        if (selected) {
            DrawRect2D(x, y, x + 42, y + 42, 1.0f, 1.0f, 1.0f);
        } else {
            DrawRect2D(x, y, x + 42, y + 42, 0.05f, 0.05f, 0.05f);
        }

        DrawRect2D(x + 3, y + 3, x + 39, y + 39, 0.35f, 0.35f, 0.35f);
    }

    if (slot.item != ITEM_NONE && slot.count > 0) {
        /* Beta GUI slots are 18x18 pixels.  At the normal 2x scale that is
           36x36, so a 32x32 item texture belongs at a two-pixel inset.  The
           same function also works in the hotbar because the selected frame
           extends around the slot, not through the item itself. */
        iconSize = 32;
        iconX = x + 2;
        iconY = y + 2;
        DrawItemIcon2D(slot.item, iconX, iconY, iconSize);

        if (slot.count > 1) {
            wsprintf(countText, "%d", slot.count);

            glDisable(GL_TEXTURE_2D);
            glColor3f(1.0f, 1.0f, 1.0f);
            DrawText2D(fontBaseNormal, x + 22, y + 34, countText);
        }
        if (GetItemMaxDurability(slot.item) > 0 && slot.damage > 0) {
            int maxDur;
            int barW;
            maxDur = GetItemMaxDurability(slot.item);
            barW = 28 - (slot.damage * 28) / maxDur;
            if (barW < 0) { barW = 0; }
            DrawRect2D(x + 4, y + 34, x + 32, y + 36, 0.08f, 0.08f, 0.08f);
            DrawRect2D(x + 4, y + 34, x + 4 + barW, y + 36, 0.20f, 0.95f, 0.20f);
        }
    }
}




