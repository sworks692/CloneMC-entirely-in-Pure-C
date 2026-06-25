/* ============================================================
   CloneMC V51 section: ITEM / COMBAT / ITEM USE / SPECIAL ENTITY UPDATE
   Keep this file included through the root unity source unless you
   are doing a later full extern/prototype object-file refactor.
   ============================================================ */

int MobMaxHealthJavaV4(int type)
{
    if (type == MOB_CHICKEN) { return 4; }
    if (type == MOB_SQUID) { return 10; }
    if (type == MOB_SHEEP || type == MOB_PIG || type == MOB_COW || type == MOB_WOLF) { return 10; }
    if (type == MOB_SLIME) { return 8; }
    if (type == MOB_CREEPER || type == MOB_ZOMBIE || type == MOB_SKELETON || type == MOB_SPIDER) { return 20; }
    return 10;
}

int TryMobInteractJavaV4(int mobIndex)
{
    Mob *m;
    int item;
    int hash;
    if (mobIndex < 0 || mobIndex >= MAX_MOBS) { return 0; }
    m = &mobs[mobIndex];
    if (!m->active) { return 0; }
    item = hotbar[selectedHotbarSlot].item;
    if (m->type == MOB_SHEEP && item == ITEM_SHEARS && !m->sheared) {
        m->sheared = 1;
        DropWoolNearMob(m);
        DamageHeldTool(1);
        PlayOneShotMP3("assets\\sounds\\mob\\sheep\\shear.mp3");
        return 1;
    }
    if (m->type == MOB_WOLF && item == ITEM_BONE && m->angry != 2) {
        hash = WorldHash3D((int)m->x, (int)m->y, (int)m->z, g_worldSeed + 9100);
        RemoveItemFromSelectedHotbar(1);
        if ((hash & 3) != 0) { m->angry = 2; m->health = MobMaxHealthJavaV4(MOB_WOLF); }
        else { m->angry = 1; }
        return 1;
    }
    if (m->type == MOB_WOLF && m->angry == 2 && (item == ITEM_PORK_RAW || item == ITEM_PORK_COOKED || item == ITEM_CHICKEN_RAW || item == ITEM_CHICKEN_COOKED)) {
        m->health += JavaCompat_GetFoodHealPoints(item);
        if (m->health > MobMaxHealthJavaV4(MOB_WOLF)) { m->health = MobMaxHealthJavaV4(MOB_WOLF); }
        RemoveItemFromSelectedHotbar(1);
        return 1;
    }
    return 0;
}

GLuint PlayerArmorTextureJavaV4(int layer)
{
    int item;
    item = layer == 2 ? g_armorSlotsV6[ARMOR_SLOT_LEGS_V6].item : g_armorSlotsV6[ARMOR_SLOT_CHEST_V6].item;
    if (item == ITEM_NONE && layer != 2) { item = g_armorSlotsV6[ARMOR_SLOT_HEAD_V6].item; }
    if (item == ITEM_NONE && layer != 2) { item = g_armorSlotsV6[ARMOR_SLOT_FEET_V6].item; }
    if (item == ITEM_DIAMOND_HELMET || item == ITEM_DIAMOND_CHESTPLATE || item == ITEM_DIAMOND_LEGGINGS || item == ITEM_DIAMOND_BOOTS) { return layer == 2 ? texCompatArmorDiamond2 : texCompatArmorDiamond1; }
    if (item == ITEM_IRON_HELMET || item == ITEM_IRON_CHESTPLATE || item == ITEM_IRON_LEGGINGS || item == ITEM_IRON_BOOTS) { return layer == 2 ? texCompatArmorIron2 : texCompatArmorIron1; }
    if (item == ITEM_GOLD_HELMET || item == ITEM_GOLD_CHESTPLATE || item == ITEM_GOLD_LEGGINGS || item == ITEM_GOLD_BOOTS) { return layer == 2 ? texCompatArmorGold2 : texCompatArmorGold1; }
    if (item == ITEM_CHAIN_HELMET || item == ITEM_CHAIN_CHESTPLATE || item == ITEM_CHAIN_LEGGINGS || item == ITEM_CHAIN_BOOTS) { return layer == 2 ? texCompatArmorChain2 : texCompatArmorChain1; }
    if (item == ITEM_LEATHER_HELMET || item == ITEM_LEATHER_CHESTPLATE || item == ITEM_LEATHER_LEGGINGS || item == ITEM_LEATHER_BOOTS) { return layer == 2 ? texCompatArmorLeather2 : texCompatArmorLeather1; }
    return 0;
}

void DrawJavaBipedModelV4(Mob *m, GLuint tex, float shade, float alpha, int skeletonLike, int heldItem, float headPitch, int zombieArms, float inflate)
{
    if (!m || !tex) { return; }
    JavaModel_BeginEntity();
    JavaModel_RenderBipedCore(m, tex, shade, alpha, skeletonLike, heldItem, headPitch, zombieArms, inflate, 1);
    JavaModel_EndEntity();
}

void RenderPlayerArmorLayersJavaV4(float shade)
{
    Mob dummy;
    GLuint tex1;
    GLuint tex2;
    memset(&dummy, 0, sizeof(dummy));
    dummy.active = 1;
    dummy.type = 0;
    dummy.vx = (double)g_playerWalkAmount;
    dummy.vz = 0.0;
    dummy.animWalk = handBob;
    tex1 = PlayerArmorTextureJavaV4(1);
    tex2 = PlayerArmorTextureJavaV4(2);
    if (tex1) { DrawJavaBipedModelV4(&dummy, tex1, shade, 0.42f, 0, ITEM_NONE, 0.0f, 0, 0.08f); }
    if (tex2) { DrawJavaBipedModelV4(&dummy, tex2, shade, 0.42f, 0, ITEM_NONE, 0.0f, 0, 0.10f); }
}

void RenderMobModelJavaV4(Mob *m, GLuint tex, float alpha)
{
    JavaModel_RenderExactMob(m, tex, alpha);
}


/* ------------------------------------------------------------ */
/* Items/tools/combat/projectiles/vehicles/special entities V6  */
/* Converts the Java responsibilities of ItemTool/ItemBow/       */
/* ItemBucket/ItemBoat/ItemMinecart/ItemPainting/ItemRecord and  */
/* EntityArrow/EntityEgg/EntitySnowball/EntityTNTPrimed/etc into */
/* compact C89/OpenGL 1.1 systems for Open Watcom.              */
/* ------------------------------------------------------------ */

int ItemCombatV6_IsSword(int item)
{
    if (item == ITEM_WOOD_SWORD || item == ITEM_STONE_SWORD || item == ITEM_IRON_SWORD || item == ITEM_DIAMOND_SWORD || item == ITEM_GOLD_SWORD) { return 1; }
    return 0;
}

int ItemCombatV6_IsPickaxe(int item)
{
    if (item == ITEM_WOOD_PICKAXE || item == ITEM_STONE_PICKAXE || item == ITEM_IRON_PICKAXE || item == ITEM_DIAMOND_PICKAXE || item == ITEM_GOLD_PICKAXE) { return 1; }
    return 0;
}

int ItemCombatV6_IsAxe(int item)
{
    if (item == ITEM_WOOD_AXE || item == ITEM_STONE_AXE || item == ITEM_IRON_AXE || item == ITEM_DIAMOND_AXE || item == ITEM_GOLD_AXE) { return 1; }
    return 0;
}

int ItemCombatV6_IsShovel(int item)
{
    if (item == ITEM_WOOD_SHOVEL || item == ITEM_STONE_SHOVEL || item == ITEM_IRON_SHOVEL || item == ITEM_DIAMOND_SHOVEL || item == ITEM_GOLD_SHOVEL) { return 1; }
    return 0;
}

int ItemCombatV6_IsHoe(int item)
{
    if (item == ITEM_WOOD_HOE || item == ITEM_STONE_HOE || item == ITEM_IRON_HOE || item == ITEM_DIAMOND_HOE || item == ITEM_GOLD_HOE) { return 1; }
    return 0;
}

int ItemCombatV6_IsTool(int item)
{
    if (ItemCombatV6_IsSword(item) || ItemCombatV6_IsPickaxe(item) || ItemCombatV6_IsAxe(item) || ItemCombatV6_IsShovel(item) || ItemCombatV6_IsHoe(item)) { return 1; }
    if (item == ITEM_SHEARS || item == ITEM_FLINT_STEEL || item == ITEM_FISHING_ROD || item == ITEM_BOW) { return 1; }
    return 0;
}

int ItemCombatV6_GetHeldAttackDamage(void)
{
    int item;
    item = hotbar[selectedHotbarSlot].item;
    if (item == ITEM_WOOD_SWORD) { return 4; }
    if (item == ITEM_STONE_SWORD) { return 5; }
    if (item == ITEM_IRON_SWORD) { return 6; }
    if (item == ITEM_DIAMOND_SWORD) { return 7; }
    if (item == ITEM_GOLD_SWORD) { return 4; }
    if (ItemCombatV6_IsAxe(item)) { return 3; }
    if (ItemCombatV6_IsPickaxe(item) || ItemCombatV6_IsShovel(item)) { return 2; }
    return 2;
}

int ItemCombatV6_GetHeldAttackToolWear(void)
{
    int item;
    item = hotbar[selectedHotbarSlot].item;
    if (ItemCombatV6_IsSword(item)) { return 1; }
    if (ItemCombatV6_IsTool(item)) { return 2; }
    return 0;
}

int ItemCombatV6_ArmorSlotForItem(int item)
{
    if (item == ITEM_LEATHER_HELMET || item == ITEM_CHAIN_HELMET || item == ITEM_IRON_HELMET || item == ITEM_DIAMOND_HELMET || item == ITEM_GOLD_HELMET) { return ARMOR_SLOT_HEAD_V6; }
    if (item == ITEM_LEATHER_CHESTPLATE || item == ITEM_CHAIN_CHESTPLATE || item == ITEM_IRON_CHESTPLATE || item == ITEM_DIAMOND_CHESTPLATE || item == ITEM_GOLD_CHESTPLATE) { return ARMOR_SLOT_CHEST_V6; }
    if (item == ITEM_LEATHER_LEGGINGS || item == ITEM_CHAIN_LEGGINGS || item == ITEM_IRON_LEGGINGS || item == ITEM_DIAMOND_LEGGINGS || item == ITEM_GOLD_LEGGINGS) { return ARMOR_SLOT_LEGS_V6; }
    if (item == ITEM_LEATHER_BOOTS || item == ITEM_CHAIN_BOOTS || item == ITEM_IRON_BOOTS || item == ITEM_DIAMOND_BOOTS || item == ITEM_GOLD_BOOTS) { return ARMOR_SLOT_FEET_V6; }
    return -1;
}

int ItemCombatV6_ArmorPointsForItem(int item)
{
    if (item == ITEM_LEATHER_HELMET) { return 1; }
    if (item == ITEM_LEATHER_CHESTPLATE) { return 3; }
    if (item == ITEM_LEATHER_LEGGINGS) { return 2; }
    if (item == ITEM_LEATHER_BOOTS) { return 1; }
    if (item == ITEM_CHAIN_HELMET || item == ITEM_IRON_HELMET || item == ITEM_GOLD_HELMET) { return 2; }
    if (item == ITEM_CHAIN_CHESTPLATE || item == ITEM_IRON_CHESTPLATE || item == ITEM_GOLD_CHESTPLATE) { return 5; }
    if (item == ITEM_CHAIN_LEGGINGS || item == ITEM_IRON_LEGGINGS || item == ITEM_GOLD_LEGGINGS) { return 4; }
    if (item == ITEM_CHAIN_BOOTS || item == ITEM_IRON_BOOTS || item == ITEM_GOLD_BOOTS) { return 1; }
    if (item == ITEM_DIAMOND_HELMET) { return 3; }
    if (item == ITEM_DIAMOND_CHESTPLATE) { return 8; }
    if (item == ITEM_DIAMOND_LEGGINGS) { return 6; }
    if (item == ITEM_DIAMOND_BOOTS) { return 3; }
    return 0;
}

int ItemCombatV6_ArmorMaxDurability(int item)
{
    if (item >= ITEM_LEATHER_HELMET && item <= ITEM_LEATHER_BOOTS) { return 80; }
    if (item >= ITEM_CHAIN_HELMET && item <= ITEM_CHAIN_BOOTS) { return 180; }
    if (item >= ITEM_IRON_HELMET && item <= ITEM_IRON_BOOTS) { return 220; }
    if (item >= ITEM_DIAMOND_HELMET && item <= ITEM_DIAMOND_BOOTS) { return 480; }
    if (item >= ITEM_GOLD_HELMET && item <= ITEM_GOLD_BOOTS) { return 100; }
    return 0;
}

void ItemCombatV6_DamageArmor(int amount)
{
    int i;
    int maxDur;
    for (i = 0; i < 4; i++) {
        if (g_armorSlotsV6[i].item == ITEM_NONE || g_armorSlotsV6[i].count <= 0) { continue; }
        maxDur = ItemCombatV6_ArmorMaxDurability(g_armorSlotsV6[i].item);
        if (maxDur <= 0) { continue; }
        g_armorSlotsV6[i].damage += amount;
        if (g_armorSlotsV6[i].damage >= maxDur) {
            g_armorSlotsV6[i].item = ITEM_NONE;
            g_armorSlotsV6[i].count = 0;
            g_armorSlotsV6[i].damage = 0;
            PlayOneShotMP3("assets\\sounds\\random\\break.mp3");
        }
    }
}

int ItemCombatV6_ApplyArmorReduction(int amount)
{
    int i;
    int points;
    int reduced;
    if (amount <= 0) { return 0; }
    points = 0;
    for (i = 0; i < 4; i++) { points += ItemCombatV6_ArmorPointsForItem(g_armorSlotsV6[i].item); }
    if (points > 20) { points = 20; }
    reduced = amount - ((amount * points) / 25);
    if (reduced < 1 && amount > 0) { reduced = 1; }
    if (points > 0) { ItemCombatV6_DamageArmor(amount); }
    return reduced;
}

int ItemCombatV6_TryEquipArmor(void)
{
    InventorySlot *slot;
    int armorSlot;
    slot = &hotbar[selectedHotbarSlot];
    armorSlot = ItemCombatV6_ArmorSlotForItem(slot->item);
    if (armorSlot < 0) { return 0; }
    if (g_armorSlotsV6[armorSlot].item != ITEM_NONE) {
        AddItemToInventory(g_armorSlotsV6[armorSlot].item, 1);
    }
    g_armorSlotsV6[armorSlot].item = slot->item;
    g_armorSlotsV6[armorSlot].count = 1;
    g_armorSlotsV6[armorSlot].damage = slot->damage;
    RemoveItemFromSelectedHotbar(1);
    PlayUIClickSound();
    return 1;
}

int ItemCombatV6_FindHotbarItem(int item)
{
    int i;
    for (i = 0; i < HOTBAR_SLOTS; i++) {
        if (hotbar[i].item == item && hotbar[i].count > 0) { return i; }
    }
    return -1;
}

int ItemCombatV6_ConsumeOneInventoryItem(int item)
{
    int i;
    if (item == ITEM_NONE) { return 1; }
    for (i = 0; i < HOTBAR_SLOTS; i++) {
        if (hotbar[i].item == item && hotbar[i].count > 0) {
            hotbar[i].count--;
            if (hotbar[i].count <= 0) { hotbar[i].item = ITEM_NONE; hotbar[i].damage = 0; }
            return 1;
        }
    }
    for (i = 0; i < INVENTORY_SLOTS; i++) {
        if (inventory[i].item == item && inventory[i].count > 0) {
            inventory[i].count--;
            if (inventory[i].count <= 0) { inventory[i].item = ITEM_NONE; inventory[i].damage = 0; }
            return 1;
        }
    }
    return 0;
}

void InitItemCombatV6(void)
{
    int i;
    for (i = 0; i < MAX_SPECIAL_ENTITIES_V6; i++) { g_specialEntitiesV6[i].active = 0; }
    for (i = 0; i < 4; i++) { g_armorSlotsV6[i].item = ITEM_NONE; g_armorSlotsV6[i].count = 0; g_armorSlotsV6[i].damage = 0; }
    g_bowChargingV6 = 0;
    g_bowChargeV6 = 0.0;
    g_fishingHookIndexV6 = -1;
    g_lightningTimerV6 = 12.0;
}

int SpawnSpecialEntityV6(int type, int item, int block, double x, double y, double z, double vx, double vy, double vz, double fuse)
{
    int i;
    double h;
    double len;
    for (i = 0; i < MAX_SPECIAL_ENTITIES_V6; i++) {
        if (!g_specialEntitiesV6[i].active) {
            g_specialEntitiesV6[i].active = 1;
            g_specialEntitiesV6[i].type = type;
            g_specialEntitiesV6[i].item = item;
            g_specialEntitiesV6[i].block = block;
            g_specialEntitiesV6[i].damage = 0;
            g_specialEntitiesV6[i].stuck = 0;
            g_specialEntitiesV6[i].meta = 0;
            g_specialEntitiesV6[i].x = x;
            g_specialEntitiesV6[i].y = y;
            g_specialEntitiesV6[i].z = z;
            g_specialEntitiesV6[i].vx = vx;
            g_specialEntitiesV6[i].vy = vy;
            g_specialEntitiesV6[i].vz = vz;
            g_specialEntitiesV6[i].age = 0.0;
            g_specialEntitiesV6[i].life = 90.0;
            g_specialEntitiesV6[i].fuse = fuse;
            h = sqrt(vx * vx + vz * vz);
            g_specialEntitiesV6[i].yaw = atan2(vx, vz) * 180.0 / PI;
            len = sqrt(vx * vx + vy * vy + vz * vz);
            if (len > 0.001) { g_specialEntitiesV6[i].pitch = asin(vy / len) * 180.0 / PI; }
            else { g_specialEntitiesV6[i].pitch = 0.0; }
            (void)h;
            return i;
        }
    }
    return -1;
}

void ItemCombatV6_GetLookVector(double *dx, double *dy, double *dz)
{
    double yawRad;
    double pitchRad;
    yawRad = yaw * PI / 180.0;
    pitchRad = pitch * PI / 180.0;
    *dx = -sin(yawRad) * cos(pitchRad);
    *dy = sin(pitchRad);
    *dz = -cos(yawRad) * cos(pitchRad);
}

int ItemCombatV6_RaycastBlock(int *hitX, int *hitY, int *hitZ, int *placeX, int *placeY, int *placeZ)
{
    double dx;
    double dy;
    double dz;
    double sx;
    double sy;
    double sz;
    double t;
    int bx;
    int by;
    int bz;
    int lx;
    int ly;
    int lz;
    int b;
    ItemCombatV6_GetLookVector(&dx, &dy, &dz);
    sx = playerX;
    sy = playerY + PlayerV22_GetEyeHeight();
    sz = playerZ;
    lx = -1; ly = -1; lz = -1;
    for (t = 0.0; t < RAY_DISTANCE; t += RAY_STEP) {
        bx = (int)floor(sx + dx * t);
        by = (int)floor(sy + dy * t);
        bz = (int)floor(sz + dz * t);
        b = GetBlock(bx, by, bz);
        if (b == BLOCK_AIR || b == BLOCK_WATER || b == BLOCK_LAVA || b == BLOCK_FIRE) {
            lx = bx; ly = by; lz = bz;
        } else {
            if (hitX) { *hitX = bx; }
            if (hitY) { *hitY = by; }
            if (hitZ) { *hitZ = bz; }
            if (placeX) { *placeX = lx; }
            if (placeY) { *placeY = ly; }
            if (placeZ) { *placeZ = lz; }
            return 1;
        }
    }
    return 0;
}

void ItemCombatV6_SpawnPlayerProjectile(int type, int item, double speed, int damage, double charge)
{
    double dx;
    double dy;
    double dz;
    double x;
    double y;
    double z;
    int idx;
    ItemCombatV6_GetLookVector(&dx, &dy, &dz);
    x = playerX + dx * 0.45;
    y = playerY + EYE_HEIGHT - 0.10;
    z = playerZ + dz * 0.45;
    idx = SpawnSpecialEntityV6(type, item, 0, x, y, z, dx * speed, dy * speed + 0.03, dz * speed, 0.0);
    if (idx >= 0) { g_specialEntitiesV6[idx].damage = damage; g_specialEntitiesV6[idx].meta = (int)(charge * 100.0); }
}

void ItemCombatV6_FireBowIfCharged(void)
{
    double charge;
    if (!g_bowChargingV6) { return; }
    if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0) { return; }
    charge = g_bowChargeV6 / 1.00;
    if (charge < 0.15) { charge = 0.15; }
    if (charge > 1.0) { charge = 1.0; }
    if (ItemCombatV6_ConsumeOneInventoryItem(ITEM_ARROW)) {
        ItemCombatV6_SpawnPlayerProjectile(ENTITY_V6_ARROW, ITEM_ARROW, 11.0 + charge * 14.0, 3 + (int)(charge * 5.0), charge);
        DamageHeldTool(1);
        PlayOneShotMP3("assets\\sounds\\random\\bow.mp3");
    }
    g_bowChargingV6 = 0;
    g_bowChargeV6 = 0.0;
}

double ItemCombatV6_BlockHardness(int block)
{
    float hardness;
    if (block == BLOCK_AIR || block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { return 0.0; }
    if (block == BLOCK_BEDROCK || block == BLOCK_BORDER) { return 9999.0; }
    hardness = BlockV49_GetHardness(block);
    if (hardness > 0.0f) { return (double)hardness; }
    return 1.0;
}

int ItemCombatV6_CanHarvestBlock(int block, int item)
{
    int level;
    int requiredLevel;
    int tool;
    level = GetToolHarvestLevel(item);
    if (block == BLOCK_BEDROCK || block == BLOCK_BORDER) { return 0; }
    if (block == BLOCK_AIR || block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { return 0; }
    if (block == BLOCK_WEB) { return (item == ITEM_SHEARS || ItemCombatV6_IsSword(item)); }
    if (block == BLOCK_LEAVES || block == BLOCK_WOOL || block == BLOCK_GLASS || block == BLOCK_ICE) { return 1; }
    tool = BlockV49_GetHarvestTool(block);
    requiredLevel = BlockV49_GetHarvestLevel(block);
    if (tool == BLOCK_HARVEST_NONE_V49) { return 1; }
    if (tool == BLOCK_HARVEST_PICKAXE_V49) { return ItemCombatV6_IsPickaxe(item) && level >= requiredLevel; }
    if (tool == BLOCK_HARVEST_AXE_V49) { return (item == ITEM_WOOD_AXE || item == ITEM_STONE_AXE || item == ITEM_IRON_AXE || item == ITEM_DIAMOND_AXE || item == ITEM_GOLD_AXE); }
    if (tool == BLOCK_HARVEST_SHOVEL_V49) { return (item == ITEM_WOOD_SHOVEL || item == ITEM_STONE_SHOVEL || item == ITEM_IRON_SHOVEL || item == ITEM_DIAMOND_SHOVEL || item == ITEM_GOLD_SHOVEL); }
    if (tool == BLOCK_HARVEST_SHEARS_V49) { return (item == ITEM_SHEARS || ItemCombatV6_IsSword(item)); }
    if (tool == BLOCK_HARVEST_SWORD_V49) { return ItemCombatV6_IsSword(item) || item == ITEM_SHEARS; }
    return 1;
}


int ItemCombatV6_ShouldFinishMine(int block, int x, int y, int z)
{
    double hardness;
    double speed;
    int item;
    if (block == BLOCK_BEDROCK || block == BLOCK_BORDER) { return 0; }
    item = hotbar[selectedHotbarSlot].item;
    hardness = ItemCombatV6_BlockHardness(block);
    if (hardness <= 0.0) { return 1; }
    if (g_miningX_V6 != x || g_miningY_V6 != y || g_miningZ_V6 != z) {
        g_miningX_V6 = x;
        g_miningY_V6 = y;
        g_miningZ_V6 = z;
        g_miningProgressV6 = 0.0;
    }
    speed = GetToolMiningSpeed(item, block);
    if (speed < 1.0) { speed = 1.0; }
    if (!ItemCombatV6_CanHarvestBlock(block, item) && (block == BLOCK_STONE || block == BLOCK_COBBLESTONE || block == BLOCK_COAL_ORE || block == BLOCK_IRON_ORE || block == BLOCK_GOLD_ORE || block == BLOCK_DIAMOND_ORE || block == BLOCK_REDSTONE_ORE || block == BLOCK_LAPIS_ORE || block == BLOCK_OBSIDIAN)) {
        speed *= 0.22;
    }
    g_miningProgressV6 += (0.38 * speed) / hardness;
    if (g_miningProgressV6 >= 1.0) {
        g_miningProgressV6 = 0.0;
        return 1;
    }
    return 0;
}



/* V11B_OPENWATCOM_LINKFIX: active held-left-click mining implementation.
   These functions were declared and called by the V11 input/render pass but
   were missing from the recreation C file, causing Open Watcom undefined
   references.  The implementation below keeps the Java-style behavior:
   attack must be held, the target must stay the same, crack progress grows
   with hardness/tool speed, and releasing the mouse cancels the damage. */
void CancelHeldBlockMiningV11(void)
{
    g_leftMouseMiningHeldV11 = 0;
    g_leftMouseContinuousAttackCooldownV28 = 0.0;
    ResetHeldMiningTargetV28();
}

void ResetHeldMiningTargetV28(void)
{
    g_leftMouseMineAccumulatorV11 = 0.0;
    g_leftMouseMineFxTimerV11 = 0.0;
    g_leftMouseMineHadTargetV11 = 0;
    g_miningX_V6 = -9999;
    g_miningY_V6 = -9999;
    g_miningZ_V6 = -9999;
    g_miningProgressV6 = 0.0;
}

int TryContinuousAttackMobV28(double dt)
{
    if (g_leftMouseContinuousAttackCooldownV28 > 0.0) {
        g_leftMouseContinuousAttackCooldownV28 -= dt;
        if (g_leftMouseContinuousAttackCooldownV28 > 0.0) { return 0; }
        g_leftMouseContinuousAttackCooldownV28 = 0.0;
    }
    if (AttackMobRaycast()) {
        StartHandSwing();
        ResetHeldMiningTargetV28();
        g_leftMouseContinuousAttackCooldownV28 = PLAYER_CONTINUOUS_ATTACK_DELAY_V28;
        return 1;
    }
    return 0;
}

void BeginHeldBlockMiningV11(void)
{
    g_leftMouseMiningHeldV11 = 1;
    g_leftMouseContinuousAttackCooldownV28 = 0.0;
    ResetHeldMiningTargetV28();

    /* V28: left click stays active until release.  A first click on a mob
       attacks immediately, but it no longer turns off the held mining state;
       UpdateHeldBlockMiningV11 keeps retrying attacks/mining while held. */
    TryContinuousAttackMobV28(0.0);
}

void FinishMinedBlockV11(int bx, int by, int bz, int block)
{
    int item;
    int dropCount;
    int hash;
    double vx;
    double vz;

    if (block == BLOCK_AIR || block == BLOCK_BEDROCK || block == BLOCK_BORDER) { return; }

    dropCount = 1;
    item = GetBlockDropItemAt(block, bx, by, bz, &dropCount);
    if (!ItemCombatV6_CanHarvestBlock(block, hotbar[selectedHotbarSlot].item)) {
        item = ITEM_NONE;
        dropCount = 0;
    }

    PlayBlockBreakSound(block);
    SpawnBlockBreakParticles(bx, by, bz, block);
    RemoveTileEntityAt(bx, by, bz);
    SetBlock(bx, by, bz, BLOCK_AIR);
    SpawnFallingBlockIfNeededV6(bx, by + 1, bz);
    DamageHeldTool(1);
    UpdateRedstoneAround(bx, by, bz);
    RecomputeLegacyLightingLocal(bx, by, bz, 18);

    if (item != ITEM_NONE && dropCount > 0) {
        hash = WorldHash3D(bx, by, bz, g_worldSeed + 55310);
        vx = (((hash & 255) / 255.0) - 0.5) * 1.4;
        vz = ((((hash >> 8) & 255) / 255.0) - 0.5) * 1.4;
        AddDroppedItem(item, dropCount,
                       (double)bx + 0.25 + (double)((hash >> 16) & 255) / 510.0,
                       (double)by + 0.25 + (double)((hash >> 24) & 127) / 254.0,
                       (double)bz + 0.25 + (double)((hash >> 4) & 255) / 510.0,
                       vx, 1.85, vz);
    }
}

void UpdateHeldBlockMiningV11(double dt)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int block;
    int heldItem;
    double hardness;
    double speed;
    double gain;

    (void)px;
    (void)py;
    (void)pz;

    if (!g_leftMouseMiningHeldV11) { return; }
    if (g_state != STATE_GAME || inventoryOpen || craftingOpen || g_containerModeV5 != CONTAINER_NONE_V5) {
        CancelHeldBlockMiningV11();
        return;
    }

    /* If focus is lost or Windows reports that the button is no longer held,
       cancel so the block cannot keep mining by itself. */
    if (GetForegroundWindow() != g_hwnd || (GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0) {
        CancelHeldBlockMiningV11();
        return;
    }

    if (TryContinuousAttackMobV28(dt)) {
        return;
    }

    if (!ItemCombatV6_RaycastBlock(&hx, &hy, &hz, &px, &py, &pz)) {
        ResetHeldMiningTargetV28();
        return;
    }

    block = GetBlock(hx, hy, hz);
    if (block == BLOCK_AIR || block == BLOCK_BORDER || block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) {
        ResetHeldMiningTargetV28();
        return;
    }

    if (g_miningX_V6 != hx || g_miningY_V6 != hy || g_miningZ_V6 != hz) {
        g_miningX_V6 = hx;
        g_miningY_V6 = hy;
        g_miningZ_V6 = hz;
        g_miningProgressV6 = 0.0;
        g_leftMouseMineAccumulatorV11 = 0.0;
        g_leftMouseMineFxTimerV11 = 0.0;
        g_leftMouseMineHadTargetV11 = 1;
    }

    heldItem = hotbar[selectedHotbarSlot].item;
    hardness = ItemCombatV6_BlockHardness(block);
    if (hardness >= 9999.0) { return; }
    if (hardness <= 0.0) { hardness = 0.05; }

    speed = GetToolMiningSpeed(heldItem, block);
    if (speed < 1.0) { speed = 1.0; }
    if (!ItemCombatV6_CanHarvestBlock(block, heldItem) &&
        (block == BLOCK_STONE || block == BLOCK_COBBLESTONE || block == BLOCK_COAL_ORE ||
         block == BLOCK_IRON_ORE || block == BLOCK_GOLD_ORE || block == BLOCK_DIAMOND_ORE ||
         block == BLOCK_REDSTONE_ORE || block == BLOCK_LAPIS_ORE || block == BLOCK_OBSIDIAN)) {
        speed *= 0.22;
    }

    /* dt-based progress: softer blocks break quickly; stone/ores require a
       hold; obsidian remains slow.  The crack overlay reads g_miningProgressV6. */
    gain = (speed * dt) / (hardness * 1.35);
    if (gain < 0.0) { gain = 0.0; }
    if (gain > 0.35) { gain = 0.35; }
    g_miningProgressV6 += gain;

    g_leftMouseMineFxTimerV11 += dt;
    if (g_leftMouseMineFxTimerV11 >= 0.22) {
        g_leftMouseMineFxTimerV11 = 0.0;
        StartHandSwing();
        SpawnBlockBreakParticles(hx, hy, hz, block);
    }

    if (g_miningProgressV6 >= 1.0) {
        FinishMinedBlockV11(hx, hy, hz, block);
        ResetHeldMiningTargetV28();
        g_leftMouseContinuousAttackCooldownV28 = PLAYER_CONTINUOUS_AIR_RETRY_V28;
        StartHandSwing();
    }
}

int ItemCombatV6_TryUseBucket(int item)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int b;
    InventorySlot *slot;
    slot = &hotbar[selectedHotbarSlot];
    if (!ItemCombatV6_RaycastBlock(&hx, &hy, &hz, &px, &py, &pz)) { return 0; }
    b = GetBlock(hx, hy, hz);
    if (item == ITEM_BUCKET) {
        if (b == BLOCK_WATER && ((int)g_blockMeta[hx][hy][hz] & 15) == 0) {
            SetBlock(hx, hy, hz, BLOCK_AIR);
            slot->item = ITEM_WATER_BUCKET;
            slot->count = 1;
            RecomputeLegacyLightingLocal(hx, hy, hz, 10);
            return 1;
        }
        if ((b == BLOCK_LAVA || b == BLOCK_STATIONARY_LAVA) && ((int)g_blockMeta[hx][hy][hz] & 15) == 0) {
            SetBlock(hx, hy, hz, BLOCK_AIR);
            slot->item = ITEM_LAVA_BUCKET;
            slot->count = 1;
            RecomputeLegacyLightingLocal(hx, hy, hz, 10);
            return 1;
        }
    }
    if ((item == ITEM_WATER_BUCKET || item == ITEM_LAVA_BUCKET) && px >= 0) {
        if (GetBlock(px, py, pz) == BLOCK_AIR || GetBlock(px, py, pz) == BLOCK_FIRE) {
            FluidV42_SetBlockWithMeta(px, py, pz, item == ITEM_WATER_BUCKET ? BLOCK_WATER : BLOCK_LAVA, 0);
            FluidV42_MixWaterLavaAround(px, py, pz);
            slot->item = ITEM_BUCKET;
            slot->count = 1;
            RecomputeLegacyLightingLocal(px, py, pz, 10);
            return 1;
        }
    }
    return 0;
}

int ItemCombatV6_TryUseHoe(int item)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int b;
    (void)item;
    if (!ItemCombatV6_RaycastBlock(&hx, &hy, &hz, &px, &py, &pz)) { return 0; }
    b = GetBlock(hx, hy, hz);
    if ((b == BLOCK_GRASS || b == BLOCK_DIRT) && GetBlock(hx, hy + 1, hz) == BLOCK_AIR) {
        SetBlock(hx, hy, hz, BLOCK_FARMLAND);
        DamageHeldTool(1);
        PlayOneShotMP3("assets\\sounds\\step\\grass1.mp3");
        InvalidateTerrainChunkMeshAt(hx, hz);
        return 1;
    }
    return 0;
}

int ItemCombatV6_TryUseFlintSteel(void)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int b;
    if (!ItemCombatV6_RaycastBlock(&hx, &hy, &hz, &px, &py, &pz)) { return 0; }
    b = GetBlock(hx, hy, hz);
    if (b == BLOCK_TNT) {
        SetBlock(hx, hy, hz, BLOCK_AIR);
        SpawnSpecialEntityV6(ENTITY_V6_TNT, ITEM_TNT, BLOCK_TNT, hx + 0.5, hy + 0.2, hz + 0.5, 0.0, 3.0, 0.0, 3.7);
        DamageHeldTool(1);
        return 1;
    }
    if (px >= 0 && GetBlock(px, py, pz) == BLOCK_AIR) {
        SetBlock(px, py, pz, BLOCK_FIRE);
        if (IsInsideWorld(px, py, pz)) { g_blockMeta[px][py][pz] = 0; }
        DamageHeldTool(1);
        RecomputeLegacyLightingLocal(px, py, pz, 8);
        PlayOneShotMP3("assets\\sounds\\fire\\fire.mp3");
        return 1;
    }
    return 0;
}

int ItemCombatV6_TryUseShears(void)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int b;
    if (!ItemCombatV6_RaycastBlock(&hx, &hy, &hz, &px, &py, &pz)) { return 0; }
    b = GetBlock(hx, hy, hz);
    if (b == BLOCK_LEAVES || b == BLOCK_WEB || b == BLOCK_WOOL) {
        AddDroppedItem(BlockToItem(b), 1, hx + 0.5, hy + 0.5, hz + 0.5, 0.0, 1.0, 0.0);
        SetBlock(hx, hy, hz, BLOCK_AIR);
        DamageHeldTool(1);
        return 1;
    }
    return 0;
}

int ItemCombatV6_TryUseBoat(void)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int b;
    if (!ItemCombatV6_RaycastBlock(&hx, &hy, &hz, &px, &py, &pz)) { return 0; }
    b = GetBlock(hx, hy, hz);
    if (b == BLOCK_WATER || (px >= 0 && GetBlock(px, py, pz) == BLOCK_WATER)) {
        if (b == BLOCK_WATER) { px = hx; py = hy + 1; pz = hz; }
        SpawnSpecialEntityV6(ENTITY_V6_BOAT, ITEM_BOAT, 0, px + 0.5, py + 0.15, pz + 0.5, -sin(yaw * PI / 180.0) * 1.2, 0.0, -cos(yaw * PI / 180.0) * 1.2, 0.0);
        RemoveItemFromSelectedHotbar(1);
        return 1;
    }
    return 0;
}

int ItemCombatV6_TryUsePainting(void)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int b;
    int idx;
    if (!ItemCombatV6_RaycastBlock(&hx, &hy, &hz, &px, &py, &pz)) { return 0; }
    b = GetBlock(hx, hy, hz);
    if (IsSolidBlock(b) && px >= 0 && GetBlock(px, py, pz) == BLOCK_AIR) {
        idx = SpawnSpecialEntityV6(ENTITY_V6_PAINTING, ITEM_PAINTING, 0, px + 0.5, py + 0.5, pz + 0.5, 0.0, 0.0, 0.0, 0.0);
        if (idx >= 0) { g_specialEntitiesV6[idx].meta = (int)(yaw / 90.0) & 3; }
        RemoveItemFromSelectedHotbar(1);
        return 1;
    }
    return 0;
}

int ItemCombatV6_TryUseFishingRod(void)
{
    double dx;
    double dy;
    double dz;
    int idx;
    if (g_fishingHookIndexV6 >= 0 && g_fishingHookIndexV6 < MAX_SPECIAL_ENTITIES_V6 && g_specialEntitiesV6[g_fishingHookIndexV6].active) {
        if (GetBlock((int)floor(g_specialEntitiesV6[g_fishingHookIndexV6].x), (int)floor(g_specialEntitiesV6[g_fishingHookIndexV6].y), (int)floor(g_specialEntitiesV6[g_fishingHookIndexV6].z)) == BLOCK_WATER && g_specialEntitiesV6[g_fishingHookIndexV6].age > 2.0) {
            AddDroppedItem(ITEM_FISH_RAW, 1, playerX, playerY + 1.0, playerZ, 0.0, 1.8, 0.0);
        }
        g_specialEntitiesV6[g_fishingHookIndexV6].active = 0;
        g_fishingHookIndexV6 = -1;
        DamageHeldTool(1);
        return 1;
    }
    ItemCombatV6_GetLookVector(&dx, &dy, &dz);
    idx = SpawnSpecialEntityV6(ENTITY_V6_FISH_HOOK, ITEM_FISHING_ROD, 0, playerX + dx * 0.3, playerY + EYE_HEIGHT, playerZ + dz * 0.3, dx * 8.0, dy * 8.0 + 1.0, dz * 8.0, 0.0);
    g_fishingHookIndexV6 = idx;
    DamageHeldTool(1);
    return 1;
}

int ItemCombatV6_TryUseRecord(int item)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    if (item != ITEM_RECORD_13 && item != ITEM_RECORD_CAT) { return 0; }
    if (!ItemCombatV6_RaycastBlock(&hx, &hy, &hz, &px, &py, &pz)) { return 0; }
    if (GetBlock(hx, hy, hz) == BLOCK_JUKEBOX) {
        if (item == ITEM_RECORD_13) { PlayOneShotMP3("assets\\records\\13.mp3"); }
        else { PlayOneShotMP3("assets\\records\\cat.mp3"); }
        RemoveItemFromSelectedHotbar(1);
        return 1;
    }
    return 0;
}

int ItemCombatV6_TryUseMap(void)
{
    if (hotbar[selectedHotbarSlot].item != ITEM_MAP) { return 0; }
    g_mapMadeV6 = 1;
    g_mapCenterXGlobalV6 = (int)GetPlayerGlobalX();
    g_mapCenterZGlobalV6 = (int)GetPlayerGlobalZ();
    PlayUIClickSound();
    return 1;
}


/* ------------------------------------------------------------ */
/* V37 full item use behavior                                   */
/* Java reference path: Item.java, ItemTool.java, ItemSword.java,*/
/* ItemBow.java, ItemBucket.java, ItemHoe.java, ItemShears.java, */
/* ItemFlintAndSteel.java, ItemFood.java, ItemDoor.java,         */
/* ItemBoat.java, ItemMinecart.java, ItemPainting.java,          */
/* ItemFishingRod.java, ItemRecord.java.                         */
/* ------------------------------------------------------------ */

int ItemV37_IsLiquidBlock(int block)
{
    return block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA;
}

int ItemV37_IsReplaceableForUse(int block)
{
    if (block == BLOCK_AIR || block == BLOCK_FIRE || block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { return 1; }
    if (block == BLOCK_TALL_GRASS || block == BLOCK_SNOW) { return 1; }
    return 0;
}

int ItemV37_RaycastUse(int includeLiquids, int *hitX, int *hitY, int *hitZ, int *placeX, int *placeY, int *placeZ, int *sideOut)
{
    double dx;
    double dy;
    double dz;
    double sx;
    double sy;
    double sz;
    double t;
    int bx;
    int by;
    int bz;
    int lx;
    int ly;
    int lz;
    int b;
    int side;

    ItemCombatV6_GetLookVector(&dx, &dy, &dz);
    sx = playerX;
    sy = playerY + PlayerV22_GetEyeHeight();
    sz = playerZ;
    lx = -1; ly = -1; lz = -1; side = -1;

    for (t = 0.0; t < RAY_DISTANCE; t += RAY_STEP) {
        bx = (int)floor(sx + dx * t);
        by = (int)floor(sy + dy * t);
        bz = (int)floor(sz + dz * t);
        b = GetBlock(bx, by, bz);
        if (b == BLOCK_BORDER) { return 0; }
        if (b == BLOCK_AIR || b == BLOCK_FIRE || (!includeLiquids && ItemV37_IsLiquidBlock(b))) {
            lx = bx; ly = by; lz = bz;
            continue;
        }
        if (includeLiquids || !ItemV37_IsLiquidBlock(b)) {
            if (hitX) { *hitX = bx; }
            if (hitY) { *hitY = by; }
            if (hitZ) { *hitZ = bz; }
            if (placeX) { *placeX = lx; }
            if (placeY) { *placeY = ly; }
            if (placeZ) { *placeZ = lz; }
            if (lx == bx && ly == by - 1 && lz == bz) { side = 0; }
            else if (lx == bx && ly == by + 1 && lz == bz) { side = 1; }
            else if (lx == bx && ly == by && lz == bz - 1) { side = 2; }
            else if (lx == bx && ly == by && lz == bz + 1) { side = 3; }
            else if (lx == bx - 1 && ly == by && lz == bz) { side = 4; }
            else if (lx == bx + 1 && ly == by && lz == bz) { side = 5; }
            if (sideOut) { *sideOut = side; }
            return 1;
        }
    }
    return 0;
}

int ItemV37_RaycastMobIndex(int desiredType)
{
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
    double bestT;
    int best;
    int i;

    ItemCombatV6_GetLookVector(&dirX, &dirY, &dirZ);
    startX = playerX;
    startY = playerY + PlayerV22_GetEyeHeight();
    startZ = playerZ;
    best = -1;
    bestT = 9999.0;

    for (t = 0.0; t < RAY_DISTANCE; t += 0.15) {
        px = startX + dirX * t;
        py = startY + dirY * t;
        pz = startZ + dirZ * t;
        for (i = 0; i < MAX_MOBS; i++) {
            if (!mobs[i].active) { continue; }
            if (desiredType >= 0 && mobs[i].type != desiredType) { continue; }
            radius = (double)MobWidth(mobs[i].type) * 0.58 + 0.12;
            dx = px - mobs[i].x;
            dy = py - (mobs[i].y + (double)MobHeight(mobs[i].type) * 0.52);
            dz = pz - mobs[i].z;
            if (dx * dx + dy * dy + dz * dz <= radius * radius && t < bestT) { best = i; bestT = t; }
        }
    }
    return best;
}

int ItemV37_PlayerHasItem(int item)
{
    int i;
    for (i = 0; i < HOTBAR_SLOTS; i++) { if (hotbar[i].item == item && hotbar[i].count > 0) { return 1; } }
    for (i = 0; i < INVENTORY_SLOTS; i++) { if (inventory[i].item == item && inventory[i].count > 0) { return 1; } }
    return 0;
}

void ItemV37_SetHeldStack(int item, int count, int damage)
{
    InventorySlot *slot;
    slot = &hotbar[selectedHotbarSlot];
    slot->item = item;
    slot->count = count;
    slot->damage = damage;
    if (slot->count <= 0 || item == ITEM_NONE) { slot->item = ITEM_NONE; slot->count = 0; slot->damage = 0; }
}

int ItemV37_UseEmptyBucketOnCow(void)
{
    int mobIndex;
    mobIndex = ItemV37_RaycastMobIndex(MOB_COW);
    if (mobIndex < 0) { return 0; }
    ItemV37_SetHeldStack(ITEM_MILK_BUCKET, 1, 0);
    StartHandUse();
    PlayItemPickupSound();
    return 1;
}

int ItemV37_TryUseBucket(int item)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int side;
    int b;
    int placeBlock;
    InventorySlot *slot;

    slot = &hotbar[selectedHotbarSlot];
    if (item == ITEM_MILK_BUCKET) {
        ItemV37_SetHeldStack(ITEM_BUCKET, 1, 0);
        StartHandUse();
        PlayOneShotMP3("assets\\sounds\\random\\drink.mp3");
        return 1;
    }
    if (item == ITEM_BUCKET && ItemV37_UseEmptyBucketOnCow()) { return 1; }

    if (item == ITEM_BUCKET) {
        if (!ItemV37_RaycastUse(1, &hx, &hy, &hz, &px, &py, &pz, &side)) { return 0; }
        b = GetBlock(hx, hy, hz);
        if (b == BLOCK_WATER && ((int)g_blockMeta[hx][hy][hz] & 15) == 0) {
            SetBlock(hx, hy, hz, BLOCK_AIR);
            ItemV37_SetHeldStack(ITEM_WATER_BUCKET, 1, 0);
            RecomputeLegacyLightingLocal(hx, hy, hz, 10);
            PlaySoundAtV35("assets\\sounds\\liquid\\splash.wav", (double)hx + 0.5, (double)hy + 0.5, (double)hz + 0.5, 0.45, 1.0, SOUND_DEFAULT_RANGE_V35);
            return 1;
        }
        if ((b == BLOCK_LAVA || b == BLOCK_STATIONARY_LAVA) && ((int)g_blockMeta[hx][hy][hz] & 15) == 0) {
            SetBlock(hx, hy, hz, BLOCK_AIR);
            ItemV37_SetHeldStack(ITEM_LAVA_BUCKET, 1, 0);
            RecomputeLegacyLightingLocal(hx, hy, hz, 10);
            PlaySoundAtV35("assets\\sounds\\random\\fizz.wav", (double)hx + 0.5, (double)hy + 0.5, (double)hz + 0.5, 0.50, 1.0, SOUND_DEFAULT_RANGE_V35);
            return 1;
        }
        return 0;
    }

    if (item != ITEM_WATER_BUCKET && item != ITEM_LAVA_BUCKET) { return 0; }
    if (!ItemV37_RaycastUse(0, &hx, &hy, &hz, &px, &py, &pz, &side)) { return 0; }
    if (side == 0) { py = hy - 1; px = hx; pz = hz; }
    else if (side == 1) { py = hy + 1; px = hx; pz = hz; }
    else if (side == 2) { pz = hz - 1; px = hx; py = hy; }
    else if (side == 3) { pz = hz + 1; px = hx; py = hy; }
    else if (side == 4) { px = hx - 1; py = hy; pz = hz; }
    else if (side == 5) { px = hx + 1; py = hy; pz = hz; }
    if (!IsInsideWorld(px, py, pz)) { return 0; }
    b = GetBlock(px, py, pz);
    if (b != BLOCK_AIR && b != BLOCK_FIRE && b != BLOCK_WATER && b != BLOCK_LAVA && b != BLOCK_STATIONARY_LAVA) { return 0; }
    placeBlock = (item == ITEM_WATER_BUCKET) ? BLOCK_WATER : BLOCK_LAVA;
    FluidV42_SetBlockWithMeta(px, py, pz, placeBlock, 0);
    FluidV42_MixWaterLavaAround(px, py, pz);
    slot->item = ITEM_BUCKET;
    slot->count = 1;
    slot->damage = 0;
    RecomputeLegacyLightingLocal(px, py, pz, 12);
    if (placeBlock == BLOCK_WATER) { PlaySoundAtV35("assets\\sounds\\liquid\\splash.wav", (double)px + 0.5, (double)py + 0.5, (double)pz + 0.5, 0.45, 1.0, SOUND_DEFAULT_RANGE_V35); }
    else { PlaySoundAtV35("assets\\sounds\\liquid\\lava.wav", (double)px + 0.5, (double)py + 0.5, (double)pz + 0.5, 0.45, 1.0, SOUND_DEFAULT_RANGE_V35); }
    StartHandUse();
    return 1;
}

int ItemV37_TryUseHoe(int item)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int side;
    int b;
    (void)item;
    if (!ItemV37_RaycastUse(0, &hx, &hy, &hz, &px, &py, &pz, &side)) { return 0; }
    b = GetBlock(hx, hy, hz);
    if (side != 0 && GetBlock(hx, hy + 1, hz) == BLOCK_AIR && (b == BLOCK_GRASS || b == BLOCK_DIRT)) {
        SetBlock(hx, hy, hz, BLOCK_FARMLAND);
        DamageHeldTool(1);
        PlaySoundAtV35("assets\\sounds\\step\\grass1.mp3", (double)hx + 0.5, (double)hy + 0.5, (double)hz + 0.5, 0.55, 0.8, SOUND_DEFAULT_RANGE_V35);
        InvalidateTerrainChunkMeshAt(hx, hz);
        StartHandUse();
        return 1;
    }
    return 0;
}

int ItemV37_TryUseFlintSteel(void)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int side;
    int b;
    if (!ItemV37_RaycastUse(0, &hx, &hy, &hz, &px, &py, &pz, &side)) { return 0; }
    b = GetBlock(hx, hy, hz);
    if (b == BLOCK_TNT) {
        SetBlock(hx, hy, hz, BLOCK_AIR);
        SpawnSpecialEntityV6(ENTITY_V6_TNT, ITEM_TNT, BLOCK_TNT, hx + 0.5, hy + 0.2, hz + 0.5, 0.0, 3.0, 0.0, 3.7);
        PlaySoundAtV35("assets\\sounds\\random\\fuse.wav", (double)hx + 0.5, (double)hy + 0.5, (double)hz + 0.5, 0.45, 1.0, SOUND_DEFAULT_RANGE_V35);
        DamageHeldTool(1);
        StartHandUse();
        return 1;
    }
    if (side == 0) { px = hx; py = hy - 1; pz = hz; }
    else if (side == 1) { px = hx; py = hy + 1; pz = hz; }
    else if (side == 2) { px = hx; py = hy; pz = hz - 1; }
    else if (side == 3) { px = hx; py = hy; pz = hz + 1; }
    else if (side == 4) { px = hx - 1; py = hy; pz = hz; }
    else if (side == 5) { px = hx + 1; py = hy; pz = hz; }
    if (IsInsideWorld(px, py, pz) && GetBlock(px, py, pz) == BLOCK_AIR) {
        SetBlock(px, py, pz, BLOCK_FIRE);
        RecomputeLegacyLightingLocal(px, py, pz, 8);
        PlaySoundAtV35("assets\\sounds\\fire\\ignite.wav", (double)px + 0.5, (double)py + 0.5, (double)pz + 0.5, 1.0, SoundRandomRangeV35(0.8, 1.2), SOUND_DEFAULT_RANGE_V35);
        StartHandUse();
    }
    DamageHeldTool(1);
    return 1;
}

int ItemV37_TryUseShears(void)
{
    int mobIndex;
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int side;
    int b;
    (void)px; (void)py; (void)pz; (void)side;
    mobIndex = ItemV37_RaycastMobIndex(MOB_SHEEP);
    if (mobIndex >= 0 && TryMobInteractJavaV4(mobIndex)) { StartHandUse(); return 1; }
    if (!ItemV37_RaycastUse(0, &hx, &hy, &hz, &px, &py, &pz, &side)) { return 0; }
    b = GetBlock(hx, hy, hz);
    if (b == BLOCK_LEAVES || b == BLOCK_WEB || b == BLOCK_WOOL) {
        AddDroppedItem(b == BLOCK_WEB ? BLOCK_WEB : BlockToItem(b), 1, hx + 0.5, hy + 0.5, hz + 0.5, 0.0, 1.0, 0.0);
        SetBlock(hx, hy, hz, BLOCK_AIR);
        DamageHeldTool(1);
        SpawnBlockBreakParticles(hx, hy, hz, b);
        StartHandUse();
        return 1;
    }
    return 0;
}

int ItemV37_TryUseDoor(int item)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int side;
    int placeX;
    int placeY;
    int placeZ;
    int i1;
    int offX;
    int offZ;
    int leftSolid;
    int rightSolid;
    int leftDoor;
    int rightDoor;
    int mirror;
    unsigned char meta;

    if (item != ITEM_WOOD_DOOR && item != ITEM_IRON_DOOR) { return 0; }
    if (!ItemV37_RaycastUse(0, &hx, &hy, &hz, &px, &py, &pz, &side)) { return 0; }
    if (side != 1) { return 0; }
    placeX = hx;
    placeY = hy + 1;
    placeZ = hz;
    if (!PlayerV22_CanPlaceBlockAt(BLOCK_WOOD_DOOR, placeX, placeY, placeZ, hx, hy, hz)) { return 0; }
    i1 = (int)floor((((yaw + 180.0) * 4.0) / 360.0) - 0.5) & 3;
    offX = 0; offZ = 0;
    if (i1 == 0) { offZ = 1; }
    if (i1 == 1) { offX = -1; }
    if (i1 == 2) { offZ = -1; }
    if (i1 == 3) { offX = 1; }
    leftSolid = (IsSolidBlock(GetBlock(placeX - offX, placeY, placeZ - offZ)) ? 1 : 0) + (IsSolidBlock(GetBlock(placeX - offX, placeY + 1, placeZ - offZ)) ? 1 : 0);
    rightSolid = (IsSolidBlock(GetBlock(placeX + offX, placeY, placeZ + offZ)) ? 1 : 0) + (IsSolidBlock(GetBlock(placeX + offX, placeY + 1, placeZ + offZ)) ? 1 : 0);
    leftDoor = (GetBlock(placeX - offX, placeY, placeZ - offZ) == BLOCK_WOOD_DOOR || GetBlock(placeX - offX, placeY + 1, placeZ - offZ) == BLOCK_WOOD_DOOR);
    rightDoor = (GetBlock(placeX + offX, placeY, placeZ + offZ) == BLOCK_WOOD_DOOR || GetBlock(placeX + offX, placeY + 1, placeZ + offZ) == BLOCK_WOOD_DOOR);
    mirror = 0;
    if (leftDoor && !rightDoor) { mirror = 1; }
    else if (rightSolid > leftSolid) { mirror = 1; }
    if (mirror) { i1 = (i1 - 1) & 3; i1 += 4; }
    meta = (unsigned char)i1;
    if (item == ITEM_IRON_DOOR) { meta |= 32; }
    SetBlock(placeX, placeY, placeZ, BLOCK_WOOD_DOOR);
    g_blockMeta[placeX][placeY][placeZ] = meta;
    SetBlock(placeX, placeY + 1, placeZ, BLOCK_WOOD_DOOR);
    g_blockMeta[placeX][placeY + 1][placeZ] = (unsigned char)(meta | 8);
    RemoveItemFromSelectedHotbar(1);
    InvalidateTerrainChunkMeshAt(placeX, placeZ);
    PlayDoorSoundV35((double)placeX + 0.5, (double)placeY + 0.5, (double)placeZ + 0.5, 1);
    StartHandUse();
    return 1;
}

int ItemV37_TryUseMinecart(int item)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int side;
    int b;
    (void)px; (void)py; (void)pz; (void)side;
    if (item != ITEM_MINECART && item != ITEM_CHEST_MINECART && item != ITEM_FURNACE_MINECART) { return 0; }
    if (!ItemV37_RaycastUse(0, &hx, &hy, &hz, &px, &py, &pz, &side)) { return 0; }
    b = GetBlock(hx, hy, hz);
    if (b != BLOCK_RAIL && b != BLOCK_DETECTOR_RAIL) { return 0; }
    if (!SpawnMinecartV5((double)hx + 0.5, (double)hy + 0.55, (double)hz + 0.5, 0.0, 0.0)) { return 0; }
    RemoveItemFromSelectedHotbar(1);
    PlaySoundAtV35("assets\\sounds\\minecart\\base.wav", (double)hx + 0.5, (double)hy + 0.5, (double)hz + 0.5, 0.50, 1.0, SOUND_DEFAULT_RANGE_V35);
    StartHandUse();
    return 1;
}

int ItemV37_TryUseBoat(void)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int side;
    int b;
    int spawnY;
    int idx;
    double dx;
    double dy;
    double dz;
    (void)px; (void)py; (void)pz; (void)side;
    if (!ItemV37_RaycastUse(1, &hx, &hy, &hz, &px, &py, &pz, &side)) { return 0; }
    b = GetBlock(hx, hy, hz);
    spawnY = hy + 1;
    if (b == BLOCK_WATER || b == BLOCK_LAVA || b == BLOCK_STATIONARY_LAVA) { spawnY = hy + 1; }
    if (!IsInsideWorld(hx, spawnY, hz) || !ItemV37_IsReplaceableForUse(GetBlock(hx, spawnY, hz))) { return 0; }
    ItemCombatV6_GetLookVector(&dx, &dy, &dz);
    idx = SpawnSpecialEntityV6(ENTITY_V6_BOAT, ITEM_BOAT, 0, hx + 0.5, spawnY + 0.10, hz + 0.5, dx * 0.20, 0.0, dz * 0.20, 0.0);
    if (idx < 0) { return 0; }
    g_specialEntitiesV6[idx].yaw = yaw;
    RemoveItemFromSelectedHotbar(1);
    StartHandUse();
    return 1;
}

int ItemV37_TryUsePainting(void)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int side;
    int facing;
    int idx;
    double ox;
    double oz;
    if (!ItemV37_RaycastUse(0, &hx, &hy, &hz, &px, &py, &pz, &side)) { return 0; }
    if (side == 0 || side == 1) { return 0; }
    if (!IsSolidBlock(GetBlock(hx, hy, hz))) { return 0; }
    facing = 0; ox = 0.0; oz = 0.0;
    if (side == 4) { facing = 1; ox = -0.51; }
    else if (side == 3) { facing = 2; oz = 0.51; }
    else if (side == 5) { facing = 3; ox = 0.51; }
    else if (side == 2) { facing = 0; oz = -0.51; }
    idx = SpawnSpecialEntityV6(ENTITY_V6_PAINTING, ITEM_PAINTING, 0, (double)hx + 0.5 + ox, (double)hy + 0.5, (double)hz + 0.5 + oz, 0.0, 0.0, 0.0, 0.0);
    if (idx < 0) { return 0; }
    g_specialEntitiesV6[idx].meta = facing;
    RemoveItemFromSelectedHotbar(1);
    StartHandUse();
    return 1;
}

int ItemV37_TryUseFishingRod(void)
{
    double dx;
    double dy;
    double dz;
    int idx;
    SpecialEntityV6 *hook;
    if (g_fishingHookIndexV6 >= 0 && g_fishingHookIndexV6 < MAX_SPECIAL_ENTITIES_V6 && g_specialEntitiesV6[g_fishingHookIndexV6].active) {
        hook = &g_specialEntitiesV6[g_fishingHookIndexV6];
        if (hook->type == ENTITY_V6_FISH_HOOK && hook->meta == 2) {
            AddDroppedItem(ITEM_FISH_RAW, 1, playerX, playerY + 1.0, playerZ, 0.0, 1.8, 0.0);
            g_lastFishCatchReadyV37 = 1;
        } else { g_lastFishCatchReadyV37 = 0; }
        hook->active = 0;
        g_fishingHookIndexV6 = -1;
        DamageHeldTool(g_lastFishCatchReadyV37 ? 1 : 0);
        StartHandSwing();
        return 1;
    }
    ItemCombatV6_GetLookVector(&dx, &dy, &dz);
    idx = SpawnSpecialEntityV6(ENTITY_V6_FISH_HOOK, ITEM_FISHING_ROD, 0, playerX + dx * 0.3, playerY + EYE_HEIGHT, playerZ + dz * 0.3, dx * 8.0, dy * 8.0 + 1.0, dz * 8.0, 0.0);
    if (idx >= 0) {
        g_specialEntitiesV6[idx].meta = 0;
        g_specialEntitiesV6[idx].fuse = 0.0;
        g_fishingHookIndexV6 = idx;
    }
    PlaySoundAtV35("assets\\sounds\\random\\bow.wav", playerX, playerY + 1.0, playerZ, 0.50, 0.4, SOUND_DEFAULT_RANGE_V35);
    StartHandSwing();
    return 1;
}


void ItemV37_PlayRecordFile(const char *filename)
{
    char path[260];
    char cmd[512];
    const char *deviceType;
    int len;
    MCIERROR err;
    mciSendString("stop recordMusicV37", NULL, 0, NULL);
    mciSendString("close recordMusicV37", NULL, 0, NULL);
    if (!SoundResolveExistingPathV35(filename, path)) { g_soundFailedThisFrameV35++; return; }
    len = (int)strlen(path);
    deviceType = "mpegvideo";
    if (len > 4 && strcmp(path + len - 4, ".wav") == 0) { deviceType = "waveaudio"; }
    wsprintf(cmd, "open \"%s\" type %s alias recordMusicV37", path, deviceType);
    err = mciSendString(cmd, NULL, 0, NULL);
    if (err != 0) { g_soundFailedThisFrameV35++; return; }
    SoundApplyMciControlsV35("recordMusicV37", 1.0, 1.0, 1);
    err = mciSendString("play recordMusicV37", NULL, 0, NULL);
    if (err != 0) { mciSendString("close recordMusicV37", NULL, 0, NULL); g_soundFailedThisFrameV35++; }
}

int ItemV37_TryUseRecord(int item)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int side;
    if (item != ITEM_RECORD_13 && item != ITEM_RECORD_CAT) { return 0; }
    if (!ItemV37_RaycastUse(0, &hx, &hy, &hz, &px, &py, &pz, &side)) { return 0; }
    if (GetBlock(hx, hy, hz) != BLOCK_JUKEBOX) { return 0; }
    if (g_blockMeta[hx][hy][hz] != 0) { return 1; }
    g_blockMeta[hx][hy][hz] = (unsigned char)(item == ITEM_RECORD_CAT ? 2 : 1);
    if (item == ITEM_RECORD_13) { ItemV37_PlayRecordFile("assets\\records\\13.mp3"); }
    else { ItemV37_PlayRecordFile("assets\\records\\cat.mp3"); }
    RemoveItemFromSelectedHotbar(1);
    StartHandUse();
    return 1;
}


int TryEatHeldFoodV13B(void)
{
    InventorySlot *slot;
    int heal;
    int item;

    if (selectedHotbarSlot < 0 || selectedHotbarSlot >= HOTBAR_SLOTS) { return 0; }
    slot = &hotbar[selectedHotbarSlot];
    if (slot->item == ITEM_NONE || slot->count <= 0) { return 0; }

    item = slot->item;
    heal = JavaCompat_GetFoodHealPoints(item);
    if (heal <= 0) { return 0; }

    /* V37: ItemFood.onItemRightClick consumes immediately and heals direct
       health, matching the classic no-hunger behavior requested by the PDF. */
    HealPlayer(heal);
    RemoveItemFromSelectedHotbar(1);
    if (item == ITEM_MUSHROOM_STEW) { AddItemToInventory(ITEM_BOWL, 1); }
    StartHandUse();
    PlayOneShotMP3("assets\\sounds\\random\\eat.mp3");
    return 1;
}


int ItemCombatV6_TryUseSelectedItem(void)
{
    InventorySlot *slot;
    int item;
    int mobIndex;
    slot = &hotbar[selectedHotbarSlot];
    item = slot->item;
    if (item == ITEM_NONE || slot->count <= 0) { return 0; }

    /* First let Java-style entity interactions consume the click: shears on
       sheep, bones/meat on wolves, and milk bucket on cows. */
    mobIndex = ItemV37_RaycastMobIndex(-1);
    if (mobIndex >= 0) {
        if (item == ITEM_BUCKET && mobs[mobIndex].type == MOB_COW) { return ItemV37_UseEmptyBucketOnCow(); }
        if (TryMobInteractJavaV4(mobIndex)) { StartHandUse(); return 1; }
    }

    if (TryEatHeldFoodV13B()) { return 1; }
    if (ItemCombatV6_TryEquipArmor()) { return 1; }
    if (item == ITEM_BOW) {
        if (!ItemV37_PlayerHasItem(ITEM_ARROW)) { PlayUIClickSound(); return 1; }
        g_bowChargingV6 = 1;
        g_bowChargeV6 = 0.0;
        StartHandUse();
        return 1;
    }
    if (item == ITEM_EGG) { ItemCombatV6_SpawnPlayerProjectile(ENTITY_V6_EGG, ITEM_EGG, 12.0, 0, 0.0); RemoveItemFromSelectedHotbar(1); StartHandSwing(); return 1; }
    if (item == ITEM_SNOWBALL) { ItemCombatV6_SpawnPlayerProjectile(ENTITY_V6_SNOWBALL, ITEM_SNOWBALL, 13.0, 0, 0.0); RemoveItemFromSelectedHotbar(1); StartHandSwing(); return 1; }
    if (item == ITEM_FIRE_CHARGE_PLACEHOLDER) { ItemCombatV6_SpawnPlayerProjectile(ENTITY_V6_FIREBALL, item, 9.0, 4, 0.0); RemoveItemFromSelectedHotbar(1); StartHandSwing(); return 1; }
    if (item == ITEM_BUCKET || item == ITEM_WATER_BUCKET || item == ITEM_LAVA_BUCKET || item == ITEM_MILK_BUCKET) { return ItemV37_TryUseBucket(item); }
    if (ItemCombatV6_IsHoe(item)) { return ItemV37_TryUseHoe(item); }
    if (item == ITEM_FLINT_STEEL) { return ItemV37_TryUseFlintSteel(); }
    if (item == ITEM_SHEARS) { return ItemV37_TryUseShears(); }
    if (item == ITEM_WOOD_DOOR || item == ITEM_IRON_DOOR) { return ItemV37_TryUseDoor(item); }
    if (item == ITEM_BOAT) { return ItemV37_TryUseBoat(); }
    if (item == ITEM_MINECART || item == ITEM_CHEST_MINECART || item == ITEM_FURNACE_MINECART) { return ItemV37_TryUseMinecart(item); }
    if (item == ITEM_PAINTING) { return ItemV37_TryUsePainting(); }
    if (item == ITEM_FISHING_ROD) { return ItemV37_TryUseFishingRod(); }
    if (item == ITEM_MAP) { return ItemCombatV6_TryUseMap(); }
    if (ItemV37_TryUseRecord(item)) { return 1; }
    return 0;
}


void SpawnExplosionV6(double x, double y, double z, double radius, int fire)
{
    int ix;
    int iy;
    int iz;
    int dx;
    int dy;
    int dz;
    int bx;
    int by;
    int bz;
    double d;
    double d2;
    SpawnExplosionParticlesV24(x, y, z, g_videoParticlesV7 ? 18 : 7);
    ix = (int)floor(x);
    iy = (int)floor(y);
    iz = (int)floor(z);
    for (dx = -(int)radius - 1; dx <= (int)radius + 1; dx++) {
        for (dy = -(int)radius - 1; dy <= (int)radius + 1; dy++) {
            for (dz = -(int)radius - 1; dz <= (int)radius + 1; dz++) {
                bx = ix + dx;
                by = iy + dy;
                bz = iz + dz;
                if (!IsInsideWorld(bx, by, bz)) { continue; }
                d2 = (double)(dx * dx + dy * dy + dz * dz);
                if (d2 <= radius * radius && GetBlock(bx, by, bz) != BLOCK_BEDROCK && GetBlock(bx, by, bz) != BLOCK_BORDER) {
                    if (GetBlock(bx, by, bz) != BLOCK_AIR) { SpawnBlockBreakParticles(bx, by, bz, GetBlock(bx, by, bz)); }
                    if (fire && GetBlock(bx, by, bz) == BLOCK_AIR && (WorldHash3D(bx, by, bz, g_worldSeed + 6100) & 3) == 0) { SetBlock(bx, by, bz, BLOCK_FIRE); }
                    else { SetBlock(bx, by, bz, BLOCK_AIR); }
                }
            }
        }
    }
    d = (playerX - x) * (playerX - x) + (playerY + 0.8 - y) * (playerY + 0.8 - y) + (playerZ - z) * (playerZ - z);
    if (d < radius * radius * 3.0) { TakeDamage((int)(10.0 - sqrt(d) * 1.5)); }
    {
        int mi; double mdx; double mdy; double mdz; double md; double force;
        for (mi = 0; mi < MAX_MOBS; mi++) {
            if (!mobs[mi].active) { continue; }
            mdx = mobs[mi].x - x; mdy = mobs[mi].y + 0.8 - y; mdz = mobs[mi].z - z; md = sqrt(mdx * mdx + mdy * mdy + mdz * mdz);
            if (md < radius * 2.0 && md > 0.001) {
                force = (1.0 - md / (radius * 2.0));
                DamageMob(mi, (int)(force * radius * 5.0) + 1, mdx, mdz);
                mobs[mi].vx += (mdx / md) * force * 4.0; mobs[mi].vy += force * 2.5; mobs[mi].vz += (mdz / md) * force * 4.0;
            }
        }
    }
    RecomputeLegacyLightingLocal(ix, iy, iz, 20);
    PlayOneShotMP3("assets\\sounds\\random\\explode.mp3");
}

void SpawnFallingBlockIfNeededV6(int x, int y, int z)
{
    int b;
    if (!IsInsideWorld(x, y, z)) { return; }
    b = GetBlock(x, y, z);
    if ((b == BLOCK_SAND || b == BLOCK_GRAVEL) && IsInsideWorld(x, y - 1, z) && GetBlock(x, y - 1, z) == BLOCK_AIR) {
        SetBlock(x, y, z, BLOCK_AIR);
        SpawnSpecialEntityV6(ENTITY_V6_FALLING_BLOCK, BlockToItem(b), b, x + 0.5, y + 0.5, z + 0.5, 0.0, 0.0, 0.0, 0.0);
    }
}

void ItemCombatV6_CheckProjectileMobHit(SpecialEntityV6 *e)
{
    int i;
    double dx;
    double dy;
    double dz;
    double r;
    if (!e || !e->active) { return; }
    for (i = 0; i < MAX_MOBS; i++) {
        if (!mobs[i].active) { continue; }
        r = (double)MobWidth(mobs[i].type) * 0.55 + 0.15;
        dx = e->x - mobs[i].x;
        dy = e->y - (mobs[i].y + (double)MobHeight(mobs[i].type) * 0.5);
        dz = e->z - mobs[i].z;
        if (dx * dx + dy * dy + dz * dz <= r * r) {
            if (e->type == ENTITY_V6_EGG) {
                if ((WorldHash3D((int)e->x, (int)e->y, (int)e->z, g_worldSeed + 601) & 7) == 0) { AddMob(MOB_CHICKEN, e->x, e->y, e->z); }
            } else if (e->type == ENTITY_V6_SNOWBALL) {
                DamageMob(i, 0 ? 3 : 1, e->vx, e->vz);
            } else if (e->type == ENTITY_V6_FIREBALL) {
                SpawnExplosionV6(e->x, e->y, e->z, 3.2, 1);
            } else {
                DamageMob(i, e->damage > 0 ? e->damage : 3, e->vx, e->vz);
            }
            e->active = 0;
            return;
        }
    }
}


/* ------------------------------------------------------------ */
/* V44 special entity behavior                                  */
/* Java reference path: EntityArrow, EntityBoat, EntityMinecart, */
/* RailLogic, EntityPainting, EntityFish, EntityTNTPrimed,       */
/* Explosion.                                                    */
/* ------------------------------------------------------------ */
int SpecialV44_IsPassableBlock(int block)
{
    if (block == BLOCK_AIR || block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA || block == BLOCK_FIRE) { return 1; }
    if (block == BLOCK_TALL_GRASS || block == BLOCK_FLOWER_YELLOW || block == BLOCK_FLOWER_RED || block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED) { return 1; }
    if (block == BLOCK_TORCH || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF || block == BLOCK_REDSTONE_WIRE) { return 1; }
    return 0;
}

void SpecialV44_UpdateYawPitch(SpecialEntityV6 *e)
{
    double h; double len;
    if (!e) { return; }
    h = sqrt(e->vx * e->vx + e->vz * e->vz);
    len = sqrt(e->vx * e->vx + e->vy * e->vy + e->vz * e->vz);
    if (h > 0.0001) { e->yaw = atan2(e->vx, e->vz) * 180.0 / PI; }
    if (len > 0.0001) { e->pitch = atan2(e->vy, h) * 180.0 / PI; }
}

void SpecialV44_DropEntityItem(SpecialEntityV6 *e)
{
    if (!e) { return; }
    if (e->type == ENTITY_V6_ARROW) { AddDroppedItem(ITEM_ARROW, 1, e->x, e->y, e->z, 0.0, 0.55, 0.0); }
    else if (e->type == ENTITY_V6_BOAT) { AddDroppedItem(ITEM_BOAT, 1, e->x, e->y, e->z, 0.0, 0.85, 0.0); }
    else if (e->type == ENTITY_V6_PAINTING) { AddDroppedItem(ITEM_PAINTING, 1, e->x, e->y, e->z, 0.0, 0.65, 0.0); }
    else if (e->type == ENTITY_V6_TNT) { AddDroppedItem(ITEM_TNT, 1, e->x, e->y, e->z, 0.0, 0.75, 0.0); }
}

int SpecialV44_MoveAndHitBlock(SpecialEntityV6 *e, double dt)
{
    double nx; double ny; double nz; int bx; int by; int bz; int block;
    if (!e) { return 0; }
    nx = e->x + e->vx * dt; ny = e->y + e->vy * dt; nz = e->z + e->vz * dt;
    bx = (int)floor(nx); by = (int)floor(ny); bz = (int)floor(nz);
    if (!IsInsideWorld(bx, by, bz)) { e->active = 0; return 1; }
    block = GetBlock(bx, by, bz);
    if (!SpecialV44_IsPassableBlock(block)) { return 1; }
    e->x = nx; e->y = ny; e->z = nz;
    return 0;
}

void SpecialV44_BreakBoat(SpecialEntityV6 *e)
{
    int k;
    if (!e) { return; }
    for (k = 0; k < 3; k++) { AddDroppedItem(ITEM_PLANKS, 1, e->x, e->y + 0.25, e->z, 0.05 * (double)(k - 1), 0.70, 0.03); }
    for (k = 0; k < 2; k++) { AddDroppedItem(ITEM_STICK, 1, e->x, e->y + 0.35, e->z, -0.04 * (double)(k - 1), 0.65, 0.02); }
    SpawnSplashParticlesV24(e->x, e->y, e->z, 10);
    PlaySoundAtV35("assets\\sounds\\random\\wood_click.wav", e->x, e->y, e->z, 0.8, 0.75, SOUND_DEFAULT_RANGE_V35);
    e->active = 0;
}

int SpecialV44_PaintingHasSupport(SpecialEntityV6 *e)
{
    int bx; int by; int bz; int backX; int backZ; int support;
    if (!e) { return 0; }
    bx = (int)floor(e->x); by = (int)floor(e->y); bz = (int)floor(e->z);
    backX = 0; backZ = 0;
    if (e->meta == 0) { backZ = 1; }
    else if (e->meta == 1) { backX = -1; }
    else if (e->meta == 2) { backZ = -1; }
    else { backX = 1; }
    support = GetBlock(bx + backX, by, bz + backZ);
    if (!IsSolidBlock(support)) { return 0; }
    return 1;
}

int SpecialV44_UpdateProjectile(int index, SpecialEntityV6 *e, double dt)
{
    int block; int water;
    if (!e) { return 1; }
    if (e->stuck) {
        e->fuse += dt;
        if (e->type == ENTITY_V6_ARROW) {
            if ((playerX - e->x) * (playerX - e->x) + (playerY + 0.8 - e->y) * (playerY + 0.8 - e->y) + (playerZ - e->z) * (playerZ - e->z) < 1.25 && e->fuse > 0.25) {
                AddItemToInventory(ITEM_ARROW, 1); PlayPickupSoundThrottled(e->x, e->y, e->z); e->active = 0;
            }
        }
        if (e->fuse > 60.0) { e->active = 0; }
        return 1;
    }
    water = (GetBlock((int)floor(e->x), (int)floor(e->y), (int)floor(e->z)) == BLOCK_WATER);
    if (water && (e->type == ENTITY_V6_ARROW || e->type == ENTITY_V6_EGG || e->type == ENTITY_V6_SNOWBALL)) { SpawnSplashParticlesV24(e->x, e->y, e->z, 1); e->vx *= 0.82; e->vy *= 0.82; e->vz *= 0.82; }
    if (SpecialV44_MoveAndHitBlock(e, dt)) {
        block = GetBlock((int)floor(e->x + e->vx * dt), (int)floor(e->y + e->vy * dt), (int)floor(e->z + e->vz * dt));
        if (e->type == ENTITY_V6_ARROW) { e->stuck = 1; e->fuse = 0.0; e->vx = 0.0; e->vy = 0.0; e->vz = 0.0; PlaySoundAtV35("assets\\sounds\\random\\bowhit.wav", e->x, e->y, e->z, 0.55, 1.1, SOUND_DEFAULT_RANGE_V35); }
        else if (e->type == ENTITY_V6_FIREBALL) { SpawnExplosionV6(e->x, e->y, e->z, 3.2, 1); e->active = 0; }
        else { SpawnBlockBreakParticles((int)floor(e->x), (int)floor(e->y), (int)floor(e->z), block == BLOCK_AIR ? BLOCK_SNOW : block); if (e->type == ENTITY_V6_EGG && (WorldHash3D((int)e->x, (int)e->y, (int)e->z, g_worldSeed + 4441) & 7) == 0) { AddMob(MOB_CHICKEN, e->x, e->y, e->z); } e->active = 0; }
        return 1;
    }
    ItemCombatV6_CheckProjectileMobHit(e);
    if (!e->active) { return 1; }
    if (e->type == ENTITY_V6_FIREBALL) { e->vy -= GRAVITY * 0.02 * dt; e->vx *= 0.995; e->vy *= 0.995; e->vz *= 0.995; }
    else { e->vy -= GRAVITY * (e->type == ENTITY_V6_ARROW ? 0.060 : 0.085) * dt; e->vx *= 0.990; e->vy *= 0.990; e->vz *= 0.990; }
    SpecialV44_UpdateYawPitch(e); (void)index; return 1;
}

int SpecialV44_UpdateBoat(SpecialEntityV6 *e, double dt)
{
    int sx; int bx; int by; int bz; int sample; int solid; double waterFrac; double speed;
    if (!e) { return 1; }
    waterFrac = 0.0; sample = 0;
    for (sx = -1; sx <= 1; sx++) { bx = (int)floor(e->x + (double)sx * 0.55); by = (int)floor(e->y - 0.12); bz = (int)floor(e->z); if (GetBlock(bx, by, bz) == BLOCK_WATER) { waterFrac += 1.0; } sample++; }
    waterFrac /= (double)sample;
    if (waterFrac > 0.0) { e->vy += (waterFrac * 2.0 - 0.55) * dt * 2.4; if (e->vy > 0.28) { e->vy = 0.28; } e->vx *= 0.985; e->vz *= 0.985; }
    else { e->vy -= GRAVITY * 0.55 * dt; e->vx *= 0.94; e->vz *= 0.94; }
    e->x += e->vx * dt; e->y += e->vy * dt; e->z += e->vz * dt;
    solid = IsSolidBlock(GetBlock((int)floor(e->x), (int)floor(e->y), (int)floor(e->z))) || IsSolidBlock(GetBlock((int)floor(e->x), (int)floor(e->y + 0.4), (int)floor(e->z)));
    if (solid) { speed = sqrt(e->vx * e->vx + e->vz * e->vz); if (speed > 1.6) { SpecialV44_BreakBoat(e); return 1; } e->vx = -e->vx * 0.35; e->vz = -e->vz * 0.35; e->x += e->vx * dt * 2.0; e->z += e->vz * dt * 2.0; }
    if ((playerX - e->x) * (playerX - e->x) + (playerZ - e->z) * (playerZ - e->z) < 1.6 && fabs(playerY - e->y) < 1.4) { e->vx += (e->x - playerX) * dt * 1.5; e->vz += (e->z - playerZ) * dt * 1.5; }
    if (sqrt(e->vx * e->vx + e->vz * e->vz) > 0.05) { e->yaw = atan2(e->vx, e->vz) * 180.0 / PI; }
    return 1;
}

int SpecialV44_UpdateFishingHook(int index, SpecialEntityV6 *e, double dt)
{
    int b; double dx; double dz;
    if (!e) { return 1; }
    b = GetBlock((int)floor(e->x), (int)floor(e->y), (int)floor(e->z));
    if (b == BLOCK_WATER) {
        e->vx *= 0.72; e->vz *= 0.72; e->vy *= 0.15; e->y += sin(e->age * 4.0) * 0.012;
        if (e->meta == 0) { e->meta = 1; e->fuse = 4.0 + (double)(WorldHash3D((int)e->x, (int)e->y, (int)e->z, g_worldSeed + 47300) & 255) / 26.0; }
        else if (e->meta == 1) { e->fuse -= dt; if (e->fuse <= 0.0) { e->meta = 2; e->fuse = 1.55; SpawnSplashParticlesV24(e->x, e->y, e->z, 10); PlaySoundAtV35("assets\\sounds\\liquid\\splash.wav", e->x, e->y, e->z, 0.70, 1.10, SOUND_DEFAULT_RANGE_V35); } }
        else if (e->meta == 2) { e->fuse -= dt; if (e->fuse <= 0.0) { e->meta = 3; e->fuse = 1.25; } }
        else if (e->meta == 3) { e->fuse -= dt; if (e->fuse <= 0.0) { e->meta = 1; e->fuse = 3.0 + (double)(WorldHash3D((int)(e->x + e->age), (int)e->y, (int)e->z, g_worldSeed + 47301) & 127) / 20.0; } }
    } else { if (SpecialV44_MoveAndHitBlock(e, dt)) { e->vx = 0.0; e->vy = 0.0; e->vz = 0.0; e->stuck = 1; } e->vy -= GRAVITY * 0.45 * dt; e->vx *= 0.92; e->vz *= 0.92; }
    dx = playerX - e->x; dz = playerZ - e->z;
    if (dx * dx + dz * dz > 96.0 || fabs((playerY + EYE_HEIGHT) - e->y) > 32.0) { e->active = 0; if (g_fishingHookIndexV6 == index) { g_fishingHookIndexV6 = -1; } }
    return 1;
}

int ItemSpecialV44_UpdateEntity(int index, SpecialEntityV6 *e, double dt)
{
    if (!e || !e->active) { return 1; }
    if (e->type == ENTITY_V6_PAINTING) { if (!SpecialV44_PaintingHasSupport(e)) { SpecialV44_DropEntityItem(e); e->active = 0; } return 1; }
    if (e->type == ENTITY_V6_ARROW || e->type == ENTITY_V6_EGG || e->type == ENTITY_V6_SNOWBALL || e->type == ENTITY_V6_FIREBALL) { return SpecialV44_UpdateProjectile(index, e, dt); }
    if (e->type == ENTITY_V6_BOAT) { return SpecialV44_UpdateBoat(e, dt); }
    if (e->type == ENTITY_V6_FISH_HOOK) { return SpecialV44_UpdateFishingHook(index, e, dt); }
    if (e->type == ENTITY_V6_TNT) { e->fuse -= dt; e->vy -= GRAVITY * 0.65 * dt; e->x += e->vx * dt; e->y += e->vy * dt; e->z += e->vz * dt; if (IsSolidBlock(GetBlock((int)floor(e->x), (int)floor(e->y - 0.25), (int)floor(e->z)))) { e->vy = -e->vy * 0.42; e->vx *= 0.70; e->vz *= 0.70; } else { e->vx *= 0.98; e->vz *= 0.98; } if (((int)(e->fuse * 5.0)) != ((int)((e->fuse + dt) * 5.0))) { SpawnSmokeParticlesV24(e->x, e->y + 0.5, e->z, 1); } if (e->fuse <= 0.0) { SpawnExplosionV6(e->x, e->y, e->z, 4.0, 0); e->active = 0; } return 1; }
    return 0;
}

void UpdateItemCombatV6(double dt)
{
    int i;
    int bx;
    int by;
    int bz;
    int b;
    int below;
    double dx;
    double dz;
    SpecialEntityV6 *e;
    if (g_bowChargingV6) {
        g_bowChargeV6 += dt;
        if (g_bowChargeV6 > 1.2) { g_bowChargeV6 = 1.2; }
        ItemCombatV6_FireBowIfCharged();
    }
    g_lightningTimerV6 -= dt;
    if (g_lightningTimerV6 <= 0.0 && g_weatherMode == 1) {
        bx = (int)floor(playerX) + ((WorldHash3D((int)g_worldTimeSeconds, (int)playerX, (int)playerZ, g_worldSeed + 7600) % 25) - 12);
        bz = (int)floor(playerZ) + ((WorldHash3D((int)g_worldTimeSeconds, (int)playerZ, (int)playerX, g_worldSeed + 7601) % 25) - 12);
        by = columnTop[ClampInt(bx, 0, WORLD_X - 1)][ClampInt(bz, 0, WORLD_Z - 1)] + 1;
        if (IsInsideWorld(bx, by, bz)) { SpawnSpecialEntityV6(ENTITY_V6_LIGHTNING, ITEM_NONE, 0, bx + 0.5, by, bz + 0.5, 0.0, 0.0, 0.0, 0.8); }
        g_lightningTimerV6 = 20.0 + (double)(WorldHash2D(bx, bz, g_worldSeed + 7610) & 31);
    }
    for (i = 0; i < MAX_SPECIAL_ENTITIES_V6; i++) {
        if (!g_specialEntitiesV6[i].active) { continue; }
        e = &g_specialEntitiesV6[i];
        e->age += dt;
        if (e->age > e->life) { e->active = 0; if (g_fishingHookIndexV6 == i) { g_fishingHookIndexV6 = -1; } continue; }
        if (e->type == ENTITY_V6_PAINTING) { continue; }
        if (e->type == ENTITY_V6_LIGHTNING) {
            e->fuse -= dt;
            if (e->fuse <= 0.0) { e->active = 0; }
            if (fabs(playerX - e->x) < 2.0 && fabs(playerZ - e->z) < 2.0 && fabs(playerY - e->y) < 8.0) { TakeDamage(5); }
            continue;
        }
        if (ItemSpecialV44_UpdateEntity(i, e, dt)) { continue; }
        if (e->type == ENTITY_V6_TNT) {
            e->fuse -= dt;
            e->vy -= GRAVITY * 0.65 * dt;
            e->x += e->vx * dt;
            e->y += e->vy * dt;
            e->z += e->vz * dt;
            if (e->fuse <= 0.0) { SpawnExplosionV6(e->x, e->y, e->z, 4.0, 0); e->active = 0; }
            continue;
        }
        if (e->type == ENTITY_V6_BOAT) {
            bx = (int)floor(e->x);
            by = (int)floor(e->y - 0.1);
            bz = (int)floor(e->z);
            if (GetBlock(bx, by, bz) == BLOCK_WATER) { e->vy += dt * 2.0; if (e->vy > 0.15) { e->vy = 0.15; } }
            else { e->vy -= GRAVITY * 0.35 * dt; }
            e->vx *= 0.985;
            e->vz *= 0.985;
            e->x += e->vx * dt;
            e->y += e->vy * dt;
            e->z += e->vz * dt;
            if (IsSolidBlock(GetBlock((int)floor(e->x), (int)floor(e->y), (int)floor(e->z)))) { e->vx = -e->vx * 0.35; e->vz = -e->vz * 0.35; }
            continue;
        }
        if (e->type == ENTITY_V6_FALLING_BLOCK) {
            e->vy -= GRAVITY * dt;
            e->x += e->vx * dt;
            e->y += e->vy * dt;
            e->z += e->vz * dt;
            bx = (int)floor(e->x);
            by = (int)floor(e->y - 0.5);
            bz = (int)floor(e->z);
            below = GetBlock(bx, by, bz);
            if (below != BLOCK_AIR && below != BLOCK_WATER) {
                if (IsInsideWorld(bx, by + 1, bz) && GetBlock(bx, by + 1, bz) == BLOCK_AIR) { SetBlock(bx, by + 1, bz, e->block); }
