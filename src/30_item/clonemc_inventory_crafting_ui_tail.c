/* ============================================================
   CloneMC V51 section: ITEM + UI / SURVIVAL INVENTORY / CRAFTING / HUD TAIL
   ============================================================ */

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

    /* V6 test equipment from ItemTool/ItemBow/ItemBucket/ItemBoat conversions. */
    hotbar[7].item = ITEM_IRON_SWORD;
    hotbar[7].count = 1;
    hotbar[8].item = ITEM_BOW;
    hotbar[8].count = 1;
    inventory[0].item = ITEM_ARROW;
    inventory[0].count = 32;
    inventory[1].item = ITEM_BUCKET;
    inventory[1].count = 1;
    inventory[2].item = ITEM_FLINT_STEEL;
    inventory[2].count = 1;
    inventory[3].item = ITEM_SHEARS;
    inventory[3].count = 1;
    inventory[4].item = ITEM_BOAT;
    inventory[4].count = 1;
    inventory[5].item = ITEM_FISHING_ROD;
    inventory[5].count = 1;
    inventory[6].item = ITEM_PAINTING;
    inventory[6].count = 2;
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
    if (block == BLOCK_GLASS) { return ITEM_GLASS; }
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
    if (block == BLOCK_FENCE) { return ITEM_FENCE; }
    if (block == BLOCK_JUKEBOX) { return ITEM_JUKEBOX; }
    if (block == BLOCK_NOTE) { return ITEM_NOTE_BLOCK; }
    if (block == BLOCK_DISPENSER) { return ITEM_DISPENSER; }
    if (block == BLOCK_TNT) { return ITEM_TNT; }
    if (block == BLOCK_BOOKSHELF) { return ITEM_BOOKSHELF; }
    if (block == BLOCK_BRICK) { return ITEM_BRICK_BLOCK; }
    if (block == BLOCK_CLAY) { return ITEM_CLAY_BLOCK; }
    if (block == BLOCK_GLOWSTONE) { return ITEM_GLOWSTONE_BLOCK; }
    if (block == BLOCK_TRAPDOOR) { return ITEM_TRAPDOOR; }
    if (block == BLOCK_PISTON) { return ITEM_PISTON; }
    if (block == BLOCK_PISTON_STICKY) { return ITEM_STICKY_PISTON; }
    if (block == BLOCK_REPEATER_OFF || block == BLOCK_REPEATER_ON) { return ITEM_REPEATER; }
    if (block == BLOCK_STONE_PRESSURE_PLATE) { return ITEM_PRESSURE_PLATE_STONE; }
    if (block == BLOCK_WOOD_PRESSURE_PLATE) { return ITEM_PRESSURE_PLATE_WOOD; }
    if (block == BLOCK_FLOWER_YELLOW || block == BLOCK_FLOWER_RED ||
        block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED ||
        block == BLOCK_GOLD_BLOCK || block == BLOCK_IRON_BLOCK ||
        block == BLOCK_DIAMOND_BLOCK || block == BLOCK_DOUBLE_STEP ||
        block == BLOCK_STEP || block == BLOCK_MOSSY_COBBLESTONE ||
        block == BLOCK_OBSIDIAN || block == BLOCK_SNOW_BLOCK ||
        block == BLOCK_PUMPKIN || block == BLOCK_NETHERRACK ||
        block == BLOCK_SOULSAND || block == BLOCK_JACK_O_LANTERN ||
        block == BLOCK_DETECTOR_RAIL || block == BLOCK_LOCKED_CHEST) { return block; }
    if (block == BLOCK_REED) { return ITEM_REED; }
    if (block == BLOCK_CROPS) { return ITEM_SEEDS; }
    if (block == BLOCK_FARMLAND) { return ITEM_DIRT; }
    if (block == BLOCK_FURNACE_LIT) { return ITEM_FURNACE; }
    if (block == BLOCK_SIGN_WALL) { return ITEM_SIGN; }
    if (block == BLOCK_REDSTONE_TORCH_OFF) { return ITEM_REDSTONE_TORCH; }
    if (block == BLOCK_CAKE) { return ITEM_CAKE; }

    return ITEM_NONE;
}

int GetBlockDropItemAt(int block, int x, int y, int z, int *countOut)
{
    int hash;

    if (countOut) {
        *countOut = 1;
    }

    hash = WorldHash3D(x, y, z, g_worldSeed + 46000);

    /* V50: metadata-sensitive Java block drops for special render/behavior blocks. */
    if (block == BLOCK_WOOD_DOOR) { return (IsInsideWorld(x, y, z) && (g_blockMeta[x][y][z] & 8)) ? ITEM_NONE : ITEM_WOOD_DOOR; }
    if (block == BLOCK_IRON_DOOR) { return (IsInsideWorld(x, y, z) && (g_blockMeta[x][y][z] & 8)) ? ITEM_NONE : ITEM_IRON_DOOR; }
    if (block == BLOCK_TRAPDOOR) { return ITEM_TRAPDOOR; }
    if (block == BLOCK_BED) { return (IsInsideWorld(x, y, z) && (g_blockMeta[x][y][z] & 8)) ? ITEM_NONE : ITEM_BED; }
    if (block == BLOCK_CAKE) { if (countOut) { *countOut = 0; } return ITEM_NONE; }
    if (block == BLOCK_GLASS || block == BLOCK_ICE || block == BLOCK_LEAVES || block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH) { if (countOut) { *countOut = 0; } return ITEM_NONE; }
    if (block == BLOCK_REDSTONE_WIRE) { return ITEM_REDSTONE; }
    if (block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) { return ITEM_REDSTONE_TORCH; }
    if (block == BLOCK_REPEATER_ON || block == BLOCK_REPEATER_OFF) { return ITEM_REPEATER; }
    if (block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL || block == BLOCK_WALL_SIGN) { return ITEM_SIGN; }
    if (block == BLOCK_STONE_BUTTON) { return ITEM_BUTTON; }
    if (block == BLOCK_STONE_PRESSURE_PLATE) { return ITEM_PRESSURE_PLATE_STONE; }
    if (block == BLOCK_WOOD_PRESSURE_PLATE) { return ITEM_PRESSURE_PLATE_WOOD; }

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
    if (block == BLOCK_FLOWER_YELLOW || block == BLOCK_FLOWER_RED ||
        block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED) { return block; }
    if (block == BLOCK_CROPS) { if (countOut) { *countOut = 1 + (hash % 3); } return ITEM_SEEDS; }
    if (block == BLOCK_REED) { return ITEM_REED; }
    if (block == BLOCK_CLAY) { if (countOut) { *countOut = 4; } return ITEM_CLAY_BALL; }

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
    if (item == ITEM_GLASS) { return BLOCK_GLASS; }
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
    if (item == ITEM_FENCE) { return BLOCK_FENCE; }
    if (item == ITEM_JUKEBOX) { return BLOCK_JUKEBOX; }
    if (item == ITEM_NOTE_BLOCK) { return BLOCK_NOTE; }
    if (item == ITEM_DISPENSER) { return BLOCK_DISPENSER; }
    if (item == ITEM_TNT) { return BLOCK_TNT; }
    if (item == ITEM_BOOKSHELF) { return BLOCK_BOOKSHELF; }
    if (item == ITEM_BRICK_BLOCK) { return BLOCK_BRICK; }
    if (item == ITEM_CLAY_BLOCK) { return BLOCK_CLAY; }
    if (item == ITEM_GLOWSTONE_BLOCK) { return BLOCK_GLOWSTONE; }
    if (item == ITEM_TRAPDOOR) { return BLOCK_TRAPDOOR; }
    if (item == ITEM_PISTON) { return BLOCK_PISTON; }
    if (item == ITEM_STICKY_PISTON) { return BLOCK_PISTON_STICKY; }
    if (item == ITEM_REPEATER || item == ITEM_REDSTONE_REPEATER) { return BLOCK_REPEATER_OFF; }
    if (item == ITEM_PRESSURE_PLATE_STONE) { return BLOCK_STONE_PRESSURE_PLATE; }
    if (item == ITEM_PRESSURE_PLATE_WOOD) { return BLOCK_WOOD_PRESSURE_PLATE; }
    if (item == BLOCK_FLOWER_YELLOW || item == BLOCK_FLOWER_RED ||
        item == BLOCK_MUSHROOM_BROWN || item == BLOCK_MUSHROOM_RED ||
        item == BLOCK_GOLD_BLOCK || item == BLOCK_IRON_BLOCK ||
        item == BLOCK_DIAMOND_BLOCK || item == BLOCK_DOUBLE_STEP ||
        item == BLOCK_STEP || item == BLOCK_MOSSY_COBBLESTONE ||
        item == BLOCK_OBSIDIAN || item == BLOCK_SNOW_BLOCK ||
        item == BLOCK_PUMPKIN || item == BLOCK_NETHERRACK ||
        item == BLOCK_SOULSAND || item == BLOCK_JACK_O_LANTERN ||
        item == BLOCK_DETECTOR_RAIL || item == BLOCK_LOCKED_CHEST) { return item; }
    if (item == ITEM_REED) { return BLOCK_REED; }
    if (item == ITEM_CAKE) { return BLOCK_CAKE; }

    return BLOCK_AIR;
}


int AddItemToSlot(InventorySlot *slot, int item, int count)
{
    InventorySlot temp;
    if (!slot) { return count; }
    if (item == ITEM_NONE || count <= 0) { return count; }
    temp.item = item;
    temp.count = count;
    temp.damage = 0;
    GuiV21_AddStackToSlot(slot, &temp);
    return temp.count;
}

int AddItemToInventoryWithDamageV38(int item, int count, int damage)
{
    InventorySlot temp;
    temp.item = item;
    temp.count = count;
    temp.damage = damage;
    GuiV21_MoveStackToArmorThenPlayer(&temp);
    GuiV21_MoveStackToPlayerAll(&temp);
    return (temp.item == ITEM_NONE || temp.count <= 0) ? 1 : 0;
}

int AddItemToInventory(int item, int count)
{
    return AddItemToInventoryWithDamageV38(item, count, 0);
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

    amount = ItemCombatV6_ApplyArmorReduction(amount);

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
                if (pass == 0) { ore = BLOCK_COAL_ORE; attempts = (g_legacyPerformanceModeV13 ? 8 : 20); minY = 5; maxY = 76; veinBlocks = 10; radius = 3; }
                else if (pass == 1) { ore = BLOCK_IRON_ORE; attempts = (g_legacyPerformanceModeV13 ? 7 : 20); minY = 5; maxY = 64; veinBlocks = 8; radius = 2; }
                else if (pass == 2) { ore = BLOCK_GOLD_ORE; attempts = 2; minY = 5; maxY = 32; veinBlocks = 7; radius = 2; }
                else if (pass == 3) { ore = BLOCK_REDSTONE_ORE; attempts = (g_legacyPerformanceModeV13 ? 3 : 8); minY = 4; maxY = 20; veinBlocks = 7; radius = 2; }
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







/* ------------------------------------------------------------ */
/* V42 Java-style fluid, fire and lighting update helpers        */
/* ------------------------------------------------------------ */

int BlockV42_IsWater(int block)
{
    return block == BLOCK_WATER;
}

int BlockV42_IsLava(int block)
{
    return block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA;
}

int BlockV42_IsLiquid(int block)
{
    return BlockV42_IsWater(block) || BlockV42_IsLava(block);
}

int FluidV42_SameLiquid(int liquid, int block)
{
    if (liquid == BLOCK_WATER) { return BlockV42_IsWater(block); }
    return BlockV42_IsLava(block);
}

int FluidV42_GetDecayAt(int x, int y, int z, int liquid)
{
    int block;
    int meta;
    if (!IsInsideWorld(x, y, z)) { return -1; }
    block = GetBlock(x, y, z);
    if (!FluidV42_SameLiquid(liquid, block)) { return -1; }
    meta = (int)g_blockMeta[x][y][z];
    if (meta & FLUID_V42_FALLING_FLAG) { return 0; }
    return meta & FLUID_V42_LEVEL_MASK;
}

int FluidV42_BlockBlocksFlow(int block)
{
    if (block == BLOCK_AIR || block == BLOCK_FIRE) { return 0; }
    if (BlockV42_IsLiquid(block)) { return 0; }
    if (block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH || block == BLOCK_FLOWER_YELLOW ||
        block == BLOCK_FLOWER_RED || block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED ||
        block == BLOCK_SAPLING || block == BLOCK_SNOW) { return 0; }
    if (block == BLOCK_WOOD_DOOR || block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL ||
        block == BLOCK_LADDER || block == BLOCK_REED || block == BLOCK_CHEST || block == BLOCK_FURNACE ||
        block == BLOCK_LIT_FURNACE || block == BLOCK_FURNACE_LIT || block == BLOCK_DISPENSER ||
        block == BLOCK_WORKBENCH) { return 1; }
    return IsSolidBlock(block);
}

int FluidV42_CanDisplace(int liquid, int x, int y, int z)
{
    int block;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    block = GetBlock(x, y, z);
    if (FluidV42_SameLiquid(liquid, block)) { return 0; }
    if (liquid == BLOCK_WATER && BlockV42_IsLava(block)) { return 1; }
    if ((liquid == BLOCK_LAVA || liquid == BLOCK_STATIONARY_LAVA) && block == BLOCK_WATER) { return 1; }
    if (block == BLOCK_AIR || block == BLOCK_FIRE) { return 1; }
    if (block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH || block == BLOCK_FLOWER_YELLOW ||
        block == BLOCK_FLOWER_RED || block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED ||
        block == BLOCK_SAPLING || block == BLOCK_SNOW) { return 1; }
    return 0;
}

void EnqueueLightUpdateV42(int x, int y, int z, int radius)
{
    QueueBlockLightingAroundV48(x, y, z, radius);
}


void ProcessLightUpdatesV42(int maxUpdates)
{
    ProcessLightUpdatesV48(maxUpdates);
}


void FluidV42_SetBlockWithMeta(int x, int y, int z, int block, int meta)
{
    if (!IsInsideWorld(x, y, z)) { return; }
    SetBlock(x, y, z, block);
    if (IsInsideWorld(x, y, z)) { g_blockMeta[x][y][z] = (unsigned char)(meta & 15); }
    EnqueueLightUpdateV42(x, y, z, 12);
    InvalidateTerrainChunkMeshAt(x, z);
}

void FluidV42_ReplaceWithStoneFromMix(int x, int y, int z, int oldLiquid, int oldMeta)
{
    int result;
    result = BLOCK_COBBLESTONE;
    if (BlockV42_IsLava(oldLiquid) && ((oldMeta & FLUID_V42_LEVEL_MASK) == 0) && !(oldMeta & FLUID_V42_FALLING_FLAG)) { result = BLOCK_OBSIDIAN; }
    if (oldLiquid == BLOCK_WATER) { result = BLOCK_STONE; }
    SetBlock(x, y, z, result);
    PlaySoundAtV35("assets\\sounds\\random\\fizz.wav", (double)x + 0.5, (double)y + 0.5, (double)z + 0.5, 0.55, 1.0, SOUND_DEFAULT_RANGE_V35);
    SpawnParticleV24(PARTICLE_V24_SMOKE, (double)x + 0.5, (double)y + 0.9, (double)z + 0.5, 0.0, 0.06, 0.0, 0.9, 0.55, BLOCK_FIRE);
    EnqueueLightUpdateV42(x, y, z, 14);
}

void FluidV42_MixWaterLavaAround(int x, int y, int z)
{
    int block;
    int meta;
    int i;
    int nx;
    int ny;
    int nz;
    static const int dx[6] = { 1, -1, 0, 0, 0, 0 };
    static const int dy[6] = { 0, 0, 1, -1, 0, 0 };
    static const int dz[6] = { 0, 0, 0, 0, 1, -1 };
    if (!IsInsideWorld(x, y, z)) { return; }
    block = GetBlock(x, y, z);
    if (!BlockV42_IsLiquid(block)) { return; }
    for (i = 0; i < 6; i++) {
        nx = x + dx[i];
        ny = y + dy[i];
        nz = z + dz[i];
        if (!IsInsideWorld(nx, ny, nz)) { continue; }
        if ((BlockV42_IsWater(block) && BlockV42_IsLava(GetBlock(nx, ny, nz))) ||
            (BlockV42_IsLava(block) && BlockV42_IsWater(GetBlock(nx, ny, nz)))) {
            meta = (int)g_blockMeta[x][y][z];
            FluidV42_ReplaceWithStoneFromMix(x, y, z, block, meta);
            return;
        }
    }
}

int FluidV42_GetSmallestAdjacentDecay(int x, int y, int z, int liquid, int *sourceCount)
{
    int best;
    int d;
    best = -1;
    d = FluidV42_GetDecayAt(x - 1, y, z, liquid); if (d >= 0) { if (d == 0) { *sourceCount = *sourceCount + 1; } if (best < 0 || d < best) { best = d; } }
    d = FluidV42_GetDecayAt(x + 1, y, z, liquid); if (d >= 0) { if (d == 0) { *sourceCount = *sourceCount + 1; } if (best < 0 || d < best) { best = d; } }
    d = FluidV42_GetDecayAt(x, y, z - 1, liquid); if (d >= 0) { if (d == 0) { *sourceCount = *sourceCount + 1; } if (best < 0 || d < best) { best = d; } }
    d = FluidV42_GetDecayAt(x, y, z + 1, liquid); if (d >= 0) { if (d == 0) { *sourceCount = *sourceCount + 1; } if (best < 0 || d < best) { best = d; } }
    return best;
}

int FluidV42_FlowCost(int liquid, int x, int y, int z, int depth, int fromDir)
{
    int i;
    int nx;
    int nz;
    int best;
    static const int dx[4] = { -1, 1, 0, 0 };
    static const int dz[4] = { 0, 0, -1, 1 };
    if (depth >= 4) { return 1000; }
    if (!FluidV42_CanDisplace(liquid, x, y, z) && !FluidV42_SameLiquid(liquid, GetBlock(x, y, z))) { return 1000; }
    if (FluidV42_CanDisplace(liquid, x, y - 1, z)) { return depth; }
    best = 1000;
    for (i = 0; i < 4; i++) {
        if ((fromDir == 0 && i == 1) || (fromDir == 1 && i == 0) || (fromDir == 2 && i == 3) || (fromDir == 3 && i == 2)) { continue; }
        nx = x + dx[i];
        nz = z + dz[i];
        if (FluidV42_BlockBlocksFlow(GetBlock(nx, y, nz))) { continue; }
        if (FluidV42_FlowCost(liquid, nx, y, nz, depth + 1, i) < best) { best = FluidV42_FlowCost(liquid, nx, y, nz, depth + 1, i); }
    }
    return best;
}

void FluidV42_FlowInto(int liquid, int x, int y, int z, int meta)
{
    int oldBlock;
    if (!FluidV42_CanDisplace(liquid, x, y, z)) { return; }
    oldBlock = GetBlock(x, y, z);
    if ((liquid == BLOCK_WATER && BlockV42_IsLava(oldBlock)) || (BlockV42_IsLava(liquid) && oldBlock == BLOCK_WATER)) {
        FluidV42_ReplaceWithStoneFromMix(x, y, z, oldBlock, (int)g_blockMeta[x][y][z]);
        return;
    }
    FluidV42_SetBlockWithMeta(x, y, z, liquid == BLOCK_STATIONARY_LAVA ? BLOCK_LAVA : liquid, meta);
}

int FluidV42_UpdateOneLiquid(int x, int y, int z, int liquid)
{
    int block;
    int meta;
    int level;
    int step;
    int best;
    int sourceCount;
    int target;
    int downMeta;
    int sideLevel;
    int dir;
    int nx;
    int nz;
    int cost[4];
    int bestCost;
    int changes;
    static const int dx[4] = { -1, 1, 0, 0 };
    static const int dz[4] = { 0, 0, -1, 1 };

    if (!IsInsideWorld(x, y, z)) { return 0; }
    block = GetBlock(x, y, z);
    if (!FluidV42_SameLiquid(liquid, block)) { return 0; }
    FluidV42_MixWaterLavaAround(x, y, z);
    if (!FluidV42_SameLiquid(liquid, GetBlock(x, y, z))) { return 1; }

    meta = (int)g_blockMeta[x][y][z];
    level = meta & FLUID_V42_LEVEL_MASK;
    step = (liquid == BLOCK_WATER) ? 1 : 2;
    changes = 0;

    if (level != 0 && !(meta & FLUID_V42_FALLING_FLAG)) {
        sourceCount = 0;
        best = FluidV42_GetSmallestAdjacentDecay(x, y, z, liquid, &sourceCount);
        target = (best < 0) ? -1 : best + step;
        if (target >= 8) { target = -1; }
        if (FluidV42_GetDecayAt(x, y + 1, z, liquid) >= 0) { target = FLUID_V42_FALLING_FLAG | FluidV42_GetDecayAt(x, y + 1, z, liquid); }
        if (sourceCount >= 2 && liquid == BLOCK_WATER && IsSolidBlock(GetBlock(x, y - 1, z))) { target = 0; }
        if (target < 0) {
            SetBlock(x, y, z, BLOCK_AIR);
            return 1;
        }
        if (target != meta) {
            g_blockMeta[x][y][z] = (unsigned char)(target & 15);
            InvalidateTerrainChunkMeshAt(x, z);
            EnqueueLightUpdateV42(x, y, z, 10);
            changes++;
            meta = target;
            level = meta & FLUID_V42_LEVEL_MASK;
        }
    }

    if (FluidV42_CanDisplace(liquid, x, y - 1, z)) {
        downMeta = FLUID_V42_FALLING_FLAG | level;
        FluidV42_FlowInto(liquid, x, y - 1, z, downMeta);
        return changes + 1;
    }

    if (level >= 7) { return changes; }
    if (meta & FLUID_V42_FALLING_FLAG) { sideLevel = 1; }
    else { sideLevel = level + step; }
    if (sideLevel >= 8) { return changes; }

    bestCost = 1000;
    for (dir = 0; dir < 4; dir++) {
        nx = x + dx[dir];
        nz = z + dz[dir];
        cost[dir] = 1000;
        if (FluidV42_BlockBlocksFlow(GetBlock(nx, y, nz))) { continue; }
        if (FluidV42_SameLiquid(liquid, GetBlock(nx, y, nz)) && FluidV42_GetDecayAt(nx, y, nz, liquid) == 0) { continue; }
        if (FluidV42_CanDisplace(liquid, nx, y - 1, nz)) { cost[dir] = 0; }
        else { cost[dir] = FluidV42_FlowCost(liquid, nx, y, nz, 1, dir); }
        if (cost[dir] < bestCost) { bestCost = cost[dir]; }
    }

    for (dir = 0; dir < 4; dir++) {
        if (cost[dir] != bestCost || bestCost >= 1000) { continue; }
        nx = x + dx[dir];
        nz = z + dz[dir];
        FluidV42_FlowInto(liquid, nx, y, nz, sideLevel);
        changes++;
    }
    return changes;
}

int FireV42_GetEncourage(int block)
{
    if (block == BLOCK_PLANKS || block == BLOCK_FENCE || block == BLOCK_WOOD_DOOR || block == BLOCK_TRAPDOOR) { return 5; }
    if (block == BLOCK_WOOD) { return 5; }
    if (block == BLOCK_LEAVES || block == BLOCK_BOOKSHELF || block == BLOCK_WOOL) { return 30; }
    if (block == BLOCK_TNT) { return 15; }
    if (block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH || block == BLOCK_SAPLING) { return 60; }
    return 0;
}

int FireV42_GetCatchAbility(int block)
{
    if (block == BLOCK_PLANKS || block == BLOCK_FENCE || block == BLOCK_WOOD_DOOR || block == BLOCK_TRAPDOOR) { return 20; }
    if (block == BLOCK_WOOD) { return 5; }
    if (block == BLOCK_LEAVES || block == BLOCK_WOOL) { return 60; }
    if (block == BLOCK_BOOKSHELF) { return 20; }
    if (block == BLOCK_TNT || block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH || block == BLOCK_SAPLING) { return 100; }
    return 0;
}

int FireV42_CanCatch(int x, int y, int z)
{
    if (!IsInsideWorld(x, y, z)) { return 0; }
    return FireV42_GetEncourage(GetBlock(x, y, z)) > 0;
}

int FireV42_HasBurnableNeighbor(int x, int y, int z)
{
    if (FireV42_CanCatch(x + 1, y, z)) { return 1; }
    if (FireV42_CanCatch(x - 1, y, z)) { return 1; }
    if (FireV42_CanCatch(x, y + 1, z)) { return 1; }
    if (FireV42_CanCatch(x, y - 1, z)) { return 1; }
    if (FireV42_CanCatch(x, y, z + 1)) { return 1; }
    if (FireV42_CanCatch(x, y, z - 1)) { return 1; }
    return 0;
}

int FireV42_CanStay(int x, int y, int z)
{
    int below;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    below = GetBlock(x, y - 1, z);
    if (below == BLOCK_NETHERRACK) { return 1; }
    if (IsSolidBlock(below)) { return 1; }
    return FireV42_HasBurnableNeighbor(x, y, z);
}

void FireV42_TryCatchBlock(int x, int y, int z, int chance, int age)
{
    int ability;
    int h;
    int block;
    if (!IsInsideWorld(x, y, z)) { return; }
    block = GetBlock(x, y, z);
    ability = FireV42_GetCatchAbility(block);
    if (ability <= 0) { return; }
    h = WorldHash3D(x, y, z, (int)GetTickCount() + age * 73 + g_worldSeed);
    if ((h % chance) < ability) {
        if (block == BLOCK_TNT) {
            SetBlock(x, y, z, BLOCK_AIR);
            SpawnSpecialEntityV6(ENTITY_V6_TNT, ITEM_TNT, BLOCK_TNT, x + 0.5, y + 0.2, z + 0.5, 0.0, 3.0, 0.0, 3.7);
            return;
        }
        if ((h & 15) < age + 10) { SetBlock(x, y, z, BLOCK_AIR); }
        else { SetBlock(x, y, z, BLOCK_FIRE); g_blockMeta[x][y][z] = (unsigned char)((age + ((h >> 5) & 3)) & 15); }
    }
}

void FireV42_TrySpreadToAir(int x, int y, int z, int baseAge)
{
    int chance;
    int h;
    int age;
    if (!IsInsideWorld(x, y, z)) { return; }
    if (GetBlock(x, y, z) != BLOCK_AIR) { return; }
    chance = 0;
    if (FireV42_CanCatch(x + 1, y, z)) { chance += FireV42_GetEncourage(GetBlock(x + 1, y, z)); }
    if (FireV42_CanCatch(x - 1, y, z)) { chance += FireV42_GetEncourage(GetBlock(x - 1, y, z)); }
    if (FireV42_CanCatch(x, y + 1, z)) { chance += FireV42_GetEncourage(GetBlock(x, y + 1, z)); }
    if (FireV42_CanCatch(x, y - 1, z)) { chance += FireV42_GetEncourage(GetBlock(x, y - 1, z)); }
    if (FireV42_CanCatch(x, y, z + 1)) { chance += FireV42_GetEncourage(GetBlock(x, y, z + 1)); }
    if (FireV42_CanCatch(x, y, z - 1)) { chance += FireV42_GetEncourage(GetBlock(x, y, z - 1)); }
    if (chance <= 0) { return; }
    h = WorldHash3D(x, y, z, (int)GetTickCount() + g_worldSeed + 44042);
    if ((h & 255) < chance / 2) {
        age = baseAge + ((h >> 8) & 3);
        if (age > 15) { age = 15; }
        SetBlock(x, y, z, BLOCK_FIRE);
        g_blockMeta[x][y][z] = (unsigned char)age;
        SpawnParticleV24(PARTICLE_V24_FLAME, x + 0.5, y + 0.25, z + 0.5, 0.0, 0.04, 0.0, 0.7, 0.45, BLOCK_FIRE);
    }
}

int FireV42_UpdateOneFire(int x, int y, int z)
{
    int age;
    int h;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    if (GetBlock(x, y, z) != BLOCK_FIRE) { return 0; }
    age = (int)g_blockMeta[x][y][z] & 15;
    if (!FireV42_CanStay(x, y, z)) { SetBlock(x, y, z, BLOCK_AIR); return 1; }
    h = WorldHash3D(x, y, z, (int)GetTickCount() + g_worldSeed + 5111);
    if (age < 15 && (h & 1)) { g_blockMeta[x][y][z] = (unsigned char)(age + 1); }
    if (GetBlock(x, y - 1, z) != BLOCK_NETHERRACK && !FireV42_HasBurnableNeighbor(x, y, z) && age > 3) { SetBlock(x, y, z, BLOCK_AIR); return 1; }
    if (GetBlock(x, y - 1, z) != BLOCK_NETHERRACK && age == 15 && ((h >> 4) & 3) == 0) { SetBlock(x, y, z, BLOCK_AIR); return 1; }
    FireV42_TryCatchBlock(x + 1, y, z, 300, age);
    FireV42_TryCatchBlock(x - 1, y, z, 300, age);
    FireV42_TryCatchBlock(x, y - 1, z, 250, age);
    FireV42_TryCatchBlock(x, y + 1, z, 250, age);
    FireV42_TryCatchBlock(x, y, z - 1, 300, age);
    FireV42_TryCatchBlock(x, y, z + 1, 300, age);
    FireV42_TrySpreadToAir(x + 1, y, z, age);
    FireV42_TrySpreadToAir(x - 1, y, z, age);
    FireV42_TrySpreadToAir(x, y + 1, z, age);
    FireV42_TrySpreadToAir(x, y, z + 1, age);
    FireV42_TrySpreadToAir(x, y, z - 1, age);
    return 1;
}

void LavaV42_TryIgniteNear(int x, int y, int z)
{
    int i;
    int nx;
    int ny;
    int nz;
    int h;
    static const int dx[6] = { 1, -1, 0, 0, 0, 0 };
    static const int dy[6] = { 0, 0, 1, 1, 0, 0 };
    static const int dz[6] = { 0, 0, 0, 0, 1, -1 };
    h = WorldHash3D(x, y, z, (int)GetTickCount() + g_worldSeed + 7144);
    if ((h & 7) != 0) { return; }
    for (i = 0; i < 6; i++) {
        nx = x + dx[i];
        ny = y + dy[i];
        nz = z + dz[i];
        if (!IsInsideWorld(nx, ny, nz)) { continue; }
        if (GetBlock(nx, ny, nz) == BLOCK_AIR && FireV42_HasBurnableNeighbor(nx, ny, nz)) {
            SetBlock(nx, ny, nz, BLOCK_FIRE);
            g_blockMeta[nx][ny][nz] = (unsigned char)((h >> 4) & 3);
            return;
        }
    }
}

void UpdateJavaStyleFluidFireV42(double dt)
{
    /* V52_PRIORITY4: keep the old public name for compatibility, but route the
       actual work through the Java-style scheduled tick queue instead of
       scanning a whole cube of nearby world blocks every fluid/fire interval. */
    ProcessScheduledBlockTicksV52(dt, SCHEDULED_TICK_V52_FRAME_BUDGET);
}

float FluidV42_GetRenderedHeight(int x, int y, int z, int block)
{
    int meta;
    int level;
    if (!IsInsideWorld(x, y, z)) { return 0.875f; }
    if (FluidV42_SameLiquid(block, GetBlock(x, y + 1, z))) { return 1.0f; }
    meta = (int)g_blockMeta[x][y][z];
    if (meta & FLUID_V42_FALLING_FLAG) { return 1.0f; }
    level = meta & FLUID_V42_LEVEL_MASK;
    if (level <= 0) { return 0.875f; }
    return 0.125f + (float)(8 - level) / 9.0f;
}

void RenderFluidBlockV42(int x, int y, int z, int block)
{
    int col;
    int row;
    float u0;
    float v0;
    float u1;
    float v1;
    float h00;
    float h10;
    float h01;
    float h11;
    float btop;
    float bside;
    float alpha;
    int sameAbove;

    if (!texTerrain) { RenderBlockCubeBoundsV19(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, 0.875f, 1.0f, 1); return; }
    sameAbove = FluidV42_SameLiquid(block, GetBlock(x, y + 1, z));
    h00 = FluidV42_GetRenderedHeight(x, y, z, block);
    h10 = h00;
    h01 = h00;
    h11 = h00;
    if (FluidV42_SameLiquid(block, GetBlock(x - 1, y, z))) { h00 += FluidV42_GetRenderedHeight(x - 1, y, z, block); h01 += FluidV42_GetRenderedHeight(x - 1, y, z, block); h00 *= 0.5f; h01 *= 0.5f; }
    if (FluidV42_SameLiquid(block, GetBlock(x + 1, y, z))) { h10 += FluidV42_GetRenderedHeight(x + 1, y, z, block); h11 += FluidV42_GetRenderedHeight(x + 1, y, z, block); h10 *= 0.5f; h11 *= 0.5f; }
    if (FluidV42_SameLiquid(block, GetBlock(x, y, z - 1))) { h00 = (h00 + FluidV42_GetRenderedHeight(x, y, z - 1, block)) * 0.5f; h10 = (h10 + FluidV42_GetRenderedHeight(x, y, z - 1, block)) * 0.5f; }
    if (FluidV42_SameLiquid(block, GetBlock(x, y, z + 1))) { h01 = (h01 + FluidV42_GetRenderedHeight(x, y, z + 1, block)) * 0.5f; h11 = (h11 + FluidV42_GetRenderedHeight(x, y, z + 1, block)) * 0.5f; }

    if (block == BLOCK_WATER) { col = TILE_WATER_STILL_COL; row = TILE_WATER_STILL_ROW; alpha = 0.72f; }
    else { col = 14; row = 1; alpha = 0.92f; }
    GetTerrainTileUV(col, row, &u0, &v0, &u1, &v1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    if (!g_tessellatorActiveV8) { glBegin(GL_QUADS); }
    btop = GetLegacyFaceBrightness(x, y, z, 0, block) * 1.0f;
    bside = GetLegacyFaceBrightness(x, y, z, 2, block) * 0.78f;

    if (!sameAbove && ShouldDrawFace(x, y + 1, z, block)) {
        EmitLitVertex(u0, v0, (float)x,     (float)y + h00, (float)z,     btop);
        EmitLitVertex(u0, v1, (float)x,     (float)y + h01, (float)z + 1, btop);
        EmitLitVertex(u1, v1, (float)x + 1, (float)y + h11, (float)z + 1, btop);
        EmitLitVertex(u1, v0, (float)x + 1, (float)y + h10, (float)z,     btop);
    }
    if (ShouldDrawFace(x, y, z - 1, block)) {
        EmitLitVertex(u0, v1, (float)x + 1, (float)y,       (float)z, bside);
        EmitLitVertex(u1, v1, (float)x,     (float)y,       (float)z, bside);
        EmitLitVertex(u1, v0, (float)x,     (float)y + h00, (float)z, bside);
        EmitLitVertex(u0, v0, (float)x + 1, (float)y + h10, (float)z, bside);
    }
    if (ShouldDrawFace(x, y, z + 1, block)) {
        EmitLitVertex(u0, v1, (float)x,     (float)y,       (float)z + 1, bside);
        EmitLitVertex(u1, v1, (float)x + 1, (float)y,       (float)z + 1, bside);
        EmitLitVertex(u1, v0, (float)x + 1, (float)y + h11, (float)z + 1, bside);
        EmitLitVertex(u0, v0, (float)x,     (float)y + h01, (float)z + 1, bside);
    }
    if (ShouldDrawFace(x - 1, y, z, block)) {
        EmitLitVertex(u0, v1, (float)x, (float)y,       (float)z,     bside);
        EmitLitVertex(u1, v1, (float)x, (float)y,       (float)z + 1, bside);
        EmitLitVertex(u1, v0, (float)x, (float)y + h01, (float)z + 1, bside);
        EmitLitVertex(u0, v0, (float)x, (float)y + h00, (float)z,     bside);
    }
    if (ShouldDrawFace(x + 1, y, z, block)) {
        EmitLitVertex(u0, v1, (float)x + 1, (float)y,       (float)z + 1, bside);
        EmitLitVertex(u1, v1, (float)x + 1, (float)y,       (float)z,     bside);
        EmitLitVertex(u1, v0, (float)x + 1, (float)y + h10, (float)z,     bside);
        EmitLitVertex(u0, v0, (float)x + 1, (float)y + h11, (float)z + 1, bside);
    }
    if (!g_tessellatorActiveV8) { glEnd(); }
    (void)alpha;
}

int CanWaterFlowIntoBlock(int block)
{
    if (block == BLOCK_AIR || block == BLOCK_FIRE) { return 1; }
    if (block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH || block == BLOCK_FLOWER_YELLOW ||
        block == BLOCK_FLOWER_RED || block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED ||
        block == BLOCK_SAPLING || block == BLOCK_SNOW) { return 1; }
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
    /* V42 keeps this old exported name for older call sites, but the actual
       water/lava/fire tick scheduler now lives in UpdateJavaStyleFluidFireV42. */
    UpdateJavaStyleFluidFireV42(dt);
}


void SpawnWaterBubbleParticles(double x, double y, double z, int count)
{
    int i;
    int h;
    for (i = 0; i < count; i++) {
        h = WorldHash3D((int)(x * 31.0) + i, (int)(y * 17.0), (int)(z * 13.0), g_worldSeed + 8100);
        SpawnParticleV24(PARTICLE_V24_BUBBLE,
                         x + ((double)((h & 255) - 128)) / 120.0,
                         y + ((double)((h >> 8) & 255)) / 120.0,
                         z + ((double)(((h >> 16) & 255) - 128)) / 120.0,
                         ((double)((h & 255) - 128)) / 700.0,
                         0.020 + ((double)((h >> 8) & 255)) / 2500.0,
                         ((double)(((h >> 16) & 255) - 128)) / 700.0,
                         0.55, 0.38, BLOCK_WATER);
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


    if (item == ITEM_APPLE) { *col = 10; *row = 0; return 1; }
    if (item == ITEM_MUSHROOM_STEW) { *col = 8; *row = 4; return 1; }
    if (item == ITEM_SEEDS) { *col = 9; *row = 0; return 1; }
    if (item == ITEM_WHEAT) { *col = 9; *row = 1; return 1; }
    if (item == ITEM_BREAD) { *col = 9; *row = 2; return 1; }
    if (item == ITEM_LEATHER_HELMET) { *col = 0; *row = 0; return 1; }
    if (item == ITEM_LEATHER_CHESTPLATE) { *col = 0; *row = 1; return 1; }
    if (item == ITEM_LEATHER_LEGGINGS) { *col = 0; *row = 2; return 1; }
    if (item == ITEM_LEATHER_BOOTS) { *col = 0; *row = 3; return 1; }
    if (item == ITEM_CHAIN_HELMET) { *col = 1; *row = 0; return 1; }
    if (item == ITEM_CHAIN_CHESTPLATE) { *col = 1; *row = 1; return 1; }
    if (item == ITEM_CHAIN_LEGGINGS) { *col = 1; *row = 2; return 1; }
    if (item == ITEM_CHAIN_BOOTS) { *col = 1; *row = 3; return 1; }
    if (item == ITEM_IRON_HELMET) { *col = 2; *row = 0; return 1; }
    if (item == ITEM_IRON_CHESTPLATE) { *col = 2; *row = 1; return 1; }
    if (item == ITEM_IRON_LEGGINGS) { *col = 2; *row = 2; return 1; }
    if (item == ITEM_IRON_BOOTS) { *col = 2; *row = 3; return 1; }
    if (item == ITEM_DIAMOND_HELMET) { *col = 3; *row = 0; return 1; }
    if (item == ITEM_DIAMOND_CHESTPLATE) { *col = 3; *row = 1; return 1; }
    if (item == ITEM_DIAMOND_LEGGINGS) { *col = 3; *row = 2; return 1; }
    if (item == ITEM_DIAMOND_BOOTS) { *col = 3; *row = 3; return 1; }
    if (item == ITEM_GOLD_HELMET) { *col = 4; *row = 0; return 1; }
    if (item == ITEM_GOLD_CHESTPLATE) { *col = 4; *row = 1; return 1; }
    if (item == ITEM_GOLD_LEGGINGS) { *col = 4; *row = 2; return 1; }
    if (item == ITEM_GOLD_BOOTS) { *col = 4; *row = 3; return 1; }
    if (item == ITEM_PAINTING) { *col = 10; *row = 1; return 1; }
    if (item == ITEM_GOLDEN_APPLE) { *col = 11; *row = 0; return 1; }
    if (item == ITEM_SADDLE) { *col = 8; *row = 6; return 1; }
    if (item == ITEM_BRICK) { *col = 6; *row = 1; return 1; }
    if (item == ITEM_CLAY_BALL) { *col = 9; *row = 3; return 1; }
    if (item == ITEM_REED) { *col = 11; *row = 1; return 1; }
    if (item == ITEM_PAPER) { *col = 10; *row = 3; return 1; }
    if (item == ITEM_BOOK) { *col = 11; *row = 3; return 1; }
    if (item == ITEM_CHEST_MINECART) { *col = 7; *row = 9; return 1; }
    if (item == ITEM_FURNACE_MINECART) { *col = 7; *row = 10; return 1; }
    if (item == ITEM_COMPASS) { *col = 6; *row = 3; return 1; }
    if (item == ITEM_CLOCK) { *col = 6; *row = 4; return 1; }
    if (item == ITEM_GLOWSTONE_DUST) { *col = 9; *row = 4; return 1; }
    if (item == ITEM_FISH_RAW) { *col = 9; *row = 5; return 1; }
    if (item == ITEM_FISH_COOKED) { *col = 10; *row = 5; return 1; }
    if (item == ITEM_SUGAR) { *col = 13; *row = 0; return 1; }
    if (item == ITEM_BED) { *col = 13; *row = 2; return 1; }
    if (item == ITEM_COOKIE) { *col = 12; *row = 5; return 1; }
    if (item == ITEM_LAVA_BUCKET) { *col = 12; *row = 4; return 1; }
    if (item == ITEM_IRON_DOOR) { *col = 12; *row = 2; return 1; }
    if (item == ITEM_REPEATER) { *col = 6; *row = 5; return 1; }
    if (item == ITEM_CHICKEN_RAW) { *col = 9; *row = 6; return 1; }
    if (item == ITEM_CHICKEN_COOKED) { *col = 10; *row = 6; return 1; }
    if (item == ITEM_RECORD_13) { *col = 0; *row = 15; return 1; }
    if (item == ITEM_RECORD_CAT) { *col = 1; *row = 15; return 1; }
    *col = 5; *row = 3; return 1;
}


void DrawItemIcon2D(int item, int x, int y, int size)
{
    int block;
    int col;
    int row;
    int pad;

    pad = size >= 30 ? 1 : 0;

    /* V46: same ordering as held rendering.  Item IDs with Java item icons
       should not be pre-converted into their placed block before drawing. */
    if (ItemRenderV46_ShouldPreferItemIcon(item) && texBetaItems && GetItemIconTile(item, &col, &row)) {
        DrawTexturedQuad2D(texBetaItems, ICONS_ATLAS_WIDTH, ICONS_ATLAS_HEIGHT,
                           col, row, x + pad, y + pad, x + size - pad, y + size - pad);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        return;
    }

    block = ItemToBlock(item);
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
    int slotPixels;

    slotPixels = GuiV16_GetSlotPixels();
    if (selected) {
        DrawRect2D(x, y, x + slotPixels, y + slotPixels, 1.0f, 1.0f, 1.0f);
    }
    if (slot.item == ITEM_NONE || slot.count <= 0) { return; }

    iconSize = slotPixels - 4;
    if (iconSize > 32) { iconSize = 32; }
    if (iconSize < 14) { iconSize = 14; }
    iconX = x + (slotPixels - iconSize) / 2;
    iconY = y + (slotPixels - iconSize) / 2;
    DrawItemIcon2D(slot.item, iconX, iconY, iconSize);

    if (slot.count > 1) {
        wsprintf(text, "%d", slot.count);
        glColor3f(1.0f, 1.0f, 1.0f);
        DrawText2D(fontBaseNormal, x + slotPixels - 15, y + slotPixels - 2, text);
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

    scale = GuiV16_GetInventoryScale();
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
        if (slotType && *slotType == GUIV21_SLOT_ARMOR) { return -1; }
        if (slotType && *slotType == GUIV21_SLOT_HOTBAR) { *slotType = 3; }
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
        if (GuiV16_IsShiftDown() && !g_draggingInventory) {
            GuiV16_MoveStackToPlayer(&craftGrid[index]);
        } else {
            CraftingSlotClick(&craftGrid[index]);
        }
        UpdateCraftingResult();
        return;
    }

    if (type == 1) {
        if (craftResult.item == ITEM_NONE || craftResult.count <= 0) { return; }
        if (GuiV16_IsShiftDown() && !g_draggingInventory) {
            GuiV16_QuickCraftResultToPlayer();
            UpdateCraftingResult();
            return;
        }
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
        if (GuiV16_IsShiftDown() && !g_draggingInventory) { GuiV16_MoveStackToHotbarOrInventory(slot, 0); }
        else { CraftingSlotClick(slot); }
        UpdateCraftingResult();
        return;
    }

    if (type == 3) {
        if (index < 0 || index >= HOTBAR_SLOTS) { return; }
        slot = &hotbar[index];
        if (GuiV16_IsShiftDown() && !g_draggingInventory) { GuiV16_MoveStackToHotbarOrInventory(slot, 1); }
        else { CraftingSlotClick(slot); }
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
        if (GuiV16_IsShiftDown() && !g_draggingInventory) {
            GuiV16_MoveStackToPlayer(&craftGrid[index]);
        } else {
            CraftingSlotRightClick(&craftGrid[index]);
        }
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
        if (GuiV16_IsShiftDown() && !g_draggingInventory) { GuiV16_MoveStackToHotbarOrInventory(slot, 0); }
        else { CraftingSlotRightClick(slot); }
        UpdateCraftingResult();
        return;
    }

    if (type == 3) {
        if (index < 0 || index >= HOTBAR_SLOTS) { return; }
        slot = &hotbar[index];
        if (GuiV16_IsShiftDown() && !g_draggingInventory) { GuiV16_MoveStackToHotbarOrInventory(slot, 1); }
        else { CraftingSlotRightClick(slot); }
        UpdateCraftingResult();
        return;
    }
}



void DrawSlotHoverFrame(int x, int y, int mx, int my)
{
    int size;
    size = GuiV16_GetSlotPixels();
    if (mx >= x && mx < x + size && my >= y && my < y + size) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glColor4f(1.0f, 1.0f, 1.0f, 0.22f);
        glBegin(GL_QUADS);
        glVertex2i(x, y); glVertex2i(x + size, y);
        glVertex2i(x + size, y + size); glVertex2i(x, y + size);
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
    scale = GuiV16_GetInventoryScale();
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
    if (!g_draggingInventory) { GuiV16_DrawSlotTooltip(mouse.x, mouse.y); }
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
    double pitchRad;
    double dx;
    double dy;
    double dz;
    int count;
    int idx;

    if (!g_draggingInventory || g_dragSlot.item == ITEM_NONE || g_dragSlot.count <= 0) {
        return;
    }

    count = g_dragSlot.count;
    if (oneOnly && count > 1) {
        count = 1;
    }

    yawRad = yaw * PI / 180.0;
    pitchRad = pitch * PI / 180.0;
    dx = -sin(yawRad) * cos(pitchRad);
    dy = sin(pitchRad);
    dz = -cos(yawRad) * cos(pitchRad);

    idx = AddDroppedItemStackV38(g_dragSlot.item,
                                 count,
                                 g_dragSlot.damage,
                                 playerX + dx * 0.48,
                                 playerY + EYE_HEIGHT - 0.28 + dy * 0.18,
                                 playerZ + dz * 0.48,
                                 dx * 1.25,
                                 dy * 0.40 + 0.10,
                                 dz * 1.25);
    if (idx < 0) { return; }
    droppedItems[idx].pickupDelay = 0.85;

    g_dragSlot.count -= count;
    if (g_dragSlot.count <= 0) {
        g_dragSlot.item = ITEM_NONE;
        g_dragSlot.count = 0;
        g_dragSlot.damage = 0;
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
    int scale;
    scale = GuiV16_GetInventoryScale();
    panelW = BETA_INV_GUI_W * scale;
    panelH = BETA_INV_GUI_H * scale;
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

    scale = GuiV16_GetInventoryScale();
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
            if (GuiV16_IsShiftDown() && !g_draggingInventory) {
                GuiV16_MoveStackToPlayer(&craftGrid[index]);
            } else {
                CraftingSlotClick(&craftGrid[index]);
            }
            UpdateCraftingResult();
            return;
        }
        if (type == 1) {
            if (craftResult.item == ITEM_NONE || craftResult.count <= 0) { return; }
            if (GuiV16_IsShiftDown() && !g_draggingInventory) {
                GuiV16_QuickCraftResultToPlayer();
                UpdateCraftingResult();
                return;
            }
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
        slot = GuiV21_GetPlayerSlotByKind(index, isHotbar);
        if (!slot) { return; }
        if (GuiV16_IsShiftDown() && !g_draggingInventory) {
            if (isHotbar == GUIV21_SLOT_ARMOR) { GuiV21_MoveStackToPlayerAll(slot); }
            else { if (!GuiV21_MoveStackToArmorThenPlayer(slot)) { GuiV16_MoveStackToHotbarOrInventory(slot, isHotbar == GUIV21_SLOT_HOTBAR); } }
        } else {
            GuiV21_SlotClick(slot, isHotbar == GUIV21_SLOT_ARMOR ? GUIV21_SLOT_ARMOR : isHotbar, index, 0);
        }
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
            if (GuiV16_IsShiftDown() && !g_draggingInventory) {
                GuiV16_MoveStackToPlayer(&craftGrid[index]);
            } else {
                CraftingSlotRightClick(&craftGrid[index]);
            }
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
        slot = GuiV21_GetPlayerSlotByKind(index, isHotbar);
        if (!slot) { return; }
        if (GuiV16_IsShiftDown() && !g_draggingInventory) {
            if (isHotbar == GUIV21_SLOT_ARMOR) { GuiV21_MoveStackToPlayerAll(slot); }
            else { if (!GuiV21_MoveStackToArmorThenPlayer(slot)) { GuiV16_MoveStackToHotbarOrInventory(slot, isHotbar == GUIV21_SLOT_HOTBAR); } }
        } else {
            GuiV21_SlotClick(slot, isHotbar == GUIV21_SLOT_ARMOR ? GUIV21_SLOT_ARMOR : isHotbar, index, 1);
        }
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
    DrawArmorIconsV21();
    DrawOxygenBubbles();
    DrawHotbar();
    if (inventoryOpen) {
        if (g_containerModeV5 != CONTAINER_NONE_V5) { DrawActiveContainerV5(); }
        else if (craftingOpen) { DrawCraftingScreen(); } else { DrawInventoryScreen(); }
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






void DrawArmorIconsV21(void)
{
    int armor;
    int i;
    int x;
    int y;
    int iconSize;
    armor = 0;
    for (i = 0; i < 4; i++) {
        if (g_armorSlotsV6[i].item != ITEM_NONE && g_armorSlotsV6[i].count > 0) {
            if (i == 0) { armor += 2; }
            else if (i == 1) { armor += 6; }
            else if (i == 2) { armor += 5; }
            else { armor += 2; }
        }
    }
    if (armor <= 0) { return; }
    if (armor > 20) { armor = 20; }
    iconSize = 18;
    x = g_windowWidth / 2 - 91 * 2;
    y = g_windowHeight - 110;
    for (i = 0; i < 10; i++) {
        DrawImageCrop2D(texBetaIcons, 256, 256, 16, 9, 9, 9, x + i * 18, y, x + i * 18 + iconSize, y + iconSize, 1.0f);
        if (i * 2 + 1 < armor) { DrawImageCrop2D(texBetaIcons, 256, 256, 34, 9, 9, 9, x + i * 18, y, x + i * 18 + iconSize, y + iconSize, 1.0f); }
        else if (i * 2 + 1 == armor) { DrawImageCrop2D(texBetaIcons, 256, 256, 25, 9, 9, 9, x + i * 18, y, x + i * 18 + iconSize, y + iconSize, 1.0f); }
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
    int scale;

    scale = GuiV16_GetInventoryScale();
    panelW = BETA_INV_GUI_W * scale;
    panelH = BETA_INV_GUI_H * scale;
    panelX = g_windowWidth / 2 - panelW / 2;
    panelY = g_windowHeight / 2 - panelH / 2;
    slotSize = BETA_INV_SLOT * scale;
    if (isHotbar) { *isHotbar = GUIV21_SLOT_PLAYER_INV; }

    for (row = 0; row < 4; row++) {
        left = panelX + 8 * scale;
        top = panelY + (8 + row * 18) * scale;
        if (mx >= left && mx < left + slotSize && my >= top && my < top + slotSize) { if (isHotbar) { *isHotbar = GUIV21_SLOT_ARMOR; } return row; }
    }
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 9; col++) {
            left = panelX + (BETA_INV_MAIN_X + col * BETA_INV_SLOT) * scale;
            top  = panelY + (BETA_INV_MAIN_Y + row * BETA_INV_SLOT) * scale;
            if (mx >= left && mx < left + slotSize && my >= top && my < top + slotSize) { if (isHotbar) { *isHotbar = GUIV21_SLOT_PLAYER_INV; } return row * 9 + col; }
        }
    }
    for (col = 0; col < HOTBAR_SLOTS; col++) {
        left = panelX + (BETA_INV_HOTBAR_X + col * BETA_INV_SLOT) * scale;
        top  = panelY + BETA_INV_HOTBAR_Y * scale;
        if (mx >= left && mx < left + slotSize && my >= top && my < top + slotSize) { if (isHotbar) { *isHotbar = GUIV21_SLOT_HOTBAR; } return col; }
    }
    return -1;
}

void InventoryMouseClick(int mx, int my)
{
    int kind;
    int index;
    InventorySlot *slot;
    index = GetInventorySlotAtPoint(mx, my, &kind);
    if (index < 0) { return; }
    slot = GuiV21_GetPlayerSlotByKind(index, kind);
    if (!slot) { return; }
    if (GuiV16_IsShiftDown() && !g_draggingInventory) {
        if (kind == GUIV21_SLOT_ARMOR) { GuiV21_MoveStackToPlayerAll(slot); }
        else { if (GuiV21_MoveStackToArmorThenPlayer(slot)) { return; } GuiV16_MoveStackToHotbarOrInventory(slot, kind == GUIV21_SLOT_HOTBAR); }
        return;
    }
    GuiV21_SlotClick(slot, kind == GUIV21_SLOT_ARMOR ? GUIV21_SLOT_ARMOR : kind, index, 0);
}

void InventoryMouseRightClick(int mx, int my)
{
    int kind;
    int index;
    InventorySlot *slot;
    index = GetInventorySlotAtPoint(mx, my, &kind);
    if (index < 0) { return; }
    slot = GuiV21_GetPlayerSlotByKind(index, kind);
    if (!slot) { return; }
    if (GuiV16_IsShiftDown() && !g_draggingInventory) {
        if (kind == GUIV21_SLOT_ARMOR) { GuiV21_MoveStackToPlayerAll(slot); }
        else { if (GuiV21_MoveStackToArmorThenPlayer(slot)) { return; } GuiV16_MoveStackToHotbarOrInventory(slot, kind == GUIV21_SLOT_HOTBAR); }
    } else { GuiV21_SlotClick(slot, kind == GUIV21_SLOT_ARMOR ? GUIV21_SLOT_ARMOR : kind, index, 1); }
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
    int scale;
    POINT mouse;

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    scale = GuiV16_GetInventoryScale();
    panelW = BETA_INV_GUI_W * scale;
    panelH = BETA_INV_GUI_H * scale;
    panelX = g_windowWidth / 2 - panelW / 2;
    panelY = g_windowHeight / 2 - panelH / 2;
    slotSize = BETA_INV_SLOT * scale;

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

    for (row = 0; row < 4; row++) {
        slotX = panelX + 8 * scale;
        slotY = panelY + (8 + row * 18) * scale;
        DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
        DrawInventorySlot(slotX, slotY, g_armorSlotsV6[row], 0);
        if (g_armorSlotsV6[row].item == ITEM_NONE) {
            glColor3f(0.42f, 0.42f, 0.42f);
            if (row == 0) { DrawText2D(fontBaseNormal, slotX + 4, slotY + 24, "H"); }
            else if (row == 1) { DrawText2D(fontBaseNormal, slotX + 4, slotY + 24, "C"); }
            else if (row == 2) { DrawText2D(fontBaseNormal, slotX + 4, slotY + 24, "L"); }
            else { DrawText2D(fontBaseNormal, slotX + 4, slotY + 24, "B"); }
        }
    }

    /* Draw the built-in 2x2 inventory crafting grid and its result slot. */
    slotX = panelX + 88 * scale;
    slotY = panelY + 26 * scale;
    DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
    DrawCenteredItemStack(slotX, slotY, craftGrid[0], 0);
    slotX = panelX + 106 * scale;
    slotY = panelY + 26 * scale;
    DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
    DrawCenteredItemStack(slotX, slotY, craftGrid[1], 0);
    slotX = panelX + 88 * scale;
    slotY = panelY + 44 * scale;
    DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
    DrawCenteredItemStack(slotX, slotY, craftGrid[3], 0);
    slotX = panelX + 106 * scale;
    slotY = panelY + 44 * scale;
    DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
    DrawCenteredItemStack(slotX, slotY, craftGrid[4], 0);
    slotX = panelX + 144 * scale;
    slotY = panelY + 36 * scale;
    DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
    DrawCenteredItemStack(slotX, slotY, craftResult, 0);

    /*
        Draw items only in the valid dark slot squares from the inventory GUI:
        3x9 storage rows and 1x9 hotbar. Armor and crafting tiles stay empty.
    */
    index = 0;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 9; col++) {
            slotX = panelX + (BETA_INV_MAIN_X + col * BETA_INV_SLOT) * scale;
            slotY = panelY + (BETA_INV_MAIN_Y + row * BETA_INV_SLOT) * scale;
            DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
            DrawInventorySlot(slotX, slotY, inventory[index], 0);
            index++;
        }
    }

    for (col = 0; col < HOTBAR_SLOTS; col++) {
        slotX = panelX + (BETA_INV_HOTBAR_X + col * BETA_INV_SLOT) * scale;
        slotY = panelY + BETA_INV_HOTBAR_Y * scale;
        DrawSlotHoverFrame(slotX, slotY, mouse.x, mouse.y);
        DrawInventorySlot(slotX, slotY, hotbar[col], col == selectedHotbarSlot);
    }

    DrawCarriedInventoryStack();
    if (!g_draggingInventory) { GuiV16_DrawSlotTooltip(mouse.x, mouse.y); }
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




