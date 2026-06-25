/* ============================================================
   CloneMC V51 section: ITEM / RECIPES / CONTAINERS / REDSTONE / FEATURE SYSTEMS
   ============================================================ */

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
    /* V37: closer ItemTool/ItemSword/ItemHoe/ItemShears/ItemFlintAndSteel/
       ItemFishingRod durability values.  These are stored as ItemStack damage
       and break when damage reaches the Java max-use count. */
    if (item == ITEM_WOOD_SWORD || item == ITEM_WOOD_SHOVEL || item == ITEM_WOOD_PICKAXE || item == ITEM_WOOD_AXE || item == ITEM_WOOD_HOE) { return 59; }
    if (item == ITEM_STONE_SWORD || item == ITEM_STONE_SHOVEL || item == ITEM_STONE_PICKAXE || item == ITEM_STONE_AXE || item == ITEM_STONE_HOE) { return 131; }
    if (item == ITEM_IRON_SWORD || item == ITEM_IRON_SHOVEL || item == ITEM_IRON_PICKAXE || item == ITEM_IRON_AXE || item == ITEM_IRON_HOE) { return 250; }
    if (item == ITEM_DIAMOND_SWORD || item == ITEM_DIAMOND_SHOVEL || item == ITEM_DIAMOND_PICKAXE || item == ITEM_DIAMOND_AXE || item == ITEM_DIAMOND_HOE) { return 1561; }
    if (item == ITEM_GOLD_SWORD || item == ITEM_GOLD_SHOVEL || item == ITEM_GOLD_PICKAXE || item == ITEM_GOLD_AXE || item == ITEM_GOLD_HOE) { return 32; }
    if (item == ITEM_FLINT_STEEL) { return 64; }
    if (item == ITEM_FISHING_ROD) { return 64; }
    if (item == ITEM_SHEARS) { return 238; }
    /* Keep bow durability enabled for the clone's charged-bow path. */
    if (item == ITEM_BOW) { return 385; }
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
    return block == BLOCK_WOOD || block == BLOCK_PLANKS || block == BLOCK_CHEST || block == BLOCK_WORKBENCH ||
           block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL || block == BLOCK_WOOD_DOOR || block == BLOCK_FENCE ||
           block == BLOCK_BOOKSHELF || block == BLOCK_TRAPDOOR || block == BLOCK_JUKEBOX || block == BLOCK_NOTE;
}


int IsShovelBlock(int block)
{
    return block == BLOCK_DIRT || block == BLOCK_GRASS || block == BLOCK_SAND || block == BLOCK_GRAVEL ||
           block == BLOCK_SNOW || block == BLOCK_SNOW_BLOCK || block == BLOCK_CLAY || block == BLOCK_FARMLAND;
}


double GetToolMiningSpeed(int item, int block)
{
    int level;
    int tool;
    double speed;
    level = GetToolHarvestLevel(item);
    tool = BlockV49_GetHarvestTool(block);
    speed = 1.0;

    /* V37: Java ItemTool/ItemSword/ItemShears style strength vs block. */
    if (item == ITEM_SHEARS) {
        if (block == BLOCK_WEB || block == BLOCK_LEAVES) { return 15.0; }
        if (block == BLOCK_WOOL) { return 5.0; }
    }
    if (ItemCombatV6_IsSword(item)) {
        if (block == BLOCK_WEB) { return 15.0; }
        return 1.5;
    }

    if ((tool == BLOCK_HARVEST_PICKAXE_V49 && (item == ITEM_WOOD_PICKAXE || item == ITEM_STONE_PICKAXE || item == ITEM_IRON_PICKAXE || item == ITEM_DIAMOND_PICKAXE || item == ITEM_GOLD_PICKAXE)) ||
        (tool == BLOCK_HARVEST_AXE_V49 && (item == ITEM_WOOD_AXE || item == ITEM_STONE_AXE || item == ITEM_IRON_AXE || item == ITEM_DIAMOND_AXE || item == ITEM_GOLD_AXE)) ||
        (tool == BLOCK_HARVEST_SHOVEL_V49 && (item == ITEM_WOOD_SHOVEL || item == ITEM_STONE_SHOVEL || item == ITEM_IRON_SHOVEL || item == ITEM_DIAMOND_SHOVEL || item == ITEM_GOLD_SHOVEL))) {
        if (level == 0) { speed = 2.0; }
        else if (level == 1) { speed = 4.0; }
        else if (level == 2) { speed = 6.0; }
        else if (level >= 3) { speed = 8.0; }
    }

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
    r->width = 0; r->height = 0; r->outItem = ITEM_NONE; r->outCount = 0; r->outDamage = 0; r->shapeless = 0;
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

    /* TILE_RECIPE_REDSTONE_V5: direct conversion targets from CraftingManager,
       RecipesIngots, RecipesFood, RecipesArmor, RecipesDyes, and FurnaceRecipes.
       Data stays table-driven so GUI/container code does not hardcode outcomes. */
    RecipeAddShaped(3, 3, ITEM_FENCE, 2, ITEM_STICK, ITEM_STICK, ITEM_STICK, ITEM_STICK, ITEM_STICK, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_BOOKSHELF, 1, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_BOOK, ITEM_BOOK, ITEM_BOOK, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS);
    RecipeAddShaped(3, 3, ITEM_JUKEBOX, 1, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_DIAMOND, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS);
    RecipeAddShaped(3, 3, ITEM_NOTE_BLOCK, 1, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_REDSTONE, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS);
    RecipeAddShaped(3, 3, ITEM_DISPENSER, 1, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_BOW, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_REDSTONE, ITEM_COBBLESTONE);
    RecipeAddShaped(3, 3, ITEM_TNT, 1, ITEM_GUNPOWDER, ITEM_SAND, ITEM_GUNPOWDER, ITEM_SAND, ITEM_GUNPOWDER, ITEM_SAND, ITEM_GUNPOWDER, ITEM_SAND, ITEM_GUNPOWDER);
    RecipeAddShaped(3, 2, ITEM_TRAPDOOR, 2, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(3, 2, ITEM_REPEATER, 1, ITEM_REDSTONE_TORCH, ITEM_REDSTONE, ITEM_REDSTONE_TORCH, ITEM_STONE, ITEM_STONE, ITEM_STONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(3, 2, ITEM_PISTON, 1, ITEM_PLANKS, ITEM_PLANKS, ITEM_PLANKS, ITEM_COBBLESTONE, ITEM_IRON_INGOT, ITEM_COBBLESTONE, ITEM_COBBLESTONE, ITEM_REDSTONE, ITEM_COBBLESTONE);
    RecipeAddShaped(1, 2, ITEM_STICKY_PISTON, 1, ITEM_SLIMEBALL, ITEM_NONE, ITEM_NONE, ITEM_PISTON, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(3, 1, ITEM_PRESSURE_PLATE_STONE, 1, ITEM_STONE, ITEM_STONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(3, 1, ITEM_PRESSURE_PLATE_WOOD, 1, ITEM_PLANKS, ITEM_PLANKS, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(1, 2, ITEM_REDSTONE_TORCH, 1, ITEM_REDSTONE, ITEM_NONE, ITEM_NONE, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(1, 1, ITEM_BUTTON, 1, ITEM_STONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(1, 2, ITEM_LEVER, 1, ITEM_STICK, ITEM_NONE, ITEM_NONE, ITEM_COBBLESTONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(2, 2, ITEM_CLAY_BLOCK, 1, ITEM_CLAY_BALL, ITEM_CLAY_BALL, ITEM_NONE, ITEM_CLAY_BALL, ITEM_CLAY_BALL, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(2, 2, ITEM_BRICK_BLOCK, 1, ITEM_BRICK, ITEM_BRICK, ITEM_NONE, ITEM_BRICK, ITEM_BRICK, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(2, 2, ITEM_GLOWSTONE_BLOCK, 1, ITEM_GLOWSTONE_DUST, ITEM_GLOWSTONE_DUST, ITEM_NONE, ITEM_GLOWSTONE_DUST, ITEM_GLOWSTONE_DUST, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(3, 1, ITEM_PAPER, 3, ITEM_REED, ITEM_REED, ITEM_REED, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(1, 3, ITEM_BOOK, 1, ITEM_PAPER, ITEM_NONE, ITEM_NONE, ITEM_PAPER, ITEM_NONE, ITEM_NONE, ITEM_PAPER, ITEM_NONE, ITEM_NONE);
    RecipeAddShapeless(ITEM_SUGAR, 1, ITEM_REED, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShapeless(ITEM_MUSHROOM_STEW, 1, ITEM_BOWL, BLOCK_MUSHROOM_BROWN, BLOCK_MUSHROOM_RED, ITEM_NONE);
    RecipeAddShapeless(ITEM_COOKIE, 8, ITEM_WHEAT, ITEM_DYE_POWDER, ITEM_WHEAT, ITEM_NONE);

    RecipeAddShaped(3, 2, ITEM_LEATHER_HELMET, 1, ITEM_LEATHER, ITEM_LEATHER, ITEM_LEATHER, ITEM_LEATHER, ITEM_NONE, ITEM_LEATHER, ITEM_NONE, ITEM_NONE, ITEM_NONE);
    RecipeAddShaped(3, 3, ITEM_LEATHER_CHESTPLATE, 1, ITEM_LEATHER, ITEM_NONE, ITEM_LEATHER, ITEM_LEATHER, ITEM_LEATHER, ITEM_LEATHER, ITEM_LEATHER, ITEM_LEATHER, ITEM_LEATHER);
    RecipeAddShaped(3, 3, ITEM_LEATHER_LEGGINGS, 1, ITEM_LEATHER, ITEM_LEATHER, ITEM_LEATHER, ITEM_LEATHER, ITEM_NONE, ITEM_LEATHER, ITEM_LEATHER, ITEM_NONE, ITEM_LEATHER);
    RecipeAddShaped(3, 2, ITEM_LEATHER_BOOTS, 1, ITEM_LEATHER, ITEM_NONE, ITEM_LEATHER, ITEM_LEATHER, ITEM_NONE, ITEM_LEATHER, ITEM_NONE, ITEM_NONE, ITEM_NONE);
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
            craftResult.damage = g_simpleRecipes[i].outDamage;
            return 1;
        }
    }
    return 0;
}


/* TILE_RECIPE_REDSTONE_V5 early prototypes for Open Watcom/C89: this block is
   intentionally duplicated before the V5 implementation because the large
   single-file build defines some UI/render helpers later in the file. */
void UpdateRedstoneNetworkV5(int cx, int cy, int cz);
void UpdatePistonsV5(void);
void UpdateMinecartsV5(double dt);
void RenderMinecartsV5(void);
void SaveRedstoneMetaV5(void);
int LoadRedstoneMetaV5(void);
int SpawnMinecartV5(double x, double y, double z, double vx, double vz);
void CloseActiveContainerV5(void);
void OpenTileContainerV5(int block, int x, int y, int z);
int TryActivateTileOrRedstoneV5(int x, int y, int z);
void TileEntitiesRedstoneV5_Update(double dt);
void DrawActiveContainerV5(void);
/* UI_HUD_V16 early prototypes used by early V5 container code. */
void DropCarriedInventoryStackToWorld(int oneOnly);
void DrawRect2D(int x1, int y1, int x2, int y2, float r, float g, float b);
void DrawCenteredText2D(GLuint base, int x1, int y1, int x2, int y2, const char *text);
void CraftingSlotClick(InventorySlot *slot);
void CraftingSlotRightClick(InventorySlot *slot);
void DrawSlotHoverFrame(int x, int y, int mx, int my);
void DrawCenteredItemStack(int x, int y, InventorySlot slot, int selected);
void DrawCarriedInventoryStack(void);
int GuiV16_IsShiftDown(void);
int GuiV16_MoveStackToPlayer(InventorySlot *src);
int GuiV16_MoveStackToTileContainer(InventorySlot *src, TileEntityState *t);
int GuiV16_ClickTakeOnlySlot(InventorySlot *slot, int rightClick);
void GuiV16_DrawSlotTooltip(int mx, int my);
int GuiV16_GetInventoryScale(void);


#define GUIV21_SLOT_PLAYER_INV 0
#define GUIV21_SLOT_HOTBAR 1
#define GUIV21_SLOT_ARMOR 2
#define GUIV21_SLOT_CRAFT_GRID 3
#define GUIV21_SLOT_CRAFT_RESULT 4
#define GUIV21_SLOT_TILE_NORMAL 5
#define GUIV21_SLOT_FURNACE_INPUT 6
#define GUIV21_SLOT_FURNACE_FUEL 7
#define GUIV21_SLOT_FURNACE_OUTPUT 8

int TileSlotMaxV5(int block);
int GetContainerSlotAtPointV5(int mx, int my, int *slotType);
int FurnaceRecipeOutputV5(int item);
int FurnaceRecipeOutputDamageV21(int item);
int ItemV21_MaxStackSize(int item);
int ItemV21_IsArmorForSlot(int item, int armorSlot);
InventorySlot *GuiV21_GetPlayerSlotByKind(int index, int kind);
int GuiV21_CanPlaceInSlot(int role, int index, int item);
int GuiV21_AddStackToSlot(InventorySlot *slot, InventorySlot *src);
int GuiV21_MoveStackToValidatedSlots(InventorySlot *src, InventorySlot *dst, int dstCount, int roleBase);
int GuiV21_MoveStackToPlayerAll(InventorySlot *src);
int GuiV21_MoveStackToArmorThenPlayer(InventorySlot *src);
int GuiV21_SlotClick(InventorySlot *slot, int role, int index, int rightClick);
const char *ItemV21_GetDisplayName(int item);
void TileEntitiesRedstoneV5_Update(double dt);
void DrawArmorIconsV21(void);
int ItemV36_AreStacksCompatible(InventorySlot *a, InventorySlot *b);
int GuiV36_MoveStackToSlotRole(InventorySlot *src, InventorySlot *dst, int role, int index);
int GuiV36_MoveStackToSlotArrayRole(InventorySlot *src, InventorySlot *dst, int dstCount, int roleBase);
int FurnaceV36_IsFurnaceBlock(int block);
int FurnaceV36_FuelBurnTicks(int item);
int FurnaceV36_CanSmeltInput(int item);
void FurnaceV36_ConsumeFuel(TileEntityState *t);
void DispenserV36_UpdateOne(TileEntityState *t, double dt);
void SignV36_DrawText(TileEntityState *t, int x1, int y1, int x2, int y2);
void HandleSignKeyV36(WPARAM key);

int ItemV21_MaxStackSize(int item)
{
    if (item == ITEM_NONE) { return 0; }
    if (GetItemMaxDurability(item) > 0) { return 1; }
    if (item == ITEM_BUCKET || item == ITEM_WATER_BUCKET || item == ITEM_LAVA_BUCKET || item == ITEM_MILK_BUCKET) { return 1; }
    if (item == ITEM_MUSHROOM_STEW || item == ITEM_BOAT || item == ITEM_MINECART || item == ITEM_SADDLE) { return 1; }
    if (ItemV21_IsArmorForSlot(item, 0) || ItemV21_IsArmorForSlot(item, 1) || ItemV21_IsArmorForSlot(item, 2) || ItemV21_IsArmorForSlot(item, 3)) { return 1; }
    if (item == ITEM_SIGN || item == ITEM_WOOD_DOOR || item == ITEM_IRON_DOOR) { return 16; }
    if (item == ITEM_EGG || item == ITEM_SNOWBALL) { return 16; }
    return MAX_STACK;
}


int ItemV36_AreStacksCompatible(InventorySlot *a, InventorySlot *b)
{
    if (!a || !b) { return 0; }
    if (a->item == ITEM_NONE || b->item == ITEM_NONE) { return 0; }
    if (a->count <= 0 || b->count <= 0) { return 0; }
    if (a->item != b->item) { return 0; }
    if (a->damage != b->damage) { return 0; }
    return 1;
}

int GuiV36_MoveStackToSlotRole(InventorySlot *src, InventorySlot *dst, int role, int index)
{
    int maxStack;
    int moved;
    int space;
    if (!src || !dst) { return 0; }
    if (src->item == ITEM_NONE || src->count <= 0) { return 0; }
    if (!GuiV21_CanPlaceInSlot(role, index, src->item)) { return 0; }
    maxStack = ItemV21_MaxStackSize(src->item);
    if (maxStack <= 0) { return 0; }
    if (dst->item == ITEM_NONE || dst->count <= 0) {
        moved = src->count;
        if (moved > maxStack) { moved = maxStack; }
        dst->item = src->item;
        dst->count = moved;
        dst->damage = src->damage;
        src->count -= moved;
        if (src->count <= 0) { src->item = ITEM_NONE; src->count = 0; src->damage = 0; }
        return moved;
    }
    if (dst->item != src->item || dst->damage != src->damage) { return 0; }
    if (dst->count >= maxStack) { return 0; }
    space = maxStack - dst->count;
    moved = src->count;
    if (moved > space) { moved = space; }
    dst->count += moved;
    src->count -= moved;
    if (src->count <= 0) { src->item = ITEM_NONE; src->count = 0; src->damage = 0; }
    return moved;
}

int GuiV36_MoveStackToSlotArrayRole(InventorySlot *src, InventorySlot *dst, int dstCount, int roleBase)
{
    int i;
    int movedAny;
    if (!src || !dst) { return 0; }
    if (src->item == ITEM_NONE || src->count <= 0) { return 0; }
    movedAny = 0;
    for (i = 0; i < dstCount; i++) {
        if (dst[i].item == src->item && dst[i].damage == src->damage && dst[i].count > 0) {
            if (GuiV36_MoveStackToSlotRole(src, &dst[i], roleBase, i) > 0) { movedAny = 1; }
            if (src->item == ITEM_NONE || src->count <= 0) { break; }
        }
    }
    for (i = 0; i < dstCount; i++) {
        if (dst[i].item == ITEM_NONE || dst[i].count <= 0) {
            if (GuiV36_MoveStackToSlotRole(src, &dst[i], roleBase, i) > 0) { movedAny = 1; }
            if (src->item == ITEM_NONE || src->count <= 0) { break; }
        }
    }
    if (movedAny) { PlayItemPickupSound(); }
    return movedAny;
}

int FurnaceV36_IsFurnaceBlock(int block)
{
    return (block == BLOCK_FURNACE || block == BLOCK_FURNACE_LIT || block == BLOCK_LIT_FURNACE);
}

int ItemV21_IsArmorForSlot(int item, int armorSlot)
{
    if (armorSlot == 0) { return item == ITEM_LEATHER_HELMET || item == ITEM_CHAIN_HELMET || item == ITEM_IRON_HELMET || item == ITEM_DIAMOND_HELMET || item == ITEM_GOLD_HELMET; }
    if (armorSlot == 1) { return item == ITEM_LEATHER_CHESTPLATE || item == ITEM_CHAIN_CHESTPLATE || item == ITEM_IRON_CHESTPLATE || item == ITEM_DIAMOND_CHESTPLATE || item == ITEM_GOLD_CHESTPLATE; }
    if (armorSlot == 2) { return item == ITEM_LEATHER_LEGGINGS || item == ITEM_CHAIN_LEGGINGS || item == ITEM_IRON_LEGGINGS || item == ITEM_DIAMOND_LEGGINGS || item == ITEM_GOLD_LEGGINGS; }
    if (armorSlot == 3) { return item == ITEM_LEATHER_BOOTS || item == ITEM_CHAIN_BOOTS || item == ITEM_IRON_BOOTS || item == ITEM_DIAMOND_BOOTS || item == ITEM_GOLD_BOOTS; }
    return 0;
}

InventorySlot *GuiV21_GetPlayerSlotByKind(int index, int kind)
{
    if (kind == GUIV21_SLOT_HOTBAR) { if (index < 0 || index >= HOTBAR_SLOTS) { return NULL; } return &hotbar[index]; }
    if (kind == GUIV21_SLOT_ARMOR) { if (index < 0 || index >= 4) { return NULL; } return &g_armorSlotsV6[index]; }
    if (index < 0 || index >= INVENTORY_SLOTS) { return NULL; }
    return &inventory[index];
}

int GuiV21_CanPlaceInSlot(int role, int index, int item)
{
    if (item == ITEM_NONE) { return 1; }
    if (role == GUIV21_SLOT_CRAFT_RESULT || role == GUIV21_SLOT_FURNACE_OUTPUT) { return 0; }
    if (role == GUIV21_SLOT_ARMOR) { return ItemV21_IsArmorForSlot(item, index); }
    if (role == GUIV21_SLOT_FURNACE_INPUT) { return FurnaceV36_CanSmeltInput(item); }
    if (role == GUIV21_SLOT_FURNACE_FUEL) { return FurnaceV36_FuelBurnTicks(item) > 0; }
    return 1;
}

int GuiV21_AddStackToSlot(InventorySlot *slot, InventorySlot *src)
{
    int maxStack;
    int space;
    int moved;
    if (!slot || !src) { return 0; }
    if (src->item == ITEM_NONE || src->count <= 0) { return 0; }
    maxStack = ItemV21_MaxStackSize(src->item);
    if (maxStack <= 0) { return 0; }
    if (slot->item == ITEM_NONE || slot->count <= 0) {
        moved = src->count;
        if (moved > maxStack) { moved = maxStack; }
        slot->item = src->item;
        slot->count = moved;
        slot->damage = src->damage;
        src->count -= moved;
        if (src->count <= 0) { src->item = ITEM_NONE; src->count = 0; src->damage = 0; }
        return moved;
    }
    if (slot->item != src->item || slot->damage != src->damage) { return 0; }
    if (slot->count >= maxStack) { return 0; }
    space = maxStack - slot->count;
    moved = src->count;
    if (moved > space) { moved = space; }
    slot->count += moved;
    src->count -= moved;
    if (src->count <= 0) { src->item = ITEM_NONE; src->count = 0; src->damage = 0; }
    return moved;
}

int GuiV21_MoveStackToValidatedSlots(InventorySlot *src, InventorySlot *dst, int dstCount, int roleBase)
{
    int i;
    int movedAny;
    if (!src || !dst) { return 0; }
    movedAny = 0;
    for (i = 0; i < dstCount; i++) {
        if (!GuiV21_CanPlaceInSlot(roleBase, i, src->item)) { continue; }
        if (dst[i].item == src->item && dst[i].damage == src->damage && dst[i].count > 0) {
            if (GuiV21_AddStackToSlot(&dst[i], src) > 0) { movedAny = 1; }
            if (src->item == ITEM_NONE || src->count <= 0) { break; }
        }
    }
    for (i = 0; i < dstCount; i++) {
        if (!GuiV21_CanPlaceInSlot(roleBase, i, src->item)) { continue; }
        if (dst[i].item == ITEM_NONE || dst[i].count <= 0) {
            if (GuiV21_AddStackToSlot(&dst[i], src) > 0) { movedAny = 1; }
            if (src->item == ITEM_NONE || src->count <= 0) { break; }
        }
    }
    if (movedAny) { PlayItemPickupSound(); }
    return movedAny;
}

int GuiV21_MoveStackToPlayerAll(InventorySlot *src)
{
    int moved;
    moved = 0;
    if (GuiV21_MoveStackToValidatedSlots(src, hotbar, HOTBAR_SLOTS, GUIV21_SLOT_HOTBAR)) { moved = 1; }
    if (src->item != ITEM_NONE && src->count > 0) { if (GuiV21_MoveStackToValidatedSlots(src, inventory, INVENTORY_SLOTS, GUIV21_SLOT_PLAYER_INV)) { moved = 1; } }
    return moved;
}

int GuiV21_MoveStackToArmorThenPlayer(InventorySlot *src)
{
    int i;
    if (!src) { return 0; }
    for (i = 0; i < 4; i++) {
        if (GuiV21_CanPlaceInSlot(GUIV21_SLOT_ARMOR, i, src->item) && (g_armorSlotsV6[i].item == ITEM_NONE || g_armorSlotsV6[i].count <= 0)) {
            if (GuiV21_AddStackToSlot(&g_armorSlotsV6[i], src) > 0) { return 1; }
        }
    }
    return 0;
}

int GuiV21_SlotClick(InventorySlot *slot, int role, int index, int rightClick)
{
    InventorySlot temp;
    int take;
    int maxStack;
    int moved;
    if (!slot) { return 0; }
    if (!rightClick) {
        if (!g_draggingInventory || g_dragSlot.item == ITEM_NONE || g_dragSlot.count <= 0) {
            if (slot->item != ITEM_NONE && slot->count > 0) { g_dragSlot = *slot; slot->item = ITEM_NONE; slot->count = 0; slot->damage = 0; g_draggingInventory = 1; return 1; }
            return 0;
        }
        if (!GuiV21_CanPlaceInSlot(role, index, g_dragSlot.item)) { return 0; }
        if (slot->item == ITEM_NONE || slot->count <= 0 || (slot->item == g_dragSlot.item && slot->damage == g_dragSlot.damage)) {
            moved = GuiV21_AddStackToSlot(slot, &g_dragSlot);
            if (g_dragSlot.item == ITEM_NONE || g_dragSlot.count <= 0) { g_draggingInventory = 0; }
            return moved > 0;
        }
        temp = *slot;
        *slot = g_dragSlot;
        g_dragSlot = temp;
        g_draggingInventory = 1;
        return 1;
    }
    if (!g_draggingInventory || g_dragSlot.item == ITEM_NONE || g_dragSlot.count <= 0) {
        if (slot->item != ITEM_NONE && slot->count > 0) {
            take = (slot->count + 1) / 2;
            g_dragSlot.item = slot->item;
            g_dragSlot.count = take;
            g_dragSlot.damage = slot->damage;
            slot->count -= take;
            if (slot->count <= 0) { slot->item = ITEM_NONE; slot->count = 0; slot->damage = 0; }
            g_draggingInventory = 1;
            return 1;
        }
        return 0;
    }
    if (!GuiV21_CanPlaceInSlot(role, index, g_dragSlot.item)) { return 0; }
    maxStack = ItemV21_MaxStackSize(g_dragSlot.item);
    if (slot->item == ITEM_NONE || slot->count <= 0) { slot->item = g_dragSlot.item; slot->count = 1; slot->damage = g_dragSlot.damage; g_dragSlot.count--; }
    else if (slot->item == g_dragSlot.item && slot->damage == g_dragSlot.damage && slot->count < maxStack) { slot->count++; g_dragSlot.count--; }
    else { return 0; }
    if (g_dragSlot.count <= 0) { g_dragSlot.item = ITEM_NONE; g_dragSlot.count = 0; g_dragSlot.damage = 0; g_draggingInventory = 0; }
    return 1;
}

int TileSlotMaxV5(int block)
{
    if (block == BLOCK_FURNACE || block == BLOCK_FURNACE_LIT || block == BLOCK_LIT_FURNACE) { return 3; }
    if (block == BLOCK_DISPENSER) { return 9; }
    return 27;
}

int GetContainerSlotAtPointV5(int mx, int my, int *slotType)
{
    int panelW;
    int panelH;
    int panelX;
    int panelY;
    int scale;
    int slotSize;
    int row;
    int col;
    int x;
    int y;
    scale = GuiV16_GetInventoryScale();
    panelW = 176 * scale;
    panelH = 166 * scale;
    panelX = g_windowWidth / 2 - panelW / 2;
    panelY = g_windowHeight / 2 - panelH / 2;
    slotSize = 18 * scale;
    if (slotType) { *slotType = -1; }
    if (g_containerModeV5 == CONTAINER_FURNACE_V5) {
        x = panelX + 56 * scale; y = panelY + 17 * scale;
        if (mx >= x && mx < x + slotSize && my >= y && my < y + slotSize) { if (slotType) { *slotType = 0; } return 0; }
        x = panelX + 56 * scale; y = panelY + 53 * scale;
        if (mx >= x && mx < x + slotSize && my >= y && my < y + slotSize) { if (slotType) { *slotType = 0; } return 1; }
        x = panelX + 116 * scale; y = panelY + 35 * scale;
        if (mx >= x && mx < x + slotSize && my >= y && my < y + slotSize) { if (slotType) { *slotType = 1; } return 2; }
    } else {
        for (row = 0; row < 3; row++) {
            for (col = 0; col < 9; col++) {
                x = panelX + (8 + col * 18) * scale;
                y = panelY + (18 + row * 18) * scale;
                if (mx >= x && mx < x + slotSize && my >= y && my < y + slotSize) { if (slotType) { *slotType = 0; } return row * 9 + col; }
            }
        }
    }
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 9; col++) {
            x = panelX + (8 + col * 18) * scale;
            y = panelY + (84 + row * 18) * scale;
            if (mx >= x && mx < x + slotSize && my >= y && my < y + slotSize) { if (slotType) { *slotType = 2; } return row * 9 + col; }
        }
    }
    for (col = 0; col < HOTBAR_SLOTS; col++) {
        x = panelX + (8 + col * 18) * scale;
        y = panelY + 142 * scale;
        if (mx >= x && mx < x + slotSize && my >= y && my < y + slotSize) { if (slotType) { *slotType = 3; } return col; }
    }
    return -1;
}

int FurnaceRecipeOutputV5(int item)
{
    if (item == ITEM_IRON_ORE || item == BLOCK_IRON_ORE) { return ITEM_IRON_INGOT; }
    if (item == ITEM_GOLD_ORE || item == BLOCK_GOLD_ORE) { return ITEM_GOLD_INGOT; }
    if (item == ITEM_SAND || item == BLOCK_SAND) { return ITEM_GLASS; }
    if (item == ITEM_COBBLESTONE || item == BLOCK_COBBLESTONE) { return ITEM_STONE; }
    if (item == ITEM_PORK_RAW) { return ITEM_PORK_COOKED; }
    if (item == ITEM_CHICKEN_RAW) { return ITEM_CHICKEN_COOKED; }
    if (item == ITEM_FISH_RAW) { return ITEM_FISH_COOKED; }
    if (item == ITEM_CLAY_BALL) { return ITEM_BRICK; }
    if (item == ITEM_CACTUS || item == BLOCK_CACTUS) { return ITEM_DYE_POWDER; }
    if (item == ITEM_WOOD || item == BLOCK_WOOD) { return ITEM_COAL; } /* charcoal via damage=1 */
    return ITEM_NONE;
}

int FurnaceRecipeOutputDamageV21(int item)
{
    if (item == ITEM_CACTUS || item == BLOCK_CACTUS) { return 2; } /* cactus green dye */
    if (item == ITEM_WOOD || item == BLOCK_WOOD) { return 1; }     /* charcoal */
    return 0;
}

int FurnaceV36_CanSmeltInput(int item)
{
    return FurnaceRecipeOutputV5(item) != ITEM_NONE;
}

int FurnaceV36_FuelBurnTicks(int item)
{
    if (item == ITEM_NONE) { return 0; }
    if (item == ITEM_LAVA_BUCKET) { return 20000; }
    if (item == ITEM_COAL) { return 1600; }
    if (item == ITEM_WOOD || item == ITEM_PLANKS || item == ITEM_STICK) { return 300; }
    if (item == ITEM_FENCE || item == ITEM_TRAPDOOR || item == ITEM_WOOD_DOOR || item == ITEM_SIGN || item == ITEM_LADDER || item == ITEM_CHEST || item == ITEM_WORKBENCH || item == ITEM_BOOKSHELF || item == ITEM_NOTE_BLOCK || item == ITEM_JUKEBOX) { return 300; }
    if (item == ITEM_WOOD_SWORD || item == ITEM_WOOD_SHOVEL || item == ITEM_WOOD_PICKAXE || item == ITEM_WOOD_AXE || item == ITEM_WOOD_HOE) { return 200; }
    if (item == ITEM_BOW) { return 300; }
    return JavaCompat_GetFuelBurnTicks(item);
}

void FurnaceV36_ConsumeFuel(TileEntityState *t)
{
    int fuel;
    if (!t) { return; }
    fuel = t->slots[1].item;
    t->slots[1].count--;
    if (t->slots[1].count <= 0) {
        if (fuel == ITEM_LAVA_BUCKET) {
            t->slots[1].item = ITEM_BUCKET;
            t->slots[1].count = 1;
            t->slots[1].damage = 0;
        } else {
            t->slots[1].item = ITEM_NONE;
            t->slots[1].count = 0;
            t->slots[1].damage = 0;
        }
    }
}


int FurnaceV21_OutputCanAccept(TileEntityState *t, int outItem, int outDamage)
{
    int maxStack;
    if (!t || outItem == ITEM_NONE) { return 0; }
    maxStack = ItemV21_MaxStackSize(outItem);
    if (t->slots[2].item == ITEM_NONE || t->slots[2].count <= 0) { return 1; }
    if (t->slots[2].item != outItem || t->slots[2].damage != outDamage) { return 0; }
    return t->slots[2].count < maxStack;
}

void FurnaceV21_FinishSmelt(TileEntityState *t, int outItem, int outDamage)
{
    if (!t || outItem == ITEM_NONE) { return; }
    if (t->slots[2].item == ITEM_NONE || t->slots[2].count <= 0) { t->slots[2].item = outItem; t->slots[2].count = 1; t->slots[2].damage = outDamage; }
    else if (t->slots[2].item == outItem && t->slots[2].damage == outDamage && t->slots[2].count < ItemV21_MaxStackSize(outItem)) { t->slots[2].count++; }
    t->slots[0].count--;
    if (t->slots[0].count <= 0) { t->slots[0].item = ITEM_NONE; t->slots[0].count = 0; t->slots[0].damage = 0; }
}

void FurnaceV21_UpdateOne(TileEntityState *t, double dt)
{
    int outItem;
    int outDamage;
    int fuelTicks;
    double burnSeconds;
    int wasLit;
    int shouldLit;
    if (!t) { return; }
    wasLit = FurnaceV36_IsFurnaceBlock(GetBlock(t->x, t->y, t->z)) && (GetBlock(t->x, t->y, t->z) == BLOCK_FURNACE_LIT || GetBlock(t->x, t->y, t->z) == BLOCK_LIT_FURNACE);
    outItem = FurnaceRecipeOutputV5(t->slots[0].item);
    outDamage = FurnaceRecipeOutputDamageV21(t->slots[0].item);
    if (t->burnTime > 0.0) { t->burnTime -= dt; if (t->burnTime < 0.0) { t->burnTime = 0.0; } }
    if (outItem != ITEM_NONE && FurnaceV21_OutputCanAccept(t, outItem, outDamage)) {
        if (t->burnTime <= 0.0 && t->slots[1].item != ITEM_NONE && t->slots[1].count > 0) {
            fuelTicks = FurnaceV36_FuelBurnTicks(t->slots[1].item);
            if (fuelTicks > 0) {
                burnSeconds = (double)fuelTicks / 20.0;
                if (burnSeconds < 0.10) { burnSeconds = 0.10; }
                t->burnTime = burnSeconds;
                t->power = fuelTicks;
                FurnaceV36_ConsumeFuel(t);
            }
        }
        if (t->burnTime > 0.0) { t->cookTime += dt; if (t->cookTime >= 10.0) { t->cookTime = 0.0; FurnaceV21_FinishSmelt(t, outItem, outDamage); } }
        else { t->cookTime = 0.0; }
    } else { t->cookTime = 0.0; }
    shouldLit = t->burnTime > 0.0 ? 1 : 0;
    if (shouldLit && !wasLit) { SetBlock(t->x, t->y, t->z, BLOCK_FURNACE_LIT); PlayFurnaceSoundV35((double)t->x + 0.5, (double)t->y + 0.5, (double)t->z + 0.5); }
    if (!shouldLit && wasLit) { SetBlock(t->x, t->y, t->z, BLOCK_FURNACE); PlaySoundAtV35("assets\\sounds\\random\\fizz.wav", (double)t->x + 0.5, (double)t->y + 0.5, (double)t->z + 0.5, 0.18, SoundRandomRangeV35(0.55, 0.75), SOUND_DEFAULT_RANGE_V35); }
}

int DispenserV36_IsPowerSourceBlock(int block, int meta)
{
    if (block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_GLOWSTONE || block == BLOCK_JACK_O_LANTERN) { return 1; }
    if (block == BLOCK_REPEATER_ON) { return 1; }
    if ((block == BLOCK_LEVER || block == BLOCK_STONE_BUTTON || block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE || block == BLOCK_DETECTOR_RAIL) && (meta & 8)) { return 1; }
    if (block == BLOCK_REDSTONE_WIRE) { return 1; }
    return 0;
}

int DispenserV36_IsPowered(TileEntityState *t)
{
    int dirs[6][3];
    int i;
    int x;
    int y;
    int z;
    int b;
    dirs[0][0]=1; dirs[0][1]=0; dirs[0][2]=0;
    dirs[1][0]=-1; dirs[1][1]=0; dirs[1][2]=0;
    dirs[2][0]=0; dirs[2][1]=1; dirs[2][2]=0;
    dirs[3][0]=0; dirs[3][1]=-1; dirs[3][2]=0;
    dirs[4][0]=0; dirs[4][1]=0; dirs[4][2]=1;
    dirs[5][0]=0; dirs[5][1]=0; dirs[5][2]=-1;
    if (!t) { return 0; }
    for (i = 0; i < 6; i++) {
        x = t->x + dirs[i][0]; y = t->y + dirs[i][1]; z = t->z + dirs[i][2];
        if (!IsInsideWorld(x, y, z)) { continue; }
        b = GetBlock(x, y, z);
        if (DispenserV36_IsPowerSourceBlock(b, (int)g_blockMeta[x][y][z])) { return 1; }
    }
    return 0;
}

int DispenserV36_PickSlot(TileEntityState *t)
{
    int available[9];
    int count;
    int i;
    int pick;
    count = 0;
    if (!t) { return -1; }
    for (i = 0; i < 9; i++) {
        if (t->slots[i].item != ITEM_NONE && t->slots[i].count > 0) { available[count++] = i; }
    }
    if (count <= 0) { return -1; }
    pick = (int)((GetTickCount() ^ WorldHash3D(t->x, t->y, t->z, g_worldSeed + t->slots[available[0]].item)) % count);
    if (pick < 0) { pick = 0; }
    return available[pick];
}

void DispenserV36_GetFacing(TileEntityState *t, double *dx, double *dy, double *dz)
{
    int meta;
    meta = 3;
    if (t && IsInsideWorld(t->x, t->y, t->z)) { meta = (int)g_blockMeta[t->x][t->y][t->z] & 7; }
    *dx = 0.0; *dy = 0.0; *dz = 1.0;
    if (meta == 2) { *dz = -1.0; }
    else if (meta == 3) { *dz = 1.0; }
    else if (meta == 4) { *dx = -1.0; *dz = 0.0; }
    else if (meta == 5) { *dx = 1.0; *dz = 0.0; }
}

void DispenserV36_Dispense(TileEntityState *t)
{
    int slot;
    int item;
    double dx;
    double dy;
    double dz;
    double sx;
    double sy;
    double sz;
    double speed;
    int ent;
    if (!t) { return; }
    slot = DispenserV36_PickSlot(t);
    if (slot < 0) { PlaySoundAtV35("assets\\sounds\\random\\click.wav", (double)t->x + 0.5, (double)t->y + 0.5, (double)t->z + 0.5, 0.22, 1.0, SOUND_DEFAULT_RANGE_V35); return; }
    item = t->slots[slot].item;
    DispenserV36_GetFacing(t, &dx, &dy, &dz);
    sx = (double)t->x + 0.5 + dx * 0.62;
    sy = (double)t->y + 0.62;
    sz = (double)t->z + 0.5 + dz * 0.62;
    speed = 10.0;
    if (item == ITEM_ARROW) { ent = SpawnSpecialEntityV6(ENTITY_V6_ARROW, ITEM_ARROW, 0, sx, sy, sz, dx * speed, 0.06, dz * speed, 0.0); if (ent >= 0) { g_specialEntitiesV6[ent].damage = 3; } PlayBowSoundV35(sx, sy, sz); }
    else if (item == ITEM_EGG) { SpawnSpecialEntityV6(ENTITY_V6_EGG, ITEM_EGG, 0, sx, sy, sz, dx * 8.5, 0.10, dz * 8.5, 0.0); PlaySoundAtV35("assets\\sounds\\random\\bow.wav", sx, sy, sz, 0.35, SoundRandomRangeV35(0.8, 1.2), SOUND_DEFAULT_RANGE_V35); }
    else if (item == ITEM_SNOWBALL) { SpawnSpecialEntityV6(ENTITY_V6_SNOWBALL, ITEM_SNOWBALL, 0, sx, sy, sz, dx * 9.0, 0.10, dz * 9.0, 0.0); PlaySoundAtV35("assets\\sounds\\random\\bow.wav", sx, sy, sz, 0.35, SoundRandomRangeV35(0.8, 1.2), SOUND_DEFAULT_RANGE_V35); }
    else if (item == ITEM_TNT) { SpawnSpecialEntityV6(ENTITY_V6_TNT, ITEM_TNT, BLOCK_TNT, sx, sy, sz, dx * 2.0, 0.20, dz * 2.0, 4.0); PlaySoundAtV35("assets\\sounds\\random\\fuse.wav", sx, sy, sz, 0.45, 1.0, SOUND_DEFAULT_RANGE_V35); }
    else { AddDroppedItem(item, 1, sx, sy, sz, dx * 2.4, 0.18, dz * 2.4); PlayItemPickupSound(); }
    t->slots[slot].count--;
    if (t->slots[slot].count <= 0) { t->slots[slot].item = ITEM_NONE; t->slots[slot].count = 0; t->slots[slot].damage = 0; }
}

void DispenserV36_UpdateOne(TileEntityState *t, double dt)
{
    int powered;
    if (!t) { return; }
    if (t->cookTime > 0.0) { t->cookTime -= dt; if (t->cookTime < 0.0) { t->cookTime = 0.0; } }
    powered = DispenserV36_IsPowered(t);
    if (powered && !t->power && t->cookTime <= 0.0) {
        DispenserV36_Dispense(t);
        t->cookTime = 0.20;
    }
    t->power = powered ? 1 : 0;
}

void TileEntitiesRedstoneV5_Update(double dt)
{
    int i;
    TileEntityState *t;
    for (i = 0; i < MAX_TILE_ENTITIES; i++) {
        if (!tileEntities[i].active) { continue; }
        t = &tileEntities[i];
        if (FurnaceV36_IsFurnaceBlock(t->type)) { FurnaceV21_UpdateOne(t, dt); }
        else if (t->type == BLOCK_DISPENSER) { DispenserV36_UpdateOne(t, dt); }
    }
}

const char *ItemV21_GetDisplayName(int item)
{
    if (item == ITEM_NONE) { return "Empty"; }
    if (item == ITEM_STONE) { return "Stone"; }
    if (item == ITEM_GRASS) { return "Grass Block"; }
    if (item == ITEM_DIRT) { return "Dirt"; }
    if (item == ITEM_COBBLESTONE) { return "Cobblestone"; }
    if (item == ITEM_PLANKS) { return "Wooden Planks"; }
    if (item == ITEM_WOOD) { return "Wood"; }
    if (item == ITEM_LEAVES) { return "Leaves"; }
    if (item == ITEM_SAND) { return "Sand"; }
    if (item == ITEM_GRAVEL) { return "Gravel"; }
    if (item == ITEM_GLASS) { return "Glass"; }
    if (item == ITEM_COAL) { return "Coal"; }
    if (item == ITEM_IRON_INGOT) { return "Iron Ingot"; }
    if (item == ITEM_GOLD_INGOT) { return "Gold Ingot"; }
    if (item == ITEM_DIAMOND) { return "Diamond"; }
    if (item == ITEM_STICK) { return "Stick"; }
    if (item == ITEM_TORCH) { return "Torch"; }
    if (item == ITEM_WORKBENCH) { return "Crafting Table"; }
    if (item == ITEM_FURNACE) { return "Furnace"; }
    if (item == ITEM_CHEST) { return "Chest"; }
    if (item == ITEM_BREAD) { return "Bread"; }
    if (item == ITEM_APPLE) { return "Apple"; }
    if (item == ITEM_PORK_RAW) { return "Raw Porkchop"; }
    if (item == ITEM_PORK_COOKED) { return "Cooked Porkchop"; }
    if (item == ITEM_CHICKEN_RAW) { return "Raw Chicken"; }
    if (item == ITEM_CHICKEN_COOKED) { return "Cooked Chicken"; }
    if (item == ITEM_FISH_RAW) { return "Raw Fish"; }
    if (item == ITEM_FISH_COOKED) { return "Cooked Fish"; }
    if (ItemV21_IsArmorForSlot(item, 0)) { return "Helmet"; }
    if (ItemV21_IsArmorForSlot(item, 1)) { return "Chestplate"; }
    if (ItemV21_IsArmorForSlot(item, 2)) { return "Leggings"; }
    if (ItemV21_IsArmorForSlot(item, 3)) { return "Boots"; }
    if (GetItemMaxDurability(item) > 0) { return "Tool"; }
    return "Item";
}

void TileContainerMouseClickV5(int mx, int my, int rightClick)
{
    int type;
    int index;
    int role;
    InventorySlot *slot;
    TileEntityState *t;

    if (g_activeTileIndexV5 < 0 || g_activeTileIndexV5 >= MAX_TILE_ENTITIES) { return; }
    if (g_containerModeV5 == CONTAINER_SIGN_V5 || g_containerModeV5 == CONTAINER_NOTE_V5) { return; }
    t = &tileEntities[g_activeTileIndexV5];
    index = GetContainerSlotAtPointV5(mx, my, &type);
    if (index < 0) { DropCarriedInventoryStackToWorld(rightClick ? 1 : 0); return; }
    if (type == 0 || type == 1) {
        if (index < 0 || index >= TileSlotMaxV5(t->type)) { return; }
        slot = &t->slots[index];
        role = GUIV21_SLOT_TILE_NORMAL;
        if (FurnaceV36_IsFurnaceBlock(t->type)) {
            if (index == 0) { role = GUIV21_SLOT_FURNACE_INPUT; }
            else if (index == 1) { role = GUIV21_SLOT_FURNACE_FUEL; }
            else { role = GUIV21_SLOT_FURNACE_OUTPUT; }
        }
        if (GuiV16_IsShiftDown() && !g_draggingInventory) { GuiV21_MoveStackToPlayerAll(slot); return; }
        if (role == GUIV21_SLOT_FURNACE_OUTPUT || type == 1) { GuiV16_ClickTakeOnlySlot(slot, rightClick); return; }
        GuiV21_SlotClick(slot, role, index, rightClick);
        return;
    }
    if (type == 2) {
        if (index >= 0 && index < INVENTORY_SLOTS) {
            slot = &inventory[index];
            if (GuiV16_IsShiftDown() && !g_draggingInventory) { GuiV16_MoveStackToTileContainer(slot, t); return; }
            GuiV21_SlotClick(slot, GUIV21_SLOT_PLAYER_INV, index, rightClick);
        }
        return;
    }
    if (type == 3) {
        if (index >= 0 && index < HOTBAR_SLOTS) {
            slot = &hotbar[index];
            if (GuiV16_IsShiftDown() && !g_draggingInventory) { GuiV16_MoveStackToTileContainer(slot, t); return; }
            GuiV21_SlotClick(slot, GUIV21_SLOT_HOTBAR, index, rightClick);
        }
        return;
    }
}

void DrawContainerProgressV5(TileEntityState *t, int panelX, int panelY, int scale)
{
    int burnH;
    int cookW;
    double totalBurn;
    if (!t) { return; }
    totalBurn = 80.0;
    if (t->power > 0 && FurnaceV36_IsFurnaceBlock(t->type)) { totalBurn = (double)t->power / 20.0; }
    if (totalBurn < 0.10) { totalBurn = 0.10; }
    burnH = (int)(14.0 * (t->burnTime / totalBurn));
    if (burnH < 0) { burnH = 0; } if (burnH > 14) { burnH = 14; }
    cookW = (int)(24.0 * (t->cookTime / 10.0));
    if (cookW < 0) { cookW = 0; } if (cookW > 24) { cookW = 24; }
    if (t->burnTime > 0.0) { DrawRect2D(panelX + 57 * scale, panelY + (51 - burnH) * scale, panelX + 70 * scale, panelY + 64 * scale, 1.0f, 0.45f, 0.10f); }
    if (t->cookTime > 0.0) { DrawRect2D(panelX + 79 * scale, panelY + 35 * scale, panelX + (79 + cookW) * scale, panelY + 51 * scale, 0.75f, 0.75f, 0.75f); }
}

void SignV36_DrawText(TileEntityState *t, int x1, int y1, int x2, int y2)
{
    char line[5][20];
    char c;
    int src;
    int dst;
    int ln;
    int i;
    int y;
    if (!t) { return; }
    for (ln = 0; ln < 4; ln++) { line[ln][0] = 0; }
    src = 0; ln = 0; dst = 0;
    while (t->text[src] != 0 && ln < 4) {
        c = t->text[src++];
        if (c == '|') { line[ln][dst] = 0; ln++; dst = 0; continue; }
        if (dst >= 15) { line[ln][dst] = 0; ln++; dst = 0; if (ln >= 4) { break; } }
        line[ln][dst++] = c;
        line[ln][dst] = 0;
    }
    for (i = 0; i < 4; i++) {
        y = y1 + 6 + i * 13;
        if (line[i][0]) { DrawCenteredText2D(fontBaseNormal, x1, y, x2, y + 14, line[i]); }
    }
}

void HandleSignKeyV36(WPARAM key)
{
    TileEntityState *t;
    int len;
    int i;
    if (!IsSignEditingV5()) { return; }
    t = &tileEntities[g_activeTileIndexV5];
    len = (int)strlen(t->text);
    if (g_signEditCursorV5 < 0) { g_signEditCursorV5 = 0; }
    if (g_signEditCursorV5 > len) { g_signEditCursorV5 = len; }
    if (key == VK_LEFT && g_signEditCursorV5 > 0) { g_signEditCursorV5--; return; }
    if (key == VK_RIGHT && g_signEditCursorV5 < len) { g_signEditCursorV5++; return; }
    if (key == VK_HOME) { g_signEditCursorV5 = 0; return; }
    if (key == VK_END) { g_signEditCursorV5 = len; return; }
    if (key == VK_DELETE && g_signEditCursorV5 < len) { for (i = g_signEditCursorV5; i < len; i++) { t->text[i] = t->text[i + 1]; } return; }
}

void DrawActiveContainerV5(void)
{
    int panelX;
    int panelY;
    int panelW;
    int panelH;
    int scale;
    int row;
    int col;
    int idx;
    int x;
    int y;
    POINT mouse;
    TileEntityState *t;
    if (g_containerModeV5 == CONTAINER_NONE_V5 || g_activeTileIndexV5 < 0) { return; }
    t = &tileEntities[g_activeTileIndexV5];
    GetCursorPos(&mouse); ScreenToClient(g_hwnd, &mouse);
    scale = GuiV16_GetInventoryScale();
    panelW = 176 * scale;
    panelH = 166 * scale;
    panelX = g_windowWidth / 2 - panelW / 2;
    panelY = g_windowHeight / 2 - panelH / 2;
    DrawRect2D(panelX, panelY, panelX + panelW, panelY + panelH, 0.18f, 0.18f, 0.18f);
    DrawRect2D(panelX + 6, panelY + 6, panelX + panelW - 6, panelY + panelH - 6, 0.35f, 0.35f, 0.35f);
    glColor3f(1.0f, 1.0f, 1.0f);
    if (g_containerModeV5 == CONTAINER_CHEST_V5) { DrawCenteredText2D(fontBaseNormal, panelX, panelY + 6, panelX + panelW, panelY + 26, "Chest"); }
    else if (g_containerModeV5 == CONTAINER_DISPENSER_V5) { DrawCenteredText2D(fontBaseNormal, panelX, panelY + 6, panelX + panelW, panelY + 26, "Dispenser"); }
    else if (g_containerModeV5 == CONTAINER_FURNACE_V5) { DrawCenteredText2D(fontBaseNormal, panelX, panelY + 6, panelX + panelW, panelY + 26, "Furnace"); }
    else if (g_containerModeV5 == CONTAINER_SIGN_V5) {
        DrawCenteredText2D(fontBaseNormal, panelX, panelY + 18, panelX + panelW, panelY + 44, "Edit sign text, Enter/Esc closes");
        DrawRect2D(panelX + 24, panelY + 74, panelX + panelW - 24, panelY + 112, 0.10f, 0.08f, 0.04f);
        SignV36_DrawText(t, panelX + 24, panelY + 76, panelX + panelW - 24, panelY + 112);
        return;
    }
    else if (g_containerModeV5 == CONTAINER_NOTE_V5) { DrawCenteredText2D(fontBaseNormal, panelX, panelY + 40, panelX + panelW, panelY + 70, "Note block pitch changed"); return; }

    if (g_containerModeV5 == CONTAINER_FURNACE_V5) {
        x = panelX + 56 * scale; y = panelY + 17 * scale; DrawSlotHoverFrame(x, y, mouse.x, mouse.y); DrawCenteredItemStack(x, y, t->slots[0], 0);
        x = panelX + 56 * scale; y = panelY + 53 * scale; DrawSlotHoverFrame(x, y, mouse.x, mouse.y); DrawCenteredItemStack(x, y, t->slots[1], 0);
        x = panelX + 116 * scale; y = panelY + 35 * scale; DrawSlotHoverFrame(x, y, mouse.x, mouse.y); DrawCenteredItemStack(x, y, t->slots[2], 0);
        DrawContainerProgressV5(t, panelX, panelY, scale);
    } else {
        for (row = 0; row < 3; row++) {
            for (col = 0; col < 9; col++) {
                if (g_containerModeV5 == CONTAINER_DISPENSER_V5 && row >= 1) { continue; }
                idx = row * 9 + col;
                x = panelX + (8 + col * 18) * scale;
                y = panelY + (18 + row * 18) * scale;
                DrawSlotHoverFrame(x, y, mouse.x, mouse.y);
                DrawCenteredItemStack(x, y, t->slots[idx], 0);
            }
        }
    }
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

int IsSolidBlockAtV5(int x, int y, int z)
{
    int block;
    block = GetBlock(x, y, z);
    if ((block == BLOCK_WOOD_DOOR || block == BLOCK_TRAPDOOR) && (g_blockMeta[x][y][z] & 1)) { return 0; }
    return IsSolidBlock(block);
}

int IsSpecialBlockV5(int block)
{
    switch (block) {
    case BLOCK_SAPLING:
    case BLOCK_WEB:
    case BLOCK_TALL_GRASS:
    case BLOCK_DEAD_BUSH:
    case BLOCK_FLOWER_YELLOW:
    case BLOCK_FLOWER_RED:
    case BLOCK_MUSHROOM_BROWN:
    case BLOCK_MUSHROOM_RED:
    case BLOCK_CROPS:
    case BLOCK_REED:
    case BLOCK_FIRE:
    case BLOCK_TORCH:
    case BLOCK_REDSTONE_TORCH_ON:
    case BLOCK_REDSTONE_TORCH_OFF:
    case BLOCK_RAIL:
    case BLOCK_DETECTOR_RAIL:
    case BLOCK_REDSTONE_WIRE:
    case BLOCK_STONE_PRESSURE_PLATE:
    case BLOCK_WOOD_PRESSURE_PLATE:
    case BLOCK_REPEATER_OFF:
    case BLOCK_REPEATER_ON:
    case BLOCK_LEVER:
    case BLOCK_STONE_BUTTON:
    case BLOCK_WOOD_DOOR:
    case BLOCK_LADDER:
    case BLOCK_SIGN_POST:
    case BLOCK_SIGN_WALL:
    case BLOCK_TRAPDOOR:
    case BLOCK_SNOW:
    case BLOCK_CACTUS:
    case BLOCK_FENCE:
    case BLOCK_STEP:
    case BLOCK_FARMLAND:
    case BLOCK_CAKE:
        return 1;
    default:
        return 0;
    }
}


void DrawFlatTopTileV5(int x, int y, int z, int block, float h)
{
    int col;
    int row;
    float u0;
    float v0;
    float u1;
    float v1;
    GetBlockTextureTile(block, 0, &col, &row);
    GetTileUVEx(col, row, TERRAIN_ATLAS_WIDTH, TERRAIN_ATLAS_HEIGHT, &u0, &v0, &u1, &v1);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v0); glVertex3f((float)x, (float)y + h, (float)z);
    glTexCoord2f(u1, v0); glVertex3f((float)x + 1.0f, (float)y + h, (float)z);
    glTexCoord2f(u1, v1); glVertex3f((float)x + 1.0f, (float)y + h, (float)z + 1.0f);
    glTexCoord2f(u0, v1); glVertex3f((float)x, (float)y + h, (float)z + 1.0f);
    glEnd();
}

void DrawSpecialBlockV5(int x, int y, int z, int block)
{
    RenderBlockAtV19(x, y, z, block);
}


void UpdatePistonsV5(void)
{
    int x;
    int y;
    int z;
    int block;
    int dir;
    int dx;
    int dy;
    int dz;
    int front;
    int next;
    for (x = 1; x < WORLD_X - 2; x++) {
        for (y = 1; y < WORLD_Y - 2; y++) {
            for (z = 1; z < WORLD_Z - 2; z++) {
                block = GetBlock(x, y, z);
                if (block != BLOCK_PISTON && block != BLOCK_PISTON_STICKY) { continue; }
                dir = g_blockMeta[x][y][z] & 7;
                dx = 0; dy = 0; dz = 0;
                if (dir == 0) { dx = 1; } else if (dir == 1) { dx = -1; } else if (dir == 2) { dz = 1; } else if (dir == 3) { dz = -1; } else if (dir == 4) { dy = 1; } else { dy = -1; }
                if (g_redstonePower[x][y][z] > 0 && !(g_blockMeta[x][y][z] & 8)) {
                    front = GetBlock(x + dx, y + dy, z + dz);
                    next = GetBlock(x + dx * 2, y + dy * 2, z + dz * 2);
                    if (front != BLOCK_AIR && next == BLOCK_AIR && front != BLOCK_BEDROCK && front != BLOCK_BORDER) {
                        SetBlock(x + dx * 2, y + dy * 2, z + dz * 2, front);
                        SetBlock(x + dx, y + dy, z + dz, BLOCK_PISTON_EXTENSION);
                        g_blockMeta[x][y][z] |= 8;
                    } else if (front == BLOCK_AIR) {
                        SetBlock(x + dx, y + dy, z + dz, BLOCK_PISTON_EXTENSION);
                        g_blockMeta[x][y][z] |= 8;
                    }
                } else if (g_redstonePower[x][y][z] == 0 && (g_blockMeta[x][y][z] & 8)) {
                    if (GetBlock(x + dx, y + dy, z + dz) == BLOCK_PISTON_EXTENSION) { SetBlock(x + dx, y + dy, z + dz, BLOCK_AIR); }
                    g_blockMeta[x][y][z] &= (unsigned char)~8;
                }
            }
        }
    }
}

int SpawnMinecartV5(double x, double y, double z, double vx, double vz)
{
    int i;
    for (i = 0; i < MAX_MINECARTS_V5; i++) {
        if (!g_minecartsV5[i].active) {
            g_minecartsV5[i].active = 1;
            g_minecartsV5[i].x = x; g_minecartsV5[i].y = y; g_minecartsV5[i].z = z;
            g_minecartsV5[i].vx = vx; g_minecartsV5[i].vz = vz;
            return 1;
        }
    }
    return 0;
}

void RailV44_GetDirectionAt(int x, int y, int z, double *dx, double *dz)
{
    int east; int west; int north; int south;
    east = (GetBlock(x + 1, y, z) == BLOCK_RAIL || GetBlock(x + 1, y, z) == BLOCK_DETECTOR_RAIL);
    west = (GetBlock(x - 1, y, z) == BLOCK_RAIL || GetBlock(x - 1, y, z) == BLOCK_DETECTOR_RAIL);
    north = (GetBlock(x, y, z - 1) == BLOCK_RAIL || GetBlock(x, y, z - 1) == BLOCK_DETECTOR_RAIL);
    south = (GetBlock(x, y, z + 1) == BLOCK_RAIL || GetBlock(x, y, z + 1) == BLOCK_DETECTOR_RAIL);
    if ((east || west) && !(north || south)) { *dx = 1.0; *dz = 0.0; return; }
    if ((north || south) && !(east || west)) { *dx = 0.0; *dz = 1.0; return; }
    if (east && south) { *dx = 0.707; *dz = 0.707; return; }
    if (east && north) { *dx = 0.707; *dz = -0.707; return; }
    if (west && south) { *dx = -0.707; *dz = 0.707; return; }
    if (west && north) { *dx = -0.707; *dz = -0.707; return; }
    *dx = 1.0; *dz = 0.0;
}

void UpdateMinecartsV44(double dt)
{
    int i; int bx; int by; int bz; int rail; double rdx; double rdz; double along; double speed; double dxp; double dzp;
    for (i = 0; i < MAX_MINECARTS_V5; i++) {
        if (!g_minecartsV5[i].active) { continue; }
        bx = (int)floor(g_minecartsV5[i].x); by = (int)floor(g_minecartsV5[i].y - 0.25); bz = (int)floor(g_minecartsV5[i].z); rail = GetBlock(bx, by, bz);
        if (rail == BLOCK_RAIL || rail == BLOCK_DETECTOR_RAIL) {
            RailV44_GetDirectionAt(bx, by, bz, &rdx, &rdz);
            along = g_minecartsV5[i].vx * rdx + g_minecartsV5[i].vz * rdz;
            g_minecartsV5[i].vx = rdx * along; g_minecartsV5[i].vz = rdz * along;
            g_minecartsV5[i].x += ((double)bx + 0.5 - g_minecartsV5[i].x) * 0.08; g_minecartsV5[i].z += ((double)bz + 0.5 - g_minecartsV5[i].z) * 0.08;
            g_minecartsV5[i].vx *= (rail == BLOCK_DETECTOR_RAIL && g_redstonePower[bx][by][bz] > 0) ? 1.03 : 0.996; g_minecartsV5[i].vz *= (rail == BLOCK_DETECTOR_RAIL && g_redstonePower[bx][by][bz] > 0) ? 1.03 : 0.996;
            speed = sqrt(g_minecartsV5[i].vx * g_minecartsV5[i].vx + g_minecartsV5[i].vz * g_minecartsV5[i].vz); if (speed > 8.0) { g_minecartsV5[i].vx *= 8.0 / speed; g_minecartsV5[i].vz *= 8.0 / speed; }
            dxp = g_minecartsV5[i].x - playerX; dzp = g_minecartsV5[i].z - playerZ; if (dxp * dxp + dzp * dzp < 1.25) { g_minecartsV5[i].vx += dxp * dt * 2.8; g_minecartsV5[i].vz += dzp * dt * 2.8; }
        } else { g_minecartsV5[i].vz *= 0.94; g_minecartsV5[i].vx *= 0.94; }
        g_minecartsV5[i].x += g_minecartsV5[i].vx * dt; g_minecartsV5[i].z += g_minecartsV5[i].vz * dt;
    }
}

void UpdateMinecartsV5(double dt)
{
    UpdateMinecartsV44(dt);
}

void RenderMinecartsV5(void)
{
    int i;
    for (i = 0; i < MAX_MINECARTS_V5; i++) {
        if (!g_minecartsV5[i].active) { continue; }
        glPushMatrix();
        glTranslatef((float)g_minecartsV5[i].x, (float)g_minecartsV5[i].y + 0.25f, (float)g_minecartsV5[i].z);
        glScalef(0.8f, 0.35f, 0.8f);
        DrawDroppedBlockCube(BLOCK_IRON_BLOCK);
        glPopMatrix();
    }
}

void SaveRedstoneMetaV5(void)
{
    FILE *f;
    char path[128];
    int x;
    int y;
    int z;
    int count;
    unsigned char rec[5];
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    wsprintf(path, "saves\\world%d_redstone_v5.dat", currentWorldSlot);
    f = fopen(path, "wb");
    if (!f) { return; }
    fwrite("CMRSV5", 1, 6, f);
    fwrite(&worldOriginBlockX, sizeof(int), 1, f);
    fwrite(&worldOriginBlockZ, sizeof(int), 1, f);
    count = 0;
    fwrite(&count, sizeof(int), 1, f);
    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                if (g_blockMeta[x][y][z] || g_redstonePower[x][y][z]) { count++; }
            }
        }
    }
    fseek(f, 6 + sizeof(int) * 2, SEEK_SET);
    fwrite(&count, sizeof(int), 1, f);
    fseek(f, 0, SEEK_END);
    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                if (g_blockMeta[x][y][z] || g_redstonePower[x][y][z]) {
                    rec[0] = (unsigned char)x;
                    rec[1] = (unsigned char)y;
                    rec[2] = (unsigned char)z;
                    rec[3] = g_blockMeta[x][y][z];
                    rec[4] = g_redstonePower[x][y][z];
                    fwrite(rec, 1, 5, f);
                }
            }
        }
    }
    fclose(f);
}

int LoadRedstoneMetaV5(void)
{
    FILE *f;
    char path[128];
    char magic[8];
    int ox;
    int oz;
    int count;
    int i;
    unsigned char rec[5];
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    wsprintf(path, "saves\\world%d_redstone_v5.dat", currentWorldSlot);
    f = fopen(path, "rb");
    if (!f) { return 0; }
    ZeroMemory(g_blockMeta, sizeof(g_blockMeta));
    ZeroMemory(g_redstonePower, sizeof(g_redstonePower));
    ZeroMemory(magic, sizeof(magic));
    fread(magic, 1, 6, f);
    if (strcmp(magic, "CMRSV5") != 0) { fclose(f); return 0; }
    fread(&ox, sizeof(int), 1, f);
    fread(&oz, sizeof(int), 1, f);
    fread(&count, sizeof(int), 1, f);
    if (count < 0) { count = 0; }
    if (count > WORLD_X * WORLD_Y * WORLD_Z) { count = WORLD_X * WORLD_Y * WORLD_Z; }
    for (i = 0; i < count; i++) {
        if (fread(rec, 1, 5, f) != 5) { break; }
        if (rec[0] < WORLD_X && rec[1] < WORLD_Y && rec[2] < WORLD_Z) {
            g_blockMeta[rec[0]][rec[1]][rec[2]] = rec[3];
            g_redstonePower[rec[0]][rec[1]][rec[2]] = rec[4];
        }
    }
    fclose(f);
    (void)ox;
    (void)oz;
    return 1;
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
    int x;
    int z;
    int y;
    int count;
    int p;
    int h;
    double px;
    double pz;
    double vx;
    double vz;
    g_weatherParticleAccumV24 += g_rainStrength * (g_videoParticlesV7 ? 10.0 : 3.0) * 0.05;
    count = (int)g_weatherParticleAccumV24;
    if (count < 1 && g_rainStrength > 0.45) { count = 1; }
    if (count > (g_videoParticlesV7 ? 8 : 2)) { count = g_videoParticlesV7 ? 8 : 2; }
    g_weatherParticleAccumV24 -= (double)count;
    if (g_weatherParticleAccumV24 < 0.0) { g_weatherParticleAccumV24 = 0.0; }
    g_weatherWindX_V24 = (float)(sin(g_worldTimeSeconds * 0.07) * 0.22);
    g_weatherWindZ_V24 = (float)(cos(g_worldTimeSeconds * 0.05) * 0.22);
    for (p = 0; p < count; p++) {
        h = WorldHash3D(p, (int)(g_worldTimeSeconds * 20.0), g_worldSeed, 9100 + p);
        x = (int)floor(playerX) - 18 + (h & 31);
        z = (int)floor(playerZ) - 18 + ((h >> 8) & 31);
        y = WORLD_Y - 2;
        while (y > 2 && GetBlock(x, y - 1, z) == BLOCK_AIR) { y--; }
        px = (double)x + 0.15 + (double)((h >> 16) & 127) / 128.0;
        pz = (double)z + 0.15 + (double)((h >> 23) & 127) / 128.0;
        vx = (double)g_weatherWindX_V24;
        vz = (double)g_weatherWindZ_V24;
        if (g_weatherMode == 2) {
            SpawnParticleV24(PARTICLE_V24_SNOW, px, (double)y + 8.0, pz, vx * 0.25, -0.35, vz * 0.25, 3.2, 0.52, BLOCK_SNOW);
        } else {
            SpawnParticleV24(PARTICLE_V24_RAIN, px, (double)y + 7.0, pz, vx * 0.08, -5.5, vz * 0.08, 1.0, 0.36, BLOCK_WATER);
            if ((h & 7) == 0) { SpawnSplashParticlesV24(px, (double)y + 0.02, pz, 1); }
        }
    }
}
void InitFeatureGapSystems(void)
{
    InitRecipeBook();
    JavaCompat_InitFeatureRegistry();
    JavaCompat_LoadLangAndSplashes();
    InitItemCombatV6();
    RendererV8_Init();
}

void UpdateFeatureGapSystems(double dt)
{
    UpdateWeatherAdvanced(dt);
    JavaCompat_UpdateFeatureSystems(dt);
}

/* DrawBiomeTintOverlayForBlock implementation moved into feature-gap block above. */

/* ------------------------------------------------------------ */
/* Java/source/resource completion bridge                       */
/* ------------------------------------------------------------ */

#define JAVA_COMPAT_MAX_LANG 192
#define JAVA_COMPAT_MAX_SPLASHES 96
#define JAVA_COMPAT_TEXT_LEN 96
#define JAVA_COMPAT_ACH_COUNT 12

typedef struct JavaCompatLangEntry {
    char key[48];
    char value[JAVA_COMPAT_TEXT_LEN];
} JavaCompatLangEntry;

typedef struct JavaCompatAchievement {
    int id;
    const char *key;
    int iconItem;
    int parent;
    int unlocked;
} JavaCompatAchievement;

JavaCompatLangEntry g_javaCompatLang[JAVA_COMPAT_MAX_LANG];
int g_javaCompatLangCount = 0;
char g_javaCompatSplashes[JAVA_COMPAT_MAX_SPLASHES][JAVA_COMPAT_TEXT_LEN];
int g_javaCompatSplashCount = 0;
int g_javaCompatSplashIndex = 0;
double g_javaCompatSplashTimer = 0.0;
int g_javaCompatInitDone = 0;
int g_javaCompatBlocksMapped = 0;
int g_javaCompatItemsMapped = 0;
int g_javaCompatAssetsMapped = 0;

JavaCompatAchievement g_javaCompatAchievements[JAVA_COMPAT_ACH_COUNT] = {
    {0, "openInventory", ITEM_BOOK, -1, 0},
    {1, "mineWood", ITEM_WOOD, 0, 0},
    {2, "buildWorkBench", ITEM_WORKBENCH, 1, 0},
    {3, "buildPickaxe", ITEM_WOOD_PICKAXE, 2, 0},
    {4, "buildFurnace", ITEM_FURNACE, 3, 0},
    {5, "acquireIron", ITEM_IRON_INGOT, 4, 0},
    {6, "diamonds", ITEM_DIAMOND, 5, 0},
    {7, "buildSword", ITEM_WOOD_SWORD, 2, 0},
    {8, "killEnemy", ITEM_BONE, 7, 0},
    {9, "bakeBread", ITEM_BREAD, 2, 0},
    {10, "cookFish", ITEM_FISH_COOKED, 4, 0},
    {11, "flyPig", ITEM_SADDLE, 1, 0}
};

int JavaCompatTrimLine(char *s)
{
    int n;

    if (!s) { return 0; }
    n = (int)strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ' || s[n - 1] == '\t')) {
        s[n - 1] = 0;
        n--;
    }
    return n;
}

int JavaCompat_LoadTitleSplashesFile(const char *path)
{
    FILE *f;
    char line[160];
    int n;

    f = fopen(path, "r");
    if (!f) { return 0; }
    g_javaCompatSplashCount = 0;
    while (fgets(line, sizeof(line), f) && g_javaCompatSplashCount < JAVA_COMPAT_MAX_SPLASHES) {
        n = JavaCompatTrimLine(line);
        if (n > 0) {
            strncpy(g_javaCompatSplashes[g_javaCompatSplashCount], line, JAVA_COMPAT_TEXT_LEN - 1);
            g_javaCompatSplashes[g_javaCompatSplashCount][JAVA_COMPAT_TEXT_LEN - 1] = 0;
            g_javaCompatSplashCount++;
        }
    }
    fclose(f);
    if (g_javaCompatSplashCount > 0) {
        g_javaCompatSplashIndex = WorldHash2D(g_worldSeed, (int)g_worldTimeSeconds, 7703) % g_javaCompatSplashCount;
    }
    return g_javaCompatSplashCount;
}

int JavaCompat_LoadLanguageFile(const char *path)
{
    FILE *f;
    char line[192];
    char *eq;
    int n;

    f = fopen(path, "r");
    if (!f) { return 0; }
    g_javaCompatLangCount = 0;
    while (fgets(line, sizeof(line), f) && g_javaCompatLangCount < JAVA_COMPAT_MAX_LANG) {
        n = JavaCompatTrimLine(line);
        if (n > 0 && line[0] != '#') {
            eq = strchr(line, '=');
            if (eq) {
                *eq = 0;
                strncpy(g_javaCompatLang[g_javaCompatLangCount].key, line, 47);
                g_javaCompatLang[g_javaCompatLangCount].key[47] = 0;
                strncpy(g_javaCompatLang[g_javaCompatLangCount].value, eq + 1, JAVA_COMPAT_TEXT_LEN - 1);
                g_javaCompatLang[g_javaCompatLangCount].value[JAVA_COMPAT_TEXT_LEN - 1] = 0;
                g_javaCompatLangCount++;
            }
        }
    }
    fclose(f);
    return g_javaCompatLangCount;
}

const char *JavaCompat_Translate(const char *key)
{
    int i;

    for (i = 0; i < g_javaCompatLangCount; i++) {
        if (strcmp(g_javaCompatLang[i].key, key) == 0) { return g_javaCompatLang[i].value; }
    }
    return key;
}

void JavaCompat_UnlockAchievement(const char *key)
{
    int i;

    if (!key) { return; }
    for (i = 0; i < JAVA_COMPAT_ACH_COUNT; i++) {
        if (strcmp(g_javaCompatAchievements[i].key, key) == 0) {
            g_javaCompatAchievements[i].unlocked = 1;
            return;
        }
    }
}

void JavaCompat_InitFeatureRegistry(void)
{
    if (g_javaCompatInitDone) { return; }
    g_javaCompatInitDone = 1;

    /* Counts here are not gameplay limits; they mark Java source groups now mapped into the C bridge. */
    g_javaCompatBlocksMapped = 97;
    g_javaCompatItemsMapped = 226;
    g_javaCompatAssetsMapped = 0;
    JavaCompat_UnlockAchievement("openInventory");
}

void JavaCompat_LoadLangAndSplashes(void)
{
    JavaCompat_LoadLanguageFile("assets/lang/en_US.lang");
    JavaCompat_LoadTitleSplashesFile("assets/title/splashes.txt");
}

void JavaCompat_UpdateFeatureSystems(double dt)
{
    /* V11: splash text rotation disabled. */
}

void JavaCompat_DrawSplashText(void)
{
    /* V11: main-menu yellow splash text is disabled by request. */
    return;
}

int JavaCompat_GetFoodHealPoints(int item)
{
    if (item == ITEM_APPLE) { return 4; }
    if (item == ITEM_BREAD) { return 5; }
    if (item == ITEM_PORK_RAW) { return 3; }
    if (item == ITEM_PORK_COOKED) { return 8; }
    if (item == ITEM_CHICKEN_RAW) { return 2; }
    if (item == ITEM_CHICKEN_COOKED) { return 6; }
    if (item == ITEM_FISH_RAW) { return 2; }
    if (item == ITEM_FISH_COOKED) { return 5; }
    if (item == ITEM_COOKIE) { return 1; }
    if (item == ITEM_MUSHROOM_STEW) { return 10; }
    return 0;
}

int JavaCompat_GetFuelBurnTicks(int item)
{
    if (item == ITEM_COAL) { return 1600; }
    if (item == ITEM_WOOD || item == ITEM_PLANKS || item == ITEM_STICK) { return 300; }
    if (item == ITEM_LAVA_BUCKET) { return 20000; }
    return 0;
}

int JavaCompat_IsMappedExtraBlock(int block)
{
    switch (block) {
    case BLOCK_SAPLING:
    case BLOCK_LAVA:
    case BLOCK_STATIONARY_LAVA:
    case BLOCK_WEB:
    case BLOCK_TALL_GRASS:
    case BLOCK_DEAD_BUSH:
    case BLOCK_PISTON_STICKY:
    case BLOCK_PISTON_EXTENSION:
    case BLOCK_FLOWER_YELLOW:
    case BLOCK_FLOWER_RED:
    case BLOCK_MUSHROOM_BROWN:
    case BLOCK_MUSHROOM_RED:
    case BLOCK_GOLD_BLOCK:
    case BLOCK_IRON_BLOCK:
    case BLOCK_DOUBLE_STEP:
    case BLOCK_STEP:
    case BLOCK_BRICK:
    case BLOCK_TNT:
    case BLOCK_BOOKSHELF:
    case BLOCK_MOSSY_COBBLESTONE:
    case BLOCK_OBSIDIAN:
    case BLOCK_FIRE:
    case BLOCK_DIAMOND_BLOCK:
    case BLOCK_CROPS:
    case BLOCK_FARMLAND:
    case BLOCK_FURNACE_LIT:
    case BLOCK_SIGN_WALL:
    case BLOCK_REDSTONE_TORCH_OFF:
    case BLOCK_SNOW_BLOCK:
    case BLOCK_CLAY:
    case BLOCK_REED:
    case BLOCK_JUKEBOX:
    case BLOCK_FENCE:
    case BLOCK_PUMPKIN:
    case BLOCK_NETHERRACK:
    case BLOCK_SOULSAND:
    case BLOCK_GLOWSTONE:
    case BLOCK_PORTAL:
    case BLOCK_JACK_O_LANTERN:
    case BLOCK_CAKE:
    case BLOCK_REPEATER_OFF:
    case BLOCK_REPEATER_ON:
    case BLOCK_LOCKED_CHEST:
    case BLOCK_TRAPDOOR:
        return 1;
    }
    return 0;
}


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
void InitMobProjectiles(void);
int SpawnSkeletonArrowProjectile(Mob *m);
void UpdateMobProjectiles(double dt);
void RenderMobProjectiles(void);
int MobHasLineOfSightToPlayer(Mob *m);
int MobMaxHealthJavaV4(int type);
int TryMobInteractJavaV4(int mobIndex);
void RenderMobModelJavaV4(Mob *m, GLuint tex, float alpha);
void DrawJavaModelBoxV4(float rpX, float rpY, float rpZ, float boxX, float boxY, float boxZ, float boxW, float boxH, float boxD, GLuint tex, float shade, float alpha, int tx, int ty, int tw, int th, int td, float rotX, float rotY, float rotZ);
void DrawJavaBipedModelV4(Mob *m, GLuint tex, float shade, float alpha, int skeletonLike, int heldItem, float headPitch, int zombieArms, float inflate);
void DrawJavaQuadrupedModelV4(Mob *m, GLuint tex, float shade, float alpha, int kind, float inflate);
void DrawJavaChickenModelV4(Mob *m, GLuint tex, float shade, float alpha);
void DrawJavaSpiderModelV4(Mob *m, GLuint tex, float shade, float alpha);
void DrawJavaCreeperModelV4(Mob *m, GLuint tex, float shade, float alpha);
void DrawJavaWolfModelV4(Mob *m, GLuint tex, float shade, float alpha);
void DrawJavaSquidModelV4(Mob *m, GLuint tex, float shade, float alpha);
void RenderPlayerArmorLayersJavaV4(float shade);
GLuint PlayerArmorTextureJavaV4(int layer);
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
int TryContinuousAttackMobV28(double dt);
void ResetHeldMiningTargetV28(void);
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
void GuiV7_DrawVideoSettings(void);
void GuiV7_DrawControls(void);
void GuiV7_DrawRenameWorld(void);
void GuiV7_DrawAchievements(void);
void GuiV7_DrawStats(void);
void GuiV7_EnterVideoSettings(void);
void GuiV7_EnterControls(void);
void GuiV7_EnterAchievements(void);
void GuiV7_EnterStats(void);
void GuiV7_EnterRenameWorld(void);
int GuiV7_HandleMouseDown(int mx, int my);
int GuiV7_HandleChar(WPARAM ch);
int GuiV7_HandleKeyDown(WPARAM key);
const char *GuiV7_KeyName(WPARAM key);
void GuiV7_RenameSelectedWorld(void);
void GuiV7_DrawSliderButton(RECT r, const char *label, int value, int maxValue, int hover);
int GuiV7_CountUnlockedAchievements(void);
void DrawDeathScreen(void);

/* GUI_PIPELINE_V9 prototypes */
void GuiV9_Init(void);
void GuiV9_InitIfNeeded(void);
void GuiV9_UpdateScaledResolution(void);
const char *GuiV9_Tr(const char *key, const char *fallback);
void GuiV9_LoadTranslations(void);
int GuiV9_IsAllowedChatChar(int ch);
void GuiV9_TextFieldInit(GuiV9TextField *f, RECT r, const char *initialText, int maxLen);
void GuiV9_TextFieldDraw(GuiV9TextField *f, const char *label);
int GuiV9_TextFieldChar(GuiV9TextField *f, WPARAM ch);
int GuiV9_TextFieldKey(GuiV9TextField *f, WPARAM key);
void GuiV9_TextFieldMouse(GuiV9TextField *f, int mx, int my);
int GuiV9_FontWidth(const char *text);
void GuiV9_DrawString(int x, int y, const char *text, float r, float g, float b);
void GuiV9_DrawCenteredString(int x1, int y1, int x2, int y2, const char *text, float r, float g, float b);
void GuiV9_DrawGradientRect(int x1, int y1, int x2, int y2, float r1, float g1, float b1, float a1, float r2, float g2, float b2, float a2);
void GuiV9_AddChatLine(const char *text);
void GuiV9_OpenChat(void);
void GuiV9_CloseChat(void);
void GuiV9_DrawChatOverlay(void);
int GuiV9_HandleChar(WPARAM ch);
int GuiV9_HandleKeyDown(WPARAM key);
int GuiV9_HandleMouseDown(int mx, int my);
int GuiV9_HandleMouseWheel(int delta);
void GuiV9_Layout(void);
void GuiV9_EnterMultiplayer(void);
void GuiV9_EnterConnecting(const char *address);
void GuiV9_EnterConnectFailed(const char *message);
void GuiV9_EnterDownloadTerrain(void);
void GuiV9_EnterSleepMP(void);
void GuiV9_EnterTexturePacks(void);
void GuiV9_EnterConflictWarning(const char *line1, const char *line2);
void GuiV9_EnterErrorScreen(const char *title, const char *message);
void GuiV9_EnterYesNo(const char *title, const char *line1, const char *line2, int action, int arg);
void GuiV9_DrawMultiplayer(void);
void GuiV9_DrawConnecting(void);
void GuiV9_DrawConnectFailed(void);
void GuiV9_DrawDownloadTerrain(void);
void GuiV9_DrawSleepMP(void);
void GuiV9_DrawTexturePacks(void);
void GuiV9_DrawConflictWarning(void);
void GuiV9_DrawErrorScreen(void);
void GuiV9_DrawYesNo(void);
void GuiV9_LoadTexturePacks(void);
void GuiV9_ApplyTexturePack(int index);
int GuiV9_SaveScreenshot(void);
void GuiV9_DrawSlotListFrame(GuiV9SlotList *slot);
void GuiV9_ScrollSlot(GuiV9SlotList *slot, int amount);
void GuiV9_RunChatCommand(const char *cmd);
void GuiV9_RecordChatHistory(const char *line);
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
void DrawButtonState2D(RECT r, const char *text, int hover, int disabled, int selected);
void GuiV16_DrawSelectionBorder(RECT r);
void GuiV16_DrawTransitionOverlay(void);
int GuiV16_GetScaleFactor(void);
int GuiV16_GetInventoryScale(void);
int GuiV16_GetSlotPixels(void);
int GuiV16_IsShiftDown(void);
int GuiV16_MoveStackToSlots(InventorySlot *src, InventorySlot *dst, int dstCount);
int GuiV16_MoveStackToPlayer(InventorySlot *src);
int GuiV16_MoveStackToHotbarOrInventory(InventorySlot *src, int fromHotbar);
int GuiV16_MoveStackToTileContainer(InventorySlot *src, TileEntityState *t);
int GuiV16_ClickTakeOnlySlot(InventorySlot *slot, int rightClick);
int GuiV16_QuickCraftResultToPlayer(void);
void GuiV16_DrawSlotTooltip(int mx, int my);
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
void SaveUserOptionsV13B(void);
void LoadUserOptionsV13B(void);
void SetRenderDistanceExactV13B(int value);
int TryEatHeldFoodV13B(void);
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
void ResourceV10_DetectCaps(void);

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
int PlayerV22_IsReplaceableBlock(int block);
void PlayerV22_RefreshEnvironment(void);
double PlayerV22_GetEyeHeight(void);
void PlayerV22_ApplyCameraBob(void);
void PlayerV22_ClipThirdPersonCamera(double eyeX, double eyeY, double eyeZ, double *camX, double *camY, double *camZ);
int PlayerV22_BlockHasCollision(int block);
int PlayerV22_AabbCollidesAt(double x, double y, double z);
int PlayerV22_MoveAxisSweep(double dx, double dy, double dz);
int PlayerV22_MoveHorizontal(double dx, double dz);
void PlayerV22_HandleMovementInput(double dt);
void PlayerV22_UpdatePhysics(double dt);
void PlayerV22_ApplyFriction(double amount, double dt);
void PlayerV22_ApproachVelocity(double targetX, double targetZ, double accel, double dt);
void PlayerV39_HandleMovementInput(double dt);
void PlayerV39_UpdatePhysics(double dt);
void PlayerV39_ApplyNoInputFriction(double dt);
int PlayerV22_CanPlaceBlockAt(int block, int x, int y, int z, int hitX, int hitY, int hitZ);
void PlayerV22_ApplyPlacementMetadata(int block, int x, int y, int z, int hitX, int hitY, int hitZ);
int CanWaterFlowIntoBlock(int block);
int IsWaterSpreadSupportBlock(int block);
void UpdateJavaStyleWaterFlow(double dt);
int BlockV42_IsWater(int block);
int BlockV42_IsLava(int block);
int BlockV42_IsLiquid(int block);
int FluidV42_GetDecayAt(int x, int y, int z, int liquid);
int FluidV42_CanDisplace(int liquid, int x, int y, int z);
void FluidV42_SetBlockWithMeta(int x, int y, int z, int block, int meta);
void FluidV42_MixWaterLavaAround(int x, int y, int z);
void UpdateJavaStyleFluidFireV42(double dt);
void EnqueueLightUpdateV42(int x, int y, int z, int radius);
void ProcessLightUpdatesV42(int maxUpdates);
int GetBlockLightOpacityV42(int block);
int GetBlockLightOpacityV48(int block);
int GetBlockLightValueSourceV48(int block);
int GetSavedLightValueV48(int type, int x, int y, int z);
void SetLightValueV48(int type, int x, int y, int z, int value);
int CanExistingBlockSeeTheSkyV48(int x, int y, int z);
int ComputeLightTargetV48(int type, int x, int y, int z);
void ScheduleLightingUpdateV48(int type, int minX, int minY, int minZ, int maxX, int maxY, int maxZ);
void NeighborLightPropagationChangedV48(int type, int x, int y, int z, int expected);
void QueueBlockLightingAroundV48(int x, int y, int z, int radius);
void ProcessLightUpdatesV48(int maxUpdates);
float FluidV42_GetRenderedHeight(int x, int y, int z, int block);
void RenderFluidBlockV42(int x, int y, int z, int block);
void OpenCraftingTable(int x, int y, int z);
void CloseCraftingTable(void);
void CloseActiveContainerV5(void);
int TryActivateTileOrRedstoneV5(int x, int y, int z);
void OpenTileContainerV5(int block, int x, int y, int z);
void DrawActiveContainerV5(void);
void TileContainerMouseClickV5(int mx, int my, int rightClick);
void TileEntitiesRedstoneV5_Update(double dt);
void HandleSignCharV5(WPARAM ch);
int IsSignEditingV5(void);
int IsSolidBlockAtV5(int x, int y, int z);
void DrawSpecialBlockV5(int x, int y, int z, int block);
int IsSpecialBlockV5(int block);
void UpdateRedstoneNetworkV5(int cx, int cy, int cz);
void UpdatePistonsV5(void);
int FurnaceRecipeOutputV5(int item);
void RenderMinecartsV5(void);
void UpdateMinecartsV5(double dt);
int SpawnMinecartV5(double x, double y, double z, double vx, double vz);
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
void InvalidateTerrainChunkMeshAt(int x, int z);
void RecomputeLegacyLightingLocal(int cx, int cy, int cz, int radius);
void AddWorldFeatures(void);


/* ------------------------------------------------------------ */
/* V25 Open Watcom link/compile fix: real V5 tile/redstone funcs */
/* ------------------------------------------------------------ */

int RedstoneV25_IsPowerSourceAt(int x, int y, int z)
{
    int block;
    int meta;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    block = GetBlock(x, y, z);
    meta = (int)g_blockMeta[x][y][z];
    if (block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_GLOWSTONE || block == BLOCK_JACK_O_LANTERN) { return 15; }
    if (block == BLOCK_REPEATER_ON) { return 15; }
    if ((block == BLOCK_LEVER || block == BLOCK_STONE_BUTTON || block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE || block == BLOCK_DETECTOR_RAIL) && (meta & 8)) { return 15; }
    if (block == BLOCK_REDSTONE_WIRE && g_redstonePower[x][y][z] > 0) { return (int)g_redstonePower[x][y][z]; }
    return 0;
}

int RedstoneV25_IsWireReceiver(int block)
{
    if (block == BLOCK_REDSTONE_WIRE) { return 1; }
    if (block == BLOCK_REPEATER_OFF || block == BLOCK_REPEATER_ON) { return 1; }
    if (block == BLOCK_PISTON || block == BLOCK_PISTON_STICKY) { return 1; }
    if (block == BLOCK_WOOD_DOOR || block == BLOCK_TRAPDOOR) { return 1; }
    if (block == BLOCK_DISPENSER || block == BLOCK_NOTE) { return 1; }
    return 0;
}

int RedstoneV25_AdjacentMaxPower(int x, int y, int z)
{
    int p;
    int best;
    best = 0;
    p = RedstoneV25_IsPowerSourceAt(x + 1, y, z); if (p > best) { best = p; }
    p = RedstoneV25_IsPowerSourceAt(x - 1, y, z); if (p > best) { best = p; }
    p = RedstoneV25_IsPowerSourceAt(x, y + 1, z); if (p > best) { best = p; }
    p = RedstoneV25_IsPowerSourceAt(x, y - 1, z); if (p > best) { best = p; }
    p = RedstoneV25_IsPowerSourceAt(x, y, z + 1); if (p > best) { best = p; }
    p = RedstoneV25_IsPowerSourceAt(x, y, z - 1); if (p > best) { best = p; }
    if (best > 0 && best < 15) { best--; }
    return best;
}

void UpdateRedstoneNetworkV5(int cx, int cy, int cz)
{
    int x0;
    int x1;
    int y0;
    int y1;
    int z0;
    int z1;
    int x;
    int y;
    int z;
    int pass;
    int block;
    int p;
    int changed;
    x0 = cx - 10; if (x0 < 0) { x0 = 0; }
    x1 = cx + 10; if (x1 >= WORLD_X) { x1 = WORLD_X - 1; }
    y0 = cy - 6; if (y0 < 0) { y0 = 0; }
    y1 = cy + 6; if (y1 >= WORLD_Y) { y1 = WORLD_Y - 1; }
    z0 = cz - 10; if (z0 < 0) { z0 = 0; }
    z1 = cz + 10; if (z1 >= WORLD_Z) { z1 = WORLD_Z - 1; }

    for (x = x0; x <= x1; x++) {
        for (y = y0; y <= y1; y++) {
            for (z = z0; z <= z1; z++) {
                block = GetBlock(x, y, z);
                if (block == BLOCK_REDSTONE_WIRE || block == BLOCK_REPEATER_OFF || block == BLOCK_REPEATER_ON || block == BLOCK_PISTON || block == BLOCK_PISTON_STICKY || block == BLOCK_DISPENSER || block == BLOCK_NOTE) {
                    if (block != BLOCK_REPEATER_ON) { g_redstonePower[x][y][z] = 0; }
                }
            }
        }
    }

    for (pass = 0; pass < 16; pass++) {
        changed = 0;
        for (x = x0; x <= x1; x++) {
            for (y = y0; y <= y1; y++) {
                for (z = z0; z <= z1; z++) {
                    block = GetBlock(x, y, z);
                    if (!RedstoneV25_IsWireReceiver(block)) { continue; }
                    p = RedstoneV25_AdjacentMaxPower(x, y, z);
                    if (block == BLOCK_REPEATER_ON && p < 15) { p = 15; }
                    if (p > 15) { p = 15; }
                    if (p < 0) { p = 0; }
                    if ((int)g_redstonePower[x][y][z] != p) { g_redstonePower[x][y][z] = (unsigned char)p; changed = 1; }
                }
            }
        }
        if (!changed) { break; }
    }
}

void UpdateRedstoneAround(int x, int y, int z)
{
    if (!IsInsideWorld(x, y, z)) { return; }
    UpdateRedstoneNetworkV5(x, y, z);
    UpdatePistonsV5();
    InvalidateTerrainChunkMeshAt(x, z);
    RecomputeLegacyLightingLocal(x, y, z, 8);
}

int IsSignEditingV5(void)
{
    if (g_containerModeV5 != CONTAINER_SIGN_V5) { return 0; }
    if (g_activeTileIndexV5 < 0 || g_activeTileIndexV5 >= MAX_TILE_ENTITIES) { return 0; }
    if (!tileEntities[g_activeTileIndexV5].active) { return 0; }
    return 1;
}

void HandleSignCharV5(WPARAM ch)
{
    TileEntityState *t;
    int len;
    int i;
    char c;
    if (!IsSignEditingV5()) { return; }
    t = &tileEntities[g_activeTileIndexV5];
    len = (int)strlen(t->text);
    if (g_signEditCursorV5 < 0 || g_signEditCursorV5 > len) { g_signEditCursorV5 = len; }
    if (ch == 13 || ch == 27) { CloseActiveContainerV5(); return; }
    if (ch == 9) { ch = '|'; }
    if (ch == 8) {
        if (g_signEditCursorV5 > 0) {
            for (i = g_signEditCursorV5 - 1; i < len; i++) { t->text[i] = t->text[i + 1]; }
            g_signEditCursorV5--;
        }
        return;
    }
    if (ch < 32 || ch > 126) { return; }
    if (len >= 63) { return; }
    c = (char)ch;
    for (i = len; i >= g_signEditCursorV5; i--) { t->text[i + 1] = t->text[i]; }
    t->text[g_signEditCursorV5] = c;
    g_signEditCursorV5++;
}

void CloseActiveContainerV5(void)
{
    if (craftingOpen) { CloseCraftingTable(); }
    if (g_containerModeV5 == CONTAINER_CHEST_V5 && g_activeTileIndexV5 >= 0 && g_activeTileIndexV5 < MAX_TILE_ENTITIES) {
        PlayChestSoundV35((double)tileEntities[g_activeTileIndexV5].x + 0.5, (double)tileEntities[g_activeTileIndexV5].y + 0.5, (double)tileEntities[g_activeTileIndexV5].z + 0.5, 0);
    }
    if (g_draggingInventory) { DropCarriedInventoryStackToWorld(0); }
    g_draggingInventory = 0;
    g_dragSlot.item = ITEM_NONE;
    g_dragSlot.count = 0;
    g_dragSlot.damage = 0;
    inventoryOpen = 0;
    craftingOpen = 0;
    g_containerModeV5 = CONTAINER_NONE_V5;
    g_activeTileIndexV5 = -1;
    g_signEditCursorV5 = 0;
    SaveHandler_SaveTileEntitiesV2();
    if (g_state == STATE_GAME) { LockMouseForGame(); }
}

void OpenTileContainerV5(int block, int x, int y, int z)
{
    int idx;
    EnsureTileEntityForBlock(block, x, y, z);
    idx = FindTileEntityAt(x, y, z);
    if (idx < 0) { return; }
    ReturnCraftingGridToInventory();
    craftingOpen = 0;
    inventoryOpen = 1;
    g_activeTileIndexV5 = idx;
    g_signEditCursorV5 = (int)strlen(tileEntities[idx].text);
    if (block == BLOCK_CHEST || block == BLOCK_LOCKED_CHEST) { g_containerModeV5 = CONTAINER_CHEST_V5; PlayChestSoundV35((double)x + 0.5, (double)y + 0.5, (double)z + 0.5, 1); }
    else if (FurnaceV36_IsFurnaceBlock(block)) { g_containerModeV5 = CONTAINER_FURNACE_V5; if (block == BLOCK_FURNACE_LIT || block == BLOCK_LIT_FURNACE) { PlayFurnaceSoundV35((double)x + 0.5, (double)y + 0.5, (double)z + 0.5); } else { PlaySoundAtV35("assets\\sounds\\dig\\dig_stone1.mp3", (double)x + 0.5, (double)y + 0.5, (double)z + 0.5, 0.25, SoundRandomRangeV35(0.75, 0.90), SOUND_DEFAULT_RANGE_V35); } }
    else if (block == BLOCK_DISPENSER) { g_containerModeV5 = CONTAINER_DISPENSER_V5; }
    else if (block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL) { g_containerModeV5 = CONTAINER_SIGN_V5; }
    else if (block == BLOCK_NOTE) { g_containerModeV5 = CONTAINER_NOTE_V5; tileEntities[idx].power = (tileEntities[idx].power + 1) & 15; }
    else { g_containerModeV5 = CONTAINER_CHEST_V5; }
    g_statsContainersOpenedV7++;
    UnlockMouseFromGame();
}

int TryActivateTileOrRedstoneV5(int x, int y, int z)
{
    int block;
    int bx;
    int by;
    int bz;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    block = GetBlock(x, y, z);
    if (block == BLOCK_JUKEBOX && g_blockMeta[x][y][z] != 0) {
        int rec;
        rec = (g_blockMeta[x][y][z] == 2) ? ITEM_RECORD_CAT : ITEM_RECORD_13;
        AddDroppedItem(rec, 1, (double)x + 0.5, (double)y + 1.0, (double)z + 0.5, 0.0, 0.18, 0.0);
        g_blockMeta[x][y][z] = 0;
        mciSendString("stop recordMusicV37", NULL, 0, NULL);
        mciSendString("close recordMusicV37", NULL, 0, NULL);
        PlayUIClickSound();
        return 1;
    }
    if (block == BLOCK_WORKBENCH) { OpenCraftingTable(x, y, z); return 1; }
    if (block == BLOCK_CHEST || block == BLOCK_LOCKED_CHEST || block == BLOCK_FURNACE || block == BLOCK_FURNACE_LIT || block == BLOCK_LIT_FURNACE || block == BLOCK_DISPENSER || block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL || block == BLOCK_NOTE) {
        OpenTileContainerV5(block, x, y, z);
        return 1;
    }
    if (block == BLOCK_WOOD_DOOR) {
        bx = x; by = y; bz = z;
        if ((g_blockMeta[x][y][z] & 8) && IsInsideWorld(x, y - 1, z) && GetBlock(x, y - 1, z) == BLOCK_WOOD_DOOR) { by = y - 1; }
        if (g_blockMeta[bx][by][bz] & 32) { return 0; }
        g_blockMeta[bx][by][bz] ^= 1;
        if (IsInsideWorld(bx, by + 1, bz) && GetBlock(bx, by + 1, bz) == BLOCK_WOOD_DOOR) { g_blockMeta[bx][by + 1][bz] ^= 1; }
        InvalidateTerrainChunkMeshAt(bx, bz);
        PlayDoorSoundV35((double)bx + 0.5, (double)by + 0.5, (double)bz + 0.5, (g_blockMeta[bx][by][bz] & 1) ? 1 : 0);
        return 1;
    }
    if (block == BLOCK_TRAPDOOR) {
        g_blockMeta[x][y][z] ^= 1;
        InvalidateTerrainChunkMeshAt(x, z);
        PlayDoorSoundV35((double)x + 0.5, (double)y + 0.5, (double)z + 0.5, (g_blockMeta[x][y][z] & 1) ? 1 : 0);
        return 1;
    }
    if (block == BLOCK_LEVER || block == BLOCK_STONE_BUTTON || block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE || block == BLOCK_DETECTOR_RAIL) {
        g_blockMeta[x][y][z] ^= 8;
        UpdateRedstoneAround(x, y, z);
        PlayUIClickSound();
        return 1;
    }
    if (block == BLOCK_REPEATER_OFF) { SetBlock(x, y, z, BLOCK_REPEATER_ON); UpdateRedstoneAround(x, y, z); PlayUIClickSound(); return 1; }
    if (block == BLOCK_REPEATER_ON) { SetBlock(x, y, z, BLOCK_REPEATER_OFF); UpdateRedstoneAround(x, y, z); PlayUIClickSound(); return 1; }
    if (block == BLOCK_REDSTONE_TORCH_ON) { SetBlock(x, y, z, BLOCK_REDSTONE_TORCH_OFF); UpdateRedstoneAround(x, y, z); PlayUIClickSound(); return 1; }
    if (block == BLOCK_REDSTONE_TORCH_OFF) { SetBlock(x, y, z, BLOCK_REDSTONE_TORCH_ON); UpdateRedstoneAround(x, y, z); PlayUIClickSound(); return 1; }
    return 0;
}

/* V20: Java-style terrain/biome/decorator pass from ChunkProviderGenerate,
   NoiseGeneratorOctaves, WorldChunkManager, WorldGenMinable, WorldGenLakes,
   WorldGenDungeons, and related WorldGen*.java responsibilities. */
double WorldGenV20_OctaveBlend2D(int gx, int gz, double scaleA, double scaleB, int saltA, int saltB);
double WorldGenV20_OceanEdgeScore(int gx, int gz);
int WorldGenV20_ShouldUseBeachSurface(int lx, int lz, int biome, int y);
int WorldGenV20_DeterministicChunkRand(int chunkX, int chunkZ, int index, int salt);
void WorldGenV20_AddJavaOrePass(void);
void WorldGenV20_AddLakesAndDungeonsPass(void);
void WorldGenV20_AddDecorationPass(void);
void WorldGenV20_AddCaveAndSpringFinishing(void);
void WorldGenV20_EnsureGeneratedTileEntities(void);
void WorldGenV20_AddTreeVariant(int x, int y, int z, int biome, int salt);

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
double MobDistanceSquaredToPlayer(Mob *m);
void TakeDamage(int amount);
void SpawnWaterBubbleParticles(double x, double y, double z, int count);
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
void DrawHeldItemFirstPersonV29(int item);
void DrawPlayerArmPrism(void);
void DrawPlayerArmFace(float x0, float x1, float y0, float y1, float z0, float z1, int face);

/* V30_LINK_FIX: real definitions restored for V27/V29 render helpers that Open Watcom was linking against. */
