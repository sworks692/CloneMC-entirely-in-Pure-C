/*
    CloneMC V57 Priority 2 Block Material Render Correctness
    ------------------------------------------------------------
    This root file preserves the original one-translation-unit behavior
    while keeping the V51 editable section split through V55 terrain work,
    and adding V57 block registry, material, collision, support, placement and
    RenderBlocks-style behavior correctness.
    Compile this file with Open Watcom; it includes ENTITY, ITEM, WORLD,
    RENDER, LIGHTING, SAVE, and UI sections in the original safe order.
*/

/* CORE / TYPES / GLOBALS / PROTOTYPES */
#include "src/00_core/clonemc_core_defs.c"

/* SAVE / TILE ENTITIES / REGION / NBT-LIKE PERSISTENCE */
#include "src/10_save/clonemc_save_tile_region.c"

/* ENTITY / MOB AI CORE / PATHING / LIVING COLLISION HELPERS */
#include "src/20_entity/clonemc_entity_mob_ai_core.c"

/* ITEM / RECIPES / CONTAINERS / REDSTONE / FEATURE SYSTEMS */
#include "src/30_item/clonemc_items_containers_redstone.c"

/* ENTITY / DROPPED ITEMS / HELD ITEMS / PLAYER PHYSICS */
#include "src/20_entity/clonemc_entity_items_player_physics.c"

/* LIGHTING / V48 QUEUE FRONT-END */
#include "src/50_lighting/clonemc_lighting_v48_front.c"

/* UI / PLATFORM / OPENGL STARTUP / TEXTURES / MENUS / LEGACY SAVE UI */
#include "src/60_ui/clonemc_platform_ui_resources.c"

/* WORLD / GENERATION / STREAMING / PARTICLES / DROPPED ITEM WORLD LOOP */
#include "src/40_world/clonemc_world_generation_streaming.c"

/* ENTITY / MOB MODELS / PLAYER CONTROL / CAMERA / WORLD ENTRY RENDER CALL */
#include "src/20_entity/clonemc_entity_mob_render_player.c"

/* RENDER / BLOCK RENDER TYPES / RENDERBLOCKS-STYLE GEOMETRY */
#include "src/70_render/clonemc_render_block_types.c"

/* LIGHTING / EMISSION / PROPAGATION / FACE BRIGHTNESS */
#include "src/50_lighting/clonemc_lighting_brightness_render.c"

/* ITEM / COMBAT / ITEM USE / SPECIAL ENTITY UPDATE */
#include "src/30_item/clonemc_item_combat_special_entities.c"

/* RENDER / CHUNK QUEUES / SPECIAL ENTITIES / FIRST-PERSON ITEMS */
#include "src/70_render/clonemc_render_chunks_special_entities.c"

/* ITEM + UI / SURVIVAL INVENTORY / CRAFTING / HUD TAIL */
#include "src/30_item/clonemc_inventory_crafting_ui_tail.c"


/* WORLD / JAVA-STYLE SCHEDULED BLOCK TICK QUEUE */
#include "src/40_world/clonemc_scheduled_ticks_v52.c"
