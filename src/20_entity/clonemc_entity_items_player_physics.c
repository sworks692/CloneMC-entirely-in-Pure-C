/* ============================================================
   CloneMC V51 section: ENTITY / DROPPED ITEMS / HELD ITEMS / PLAYER PHYSICS
   Keep this file included through the root unity source unless you
   are doing a later full extern/prototype object-file refactor.
   ============================================================ */

void RendererV8_RenderSpecialWorldBlocks(void)
{
    int i;
    int cx;
    int cz;
    int x;
    int y;
    int z;
    int minX;
    int maxX;
    int minZ;
    int maxZ;
    int block;
    int topY;
    int minY;
    int maxY;
    int pcx;
    int pcz;
    int dx;
    int dz;
    int radius;
    DWORD passStartMs;

    /* V47: render the Java RenderBlocks-style special shapes from visible
       chunk queues instead of scanning a square around the player every frame.
       This keeps torches, rails, crops, signs, levers, redstone and similar
       non-full cubes correct, but their cost now scales with visible chunks and
       frustum culling rather than WORLD_X * WORLD_Z. */
    if (!texTerrain) { return; }
    if (g_renderOpaqueCountV33 <= 0) { return; }

    passStartMs = GetTickCount();
    pcx = ((int)floor(playerX)) / CHUNK_SIZE;
    pcz = ((int)floor(playerZ)) / CHUNK_SIZE;
    radius = g_renderV47SpecialChunkRadius;
    if (radius < 1) { radius = 1; }
    if (radius > g_renderDistanceChunks) { radius = g_renderDistanceChunks; }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (i = 0; i < g_renderOpaqueCountV33; i++) {
        cx = g_renderOpaqueQueueV33[i].cx;
        cz = g_renderOpaqueQueueV33[i].cz;
        dx = cx - pcx;
        dz = cz - pcz;
        if (dx * dx + dz * dz > radius * radius) { continue; }
        if (!terrainChunkHasSpecialV47[cx][cz]) { continue; }

        minX = cx * CHUNK_SIZE;
        maxX = minX + CHUNK_SIZE - 1;
        minZ = cz * CHUNK_SIZE;
        maxZ = minZ + CHUNK_SIZE - 1;
        if (maxX >= WORLD_X) { maxX = WORLD_X - 1; }
        if (maxZ >= WORLD_Z) { maxZ = WORLD_Z - 1; }
        g_renderProfileV33.specialChunksScanned++;

        for (x = minX; x <= maxX; x++) {
            for (z = minZ; z <= maxZ; z++) {
                topY = columnTop[x][z] + 8;
                if (topY < 8) { topY = 8; }
                if (topY >= WORLD_Y) { topY = WORLD_Y - 1; }
                minY = (int)floor(playerY) - 18;
                maxY = (int)floor(playerY) + 18;
                if (minY < 1) { minY = 1; }
                if (maxY < topY) { maxY = topY; }
                if (maxY >= WORLD_Y) { maxY = WORLD_Y - 1; }
                for (y = minY; y <= maxY; y++) {
                    block = world[x][y][z];
                    if (block == BLOCK_AIR) { continue; }
                    if (block == BLOCK_TORCH || IsSpecialBlockV5(block) || WorldGenV3_IsCrossPlantBlock(block)) {
                        DrawBlock(x, y, z, block);
                        g_rendererStatsV8.specialBlocks++;
                        g_renderProfileV33.specialBlocksDrawn++;
                    }
                }
            }
        }
    }

    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    g_renderProfileV33.specialPassMs += (double)(GetTickCount() - passStartMs);
}

void RendererV8_RenderTransparentBlocks(void);
void RendererV8_RenderSpecialWorldBlocks(void);
void RendererV8_RenderSelectionOutline(void);
void RendererV8_RenderBreakingOverlay(void);
void RendererV11_RenderBreakingTextureFace(int face, float x0, float y0, float z0, float x1, float y1, float z1, float u0, float v0, float u1, float v1);
void RendererV8_DrawBlockWireBox(int x, int y, int z, float expand, float r, float g, float b, float a);
void RendererV8_RenderSpecialEntities(void);
void RendererV8_RenderArrowEntity(SpecialEntityV6 *e);
void RendererV8_RenderThrowableEntity(SpecialEntityV6 *e);
void RendererV8_RenderFireballEntity(SpecialEntityV6 *e);
void RendererV8_RenderFishHookEntity(SpecialEntityV6 *e);
void RendererV8_RenderBoatEntity(SpecialEntityV6 *e);
void RendererV8_RenderPaintingEntity(SpecialEntityV6 *e);
void RendererV8_RenderTNTEntity(SpecialEntityV6 *e);
void RendererV8_RenderFallingBlockEntity(SpecialEntityV6 *e);
void RendererV8_RenderLightningEntity(SpecialEntityV6 *e);
void RendererV8_RenderMobProjectiles(void);
void RendererV8_RenderFirstPersonItem(void);
void RendererV8_RenderWeather3D(void);
void RendererV8_DrawTexturedItemQuad(int item, float size);
void RendererV8_DrawBlockItemMini(int block, float size);
void RendererV8_DrawPaintingByMeta(SpecialEntityV6 *e);
void RendererV8_DrawBoatModel(SpecialEntityV6 *e);
void RendererV8_DrawMinecartModel(double x, double y, double z, double yawDeg);
void RendererV8_RenderMinecarts(void);
void RendererV8_DrawCloudLayer(void);
void RendererV8_PostWorldEffects(void);

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


/* V46_HELD_ITEM_TEXTURE_FIX
   Java RenderItem draws true item IDs (door/sign/repeater/cake/boat/etc.)
   from gui/items.png even when right-clicking that item places a block.
   Older CloneMC passes called ItemToBlock() first, so these items could be
   rendered with terrain/block tiles while held.  This helper chooses the
   item atlas first for real Java item IDs, but preserves terrain rendering for
   block IDs and internal block-item aliases. */
int ItemRenderV46_ShouldPreferItemIcon(int item)
{
    int c;
    int r;

    if (item == ITEM_NONE) { return 0; }

    /* Item IDs 256+ are Java Item.java icon entries.  ITEM_LIGHT and
       ITEM_WATER are internal CloneMC stand-ins and should still render from
       the terrain atlas when they map to a block. */
    if (item >= 256 && item != ITEM_LIGHT && item != ITEM_WATER) {
        if (GetItemIconTile(item, &c, &r)) { return 1; }
    }

    return 0;
}

int ShouldRenderBlockItemAs3DV27(int block)
{
    if (block == BLOCK_AIR) { return 0; }

    /* This helper sits before the V19 BlockDef table in the single C file, so
       keep it ID-based and Open Watcom-friendly.  Java RenderItem renders full
       3D inventory blocks only for normal-ish blocks; flowers, rails, torches,
       ladders, doors, reeds, crops, redstone and other special shapes are flat
       terrain icons. */
    if (block == BLOCK_TORCH || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF ||
        block == BLOCK_FLOWER_YELLOW || block == BLOCK_FLOWER_RED ||
        block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED ||
        block == BLOCK_CROPS || block == BLOCK_REED || block == BLOCK_LADDER ||
        block == BLOCK_RAIL || block == BLOCK_DETECTOR_RAIL ||
        block == BLOCK_REDSTONE_WIRE || block == BLOCK_REPEATER_OFF || block == BLOCK_REPEATER_ON ||
        block == BLOCK_WOOD_DOOR || block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL ||
        block == BLOCK_LEVER || block == BLOCK_STONE_BUTTON || block == BLOCK_WOOD_PRESSURE_PLATE ||
        block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_FIRE || block == BLOCK_PORTAL ||
        block == BLOCK_CAKE || block == BLOCK_TRAPDOOR || block == BLOCK_BED ||
        block == BLOCK_WOOD_STAIRS || block == BLOCK_COBBLESTONE_STAIRS) { return 0; }

    return 1;
}

int ResolveItemDrawTileV27(int item, int *col, int *row, int *useTerrain)
{
    int block;

    if (col) { *col = 0; }
    if (row) { *row = 0; }
    if (useTerrain) { *useTerrain = 0; }
    if (item == ITEM_NONE) { return 0; }

    /* V46: Java ItemRenderer/RenderItem order.  For true Item.java IDs,
       draw the item icon first.  This fixes doors, signs, cakes, beds,
       boats, minecarts, buckets, tools, food, records, maps, repeaters and
       other held items that were accidentally treated as block terrain tiles
       because ItemToBlock() returned the block they place. */
    if (ItemRenderV46_ShouldPreferItemIcon(item)) {
        if (GetItemIconTile(item, col, row)) {
            if (useTerrain) { *useTerrain = 0; }
            return 1;
        }
    }

    block = ItemToBlock(item);
    if (block != BLOCK_AIR) {
        if (useTerrain) { *useTerrain = 1; }
        GetBlockTile(block, 0, col, row);
        return 1;
    }

    if (GetItemIconTile(item, col, row)) {
        if (useTerrain) { *useTerrain = 0; }
        return 1;
    }

    if (col) { *col = 5; }
    if (row) { *row = 3; }
    if (useTerrain) { *useTerrain = 0; }
    return 1;
}

void DrawTexturedItemCardUVV27(GLuint tex, int atlasW, int atlasH, int col, int row, float size, float thickness)
{
    float u0;
    float v0;
    float u1;
    float v1;
    float t;

    if (!tex) { return; }
    GetTileUVEx(col, row, atlasW, atlasH, &u0, &v0, &u1, &v1);
    t = thickness;
    if (t < 0.004f) { t = 0.004f; }

    /* V31: draw item/special-block icons as alpha-tested two-sided sprites.
       The earlier edge-quads made transparent icons look like solid glitched
       squares/slabs when mobs or players dropped feathers, pork, seeds, rails,
       torches, flowers, etc.  Alpha test is OpenGL 1.1 compatible and works
       on Windows 98 era drivers. */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.08f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(u0, v1); glVertex3f(-size, -size,  t);
    glTexCoord2f(u1, v1); glVertex3f( size, -size,  t);
    glTexCoord2f(u1, v0); glVertex3f( size,  size,  t);
    glTexCoord2f(u0, v0); glVertex3f(-size,  size,  t);

    glTexCoord2f(u1, v1); glVertex3f(-size, -size, -t);
    glTexCoord2f(u0, v1); glVertex3f( size, -size, -t);
    glTexCoord2f(u0, v0); glVertex3f( size,  size, -t);
    glTexCoord2f(u1, v0); glVertex3f(-size,  size, -t);
    glEnd();

    glDisable(GL_ALPHA_TEST);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawDroppedItemSpriteUVV31(GLuint tex, int atlasW, int atlasH, int col, int row, float size)
{
    DrawTexturedItemCardUVV27(tex, atlasW, atlasH, col, row, size, 0.006f);
}

void DrawItemCardLocalV27(int item, float size, float thickness)
{
    int col;
    int row;
    int useTerrain;
    GLuint tex;
    int atlasW;
    int atlasH;

    if (!ResolveItemDrawTileV27(item, &col, &row, &useTerrain)) { return; }
    tex = useTerrain ? texTerrain : texBetaItems;
    atlasW = useTerrain ? TERRAIN_ATLAS_WIDTH : ICONS_ATLAS_WIDTH;
    atlasH = useTerrain ? TERRAIN_ATLAS_HEIGHT : ICONS_ATLAS_HEIGHT;
    DrawTexturedItemCardUVV27(tex, atlasW, atlasH, col, row, size, thickness);
}

void DrawTerrainItemCardLocalV27(int block, float size, float thickness)
{
    int item;
    item = BlockToItem(block);
    if (item == ITEM_NONE) { item = block; }
    DrawItemCardLocalV27(item, size, thickness);
}

void DrawHeldBlockCubeLocal(int block, float scale)
{
    if (block == BLOCK_AIR || !texTerrain) { return; }
    if (!ShouldRenderBlockItemAs3DV27(block)) {
        /* Block IDs such as torches, flowers, rails and redstone should render
           from terrain.png when the held stack itself is a block alias.  True
           Java item IDs are handled by DrawHeldItemQuadLocal via the item
           atlas, so this path avoids mixed/wrong held textures. */
        DrawTerrainItemCardLocalV27(block, 0.42f, 0.018f);
        return;
    }
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
    DrawItemCardLocalV27(item, size, size * 0.085f);
}

void DrawHeldItemFirstPerson(int item)
{
    int block;

    if (item == ITEM_NONE) { return; }
    block = ItemToBlock(item);

    glPushMatrix();
    if (block != BLOCK_AIR && !ItemRenderV46_ShouldPreferItemIcon(item) && ShouldRenderBlockItemAs3DV27(block)) {
        glTranslatef(-0.03f, 0.09f, -0.28f);
        glRotatef(24.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(47.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-7.0f, 0.0f, 0.0f, 1.0f);
        DrawHeldBlockCubeLocal(block, 0.44f);
    } else {
        glTranslatef(0.05f, 0.14f, -0.30f);
        glRotatef(52.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(335.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-18.0f, 1.0f, 0.0f, 0.0f);
        DrawHeldItemQuadLocal(item, 0.38f);
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
    glTranslatef(-0.47f, 1.04f, -0.24f);
    if (block != BLOCK_AIR && !ItemRenderV46_ShouldPreferItemIcon(item) && ShouldRenderBlockItemAs3DV27(block)) {
        glTranslatef(0.00f, 0.14f, -0.13f);
        glRotatef(18.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
        DrawHeldBlockCubeLocal(block, 0.19f);
    } else {
        glTranslatef(0.08f, 0.13f, -0.15f);
        glRotatef(50.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(335.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-12.0f, 1.0f, 0.0f, 0.0f);
        DrawHeldItemQuadLocal(item, 0.17f);
    }
    glPopMatrix();
}


int IsInsideWorld(int x, int y, int z);
int GetBlock(int x, int y, int z);
void SetBlock(int x, int y, int z, int block);
int IsSolidBlock(int block);

void UpdateMouseLook(void);
void CenterMouse(void);
void HandleGameInput(double dt);


int PlayerV22_IsReplaceableBlock(int block)
{
    return BlockV49_IsReplaceable(block);
}

int PlayerV22_BlockHasCollision(int block)
{
    return BlockV49_BlocksMovement(block);
}

void PlayerV22_GetBlockBounds(int block, int bx, int by, int bz,
                              double *minX, double *minY, double *minZ,
                              double *maxX, double *maxY, double *maxZ)
{
    if (BlockV49_GetPrimaryCollisionBounds(block, bx, by, bz, minX, minY, minZ, maxX, maxY, maxZ)) { return; }
    *minX = (double)bx;
    *minY = (double)by;
    *minZ = (double)bz;
    *maxX = (double)bx + 1.0;
    *maxY = (double)by + 1.0;
    *maxZ = (double)bz + 1.0;
}

int PlayerV22_AabbIntersects(double aMinX, double aMinY, double aMinZ,
                             double aMaxX, double aMaxY, double aMaxZ,
                             double bMinX, double bMinY, double bMinZ,
                             double bMaxX, double bMaxY, double bMaxZ)
{
    if (aMaxX <= bMinX || aMinX >= bMaxX) { return 0; }
    if (aMaxY <= bMinY || aMinY >= bMaxY) { return 0; }
    if (aMaxZ <= bMinZ || aMinZ >= bMaxZ) { return 0; }
    return 1;
}

int PlayerV22_AabbCollidesAt(double x, double y, double z)
{
    double pMinX;
    double pMinY;
    double pMinZ;
    double pMaxX;
    double pMaxY;
    double pMaxZ;
    double bMinX;
    double bMinY;
    double bMinZ;
    double bMaxX;
    double bMaxY;
    double bMaxZ;
    int minX;
    int maxX;
    int minY;
    int maxY;
    int minZ;
    int maxZ;
    int bx;
    int by;
    int bz;
    int block;

    pMinX = x - PLAYER_RADIUS;
    pMaxX = x + PLAYER_RADIUS;
    pMinY = y;
    pMaxY = y + PLAYER_HEIGHT;
    pMinZ = z - PLAYER_RADIUS;
    pMaxZ = z + PLAYER_RADIUS;

    minX = (int)floor(pMinX);
    maxX = (int)floor(pMaxX);
    minY = (int)floor(pMinY);
    maxY = (int)floor(pMaxY);
    minZ = (int)floor(pMinZ);
    maxZ = (int)floor(pMaxZ);

    for (bx = minX; bx <= maxX; bx++) {
        for (by = minY; by <= maxY; by++) {
            for (bz = minZ; bz <= maxZ; bz++) {
                block = GetBlock(bx, by, bz);
                if (!PlayerV22_BlockHasCollision(block)) { continue; }
                PlayerV22_GetBlockBounds(block, bx, by, bz, &bMinX, &bMinY, &bMinZ, &bMaxX, &bMaxY, &bMaxZ);
                if (PlayerV22_AabbIntersects(pMinX, pMinY, pMinZ, pMaxX, pMaxY, pMaxZ,
                                             bMinX, bMinY, bMinZ, bMaxX, bMaxY, bMaxZ)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int PlayerV22_HasGroundSupport(double x, double y, double z)
{
    if (PlayerV22_AabbCollidesAt(x, y - 0.08, z)) { return 1; }
    return 0;
}

int PlayerV22_MoveHorizontalRaw(double dx, double dz)
{
    int cx;
    int cz;
    cx = 0;
    cz = 0;
    if (dx != 0.0) { cx = PlayerV22_MoveAxisSweep(dx, 0.0, 0.0); }
    if (dz != 0.0) { cz = PlayerV22_MoveAxisSweep(0.0, 0.0, dz); }
    return cx || cz;
}


/* ------------------------------------------------------------ */
/* V39 Java-style Entity / AxisAlignedBB movement core           */
/* ------------------------------------------------------------ */
/*
   This section is the movement foundation that was still missing after the
   earlier mob smoothing patches.  It ports the important behavior pattern from
   the uploaded Java Entity.java, EntityLiving.java, AxisAlignedBB.java and
   MathHelper.java into C89/OpenGL-era code:
     - AxisAlignedBB calculateX/Y/ZOffset clipping instead of tiny sweep steps
     - moveEntity axis order: Y, then X, then Z
     - stepHeight retry path for walking over half-block/high edges
     - collidedHorizontally, collidedVertically, onGround semantics
     - slipperiness/friction from the block under the entity
     - gravity, water/lava damping and environmental push
     - fall-distance accumulation and damage when landing
     - smooth living-entity yaw/body yaw convergence
*/
typedef struct AxisAlignedBBV39 {
    double minX;
    double minY;
    double minZ;
    double maxX;
    double maxY;
    double maxZ;
} AxisAlignedBBV39;

int BlockV49_AddCollisionBoxes(int block, int bx, int by, int bz, AxisAlignedBBV39 *boxes, int maxBoxes);

typedef struct EntityMoveResultV39 {
    AxisAlignedBBV39 box;
    double movedX;
    double movedY;
    double movedZ;
    int collidedHorizontally;
    int collidedVertically;
    int onGround;
} EntityMoveResultV39;

AxisAlignedBBV39 AABBV39_Make(double minX, double minY, double minZ,
                              double maxX, double maxY, double maxZ)
{
    AxisAlignedBBV39 b;
    b.minX = minX;
    b.minY = minY;
    b.minZ = minZ;
    b.maxX = maxX;
    b.maxY = maxY;
    b.maxZ = maxZ;
    return b;
}

AxisAlignedBBV39 AABBV39_Offset(AxisAlignedBBV39 b, double x, double y, double z)
{
    b.minX += x; b.maxX += x;
    b.minY += y; b.maxY += y;
    b.minZ += z; b.maxZ += z;
    return b;
}

AxisAlignedBBV39 AABBV39_Expand(AxisAlignedBBV39 b, double x, double y, double z)
{
    b.minX -= x; b.maxX += x;
    b.minY -= y; b.maxY += y;
    b.minZ -= z; b.maxZ += z;
    return b;
}

AxisAlignedBBV39 AABBV39_AddCoord(AxisAlignedBBV39 b, double x, double y, double z)
{
    if (x < 0.0) { b.minX += x; } else if (x > 0.0) { b.maxX += x; }
    if (y < 0.0) { b.minY += y; } else if (y > 0.0) { b.maxY += y; }
    if (z < 0.0) { b.minZ += z; } else if (z > 0.0) { b.maxZ += z; }
    return b;
}

int AABBV39_Intersects(AxisAlignedBBV39 a, AxisAlignedBBV39 b)
{
    if (b.maxX <= a.minX || b.minX >= a.maxX) { return 0; }
    if (b.maxY <= a.minY || b.minY >= a.maxY) { return 0; }
    if (b.maxZ <= a.minZ || b.minZ >= a.maxZ) { return 0; }
    return 1;
}

double AABBV39_CalculateXOffset(AxisAlignedBBV39 solid, AxisAlignedBBV39 moving, double offset)
{
    double d;
    if (moving.maxY <= solid.minY || moving.minY >= solid.maxY) { return offset; }
    if (moving.maxZ <= solid.minZ || moving.minZ >= solid.maxZ) { return offset; }
    if (offset > 0.0 && moving.maxX <= solid.minX) {
        d = solid.minX - moving.maxX;
        if (d < offset) { offset = d; }
    }
    if (offset < 0.0 && moving.minX >= solid.maxX) {
        d = solid.maxX - moving.minX;
        if (d > offset) { offset = d; }
    }
    return offset;
}

double AABBV39_CalculateYOffset(AxisAlignedBBV39 solid, AxisAlignedBBV39 moving, double offset)
{
    double d;
    if (moving.maxX <= solid.minX || moving.minX >= solid.maxX) { return offset; }
    if (moving.maxZ <= solid.minZ || moving.minZ >= solid.maxZ) { return offset; }
    if (offset > 0.0 && moving.maxY <= solid.minY) {
        d = solid.minY - moving.maxY;
        if (d < offset) { offset = d; }
    }
    if (offset < 0.0 && moving.minY >= solid.maxY) {
        d = solid.maxY - moving.minY;
        if (d > offset) { offset = d; }
    }
    return offset;
}

double AABBV39_CalculateZOffset(AxisAlignedBBV39 solid, AxisAlignedBBV39 moving, double offset)
{
    double d;
    if (moving.maxX <= solid.minX || moving.minX >= solid.maxX) { return offset; }
    if (moving.maxY <= solid.minY || moving.minY >= solid.maxY) { return offset; }
    if (offset > 0.0 && moving.maxZ <= solid.minZ) {
        d = solid.minZ - moving.maxZ;
        if (d < offset) { offset = d; }
    }
    if (offset < 0.0 && moving.minZ >= solid.maxZ) {
        d = solid.maxZ - moving.minZ;
        if (d > offset) { offset = d; }
    }
    return offset;
}

AxisAlignedBBV39 EntityV39_GetPlayerBoxAt(double x, double y, double z)
{
    return AABBV39_Make(x - PLAYER_RADIUS, y, z - PLAYER_RADIUS,
                        x + PLAYER_RADIUS, y + PLAYER_HEIGHT, z + PLAYER_RADIUS);
}

AxisAlignedBBV39 EntityV39_GetMobBoxAt(int type, double x, double y, double z)
{
    double half;
    double height;
    half = (double)MobWidth(type) * 0.50;
    height = (double)MobHeight(type);
    return AABBV39_Make(x - half, y, z - half, x + half, y + height, z + half);
}

void EntityV39_BoxToPlayerPosition(AxisAlignedBBV39 box)
{
    playerX = (box.minX + box.maxX) * 0.5;
    playerY = box.minY;
    playerZ = (box.minZ + box.maxZ) * 0.5;
}

void EntityV39_BoxToMobPosition(Mob *m, AxisAlignedBBV39 box)
{
    if (!m) { return; }
    m->x = (box.minX + box.maxX) * 0.5;
    m->y = box.minY;
    m->z = (box.minZ + box.maxZ) * 0.5;
}

int EntityV39_CollectCollisionBoxes(AxisAlignedBBV39 query, AxisAlignedBBV39 *boxes, int maxBoxes)
{
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
    int count;
    double bMinX;
    double bMinY;
    double bMinZ;
    double bMaxX;
    double bMaxY;
    double bMaxZ;
    AxisAlignedBBV39 solid;

    count = 0;
    minX = (int)floor(query.minX - 1.0);
    maxX = (int)floor(query.maxX + 1.0);
    minY = (int)floor(query.minY - 1.0);
    maxY = (int)floor(query.maxY + 1.0);
    minZ = (int)floor(query.minZ - 1.0);
    maxZ = (int)floor(query.maxZ + 1.0);

    for (x = minX; x <= maxX; x++) {
        for (y = minY; y <= maxY; y++) {
            for (z = minZ; z <= maxZ; z++) {
                block = GetBlock(x, y, z);
                if (!PlayerV22_BlockHasCollision(block)) { continue; }
                {
                    AxisAlignedBBV39 blockBoxes[8];
                    int boxCount;
                    int bi;
                    boxCount = BlockV49_AddCollisionBoxes(block, x, y, z, blockBoxes, 8);
                    for (bi = 0; bi < boxCount; bi++) {
                        solid = blockBoxes[bi];
                        if (!AABBV39_Intersects(solid, query)) { continue; }
                        if (count < maxBoxes) {
                            boxes[count] = solid;
                            count++;
                        } else {
                            return count;
                        }
                    }
                }
            }
        }
    }
    return count;
}

void EntityV39_ClipMoveNoStep(AxisAlignedBBV39 startBox, double moveX, double moveY, double moveZ,
                              EntityMoveResultV39 *out)
{
    AxisAlignedBBV39 boxes[ENTITY_V39_MAX_COLLISION_BOXES];
    AxisAlignedBBV39 query;
    AxisAlignedBBV39 box;
    double inX;
    double inY;
    double inZ;
    int count;
    int i;

    inX = moveX;
    inY = moveY;
    inZ = moveZ;
    query = AABBV39_AddCoord(startBox, moveX, moveY, moveZ);
    query = AABBV39_Expand(query, 0.001, 0.001, 0.001);
    count = EntityV39_CollectCollisionBoxes(query, boxes, ENTITY_V39_MAX_COLLISION_BOXES);
    box = startBox;

    for (i = 0; i < count; i++) { moveY = AABBV39_CalculateYOffset(boxes[i], box, moveY); }
    box = AABBV39_Offset(box, 0.0, moveY, 0.0);
    for (i = 0; i < count; i++) { moveX = AABBV39_CalculateXOffset(boxes[i], box, moveX); }
    box = AABBV39_Offset(box, moveX, 0.0, 0.0);
    for (i = 0; i < count; i++) { moveZ = AABBV39_CalculateZOffset(boxes[i], box, moveZ); }
    box = AABBV39_Offset(box, 0.0, 0.0, moveZ);

    out->box = box;
    out->movedX = moveX;
    out->movedY = moveY;
    out->movedZ = moveZ;
    out->collidedHorizontally = (fabs(inX - moveX) > ENTITY_V39_EPSILON || fabs(inZ - moveZ) > ENTITY_V39_EPSILON) ? 1 : 0;
    out->collidedVertically = (fabs(inY - moveY) > ENTITY_V39_EPSILON) ? 1 : 0;
    out->onGround = (out->collidedVertically && inY < 0.0) ? 1 : 0;
}

void EntityV39_MoveEntity(AxisAlignedBBV39 startBox, double moveX, double moveY, double moveZ,
                          double stepHeight, int enableStep, EntityMoveResultV39 *out)
{
    EntityMoveResultV39 normal;
    EntityMoveResultV39 up;
    EntityMoveResultV39 across;
    EntityMoveResultV39 down;
    double normalDist;
    double stepDist;

    EntityV39_ClipMoveNoStep(startBox, moveX, moveY, moveZ, &normal);
    *out = normal;

    if (!enableStep || stepHeight <= 0.0) { return; }
    if (!normal.collidedHorizontally) { return; }
    if (!(normal.onGround || moveY < 0.0 || fabs(moveY) < 0.00001)) { return; }
    if (fabs(moveX) < 0.000001 && fabs(moveZ) < 0.000001) { return; }

    EntityV39_ClipMoveNoStep(startBox, 0.0, stepHeight, 0.0, &up);
    EntityV39_ClipMoveNoStep(up.box, moveX, 0.0, moveZ, &across);
    EntityV39_ClipMoveNoStep(across.box, 0.0, -stepHeight, 0.0, &down);

    normalDist = normal.movedX * normal.movedX + normal.movedZ * normal.movedZ;
    stepDist = across.movedX * across.movedX + across.movedZ * across.movedZ;

    if (stepDist > normalDist + 0.000001) {
        out->box = down.box;
        out->movedX = across.movedX;
        out->movedY = up.movedY + down.movedY;
        out->movedZ = across.movedZ;
        out->collidedHorizontally = across.collidedHorizontally;
        out->collidedVertically = down.collidedVertically;
        out->onGround = down.onGround;
    }
}

double EntityV39_BlockSlipperiness(int block)
{
    if (block == BLOCK_ICE) { return 0.98; }
    if (block == BLOCK_SOULSAND) { return 0.40; }
    return 0.60;
}

double EntityV39_ExpDrag(double base, double dt)
{
    if (dt <= 0.0) { return 1.0; }
    if (base < 0.01) { base = 0.01; }
    if (base > 1.0) { base = 1.0; }
    return pow(base, dt * 20.0);
}

int EntityV39_BlockBelowPlayer(void)
{
    return GetBlock((int)floor(playerX), (int)floor(playerY - 0.08), (int)floor(playerZ));
}

void EntityV39_ApplyFluidPushToVelocity(AxisAlignedBBV39 box, int wantLava, double dt,
                                        double *vx, double *vy, double *vz)
{
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
    double pushX;
    double pushY;
    double pushZ;
    double len;

    if (!vx || !vy || !vz) { return; }
    pushX = 0.0;
    pushY = 0.0;
    pushZ = 0.0;
    minX = (int)floor(box.minX);
    maxX = (int)floor(box.maxX);
    minY = (int)floor(box.minY);
    maxY = (int)floor(box.maxY);
    minZ = (int)floor(box.minZ);
    maxZ = (int)floor(box.maxZ);

    for (x = minX; x <= maxX; x++) {
        for (y = minY; y <= maxY; y++) {
            for (z = minZ; z <= maxZ; z++) {
                block = GetBlock(x, y, z);
                if (wantLava) {
                    if (!(block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA)) { continue; }
                } else if (block != BLOCK_WATER) {
                    continue;
                }
                if (GetBlock(x + 1, y, z) == BLOCK_AIR) { pushX += 1.0; }
                if (GetBlock(x - 1, y, z) == BLOCK_AIR) { pushX -= 1.0; }
                if (GetBlock(x, y, z + 1) == BLOCK_AIR) { pushZ += 1.0; }
                if (GetBlock(x, y, z - 1) == BLOCK_AIR) { pushZ -= 1.0; }
                if (GetBlock(x, y - 1, z) == BLOCK_AIR) { pushY -= 0.65; }
            }
        }
    }

    len = sqrt(pushX * pushX + pushY * pushY + pushZ * pushZ);
    if (len > 0.0001) {
        pushX /= len;
        pushY /= len;
        pushZ /= len;
        *vx += pushX * dt * (wantLava ? 0.55 : 0.95);
        *vy += pushY * dt * (wantLava ? 0.35 : 0.55);
        *vz += pushZ * dt * (wantLava ? 0.55 : 0.95);
    }
}

void PlayerV39_ApplyNoInputFriction(double dt)
{
    double drag;
    int blockBelow;
    if (dt <= 0.0) { return; }
    PlayerV22_RefreshEnvironment();
    blockBelow = EntityV39_BlockBelowPlayer();
    if (g_playerInWaterV22) { drag = EntityV39_ExpDrag(ENTITY_V39_WATER_DRAG, dt); }
    else if (g_playerInLavaV22) { drag = EntityV39_ExpDrag(ENTITY_V39_LAVA_DRAG, dt); }
    else if (onGround) { drag = EntityV39_ExpDrag(EntityV39_BlockSlipperiness(blockBelow) * 0.91, dt); }
    else { drag = EntityV39_ExpDrag(ENTITY_V39_AIR_DRAG, dt); }
    velocityX *= drag;
    velocityZ *= drag;
}

void PlayerV39_HandleMovementInput(double dt)
{
    double yawRad;
    double forwardX;
    double forwardZ;
    double rightX;
    double rightZ;
    double inputX;
    double inputZ;
    double len;
    double accel;
    double speedScale;
    double blockSlip;
    int blockBelow;

    if (dt <= 0.0) { return; }
    if (dt > 0.10) { dt = 0.10; }

    PlayerV22_RefreshEnvironment();
    g_playerSneakingV22 = keySneak ? 1 : 0;

    yawRad = yaw * PI / 180.0;
    forwardX = -sin(yawRad);
    forwardZ = -cos(yawRad);
    rightX = cos(yawRad);
    rightZ = -sin(yawRad);
    inputX = 0.0;
    inputZ = 0.0;

    if (keyForward) { inputX += forwardX; inputZ += forwardZ; }
    if (keyBack) { inputX -= forwardX; inputZ -= forwardZ; }
    if (keyRight) { inputX += rightX; inputZ += rightZ; }
    if (keyLeft) { inputX -= rightX; inputZ -= rightZ; }

    len = sqrt(inputX * inputX + inputZ * inputZ);
    if (len > 0.0001) { inputX /= len; inputZ /= len; }

    blockBelow = EntityV39_BlockBelowPlayer();
    blockSlip = EntityV39_BlockSlipperiness(blockBelow);
    speedScale = MOVE_SPEED;
    accel = 22.0;

    if (g_playerSneakingV22 && onGround) { speedScale *= PLAYER_SNEAK_SPEED_SCALE_V22; }
    if (g_playerInWaterV22) { speedScale *= WATER_HORIZONTAL_SCALE; accel = 7.0; }
    else if (g_playerInLavaV22) { speedScale *= PLAYER_LAVA_SCALE_V22; accel = 4.0; }
    else if (!onGround) { speedScale *= 0.56; accel = 5.5; }
    else {
        /* Java EntityLiving uses ground slipperiness to determine how much the
           moveFlying acceleration sticks.  Keep the same feel: ice is slippery,
           soul sand is slow, normal blocks stay responsive. */
        if (blockSlip > 0.90) { accel = 10.0; }
        if (blockSlip < 0.50) { speedScale *= 0.55; accel = 16.0; }
    }

    if (g_playerInWebV22) { speedScale *= PLAYER_WEB_SCALE_V22; accel = 4.0; }

    if (len > 0.0001) {
        PlayerV22_ApproachVelocity(inputX * speedScale, inputZ * speedScale, accel, dt);
    } else {
        PlayerV39_ApplyNoInputFriction(dt);
    }

    if (keyJump) {
        if (g_playerInWaterV22) {
            velocityY += WATER_SWIM_UP_ACCEL * dt;
            if (velocityY < WATER_SWIM_UP_MIN_SPEED) { velocityY = WATER_SWIM_UP_MIN_SPEED; }
            if (velocityY > WATER_SWIM_UP_MAX_SPEED) { velocityY = WATER_SWIM_UP_MAX_SPEED; }
            onGround = 0;
        } else if (g_playerInLavaV22) {
            velocityY += WATER_SWIM_UP_ACCEL * 0.45 * dt;
            if (velocityY > WATER_SWIM_UP_MAX_SPEED * 0.45) { velocityY = WATER_SWIM_UP_MAX_SPEED * 0.45; }
            onGround = 0;
        } else if (g_playerOnLadderV22) {
            velocityY = 2.1;
            onGround = 0;
        } else if (onGround) {
            velocityY = JUMP_SPEED;
            onGround = 0;
        }
    }
}

void PlayerV39_UpdatePhysics(double dt)
{
    AxisAlignedBBV39 box;
    EntityMoveResultV39 move;
    double originalVelY;
    double drag;
    int blockBelow;

    if (dt <= 0.0) { return; }
    if (dt > 0.10) { dt = 0.10; }

    PlayerV22_RefreshEnvironment();
    originalVelY = velocityY;
    blockBelow = EntityV39_BlockBelowPlayer();

    if (g_playerInWebV22) {
        velocityX *= 0.25;
        velocityZ *= 0.25;
        if (velocityY < -0.55) { velocityY = -0.55; }
    }

    box = EntityV39_GetPlayerBoxAt(playerX, playerY, playerZ);
    if (g_playerInWaterV22) {
        EntityV39_ApplyFluidPushToVelocity(box, 0, dt, &velocityX, &velocityY, &velocityZ);
        if (!keyJump) { velocityY -= WATER_PASSIVE_SINK_ACCEL * dt; }
        if (velocityY < -WATER_DOWN_MAX_SPEED) { velocityY = -WATER_DOWN_MAX_SPEED; }
        g_playerFallDistanceV22 = 0.0;
    } else if (g_playerInLavaV22) {
        EntityV39_ApplyFluidPushToVelocity(box, 1, dt, &velocityX, &velocityY, &velocityZ);
        if (!keyJump) { velocityY -= 1.25 * dt; }
        if (velocityY < -1.25) { velocityY = -1.25; }
        g_playerFallDistanceV22 = 0.0;
    } else if (g_playerOnLadderV22) {
        if (velocityY < -2.0) { velocityY = -2.0; }
        if (keySneak && velocityY < 0.0) { velocityY = 0.0; }
        g_playerFallDistanceV22 = 0.0;
    } else {
        velocityY -= ENTITY_V39_GRAVITY_PER_SECOND * dt;
    }

    box = EntityV39_GetPlayerBoxAt(playerX, playerY, playerZ);
    EntityV39_MoveEntity(box, velocityX * dt, velocityY * dt, velocityZ * dt,
                         ENTITY_V39_PLAYER_STEP_HEIGHT, 1, &move);
    EntityV39_BoxToPlayerPosition(move.box);

    onGround = move.onGround;
    if (fabs(move.movedX - velocityX * dt) > ENTITY_V39_EPSILON) { velocityX = 0.0; }
    if (fabs(move.movedZ - velocityZ * dt) > ENTITY_V39_EPSILON) { velocityZ = 0.0; }
    if (move.collidedVertically) {
        if (originalVelY < 0.0 && !g_playerInWaterV22 && !g_playerInLavaV22 && !g_playerOnLadderV22) {
            if (g_playerFallDistanceV22 > ENTITY_V39_SAFE_FALL_DISTANCE) {
                TakeDamage((int)(g_playerFallDistanceV22 - ENTITY_V39_SAFE_FALL_DISTANCE));
            }
            g_playerFallDistanceV22 = 0.0;
        }
        velocityY = 0.0;
    } else if (velocityY < 0.0 && !g_playerInWaterV22 && !g_playerInLavaV22 && !g_playerOnLadderV22) {
        g_playerFallDistanceV22 += -velocityY * dt;
    }

    if (g_playerInWaterV22) { drag = EntityV39_ExpDrag(ENTITY_V39_WATER_DRAG, dt); }
    else if (g_playerInLavaV22) { drag = EntityV39_ExpDrag(ENTITY_V39_LAVA_DRAG, dt); }
    else if (onGround) { drag = EntityV39_ExpDrag(EntityV39_BlockSlipperiness(blockBelow) * 0.91, dt); }
    else { drag = EntityV39_ExpDrag(ENTITY_V39_AIR_DRAG, dt); }
    velocityX *= drag;
    velocityZ *= drag;
    velocityY *= EntityV39_ExpDrag(ENTITY_V39_VERTICAL_DRAG, dt);

    PlayerV22_RefreshEnvironment();
    if (g_playerHeadWaterV22) {
        g_playerAirTimer -= dt;
        if (((int)(g_playerAirTimer * 4.0)) != ((int)((g_playerAirTimer + dt) * 4.0))) {
            SpawnWaterBubbleParticles(playerX, playerY + PlayerV22_GetEyeHeight() - 0.20, playerZ, 3);
        }
        if (g_playerAirTimer <= 0.0) {
            g_drownDamageTimer -= dt;
            if (g_drownDamageTimer <= 0.0) {
                TakeDamage(2);
                SpawnWaterBubbleParticles(playerX, playerY + PlayerV22_GetEyeHeight() - 0.20, playerZ, 9);
                g_drownDamageTimer = 1.0;
            }
        } else { g_drownDamageTimer = 1.0; }
    } else { g_playerAirTimer = 12.0; g_drownDamageTimer = 1.0; }

    if (g_playerInLavaV22) {
        g_playerLavaTickV22 -= dt;
        if (g_playerLavaTickV22 <= 0.0) { TakeDamage(4); g_playerLavaTickV22 = 0.75; }
        g_playerFireTickV22 = 4.0;
    }

    if (g_playerInFireV22 || g_playerFireTickV22 > 0.0) {
        g_playerFireTickV22 -= dt;
        if (g_playerFireTickV22 < 0.0) { g_playerFireTickV22 = 0.0; }
        if (!g_playerInWaterV22) {
            g_playerUseCooldownV22 -= dt;
            if (g_playerUseCooldownV22 <= 0.0) { TakeDamage(1); g_playerUseCooldownV22 = 1.0; }
        } else { g_playerFireTickV22 = 0.0; }
    }

    if (g_playerUseCooldownV22 > 0.0 && !g_playerInFireV22) {
        g_playerUseCooldownV22 -= dt;
        if (g_playerUseCooldownV22 < 0.0) { g_playerUseCooldownV22 = 0.0; }
    }
}

void MobV39_UpdateBodyYaw(Mob *m, double moveX, double moveZ, double dt)
{
    double desired;
    double diff;
    if (!m) { return; }
    if (fabs(moveX) > 0.0001 || fabs(moveZ) > 0.0001) {
        desired = MobFaceYawFromDelta(moveX, moveZ);
        m->yaw = ApproachAngleDeg(m->yaw, desired, dt * 420.0);
    }
    m->renderYawOffset = ApproachAngleDeg(m->renderYawOffset, m->yaw, dt * 180.0);
    diff = NormalizeAngle180(m->yaw - m->renderYawOffset);
    if (diff < -75.0) { m->renderYawOffset = NormalizeAngle180(m->yaw + 75.0); }
    if (diff > 75.0) { m->renderYawOffset = NormalizeAngle180(m->yaw - 75.0); }
}

void MobV39_MoveEntity(Mob *m, double dx, double dy, double dz, double dt, int *mobOnGround)
{
    AxisAlignedBBV39 box;
    EntityMoveResultV39 move;
    double oldX;
    double oldY;
    double oldZ;
    int damage;
    if (!m) { return; }
    oldX = m->x;
    oldY = m->y;
    oldZ = m->z;
    box = EntityV39_GetMobBoxAt(m->type, m->x, m->y, m->z);
    EntityV39_MoveEntity(box, dx, dy, dz, ENTITY_V39_MOB_STEP_HEIGHT, (m->type != MOB_SQUID), &move);
    EntityV39_BoxToMobPosition(m, move.box);
    m->onGround = move.onGround;
    if (mobOnGround) { *mobOnGround = move.onGround; }
    if (fabs(move.movedX - dx) > ENTITY_V39_EPSILON) { m->vx = 0.0; }
    if (fabs(move.movedZ - dz) > ENTITY_V39_EPSILON) { m->vz = 0.0; }
    if (move.collidedVertically) {
        if (dy < 0.0 && !m->inWater && !m->inLava && !m->onLadder) {
            if (m->fallDistance > MOB_AI_FALL_SAFE_DISTANCE) {
                damage = (int)(m->fallDistance - MOB_AI_FALL_SAFE_DISTANCE);
                if (damage > 0) { MobAI_DamageSelfV24(m, damage, BLOCK_STONE, oldX, oldZ); }
            }
            m->fallDistance = 0.0;
        }
        m->vy = 0.0;
    } else if (dy < 0.0 && !m->inWater && !m->inLava && !m->onLadder) {
        m->fallDistance += -dy;
    } else if (m->inWater || m->inLava || m->onLadder) {
        m->fallDistance = 0.0;
    }
    MobV39_UpdateBodyYaw(m, m->x - oldX, m->z - oldZ, dt);
    if (m->x < 2.0) { m->x = 2.0; m->vx = 0.0; }
    if (m->z < 2.0) { m->z = 2.0; m->vz = 0.0; }
    if (m->x > WORLD_X - 3.0) { m->x = WORLD_X - 3.0; m->vx = 0.0; }
    if (m->z > WORLD_Z - 3.0) { m->z = WORLD_Z - 3.0; m->vz = 0.0; }
    if (m->y < 2.0) { m->y = 2.0; m->vy = 0.0; }
    if (m->y > WORLD_Y - 4) { m->y = WORLD_Y - 4; m->vy = 0.0; }
}

void MobV39_ApplyFriction(Mob *m, int blockBelow, double dt)
{
    double drag;
    if (!m) { return; }
    if (m->inWater) { drag = EntityV39_ExpDrag(ENTITY_V39_WATER_DRAG, dt); }
    else if (m->inLava) { drag = EntityV39_ExpDrag(ENTITY_V39_LAVA_DRAG, dt); }
    else if (m->onGround) { drag = EntityV39_ExpDrag(EntityV39_BlockSlipperiness(blockBelow) * 0.91, dt); }
    else { drag = EntityV39_ExpDrag(ENTITY_V39_AIR_DRAG, dt); }
    if (m->type == MOB_SLIME && m->onGround) { drag = EntityV39_ExpDrag(0.82, dt); }
    m->vx *= drag;
    m->vz *= drag;
}

int PlayerCollidesAt(double x, double y, double z);
int MovePlayerAxis(double dx, double dy, double dz);

void BreakBlockRaycast(void);
void PlaceBlockRaycast(void);

void SetupCamera(void);
void RenderWorld(void);

/* V29 forward declarations for late renderer passes used before their definitions. */
void RendererV8_PostWorldEffects(void);
void RendererV8_RenderSelectionOutline(void);
void RendererV8_RenderBreakingOverlay(void);
void RendererV8_RenderWeather3D(void);
void RendererV8_RenderTransparentBlocks(void);
void RendererV8_RenderSpecialEntities(void);
void RendererV8_RenderMobProjectiles(void);
void RendererV8_RenderFirstPersonItem(void);
void RendererV8_RenderMinecarts(void);

/* V29 compile-order prototypes for functions defined later in this single C file. */
void glFogi(GLenum pname, GLint param);
void gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz);
void TessellatorV8_BeginTerrain(GLuint tex);
void TessellatorV8_End(void);
int RendererV8_IsTranslucentBlock(int block);
int WorldGenV3_IsCrossPlantBlock(int block);
void RendererV8_ResetStats(void);
void RendererV8_RenderArrowEntity(SpecialEntityV6 *e);
void RendererV8_RenderThrowableEntity(SpecialEntityV6 *e);
void RendererV8_RenderFireballEntity(SpecialEntityV6 *e);
void RendererV8_RenderFishHookEntity(SpecialEntityV6 *e);
void RendererV8_RenderBoatEntity(SpecialEntityV6 *e);
void RendererV8_RenderPaintingEntity(SpecialEntityV6 *e);
void RendererV8_RenderTNTEntity(SpecialEntityV6 *e);
void RendererV8_RenderFallingBlockEntity(SpecialEntityV6 *e);
void RendererV8_RenderLightningEntity(SpecialEntityV6 *e);
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
