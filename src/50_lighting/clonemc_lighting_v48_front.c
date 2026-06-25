/* ============================================================
   CloneMC V51 section: LIGHTING / V48 QUEUE FRONT-END
   ============================================================ */

int GetSavedLightValueV48(int type, int x, int y, int z)
{
    if (y < 0) { return 0; }
    if (y >= WORLD_Y) { return type == ENUM_SKY_BLOCK_SKY_V48 ? 15 : 0; }
    if (!IsInsideWorld(x, y, z)) { return type == ENUM_SKY_BLOCK_SKY_V48 ? 15 : 0; }
    if (type == ENUM_SKY_BLOCK_SKY_V48) { return (int)skyLight[x][y][z]; }
    return (int)blockLight[x][y][z];
}

void SetLightValueV48(int type, int x, int y, int z, int value)
{
    if (!IsInsideWorld(x, y, z)) { return; }
    if (value < 0) { value = 0; }
    if (value > 15) { value = 15; }
    if (type == ENUM_SKY_BLOCK_SKY_V48) { skyLight[x][y][z] = (unsigned char)value; }
    else { blockLight[x][y][z] = (unsigned char)value; }
}

int CanExistingBlockSeeTheSkyV48(int x, int y, int z)
{
    int yy;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    if (y >= WORLD_Y - 1) { return 1; }
    for (yy = y + 1; yy < WORLD_Y; yy++) {
        if (GetBlockLightOpacityV48(GetBlock(x, yy, z)) >= 15) { return 0; }
    }
    return 1;
}

int ComputeLightTargetV48(int type, int x, int y, int z)
{
    int block;
    int opacity;
    int source;
    int best;
    int n;
    if (!IsInsideWorld(x, y, z)) { return type == ENUM_SKY_BLOCK_SKY_V48 ? 15 : 0; }
    block = GetBlock(x, y, z);
    opacity = GetBlockLightOpacityV48(block);
    if (opacity <= 0) { opacity = 1; }
    if (opacity > 15) { opacity = 15; }
    source = 0;
    if (type == ENUM_SKY_BLOCK_SKY_V48) { if (CanExistingBlockSeeTheSkyV48(x, y, z)) { source = 15; } }
    else { source = GetBlockLightValueSourceV48(block); }
    if (opacity >= 15 && source == 0) { return 0; }
    best = GetSavedLightValueV48(type, x - 1, y, z);
    n = GetSavedLightValueV48(type, x + 1, y, z); if (n > best) { best = n; }
    n = GetSavedLightValueV48(type, x, y - 1, z); if (n > best) { best = n; }
    n = GetSavedLightValueV48(type, x, y + 1, z); if (n > best) { best = n; }
    n = GetSavedLightValueV48(type, x, y, z - 1); if (n > best) { best = n; }
    n = GetSavedLightValueV48(type, x, y, z + 1); if (n > best) { best = n; }
    best -= opacity;
    if (best < 0) { best = 0; }
    if (source > best) { best = source; }
    if (best > 15) { best = 15; }
    return best;
}

void ClampLightBoxV48(MetadataChunkBlockV48 *b)
{
    if (b->minX < 0) { b->minX = 0; }
    if (b->minY < 0) { b->minY = 0; }
    if (b->minZ < 0) { b->minZ = 0; }
    if (b->maxX >= WORLD_X) { b->maxX = WORLD_X - 1; }
    if (b->maxY >= WORLD_Y) { b->maxY = WORLD_Y - 1; }
    if (b->maxZ >= WORLD_Z) { b->maxZ = WORLD_Z - 1; }
    if (b->minX > b->maxX) { b->minX = b->maxX; }
    if (b->minY > b->maxY) { b->minY = b->maxY; }
    if (b->minZ > b->maxZ) { b->minZ = b->maxZ; }
}

int LightBoxVolumeV48(MetadataChunkBlockV48 *b)
{
    int sx;
    int sy;
    int sz;
    sx = (int)b->maxX - (int)b->minX + 1;
    sy = (int)b->maxY - (int)b->minY + 1;
    sz = (int)b->maxZ - (int)b->minZ + 1;
    if (sx < 0 || sy < 0 || sz < 0) { return 0; }
    return sx * sy * sz;
}

void ScheduleLightingUpdateV48(int type, int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    MetadataChunkBlockV48 nb;
    MetadataChunkBlockV48 *b;
    int i;
    int oldVol;
    int newVol;
    nb.type = (unsigned char)type;
    nb.minX = (short)minX; nb.minY = (short)minY; nb.minZ = (short)minZ;
    nb.maxX = (short)maxX; nb.maxY = (short)maxY; nb.maxZ = (short)maxZ;
    ClampLightBoxV48(&nb);
    if (LightBoxVolumeV48(&nb) <= 0) { return; }
    for (i = 0; i < g_lightQueueCountV48; i++) {
        b = &g_lightQueueV48[i];
        if ((int)b->type != type) { continue; }
        if (minX >= (int)b->minX - 1 && minY >= (int)b->minY - 1 && minZ >= (int)b->minZ - 1 &&
            maxX <= (int)b->maxX + 1 && maxY <= (int)b->maxY + 1 && maxZ <= (int)b->maxZ + 1) {
            oldVol = LightBoxVolumeV48(b);
            if (nb.minX < b->minX) { b->minX = nb.minX; }
            if (nb.minY < b->minY) { b->minY = nb.minY; }
            if (nb.minZ < b->minZ) { b->minZ = nb.minZ; }
            if (nb.maxX > b->maxX) { b->maxX = nb.maxX; }
            if (nb.maxY > b->maxY) { b->maxY = nb.maxY; }
            if (nb.maxZ > b->maxZ) { b->maxZ = nb.maxZ; }
            newVol = LightBoxVolumeV48(b);
            if (newVol > LIGHT_V48_MAX_REGION_VOLUME && oldVol > 0) { *b = nb; }
            return;
        }
    }
    if (g_lightQueueCountV48 >= LIGHT_V48_QUEUE_MAX) { g_lightV48SkippedLarge++; return; }
    g_lightQueueV48[g_lightQueueCountV48] = nb;
    g_lightQueueCountV48++;
    if (type == ENUM_SKY_BLOCK_SKY_V48) { g_lightV48ScheduledSky++; }
    else { g_lightV48ScheduledBlock++; }
}

void NeighborLightPropagationChangedV48(int type, int x, int y, int z, int expected)
{
    int block;
    if (!IsInsideWorld(x, y, z)) { return; }
    if (type == ENUM_SKY_BLOCK_SKY_V48) { if (CanExistingBlockSeeTheSkyV48(x, y, z)) { expected = 15; } }
    else {
        block = GetBlock(x, y, z);
        if (GetBlockLightValueSourceV48(block) > expected) { expected = GetBlockLightValueSourceV48(block); }
    }
    if (expected < 0) { expected = 0; }
    if (expected > 15) { expected = 15; }
    if (GetSavedLightValueV48(type, x, y, z) != expected) { ScheduleLightingUpdateV48(type, x, y, z, x, y, z); }
}

void QueueBlockLightingAroundV48(int x, int y, int z, int radius)
{
    int r;
    r = radius;
    if (r < 6) { r = 6; }
    if (r > LIGHT_V48_MAX_BOX_SPAN) { r = LIGHT_V48_MAX_BOX_SPAN; }
    ScheduleLightingUpdateV48(ENUM_SKY_BLOCK_SKY_V48, x - r, y - r, z - r, x + r, WORLD_Y - 1, z + r);
    ScheduleLightingUpdateV48(ENUM_SKY_BLOCK_BLOCK_V48, x - r, y - r, z - r, x + r, y + r, z + r);
}

void ProcessMetadataChunkBlockV48(MetadataChunkBlockV48 box)
{
    int volume;
    int x;
    int y;
    int z;
    int oldValue;
    int newValue;
    int spread;
    ClampLightBoxV48(&box);
    volume = LightBoxVolumeV48(&box);
    if (volume <= 0) { return; }
    if (volume > LIGHT_V48_MAX_REGION_VOLUME) { g_lightV48SkippedLarge++; return; }
    for (x = box.minX; x <= box.maxX; x++) {
        for (z = box.minZ; z <= box.maxZ; z++) {
            for (y = box.minY; y <= box.maxY; y++) {
                oldValue = GetSavedLightValueV48((int)box.type, x, y, z);
                newValue = ComputeLightTargetV48((int)box.type, x, y, z);
                if (oldValue == newValue) { continue; }
                SetLightValueV48((int)box.type, x, y, z, newValue);
                g_lightV48ChangedCells++;
                spread = newValue - 1;
                if (spread < 0) { spread = 0; }
                NeighborLightPropagationChangedV48((int)box.type, x - 1, y, z, spread);
                NeighborLightPropagationChangedV48((int)box.type, x + 1, y, z, spread);
                NeighborLightPropagationChangedV48((int)box.type, x, y - 1, z, spread);
                NeighborLightPropagationChangedV48((int)box.type, x, y + 1, z, spread);
                NeighborLightPropagationChangedV48((int)box.type, x, y, z - 1, spread);
                NeighborLightPropagationChangedV48((int)box.type, x, y, z + 1, spread);
            }
        }
    }
    g_lightV48ProcessedBoxes++;
}

void ProcessLightUpdatesV48(int maxUpdates)
{
    MetadataChunkBlockV48 box;
    int i;
    if (maxUpdates < 1) { maxUpdates = 1; }
    for (i = 0; i < maxUpdates && g_lightQueueCountV48 > 0; i++) {
        box = g_lightQueueV48[0];
        g_lightQueueCountV48--;
        if (g_lightQueueCountV48 > 0) { memmove(&g_lightQueueV48[0], &g_lightQueueV48[1], sizeof(MetadataChunkBlockV48) * g_lightQueueCountV48); }
        ProcessMetadataChunkBlockV48(box);
    }
}


void PropagateLightArrayLocal(unsigned char light[WORLD_X][WORLD_Y][WORLD_Z], LightNode *queue, int startTail, int minX, int maxX, int minY, int maxY, int minZ, int maxZ);
void ClearLightArrays(void);
int BlocksLightForLighting(int block);
int GetBlockLightOpacityV42(int block);
int IsAOBlock(int block);
int GetBlockEmission(int block);
void PropagateLightArray(unsigned char light[WORLD_X][WORLD_Y][WORLD_Z], LightNode *queue, int startTail);
int PushLightNode(LightNode *queue, int *tail, int x, int y, int z);
int GetLegacyLightLevel(int x, int y, int z);
float ClampLightFloat(float v);
int GetMixedLightLevelV48(int x, int y, int z);
float GetMixedBrightnessV48(int x, int y, int z);
void ComputeFaceVertexBrightnessV48(int x, int y, int z, int face, int block, float *b0, float *b1, float *b2, float *b3);
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
int AddDroppedItemLocalV29(int item, int count, double x, double y, double z, double vx, double vy, double vz);
void DropSelectedItem(void);
void DropSelectedItemV29(void);
void UpdateDroppedItems(double dt);
void RenderDroppedItems(void);
void DrawDroppedItem(DroppedItem *d);
void DrawDroppedBlockCube(int block);
int DroppedItemBoxBlockedV27(double x, double y, double z);
void RebaseDroppedItemsAfterWorldStream(int oldOriginX, int oldOriginZ);
int ShouldRenderBlockItemAs3DV27(int block);
int ItemRenderV46_ShouldPreferItemIcon(int item);
int ResolveItemDrawTileV27(int item, int *col, int *row, int *useTerrain);
void DrawItemCardLocalV27(int item, float size, float thickness);
void DrawTerrainItemCardLocalV27(int block, float size, float thickness);
void DrawTexturedItemCardUVV27(GLuint tex, int atlasW, int atlasH, int col, int row, float size, float thickness);
void DrawDroppedItemSpriteUVV31(GLuint tex, int atlasW, int atlasH, int col, int row, float size);
int ItemRenderV38_GetRenderCopies(int count);
float ItemRenderV38_CopyOffset(int item, int damage, int copy, int axis, float spread);
float ItemRenderV38_GetDroppedBlockScale(int block);
void DrawDroppedItemVisualV38(int item, int count, int damage, double age, double hoverStart, double x, double y, double z, int pickupFx);
int ItemRenderV38_GetRenderCopies(int count);
float ItemRenderV38_CopyOffset(int item, int damage, int copy, int axis, float spread);
float ItemRenderV38_GetDroppedBlockScale(int block);
void DrawDroppedItemVisualV38(int item, int count, int damage, double age, double hoverStart, double x, double y, double z, int pickupFx);
void DrawTorchBlock(int x, int y, int z);
void DrawPlayerThirdPerson(void);
void SavePath_LegacyRegionFileV29(int slot, char *path);
void SaveLoadedWindowBeforeStreamingV29(void);
void LoadGeneratedWindowAfterStreamingV29(void);
