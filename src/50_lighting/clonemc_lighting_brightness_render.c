/* ============================================================
   CloneMC V51 section: LIGHTING / EMISSION / PROPAGATION / FACE BRIGHTNESS
   ============================================================ */

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

int GetBlockLightOpacityV42(int block)
{
    return GetBlockLightOpacityV48(block);
}

int GetBlockLightOpacityV48(int block)
{
    BlockDefV19 *d;

    if (block == BLOCK_AIR) { return 1; }
    if (block == BLOCK_WATER) { return 3; }
    if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { return 1; }
    if (block == BLOCK_ICE) { return 3; }
    if (block == BLOCK_LEAVES) { return 1; }
    if (block == BLOCK_GLASS) { return 1; }
    if (block == BLOCK_PORTAL) { return 1; }
    if (block == BLOCK_FIRE || block == BLOCK_TORCH || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) { return 1; }

    if (block == BLOCK_STEP || block == BLOCK_DOUBLE_STEP || block == BLOCK_WOOD_STAIRS ||
        block == BLOCK_COBBLESTONE_STAIRS || block == BLOCK_FARMLAND || block == BLOCK_SNOW ||
        block == BLOCK_CAKE || block == BLOCK_BED || block == BLOCK_TRAPDOOR ||
        block == BLOCK_PISTON_EXTENSION) { return 1; }

    if (block == BLOCK_CROPS || block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH ||
        block == BLOCK_FLOWER_YELLOW || block == BLOCK_FLOWER_RED ||
        block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED || block == BLOCK_SAPLING ||
        block == BLOCK_REED || block == BLOCK_RAIL || block == BLOCK_DETECTOR_RAIL ||
        block == BLOCK_REDSTONE_WIRE || block == BLOCK_LADDER || block == BLOCK_SIGN_POST ||
        block == BLOCK_SIGN_WALL || block == BLOCK_LEVER || block == BLOCK_STONE_BUTTON ||
        block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE ||
        block == BLOCK_WOOD_DOOR || block == BLOCK_IRON_DOOR) { return 1; }

    d = BlockRegistryV19_Get(block);
    if (d->opaque) { return 15; }
    return 1;
}

int GetBlockLightValueSourceV48(int block)
{
    int v;
    v = GetBlockEmission(block);
    if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { if (v < 15) { v = 15; } }
    if (block == BLOCK_FIRE) { if (v < 15) { v = 15; } }
    if (block == BLOCK_TORCH) { if (v < 14) { v = 14; } }
    if (block == BLOCK_REDSTONE_TORCH_ON) { if (v < 7) { v = 7; } }
    if (block == BLOCK_GLOWSTONE) { if (v < 15) { v = 15; } }
    if (v < 0) { v = 0; }
    if (v > 15) { v = 15; }
    return v;
}


int BlocksLightForLighting(int block)
{
    return GetBlockLightOpacityV42(block) >= 15 ? 1 : 0;
}



int IsAOBlock(int block)
{
    /* Use the same transparency policy for smoother corner shading. */
    if (!BlocksLightForLighting(block)) {
        return 0;
    }
    return 1;
}


int GetBlockEmission(int block)
{
    BlockDefV19 *d;
    d = BlockRegistryV19_Get(block);
    return d->lightEmit;
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

            {
                int opacity;
                opacity = GetBlockLightOpacityV42(GetBlock(nx, ny, nz));
                if (opacity >= 15) { continue; }
                nextLevel = level - opacity;
                if (nextLevel < 0) { nextLevel = 0; }
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
    int tail;
    int emission;
    LightNode *queue;

    ClearLightArrays();
    g_lightQueueCountV48 = 0;
    g_lightV48ProcessedBoxes = 0;
    g_lightV48ChangedCells = 0;
    g_lightV48SkippedLarge = 0;
    queue = (LightNode *)malloc(sizeof(LightNode) * MAX_LIGHT_NODES);
    if (!queue) { return; }

    /* Full skylight flood: seed the top open sky and let it decay through caves,
       ravines, glass, leaves and open sides.  This makes closed caves dark instead
       of receiving fake vertical brightness. */
    tail = 0;
    for (x = 0; x < WORLD_X; x++) {
        for (z = 0; z < WORLD_Z; z++) {
            for (y = WORLD_Y - 1; y >= 0; y--) {
                block = GetBlock(x, y, z);
                if (BlocksLightForLighting(block)) { break; }
                skyLight[x][y][z] = LIGHT_MAX_LEVEL;
                PushLightNode(queue, &tail, x, y, z);
            }
        }
    }
    PropagateLightArray(skyLight, queue, tail);

    /* Full artificial-light flood.  This is called on world generation/load or
       larger lighting repairs; block edits use RecomputeLegacyLightingLocal(). */
    tail = 0;
    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                emission = GetBlockEmission(GetBlock(x, y, z));
                if (emission > 0) {
                    blockLight[x][y][z] = (unsigned char)emission;
                    PushLightNode(queue, &tail, x, y, z);
                }
            }
        }
    }
    PropagateLightArray(blockLight, queue, tail);
    free(queue);
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

            {
                int opacity;
                opacity = GetBlockLightOpacityV42(GetBlock(nx, ny, nz));
                if (opacity >= 15) { continue; }
                nextLevel = level - opacity;
                if (nextLevel < 0) { nextLevel = 0; }
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
    int minX;
    int maxX;
    int minY;
    int maxY;
    int minZ;
    int maxZ;
    int pad;
    int x;
    int y;
    int z;
    int block;
    int tail;
    int emission;
    LightNode *queue;

    if (radius < 8) { radius = 8; }
    if (radius > 30) { radius = 30; }
    pad = 2;
    minX = cx - radius - pad;
    maxX = cx + radius + pad;
    minY = cy - radius - pad;
    maxY = cy + radius + pad;
    minZ = cz - radius - pad;
    maxZ = cz + radius + pad;
    if (minX < 0) { minX = 0; }
    if (minY < 0) { minY = 0; }
    if (minZ < 0) { minZ = 0; }
    if (maxX >= WORLD_X) { maxX = WORLD_X - 1; }
    if (maxY >= WORLD_Y) { maxY = WORLD_Y - 1; }
    if (maxZ >= WORLD_Z) { maxZ = WORLD_Z - 1; }

    queue = (LightNode *)malloc(sizeof(LightNode) * MAX_LIGHT_NODES);
    if (!queue) { return; }

    for (x = minX; x <= maxX; x++) {
        for (y = minY; y <= maxY; y++) {
            for (z = minZ; z <= maxZ; z++) {
                if (!(x == minX || x == maxX || y == minY || y == maxY || z == minZ || z == maxZ)) {
                    skyLight[x][y][z] = 0;
                    blockLight[x][y][z] = 0;
                }
            }
        }
    }

    tail = 0;
    for (x = minX; x <= maxX; x++) {
        for (z = minZ; z <= maxZ; z++) {
            for (y = WORLD_Y - 1; y >= minY; y--) {
                block = GetBlock(x, y, z);
                if (BlocksLightForLighting(block)) { break; }
                if (y <= maxY) {
                    skyLight[x][y][z] = LIGHT_MAX_LEVEL;
                    PushLightNode(queue, &tail, x, y, z);
                }
            }
        }
    }
    /* Re-seed border light so a local edit can pull existing light back inward. */
    for (x = minX; x <= maxX; x++) {
        for (y = minY; y <= maxY; y++) {
            for (z = minZ; z <= maxZ; z++) {
                if (x == minX || x == maxX || y == minY || y == maxY || z == minZ || z == maxZ) {
                    if ((int)skyLight[x][y][z] > 0) { PushLightNode(queue, &tail, x, y, z); }
                }
            }
        }
    }
    PropagateLightArrayLocal(skyLight, queue, tail, minX, maxX, minY, maxY, minZ, maxZ);

    tail = 0;
    for (x = minX; x <= maxX; x++) {
        for (y = minY; y <= maxY; y++) {
            for (z = minZ; z <= maxZ; z++) {
                emission = GetBlockEmission(GetBlock(x, y, z));
                if (emission > 0) {
                    blockLight[x][y][z] = (unsigned char)emission;
                    PushLightNode(queue, &tail, x, y, z);
                } else if (x == minX || x == maxX || y == minY || y == maxY || z == minZ || z == maxZ) {
                    if ((int)blockLight[x][y][z] > 0) { PushLightNode(queue, &tail, x, y, z); }
                }
            }
        }
    }
    PropagateLightArrayLocal(blockLight, queue, tail, minX, maxX, minY, maxY, minZ, maxZ);
    free(queue);
}



/* V40: compatibility wrapper for newer systems that call GetLightLevel().
   Keep cave ambience, AI darkness checks, and future Java-style code paths using
   one stable name while preserving the existing legacy skylight/block-light arrays. */
int GetLightLevel(int x, int y, int z)
{
    return GetLegacyLightLevel(x, y, z);
}

int GetLegacyLightLevel(int x, int y, int z)
{
    return GetMixedLightLevelV48(x, y, z);
}

int GetMixedLightLevelV48(int x, int y, int z)
{
    int sky;
    int block;
    if (!IsInsideWorld(x, y, z)) { return LIGHT_MAX_LEVEL; }
    sky = (int)skyLight[x][y][z] - g_skyLightSubtractedV48;
    if (sky < 0) { sky = 0; }
    block = (int)blockLight[x][y][z];
    return block > sky ? block : sky;
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
    if (level < 0) { level = 0; }
    if (level > LIGHT_MAX_LEVEL) { level = LIGHT_MAX_LEVEL; }
    f = (float)level / 15.0f;
    return ClampLightFloat(0.05f + f * f * 0.95f);
}

float GetMixedBrightnessV48(int x, int y, int z)
{
    int sky;
    int block;
    float skyBrightness;
    float blockBrightness;
    float brightness;
    if (!IsInsideWorld(x, y, z)) { return ApplyGammaBoost(g_daySkyBrightness); }
    sky = (int)skyLight[x][y][z] - g_skyLightSubtractedV48;
    if (sky < 0) { sky = 0; }
    block = (int)blockLight[x][y][z];
    skyBrightness = LegacyLevelToBrightness(sky) * g_daySkyBrightness;
    blockBrightness = LegacyLevelToBrightness(block);
    brightness = blockBrightness > skyBrightness ? blockBrightness : skyBrightness;
    brightness = brightness * (0.75f + ((float)g_highGamma * 0.08f));
    return ApplyGammaBoost(brightness);
}


void ComputeFaceVertexBrightnessV48(int x, int y, int z, int face, int block, float *b0, float *b1, float *b2, float *b3)
{
    int sx;
    int sy;
    int sz;
    sx = x; sy = y; sz = z;
    if (block == BLOCK_LIGHT || block == BLOCK_GLOWSTONE || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA || block == BLOCK_FIRE) {
        *b0 = 1.0f; *b1 = 1.0f; *b2 = 1.0f; *b3 = 1.0f; return;
    }
    if (face == 0) { sy = y + 1; }
    else if (face == 1) { sy = y - 1; }
    else if (face == 2) { sz = z - 1; }
    else if (face == 3) { sz = z + 1; }
    else if (face == 4) { sx = x - 1; }
    else if (face == 5) { sx = x + 1; }
    if (!g_videoSmoothLightingV7) { *b0 = GetMixedBrightnessV48(sx, sy, sz); *b1 = *b0; *b2 = *b0; *b3 = *b0; return; }
    if (face == 0) {
        *b0 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x - 1, sy, z) + GetMixedBrightnessV48(x, sy, z - 1) + GetMixedBrightnessV48(x - 1, sy, z - 1)) * 0.25f;
        *b1 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x - 1, sy, z) + GetMixedBrightnessV48(x, sy, z + 1) + GetMixedBrightnessV48(x - 1, sy, z + 1)) * 0.25f;
        *b2 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x + 1, sy, z) + GetMixedBrightnessV48(x, sy, z + 1) + GetMixedBrightnessV48(x + 1, sy, z + 1)) * 0.25f;
        *b3 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x + 1, sy, z) + GetMixedBrightnessV48(x, sy, z - 1) + GetMixedBrightnessV48(x + 1, sy, z - 1)) * 0.25f;
    } else if (face == 1) {
        *b0 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x - 1, sy, z) + GetMixedBrightnessV48(x, sy, z + 1) + GetMixedBrightnessV48(x - 1, sy, z + 1)) * 0.25f;
        *b1 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x - 1, sy, z) + GetMixedBrightnessV48(x, sy, z - 1) + GetMixedBrightnessV48(x - 1, sy, z - 1)) * 0.25f;
        *b2 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x + 1, sy, z) + GetMixedBrightnessV48(x, sy, z - 1) + GetMixedBrightnessV48(x + 1, sy, z - 1)) * 0.25f;
        *b3 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x + 1, sy, z) + GetMixedBrightnessV48(x, sy, z + 1) + GetMixedBrightnessV48(x + 1, sy, z + 1)) * 0.25f;
    } else if (face == 2) {
        *b0 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x + 1, y, sz) + GetMixedBrightnessV48(x, y - 1, sz) + GetMixedBrightnessV48(x + 1, y - 1, sz)) * 0.25f;
        *b1 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x - 1, y, sz) + GetMixedBrightnessV48(x, y - 1, sz) + GetMixedBrightnessV48(x - 1, y - 1, sz)) * 0.25f;
        *b2 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x - 1, y, sz) + GetMixedBrightnessV48(x, y + 1, sz) + GetMixedBrightnessV48(x - 1, y + 1, sz)) * 0.25f;
        *b3 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x + 1, y, sz) + GetMixedBrightnessV48(x, y + 1, sz) + GetMixedBrightnessV48(x + 1, y + 1, sz)) * 0.25f;
    } else if (face == 3) {
        *b0 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x - 1, y, sz) + GetMixedBrightnessV48(x, y - 1, sz) + GetMixedBrightnessV48(x - 1, y - 1, sz)) * 0.25f;
        *b1 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x + 1, y, sz) + GetMixedBrightnessV48(x, y - 1, sz) + GetMixedBrightnessV48(x + 1, y - 1, sz)) * 0.25f;
        *b2 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x + 1, y, sz) + GetMixedBrightnessV48(x, y + 1, sz) + GetMixedBrightnessV48(x + 1, y + 1, sz)) * 0.25f;
        *b3 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(x - 1, y, sz) + GetMixedBrightnessV48(x, y + 1, sz) + GetMixedBrightnessV48(x - 1, y + 1, sz)) * 0.25f;
    } else if (face == 4) {
        *b0 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(sx, y - 1, z) + GetMixedBrightnessV48(sx, y, z - 1) + GetMixedBrightnessV48(sx, y - 1, z - 1)) * 0.25f;
        *b1 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(sx, y - 1, z) + GetMixedBrightnessV48(sx, y, z + 1) + GetMixedBrightnessV48(sx, y - 1, z + 1)) * 0.25f;
        *b2 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(sx, y + 1, z) + GetMixedBrightnessV48(sx, y, z + 1) + GetMixedBrightnessV48(sx, y + 1, z + 1)) * 0.25f;
        *b3 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(sx, y + 1, z) + GetMixedBrightnessV48(sx, y, z - 1) + GetMixedBrightnessV48(sx, y + 1, z - 1)) * 0.25f;
    } else {
        *b0 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(sx, y - 1, z) + GetMixedBrightnessV48(sx, y, z + 1) + GetMixedBrightnessV48(sx, y - 1, z + 1)) * 0.25f;
        *b1 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(sx, y - 1, z) + GetMixedBrightnessV48(sx, y, z - 1) + GetMixedBrightnessV48(sx, y - 1, z - 1)) * 0.25f;
        *b2 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(sx, y + 1, z) + GetMixedBrightnessV48(sx, y, z - 1) + GetMixedBrightnessV48(sx, y + 1, z - 1)) * 0.25f;
        *b3 = (GetMixedBrightnessV48(sx, sy, sz) + GetMixedBrightnessV48(sx, y + 1, z) + GetMixedBrightnessV48(sx, y, z + 1) + GetMixedBrightnessV48(sx, y + 1, z + 1)) * 0.25f;
    }
}


float GetLegacyFaceBrightness(int x, int y, int z, int face, int block)
{
    int sx;
    int sy;
    int sz;
    sx = x; sy = y; sz = z;
    if (face == 0) { sy = y + 1; }
    else if (face == 1) { sy = y - 1; }
    else if (face == 2) { sz = z - 1; }
    else if (face == 3) { sz = z + 1; }
    else if (face == 4) { sx = x - 1; }
    else if (face == 5) { sx = x + 1; }
    if (block == BLOCK_LIGHT || block == BLOCK_GLOWSTONE || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA || block == BLOCK_FIRE) { return 1.0f; }
    return GetMixedBrightnessV48(sx, sy, sz);
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
/* Missing MobModelAI V4 compatibility definitions              */
/* ------------------------------------------------------------ */

