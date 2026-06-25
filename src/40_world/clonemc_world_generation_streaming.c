/* ============================================================
   CloneMC V55 section: WORLD / TERRAIN / INFINITE STREAMING / PARTICLES / DROPPED ITEM WORLD LOOP
   ============================================================ */

void GenerateWorld(void)
{
    int startChunkX;
    int startChunkZ;

    /*
        V55 terrain bootstrap.
        Infinite worlds are back: the engine still keeps only a 128x128
        block streaming window in RAM for Win98-friendly memory use, but
        the global generator no longer stops at 862x862 unless the world
        was created with the finite 862x862 option.
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
                } else if (g_fastWorldGenV13) {
                    /* V13 legacy-performance terrain path.
                       Use the original deterministic 2D Perlin height field as the
                       backbone and skip the expensive per-block 3D density scan.
                       Caves and ores are still carved afterward, but chunk streaming
                       no longer performs hundreds of thousands of 3D noise calls. */
                    if (y <= h) {
                        world[x][y][z] = BLOCK_STONE;
                    } else if (y <= GEN_WATER_LEVEL && biomeMap[x][z] == BIOME_OCEAN) {
                        world[x][y][z] = BLOCK_WATER;
                    } else {
                        world[x][y][z] = BLOCK_AIR;
                    }
                } else if (y < h - 8) {
                    world[x][y][z] = BLOCK_STONE;
                } else if (y > h + 4) {
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
    WorldGenV20_AddCaveAndSpringFinishing();

    AddOrePass();
    WorldGenV20_AddJavaOrePass();

    AddLiquidPass();
    AddSurfaceTexturePass();
    WorldGenV58_ApplyTerrainSurfacePass();

    WorldGenV20_AddLakesAndDungeonsPass();
    WorldGenV58_ApplyTerrainSurfacePass();
    AddWorldFeatures();
    WorldGenV20_AddDecorationPass();
    WorldGenV58_AddBiomeDecorationPass();
    WorldGenV58_ApplyFinalTerrainCleanupPass();
    WorldGenV20_EnsureGeneratedTileEntities();

    RemovePlantsFromWorldV12();
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

void SaveLoadedWindowBeforeStreamingV29(void)
{
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    SaveHandler_SaveRegionWindowV2();
    SaveCurrentRegionLite();
    SaveCurrentTileEntities();
    SaveRedstoneMetaV5();
}

void LoadGeneratedWindowAfterStreamingV29(void)
{
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    if (!SaveHandler_LoadRegionWindowV2()) {
        LoadCurrentRegionLite();
    }
    LoadCurrentTileEntities();
    LoadRedstoneMetaV5();
    RebuildColumnTops();
    InvalidateAllTerrainChunkMeshes();
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

    SaveLoadedWindowBeforeStreamingV29();
    GenerateWorldWindow(newCenterChunkX, newCenterChunkZ);
    LoadGeneratedWindowAfterStreamingV29();

    playerX = globalX - (double)worldOriginBlockX;
    playerZ = globalZ - (double)worldOriginBlockZ;

    KeepPlayerSafeAfterStreaming();

    /* New chunks get cheap skylight values without expensive full flood-fill. */
    ComputeLegacyLighting();

    /* Keep existing mobs and EntityItem-style drops by rebasing their local
       positions to the new world window.  Without this, dropped blocks keep
       old local coordinates and appear scattered after streaming. */
    RebaseMobsAfterWorldStream(oldOriginX, oldOriginZ);
    RebaseDroppedItemsAfterWorldStream(oldOriginX, oldOriginZ);
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


/* WORLDGEN_V3_CONVERSION_PASS_BEGIN
   Java world/chunk generation conversion pass for the CloneMC personal-use C port.
   This section maps the responsibilities from ChunkProviderGenerate, WorldChunkManager,
   BiomeGenBase subclasses, MapGenBase/MapGenCaves, and WorldGen* feature classes into
   original Open Watcom-friendly C89 functions.  It keeps the global-coordinate rule so
   chunks line up when the 128x128 working window streams on Windows 98-class machines. */

typedef struct WorldGenV3ClimateTag {
    double temperature;
    double humidity;
    double weirdness;
    double ocean;
    double erosion;
    double mountain;
} WorldGenV3Climate;

int WorldGenV3_JavaRand01(int x, int y, int z, int salt)
{
    return WorldHash3D(x, y, z, g_worldSeed + salt) & 0x7fffffff;
}

double WorldGenV3_UnitNoise2D(int gx, int gz, double sx, double sz, int salt, int octaves, double persistence)
{
    double v;
    v = WorldFractal2D((double)gx * sx, (double)gz * sz, g_worldSeed + salt, octaves, persistence);
    v = (v + 1.0) * 0.5;
    return ClampDouble(v, 0.0, 1.0);
}

void WorldGenV3_ComputeClimate(int gx, int gz, WorldGenV3Climate *c)
{
    double temp;
    double hum;
    double weird;
    double ocean;
    double erosion;
    double mountain;

    if (!c) { return; }

    temp = WorldGenV3_UnitNoise2D(gx, gz, 0.00285, 0.00285, 8841, 4, 0.50);
    hum = WorldGenV3_UnitNoise2D(gx, gz, 0.00320, 0.00320, 8842, 4, 0.50);
    weird = WorldGenV3_UnitNoise2D(gx, gz, 0.01750, 0.01750, 8843, 2, 0.55);
    ocean = WorldGenV3_UnitNoise2D(gx, gz, 0.00165, 0.00165, 8844, 4, 0.54);
    erosion = WorldGenV3_UnitNoise2D(gx, gz, 0.00600, 0.00600, 8845, 3, 0.52);
    mountain = BetaMountainMask(gx, gz);

    /* WorldChunkManager-like adjustment: add a tiny detail field, then square
       temperature back toward warmer values like the Java manager did. */
    temp = temp * 0.96 + weird * 0.04;
    hum = hum * 0.98 + weird * 0.02;
    temp = 1.0 - (1.0 - temp) * (1.0 - temp);

    c->temperature = ClampDouble(temp, 0.0, 1.0);
    c->humidity = ClampDouble(hum, 0.0, 1.0);
    c->weirdness = ClampDouble(weird, 0.0, 1.0);
    c->ocean = ClampDouble(ocean, 0.0, 1.0);
    c->erosion = ClampDouble(erosion, 0.0, 1.0);
    c->mountain = ClampDouble(mountain, 0.0, 1.0);
}

int WorldGenV3_BiomeFromClimate(WorldGenV3Climate *c)
{
    double t;
    double h;
    double th;
    if (!c) { return BIOME_PLAINS; }
    t = c->temperature;
    h = c->humidity;
    th = t * h;

    if (c->ocean < 0.185) { return BIOME_OCEAN; }
    if (t < 0.105) { return BIOME_TUNDRA; }
    if (th < 0.200) {
        if (t < 0.500) { return BIOME_TUNDRA; }
        if (t < 0.950) { return BIOME_SHRUBLAND; }
        return BIOME_DESERT;
    }
    if (th > 0.800 && t > 0.700) { return BIOME_RAINFOREST; }
    if (h > 0.760 && t > 0.480) { return BIOME_SWAMPLAND; }
    if (t < 0.420) { return BIOME_TAIGA; }
    if (h > 0.560) {
        if (c->mountain > 0.560) { return BIOME_SEASONAL_FOREST; }
        return BIOME_FOREST;
    }
    if (h < 0.330) { return BIOME_SHRUBLAND; }
    return BIOME_PLAINS;
}

int WorldGenV3_IsColdBiome(int biome)
{
    if (biome == BIOME_TUNDRA || biome == BIOME_TAIGA) { return 1; }
    return 0;
}

int WorldGenV3_IsWetBiome(int biome)
{
    if (biome == BIOME_SWAMPLAND || biome == BIOME_RAINFOREST || biome == BIOME_OCEAN) { return 1; }
    return 0;
}

int WorldGenV3_TopBlockForBiome(int biome, int y, int belowWater)
{
    if (biome == BIOME_DESERT) { return BLOCK_SAND; }
    if (biome == BIOME_OCEAN) { return BLOCK_SAND; }
    if (biome == BIOME_SWAMPLAND && y <= GEN_WATER_LEVEL + 1) { return BLOCK_DIRT; }
    if (belowWater) { return BLOCK_SAND; }
    return BLOCK_GRASS;
}

int WorldGenV3_FillerBlockForBiome(int biome, int y)
{
    (void)y;
    if (biome == BIOME_DESERT || biome == BIOME_OCEAN) { return BLOCK_SAND; }
    if (biome == BIOME_SWAMPLAND) { return BLOCK_DIRT; }
    return BLOCK_DIRT;
}

int WorldGenV3_BlockIsNaturalGround(int block)
{
    if (block == BLOCK_GRASS || block == BLOCK_DIRT || block == BLOCK_SAND ||
        block == BLOCK_CLAY || block == BLOCK_SNOW_BLOCK) { return 1; }
    return 0;
}

int WorldGenV3_BlockIsStoneLike(int block)
{
    if (block == BLOCK_STONE || block == BLOCK_DIRT || block == BLOCK_GRAVEL ||
        block == BLOCK_SAND || block == BLOCK_SANDSTONE) { return 1; }
    return 0;
}

void WorldGenV3_SetBlockGen(int x, int y, int z, int block)
{
    if (x < 0 || x >= WORLD_X || y < 0 || y >= WORLD_Y || z < 0 || z >= WORLD_Z) { return; }
    world[x][y][z] = block;
}

int WorldGenV3_FindTopGroundY(int x, int z)
{
    int y;
    int b;
    if (x < 0 || x >= WORLD_X || z < 0 || z >= WORLD_Z) { return 0; }
    for (y = WORLD_Y - 2; y >= 1; y--) {
        b = world[x][y][z];
        if (b != BLOCK_AIR && b != BLOCK_WATER && b != BLOCK_LAVA && b != BLOCK_SNOW) { return y; }
    }
    return 0;
}

int WorldGenV3_FindTopAirY(int x, int z)
{
    int ground;
    ground = WorldGenV3_FindTopGroundY(x, z);
    if (ground + 1 >= WORLD_Y) { return WORLD_Y - 1; }
    return ground + 1;
}

int WorldGenV3_HasAdjacentWater(int x, int y, int z)
{
    if (GetBlock(x + 1, y, z) == BLOCK_WATER) { return 1; }
    if (GetBlock(x - 1, y, z) == BLOCK_WATER) { return 1; }
    if (GetBlock(x, y, z + 1) == BLOCK_WATER) { return 1; }
    if (GetBlock(x, y, z - 1) == BLOCK_WATER) { return 1; }
    if (GetBlock(x + 1, y - 1, z) == BLOCK_WATER) { return 1; }
    if (GetBlock(x - 1, y - 1, z) == BLOCK_WATER) { return 1; }
    if (GetBlock(x, y - 1, z + 1) == BLOCK_WATER) { return 1; }
    if (GetBlock(x, y - 1, z - 1) == BLOCK_WATER) { return 1; }
    return 0;
}

int WorldGenV3_CanPlacePlantAt(int x, int y, int z, int plant)
{
    int below;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    if (GetBlock(x, y, z) != BLOCK_AIR) { return 0; }
    below = GetBlock(x, y - 1, z);
    if (plant == BLOCK_DEAD_BUSH) { return below == BLOCK_SAND; }
    if (plant == BLOCK_REED) {
        if (!(below == BLOCK_GRASS || below == BLOCK_DIRT || below == BLOCK_SAND)) { return 0; }
        return WorldGenV3_HasAdjacentWater(x, y - 1, z);
    }
    if (plant == BLOCK_MUSHROOM_BROWN || plant == BLOCK_MUSHROOM_RED) {
        if (below == BLOCK_GRASS || below == BLOCK_DIRT || below == BLOCK_STONE || below == BLOCK_MOSSY_COBBLESTONE) { return 1; }
        return 0;
    }
    if (below == BLOCK_GRASS || below == BLOCK_DIRT) { return 1; }
    return 0;
}

void WorldGenV3_PlacePlantAt(int x, int y, int z, int plant)
{
    if (WorldGenV3_CanPlacePlantAt(x, y, z, plant)) { WorldGenV3_SetBlockGen(x, y, z, plant); }
}

void WorldGenV3_AddOreVeinJava(int gx, int gy, int gz, int oreBlock, int blocks, int salt)
{
    int lx;
    int lz;
    int i;
    double angle;
    double x1;
    double z1;
    double x2;
    double z2;
    double y1;
    double y2;
    double t;
    double px;
    double py;
    double pz;
    double r;
    int rx;
    int ry;
    int rz;
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

    lx = GlobalToLocalBlockX(gx);
    lz = GlobalToLocalBlockZ(gz);
    if (!IsInsideWorld(lx, gy, lz)) { return; }

    angle = ((double)(WorldHash3D(gx, gy, gz, g_worldSeed + salt) % 6283)) / 1000.0;
    x1 = (double)gx + sin(angle) * (double)blocks / 8.0;
    x2 = (double)gx - sin(angle) * (double)blocks / 8.0;
    z1 = (double)gz + cos(angle) * (double)blocks / 8.0;
    z2 = (double)gz - cos(angle) * (double)blocks / 8.0;
    y1 = (double)gy + ((double)(WorldHash3D(gx, gy, gz, g_worldSeed + salt + 11) % 3) - 1.0);
    y2 = (double)gy + ((double)(WorldHash3D(gx, gy, gz, g_worldSeed + salt + 12) % 3) - 1.0);

    for (i = 0; i < blocks; i++) {
        t = (double)i / (double)blocks;
        px = x1 + (x2 - x1) * t;
        py = y1 + (y2 - y1) * t;
        pz = z1 + (z2 - z1) * t;
        r = (sin(t * PI) + 1.0) * 0.5 + 0.35;
        rx = (int)(r * 1.8);
        ry = (int)(r * 1.2);
        rz = (int)(r * 1.8);
        if (rx < 1) { rx = 1; }
        if (ry < 1) { ry = 1; }
        if (rz < 1) { rz = 1; }
        lx = GlobalToLocalBlockX((int)floor(px));
        lz = GlobalToLocalBlockZ((int)floor(pz));
        for (dx = -rx; dx <= rx; dx++) {
            for (dy = -ry; dy <= ry; dy++) {
                for (dz = -rz; dz <= rz; dz++) {
                    x = lx + dx;
                    y = (int)floor(py) + dy;
                    z = lz + dz;
                    if (!IsInsideWorld(x, y, z)) { continue; }
                    nx = (double)dx / (double)rx;
                    ny = (double)dy / (double)ry;
                    nz = (double)dz / (double)rz;
                    d = nx * nx + ny * ny + nz * nz;
                    if (d <= 1.0 && CanReplaceStoneWithOre(x, y, z)) { world[x][y][z] = oreBlock; }
                }
            }
        }
    }
}

void WorldGenV3_CarveLakeLocal(int cx, int cy, int cz, int liquidBlock, int salt)
{
    int dx;
    int dy;
    int dz;
    int x;
    int y;
    int z;
    int rx;
    int ry;
    int rz;
    double nx;
    double ny;
    double nz;
    double d;
    int block;
    int waterLine;

    rx = 5 + (WorldHash3D(cx, cy, cz, g_worldSeed + salt) % 3);
    ry = 3 + (WorldHash3D(cx, cy, cz, g_worldSeed + salt + 1) % 2);
    rz = 5 + (WorldHash3D(cx, cy, cz, g_worldSeed + salt + 2) % 3);
    waterLine = cy;

    if (!IsInsideWorld(cx, cy, cz)) { return; }
    if (cy < 8 || cy > WORLD_Y - 12) { return; }

    for (dx = -rx - 1; dx <= rx + 1; dx++) {
        for (dy = -ry - 1; dy <= ry + 1; dy++) {
            for (dz = -rz - 1; dz <= rz + 1; dz++) {
                x = cx + dx;
                y = cy + dy;
                z = cz + dz;
                if (!IsInsideWorld(x, y, z)) { continue; }
                nx = (double)dx / (double)rx;
                ny = (double)dy / (double)ry;
                nz = (double)dz / (double)rz;
                d = nx * nx + ny * ny * 1.6 + nz * nz;
                if (d <= 1.0) {
                    if (y <= waterLine) { block = liquidBlock; }
                    else { block = BLOCK_AIR; }
                    if (world[x][y][z] != BLOCK_BORDER) { world[x][y][z] = block; }
                }
            }
        }
    }
}

void WorldGenV3_AddClayDisk(int cx, int cy, int cz, int salt)
{
    int radius;
    int dx;
    int dz;
    int dy;
    int x;
    int y;
    int z;
    double d;
    radius = 4 + (WorldHash3D(cx, cy, cz, g_worldSeed + salt) % 4);
    for (dx = -radius; dx <= radius; dx++) {
        for (dz = -radius; dz <= radius; dz++) {
            d = (double)(dx * dx + dz * dz) / (double)(radius * radius);
            if (d > 1.0) { continue; }
            for (dy = -1; dy <= 1; dy++) {
                x = cx + dx;
                y = cy + dy;
                z = cz + dz;
                if (!IsInsideWorld(x, y, z)) { continue; }
                if (world[x][y][z] == BLOCK_SAND || world[x][y][z] == BLOCK_DIRT || world[x][y][z] == BLOCK_GRAVEL) { world[x][y][z] = BLOCK_CLAY; }
            }
        }
    }
}

void WorldGenV3_AddReedCluster(int x, int y, int z, int salt)
{
    int i;
    int h;
    h = 2 + (WorldHash3D(x, y, z, g_worldSeed + salt) % 3);
    if (!WorldGenV3_CanPlacePlantAt(x, y, z, BLOCK_REED)) { return; }
    for (i = 0; i < h; i++) {
        if (!IsInsideWorld(x, y + i, z)) { return; }
        if (GetBlock(x, y + i, z) != BLOCK_AIR) { return; }
    }
    for (i = 0; i < h; i++) { WorldGenV3_SetBlockGen(x, y + i, z, BLOCK_REED); }
}

void WorldGenV3_AddPumpkinPatch(int x, int y, int z, int salt)
{
    int i;
    int px;
    int pz;
    int py;
    for (i = 0; i < 12; i++) {
        px = x + (WorldHash3D(x, i, z, g_worldSeed + salt) % 9) - 4;
        pz = z + (WorldHash3D(x, i, z, g_worldSeed + salt + 1) % 9) - 4;
        py = WorldGenV3_FindTopAirY(px, pz);
        if (!IsInsideWorld(px, py, pz)) { continue; }
        if (GetBlock(px, py, pz) == BLOCK_AIR && GetBlock(px, py - 1, pz) == BLOCK_GRASS) { WorldGenV3_SetBlockGen(px, py, pz, BLOCK_PUMPKIN); }
    }
}

void WorldGenV3_AddGrassFlowerPatch(int x, int y, int z, int block, int tries, int salt)
{
    int i;
    int px;
    int pz;
    int py;
    for (i = 0; i < tries; i++) {
        px = x + (WorldHash3D(x, i, z, g_worldSeed + salt) % 15) - 7;
        pz = z + (WorldHash3D(x, i, z, g_worldSeed + salt + 1) % 15) - 7;
        py = WorldGenV3_FindTopAirY(px, pz);
        WorldGenV3_PlacePlantAt(px, py, pz, block);
    }
}

void WorldGenV3_AddDungeonRoom(int cx, int cy, int cz, int salt)
{
    int dx;
    int dy;
    int dz;
    int x;
    int y;
    int z;
    int wall;
    if (!IsInsideWorld(cx, cy, cz)) { return; }
    if (cy < 10 || cy > 54) { return; }
    for (dx = -4; dx <= 4; dx++) {
        for (dy = -2; dy <= 3; dy++) {
            for (dz = -4; dz <= 4; dz++) {
                x = cx + dx;
                y = cy + dy;
                z = cz + dz;
                if (!IsInsideWorld(x, y, z)) { continue; }
                wall = (dx == -4 || dx == 4 || dz == -4 || dz == 4 || dy == -2 || dy == 3);
                if (wall) {
                    if (world[x][y][z] != BLOCK_BORDER) {
                        if ((WorldHash3D(x, y, z, g_worldSeed + salt) % 4) == 0) { world[x][y][z] = BLOCK_MOSSY_COBBLESTONE; }
                        else { world[x][y][z] = BLOCK_COBBLESTONE; }
                    }
                } else { world[x][y][z] = BLOCK_AIR; }
            }
        }
    }
    WorldGenV3_SetBlockGen(cx, cy - 1, cz, BLOCK_MOB_SPAWNER);
    if (IsInsideWorld(cx + 2, cy - 1, cz)) { WorldGenV3_SetBlockGen(cx + 2, cy - 1, cz, BLOCK_CHEST); }
    if (IsInsideWorld(cx - 2, cy - 1, cz)) { WorldGenV3_SetBlockGen(cx - 2, cy - 1, cz, BLOCK_CHEST); }
}

void WorldGenV3_AddCaveNoiseCarving(void)
{
    int x;
    int y;
    int z;
    int gx;
    int gz;
    int surface;
    double cave;
    double detail;
    for (x = 1; x < WORLD_X - 1; x++) {
        gx = LocalToGlobalBlockX(x);
        for (z = 1; z < WORLD_Z - 1; z++) {
            gz = LocalToGlobalBlockZ(z);
            surface = BetaTerrainHeight(gx, gz);
            for (y = 6; y < surface - 5 && y < WORLD_Y - 7; y++) {
                cave = WorldFractal3D((double)gx * 0.034, (double)y * 0.050, (double)gz * 0.034, g_worldSeed + 17001, 3, 0.55);
                detail = WorldFractal3D((double)gx * 0.085, (double)y * 0.080, (double)gz * 0.085, g_worldSeed + 17002, 2, 0.50);
                if (cave > 0.47 && detail > -0.15 && world[x][y][z] != BLOCK_BORDER) { world[x][y][z] = BLOCK_AIR; }
            }
        }
    }
}

void WorldGenV3_AddCaveLavaFloors(void)
{
    int x;
    int y;
    int z;
    for (x = 1; x < WORLD_X - 1; x++) {
        for (z = 1; z < WORLD_Z - 1; z++) {
            for (y = 3; y < 11; y++) {
                if (world[x][y][z] == BLOCK_AIR && WorldGenV3_BlockIsStoneLike(world[x][y - 1][z])) {
                    if ((WorldHash3D(LocalToGlobalBlockX(x), y, LocalToGlobalBlockZ(z), g_worldSeed + 18000) % 100) < 9) { world[x][y][z] = BLOCK_LAVA; }
                }
            }
        }
    }
}

void WorldGenV3_AddSpringPockets(void)
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
    int block;
    minChunkX = FloorDivInt(worldOriginBlockX, CHUNK_SIZE);
    maxChunkX = FloorDivInt(worldOriginBlockX + WORLD_X - 1, CHUNK_SIZE);
    minChunkZ = FloorDivInt(worldOriginBlockZ, CHUNK_SIZE);
    maxChunkZ = FloorDivInt(worldOriginBlockZ + WORLD_Z - 1, CHUNK_SIZE);
    for (chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
        for (chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
            for (i = 0; i < 12; i++) {
                gx = chunkX * CHUNK_SIZE + (WorldHash3D(chunkX, i, chunkZ, g_worldSeed + 18501) & 15);
                gz = chunkZ * CHUNK_SIZE + (WorldHash3D(chunkX, i, chunkZ, g_worldSeed + 18502) & 15);
                gy = 8 + (WorldHash3D(chunkX, i, chunkZ, g_worldSeed + 18503) % 54);
                lx = GlobalToLocalBlockX(gx);
                lz = GlobalToLocalBlockZ(gz);
                if (!IsInsideWorld(lx, gy, lz)) { continue; }
                block = BLOCK_WATER;
                if (i < 4 && gy < 24) { block = BLOCK_LAVA; }
                if (world[lx][gy][lz] == BLOCK_AIR && WorldGenV3_BlockIsStoneLike(world[lx][gy - 1][lz])) { world[lx][gy][lz] = block; }
            }
        }
    }
}

int WorldGenV3_TreeCountForBiome(int biome, int gx, int gz)
{
    double n;
    int base;
    base = 0;
    if (biome == BIOME_FOREST) { base = 6; }
    else if (biome == BIOME_RAINFOREST) { base = 9; }
    else if (biome == BIOME_SEASONAL_FOREST) { base = 4; }
    else if (biome == BIOME_TAIGA) { base = 6; }
    else if (biome == BIOME_SWAMPLAND) { base = 2; }
    else if (biome == BIOME_PLAINS) { base = -18; }
    else if (biome == BIOME_DESERT || biome == BIOME_TUNDRA || biome == BIOME_OCEAN) { base = -20; }
    n = WorldFractal2D((double)gx * 0.030, (double)gz * 0.030, g_worldSeed + 19000, 2, 0.50);
    if (n > 0.40) { base += 2; }
    if (base < 0) { base = 0; }
    return base;
}
/* WORLDGEN_V3_CONVERSION_PASS_END */


/* WORLDGEN_V20_TERRAIN_BIOME_DECORATION_PASS_BEGIN
   This pass is intentionally real runtime code, not scaffolding.  It keeps the
   old single-file C/OpenGL/Open Watcom target while moving terrain closer to
   the responsibilities of ChunkProviderGenerate, NoiseGeneratorOctaves,
   WorldChunkManager, WorldGenMinable, WorldGenLakes, WorldGenDungeons, and
   the smaller WorldGen* feature classes.  The pass works per 16x16 chunk so
   Win98-era machines avoid expensive per-block decoration scans. */

double WorldGenV20_OctaveBlend2D(int gx, int gz, double scaleA, double scaleB, int saltA, int saltB)
{
    double a;
    double b;
    double sel;

    a = WorldFractal2D((double)gx * scaleA, (double)gz * scaleA, g_worldSeed + saltA, 4, 0.55);
    b = WorldFractal2D((double)gx * scaleB, (double)gz * scaleB, g_worldSeed + saltB, 5, 0.57);
    sel = WorldFractal2D((double)gx * 0.0040, (double)gz * 0.0040, g_worldSeed + saltA + saltB + 73, 3, 0.52);
    if (sel < -0.15) { return a; }
    if (sel > 0.15) { return b; }
    return a * (0.5 - sel * 1.6666667) + b * (0.5 + sel * 1.6666667);
}

double WorldGenV20_OceanEdgeScore(int gx, int gz)
{
    double c;
    double n;
    double edge;

    c = WorldGenV3_UnitNoise2D(gx, gz, 0.00165, 0.00165, 8844, 4, 0.54);
    n = WorldGenV3_UnitNoise2D(gx + 19, gz - 11, 0.00215, 0.00215, 8846, 3, 0.52);
    edge = fabs(c - 0.185);
    edge = 1.0 - ClampDouble(edge / 0.075, 0.0, 1.0);
    edge = edge * 0.82 + n * 0.18;
    return ClampDouble(edge, 0.0, 1.0);
}

int WorldGenV20_ShouldUseBeachSurface(int lx, int lz, int biome, int y)
{
    int gx;
    int gz;
    double edge;

    gx = LocalToGlobalBlockX(lx);
    gz = LocalToGlobalBlockZ(lz);
    edge = WorldGenV20_OceanEdgeScore(gx, gz);

    if (biome == BIOME_OCEAN) { return 1; }
    if (y <= GEN_WATER_LEVEL + 1) { return 1; }
    if (edge > 0.68 && y <= GEN_WATER_LEVEL + 4 && y >= GEN_WATER_LEVEL - 3) { return 1; }
    return 0;
}

int WorldGenV20_DeterministicChunkRand(int chunkX, int chunkZ, int index, int salt)
{
    return WorldHash3D(chunkX, index, chunkZ, g_worldSeed + salt) & 0x7fffffff;
}

void WorldGenV20_AddTreeVariant(int x, int y, int z, int biome, int salt)
{
    int r;

    if (!IsInsideWorld(x, y, z)) { return; }
    if (GetBlock(x, y, z) != BLOCK_AIR) { return; }
    if (GetBlock(x, y - 1, z) != BLOCK_GRASS && GetBlock(x, y - 1, z) != BLOCK_DIRT) { return; }

    r = WorldHash3D(LocalToGlobalBlockX(x), y, LocalToGlobalBlockZ(z), g_worldSeed + salt) % 100;
    if (biome == BIOME_RAINFOREST) {
        if (r < 42) { AddBigBetaTree(x, y, z); }
        else { AddBetaTree(x, y, z); }
    } else if (biome == BIOME_TAIGA) {
        if (r < 28) { AddBigBetaTree(x, y, z); }
        else { AddBetaTree(x, y, z); }
    } else if (biome == BIOME_FOREST || biome == BIOME_SEASONAL_FOREST) {
        if (r < 18) { AddBigBetaTree(x, y, z); }
        else { AddBetaTree(x, y, z); }
    } else {
        AddBetaTree(x, y, z);
    }
}

void WorldGenV20_AddJavaOrePass(void)
{
    int chunkX;
    int chunkZ;
    int minChunkX;
    int maxChunkX;
    int minChunkZ;
    int maxChunkZ;
    int pass;
    int attempts;
    int i;
    int gx;
    int gy;
    int gz;
    int ore;
    int minY;
    int maxY;
    int size;
    int r;

    minChunkX = FloorDivInt(worldOriginBlockX, CHUNK_SIZE);
    maxChunkX = FloorDivInt(worldOriginBlockX + WORLD_X - 1, CHUNK_SIZE);
    minChunkZ = FloorDivInt(worldOriginBlockZ, CHUNK_SIZE);
    maxChunkZ = FloorDivInt(worldOriginBlockZ + WORLD_Z - 1, CHUNK_SIZE);

    for (chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
        for (chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
            for (pass = 0; pass < 6; pass++) {
                if (pass == 0) { ore = BLOCK_COAL_ORE; attempts = g_legacyPerformanceModeV13 ? 4 : 10; minY = 8; maxY = 92; size = 12; }
                else if (pass == 1) { ore = BLOCK_IRON_ORE; attempts = g_legacyPerformanceModeV13 ? 4 : 9; minY = 6; maxY = 64; size = 8; }
                else if (pass == 2) { ore = BLOCK_GOLD_ORE; attempts = 2; minY = 5; maxY = 32; size = 8; }
                else if (pass == 3) { ore = BLOCK_REDSTONE_ORE; attempts = g_legacyPerformanceModeV13 ? 2 : 5; minY = 4; maxY = 18; size = 7; }
                else if (pass == 4) { ore = BLOCK_DIAMOND_ORE; attempts = 1; minY = 4; maxY = 16; size = 7; }
                else { ore = BLOCK_LAPIS_ORE; attempts = 1; minY = 8; maxY = 24; size = 6; }

                for (i = 0; i < attempts; i++) {
                    r = WorldGenV20_DeterministicChunkRand(chunkX, chunkZ, i + pass * 31, 20100);
                    gx = chunkX * CHUNK_SIZE + (r & 15);
                    gz = chunkZ * CHUNK_SIZE + ((r >> 8) & 15);
                    gy = minY + ((r >> 16) % (maxY - minY + 1));
                    WorldGenV3_AddOreVeinJava(gx, gy, gz, ore, size, 20200 + pass * 97);
                }
            }
        }
    }
}

void WorldGenV20_AddLakesAndDungeonsPass(void)
{
    int chunkX;
    int chunkZ;
    int minChunkX;
    int maxChunkX;
    int minChunkZ;
    int maxChunkZ;
    int r;
    int gx;
    int gz;
    int lx;
    int lz;
    int gy;
    int surface;
    int liquid;

    minChunkX = FloorDivInt(worldOriginBlockX, CHUNK_SIZE);
    maxChunkX = FloorDivInt(worldOriginBlockX + WORLD_X - 1, CHUNK_SIZE);
    minChunkZ = FloorDivInt(worldOriginBlockZ, CHUNK_SIZE);
    maxChunkZ = FloorDivInt(worldOriginBlockZ + WORLD_Z - 1, CHUNK_SIZE);

    for (chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
        for (chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
            r = WorldGenV20_DeterministicChunkRand(chunkX, chunkZ, 0, 21000);
            gx = chunkX * CHUNK_SIZE + 4 + (r & 7);
            gz = chunkZ * CHUNK_SIZE + 4 + ((r >> 8) & 7);
            lx = GlobalToLocalBlockX(gx);
            lz = GlobalToLocalBlockZ(gz);
            if (lx <= 6 || lz <= 6 || lx >= WORLD_X - 7 || lz >= WORLD_Z - 7) { continue; }
            surface = WorldGenV3_FindTopGroundY(lx, lz);
            if (surface < 8) { surface = BetaTerrainHeight(gx, gz); }

            if ((r % (g_legacyPerformanceModeV13 ? 58 : 28)) == 0 && surface > GEN_WATER_LEVEL + 2) {
                liquid = BLOCK_WATER;
                gy = surface - 2 - ((r >> 17) & 3);
                WorldGenV3_CarveLakeLocal(lx, gy, lz, liquid, 21100);
            }

            if ((r % (g_legacyPerformanceModeV13 ? 150 : 84)) == 5) {
                gy = 8 + ((r >> 12) % 20);
                WorldGenV3_CarveLakeLocal(lx, gy, lz, BLOCK_LAVA, 21200);
            }

            if ((r % (g_legacyPerformanceModeV13 ? 62 : 28)) == 9) {
                /* V28: place dungeon rooms a little underground from the local
                   surface, instead of at a random absolute Y that can cut into
                   mountaintops or appear too deep. */
                gy = surface - 8 - ((r >> 13) % 13);
                if (gy < 12) { gy = 12 + ((r >> 9) & 7); }
                if (gy > 46) { gy = 46 - ((r >> 5) & 7); }
                if (gy < surface - 4) { WorldGenV3_AddDungeonRoom(lx, gy, lz, 21300); }
            }
        }
    }
}

void WorldGenV20_AddDecorationPass(void)
{
    int chunkX;
    int chunkZ;
    int minChunkX;
    int maxChunkX;
    int minChunkZ;
    int maxChunkZ;
    int tries;
    int i;
    int r;
    int gx;
    int gz;
    int lx;
    int lz;
    int y;
    int biome;
    int chance;
    int plant;
    int treeCount;

    minChunkX = FloorDivInt(worldOriginBlockX, CHUNK_SIZE);
    maxChunkX = FloorDivInt(worldOriginBlockX + WORLD_X - 1, CHUNK_SIZE);
    minChunkZ = FloorDivInt(worldOriginBlockZ, CHUNK_SIZE);
    maxChunkZ = FloorDivInt(worldOriginBlockZ + WORLD_Z - 1, CHUNK_SIZE);
    tries = g_legacyPerformanceModeV13 ? 5 : 11;

    for (chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
        for (chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
            r = WorldGenV20_DeterministicChunkRand(chunkX, chunkZ, 0, 22000);
            biome = GetBetaBiomeAt(chunkX * CHUNK_SIZE + 8, chunkZ * CHUNK_SIZE + 8);
            treeCount = WorldGenV3_TreeCountForBiome(biome, chunkX * CHUNK_SIZE + 8, chunkZ * CHUNK_SIZE + 8);
            if (g_legacyPerformanceModeV13 && treeCount > 3) { treeCount = 3; }
            if (!g_legacyPerformanceModeV13 && treeCount > 7) { treeCount = 7; }

            for (i = 0; i < treeCount; i++) {
                r = WorldGenV20_DeterministicChunkRand(chunkX, chunkZ, i, 22100);
                gx = chunkX * CHUNK_SIZE + (r & 15);
                gz = chunkZ * CHUNK_SIZE + ((r >> 8) & 15);
                lx = GlobalToLocalBlockX(gx);
                lz = GlobalToLocalBlockZ(gz);
                if (lx < 4 || lz < 4 || lx >= WORLD_X - 4 || lz >= WORLD_Z - 4) { continue; }
                y = WorldGenV3_FindTopAirY(lx, lz);
                WorldGenV20_AddTreeVariant(lx, y, lz, biome, 22200 + i);
            }

            for (i = 0; i < tries; i++) {
                r = WorldGenV20_DeterministicChunkRand(chunkX, chunkZ, i, 22300);
                gx = chunkX * CHUNK_SIZE + (r & 15);
                gz = chunkZ * CHUNK_SIZE + ((r >> 8) & 15);
                lx = GlobalToLocalBlockX(gx);
                lz = GlobalToLocalBlockZ(gz);
                if (lx < 2 || lz < 2 || lx >= WORLD_X - 2 || lz >= WORLD_Z - 2) { continue; }
                y = WorldGenV3_FindTopAirY(lx, lz);
                biome = GetLocalBiome(lx, lz);

                if ((biome == BIOME_OCEAN || y <= GEN_WATER_LEVEL + 2) && WorldGenV3_HasAdjacentWater(lx, y - 1, lz)) {
                    if ((r % 3) == 0) { WorldGenV3_AddClayDisk(lx, y - 1, lz, 22400 + i); }
                    if ((r % 2) == 0) { WorldGenV3_AddReedCluster(lx, y, lz, 22500 + i); }
                    continue;
                }

                if (biome == BIOME_DESERT) {
                    if ((r % 9) == 0) { AddBetaCactus(lx, y, lz, 1 + ((r >> 18) % 3)); }
                    else if ((r % 11) == 0) { WorldGenV3_PlacePlantAt(lx, y, lz, BLOCK_DEAD_BUSH); }
                    continue;
                }

                if (GetBlock(lx, y - 1, lz) != BLOCK_GRASS) { continue; }
                chance = r % 100;
                plant = BLOCK_TALL_GRASS;
                if (chance < 8) { plant = BLOCK_FLOWER_YELLOW; }
                else if (chance < 13) { plant = BLOCK_FLOWER_RED; }
                else if (chance < 17 && WorldGenV3_IsWetBiome(biome)) { plant = BLOCK_MUSHROOM_BROWN; }
                else if (chance < 20 && biome == BIOME_TAIGA) { plant = BLOCK_MUSHROOM_RED; }

                if (chance < 30 || biome == BIOME_PLAINS || biome == BIOME_FOREST || biome == BIOME_RAINFOREST) {
                    WorldGenV3_AddGrassFlowerPatch(lx, y, lz, plant, g_legacyPerformanceModeV13 ? 3 : 7, 22600 + i);
                }
                if ((r % (g_legacyPerformanceModeV13 ? 520 : 260)) == 17) { WorldGenV3_AddPumpkinPatch(lx, y, lz, 22700); }
            }
        }
    }
}

void WorldGenV20_AddCaveAndSpringFinishing(void)
{
    if (!g_legacyPerformanceModeV13) { WorldGenV3_AddCaveNoiseCarving(); }
    WorldGenV3_AddCaveLavaFloors();
    if (!g_legacyPerformanceModeV13) { WorldGenV3_AddSpringPockets(); }
}

void WorldGenV20_EnsureGeneratedTileEntities(void)
{
    int x;
    int y;
    int z;
    int block;

    for (x = 0; x < WORLD_X; x++) {
        for (z = 0; z < WORLD_Z; z++) {
            for (y = 1; y < WORLD_Y - 1; y++) {
                block = world[x][y][z];
                if (block == BLOCK_CHEST || block == BLOCK_MOB_SPAWNER || block == BLOCK_FURNACE || block == BLOCK_FURNACE_LIT) {
                    EnsureTileEntityForBlock(block, x, y, z);
                }
            }
        }
    }
}
/* WORLDGEN_V20_TERRAIN_BIOME_DECORATION_PASS_END */



/* ------------------------------------------------------------ */
/* Beta-style climate biome helpers and cheap view culling       */
/* ------------------------------------------------------------ */



int GetBetaBiomeAt(int gx, int gz)
{
    WorldGenV3Climate c;
    int biome;
    double edge;

    /* V20 routes the normal biome lookup through the climate helper so
       temperature/humidity/ocean fields are shared by terrain, decorations,
       weather, and mob spawning instead of each system inventing its own map. */
    WorldGenV3_ComputeClimate(gx, gz, &c);
    biome = WorldGenV3_BiomeFromClimate(&c);
    edge = WorldGenV20_OceanEdgeScore(gx, gz);

    /* Near ocean borders, avoid sharp desert/forest cliffs.  There is no
       separate beach biome ID in this C port, so sand beaches are applied in
       BiomeTopBlock/AddSurfaceTexturePass while this returns a mild biome. */
    if (biome != BIOME_OCEAN && edge > 0.78 && c.temperature > 0.20) {
        if (c.humidity < 0.36) { return BIOME_SHRUBLAND; }
        return BIOME_PLAINS;
    }

    return biome;
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
    if (biome == BIOME_DESERT) { return BLOCK_SAND; }
    if (biome == BIOME_OCEAN) { return BLOCK_SAND; }
    if (biome == BIOME_SWAMPLAND && y <= GEN_WATER_LEVEL + 1) { return BLOCK_DIRT; }
    return BLOCK_GRASS;
}




int BiomeFillerBlock(int biome)
{
    if (biome == BIOME_DESERT || biome == BIOME_OCEAN) { return BLOCK_SAND; }
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
    if (d < 18) { d = 18; }
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
    if (d > 34) { d = 34; }
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
    d = GetTerrainRenderDistanceBlocks();
    if (d < 32) { d = 32; }
    return d;
}

const char *RenderDistanceLabel(void)
{
    static char label[32];

    if (g_renderDistanceChunks <= 1) { return "Tiny 1 chunk"; }
    if (g_renderDistanceChunks == 2) { return "Short 2 chunks"; }
    if (g_renderDistanceChunks == 4) { return "Normal 4 chunks"; }
    if (g_renderDistanceChunks >= RENDER_DISTANCE_MAX_CHUNKS) { return "Far 6 chunks"; }

    sprintf(label, "%d chunks", g_renderDistanceChunks);
    return label;
}


void SetRenderDistanceExactV13B(int value)
{
    if (value < RENDER_DISTANCE_MIN_CHUNKS) {
        value = RENDER_DISTANCE_MIN_CHUNKS;
    }
    if (value > RENDER_DISTANCE_MAX_CHUNKS) {
        value = RENDER_DISTANCE_MAX_CHUNKS;
    }

    g_renderDistanceChunks = value;
    g_renderDistancePinnedV13B = 1;
    g_autoPerfMayLowerRenderDistanceV13B = 0;
}

void SaveUserOptionsV13B(void)
{
    FILE *fp;

    fp = fopen("options_clonemc.cfg", "w");
    if (fp == NULL) {
        return;
    }

    fprintf(fp, "render_distance_chunks=%d\n", g_renderDistanceChunks);
    fprintf(fp, "legacy_performance=%d\n", g_legacyPerformanceModeV13);
    fprintf(fp, "fast_worldgen=%d\n", g_fastWorldGenV13);
    fprintf(fp, "skip_weather=%d\n", g_skipWeatherRenderV13);
    fprintf(fp, "chunk_mesh_budget=%d\n", g_chunkMeshBuildBudget);
    fclose(fp);
}

void LoadUserOptionsV13B(void)
{
    FILE *fp;
    char line[128];
    int value;

    fp = fopen("options_clonemc.cfg", "r");
    if (fp == NULL) {
        SetRenderDistanceExactV13B(g_renderDistanceChunks);
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "render_distance_chunks=%d", &value) == 1) {
            SetRenderDistanceExactV13B(value);
        } else if (sscanf(line, "legacy_performance=%d", &value) == 1) {
            g_legacyPerformanceModeV13 = value ? 1 : 0;
        } else if (sscanf(line, "fast_worldgen=%d", &value) == 1) {
            g_fastWorldGenV13 = value ? 1 : 0;
        } else if (sscanf(line, "skip_weather=%d", &value) == 1) {
            g_skipWeatherRenderV13 = value ? 1 : 0;
        } else if (sscanf(line, "chunk_mesh_budget=%d", &value) == 1) {
            if (value < 1) { value = 1; }
            if (value > 4) { value = 4; }
            g_chunkMeshBuildBudget = value;
        }
    }

    fclose(fp);
}

void ChangeRenderDistance(int delta)
{
    int value;
    value = g_renderDistanceChunks + delta;
    if (value > RENDER_DISTANCE_MAX_CHUNKS) {
        value = RENDER_DISTANCE_MIN_CHUNKS;
    }
    if (value < RENDER_DISTANCE_MIN_CHUNKS) {
        value = RENDER_DISTANCE_MAX_CHUNKS;
    }
    SetRenderDistanceExactV13B(value);
    SaveUserOptionsV13B();
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

    SetRenderDistanceExactV13B(value);
    SaveUserOptionsV13B();
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



void AddFastCaveWormsV13(void)
{
    int chunkX;
    int chunkZ;
    int minChunkX;
    int maxChunkX;
    int minChunkZ;
    int maxChunkZ;
    int chance;
    int step;
    int steps;
    int h;
    int gx;
    int gz;
    int lx;
    int lz;
    int cy;
    int radius;
    double px;
    double py;
    double pz;
    double yawAngle;
    double pitchAngle;

    minChunkX = FloorDivInt(worldOriginBlockX, CHUNK_SIZE);
    maxChunkX = FloorDivInt(worldOriginBlockX + WORLD_X - 1, CHUNK_SIZE);
    minChunkZ = FloorDivInt(worldOriginBlockZ, CHUNK_SIZE);
    maxChunkZ = FloorDivInt(worldOriginBlockZ + WORLD_Z - 1, CHUNK_SIZE);

    for (chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
        for (chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
            chance = WorldHash2D(chunkX, chunkZ, g_worldSeed + 13013) % 100;
            if (chance > 34) { continue; }

            gx = chunkX * CHUNK_SIZE + 3 + (WorldHash3D(chunkX, 1, chunkZ, g_worldSeed + 13014) & 9);
            gz = chunkZ * CHUNK_SIZE + 3 + (WorldHash3D(chunkX, 2, chunkZ, g_worldSeed + 13015) & 9);
            h = BetaTerrainHeight(gx, gz);
            cy = 8 + (WorldHash3D(chunkX, 3, chunkZ, g_worldSeed + 13016) % 34);
            if (cy > h - 7) { cy = h - 7; }
            if (cy < 6) { cy = 6; }

            px = (double)gx;
            py = (double)cy;
            pz = (double)gz;
            yawAngle = (double)(WorldHash3D(chunkX, 4, chunkZ, g_worldSeed + 13017) % 6283) / 1000.0;
            pitchAngle = ((double)(WorldHash3D(chunkX, 5, chunkZ, g_worldSeed + 13018) % 900) / 1000.0) - 0.45;
            steps = 16 + (WorldHash3D(chunkX, 6, chunkZ, g_worldSeed + 13019) & 15);

            for (step = 0; step < steps; step++) {
                lx = GlobalToLocalBlockX((int)floor(px));
                lz = GlobalToLocalBlockZ((int)floor(pz));
                cy = (int)floor(py);
                if (lx >= 2 && lx < WORLD_X - 2 && lz >= 2 && lz < WORLD_Z - 2 && cy > 4 && cy < WORLD_Y - 4) {
                    radius = 1;
                    if ((step & 7) == 0) { radius = 2; }
                    CarveSphere(lx, cy, lz, radius);
                }

                yawAngle += ((double)((WorldHash3D(chunkX, step, chunkZ, g_worldSeed + 13020) % 200) - 100)) / 900.0;
                pitchAngle += ((double)((WorldHash3D(chunkZ, step, chunkX, g_worldSeed + 13021) % 100) - 50)) / 1600.0;
                pitchAngle = ClampDouble(pitchAngle, -0.35, 0.35);
                px += cos(yawAngle) * cos(pitchAngle) * 1.35;
                pz += sin(yawAngle) * cos(pitchAngle) * 1.35;
                py += sin(pitchAngle) * 0.75;
                if (py < 6.0) { py = 6.0; pitchAngle = fabs(pitchAngle); }
                if (py > (double)(WORLD_Y - 8)) { py = (double)(WORLD_Y - 8); pitchAngle = -fabs(pitchAngle); }
            }
        }
    }
}


void AddRandomWalkerCaves(void)
{
    /* V13: default to a light cave pass for Windows 98-class machines.
       The full worm/shaft pass is still present, but it is too expensive to
       run during every streamed 128x128 window rebuild on weak hardware. */
    if (g_legacyPerformanceModeV13) {
        AddFastCaveWormsV13();
        return;
    }
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

                        if (WorldGenV20_ShouldUseBeachSurface(x, z, biome, y)) {
                            world[x][y][z] = BLOCK_SAND;
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







/* ------------------------------------------------------------ */
/* V55_PRIORITY7_TERRAIN_PARITY                                 */
/* Java ChunkProviderGenerate/BiomeGen-style terrain helpers     */
/* ------------------------------------------------------------ */

double WorldGenV55_Ridged01(double n)
{
    n = 1.0 - fabs(n);
    if (n < 0.0) { n = 0.0; }
    if (n > 1.0) { n = 1.0; }
    return n * n;
}

double WorldGenV55_MountainRangeMask(int gx, int gz)
{
    double range;
    double ridge;
    double detail;
    double mask;

    /* Broad continental field chooses where mountain ranges exist. */
    range = WorldFractal2D((double)gx * 0.0026, (double)gz * 0.0026, g_worldSeed + 55100, 5, 0.56);

    /* Thin ridges make long high spines instead of round hills. */
    ridge = WorldGenV55_Ridged01(WorldFractal2D((double)gx * 0.0095, (double)gz * 0.0095, g_worldSeed + 55101, 5, 0.54));
    detail = WorldGenV55_Ridged01(WorldFractal2D((double)gx * 0.0240, (double)gz * 0.0240, g_worldSeed + 55102, 4, 0.52));

    mask = (range + 0.18) * 0.80 + ridge * 0.92 + detail * 0.25;
    mask = (mask - 0.46) / 0.74;
    mask = ClampDouble(mask, 0.0, 1.0);
    mask = mask * mask * (3.0 - 2.0 * mask);
    return mask;
}

double WorldGenV55_MountainPeakBoost(int gx, int gz)
{
    double spine;
    double spike;
    double broken;

    spine = WorldGenV55_Ridged01(WorldFractal2D((double)gx * 0.0135, (double)gz * 0.0135, g_worldSeed + 55120, 5, 0.55));
    spike = WorldGenV55_Ridged01(WorldFractal2D((double)gx * 0.0310, (double)gz * 0.0310, g_worldSeed + 55121, 4, 0.52));
    broken = WorldFractal2D((double)gx * 0.0550, (double)gz * 0.0550, g_worldSeed + 55122, 3, 0.50);
    return ClampDouble(spine * 0.75 + spike * 0.45 + broken * 0.10, 0.0, 1.20);
}

int WorldGenV55_SurfaceSlopeLocal(int lx, int lz)
{
    int h;
    int h1;
    int h2;
    int h3;
    int h4;
    int maxd;
    if (lx < 1 || lz < 1 || lx >= WORLD_X - 1 || lz >= WORLD_Z - 1) { return 0; }
    h = worldHeightMap[lx][lz];
    h1 = worldHeightMap[lx - 1][lz];
    h2 = worldHeightMap[lx + 1][lz];
    h3 = worldHeightMap[lx][lz - 1];
    h4 = worldHeightMap[lx][lz + 1];
    maxd = abs(h - h1);
    if (abs(h - h2) > maxd) { maxd = abs(h - h2); }
    if (abs(h - h3) > maxd) { maxd = abs(h - h3); }
    if (abs(h - h4) > maxd) { maxd = abs(h - h4); }
    return maxd;
}

int WorldGenV55_FindTopSolidYLocal(int lx, int lz)
{
    int y;
    for (y = WORLD_Y - 2; y >= 1; y--) {
        if (world[lx][y][lz] != BLOCK_AIR && world[lx][y][lz] != BLOCK_WATER && world[lx][y][lz] != BLOCK_LAVA && world[lx][y][lz] != BLOCK_FIRE) {
            return y;
        }
    }
    return 0;
}

void WorldGenV55_AddSurfaceVarietyPass(void)
{
    int x;
    int z;
    int y;
    int gy;
    int gx;
    int gz;
    int biome;
    int top;
    int slope;
    int hash;
    double edge;
    double range;

    for (x = 1; x < WORLD_X - 1; x++) {
        gx = LocalToGlobalBlockX(x);
        for (z = 1; z < WORLD_Z - 1; z++) {
            gz = LocalToGlobalBlockZ(z);
            biome = GetLocalBiome(x, z);
            y = WorldGenV55_FindTopSolidYLocal(x, z);
            if (y <= 1 || y >= WORLD_Y - 2) { continue; }
            top = world[x][y][z];
            slope = WorldGenV55_SurfaceSlopeLocal(x, z);
            hash = WorldHash2D(gx, gz, g_worldSeed + 55200);
            edge = WorldGenV20_OceanEdgeScore(gx, gz);
            range = WorldGenV55_MountainRangeMask(gx, gz);

            /* Freeze cold water at the surface, then sprinkle snow layers over
               cold/high land.  This is cheap but gives tundra/taiga/mountain tops
               the visual variety expected from the Java BiomeGen passes. */
            if ((biome == BIOME_TUNDRA || biome == BIOME_TAIGA || y > 78) && world[x][GEN_WATER_LEVEL][z] == BLOCK_WATER && y <= GEN_WATER_LEVEL) {
                world[x][GEN_WATER_LEVEL][z] = BLOCK_ICE;
            }
            if ((biome == BIOME_TUNDRA || biome == BIOME_TAIGA || y > 82) && world[x][y + 1][z] == BLOCK_AIR && top != BLOCK_WATER && top != BLOCK_LAVA) {
                if ((hash & 3) != 0) { world[x][y + 1][z] = BLOCK_SNOW; }
            }

            /* Beaches and river/ocean edges: mix sand, gravel and clay instead of
               one flat material everywhere. */
            if (y <= GEN_WATER_LEVEL + 3 || edge > 0.70) {
                if ((hash % 13) == 0) { world[x][y][z] = BLOCK_GRAVEL; }
                else if ((hash % 17) == 0) { world[x][y][z] = BLOCK_CLAY; }
                else if (top == BLOCK_GRASS || top == BLOCK_DIRT || top == BLOCK_STONE) { world[x][y][z] = BLOCK_SAND; }
                if (y > 2 && world[x][y - 1][z] == BLOCK_STONE) { world[x][y - 1][z] = BLOCK_SAND; }
                continue;
            }

            /* High steep mountain faces expose stone/gravel like old terrain.
               High flatter cold parts get snow caps. */
            if (range > 0.45 && y > 62) {
                if (slope >= 7 || y > 92) {
                    if ((hash & 7) == 0) { world[x][y][z] = BLOCK_GRAVEL; }
                    else { world[x][y][z] = BLOCK_STONE; }
                } else if (y > 78 && top == BLOCK_GRASS && ((hash >> 4) & 3) == 0) {
                    world[x][y][z] = BLOCK_DIRT;
                }
            }

            /* Desert underlayers become sandstone more reliably. */
            if (biome == BIOME_DESERT && top == BLOCK_SAND) {
                for (gy = y - 3; gy >= y - 6 && gy > 1; gy--) {
                    if (world[x][gy][z] == BLOCK_STONE || world[x][gy][z] == BLOCK_DIRT) { world[x][gy][z] = BLOCK_SANDSTONE; }
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
    double rangeMask;
    double peakBoost;
    double ridge;
    double ridgeFine;
    double detail;
    double valley;
    double continental;
    double beachEdge;
    double heightValue;
    double climateScale;
    double oceanPull;
    int biome;
    int h;

    biome = GetBetaBiomeAt(x, z);

    /* V58_PRIORITY3_TERRAIN_PARITY:
       This replaces the old small height tweak with a full Java-inspired
       ChunkProviderGenerate/NoiseGeneratorOctaves style height stack.  It keeps
       deterministic global coordinates so infinite chunks line up after the
       128x128 RAM window streams. */
    low = WorldGenV20_OctaveBlend2D(x, z, 0.0105, 0.0065, 58100, 58200);
    high = WorldGenV20_OctaveBlend2D(x, z, 0.0180, 0.0110, 58300, 58400);
    selector = WorldFractal2D((double)x * 0.0034, (double)z * 0.0034, g_worldSeed + 58500, 5, 0.54);

    rangeMask = WorldGenV55_MountainRangeMask(x, z);
    peakBoost = WorldGenV55_MountainPeakBoost(x, z);
    ridge = WorldGenV55_Ridged01(WorldFractal2D((double)x * 0.0110, (double)z * 0.0110, g_worldSeed + 58600, 5, 0.56));
    ridgeFine = WorldGenV55_Ridged01(WorldFractal2D((double)x * 0.0300, (double)z * 0.0300, g_worldSeed + 58601, 4, 0.53));
    detail = WorldFractal2D((double)x * 0.0720, (double)z * 0.0720, g_worldSeed + 58602, 3, 0.50);
    valley = WorldFractal2D((double)x * 0.0065, (double)z * 0.0065, g_worldSeed + 58603, 4, 0.55);
    continental = WorldFractal2D((double)x * 0.00135, (double)z * 0.00135, g_worldSeed + 58604, 5, 0.57);
    beachEdge = WorldGenV20_OceanEdgeScore(x, z);

    heightValue = selector < -0.12 ? low : high;

    climateScale = 1.0;
    if (biome == BIOME_PLAINS) { climateScale = 0.46; }
    else if (biome == BIOME_SHRUBLAND) { climateScale = 0.58; }
    else if (biome == BIOME_DESERT) { climateScale = 0.62; }
    else if (biome == BIOME_SWAMPLAND) { climateScale = 0.18; }
    else if (biome == BIOME_TUNDRA) { climateScale = 0.48; }
    else if (biome == BIOME_TAIGA) { climateScale = 0.82; }
    else if (biome == BIOME_FOREST) { climateScale = 0.90; }
    else if (biome == BIOME_SEASONAL_FOREST) { climateScale = 1.05; }
    else if (biome == BIOME_RAINFOREST) { climateScale = 1.10; }
    else if (biome == BIOME_OCEAN) { climateScale = 0.16; }

    heightValue = heightValue * 19.0 * climateScale;

    if (biome != BIOME_OCEAN && biome != BIOME_SWAMPLAND) {
        /* broad mountains + long ridges + sharp Beta-like peaks */
        heightValue += rangeMask * 28.0;
        heightValue += rangeMask * ridge * 44.0;
        heightValue += rangeMask * ridgeFine * 18.0;
        heightValue += rangeMask * peakBoost * 42.0;
        if (rangeMask > 0.70) { heightValue += (rangeMask - 0.70) * 95.0; }
        heightValue += continental * 12.0;
    }

    if (valley < -0.25 && biome != BIOME_OCEAN) {
        heightValue -= (-valley - 0.25) * 22.0;
    }
    heightValue += detail * 6.0;

    h = GEN_BASE_HEIGHT + (int)heightValue;

    if (biome == BIOME_DESERT) { h = h - 1 + (int)(detail * 2.0); }
    if (biome == BIOME_SWAMPLAND) { h = GEN_WATER_LEVEL + 1 + (int)(detail * 2.0); }
    if (biome == BIOME_OCEAN) {
        oceanPull = WorldGenV3_UnitNoise2D(x, z, 0.0070, 0.0070, 58700, 3, 0.50);
        h = GEN_WATER_LEVEL - 12 + (int)(oceanPull * 7.0) + (int)(detail * 2.0);
    }

    /* Blend coastlines down instead of producing vertical terrain walls at
       ocean/land biome borders. */
    if (biome != BIOME_OCEAN && beachEdge > 0.68 && h > GEN_WATER_LEVEL + 4) {
        h = GEN_WATER_LEVEL + 2 + (int)(detail * 2.0) + (int)((1.0 - beachEdge) * 5.0);
    }

    if ((biome == BIOME_TUNDRA || biome == BIOME_TAIGA) && rangeMask < 0.42) { h -= 2; }
    if (rangeMask > 0.80 && biome != BIOME_OCEAN && biome != BIOME_SWAMPLAND) { h += (int)((rangeMask - 0.80) * 24.0); }

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
            h = worldHeightMap[x][z];

            if (g_legacyPerformanceModeV13 && ((x + z) & 1)) {
                continue;
            }

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

            if (g_legacyPerformanceModeV13) {
                if (forest > -0.12 && r < 7) {
                    AddBetaTree(x, h + 1, z);
                } else if (forest > 0.20 && r >= 7 && r < 9) {
                    AddBigBetaTree(x, h + 1, z);
                }
            } else {
                if (forest > -0.12 && r < 16) {
                    AddBetaTree(x, h + 1, z);
                } else if (forest > 0.12 && r >= 16 && r < 20) {
                    AddBigBetaTree(x, h + 1, z);
                }
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
/* V58_PRIORITY3_TERRAIN_WORLDGEN_PARITY                        */
/* ------------------------------------------------------------ */

int WorldGenV58_IsSolidTerrainBlock(int block)
{
    if (block == BLOCK_STONE || block == BLOCK_DIRT || block == BLOCK_GRASS ||
        block == BLOCK_SAND || block == BLOCK_SANDSTONE || block == BLOCK_GRAVEL ||
        block == BLOCK_CLAY || block == BLOCK_SNOW_BLOCK || block == BLOCK_COBBLESTONE) { return 1; }
    return 0;
}

int WorldGenV58_FindTopTerrainY(int lx, int lz)
{
    int y;
    int b;
    if (lx < 0 || lx >= WORLD_X || lz < 0 || lz >= WORLD_Z) { return 0; }
    for (y = WORLD_Y - 2; y >= 1; y--) {
        b = world[lx][y][lz];
        if (WorldGenV58_IsSolidTerrainBlock(b)) { return y; }
    }
    return 0;
}

int WorldGenV58_LocalSlope(int lx, int lz)
{
    int h;
    int d;
    int t;
    if (lx < 1 || lz < 1 || lx >= WORLD_X - 1 || lz >= WORLD_Z - 1) { return 0; }
    h = WorldGenV58_FindTopTerrainY(lx, lz);
    d = abs(h - WorldGenV58_FindTopTerrainY(lx - 1, lz));
    t = abs(h - WorldGenV58_FindTopTerrainY(lx + 1, lz)); if (t > d) { d = t; }
    t = abs(h - WorldGenV58_FindTopTerrainY(lx, lz - 1)); if (t > d) { d = t; }
    t = abs(h - WorldGenV58_FindTopTerrainY(lx, lz + 1)); if (t > d) { d = t; }
    return d;
}

void WorldGenV58_SetSurfaceColumn(int lx, int lz)
{
    int gx;
    int gz;
    int y;
    int d;
    int biome;
    int slope;
    int hash;
    int top;
    int filler;
    double edge;
    double range;
    int cold;
    int nearWater;

    if (lx < 1 || lz < 1 || lx >= WORLD_X - 1 || lz >= WORLD_Z - 1) { return; }
    gx = LocalToGlobalBlockX(lx);
    gz = LocalToGlobalBlockZ(lz);
    biome = GetBetaBiomeAt(gx, gz);
    biomeMap[lx][lz] = (unsigned char)biome;
    y = WorldGenV58_FindTopTerrainY(lx, lz);
    if (y <= 1 || y >= WORLD_Y - 2) { return; }

    slope = WorldGenV58_LocalSlope(lx, lz);
    hash = WorldHash2D(gx, gz, g_worldSeed + 58800);
    edge = WorldGenV20_OceanEdgeScore(gx, gz);
    range = WorldGenV55_MountainRangeMask(gx, gz);
    cold = (biome == BIOME_TUNDRA || biome == BIOME_TAIGA || y > 84) ? 1 : 0;
    nearWater = (y <= GEN_WATER_LEVEL + 3 || edge > 0.70) ? 1 : 0;

    top = BLOCK_GRASS;
    filler = BLOCK_DIRT;

    if (biome == BIOME_DESERT) { top = BLOCK_SAND; filler = BLOCK_SAND; }
    if (biome == BIOME_OCEAN) { top = BLOCK_SAND; filler = BLOCK_SAND; }
    if (biome == BIOME_SWAMPLAND && y <= GEN_WATER_LEVEL + 2) { top = BLOCK_DIRT; filler = BLOCK_DIRT; }
    if (nearWater && biome != BIOME_TUNDRA && biome != BIOME_TAIGA) { top = BLOCK_SAND; filler = BLOCK_SAND; }

    if (range > 0.50 && y > 66 && biome != BIOME_DESERT && biome != BIOME_OCEAN) {
        if (slope >= 6 || y > 92) {
            top = ((hash & 7) == 0) ? BLOCK_GRAVEL : BLOCK_STONE;
            filler = BLOCK_STONE;
        } else if (y > 82 && cold) {
            top = BLOCK_GRASS;
            filler = BLOCK_DIRT;
        }
    }

    if ((hash % 31) == 0 && nearWater) { top = BLOCK_GRAVEL; filler = BLOCK_GRAVEL; }
    if ((hash % 47) == 0 && nearWater && y <= GEN_WATER_LEVEL + 2) { top = BLOCK_CLAY; filler = BLOCK_CLAY; }

    if (WorldGenV58_IsSolidTerrainBlock(world[lx][y][lz])) { world[lx][y][lz] = top; }
    worldHeightMap[lx][lz] = y;

    for (d = 1; d <= 5 && y - d > 1; d++) {
        if (!WorldGenV58_IsSolidTerrainBlock(world[lx][y - d][lz])) { break; }
        if ((top == BLOCK_SAND || filler == BLOCK_SAND) && d >= 3) { world[lx][y - d][lz] = BLOCK_SANDSTONE; }
        else { world[lx][y - d][lz] = filler; }
    }

    if (cold && y + 1 < WORLD_Y - 1 && world[lx][y + 1][lz] == BLOCK_AIR && top != BLOCK_WATER && top != BLOCK_LAVA) {
        if ((hash & 3) != 0) { world[lx][y + 1][lz] = BLOCK_SNOW; }
    }
    if (cold && world[lx][GEN_WATER_LEVEL][lz] == BLOCK_WATER && y <= GEN_WATER_LEVEL) {
        world[lx][GEN_WATER_LEVEL][lz] = BLOCK_ICE;
    }
}

void WorldGenV58_ApplyTerrainSurfacePass(void)
{
    int x;
    int z;
    for (x = 1; x < WORLD_X - 1; x++) {
        for (z = 1; z < WORLD_Z - 1; z++) {
            WorldGenV58_SetSurfaceColumn(x, z);
        }
    }
}

void WorldGenV58_AddPumpkinPatch(int lx, int y, int lz, int salt)
{
    int dx;
    int dz;
    int r;
    for (dx = -2; dx <= 2; dx++) {
        for (dz = -2; dz <= 2; dz++) {
            r = WorldHash3D(LocalToGlobalBlockX(lx + dx), y, LocalToGlobalBlockZ(lz + dz), g_worldSeed + salt);
            if ((r & 7) == 0 && IsInsideWorld(lx + dx, y, lz + dz) && GetBlock(lx + dx, y, lz + dz) == BLOCK_AIR && GetBlock(lx + dx, y - 1, lz + dz) == BLOCK_GRASS) {
                world[lx + dx][y][lz + dz] = BLOCK_PUMPKIN;
            }
        }
    }
}

void WorldGenV58_AddBiomeDecorationPass(void)
{
    int chunkX;
    int chunkZ;
    int minChunkX;
    int maxChunkX;
    int minChunkZ;
    int maxChunkZ;
    int i;
    int r;
    int gx;
    int gz;
    int lx;
    int lz;
    int y;
    int biome;
    int treeCount;
    int plant;

    minChunkX = FloorDivInt(worldOriginBlockX, CHUNK_SIZE);
    maxChunkX = FloorDivInt(worldOriginBlockX + WORLD_X - 1, CHUNK_SIZE);
    minChunkZ = FloorDivInt(worldOriginBlockZ, CHUNK_SIZE);
    maxChunkZ = FloorDivInt(worldOriginBlockZ + WORLD_Z - 1, CHUNK_SIZE);

    for (chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
        for (chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
            biome = GetBetaBiomeAt(chunkX * CHUNK_SIZE + 8, chunkZ * CHUNK_SIZE + 8);
            r = WorldGenV20_DeterministicChunkRand(chunkX, chunkZ, 0, 58900);

            treeCount = WorldGenV3_TreeCountForBiome(biome, chunkX * CHUNK_SIZE + 8, chunkZ * CHUNK_SIZE + 8);
            if (biome == BIOME_FOREST || biome == BIOME_RAINFOREST) { treeCount += 1 + (r & 1); }
            if (biome == BIOME_PLAINS || biome == BIOME_SHRUBLAND) { treeCount = treeCount > 2 ? 2 : treeCount; }
            if (biome == BIOME_DESERT || biome == BIOME_OCEAN || biome == BIOME_SWAMPLAND) { treeCount = 0; }
            if (g_legacyPerformanceModeV13 && treeCount > 3) { treeCount = 3; }
            if (treeCount > 8) { treeCount = 8; }

            for (i = 0; i < treeCount; i++) {
                r = WorldGenV20_DeterministicChunkRand(chunkX, chunkZ, i + 11, 58910);
                gx = chunkX * CHUNK_SIZE + (r & 15);
                gz = chunkZ * CHUNK_SIZE + ((r >> 8) & 15);
                lx = GlobalToLocalBlockX(gx);
                lz = GlobalToLocalBlockZ(gz);
                if (lx < 5 || lz < 5 || lx >= WORLD_X - 5 || lz >= WORLD_Z - 5) { continue; }
                y = WorldGenV3_FindTopAirY(lx, lz);
                if (GetBlock(lx, y - 1, lz) == BLOCK_GRASS || GetBlock(lx, y - 1, lz) == BLOCK_DIRT) {
                    WorldGenV20_AddTreeVariant(lx, y, lz, biome, 58920 + i);
                }
            }

            for (i = 0; i < (g_legacyPerformanceModeV13 ? 6 : 14); i++) {
                r = WorldGenV20_DeterministicChunkRand(chunkX, chunkZ, i + 31, 58930);
                gx = chunkX * CHUNK_SIZE + (r & 15);
                gz = chunkZ * CHUNK_SIZE + ((r >> 8) & 15);
                lx = GlobalToLocalBlockX(gx);
                lz = GlobalToLocalBlockZ(gz);
                if (lx < 2 || lz < 2 || lx >= WORLD_X - 2 || lz >= WORLD_Z - 2) { continue; }
                y = WorldGenV3_FindTopAirY(lx, lz);
                biome = GetLocalBiome(lx, lz);

                if (biome == BIOME_DESERT) {
                    if ((r % 6) == 0) { AddBetaCactus(lx, y, lz, 1 + ((r >> 14) % 3)); }
                    else if ((r % 9) == 0) { WorldGenV3_PlacePlantAt(lx, y, lz, BLOCK_DEAD_BUSH); }
                    continue;
                }

                if (WorldGenV3_HasAdjacentWater(lx, y - 1, lz)) {
                    if ((r & 1) == 0) { WorldGenV3_AddReedCluster(lx, y, lz, 58940 + i); }
                    if ((r % 5) == 0) { WorldGenV3_AddClayDisk(lx, y - 1, lz, 58950 + i); }
                    continue;
                }

                if (GetBlock(lx, y - 1, lz) != BLOCK_GRASS) { continue; }
                plant = BLOCK_TALL_GRASS;
                if ((r % 23) == 0) { plant = BLOCK_FLOWER_RED; }
                else if ((r % 17) == 0) { plant = BLOCK_FLOWER_YELLOW; }
                else if ((r % 41) == 0) { plant = BLOCK_MUSHROOM_BROWN; }
                WorldGenV3_PlacePlantAt(lx, y, lz, plant);

                if ((r % 251) == 0 && biome != BIOME_TUNDRA && biome != BIOME_TAIGA) { WorldGenV58_AddPumpkinPatch(lx, y, lz, 58960 + i); }
            }
        }
    }
}

void WorldGenV58_ApplyFinalTerrainCleanupPass(void)
{
    int x;
    int z;
    int y;
    int b;
    for (x = 1; x < WORLD_X - 1; x++) {
        for (z = 1; z < WORLD_Z - 1; z++) {
            y = WorldGenV58_FindTopTerrainY(x, z);
            if (y <= 1) { continue; }
            b = world[x][y + 1][z];
            if (b == BLOCK_SNOW && !(GetLocalBiome(x, z) == BIOME_TUNDRA || GetLocalBiome(x, z) == BIOME_TAIGA || y > 82)) {
                world[x][y + 1][z] = BLOCK_AIR;
            }
            RebuildColumnTopAt(x, z);
        }
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


int SpawnParticleV24(int ptype, double x, double y, double z, double vx, double vy, double vz, double life, double size, int block)
{
    int i;
    int best;
    int activeParticles;
    int budget;
    double dx;
    double dz;

    dx = x - playerX;
    dz = z - playerZ;
    if (dx * dx + dz * dz > (double)g_particleCullDistanceBlocks * (double)g_particleCullDistanceBlocks) {
        g_renderProfileV33.particlesSpawnSkipped++;
        return -1;
    }

    activeParticles = 0;
    for (i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0.0f) { activeParticles++; }
    }
    budget = g_videoParticlesV7 ? RENDER_V33_PARTICLE_BUDGET_HIGH : RENDER_V33_PARTICLE_BUDGET_LOW;
    if ((ptype == PARTICLE_V24_RAIN || ptype == PARTICLE_V24_SNOW || ptype == PARTICLE_V24_SMOKE || ptype == PARTICLE_V24_BUBBLE) && activeParticles >= budget) {
        g_renderProfileV33.particlesSpawnSkipped++;
        return -1;
    }

    best = -1;
    for (i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0.0f) { best = i; break; }
        if (best < 0 || particles[i].life < particles[best].life) { best = i; }
    }
    if (best < 0) { g_renderProfileV33.particlesSpawnSkipped++; return -1; }
    /* Low-particle mode keeps old machines from filling all slots with rain/snow. */
    if (!g_videoParticlesV7 && (ptype == PARTICLE_V24_RAIN || ptype == PARTICLE_V24_SNOW || ptype == PARTICLE_V24_SMOKE)) {
        if ((WorldHash3D((int)x, (int)y, (int)z, g_worldSeed + best) & 3) != 0) {
            g_renderProfileV33.particlesSpawnSkipped++;
            return -1;
        }
    }
    particles[best].x = (float)x;
    particles[best].y = (float)y;
    particles[best].z = (float)z;
    particles[best].vx = (float)vx;
    particles[best].vy = (float)vy;
    particles[best].vz = (float)vz;
    particles[best].life = (float)life;
    particles[best].maxLife = (float)life;
    particles[best].block = block;
    particles[best].type = ptype;
    particles[best].variant = WorldHash3D((int)(x * 16.0), (int)(y * 16.0), (int)(z * 16.0), g_worldSeed + ptype + best) & 15;
    particles[best].size = (float)size;
    particles[best].age = 0.0f;
    particles[best].noClip = (ptype == PARTICLE_V24_SMOKE || ptype == PARTICLE_V24_FLAME || ptype == PARTICLE_V24_BUBBLE || ptype == PARTICLE_V24_SNOW) ? 1 : 0;
    if (ptype == PARTICLE_V24_SMOKE) { particles[best].gravity = -0.25f; }
    else if (ptype == PARTICLE_V24_FLAME || ptype == PARTICLE_V24_TORCH) { particles[best].gravity = -0.10f; }
    else if (ptype == PARTICLE_V24_BUBBLE) { particles[best].gravity = -0.80f; }
    else if (ptype == PARTICLE_V24_RAIN) { particles[best].gravity = 8.0f; }
    else if (ptype == PARTICLE_V24_SNOW) { particles[best].gravity = 0.35f; }
    else { particles[best].gravity = 6.0f; }
    g_renderProfileV33.particlesSpawned++;
    return best;
}

void SpawnSmokeParticlesV24(double x, double y, double z, int count)
{
    int i;
    int h;
    double rx;
    double rz;
    for (i = 0; i < count; i++) {
        h = WorldHash3D((int)(x * 11.0) + i, (int)(y * 7.0), (int)(z * 13.0), g_worldSeed + 9201);
        rx = (double)((h & 255) - 128) / 950.0;
        rz = (double)(((h >> 8) & 255) - 128) / 950.0;
        SpawnParticleV24(PARTICLE_V24_SMOKE, x + rx * 4.0, y, z + rz * 4.0, rx, 0.12 + (double)((h >> 16) & 63) / 500.0, rz, 1.1, 0.75, BLOCK_COBBLESTONE);
    }
}

void SpawnFlameParticlesV24(double x, double y, double z, int count)
{
    int i;
    int h;
    double rx;
    double rz;
    for (i = 0; i < count; i++) {
        h = WorldHash3D((int)(x * 17.0) + i, (int)(y * 19.0), (int)(z * 23.0), g_worldSeed + 9202);
        rx = (double)((h & 255) - 128) / 850.0;
        rz = (double)(((h >> 8) & 255) - 128) / 850.0;
        SpawnParticleV24(PARTICLE_V24_FLAME, x + rx * 2.0, y, z + rz * 2.0, rx, 0.20 + (double)((h >> 16) & 63) / 350.0, rz, 0.55, 0.62, BLOCK_FIRE);
    }
}

void SpawnSplashParticlesV24(double x, double y, double z, int count)
{
    int i;
    int h;
    double rx;
    double rz;
    for (i = 0; i < count; i++) {
        h = WorldHash3D((int)(x * 9.0) + i, (int)(y * 5.0), (int)(z * 3.0), g_worldSeed + 9203);
        rx = (double)((h & 255) - 128) / 210.0;
        rz = (double)(((h >> 8) & 255) - 128) / 210.0;
        SpawnParticleV24(PARTICLE_V24_SPLASH, x, y + 0.05, z, rx, 0.75 + (double)((h >> 16) & 63) / 120.0, rz, 0.45, 0.48, BLOCK_WATER);
    }
}

void SpawnExplosionParticlesV24(double x, double y, double z, int count)
{
    int i;
    int h;
    double rx;
    double ry;
    double rz;
    for (i = 0; i < count; i++) {
        h = WorldHash3D((int)(x * 31.0) + i, (int)(y * 29.0), (int)(z * 27.0), g_worldSeed + 9204);
        rx = (double)((h & 255) - 128) / 90.0;
        ry = (double)(((h >> 8) & 255)) / 120.0;
        rz = (double)(((h >> 16) & 255) - 128) / 90.0;
        SpawnParticleV24(PARTICLE_V24_EXPLOSION, x, y, z, rx, ry, rz, 0.65, 1.25, BLOCK_TNT);
    }
}

void SpawnBlockBreakParticles(int x, int y, int z, int block)
{
    int p;
    int h;
    float rx;
    float ry;
    float rz;
    int count;
    count = g_videoParticlesV7 ? 18 : 7;
    for (p = 0; p < count; p++) {
        h = WorldHash3D(LocalToGlobalBlockX(x) + p, y + p * 3, LocalToGlobalBlockZ(z) - p, g_worldSeed + 7000);
        rx = (float)((h & 255) - 128) / 128.0f;
        ry = (float)(((h >> 8) & 255)) / 255.0f;
        rz = (float)(((h >> 16) & 255) - 128) / 128.0f;
        SpawnParticleV24(PARTICLE_V24_DIGGING,
                         (double)x + 0.50 + (double)rx * 0.35,
                         (double)y + 0.50 + (double)ry * 0.30,
                         (double)z + 0.50 + (double)rz * 0.35,
                         (double)rx * 1.20,
                         1.10 + (double)ry * 1.40,
                         (double)rz * 1.20,
                         0.55, 0.70, block);
    }
}


void SpawnMobEffectParticles(Mob *m, int block, int count)
{
    int made;
    int h;
    float rx;
    float ry;
    float rz;
    int ptype;
    if (!m) { return; }
    ptype = PARTICLE_V24_DIGGING;
    if (block == BLOCK_FIRE || block == BLOCK_LIGHT || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { ptype = PARTICLE_V24_FLAME; }
    if (block == BLOCK_WATER) { ptype = PARTICLE_V24_SPLASH; }
    for (made = 0; made < count; made++) {
        h = WorldHash3D((int)(m->x * 16.0) + made, (int)(m->y * 16.0) + made * 5, (int)(m->z * 16.0) - made * 7, g_worldSeed + 7100 + m->type * 13);
        rx = (float)((h & 255) - 128) / 128.0f;
        ry = (float)(((h >> 8) & 255)) / 255.0f;
        rz = (float)(((h >> 16) & 255) - 128) / 128.0f;
        SpawnParticleV24(ptype,
                         (double)m->x + (double)rx * 0.35,
                         (double)m->y + 0.75 + (double)ry * (double)MobHeight(m->type) * 0.45,
                         (double)m->z + (double)rz * 0.35,
                         (double)rx * 0.90,
                         0.70 + (double)ry * 0.90,
                         (double)rz * 0.90,
                         0.40, 0.64, block);
    }
}

void UpdateParticles(double dt)
{
    int i;
    float fdt;
    int bx;
    int by;
    int bz;
    fdt = (float)dt;
    for (i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0.0f) { continue; }
        particles[i].age += fdt;
        particles[i].life -= fdt;
        if (particles[i].life <= 0.0f) { particles[i].life = 0.0f; continue; }
        particles[i].vy -= particles[i].gravity * fdt;
        if (particles[i].type == PARTICLE_V24_SMOKE) {
            particles[i].vx *= 0.98f; particles[i].vz *= 0.98f;
        } else if (particles[i].type == PARTICLE_V24_FLAME || particles[i].type == PARTICLE_V24_TORCH) {
            particles[i].vx *= 0.96f; particles[i].vz *= 0.96f;
        } else if (particles[i].type == PARTICLE_V24_SNOW) {
            particles[i].vx += g_weatherWindX_V24 * fdt * 0.20f;
            particles[i].vz += g_weatherWindZ_V24 * fdt * 0.20f;
        }
        particles[i].x += particles[i].vx * fdt;
        particles[i].y += particles[i].vy * fdt;
        particles[i].z += particles[i].vz * fdt;
        if (!particles[i].noClip) {
            bx = (int)floor(particles[i].x);
            by = (int)floor(particles[i].y);
            bz = (int)floor(particles[i].z);
            if (IsSolidBlock(GetBlock(bx, by, bz))) {
                if (particles[i].type == PARTICLE_V24_RAIN) { SpawnSplashParticlesV24(particles[i].x, (double)by + 1.02, particles[i].z, 2); }
                particles[i].life = 0.0f;
            }
        }
        if (particles[i].type == PARTICLE_V24_BUBBLE) {
            if (GetBlock((int)floor(particles[i].x), (int)floor(particles[i].y), (int)floor(particles[i].z)) != BLOCK_WATER) { particles[i].life = 0.0f; }
        }
    }
}

int ParticleV54_UseTerrainAtlas(int ptype)
{
    if (ptype == PARTICLE_V24_DIGGING) { return 1; }
    return 0;
}

void ParticleV54_GetParticleTile(int ptype, int variant, int block, int *col, int *row)
{
    int frame;
    frame = variant & 3;
    if (ptype == PARTICLE_V24_SMOKE) { *col = 7 + frame; *row = 0; return; }
    if (ptype == PARTICLE_V24_FLAME || ptype == PARTICLE_V24_TORCH) { *col = 0 + (g_textureFxFireFrameV54 & 1); *row = 1; return; }
    if (ptype == PARTICLE_V24_BUBBLE) { *col = 0; *row = 2; return; }
    if (ptype == PARTICLE_V24_SPLASH || ptype == PARTICLE_V24_RAIN) { *col = 1 + (frame & 1); *row = 2; return; }
    if (ptype == PARTICLE_V24_SNOW) { *col = 2 + (frame & 1); *row = 2; return; }
    if (ptype == PARTICLE_V24_EXPLOSION) { *col = 4 + (frame & 3); *row = 1; return; }
    *col = 0; *row = 0;
    (void)block;
}

void ParticleV54_GetColorAndAlpha(Particle *p, float *r, float *g, float *b, float *a, float *size)
{
    float fade;
    fade = p->maxLife > 0.001f ? p->life / p->maxLife : 0.0f;
    if (fade < 0.0f) { fade = 0.0f; }
    if (fade > 1.0f) { fade = 1.0f; }

    GetParticleColorForBlock(p->block, r, g, b);
    *a = fade;
    *size = p->size * (0.70f + 0.30f * fade);

    if (p->type == PARTICLE_V24_SMOKE) {
        *r = 0.22f; *g = 0.22f; *b = 0.22f;
        *a = fade * 0.55f;
        *size = p->size * (0.80f + p->age * 0.55f);
    } else if (p->type == PARTICLE_V24_FLAME || p->type == PARTICLE_V24_TORCH) {
        *r = 1.0f; *g = 0.48f; *b = 0.10f;
        *a = fade * 0.92f;
        *size = p->size * (0.62f + 0.18f * (float)sin((double)p->age * 18.0));
    } else if (p->type == PARTICLE_V24_BUBBLE) {
        *r = 0.70f; *g = 0.86f; *b = 1.0f; *a = fade * 0.62f;
    } else if (p->type == PARTICLE_V24_SPLASH || p->type == PARTICLE_V24_RAIN) {
        *r = 0.55f; *g = 0.72f; *b = 1.0f; *a = fade * 0.72f;
    } else if (p->type == PARTICLE_V24_SNOW) {
        *r = 1.0f; *g = 1.0f; *b = 1.0f; *a = fade * 0.82f;
    } else if (p->type == PARTICLE_V24_EXPLOSION) {
        *r = 1.0f; *g = 0.35f + 0.25f * fade; *b = 0.08f;
        *a = fade * 0.95f;
        *size = p->size * (0.55f + (1.0f - fade) * 0.90f);
    }
}

void ParticleV54_DrawBillboard(float x, float y, float z, float size, float u0, float v0, float u1, float v1, float rx, float rz, float ux, float uy, float uz)
{
    glTexCoord2f(u0, v0); glVertex3f(x - rx * size - ux, y - uy * size, z - rz * size - uz);
    glTexCoord2f(u1, v0); glVertex3f(x + rx * size - ux, y - uy * size, z + rz * size - uz);
    glTexCoord2f(u1, v1); glVertex3f(x + rx * size + ux, y + uy * size, z + rz * size + uz);
    glTexCoord2f(u0, v1); glVertex3f(x - rx * size + ux, y + uy * size, z - rz * size + uz);
}

void RenderParticles(void)
{
    int pass;
    int i;
    int col;
    int row;
    int shard;
    int useTerrain;
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
    GLuint tex;
    int atlasW;
    int atlasH;

    yawRad = (float)(yaw * PI / 180.0);
    rx = (float)cos(yawRad) * 0.08f;
    rz = (float)-sin(yawRad) * 0.08f;
    ux = 0.0f;
    uy = 0.08f;
    uz = 0.0f;

    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.04f);
    glDepthMask(GL_FALSE);

    for (pass = 0; pass < 2; pass++) {
        tex = pass == 0 ? texTerrain : texBetaParticles;
        if (!tex) { continue; }
        atlasW = pass == 0 ? TERRAIN_ATLAS_WIDTH : 256;
        atlasH = pass == 0 ? TERRAIN_ATLAS_HEIGHT : 256;
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBegin(GL_QUADS);
        for (i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life <= 0.0f) { continue; }
            useTerrain = ParticleV54_UseTerrainAtlas(particles[i].type);
            if ((pass == 0 && !useTerrain) || (pass == 1 && useTerrain)) { continue; }
            if ((particles[i].x - (float)playerX) * (particles[i].x - (float)playerX) +
                (particles[i].z - (float)playerZ) * (particles[i].z - (float)playerZ) >
                (float)(g_particleCullDistanceBlocks * g_particleCullDistanceBlocks)) {
                g_renderProfileV33.particlesDistanceCulled++;
                continue;
            }
            if (!IsPointProbablyInView((double)particles[i].x, (double)particles[i].z, 8.0, -0.25)) {
                g_renderProfileV33.particlesFrustumCulled++;
                continue;
            }
            g_renderProfileV33.particlesDrawn++;

            if (useTerrain) {
                GetBlockTile(particles[i].block, 0, &col, &row);
                GetTileUVEx(col, row, atlasW, atlasH, &u0, &v0, &u1, &v1);
                du = (u1 - u0) * 0.25f;
                dv = (v1 - v0) * 0.25f;
                shard = (i + particles[i].block * 3 + particles[i].variant) & 15;
                u0 = u0 + du * (float)(shard & 3);
                v0 = v0 + dv * (float)((shard >> 2) & 3);
                u1 = u0 + du;
                v1 = v0 + dv;
            } else {
                ParticleV54_GetParticleTile(particles[i].type, particles[i].variant, particles[i].block, &col, &row);
                GetTileUVEx(col, row, atlasW, atlasH, &u0, &v0, &u1, &v1);
            }

            ParticleV54_GetColorAndAlpha(&particles[i], &r, &g, &b, &a, &size);
            glColor4f(r, g, b, a);
            ParticleV54_DrawBillboard(particles[i].x, particles[i].y, particles[i].z, size, u0, v0, u1, v1, rx, rz, ux, uy, uz);
        }
        glEnd();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_ALPHA_TEST);
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
            terrainChunkTransDirty[cx][cz] = 1;
            terrainChunkSkipPassSolidV47[cx][cz] = 0;
            terrainChunkSkipPassTransV47[cx][cz] = 0;
            terrainChunkHasSpecialV47[cx][cz] = 1;
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
            if (terrainChunkTransLists[cx][cz]) {
                glDeleteLists(terrainChunkTransLists[cx][cz], 1);
                terrainChunkTransLists[cx][cz] = 0;
            }
            terrainChunkDirty[cx][cz] = 1;
            terrainChunkTransDirty[cx][cz] = 1;
            terrainChunkSkipPassSolidV47[cx][cz] = 0;
            terrainChunkSkipPassTransV47[cx][cz] = 0;
            terrainChunkHasSpecialV47[cx][cz] = 1;
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

#define MARK_CHUNK_DIRTY_V33(mx, mz) \
    do { \
        if ((mx) >= 0 && (mx) < WORLD_CHUNKS_X && (mz) >= 0 && (mz) < WORLD_CHUNKS_Z) { \
            terrainChunkDirty[(mx)][(mz)] = 1; \
            terrainChunkTransDirty[(mx)][(mz)] = 1; \
            terrainChunkSkipPassSolidV47[(mx)][(mz)] = 0; \
            terrainChunkSkipPassTransV47[(mx)][(mz)] = 0; \
            terrainChunkHasSpecialV47[(mx)][(mz)] = 1; \
            if (terrainChunkDirtyAgeV33[(mx)][(mz)] < 65535) { terrainChunkDirtyAgeV33[(mx)][(mz)]++; } \
        } \
    } while (0)

    MARK_CHUNK_DIRTY_V33(cx, cz);

    /* Java WorldRenderer.markDirty style: block edits on a seam invalidate the
       neighboring display list too, or the face that becomes visible at the
       seam can stay stale. */
    if ((x & (CHUNK_SIZE - 1)) == 0) { MARK_CHUNK_DIRTY_V33(cx - 1, cz); }
    if ((x & (CHUNK_SIZE - 1)) == CHUNK_SIZE - 1) { MARK_CHUNK_DIRTY_V33(cx + 1, cz); }
    if ((z & (CHUNK_SIZE - 1)) == 0) { MARK_CHUNK_DIRTY_V33(cx, cz - 1); }
    if ((z & (CHUNK_SIZE - 1)) == CHUNK_SIZE - 1) { MARK_CHUNK_DIRTY_V33(cx, cz + 1); }

#undef MARK_CHUNK_DIRTY_V33
}

int IsChunkProbablyVisible(int cx, int cz, int distSq)
{
    float dot;
    return RendererV33_IsChunkInFrustum(cx, cz, distSq, &dot);
}

int RendererV41_ShouldBuildFullChunk(int cx, int cz)
{
    int pcx;
    int pcz;
    int dx;
    int dz;

    pcx = ((int)floor(playerX)) / CHUNK_SIZE;
    pcz = ((int)floor(playerZ)) / CHUNK_SIZE;
    dx = cx - pcx;
    dz = cz - pcz;

    if (!g_renderV41SurfaceShellFarChunks) { return 1; }
    if (dx * dx + dz * dz <= g_renderV41FullMeshRadiusChunks * g_renderV41FullMeshRadiusChunks) { return 1; }
    return 0;
}

int RendererV41_GetColumnStartY(int cx, int cz, int topY)
{
    int startY;

    if (topY < 1) { return 1; }
    if (RendererV41_ShouldBuildFullChunk(cx, cz)) { return 1; }

    /* Far chunks use a Java/ClassicCube-inspired visible surface shell:
       keep the skyline, terrain sides, trees and above-ground structures while
       avoiding an expensive scan through every underground air/stone block. */
    startY = topY - g_renderV41FarShellDepth;
    if (startY < 1) { startY = 1; }
    return startY;
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
    int emitted;
    int specialFound;
    DWORD buildStartMs;

    buildStartMs = GetTickCount();

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

    emitted = 0;
    specialFound = 0;

    glNewList(terrainChunkLists[cx][cz], GL_COMPILE);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    if (texTerrain) { TessellatorV8_BeginTerrain(texTerrain); }

    for (x = minX; x <= maxX; x++) {
        for (z = minZ; z <= maxZ; z++) {
            topY = columnTop[x][z];
            if (topY < 0) {
                continue;
            }
            if (topY >= WORLD_Y) {
                topY = WORLD_Y - 1;
            }

            /* V41: full near meshes, surface-shell far meshes.  This keeps
               nearby caves/mines correct while stopping render distance 4-6
               from compiling every hidden underground voxel on old hardware. */
            startY = RendererV41_GetColumnStartY(cx, cz, topY);

            for (y = startY; y <= topY; y++) {
                block = world[x][y][z];
                if (block == BLOCK_AIR) {
                    continue;
                }
                if (IsRemovedPlantBlockV12(block)) {
                    continue;
                }
                if (g_rendererV8SkipTranslucentInSolidPass && RendererV8_IsTranslucentBlock(block)) {
                    continue;
                }

                if (block == BLOCK_TORCH || IsSpecialBlockV5(block) || WorldGenV3_IsCrossPlantBlock(block)) {
                    /* Special/cross blocks are rendered in the near dynamic pass, not baked
                       into every far display list.  This reduces stale display lists and
                       makes alpha/small blocks cheaper on old OpenGL 1.1 drivers. */
                    specialFound = 1;
                    continue;
                }

                if (ShouldDrawFace(x, y + 1, z, block) ||
                    ShouldDrawFace(x, y - 1, z, block) ||
                    ShouldDrawFace(x, y, z - 1, block) ||
                    ShouldDrawFace(x, y, z + 1, block) ||
                    ShouldDrawFace(x - 1, y, z, block) ||
                    ShouldDrawFace(x + 1, y, z, block)) {
                    DrawBlock(x, y, z, block);
                    emitted = 1;
                }
            }
        }
    }

    if (g_tessellatorActiveV8) { TessellatorV8_End(); }
    glEndList();
    terrainChunkDirty[cx][cz] = 0;
    terrainChunkDirtyAgeV33[cx][cz] = 0;
    terrainChunkSkipPassSolidV47[cx][cz] = emitted ? 0 : 1;
    terrainChunkHasSpecialV47[cx][cz] = specialFound ? 1 : 0;
    terrainChunkLastSolidBuildFrameV47[cx][cz] = g_rendererFrameIdV33;
    g_renderProfileV33.meshBuildMs += (double)(GetTickCount() - buildStartMs);
}

int GetBlockSoundGroup(int block)
{
    if (block == BLOCK_WOOD || block == BLOCK_PLANKS || block == BLOCK_CHEST || block == BLOCK_WORKBENCH || block == BLOCK_WOOD_DOOR || block == BLOCK_FENCE || block == BLOCK_TRAPDOOR || block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL) { return SOUND_GROUP_WOOD_V35; }
    if (block == BLOCK_SAND || block == BLOCK_SANDSTONE || block == BLOCK_SOULSAND) { return SOUND_GROUP_SAND_V35; }
    if (block == BLOCK_GRAVEL || block == BLOCK_CLAY) { return SOUND_GROUP_GRAVEL_V35; }
    if (block == BLOCK_SNOW || block == BLOCK_ICE) { return SOUND_GROUP_SNOW_V35; }
    if (block == BLOCK_LEAVES || block == BLOCK_WOOL) { return SOUND_GROUP_CLOTH_V35; }
    if (block == BLOCK_DIRT || block == BLOCK_GRASS || block == BLOCK_FARMLAND || block == BLOCK_FLOWER_YELLOW || block == BLOCK_FLOWER_RED || block == BLOCK_TALL_GRASS || block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED) { return SOUND_GROUP_GRASS_V35; }
    if (block == BLOCK_GLASS) { return SOUND_GROUP_GLASS_V35; }
    if (block == BLOCK_IRON_ORE || block == BLOCK_GOLD_ORE || block == BLOCK_DIAMOND_ORE || block == BLOCK_REDSTONE_ORE || block == BLOCK_IRON_BLOCK || block == BLOCK_GOLD_BLOCK || block == BLOCK_DIAMOND_BLOCK || block == BLOCK_FURNACE || block == BLOCK_FURNACE_LIT) { return SOUND_GROUP_METAL_V35; }
    return SOUND_GROUP_STONE_V35;
}


void PlayBlockBreakSound(int block)
{
    int group; const char *prefix;
    group = GetBlockSoundGroup(block); prefix = "assets\\sounds\\dig\\dig_stone";
    if (group == SOUND_GROUP_WOOD_V35) { prefix = "assets\\sounds\\dig\\dig_wood"; }
    else if (group == SOUND_GROUP_SAND_V35) { prefix = "assets\\sounds\\dig\\dig_sand"; }
    else if (group == SOUND_GROUP_GRAVEL_V35) { prefix = "assets\\sounds\\dig\\dig_gravel"; }
    else if (group == SOUND_GROUP_SNOW_V35) { prefix = "assets\\sounds\\dig\\dig_snow"; }
    else if (group == SOUND_GROUP_CLOTH_V35) { prefix = "assets\\sounds\\dig\\dig_cloth"; }
    else if (group == SOUND_GROUP_GRASS_V35) { prefix = "assets\\sounds\\dig\\dig_grass"; }
    else if (group == SOUND_GROUP_GLASS_V35) { prefix = "assets\\sounds\\dig\\dig_glass"; }
    PlayRandomPathV35(prefix, 1, 4, ".mp3", "assets\\sounds\\dig\\dig_stone1.mp3", 0.90, 1.00);
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

        /* V41 performance fix:
           Do NOT silently force render distance back to 1.  The Java renderer
           keeps the selected renderer grid and instead throttles updates.  We
           now lower mesh/particle/model budgets, not the user's distance. */
        if (g_currentFPS > 0 && g_currentFPS < 24) {
            g_chunkMeshBuildBudget = 2;
            g_particleCullDistanceBlocks = 24;
            g_mobFullModelDistanceBlocks = 10;
        } else if (g_currentFPS > 0 && g_currentFPS < 38) {
            g_chunkMeshBuildBudget = 3;
            g_particleCullDistanceBlocks = 32;
            g_mobFullModelDistanceBlocks = 14;
        } else if (g_currentFPS > 0 && g_currentFPS < 55) {
            g_chunkMeshBuildBudget = 4;
            g_particleCullDistanceBlocks = 40;
            g_mobFullModelDistanceBlocks = 16;
        } else {
            g_chunkMeshBuildBudget = 6;
            g_particleCullDistanceBlocks = 48;
            g_mobFullModelDistanceBlocks = 18;
        }

        g_fpsFrameCounter = 0;
        g_fpsTimer = 0.0;
    }
}

int DroppedItemBoxBlockedV27(double x, double y, double z)
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
    int block;
    double r;
    double h;

    r = 0.125;
    h = 0.25;
    minX = (int)floor(x - r);
    maxX = (int)floor(x + r);
    minY = (int)floor(y - h * 0.5);
    maxY = (int)floor(y + h * 0.5);
    minZ = (int)floor(z - r);
    maxZ = (int)floor(z + r);

    for (bx = minX; bx <= maxX; bx++) {
        for (by = minY; by <= maxY; by++) {
            for (bz = minZ; bz <= maxZ; bz++) {
                if (!IsInsideWorld(bx, by, bz)) { return 1; }
                block = GetBlock(bx, by, bz);
                if (IsSolidBlock(block)) { return 1; }
            }
        }
    }
    return 0;
}

void RebaseDroppedItemsAfterWorldStream(int oldOriginX, int oldOriginZ)
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
    for (i = 0; i < MAX_DROPPED_ITEMS; i++) {
        if (!droppedItems[i].active) { continue; }
        gx = (double)oldOriginX + droppedItems[i].x;
        gz = (double)oldOriginZ + droppedItems[i].z;
        droppedItems[i].x = gx - (double)worldOriginBlockX;
        droppedItems[i].z = gz - (double)worldOriginBlockZ;
        dx = gx - pgx;
        dz = gz - pgz;
        if (droppedItems[i].x < -8.0 || droppedItems[i].z < -8.0 ||
            droppedItems[i].x > (double)(WORLD_X + 8) ||
            droppedItems[i].z > (double)(WORLD_Z + 8) ||
            (dx * dx + dz * dz) > 96.0 * 96.0) {
            droppedItems[i].active = 0;
        }
    }
}

void MergeNearbyDroppedItemsV27(int index)
{
    int i;
    DroppedItem *a;
    DroppedItem *b;
    int move;
    double dx;
    double dy;
    double dz;

    if (index < 0 || index >= MAX_DROPPED_ITEMS) { return; }
    a = &droppedItems[index];
    if (!a->active) { return; }
    for (i = 0; i < MAX_DROPPED_ITEMS; i++) {
        if (i == index) { continue; }
        b = &droppedItems[i];
        if (!b->active) { continue; }
        if (b->item != a->item) { continue; }
        if (a->count >= MAX_STACK) { return; }
        dx = b->x - a->x;
        dy = b->y - a->y;
        dz = b->z - a->z;
        if (dx * dx + dy * dy + dz * dz > 0.75 * 0.75) { continue; }
        move = MAX_STACK - a->count;
        if (move > b->count) { move = b->count; }
        a->count += move;
        b->count -= move;
        if (b->count <= 0) { b->active = 0; }
    }
}

void InitPickupFxV38(void)
{
    int i;
    for (i = 0; i < MAX_PICKUP_FX_V38; i++) {
        pickupFxV38[i].active = 0;
        pickupFxV38[i].item = ITEM_NONE;
        pickupFxV38[i].damage = 0;
        pickupFxV38[i].count = 0;
        pickupFxV38[i].sx = 0.0;
        pickupFxV38[i].sy = 0.0;
        pickupFxV38[i].sz = 0.0;
        pickupFxV38[i].age = 0.0;
        pickupFxV38[i].duration = ENTITY_PICKUP_FX_TIME_V38;
        pickupFxV38[i].hoverStart = 0.0;
    }
}

void InitDroppedItems(void)
{
    int i;

    for (i = 0; i < MAX_DROPPED_ITEMS; i++) {
        droppedItems[i].active = 0;
        droppedItems[i].item = ITEM_NONE;
        droppedItems[i].count = 0;
        droppedItems[i].damage = 0;
        droppedItems[i].health = 5;
        droppedItems[i].x = 0.0;
        droppedItems[i].y = 0.0;
        droppedItems[i].z = 0.0;
        droppedItems[i].prevX = 0.0;
        droppedItems[i].prevY = 0.0;
        droppedItems[i].prevZ = 0.0;
        droppedItems[i].vx = 0.0;
        droppedItems[i].vy = 0.0;
        droppedItems[i].vz = 0.0;
        droppedItems[i].age = 0.0;
        droppedItems[i].spin = 0.0;
        droppedItems[i].hoverStart = 0.0;
        droppedItems[i].pickupDelay = 0.0;
    }
    InitPickupFxV38();
}


int WorldGenV55_FindDroppedItemRestY(double x, double y, double z, double *outY)
{
    double testY;
    double bestY;
    int found;
    int steps;

    if (!outY) { return 0; }
    bestY = y;
    found = 0;
    testY = y;
    if (testY > (double)(WORLD_Y - 2)) { testY = (double)(WORLD_Y - 2); }
    if (testY < 1.25) { testY = 1.25; }

    for (steps = 0; steps < 900 && testY > 1.10; steps++) {
        if (DroppedItemBoxBlockedV27(x, testY - 0.075, z)) {
            found = 1;
            bestY = floor(testY - 0.075) + 1.0625;
            break;
        }
        testY -= 0.10;
    }

    if (!found) { return 0; }
    if (bestY < 1.0625) { bestY = 1.0625; }
    if (bestY > (double)(WORLD_Y - 2)) { bestY = (double)(WORLD_Y - 2); }
    *outY = bestY;
    return 1;
}

int AddDroppedItemStackV38(int item, int count, int damage, double x, double y, double z,
                            double vx, double vy, double vz)
{
    int i;
    int idx;
    int firstIdx;
    int chunk;
    double lx;
    double ly;
    double lz;
    double px;
    double pz;
    int lift;

    if (item == ITEM_NONE || count <= 0) { return -1; }

    lx = x;
    ly = y;
    lz = z;

    /* V29 EntityItem local-coordinate rule:
       all runtime entities use local world-window coordinates.  The old helper
       tried to guess whether coordinates were local or global and could rebase
       Q-dropped items against worldOriginBlockX/Z, making them appear far away.
       Keep the public AddDroppedItem API local-only, with a small safety clamp
       near the player if a caller sends nonsense. */
    if (lx < -2.0 || lz < -2.0 || lx > (double)(WORLD_X + 2) || lz > (double)(WORLD_Z + 2)) {
        px = playerX;
        pz = playerZ;
        lx = px;
        lz = pz;
    }
    if (lx < 0.20) { lx = 0.20; }
    if (lz < 0.20) { lz = 0.20; }
    if (lx > (double)WORLD_X - 1.20) { lx = (double)WORLD_X - 1.20; }
    if (lz > (double)WORLD_Z - 1.20) { lz = (double)WORLD_Z - 1.20; }
    if (ly < 1.0) { ly = playerY + 0.65; }
    if (ly > (double)(WORLD_Y - 2)) { ly = (double)(WORLD_Y - 2); }

    /* If the item starts inside the player or a block face, lift it just enough
       to sit in free space.  Do not snap it to columnTop because that is what
       makes cave/dungeon/mined drops jump to the surface. */
    for (lift = 0; lift < 12 && DroppedItemBoxBlockedV27(lx, ly, lz); lift++) {
        ly += 0.10;
        if (ly > (double)(WORLD_Y - 2)) { break; }
    }
    /* V56 P0.1: old CloneMC dropped items sit on the ground and spin.
       The rest height must be just above the supporting block so the onGround
       test remains true; earlier 1.125 placement made the item repeatedly fall
       and pop back up. */
    WorldGenV55_FindDroppedItemRestY(lx, ly, lz, &ly);
    vx = 0.0;
    vy = 0.0;
    vz = 0.0;

    firstIdx = -1;
    while (count > 0) {
        idx = -1;
        for (i = 0; i < MAX_DROPPED_ITEMS; i++) { if (!droppedItems[i].active) { idx = i; break; } }
        if (idx < 0) { return firstIdx; }
        if (firstIdx < 0) { firstIdx = idx; }
        chunk = count > MAX_STACK ? MAX_STACK : count;
        droppedItems[idx].active = 1;
        droppedItems[idx].item = item;
        droppedItems[idx].count = chunk;
        droppedItems[idx].damage = damage;
        droppedItems[idx].health = 5;
        droppedItems[idx].x = lx;
        droppedItems[idx].y = ly;
        droppedItems[idx].z = lz;
        droppedItems[idx].prevX = lx;
        droppedItems[idx].prevY = ly;
        droppedItems[idx].prevZ = lz;
        droppedItems[idx].vx = vx;
        droppedItems[idx].vy = vy;
        droppedItems[idx].vz = vz;
        droppedItems[idx].age = 0.0;
        droppedItems[idx].hoverStart = ((double)(WorldHash3D((int)(lx * 16.0), (int)(ly * 16.0), (int)(lz * 16.0), g_worldSeed + item + damage) & 1023) / 1024.0) * PI * 2.0;
        droppedItems[idx].spin = (droppedItems[idx].hoverStart) * 57.29578;
        droppedItems[idx].pickupDelay = 0.0;
        MergeNearbyDroppedItemsV27(idx);
        count -= chunk;
    }
    return firstIdx;
}

int AddDroppedItemLocalV29(int item, int count, double x, double y, double z,
                            double vx, double vy, double vz)
{
    return AddDroppedItemStackV38(item, count, 0, x, y, z, vx, vy, vz);
}

int AddDroppedItem(int item, int count, double x, double y, double z,
                   double vx, double vy, double vz)
{
    return AddDroppedItemStackV38(item, count, 0, x, y, z, vx, vy, vz);
}

void DropSelectedItemV29(void)
{
    InventorySlot *slot;
    double yawRad;
    double dx;
    double dz;
    double sx;
    double sy;
    double sz;
    int item;
    int damage;
    int dropIndex;

    /* V54_P0_1_Q_DROP_CLEANUP
       One final Q-drop path: local coordinates only, no global rebasing, no
       weird surface snapping, no high throw impulse.  This matches the old
       CloneMC feeling: the item appears about one block in front of the player,
       spins in place, and waits briefly before it can be picked up. */
    if (selectedHotbarSlot < 0 || selectedHotbarSlot >= HOTBAR_SLOTS) { return; }
    slot = &hotbar[selectedHotbarSlot];
    if (slot->item == ITEM_NONE || slot->count <= 0) { return; }

    item = slot->item;
    damage = slot->damage;
    yawRad = yaw * PI / 180.0;
    dx = -sin(yawRad);
    dz = -cos(yawRad);

    sx = playerX + dx * 1.05;
    sy = playerY + 0.82;
    sz = playerZ + dz * 1.05;

    dropIndex = AddDroppedItemStackV38(item, 1, damage, sx, sy, sz,
                                      0.0, 0.0, 0.0);
    if (dropIndex >= 0 && dropIndex < MAX_DROPPED_ITEMS) {
        droppedItems[dropIndex].pickupDelay = 0.90;
        droppedItems[dropIndex].hoverStart = ((double)(WorldHash3D((int)(sx * 16.0), (int)(sy * 16.0), (int)(sz * 16.0), g_worldSeed + item + damage + 5401) & 1023) / 1024.0) * PI * 2.0;
        droppedItems[dropIndex].spin = droppedItems[dropIndex].hoverStart * 57.29578;
    }

    slot->count--;
    if (slot->count <= 0) { slot->item = ITEM_NONE; slot->count = 0; slot->damage = 0; }
    StartHandUse();
}

void DropSelectedItem(void)
{
    DropSelectedItemV29();
}

void UpdatePickupFxV38(double dt)
{
    int i;
    for (i = 0; i < MAX_PICKUP_FX_V38; i++) {
        if (!pickupFxV38[i].active) { continue; }
        pickupFxV38[i].age += dt;
        if (pickupFxV38[i].age >= pickupFxV38[i].duration) {
            pickupFxV38[i].active = 0;
        }
    }
}

void SpawnPickupFxV38(DroppedItem *d)
{
    int i;
    int best;
    double oldest;
    if (!d || !d->active) { return; }
    best = -1;
    oldest = -1.0;
    for (i = 0; i < MAX_PICKUP_FX_V38; i++) {
        if (!pickupFxV38[i].active) { best = i; break; }
        if (pickupFxV38[i].age > oldest) { oldest = pickupFxV38[i].age; best = i; }
    }
    if (best < 0) { return; }
    pickupFxV38[best].active = 1;
    pickupFxV38[best].item = d->item;
    pickupFxV38[best].damage = d->damage;
    pickupFxV38[best].count = d->count;
    pickupFxV38[best].sx = d->x;
    pickupFxV38[best].sy = d->y;
    pickupFxV38[best].sz = d->z;
    pickupFxV38[best].age = 0.0;
    pickupFxV38[best].duration = ENTITY_PICKUP_FX_TIME_V38;
    pickupFxV38[best].hoverStart = d->hoverStart;
}

void UpdateDroppedItems(double dt)
{
    int i;
    double dx;
    double dy;
    double dz;
    double d2;
    double oldX;
    double oldY;
    double oldZ;
    double nx;
    double ny;
    double nz;
    int bx;
    int by;
    int bz;
    int block;
    int blockBelow;
    int inLava;
    int inWater;
    int onGround;
    double friction;
    double gravity;
    double restY;

    if (dt > 0.10) { dt = 0.10; }
    UpdatePickupFxV38(dt);
    for (i = 0; i < MAX_DROPPED_ITEMS; i++) {
        if (!droppedItems[i].active) { continue; }
        droppedItems[i].prevX = droppedItems[i].x;
        droppedItems[i].prevY = droppedItems[i].y;
        droppedItems[i].prevZ = droppedItems[i].z;
        droppedItems[i].age += dt;
        droppedItems[i].spin = ((droppedItems[i].age) + droppedItems[i].hoverStart) * 57.29578;
        if (droppedItems[i].pickupDelay > 0.0) {
            droppedItems[i].pickupDelay -= dt;
            if (droppedItems[i].pickupDelay < 0.0) { droppedItems[i].pickupDelay = 0.0; }
        }
        bx = (int)floor(droppedItems[i].x);
        by = (int)floor(droppedItems[i].y);
        bz = (int)floor(droppedItems[i].z);
        block = GetBlock(bx, by, bz);
        inLava = (block == BLOCK_LAVA);
        inWater = (block == BLOCK_WATER);
        onGround = DroppedItemBoxBlockedV27(droppedItems[i].x, droppedItems[i].y - 0.075, droppedItems[i].z);
        if (onGround && !inLava && !inWater) {
            if (WorldGenV55_FindDroppedItemRestY(droppedItems[i].x, droppedItems[i].y + 0.20, droppedItems[i].z, &restY)) { droppedItems[i].y = restY; }
            droppedItems[i].vy = 0.0;
            if (fabs(droppedItems[i].vx) < 0.012) { droppedItems[i].vx = 0.0; }
            if (fabs(droppedItems[i].vz) < 0.012) { droppedItems[i].vz = 0.0; }
        } else if (inLava) {
            droppedItems[i].vy = 0.00;
            droppedItems[i].vx *= 0.80;
            droppedItems[i].vz *= 0.80;
            if (((int)(droppedItems[i].age * 5.0) & 7) == 0) {
                SpawnSmokeParticlesV24(droppedItems[i].x, droppedItems[i].y + 0.08, droppedItems[i].z, 1);
            }
        } else {
            gravity = inWater ? 1.20 : 3.20;
            droppedItems[i].vy -= gravity * dt;
            if (droppedItems[i].vy < -3.80) { droppedItems[i].vy = -3.80; }
        }
        oldX = droppedItems[i].x;
        oldY = droppedItems[i].y;
        oldZ = droppedItems[i].z;
        nx = oldX + droppedItems[i].vx * dt;
        if (!DroppedItemBoxBlockedV27(nx, oldY, oldZ)) { droppedItems[i].x = nx; } else { droppedItems[i].vx *= -0.10; }
        nz = oldZ + droppedItems[i].vz * dt;
        if (!DroppedItemBoxBlockedV27(droppedItems[i].x, oldY, nz)) { droppedItems[i].z = nz; } else { droppedItems[i].vz *= -0.10; }
        ny = oldY + droppedItems[i].vy * dt;
        if (!DroppedItemBoxBlockedV27(droppedItems[i].x, ny, droppedItems[i].z)) { droppedItems[i].y = ny; }
        else {
            if (droppedItems[i].vy < 0.0) {
                droppedItems[i].y = floor(oldY - 0.125) + 1.0625;
                /* V54: old CloneMC-style dropped entities rest on the ground and
                   spin; they should not keep hopping up and down. */
                droppedItems[i].vy = 0.0;
            }
            else { droppedItems[i].vy = 0.0; }
        }
        friction = 0.98;
        blockBelow = GetBlock((int)floor(droppedItems[i].x), (int)floor(droppedItems[i].y - 0.08), (int)floor(droppedItems[i].z));
        if (DroppedItemBoxBlockedV27(droppedItems[i].x, droppedItems[i].y - 0.06, droppedItems[i].z)) {
            friction = 0.588;
            if (blockBelow == BLOCK_ICE) { friction = 0.96; }
            if (blockBelow == BLOCK_SOULSAND) { friction = 0.40; }
        }
        if (inWater) { friction = 0.80; }
        if (inLava) { friction = 0.50; }
        droppedItems[i].vx *= friction;
        droppedItems[i].vz *= friction;
        droppedItems[i].vy *= 0.98;
        if (droppedItems[i].x < -8.0 || droppedItems[i].z < -8.0 ||
            droppedItems[i].x > (double)(WORLD_X + 8) || droppedItems[i].z > (double)(WORLD_Z + 8) ||
            droppedItems[i].y < -4.0 || droppedItems[i].age > ENTITY_ITEM_DESPAWN_SECONDS_V38 ||
            droppedItems[i].health <= 0) { droppedItems[i].active = 0; continue; }
        if (((int)(droppedItems[i].age * 2.0) % 2) == 0) { MergeNearbyDroppedItemsV27(i); }
        dx = droppedItems[i].x - playerX;
        dy = droppedItems[i].y - (playerY + 0.85);
        dz = droppedItems[i].z - playerZ;
        d2 = dx * dx + dy * dy + dz * dz;
        if (droppedItems[i].pickupDelay <= 0.0 && d2 < 0.62) {
            if (AddItemToInventoryWithDamageV38(droppedItems[i].item, droppedItems[i].count, droppedItems[i].damage)) {
                SpawnPickupFxV38(&droppedItems[i]);
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

int ItemRenderV38_GetRenderCopies(int count)
{
    if (count > 20) { return 4; }
    if (count > 5) { return 3; }
    if (count > 1) { return 2; }
    return 1;
}

float ItemRenderV38_CopyOffset(int item, int damage, int copy, int axis, float spread)
{
    int h;
    if (copy <= 0) { return 0.0f; }
    h = WorldHash3D(item + copy * 37, damage + axis * 97, copy * 53 + axis * 17, 187);
    return (float)(((double)(h & 1023) / 1023.0) * 2.0 - 1.0) * spread;
}

float ItemRenderV38_GetDroppedBlockScale(int block)
{
    if (!ShouldRenderBlockItemAs3DV27(block)) { return 0.50f; }
    return 0.25f;
}

void DrawDroppedItemVisualV38(int item, int count, int damage, double age, double hoverStart, double x, double y, double z, int pickupFx)
{
    int block;
    int col;
    int row;
    int useTerrain;
    int copies;
    int copy;
    float size;
    float bob;
    float rot;
    float offX;
    float offY;
    float offZ;
    float scale;
    GLuint tex;
    int atlasW;
    int atlasH;

    copies = ItemRenderV38_GetRenderCopies(count);
    if (pickupFx) { copies = 1; }
    /* V54_P0_1: dropped entities now spin in place like older CloneMC instead of
       visibly bouncing/bobbing.  Pickup FX still gets a tiny lift so the
       player can see the pickup animation. */
    bob = pickupFx ? (float)(sin((age * 2.0) + hoverStart) * 0.035 + 0.035) : 0.0f;
    rot = (float)((age + hoverStart) * 57.29578);
    block = ItemToBlock(item);
    if (block != BLOCK_AIR && ShouldRenderBlockItemAs3DV27(block) && texTerrain) {
        scale = ItemRenderV38_GetDroppedBlockScale(block);
        glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, texTerrain);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glPushMatrix();
        glTranslatef((float)x, (float)y + bob, (float)z);
        glRotatef(rot, 0.0f, 1.0f, 0.0f);
        glScalef(scale, scale, scale);
        for (copy = 0; copy < copies; copy++) {
            glPushMatrix();
            if (copy > 0) {
                offX = ItemRenderV38_CopyOffset(item, damage, copy, 0, 0.20f) / scale;
                offY = 0.0f;
                offZ = ItemRenderV38_CopyOffset(item, damage, copy, 2, 0.20f) / scale;
                glTranslatef(offX, offY, offZ);
            }
            DrawDroppedBlockCube(block);
            glPopMatrix();
        }
        glPopMatrix();
        glEnable(GL_CULL_FACE);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        return;
    }
    if (!ResolveItemDrawTileV27(item, &col, &row, &useTerrain)) { return; }
    tex = useTerrain ? texTerrain : texBetaItems;
    atlasW = useTerrain ? TERRAIN_ATLAS_WIDTH : ICONS_ATLAS_WIDTH;
    atlasH = useTerrain ? TERRAIN_ATLAS_HEIGHT : ICONS_ATLAS_HEIGHT;
    if (!tex) { return; }
    size = pickupFx ? 0.26f : 0.32f;
    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, tex);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.08f);
    glDisable(GL_CULL_FACE);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef((float)x, (float)y + bob, (float)z);
    glRotatef(180.0f - (float)yaw, 0.0f, 1.0f, 0.0f);
    for (copy = 0; copy < copies; copy++) {
        glPushMatrix();
        if (copy > 0) {
            offX = ItemRenderV38_CopyOffset(item, damage, copy, 0, 0.30f);
            offY = 0.0f;
            offZ = ItemRenderV38_CopyOffset(item, damage, copy, 2, 0.30f);
            glTranslatef(offX, offY, offZ);
        }
        DrawDroppedItemSpriteUVV31(tex, atlasW, atlasH, col, row, size);
        glPopMatrix();
    }
    glPopMatrix();
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_CULL_FACE);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawDroppedItem(DroppedItem *d)
{
    if (!d || !d->active) { return; }
    DrawDroppedItemShadow(d);
    DrawDroppedItemVisualV38(d->item, d->count, d->damage, d->age, d->hoverStart, d->x, d->y, d->z, 0);
}


void RenderPickupFxV38(void)
{
    int i;
    double p;
    double q;
    double tx;
    double ty;
    double tz;
    double rx;
    double ry;
    double rz;

    for (i = 0; i < MAX_PICKUP_FX_V38; i++) {
        if (!pickupFxV38[i].active) { continue; }
        p = pickupFxV38[i].age / pickupFxV38[i].duration;
        if (p < 0.0) { p = 0.0; }
        if (p > 1.0) { p = 1.0; }
        q = p * p;
        tx = playerX;
        ty = playerY + 1.15;
        tz = playerZ;
        rx = pickupFxV38[i].sx + (tx - pickupFxV38[i].sx) * q;
        ry = pickupFxV38[i].sy + (ty - pickupFxV38[i].sy) * q;
        rz = pickupFxV38[i].sz + (tz - pickupFxV38[i].sz) * q;
        DrawDroppedItemVisualV38(pickupFxV38[i].item, pickupFxV38[i].count, pickupFxV38[i].damage,
                                pickupFxV38[i].age * 10.0, pickupFxV38[i].hoverStart, rx, ry, rz, 1);
    }
}

void RenderDroppedItems(void)
{
    int i;
    double dx;
    double dz;
    double maxDist2;

    maxDist2 = (double)GetMobRenderDistanceBlocks() * (double)GetMobRenderDistanceBlocks();

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
    RenderPickupFxV38();
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
    float shade;
    GLuint tex;
    int heldItem;
    Mob dummy;

    shade = ApplyGammaBoost(0.65f + g_daySkyBrightness * 0.42f);
    tex = texMobPlayer;
    heldItem = itemRendererHeldItemV38;
    memset(&dummy, 0, sizeof(dummy));
    dummy.type = 0;
    dummy.vx = (double)g_playerWalkAmount;
    dummy.vz = 0.0;
    dummy.animWalk = handBob;

    glPushMatrix();
    glTranslatef((float)playerX, (float)playerY, (float)playerZ);
    glRotatef((float)-yaw, 0.0f, 1.0f, 0.0f);
    glDisable(GL_CULL_FACE);

    if (tex) {
        DrawJavaBipedModelV4(&dummy, tex, shade, 1.0f, 0, heldItem, (float)pitch, 0, 0.0f);
        RenderPlayerArmorLayersJavaV4(shade);
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
    RendererV33_DrawProfilerOverlay();
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


/* V35_SOUND_SYSTEM_ACCURACY: SoundPool-style helpers. */
int SoundFileExistsV35(const char *filename)
{
    FILE *f;
    if (!filename || filename[0] == '\0') { return 0; }
    f = fopen(filename, "rb");
    if (f) { fclose(f); return 1; }
    return 0;
}

int SoundResolveExistingPathV35(const char *filename, char *outPath)
{
    int len;
    if (!filename || !outPath) { return 0; }
    strcpy(outPath, filename);
    if (SoundFileExistsV35(outPath)) { return 1; }
    len = (int)strlen(filename);
    if (len > 4 && strcmp(filename + len - 4, ".mp3") == 0) {
        strcpy(outPath, filename);
        outPath[len - 3] = 'w'; outPath[len - 2] = 'a'; outPath[len - 1] = 'v';
        if (SoundFileExistsV35(outPath)) { return 1; }
    }
    return 0;
}

int SoundMciVolumeV35(double volume, int isMusic)
{
    int v;
    double scaled;
    scaled = volume;
    if (isMusic) { scaled *= g_musicVolumeV35; } else { scaled *= g_soundVolumeV35; }
    if (scaled < 0.0) { scaled = 0.0; }
    if (scaled > 1.0) { scaled = 1.0; }
    v = (int)(scaled * (double)SOUND_MAX_VOLUME_MCI_V35 + 0.5);
    if (v < 0) { v = 0; }
    if (v > SOUND_MAX_VOLUME_MCI_V35) { v = SOUND_MAX_VOLUME_MCI_V35; }
    return v;
}

double SoundRandUnitV35(void)
{
    unsigned int x;
    g_soundVariantSeedV35 = g_soundVariantSeedV35 * 1103515245 + 12345;
    x = (unsigned int)((g_soundVariantSeedV35 >> 16) & 32767);
    return (double)x / 32767.0;
}

double SoundRandomRangeV35(double minValue, double maxValue)
{
    return minValue + (maxValue - minValue) * SoundRandUnitV35();
}

void SoundApplyMciControlsV35(const char *aliasName, double volume, double pitch, int isMusic)
{
    char cmd[160];
    int vol;
    int speed;
    vol = SoundMciVolumeV35(volume, isMusic);
    wsprintf(cmd, "setaudio %s volume to %d", aliasName, vol);
    mciSendString(cmd, NULL, 0, NULL);
    if (pitch < 0.50) { pitch = 0.50; }
    if (pitch > 2.00) { pitch = 2.00; }
    speed = (int)(pitch * 1000.0 + 0.5);
    wsprintf(cmd, "set %s speed to %d", aliasName, speed);
    mciSendString(cmd, NULL, 0, NULL);
}

void PlaySoundFileExV35(const char *filename, double volume, double pitch, int isMusic)
{
    char aliasName[32]; char cmd[512]; char path[260]; const char *deviceType; int len; MCIERROR err;
    if (!SoundResolveExistingPathV35(filename, path)) { g_soundFailedThisFrameV35++; return; }
    if (!isMusic && g_soundVolumeV35 <= 0.0) { return; }
    if (isMusic && g_musicVolumeV35 <= 0.0) { return; }
    wsprintf(aliasName, "sndV35_%d", g_nextMobSoundAlias);
    g_nextMobSoundAlias++; if (g_nextMobSoundAlias >= SOUND_CHANNEL_COUNT_V35) { g_nextMobSoundAlias = 0; }
    wsprintf(cmd, "stop %s", aliasName); mciSendString(cmd, NULL, 0, NULL);
    wsprintf(cmd, "close %s", aliasName); mciSendString(cmd, NULL, 0, NULL);
    len = (int)strlen(path); deviceType = "mpegvideo";
    if (len > 4 && strcmp(path + len - 4, ".wav") == 0) { deviceType = "waveaudio"; }
    wsprintf(cmd, "open \"%s\" type %s alias %s", path, deviceType, aliasName);
    err = mciSendString(cmd, NULL, 0, NULL); if (err != 0) { g_soundFailedThisFrameV35++; return; }
    SoundApplyMciControlsV35(aliasName, volume, pitch, isMusic);
    wsprintf(cmd, "play %s", aliasName); err = mciSendString(cmd, NULL, 0, NULL);
    if (err != 0) { wsprintf(cmd, "close %s", aliasName); mciSendString(cmd, NULL, 0, NULL); g_soundFailedThisFrameV35++; return; }
    g_soundPlayedThisFrameV35++;
}

void PlaySoundAtV35(const char *filename, double x, double y, double z, double volume, double pitch, double maxDistance)
{
    double dx, dy, dz, dist, atten;
    dx = x - playerX; dy = y - (playerY + EYE_HEIGHT); dz = z - playerZ;
    dist = sqrt(dx * dx + dy * dy + dz * dz);
    if (maxDistance <= 0.0) { maxDistance = SOUND_DEFAULT_RANGE_V35; }
    if (dist > maxDistance) { return; }
    atten = 1.0 - (dist / maxDistance); if (atten < 0.0) { atten = 0.0; }
    volume *= (0.20 + atten * 0.80);
    PlaySoundFileExV35(filename, volume, pitch, 0);
}

void PlayRandomPathV35(const char *prefix, int minVariant, int maxVariant, const char *suffix, const char *fallback, double volume, double pitch)
{
    char path[260]; int count, first, variant, tries;
    if (!prefix || !suffix || maxVariant < minVariant) { PlaySoundFileExV35(fallback, volume, pitch, 0); return; }
    count = maxVariant - minVariant + 1; first = minVariant + (int)(SoundRandUnitV35() * (double)count); if (first > maxVariant) { first = maxVariant; }
    for (tries = 0; tries < count; tries++) { variant = minVariant + ((first - minVariant + tries) % count); wsprintf(path, "%s%d%s", prefix, variant, suffix); if (SoundFileExistsV35(path)) { PlaySoundFileExV35(path, volume * SoundRandomRangeV35(0.88, 1.08), pitch * SoundRandomRangeV35(0.94, 1.06), 0); return; } }
    PlaySoundFileExV35(fallback, volume, pitch, 0);
}

void PlayRandomPathAtV35(const char *prefix, int minVariant, int maxVariant, const char *suffix, const char *fallback, double x, double y, double z, double volume, double pitch, double maxDistance)
{
    char path[260]; int count, first, variant, tries;
    if (!prefix || !suffix || maxVariant < minVariant) { PlaySoundAtV35(fallback, x, y, z, volume, pitch, maxDistance); return; }
    count = maxVariant - minVariant + 1; first = minVariant + (int)(SoundRandUnitV35() * (double)count); if (first > maxVariant) { first = maxVariant; }
    for (tries = 0; tries < count; tries++) { variant = minVariant + ((first - minVariant + tries) % count); wsprintf(path, "%s%d%s", prefix, variant, suffix); if (SoundFileExistsV35(path)) { PlaySoundAtV35(path, x, y, z, volume * SoundRandomRangeV35(0.88, 1.08), pitch * SoundRandomRangeV35(0.94, 1.06), maxDistance); return; } }
    PlaySoundAtV35(fallback, x, y, z, volume, pitch, maxDistance);
}

const char *MobSoundDirV35(int type)
{
    if (type == MOB_CHICKEN) { return "chicken"; } if (type == MOB_COW) { return "cow"; } if (type == MOB_SHEEP) { return "sheep"; } if (type == MOB_PIG) { return "pig"; }
    if (type == MOB_WOLF) { return "wolf"; } if (type == MOB_ZOMBIE) { return "zombie"; } if (type == MOB_SKELETON) { return "skeleton"; } if (type == MOB_CREEPER) { return "creeper"; }
    if (type == MOB_SPIDER) { return "spider"; } if (type == MOB_SLIME) { return "slime"; } return "pig";
}

void PlayBowSoundV35(double x, double y, double z) { PlaySoundAtV35("assets\\sounds\\random\\bow.wav", x, y, z, 0.85, SoundRandomRangeV35(0.85, 1.15), SOUND_DEFAULT_RANGE_V35); }
void PlayDoorSoundV35(double x, double y, double z, int open) { if (open) { PlaySoundAtV35("assets\\sounds\\random\\door_open.wav", x, y, z, 0.75, SoundRandomRangeV35(0.90, 1.05), SOUND_DEFAULT_RANGE_V35); } else { PlaySoundAtV35("assets\\sounds\\random\\door_close.wav", x, y, z, 0.75, SoundRandomRangeV35(0.90, 1.05), SOUND_DEFAULT_RANGE_V35); } }
void PlayChestSoundV35(double x, double y, double z, int open) { if (open) { PlaySoundAtV35("assets\\sounds\\random\\chestopen.wav", x, y, z, 0.60, SoundRandomRangeV35(0.90, 1.10), SOUND_DEFAULT_RANGE_V35); } else { PlaySoundAtV35("assets\\sounds\\random\\chestclosed.wav", x, y, z, 0.60, SoundRandomRangeV35(0.90, 1.10), SOUND_DEFAULT_RANGE_V35); } }
void PlayFurnaceSoundV35(double x, double y, double z) { PlaySoundAtV35("assets\\sounds\\random\\fizz.wav", x, y, z, 0.35, SoundRandomRangeV35(0.85, 1.10), SOUND_DEFAULT_RANGE_V35); }

int PlayerNearBlockTypeV35(int blockType, int radius)
{
    int px, py, pz, x, y, z; px = (int)floor(playerX); py = (int)floor(playerY + 0.5); pz = (int)floor(playerZ);
    for (x = px - radius; x <= px + radius; x++) { for (y = py - 2; y <= py + 3; y++) { for (z = pz - radius; z <= pz + radius; z++) { if (!IsInsideWorld(x, y, z)) { continue; } if (GetBlock(x, y, z) == blockType) { return 1; } } } }
    return 0;
}

void UpdateAmbientSoundsV35(double dt)
{
    if (g_state != STATE_GAME) { return; }
    if (g_nextCaveAmbientTimeV35 > 0.0) { g_nextCaveAmbientTimeV35 -= dt; } if (g_waterAmbienceCooldownV35 > 0.0) { g_waterAmbienceCooldownV35 -= dt; } if (g_lavaAmbienceCooldownV35 > 0.0) { g_lavaAmbienceCooldownV35 -= dt; } if (g_fireAmbienceCooldownV35 > 0.0) { g_fireAmbienceCooldownV35 -= dt; }
    if (g_nextCaveAmbientTimeV35 <= 0.0) { int cy; cy = (int)floor(playerY); if (cy < 42 && GetLightLevel((int)floor(playerX), cy, (int)floor(playerZ)) < 5) { PlayRandomPathV35("assets\\sounds\\ambient\\cave\\cave", 1, 3, ".wav", "assets\\sounds\\ambient\\cave\\cave1.wav", 0.40, 1.0); } g_nextCaveAmbientTimeV35 = SOUND_CAVE_MIN_DELAY_V35 + SoundRandUnitV35() * SOUND_CAVE_RANDOM_DELAY_V35; }
    if (g_waterAmbienceCooldownV35 <= 0.0 && PlayerNearBlockTypeV35(BLOCK_WATER, 5)) { PlayRandomPathV35("assets\\sounds\\ambient\\water\\water", 1, 2, ".wav", "assets\\sounds\\ambient\\water\\water1.wav", 0.20, 1.0); g_waterAmbienceCooldownV35 = 4.5 + SoundRandUnitV35() * 4.0; }
    if (g_lavaAmbienceCooldownV35 <= 0.0 && (PlayerNearBlockTypeV35(BLOCK_LAVA, 6) || PlayerNearBlockTypeV35(BLOCK_STATIONARY_LAVA, 6))) { PlayRandomPathV35("assets\\sounds\\ambient\\lava\\lava", 1, 2, ".wav", "assets\\sounds\\ambient\\lava\\lava1.wav", 0.25, 0.85); g_lavaAmbienceCooldownV35 = 3.0 + SoundRandUnitV35() * 3.0; }
    if (g_fireAmbienceCooldownV35 <= 0.0 && PlayerNearBlockTypeV35(BLOCK_FIRE, 4)) { PlaySoundFileExV35("assets\\sounds\\fire\\fire.wav", 0.18, 1.0, 0); g_fireAmbienceCooldownV35 = 2.0 + SoundRandUnitV35() * 2.0; }
}

void StopMobSounds(void)
{
    int i; char aliasName[32]; char cmd[128];
    for (i = 0; i < SOUND_CHANNEL_COUNT_V35; i++) { wsprintf(aliasName, "sndV35_%d", i); wsprintf(cmd, "stop %s", aliasName); mciSendString(cmd, NULL, 0, NULL); wsprintf(cmd, "close %s", aliasName); mciSendString(cmd, NULL, 0, NULL); }
    mciSendString("stop waterAmbV35", NULL, 0, NULL); mciSendString("close waterAmbV35", NULL, 0, NULL);
    mciSendString("stop lavaAmbV35", NULL, 0, NULL); mciSendString("close lavaAmbV35", NULL, 0, NULL);
    mciSendString("stop fireAmbV35", NULL, 0, NULL); mciSendString("close fireAmbV35", NULL, 0, NULL);
    mciSendString("stop caveAmbV35", NULL, 0, NULL); mciSendString("close caveAmbV35", NULL, 0, NULL);
}


void PlayOneShotMP3(const char *filename)
{
    PlaySoundFileExV35(filename, 1.0, 1.0, 0);
}



void PlayUIClickSound(void)
{
    PlaySoundFileExV35(SOUND_UI_CLICK_FILE, 0.60, SoundRandomRangeV35(0.95, 1.05), 0);
}


void PlayPlayerHitSound(void)
{
    PlaySoundFileExV35(SOUND_PLAYER_HIT_FILE, 0.85, SoundRandomRangeV35(0.90, 1.05), 0);
}


void PlayItemPickupSound(void)
{
    if (g_worldTimeSeconds - g_lastPickupSoundTime < 0.04) { return; }
    g_lastPickupSoundTime = g_worldTimeSeconds;
    PlaySoundFileExV35(SOUND_PICKUP_FILE, 0.45, SoundRandomRangeV35(1.25, 1.70), 0);
}

/* V45_OPENWATCOM_PICKUP_LINK_FIX:
   V44 called PlayPickupSoundThrottled(x,y,z) from special entity pickup code,
   but only PlayItemPickupSound() existed.  Open Watcom therefore compiled with
   an implicit external and failed at link time.  Keep the Java-style pickup
   throttling and make the 3D version real instead of macro-removing the call. */
void PlayPickupSoundThrottled(double x, double y, double z)
{
    if (g_worldTimeSeconds - g_lastPickupSoundTime < 0.04) { return; }
    g_lastPickupSoundTime = g_worldTimeSeconds;
    PlaySoundAtV35(SOUND_PICKUP_FILE, x, y, z, 0.45, SoundRandomRangeV35(1.25, 1.70), SOUND_DEFAULT_RANGE_V35);
}


void PlayPlayerStepSound(int blockBelow)
{
    int group; const char *prefix; double pitch;
    group = GetBlockSoundGroup(blockBelow); prefix = "assets\\sounds\\player\\step_stone"; pitch = 1.0;
    if (group == SOUND_GROUP_WOOD_V35) { prefix = "assets\\sounds\\player\\step_wood"; pitch = 0.92; }
    else if (group == SOUND_GROUP_SAND_V35) { prefix = "assets\\sounds\\player\\step_sand"; pitch = 0.86; }
    else if (group == SOUND_GROUP_GRAVEL_V35) { prefix = "assets\\sounds\\player\\step_gravel"; pitch = 0.86; }
    else if (group == SOUND_GROUP_SNOW_V35) { prefix = "assets\\sounds\\player\\step_snow"; pitch = 0.78; }
    else if (group == SOUND_GROUP_CLOTH_V35) { prefix = "assets\\sounds\\player\\step_cloth"; }
    else if (group == SOUND_GROUP_GRASS_V35) { prefix = "assets\\sounds\\player\\step_grass"; }
    PlayRandomPathV35(prefix, 1, 4, ".mp3", "assets\\sounds\\player\\step_stone1.mp3", 0.55, pitch);
}



void PlayMobIdleSound(int type, int angry)
{
    char prefix[180]; char fallback[180]; const char *dir;
    dir = MobSoundDirV35(type);
    if (type == MOB_WOLF && angry) { wsprintf(prefix, "assets\\sounds\\mob\\%s\\growl", dir); wsprintf(fallback, "assets\\sounds\\mob\\%s\\growl1.mp3", dir); PlayRandomPathV35(prefix, 1, 3, ".mp3", fallback, 0.90, 0.95); return; }
    if (type == MOB_WOLF) { wsprintf(prefix, "assets\\sounds\\mob\\%s\\bark", dir); wsprintf(fallback, "assets\\sounds\\mob\\%s\\bark1.mp3", dir); }
    else if (type == MOB_SLIME) { wsprintf(prefix, "assets\\sounds\\mob\\%s\\small", dir); wsprintf(fallback, "assets\\sounds\\mob\\%s\\small1.mp3", dir); }
    else { wsprintf(prefix, "assets\\sounds\\mob\\%s\\say", dir); wsprintf(fallback, "assets\\sounds\\mob\\%s\\say1.mp3", dir); }
    PlayRandomPathV35(prefix, 1, 3, ".mp3", fallback, 0.75, 1.0);
}


void PlayMobHurtSound(int type)
{
    char prefix[180]; char fallback[180]; const char *dir;
    dir = MobSoundDirV35(type);
    if (type == MOB_CREEPER || type == MOB_SPIDER) { wsprintf(prefix, "assets\\sounds\\mob\\%s\\death", dir); wsprintf(fallback, "assets\\sounds\\mob\\%s\\death.mp3", dir); PlayRandomPathV35(prefix, 1, 2, ".mp3", fallback, 0.90, 0.90); return; }
    if (type == MOB_SHEEP) { PlaySoundFileExV35("assets\\sounds\\mob\\sheep\\shear.mp3", 0.85, SoundRandomRangeV35(0.95, 1.08), 0); return; }
    if (type == MOB_PIG) { wsprintf(prefix, "assets\\sounds\\mob\\%s\\say", dir); wsprintf(fallback, "assets\\sounds\\mob\\%s\\say1.mp3", dir); }
    else if (type == MOB_SLIME) { wsprintf(prefix, "assets\\sounds\\mob\\%s\\small", dir); wsprintf(fallback, "assets\\sounds\\mob\\%s\\small1.mp3", dir); }
    else { wsprintf(prefix, "assets\\sounds\\mob\\%s\\hurt", dir); wsprintf(fallback, "assets\\sounds\\mob\\%s\\hurt1.mp3", dir); }
    PlayRandomPathV35(prefix, 1, 3, ".mp3", fallback, 0.90, 1.0);
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
    char prefix[180]; char fallback[180]; const char *dir; double volume; double pitch;
    if (!m) { return; } if (blockBelow == BLOCK_WATER || m->type == MOB_SQUID) { return; } if (MobDistanceSquaredToPlayer(m) > MOB_HEAR_STEP_DISTANCE * MOB_HEAR_STEP_DISTANCE) { return; }
    dir = MobSoundDirV35(m->type); volume = 0.50; pitch = 1.0;
    if (m->type == MOB_CHICKEN) { volume = 0.35; pitch = 1.25; }
    else if (m->type == MOB_ZOMBIE || m->type == MOB_CREEPER) { dir = "zombie"; pitch = 0.90; }
    else if (m->type == MOB_SPIDER) { volume = 0.42; pitch = 1.10; }
    else if (m->type == MOB_SLIME) { PlaySoundAtV35("assets\\sounds\\mob\\slime\\small1.mp3", m->x, m->y, m->z, 0.48, SoundRandomRangeV35(0.90, 1.15), SOUND_STEP_RANGE_V35); return; }
    wsprintf(prefix, "assets\\sounds\\mob\\%s\\step", dir); wsprintf(fallback, "assets\\sounds\\mob\\%s\\step1.mp3", dir);
    PlayRandomPathAtV35(prefix, 1, 3, ".mp3", fallback, m->x, m->y, m->z, volume, pitch, SOUND_STEP_RANGE_V35);
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

    if (m->angry == 1) {
        return 1;
    }
    if (m->angry == 2) {
        return 0;
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
    int biome;

    if (below == BLOCK_WATER) {
        return MOB_SQUID;
    }

    gx = LocalToGlobalBlockX(x);
    gz = LocalToGlobalBlockZ(z);
    biome = GetBetaBiomeAt(gx, gz);
    r = WorldHash2D(gx, gz, g_worldSeed + 9100) % 100;

    /* V20: biome-weighted passive spawns so forests/plains/swamps do not all
       feel identical, while keeping the small MAX_MOBS cap safe for Win98. */
    if (biome == BIOME_TAIGA || biome == BIOME_FOREST || biome == BIOME_SEASONAL_FOREST) {
        if (r < 28) { return MOB_SHEEP; }
        if (r < 50) { return MOB_COW; }
        if (r < 70) { return MOB_PIG; }
        if (r < 86) { return MOB_CHICKEN; }
        return MOB_WOLF;
    }
    if (biome == BIOME_SWAMPLAND || biome == BIOME_RAINFOREST) {
        if (r < 25) { return MOB_PIG; }
        if (r < 48) { return MOB_CHICKEN; }
        if (r < 73) { return MOB_COW; }
        if (r < 92) { return MOB_SHEEP; }
        return MOB_WOLF;
    }
    if (biome == BIOME_DESERT || biome == BIOME_TUNDRA) {
        if (r < 36) { return MOB_CHICKEN; }
        if (r < 58) { return MOB_SHEEP; }
        if (r < 76) { return MOB_PIG; }
        return MOB_COW;
    }

    if (r < 30) { return MOB_SHEEP; }
    if (r < 55) { return MOB_COW; }
    if (r < 78) { return MOB_PIG; }
    if (r < 96) { return MOB_CHICKEN; }
    return MOB_WOLF;
}

int PickHostileMobType(int x, int y, int z, int underground)
{
    int r;
    int gx;
    int gz;
    int biome;

    gx = LocalToGlobalBlockX(x);
    gz = LocalToGlobalBlockZ(z);
    biome = GetBetaBiomeAt(gx, gz);
    r = WorldHash3D(gx, y, gz, g_worldSeed + 9300) % 100;

    if (underground && y < 38 && r > 90) { return MOB_SLIME; }
    if (biome == BIOME_DESERT && !underground) {
        if (r < 42) { return MOB_SKELETON; }
        if (r < 69) { return MOB_ZOMBIE; }
        if (r < 90) { return MOB_SPIDER; }
        return MOB_CREEPER;
    }
    if (biome == BIOME_SWAMPLAND || biome == BIOME_RAINFOREST) {
        if (r < 34) { return MOB_SPIDER; }
        if (r < 60) { return MOB_ZOMBIE; }
        if (r < 81) { return MOB_CREEPER; }
        return MOB_SKELETON;
    }

    if (r < 29) { return MOB_ZOMBIE; }
    if (r < 54) { return MOB_SKELETON; }
    if (r < 75) { return MOB_SPIDER; }
    if (r < 96) { return MOB_CREEPER; }
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
        if (m->angry == 2) { return texMobWolfTame ? texMobWolfTame : texMobWolf; }
        if (m->angry == 1) { return texMobWolfAngry ? texMobWolfAngry : texMobWolf; }
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
    g_mobSpawnTimer = 0.5;
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
            health = MobMaxHealthJavaV4(type);
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
            mobs[i].renderYawOffset = 0.0;
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
            mobs[i].hurtTime = 0.0;
            mobs[i].deathTime = 0.0;
            mobs[i].deathDropsDone = 0;
            mobs[i].prevX = x;
            mobs[i].prevY = y;
            mobs[i].prevZ = z;
            mobs[i].prevYaw = mobs[i].yaw;
            mobs[i].prevRenderYawOffset = mobs[i].renderYawOffset;
            mobs[i].prevAnimWalk = mobs[i].animWalk;
            mobs[i].targetX = 0.0;
            mobs[i].targetZ = 0.0;
            MobAI_ResetV17(&mobs[i]);
            MobInterp_SyncV32(&mobs[i]);
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
    for (i = 0; i < 30; i++) {
        if (CountMobGroup(0) < MOB_PASSIVE_CAP) {
            if (FindMobSpawnPoint(0, &x, &y, &z, &type)) {
                AddMob(type, (double)x + 0.5, (double)y, (double)z + 0.5);
            }
        }
    }

    for (i = 0; i < 18; i++) {
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


/* ------------------------------------------------------------ */
/* V53 Priority 5: deeper Java-style EntityCreature/Pathfinder  */
/* and SpawnerAnimals behavior.  This keeps the fixed-size C89   */
/* arrays required by Open Watcom while using Java-like concepts: */
/* target task selection, path target scoring, pack spawning,     */
/* panic/flee, daylight despawn/burning, skeleton ranged spacing, */
/* creeper swelling, spider leap, wolf/sheep/chicken specialties. */
/* ------------------------------------------------------------ */

int MobAIV53_GetAttackDamage(int type)
{
    if (type == MOB_ZOMBIE) { return 3; }
    if (type == MOB_SPIDER) { return 2; }
    if (type == MOB_SLIME) { return 2; }
    if (type == MOB_WOLF) { return 4; }
    if (type == MOB_CREEPER) { return 0; }
    return 2;
}

int MobAIV53_IsPassivePackType(int type)
{
    if (type == MOB_COW || type == MOB_SHEEP || type == MOB_PIG || type == MOB_CHICKEN) { return 1; }
    return 0;
}

int MobAIV53_IsDarkEnoughForHostile(int x, int y, int z)
{
    int sky;
    int light;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    sky = IsSkyOpenForSpawn(x, y, z);
    light = GetLegacyLightLevel(x, y, z);
    if (IsDaylightForMobs() && sky) { return 0; }
    if (light > 7 && !sky) { return 0; }
    return 1;
}

int MobAIV53_PathToGoal(Mob *m, double goalX, double goalY, double goalZ, double dt, double *outX, double *outZ)
{
    double directX;
    double directZ;
    double len;
    int ok;
    int bx;
    int by;
    int bz;
    int tx;
    int ty;
    int tz;
    if (!m || !outX || !outZ) { return 0; }
    ok = MobAI_GetCachedPathSteerV24(m, goalX, goalY, goalZ, dt, outX, outZ);
    if (ok && (*outX != 0.0 || *outZ != 0.0)) { return 1; }

    directX = goalX - m->x;
    directZ = goalZ - m->z;
    len = sqrt(directX * directX + directZ * directZ);
    if (len <= 0.001) { *outX = 0.0; *outZ = 0.0; return 0; }
    directX /= len;
    directZ /= len;

    bx = (int)floor(m->x);
    by = (int)floor(m->y);
    bz = (int)floor(m->z);
    tx = (int)floor(m->x + directX * 1.1);
    ty = by;
    tz = (int)floor(m->z + directZ * 1.1);
    if (MobAI_CanStandAtV24(m, tx, ty, tz) || MobAI_CanStandAtV24(m, tx, ty + 1, tz) || MobAI_CanStandAtV24(m, tx, ty - 1, tz)) {
        *outX = directX;
        *outZ = directZ;
        return 1;
    }

    /* Pathfinder fallback: rotate around the obstacle before declaring the path stuck. */
    tx = (int)floor(m->x - directZ * 1.1);
    tz = (int)floor(m->z + directX * 1.1);
    if (MobAI_CanStandAtV24(m, tx, by, tz) || MobAI_CanStandAtV24(m, tx, by + 1, tz)) {
        *outX = -directZ;
        *outZ = directX;
        return 1;
    }
    tx = (int)floor(m->x + directZ * 1.1);
    tz = (int)floor(m->z - directX * 1.1);
    if (MobAI_CanStandAtV24(m, tx, by, tz) || MobAI_CanStandAtV24(m, tx, by + 1, tz)) {
        *outX = directZ;
        *outZ = -directX;
        return 1;
    }

    (void)bx;
    (void)bz;
    *outX = 0.0;
    *outZ = 0.0;
    return 0;
}

int MobAIV53_SelectTargetKind(Mob *m, double dist2, double dt)
{
    int baseKind;
    int visible;
    int bx;
    int by;
    int bz;
    if (!m) { return MOB_TARGET_NONE; }

    baseKind = MobAI_SelectTargetKindV24(m, dist2, dt);
    bx = (int)floor(m->x);
    by = (int)floor(m->y + 0.5);
    bz = (int)floor(m->z);

    if (m->fleeTimer > 0.0 && IsPassiveMobType(m->type) && m->type != MOB_WOLF && m->type != MOB_SQUID) {
        return MOB_TARGET_FLEE_PLAYER;
    }

    if (MobAIV44_PlayerHoldsBreedingFood(m) && !IsHostileMobType(m->type) && m->fleeTimer <= 0.0 && dist2 < 100.0) {
        return MOB_TARGET_WANDER;
    }

    if (IsHostileMobType(m->type) || (m->type == MOB_WOLF && m->angry == 1)) {
        visible = MobAI_HasLineOfSightToPlayerV24(m);
        if (visible) {
            m->lastTargetVisible = 1;
            m->targetLostTimer = 0.0;
        } else {
            m->targetLostTimer += dt;
            if (m->targetLostTimer > 4.0 && dist2 > 49.0) { return MOB_TARGET_WANDER; }
        }

        if (m->type == MOB_SPIDER && IsDaylightForMobs() && IsSkyOpenForSpawn(bx, by, bz) && m->angry == 0) {
            return MOB_TARGET_WANDER;
        }
        if (m->type == MOB_SKELETON || m->type == MOB_ZOMBIE) {
            if (IsDaylightForMobs() && IsSkyOpenForSpawn(bx, by, bz) && dist2 > 25.0) {
                return MOB_TARGET_WANDER;
            }
        }
        if (dist2 < 18.0 * 18.0 || m->lastTargetVisible) { return MOB_TARGET_PLAYER; }
    }

    return baseKind;
}

void MobAIV53_OnMobDamaged(Mob *m, double knockX, double knockZ)
{
    int i;
    double dx;
    double dz;
    double d;
    double len;
    if (!m) { return; }
    MobAIV44_OnMobDamaged(m, knockX, knockZ);

    m->pathLength = 0;
    m->pathIndex = 0;
    m->pathRecalcTimer = 0.0;
    m->targetLostTimer = 0.0;
    m->lastTargetVisible = 1;
    m->hurtResistantTime = 0.28;

    len = sqrt(knockX * knockX + knockZ * knockZ);
    if (len < 0.001) {
        knockX = m->x - playerX;
        knockZ = m->z - playerZ;
        len = sqrt(knockX * knockX + knockZ * knockZ);
    }
    if (len > 0.001) {
        m->knockbackX = (knockX / len) * 7.0;
        m->knockbackY = 1.8;
        m->knockbackZ = (knockZ / len) * 7.0;
        m->knockbackTimer = 0.11;
    }

    if (IsPassiveMobType(m->type) && m->type != MOB_WOLF && m->type != MOB_SQUID) {
        m->fleeTimer = 9.0;
        m->targetX = knockX;
        m->targetZ = knockZ;
        for (i = 0; i < MAX_MOBS; i++) {
            if (!mobs[i].active || &mobs[i] == m) { continue; }
            if (!IsPassiveMobType(mobs[i].type) || mobs[i].type == MOB_SQUID) { continue; }
            dx = mobs[i].x - m->x;
            dz = mobs[i].z - m->z;
            d = dx * dx + dz * dz;
            if (d < 10.0 * 10.0) {
                mobs[i].fleeTimer = 4.0 + (double)(i & 3) * 0.35;
                mobs[i].targetX = dx;
                mobs[i].targetZ = dz;
                mobs[i].pathLength = 0;
                mobs[i].pathRecalcTimer = 0.0;
                mobs[i].thinkTimer = 0.0;
            }
        }
    }
    if (m->type == MOB_WOLF) {
        m->angry = 1;
        m->fleeTimer = 0.0;
    }
}

int MobAIV53_ShouldDespawn(Mob *m, double dist2, double dt)
{
    if (!m || m->persistent || m->angry == 2) { return 0; }
    if (m->spawnGraceTimer > 0.0) { return 0; }
    if (dist2 < 32.0 * 32.0) { m->despawnTimer = 0.0; return 0; }
    if (dist2 > 160.0 * 160.0) { return 1; }
    if (IsPassiveMobType(m->type) && dist2 < 96.0 * 96.0) { return 0; }
    m->despawnTimer += dt;
    if (dist2 > 72.0 * 72.0 && m->despawnTimer > 30.0) { return 1; }
    return 0;
}

void MobAIV53_FindPanicVector(Mob *m, double *outX, double *outZ)
{
    double dx;
    double dz;
    double len;
    double tryX;
    double tryZ;
    double bestX;
    double bestZ;
    int bx;
    int by;
    int bz;
    int k;
    if (!m || !outX || !outZ) { return; }
    dx = m->x - playerX;
    dz = m->z - playerZ;
    len = sqrt(dx * dx + dz * dz);
    if (len < 0.001) { dx = m->targetX; dz = m->targetZ; len = sqrt(dx * dx + dz * dz); }
    if (len < 0.001) { dx = 1.0; dz = 0.0; len = 1.0; }
    dx /= len; dz /= len;
    bestX = dx; bestZ = dz;
    by = (int)floor(m->y);
    for (k = 0; k < 7; k++) {
        double a;
        a = ((double)k - 3.0) * 0.45;
        tryX = dx * cos(a) - dz * sin(a);
        tryZ = dx * sin(a) + dz * cos(a);
        bx = (int)floor(m->x + tryX * 4.0);
        bz = (int)floor(m->z + tryZ * 4.0);
        if (MobAI_CanStandAtV24(m, bx, by, bz) || MobAI_CanStandAtV24(m, bx, by + 1, bz) || MobAI_CanStandAtV24(m, bx, by - 1, bz)) {
            bestX = tryX;
            bestZ = tryZ;
            break;
        }
    }
    *outX = bestX;
    *outZ = bestZ;
}

void MobAIV53_ApplyAdvancedBehavior(int index, Mob *m, int targetKind, double dist2, double dt, double *targetX, double *targetZ, double *speed)
{
    int other;
    double dx;
    double dz;
    double len;
    double sideX;
    double sideZ;
    double strafeSign;
    double visible;
    if (!m || !targetX || !targetZ || !speed) { return; }

    MobAIV44_ApplyAdvancedBehavior(index, m, targetKind, dist2, dt, targetX, targetZ, speed);

    visible = (double)MobAI_HasLineOfSightToPlayerV24(m);
    if (visible > 0.5) { m->lastTargetVisible = 1; m->targetLostTimer = 0.0; }

    if (targetKind == MOB_TARGET_FLEE_PLAYER && IsPassiveMobType(m->type) && m->type != MOB_SQUID) {
        MobAIV53_FindPanicVector(m, targetX, targetZ);
        *speed = 3.15;
        if (m->type == MOB_CHICKEN) { *speed = 2.75; }
        if (m->type == MOB_PIG || m->type == MOB_COW || m->type == MOB_SHEEP) { *speed = 3.05; }
        if (m->hurtTime > 0.0) { *speed += 0.35; }
        return;
    }

    if (MobAIV44_PlayerHoldsBreedingFood(m) && !IsHostileMobType(m->type) && m->fleeTimer <= 0.0 && dist2 < 100.0) {
        dx = playerX - m->x;
        dz = playerZ - m->z;
        len = sqrt(dx * dx + dz * dz);
        if (len > 0.001) {
            *targetX = dx / len;
            *targetZ = dz / len;
            *speed = (m->type == MOB_CHICKEN) ? 1.05 : 1.22;
            if (dist2 < 4.0) { *speed = 0.0; }
        }
    }

    if (m->type == MOB_SKELETON && targetKind == MOB_TARGET_PLAYER) {
        dx = playerX - m->x;
        dz = playerZ - m->z;
        len = sqrt(dx * dx + dz * dz);
        if (len > 0.001 && dist2 < 196.0 && m->lastTargetVisible) {
            if (dist2 < 24.0) {
                *targetX = -dx / len;
                *targetZ = -dz / len;
                *speed = 1.45;
            } else {
                strafeSign = ((WorldHash3D((int)m->x, (int)m->y, (int)m->z, g_worldSeed + index + (int)(g_worldTimeSeconds * 2.0)) & 1) ? 1.0 : -1.0);
                sideX = (-dz / len) * strafeSign;
                sideZ = ( dx / len) * strafeSign;
                *targetX = sideX * 0.72 + (dx / len) * 0.16;
                *targetZ = sideZ * 0.72 + (dz / len) * 0.16;
                *speed = 0.95;
            }
        }
    }

    if (m->type == MOB_CREEPER && targetKind == MOB_TARGET_PLAYER) {
        if (dist2 < 12.0) { *speed = 0.24; }
        else if (dist2 > 49.0 && m->fuseTimer > 0.0) { m->fuseTimer -= dt * 2.5; if (m->fuseTimer < 0.0) { m->fuseTimer = 0.0; } }
    }

    if (m->type == MOB_SPIDER && targetKind == MOB_TARGET_PLAYER && m->onGround && dist2 > 5.0 && dist2 < 80.0 && m->attackTimer <= 0.0) {
        dx = playerX - m->x;
        dz = playerZ - m->z;
        len = sqrt(dx * dx + dz * dz);
        if (len > 0.001 && m->lastTargetVisible) {
            m->vx += (dx / len) * 2.25;
            m->vz += (dz / len) * 2.25;
            m->vy = 5.35;
            m->attackTimer = 1.15;
        }
    }

    if (m->type == MOB_CHICKEN && !m->onGround && m->vy < -1.1) { m->vy *= 0.70; }

    if (m->type == MOB_WOLF && m->angry == 0) {
        other = MobAIV44_FindNearestMobType(MOB_SHEEP, m->x, m->y, m->z, 16.0, index);
        if (other >= 0) {
            dx = mobs[other].x - m->x;
            dz = mobs[other].z - m->z;
            len = sqrt(dx * dx + dz * dz);
            if (len > 0.001) { *targetX = dx / len; *targetZ = dz / len; *speed = 1.75; }
            if (len < 1.75 && m->attackTimer <= 0.0) { DamageMob(other, 3, dx, dz); m->attackTimer = 1.0; }
        }
    }
}

int MobAIV53_TrySpawnPack(int hostile)
{
    int x;
    int y;
    int z;
    int type;
    int count;
    int spawned;
    int tries;
    int px;
    int pz;
    int h;
    int idx;
    if (!FindMobSpawnPoint(hostile, &x, &y, &z, &type)) { return 0; }
    if (hostile && CountMobGroup(1) >= MOB_HOSTILE_CAP) { return 0; }
    if (!hostile && CountMobGroup(0) >= MOB_PASSIVE_CAP && type != MOB_SQUID) { return 0; }
    if (type == MOB_SQUID && CountMobGroup(2) >= MOB_WATER_CAP) { return 0; }

    count = 1;
    if (MobAIV53_IsPassivePackType(type)) { count = 2 + (WorldHash3D(x, y, z, g_worldSeed + type) & 2); }
    else if (hostile && type != MOB_CREEPER) { count = 1 + (WorldHash3D(x, y, z, g_worldSeed + type + 77) & 1); }

    spawned = 0;
    for (tries = 0; tries < count * 4 && spawned < count; tries++) {
        h = WorldHash3D(x + tries * 13, y + tries * 7, z - tries * 11, g_worldSeed + 5123 + type);
        px = x + ((h & 7) - 3);
        pz = z + (((h >> 5) & 7) - 3);
        if (!IsValidMobSpawnSpace(type, hostile, px, y, pz)) { continue; }
        if (hostile && !MobAIV53_IsDarkEnoughForHostile(px, y, pz)) { continue; }
        idx = AddMob(type, (double)px + 0.5, (double)y, (double)pz + 0.5);
        if (idx >= 0) {
            mobs[idx].spawnGraceTimer = 1.0;
            mobs[idx].persistent = 0;
            spawned++;
        }
        if (hostile && CountMobGroup(1) >= MOB_HOSTILE_CAP) { break; }
        if (!hostile && type != MOB_SQUID && CountMobGroup(0) >= MOB_PASSIVE_CAP) { break; }
    }
    return spawned;
}

void MobAIV53_RunSpawner(double dt)
{
    g_mobSpawnTimer -= dt;
    if (g_mobSpawnTimer > 0.0) { return; }
    g_mobSpawnTimer = 3.5 + (double)(WorldHash3D((int)playerX, (int)playerY, (int)playerZ, g_worldSeed + (int)(g_worldTimeSeconds * 13.0)) & 7) * 0.25;
    if (CountActiveMobs() >= MAX_MOBS - 3) { return; }

    if (IsDaylightForMobs() && CountMobGroup(0) < MOB_PASSIVE_CAP) {
        MobAIV53_TrySpawnPack(0);
    }
    if (CountMobGroup(1) < MOB_HOSTILE_CAP) {
        MobAIV53_TrySpawnPack(1);
    }
}

void DamageMob(int index, int amount, double knockX, double knockZ)
{
    Mob *m;
    double len;
    double awayX;
    double awayZ;
    if (index < 0 || index >= MAX_MOBS) {
        return;
    }
    m = &mobs[index];
    if (!m->active) {
        return;
    }

    if (amount > 0) { m->health -= amount; }
    m->hurtTime = 0.60;
    m->fleeTimer = 5.0;
    m->targetTimer = 0.0;
    m->thinkTimer = 0.0;
    m->pathLength = 0;
    m->pathIndex = 0;
    m->pathRecalcTimer = 0.0;

    len = sqrt(knockX * knockX + knockZ * knockZ);
    if (len < 0.001) {
        knockX = m->x - playerX;
        knockZ = m->z - playerZ;
        len = sqrt(knockX * knockX + knockZ * knockZ);
    }
    if (len > 0.001) {
        awayX = knockX / len;
        awayZ = knockZ / len;
        /* Java EntityLiving.knockBack halves current motion then adds a clear
           push away from the attacker.  Stronger passive panic fixes animals
           standing still after a hit. */
        m->vx *= 0.50;
        m->vy *= 0.50;
        m->vz *= 0.50;
        m->vx += awayX * 1.05;
        m->vz += awayZ * 1.05;
        m->vy += 0.42;
        if (m->vy > 0.42) { m->vy = 0.42; }
        m->targetX = awayX;
        m->targetZ = awayZ;
        if (IsPassiveMobType(m->type) && m->type != MOB_SQUID) {
            m->vx += awayX * 0.70;
            m->vz += awayZ * 0.70;
            m->thinkTimer = 0.85;
        }
    }
    if (m->type == MOB_WOLF) {
        m->angry = 1;
        m->fleeTimer = 0.0;
    }
    PlayMobHurtSoundNear(m);
    SpawnMobEffectParticles(m, BLOCK_STONE, 14);
    MobAIV53_OnMobDamaged(m, m->targetX, m->targetZ);
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
                if (TryMobInteractJavaV4(i)) { return 1; }
                DamageMob(i, ItemCombatV6_GetHeldAttackDamage(), mobs[i].x - playerX, mobs[i].z - playerZ);
                DamageHeldTool(ItemCombatV6_GetHeldAttackToolWear());
                return 1;
            }
        }
    }
    return 0;
}


/* ------------------------------------------------------------ */
/* V44 full mob AI/pathing behavior                             */
/* Java reference path: EntityCreature, Pathfinder, Path,        */
/* PathEntity, PathPoint, SpawnerAnimals and mob subclasses.     */
/* ------------------------------------------------------------ */
int MobAIV44_PlayerHoldsBreedingFood(Mob *m)
{
    int held;
    if (!m) { return 0; }
    held = GetHeldHotbarItem();
    if ((m->type == MOB_COW || m->type == MOB_SHEEP || m->type == MOB_PIG) && held == ITEM_WHEAT) { return 1; }
    if (m->type == MOB_CHICKEN && held == ITEM_SEEDS) { return 1; }
    if (m->type == MOB_WOLF && (held == ITEM_BONE || held == ITEM_PORK_RAW || held == ITEM_PORK_COOKED || held == ITEM_FISH_RAW)) { return 1; }
    return 0;
}

int MobAIV44_FindNearestMobType(int type, double x, double y, double z, double maxDist, int skip)
{
    int i; int best; double bestD; double dx; double dy; double dz; double d;
    best = -1; bestD = maxDist * maxDist;
    for (i = 0; i < MAX_MOBS; i++) { if (i == skip || !mobs[i].active || mobs[i].type != type) { continue; } dx = mobs[i].x - x; dy = mobs[i].y - y; dz = mobs[i].z - z; d = dx * dx + dy * dy + dz * dz; if (d < bestD) { bestD = d; best = i; } }
    return best;
}

void MobAIV44_OnMobDamaged(Mob *m, double knockX, double knockZ)
{
    int i; double dx; double dz; double d;
    if (!m) { return; }
    if (IsPassiveMobType(m->type) && m->type != MOB_WOLF && m->type != MOB_SQUID) {
        m->fleeTimer = 8.0; m->thinkTimer = 0.15; m->pathRecalcTimer = 0.0; m->targetX = knockX; m->targetZ = knockZ;
        for (i = 0; i < MAX_MOBS; i++) { if (!mobs[i].active || &mobs[i] == m) { continue; } if (mobs[i].type != m->type && !(IsPassiveMobType(mobs[i].type) && IsPassiveMobType(m->type))) { continue; } dx = mobs[i].x - m->x; dz = mobs[i].z - m->z; d = dx * dx + dz * dz; if (d < 64.0) { mobs[i].fleeTimer = 3.5; mobs[i].thinkTimer = 0.0; mobs[i].targetX = dx; mobs[i].targetZ = dz; } }
    }
    if (m->type == MOB_WOLF) { m->angry = 1; m->fleeTimer = 0.0; }
}

int MobAIV44_ShouldDespawn(Mob *m, double dist2, double dt)
{
    if (!m || m->persistent || m->deathTime > 0.0) { return 0; }
    if (!IsHostileMobType(m->type) && dist2 < 128.0 * 128.0) { return 0; }
    if (dist2 < 36.0 * 36.0) { m->despawnTimer = 0.0; return 0; }
    if (dist2 > 144.0 * 144.0) { return 1; }
    m->despawnTimer += dt;
    if (m->despawnTimer > 30.0 && dist2 > 72.0 * 72.0) { return 1; }
    return 0;
}

void MobAIV44_ApplyAdvancedBehavior(int index, Mob *m, int targetKind, double dist2, double dt, double *targetX, double *targetZ, double *speed)
{
    int other; double dx; double dz; double len; int daylight; int heldFollow;
    if (!m || !targetX || !targetZ || !speed) { return; }
    daylight = IsDaylightForMobs(); heldFollow = MobAIV44_PlayerHoldsBreedingFood(m);
    if (heldFollow && !IsHostileMobType(m->type) && dist2 < 64.0 && m->fleeTimer <= 0.0) { dx = playerX - m->x; dz = playerZ - m->z; len = sqrt(dx * dx + dz * dz); if (len > 0.001) { *targetX = dx / len; *targetZ = dz / len; *speed = (m->type == MOB_CHICKEN) ? 1.05 : 1.20; } }
    if (m->type == MOB_WOLF && m->angry && dist2 < 256.0) { dx = playerX - m->x; dz = playerZ - m->z; len = sqrt(dx * dx + dz * dz); if (len > 0.001) { *targetX = dx / len; *targetZ = dz / len; *speed = 2.85; } }
    else if (m->type == MOB_WOLF && m->angry == 0 && targetKind != MOB_TARGET_PLAYER) { other = MobAIV44_FindNearestMobType(MOB_SHEEP, m->x, m->y, m->z, 16.0, index); if (other >= 0) { dx = mobs[other].x - m->x; dz = mobs[other].z - m->z; len = sqrt(dx * dx + dz * dz); if (len > 0.001) { *targetX = dx / len; *targetZ = dz / len; *speed = 1.85; } if (len < 1.7 && m->attackTimer <= 0.0) { DamageMob(other, 2, dx, dz); m->attackTimer = 1.0; } } }
    if (m->type == MOB_CHICKEN && m->onGround && m->vy < -0.05) { m->vy *= 0.55; }
    if (m->type == MOB_CHICKEN && m->soundTimer < 1.0 && ((WorldHash3D((int)m->x, (int)m->y, (int)m->z, g_worldSeed + index + (int)g_worldTimeSeconds) & 127) == 0)) { AddDroppedItem(ITEM_EGG, 1, m->x, m->y + 0.45, m->z, 0.0, 0.65, 0.0); }
    if ((m->type == MOB_ZOMBIE || m->type == MOB_SKELETON) && daylight && !m->inWater) { m->burning = 1; m->fireTimer += dt; }
    if (m->type == MOB_CREEPER && targetKind == MOB_TARGET_PLAYER) { if (dist2 < 49.0) { *speed = 2.00; } if (dist2 > 64.0 && m->fuseTimer > 0.0) { m->fuseTimer -= dt * 2.0; if (m->fuseTimer < 0.0) { m->fuseTimer = 0.0; } } }
    if (m->type == MOB_SPIDER && daylight && targetKind == MOB_TARGET_PLAYER && dist2 > 12.0 && !m->angry) { *targetX = 0.0; *targetZ = 0.0; *speed = 0.0; }
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
    double oldX;
    double oldZ;
    double horSpeed;
    double maxSpeed;
    double stepDelay;
    double avoidX;
    double avoidZ;
    double goalX;
    double goalY;
    double goalZ;
    int targetKind;
    int testX;
    int testY;
    int testZ;
    Mob *m;

    /* V53: Java SpawnerAnimals-style pack spawn/cap logic. */
    MobAIV53_RunSpawner(dt);

    for (i = 0; i < MAX_MOBS; i++) {
        if (!mobs[i].active) {
            continue;
        }

        m = &mobs[i];

        dist2 = MobDistanceSquaredToPlayer(m);

        if (m->angry == 2) {
            m->despawnTimer = 0.0;
        } else if (dist2 > MOB_DESPAWN_DISTANCE * MOB_DESPAWN_DISTANCE) {
            m->despawnTimer += dt;
            if (m->despawnTimer > 2.5) {
                m->active = 0;
                continue;
            }
        } else if (dist2 > 48.0 * 48.0) {
            m->despawnTimer += dt * 0.10;
            if (m->despawnTimer > 70.0) {
                m->active = 0;
                continue;
            }
        } else {
            m->despawnTimer = 0.0;
        }

        if (!IsMobInsideLoadedWindow(m)) {
            MobInterp_SyncV32(m);
            continue;
        }

        if (dist2 > (double)(GetMobRenderDistanceBlocks() - 8) * (double)(GetMobRenderDistanceBlocks() - 8)) {
            /* Keep mobs persistent around saved/loaded chunks, but do not run heavy AI/pathing far away. */
            MobInterp_SyncV32(m);
            continue;
        }

        MobInterp_BeginStepV32(m);

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

        if (m->jumpDelay > 0.0) {
            m->jumpDelay -= dt;
            if (m->jumpDelay < 0.0) { m->jumpDelay = 0.0; }
        }
        if (m->hurtResistantTime > 0.0) {
            m->hurtResistantTime -= dt;
            if (m->hurtResistantTime < 0.0) { m->hurtResistantTime = 0.0; }
        }
        if (m->attackCooldown > 0.0) {
            m->attackCooldown -= dt;
            if (m->attackCooldown < 0.0) { m->attackCooldown = 0.0; }
        }
        if (m->knockbackTimer > 0.0) {
            m->knockbackTimer -= dt;
            m->vx += m->knockbackX * dt;
            m->vy += m->knockbackY * dt;
            m->vz += m->knockbackZ * dt;
            if (m->knockbackTimer <= 0.0) { m->knockbackX = 0.0; m->knockbackY = 0.0; m->knockbackZ = 0.0; }
        }
        m->entityAge += dt;

        MobAI_RefreshEntityStateV18(m);
        MobAI_ApplyLivingEnvironmentV18(m, dt);
        if (!m->active || m->deathTime > 0.0) { continue; }

        bx = (int)floor(m->x);
        by = (int)floor(m->y);
        bz = (int)floor(m->z);

        if (MobBurnsInDaylight(m->type) && IsDaylightForMobs() &&
            IsSkyOpenForSpawn(bx, by, bz)) {
            m->burning = 1;
            m->burnTimer += dt;
            if (m->burnTimer > 1.0) {
                m->burnTimer = 0.0;
                SpawnFlameParticlesV24(m->x, m->y + 0.85, m->z, 3);
                MobAI_DamageSelfV24(m, 2, BLOCK_LIGHT, playerX, playerZ);
                if (!m->active || m->deathTime > 0.0) {
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
        if (MobAIV53_ShouldDespawn(m, dist2, dt)) { m->active = 0; continue; }
        targetKind = MobAIV53_SelectTargetKind(m, dist2, dt);
        m->targetKind = targetKind;
        targetX = 0.0;
        targetZ = 0.0;
        goalX = playerX;
        goalY = playerY;
        goalZ = playerZ;
        speed = 1.05;

        mobOnGround = 0;
        blockBelow = GetBlock((int)floor(m->x), (int)floor(m->y - 0.06), (int)floor(m->z));
        if (IsSolidBlock(blockBelow)) {
            mobOnGround = 1;
        }
        m->onGround = mobOnGround;

        /* PATCH_F11_MOB_GUI: face the player while targeting, even if the mob
           is pausing to attack or backing up. This avoids sideways sliding. */
        if (targetKind == MOB_TARGET_PLAYER && dist2 > 0.0001) {
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
        } else if (targetKind == MOB_TARGET_PLAYER) {
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
                } else if (dist2 < 196.0) {
                    targetX = 0.0;
                    targetZ = 0.0;
                    speed = 0.0;
                    if (m->attackTimer <= 0.0 && m->lastTargetVisible) {
                        SpawnSkeletonArrowProjectile(m);
                        m->attackTimer = 1.45 + (double)(WorldHash3D(bx, by, bz, g_worldSeed + i) & 31) / 100.0;
                        m->attackCooldown = m->attackTimer;
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
                    MobAI_AttackPlayerV24(m, 2, MOB_AI_V24_MELEE_COOLDOWN);
                }
            }
        } else if (targetKind == MOB_TARGET_FLEE_PLAYER) {
            len = sqrt(dist2);
            if (len > 0.001) {
                targetX = -dx / len;
                targetZ = -dz / len;
            } else {
                targetX = m->targetX;
                targetZ = m->targetZ;
            }
            speed = 3.05;
            if (m->type == MOB_CHICKEN) {
                speed = 2.70;
            } else if (m->type == MOB_COW || m->type == MOB_SHEEP || m->type == MOB_PIG) {
                speed = 2.95;
            }
            if (m->hurtTime > 0.0) { speed += 0.35; }
        } else {
            if (m->thinkTimer <= 0.0 ||
                (m->targetX == 0.0 && m->targetZ == 0.0)) {
                wander = (double)(WorldHash3D(bx, by, bz, g_worldSeed + i * 31) % 6283) / 1000.0;
                m->targetX = sin(wander);
                m->targetZ = cos(wander);
                if ((WorldHash3D(bz, by, bx, g_worldSeed + i * 43) & 3) == 0) {
                    m->targetX = 0.0;
                    m->targetZ = 0.0;
                }
                m->thinkTimer = 1.5 + (double)(WorldHash3D(bz, by, bx, g_worldSeed + i * 17) % 300) / 100.0;
            }
            targetX = m->targetX;
            targetZ = m->targetZ;
            speed = 0.78;
            if (m->type == MOB_CHICKEN) { speed = 0.62; }
            else if (m->type == MOB_PIG || m->type == MOB_COW || m->type == MOB_SHEEP) { speed = 0.72; }
            else if (m->type == MOB_WOLF) { speed = 0.95; }
            else if (m->type == MOB_SPIDER) { speed = 1.05; }
            else if (m->type == MOB_SLIME) { speed = 0.55; }
        }

        MobAIV53_ApplyAdvancedBehavior(i, m, targetKind, dist2, dt, &targetX, &targetZ, &speed);

        if ((targetX != 0.0 || targetZ != 0.0) && m->type != MOB_SQUID) {
            testX = (int)floor(m->x + targetX * 0.85);
            testY = (int)floor(m->y);
            testZ = (int)floor(m->z + targetZ * 0.85);

            if (IsSolidBlock(GetBlock(testX, testY, testZ)) ||
