/* ------------------------------------------------------------ */
/* V52_PRIORITY4_SCHEDULED_TICKS                                 */
/* Java-inspired World.scheduleBlockUpdate / NextTickListEntry    */
/* ------------------------------------------------------------ */

int BlockV52_IsRedstoneTickBlock(int block)
{
    if (block == BLOCK_REDSTONE_WIRE || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) { return 1; }
    if (block == BLOCK_REPEATER_ON || block == BLOCK_REPEATER_OFF || block == BLOCK_STONE_BUTTON) { return 1; }
    if (block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE || block == BLOCK_LEVER) { return 1; }
    if (block == BLOCK_PISTON || block == BLOCK_PISTON_STICKY || block == BLOCK_PISTON_EXTENSION) { return 1; }
    return 0;
}

int BlockV52_IsPlantTickBlock(int block)
{
    if (block == BLOCK_CROPS || block == BLOCK_SAPLING || block == BLOCK_REED || block == BLOCK_CACTUS) { return 1; }
    if (block == BLOCK_LEAVES || block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH) { return 1; }
    return 0;
}

int BlockV52_GetTickKind(int block)
{
    if (block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { return SCHEDULED_TICK_V52_KIND_FLUID; }
    if (block == BLOCK_FIRE) { return SCHEDULED_TICK_V52_KIND_FIRE; }
    if (BlockV52_IsRedstoneTickBlock(block)) { return SCHEDULED_TICK_V52_KIND_REDSTONE; }
    if (BlockV52_IsPlantTickBlock(block) || block == BLOCK_FARMLAND || block == BLOCK_SNOW) { return SCHEDULED_TICK_V52_KIND_PLANT; }
    return SCHEDULED_TICK_V52_KIND_NORMAL;
}

int BlockV52_ShouldUseScheduledTick(int block)
{
    if (block == BLOCK_AIR || block == BLOCK_BORDER) { return 0; }
    if (block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA || block == BLOCK_FIRE) { return 1; }
    if (BlockV52_IsRedstoneTickBlock(block)) { return 1; }
    if (BlockV52_IsPlantTickBlock(block)) { return 1; }
    if (block == BLOCK_FARMLAND || block == BLOCK_SNOW) { return 1; }
    return 0;
}

int BlockV52_GetTickDelay(int block)
{
    if (block == BLOCK_WATER) { return 5; }
    if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { return 30; }
    if (block == BLOCK_FIRE) { return 30; }
    if (block == BLOCK_REDSTONE_WIRE || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) { return 2; }
    if (block == BLOCK_REPEATER_ON || block == BLOCK_REPEATER_OFF) { return 4; }
    if (block == BLOCK_STONE_BUTTON) { return 20; }
    if (block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE) { return 10; }
    if (block == BLOCK_PISTON || block == BLOCK_PISTON_STICKY || block == BLOCK_PISTON_EXTENSION) { return 2; }
    if (block == BLOCK_CROPS) { return 600; }
    if (block == BLOCK_SAPLING) { return 600; }
    if (block == BLOCK_REED || block == BLOCK_CACTUS) { return 400; }
    if (block == BLOCK_LEAVES) { return 80; }
    if (block == BLOCK_FARMLAND || block == BLOCK_SNOW) { return 80; }
    return 40;
}

void ScheduleBlockUpdateV52(int x, int y, int z, int block, int delayTicks)
{
    int i;
    int slot;
    int worst;
    double due;
    if (!IsInsideWorld(x, y, z)) { return; }
    if (block == BLOCK_AIR || block == BLOCK_BORDER) { return; }
    if (!BlockV52_ShouldUseScheduledTick(block)) { return; }
    if (delayTicks < 1) { delayTicks = 1; }
    due = g_worldTimeSeconds + (double)delayTicks * SCHEDULED_TICK_V52_SECOND;

    for (i = 0; i < SCHEDULED_TICK_V52_MAX; i++) {
        if (!g_scheduledTicksV52[i].active) { continue; }
        if (g_scheduledTicksV52[i].x == x && g_scheduledTicksV52[i].y == y && g_scheduledTicksV52[i].z == z && g_scheduledTicksV52[i].block == block) {
            if (due < g_scheduledTicksV52[i].dueTime) { g_scheduledTicksV52[i].dueTime = due; }
            g_scheduledTickDuplicatesV52++;
            return;
        }
    }

    slot = -1;
    for (i = 0; i < SCHEDULED_TICK_V52_MAX; i++) {
        if (!g_scheduledTicksV52[i].active) { slot = i; break; }
    }
    if (slot < 0) {
        worst = 0;
        for (i = 1; i < SCHEDULED_TICK_V52_MAX; i++) {
            if (g_scheduledTicksV52[i].dueTime > g_scheduledTicksV52[worst].dueTime) { worst = i; }
        }
        if (g_scheduledTicksV52[worst].dueTime <= due) { g_scheduledTickDroppedV52++; return; }
        slot = worst;
    } else {
        g_scheduledTickCountV52++;
    }

    g_scheduledTicksV52[slot].active = 1;
    g_scheduledTicksV52[slot].kind = (unsigned char)BlockV52_GetTickKind(block);
    g_scheduledTicksV52[slot].seq = (unsigned short)(g_scheduledTickWriteSeqV52++ & 65535);
    g_scheduledTicksV52[slot].x = (short)x;
    g_scheduledTicksV52[slot].y = (short)y;
    g_scheduledTicksV52[slot].z = (short)z;
    g_scheduledTicksV52[slot].block = (short)block;
    g_scheduledTicksV52[slot].dueTime = due;
    g_scheduledTickScheduledV52++;
}

void ScheduleNeighborBlockUpdatesV52(int x, int y, int z)
{
    int i;
    int nx;
    int ny;
    int nz;
    int b;
    static const int dx[6] = { 1, -1, 0, 0, 0, 0 };
    static const int dy[6] = { 0, 0, 1, -1, 0, 0 };
    static const int dz[6] = { 0, 0, 0, 0, 1, -1 };
    for (i = 0; i < 6; i++) {
        nx = x + dx[i];
        ny = y + dy[i];
        nz = z + dz[i];
        if (!IsInsideWorld(nx, ny, nz)) { continue; }
        b = GetBlock(nx, ny, nz);
        if (BlockV52_ShouldUseScheduledTick(b)) { ScheduleBlockUpdateV52(nx, ny, nz, b, BlockV52_GetTickDelay(b)); }
    }
}

void BlockV52_OnBlockChanged(int x, int y, int z, int oldBlock, int newBlock)
{
    (void)oldBlock;
    if (BlockV52_ShouldUseScheduledTick(newBlock)) { ScheduleBlockUpdateV52(x, y, z, newBlock, BlockV52_GetTickDelay(newBlock)); }
    ScheduleNeighborBlockUpdatesV52(x, y, z);
}

int BlockV52_HasNearbyLog(int x, int y, int z)
{
    int ix;
    int iy;
    int iz;
    int nx;
    int ny;
    int nz;
    for (ix = -4; ix <= 4; ix++) {
        for (iy = -4; iy <= 4; iy++) {
            for (iz = -4; iz <= 4; iz++) {
                nx = x + ix; ny = y + iy; nz = z + iz;
                if (!IsInsideWorld(nx, ny, nz)) { continue; }
                if (GetBlock(nx, ny, nz) == BLOCK_WOOD) { return 1; }
            }
        }
    }
    return 0;
}

int BlockV52_UpdatePlantTick(int x, int y, int z, int block)
{
    int meta;
    int h;
    int above;
    int height;
    int yy;
    if (!BlockV50_CanBlockStayAt(block, x, y, z)) {
        BlockV50_DropBlockAsItem(x, y, z, block);
        SetBlock(x, y, z, BLOCK_AIR);
        return 1;
    }
    if (block == BLOCK_CROPS) {
        meta = (int)g_blockMeta[x][y][z] & 7;
        if (meta < 7 && GetLightLevel(x, y + 1, z) >= 9) {
            h = WorldHash3D(x, y, z, g_worldSeed + (int)(g_worldTimeSeconds * 20.0));
            if ((h & 15) == 0) { g_blockMeta[x][y][z] = (unsigned char)(meta + 1); InvalidateTerrainChunkMeshAt(x, z); }
        }
        ScheduleBlockUpdateV52(x, y, z, block, BlockV52_GetTickDelay(block));
        return 1;
    }
    if (block == BLOCK_REED || block == BLOCK_CACTUS) {
        height = 1;
        for (yy = y - 1; yy >= 1 && GetBlock(x, yy, z) == block; yy--) { height++; }
        above = GetBlock(x, y + 1, z);
        if (height < 3 && above == BLOCK_AIR) {
            meta = (int)g_blockMeta[x][y][z] & 15;
            meta++;
            if (meta >= 15) { g_blockMeta[x][y][z] = 0; SetBlock(x, y + 1, z, block); }
            else { g_blockMeta[x][y][z] = (unsigned char)meta; }
            InvalidateTerrainChunkMeshAt(x, z);
        }
        ScheduleBlockUpdateV52(x, y, z, block, BlockV52_GetTickDelay(block));
        return 1;
    }
    if (block == BLOCK_LEAVES) {
        if (!BlockV52_HasNearbyLog(x, y, z)) {
            h = WorldHash3D(x, y, z, g_worldSeed + (int)(g_worldTimeSeconds * 20.0));
            if ((h & 31) == 0) { SetBlock(x, y, z, BLOCK_AIR); return 1; }
        }
        ScheduleBlockUpdateV52(x, y, z, block, BlockV52_GetTickDelay(block));
        return 1;
    }
    if (block == BLOCK_SNOW || block == BLOCK_FARMLAND || block == BLOCK_SAPLING || block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH) {
        ScheduleBlockUpdateV52(x, y, z, block, BlockV52_GetTickDelay(block));
        return 1;
    }
    return 0;
}

int BlockV52_UpdateRedstoneTick(int x, int y, int z, int block)
{
    int pressed;
    double dx;
    double dz;
    if (block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE) {
        dx = playerX - ((double)x + 0.5);
        dz = playerZ - ((double)z + 0.5);
        pressed = (fabs(dx) < 0.75 && fabs(dz) < 0.75 && fabs(playerY - (double)y) < 1.5) ? 15 : 0;
        if ((int)g_redstonePower[x][y][z] != pressed) {
            g_redstonePower[x][y][z] = (unsigned char)pressed;
            UpdateRedstoneAround(x, y, z);
        }
        ScheduleBlockUpdateV52(x, y, z, block, 10);
        return 1;
    }
    if (block == BLOCK_STONE_BUTTON) {
        if ((g_blockMeta[x][y][z] & 8) && g_redstonePower[x][y][z] > 0) {
            g_blockMeta[x][y][z] &= (unsigned char)7;
            g_redstonePower[x][y][z] = 0;
            UpdateRedstoneAround(x, y, z);
            InvalidateTerrainChunkMeshAt(x, z);
        }
        return 1;
    }
    UpdateRedstoneAround(x, y, z);
    if (block == BLOCK_REDSTONE_WIRE || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF || block == BLOCK_REPEATER_ON || block == BLOCK_REPEATER_OFF) {
        ScheduleBlockUpdateV52(x, y, z, block, BlockV52_GetTickDelay(block));
    }
    return 1;
}

int BlockV52_UpdateTickAt(int x, int y, int z, int scheduledBlock)
{
    int block;
    int changed;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    block = GetBlock(x, y, z);
    if (block == BLOCK_AIR || block == BLOCK_BORDER) { return 0; }
    if (scheduledBlock != block && !(scheduledBlock == BLOCK_STATIONARY_LAVA && block == BLOCK_LAVA)) {
        if (BlockV52_ShouldUseScheduledTick(block)) { ScheduleBlockUpdateV52(x, y, z, block, BlockV52_GetTickDelay(block)); }
        return 0;
    }
    if (block == BLOCK_WATER) {
        changed = FluidV42_UpdateOneLiquid(x, y, z, BLOCK_WATER);
        g_fluidChangesThisTickV42 += changed;
        if (GetBlock(x, y, z) == BLOCK_WATER) { ScheduleBlockUpdateV52(x, y, z, BLOCK_WATER, BlockV52_GetTickDelay(BLOCK_WATER)); }
        ScheduleNeighborBlockUpdatesV52(x, y, z);
        return 1;
    }
    if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) {
        LavaV42_TryIgniteNear(x, y, z);
        changed = FluidV42_UpdateOneLiquid(x, y, z, block);
        g_fluidChangesThisTickV42 += changed;
        if (BlockV42_IsLava(GetBlock(x, y, z))) { ScheduleBlockUpdateV52(x, y, z, GetBlock(x, y, z), BlockV52_GetTickDelay(block)); }
        ScheduleNeighborBlockUpdatesV52(x, y, z);
        return 1;
    }
    if (block == BLOCK_FIRE) {
        changed = FireV42_UpdateOneFire(x, y, z);
        g_fireChangesThisTickV42 += changed;
        if (GetBlock(x, y, z) == BLOCK_FIRE) { ScheduleBlockUpdateV52(x, y, z, BLOCK_FIRE, BlockV52_GetTickDelay(BLOCK_FIRE)); }
        ScheduleNeighborBlockUpdatesV52(x, y, z);
        return 1;
    }
    if (BlockV52_IsRedstoneTickBlock(block)) { return BlockV52_UpdateRedstoneTick(x, y, z, block); }
    if (BlockV52_IsPlantTickBlock(block) || block == BLOCK_FARMLAND || block == BLOCK_SNOW) { return BlockV52_UpdatePlantTick(x, y, z, block); }
    return 0;
}

void SeedScheduledTicksNearPlayerV52(int maxSeeds)
{
    int px;
    int py;
    int pz;
    int x0;
    int x1;
    int y0;
    int y1;
    int z0;
    int z1;
    int x;
    int y;
    int z;
    int block;
    int seeded;
    px = (int)floor(playerX);
    py = (int)floor(playerY);
    pz = (int)floor(playerZ);
    x0 = px - SCHEDULED_TICK_V52_SEED_RADIUS; if (x0 < 1) { x0 = 1; }
    x1 = px + SCHEDULED_TICK_V52_SEED_RADIUS; if (x1 > WORLD_X - 2) { x1 = WORLD_X - 2; }
    y0 = py - 20; if (y0 < 1) { y0 = 1; }
    y1 = py + 12; if (y1 > WORLD_Y - 2) { y1 = WORLD_Y - 2; }
    z0 = pz - SCHEDULED_TICK_V52_SEED_RADIUS; if (z0 < 1) { z0 = 1; }
    z1 = pz + SCHEDULED_TICK_V52_SEED_RADIUS; if (z1 > WORLD_Z - 2) { z1 = WORLD_Z - 2; }
    seeded = 0;
    for (y = y0; y <= y1 && seeded < maxSeeds; y++) {
        for (x = x0; x <= x1 && seeded < maxSeeds; x++) {
            for (z = z0; z <= z1 && seeded < maxSeeds; z++) {
                block = GetBlock(x, y, z);
                if (BlockV52_ShouldUseScheduledTick(block)) {
                    ScheduleBlockUpdateV52(x, y, z, block, 1 + (WorldHash3D(x, y, z, g_worldSeed + 5200) & 15));
                    seeded++;
                }
            }
        }
    }
    g_scheduledTickSeededV52 += seeded;
}

void ProcessScheduledBlockTicksV52(double dt, int maxTicks)
{
    int processed;
    int i;
    int best;
    double bestDue;
    ScheduledTickV52 tick;
    if (g_state != STATE_GAME) { return; }
    if (maxTicks <= 0) { maxTicks = SCHEDULED_TICK_V52_FRAME_BUDGET_SMALL; }
    if (dt > 0.20) { dt = 0.20; }
    g_scheduledTickSeedTimerV52 += dt;
    if (g_scheduledTickSeedTimerV52 >= 1.0 || g_scheduledTickCountV52 < 16) {
        g_scheduledTickSeedTimerV52 = 0.0;
        SeedScheduledTicksNearPlayerV52(SCHEDULED_TICK_V52_SEED_BUDGET);
    }

    g_fluidChangesThisTickV42 = 0;
    g_fireChangesThisTickV42 = 0;
    processed = 0;
    while (processed < maxTicks) {
        best = -1;
        bestDue = g_worldTimeSeconds + 999999.0;
        for (i = 0; i < SCHEDULED_TICK_V52_MAX; i++) {
            if (!g_scheduledTicksV52[i].active) { continue; }
            if (g_scheduledTicksV52[i].dueTime <= g_worldTimeSeconds && g_scheduledTicksV52[i].dueTime < bestDue) {
                best = i;
                bestDue = g_scheduledTicksV52[i].dueTime;
            }
        }
        if (best < 0) { break; }
        tick = g_scheduledTicksV52[best];
        g_scheduledTicksV52[best].active = 0;
        if (g_scheduledTickCountV52 > 0) { g_scheduledTickCountV52--; }
        BlockV52_UpdateTickAt((int)tick.x, (int)tick.y, (int)tick.z, (int)tick.block);
        processed++;
    }
    g_scheduledTickProcessedV52 += processed;
}
