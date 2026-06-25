/* ============================================================
   CloneMC V51 section: ENTITY / MOB AI CORE / PATHING / LIVING COLLISION HELPERS
   ============================================================ */

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


/* V32_MOB_SMOOTH: keep previous tick state and build a blended render copy.
   This mirrors the important part of Java RenderLiving: previous position/yaw
   are kept by the simulation tick, while rendering uses partialTicks to avoid
   visible 20 Hz stepping. */
void MobInterp_SyncV32(Mob *m)
{
    if (!m) { return; }
    m->prevX = m->x;
    m->prevY = m->y;
    m->prevZ = m->z;
    m->prevYaw = m->yaw;
    m->prevRenderYawOffset = m->renderYawOffset;
    m->prevAnimWalk = m->animWalk;
}

void MobInterp_BeginStepV32(Mob *m)
{
    if (!m) { return; }
    m->prevX = m->x;
    m->prevY = m->y;
    m->prevZ = m->z;
    m->prevYaw = m->yaw;
    m->prevRenderYawOffset = m->renderYawOffset;
    m->prevAnimWalk = m->animWalk;
}

double MobInterp_LerpV32(double a, double b, double t)
{
    return a + (b - a) * t;
}

double MobInterp_LerpAngleV32(double a, double b, double t)
{
    double d;
    d = b - a;
    while (d < -180.0) { d += 360.0; }
    while (d >= 180.0) { d -= 360.0; }
    return a + d * t;
}

void MobInterp_BuildRenderMobV32(Mob *src, Mob *dst, double alpha)
{
    double dx;
    double dy;
    double dz;
    if (!src || !dst) { return; }
    *dst = *src;
    if (alpha < 0.0) { alpha = 0.0; }
    if (alpha > 1.0) { alpha = 1.0; }

    dx = src->x - src->prevX;
    dy = src->y - src->prevY;
    dz = src->z - src->prevZ;
    if (dx * dx + dy * dy + dz * dz > 16.0) {
        dst->prevX = src->x;
        dst->prevY = src->y;
        dst->prevZ = src->z;
        dst->prevYaw = src->yaw;
        dst->prevRenderYawOffset = src->renderYawOffset;
        dst->prevAnimWalk = src->animWalk;
        return;
    }

    dst->x = MobInterp_LerpV32(src->prevX, src->x, alpha);
    dst->y = MobInterp_LerpV32(src->prevY, src->y, alpha);
    dst->z = MobInterp_LerpV32(src->prevZ, src->z, alpha);
    dst->yaw = MobInterp_LerpAngleV32(src->prevYaw, src->yaw, alpha);
    dst->renderYawOffset = MobInterp_LerpAngleV32(src->prevRenderYawOffset, src->renderYawOffset, alpha);
    dst->animWalk = MobInterp_LerpV32(src->prevAnimWalk, src->animWalk, alpha);
}


/* ENTITY_AI_PATHING_V17:
   Java-inspired Entity/EntityLiving/EntityCreature/PathEntity behavior port.
   This is runtime code, not a placeholder: mobs now carry a compact cached path,
   refresh water/lava state from their bounding boxes, repath when the target moves,
   jump over one-block obstacles with a cooldown, and use timed despawn logic. */
void MobAI_ResetV17(Mob *m)
{
    int i;
    if (!m) { return; }
    m->targetKind = MOB_TARGET_NONE;
    m->pathLength = 0;
    m->pathIndex = 0;
    for (i = 0; i < MOB_PATH_MAX_POINTS; i++) {
        m->pathNodeX[i] = 0;
        m->pathNodeY[i] = 0;
        m->pathNodeZ[i] = 0;
    }
    m->lastPathGoalX = -99999;
    m->lastPathGoalY = -99999;
    m->lastPathGoalZ = -99999;
    m->inWater = 0;
    m->inLava = 0;
    m->onGround = 0;
    m->pathRecalcTimer = 0.0;
    m->jumpDelay = 0.0;
    m->targetTimer = 0.0;
    m->despawnTimer = 0.0;
    m->stuckTimer = 0.0;
    m->lastMoveX = m->x;
    m->lastMoveZ = m->z;
    m->onLadder = 0;
    m->inWeb = 0;
    m->inFire = 0;
    m->fallDistance = 0.0;
    m->airTimer = MOB_AI_AIR_SECONDS;
    m->drownTimer = MOB_AI_DROWN_INTERVAL;
    m->fireTimer = 0.0;
    m->pushTimer = 0.0;
    m->hurtResistantTime = 0.0;
    m->attackCooldown = 0.0;
    m->targetLostTimer = 0.0;
    m->lineOfSightTimer = 0.0;
    m->lastTargetVisible = 0;
    m->pathFailTimer = 0.0;
    m->entityAge = 0.0;
    m->knockbackTimer = 0.0;
    m->knockbackX = 0.0;
    m->knockbackY = 0.0;
    m->knockbackZ = 0.0;
    m->prevX = m->x;
    m->prevY = m->y;
    m->prevZ = m->z;
    m->prevYaw = m->yaw;
    m->prevRenderYawOffset = m->renderYawOffset;
    m->prevAnimWalk = m->animWalk;
    m->persistent = 0;
}

int MobAI_IsWaterBlockV17(int block)
{
    if (block == BLOCK_WATER) { return 1; }
    return 0;
}

int MobAI_IsLavaBlockV17(int block)
{
    if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { return 1; }
    return 0;
}

void MobAI_RefreshFluidStateV17(Mob *m)
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
    double half;
    double height;

    if (!m) { return; }

    half = (double)MobWidth(m->type) * 0.50;
    height = (double)MobHeight(m->type);
    minX = (int)floor(m->x - half);
    maxX = (int)floor(m->x + half);
    minY = (int)floor(m->y);
    maxY = (int)floor(m->y + height * 0.85);
    minZ = (int)floor(m->z - half);
    maxZ = (int)floor(m->z + half);

    m->inWater = 0;
    m->inLava = 0;

    for (x = minX; x <= maxX; x++) {
        for (y = minY; y <= maxY; y++) {
            for (z = minZ; z <= maxZ; z++) {
                block = GetBlock(x, y, z);
                if (MobAI_IsWaterBlockV17(block)) { m->inWater = 1; }
                if (MobAI_IsLavaBlockV17(block)) { m->inLava = 1; }
            }
        }
    }
}

int MobAI_SelectTargetKindV17(Mob *m, double dist2)
{
    int bx;
    int by;
    int bz;
    int openSky;

    if (!m) { return MOB_TARGET_NONE; }

    if (m->fleeTimer > 0.0 && IsPassiveMobType(m->type)) {
        return MOB_TARGET_FLEE_PLAYER;
    }

    if (m->angry == 2) {
        return MOB_TARGET_NONE;
    }

    if (m->angry == 1) {
        if (dist2 < 48.0 * 48.0) { return MOB_TARGET_PLAYER; }
        return MOB_TARGET_WANDER;
    }

    if (m->type == MOB_ZOMBIE || m->type == MOB_CREEPER || m->type == MOB_SLIME) {
        if (dist2 < 32.0 * 32.0) { return MOB_TARGET_PLAYER; }
        return MOB_TARGET_WANDER;
    }

    if (m->type == MOB_SKELETON) {
        if (dist2 < 36.0 * 36.0) { return MOB_TARGET_PLAYER; }
        return MOB_TARGET_WANDER;
    }

    if (m->type == MOB_SPIDER) {
        bx = (int)floor(m->x);
        by = (int)floor(m->y);
        bz = (int)floor(m->z);
        openSky = IsSkyOpenForSpawn(bx, by, bz);
        if ((!IsDaylightForMobs() || !openSky) && dist2 < 32.0 * 32.0) {
            return MOB_TARGET_PLAYER;
        }
        return MOB_TARGET_WANDER;
    }

    return MOB_TARGET_WANDER;
}

int MobAI_AabbBlockedAtV17(int type, double x, double y, double z)
{
    int minX;
    int maxX;
    int minY;
    int maxY;
    int minZ;
    int maxZ;
    int ix;
    int iy;
    int iz;
    int block;
    double half;
    double height;

    if (type == MOB_SQUID) { return 0; }

    half = (double)MobWidth(type) * 0.50 - 0.02;
    if (half < 0.22) { half = 0.22; }
    height = (double)MobHeight(type) - 0.03;
    if (height < 0.50) { height = 0.50; }

    minX = (int)floor(x - half);
    maxX = (int)floor(x + half);
    minY = (int)floor(y + 0.02);
    maxY = (int)floor(y + height);
    minZ = (int)floor(z - half);
    maxZ = (int)floor(z + half);

    for (ix = minX; ix <= maxX; ix++) {
        for (iy = minY; iy <= maxY; iy++) {
            for (iz = minZ; iz <= maxZ; iz++) {
                block = GetBlock(ix, iy, iz);
                if (IsSolidBlock(block)) { return 1; }
                if (block == BLOCK_WEB && type != MOB_SPIDER) { return 1; }
            }
        }
    }

    return 0;
}

int MobAI_BuildPathV17(Mob *m, int goalX, int goalY, int goalZ)
{
    int startX;
    int startY;
    int startZ;
    int grid[PATH_GRID_SIZE][PATH_GRID_SIZE];
    int parent[PATH_GRID_SIZE][PATH_GRID_SIZE];
    int qx[PATH_GRID_SIZE * PATH_GRID_SIZE];
    int qz[PATH_GRID_SIZE * PATH_GRID_SIZE];
    int dirs[8][2];
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
    int traceX[MOB_PATH_MAX_POINTS];
    int traceY[MOB_PATH_MAX_POINTS];
    int traceZ[MOB_PATH_MAX_POINTS];
    int traceCount;
    int p;
    int rx;
    int rz;
    int i;

    if (!m) { return 0; }
    if (m->type == MOB_SQUID) { return 0; }

    startX = (int)floor(m->x);
    startY = (int)floor(m->y);
    startZ = (int)floor(m->z);

    if (goalX < startX - PATH_GRID_RADIUS) { goalX = startX - PATH_GRID_RADIUS; }
    if (goalX > startX + PATH_GRID_RADIUS) { goalX = startX + PATH_GRID_RADIUS; }
    if (goalZ < startZ - PATH_GRID_RADIUS) { goalZ = startZ - PATH_GRID_RADIUS; }
    if (goalZ > startZ + PATH_GRID_RADIUS) { goalZ = startZ + PATH_GRID_RADIUS; }

    dirs[0][0] = 1; dirs[0][1] = 0;
    dirs[1][0] = -1; dirs[1][1] = 0;
    dirs[2][0] = 0; dirs[2][1] = 1;
    dirs[3][0] = 0; dirs[3][1] = -1;
    dirs[4][0] = 1; dirs[4][1] = 1;
    dirs[5][0] = -1; dirs[5][1] = 1;
    dirs[6][0] = 1; dirs[6][1] = -1;
    dirs[7][0] = -1; dirs[7][1] = -1;

    for (cx = 0; cx < PATH_GRID_SIZE; cx++) {
        for (cz = 0; cz < PATH_GRID_SIZE; cz++) {
            grid[cx][cz] = 0;
            parent[cx][cz] = -1;
        }
    }

    head = 0;
    tail = 0;
    qx[tail] = PATH_GRID_RADIUS;
    qz[tail] = PATH_GRID_RADIUS;
    tail++;
    grid[PATH_GRID_RADIUS][PATH_GRID_RADIUS] = 1;
    bestX = PATH_GRID_RADIUS;
    bestZ = PATH_GRID_RADIUS;
    bestScore = 999999;

    while (head < tail) {
        cx = qx[head];
        cz = qz[head];
        head++;
        wx = startX + cx - PATH_GRID_RADIUS;
        wz = startZ + cz - PATH_GRID_RADIUS;
        score = abs(wx - goalX) + abs(wz - goalZ) + abs(startY - goalY) / 2 + grid[cx][cz] / 3;
        if (score < bestScore) {
            bestScore = score;
            bestX = cx;
            bestZ = cz;
        }
        if (wx == goalX && wz == goalZ) {
            bestX = cx;
            bestZ = cz;
            break;
        }
        for (dir = 0; dir < 8; dir++) {
            nx = cx + dirs[dir][0];
            nz = cz + dirs[dir][1];
            if (nx < 0 || nz < 0 || nx >= PATH_GRID_SIZE || nz >= PATH_GRID_SIZE) { continue; }
            if (grid[nx][nz]) { continue; }
            wx = startX + nx - PATH_GRID_RADIUS;
            wz = startZ + nz - PATH_GRID_RADIUS;
            sy = MobFindStepY(m->type, wx, startY, wz);
            if (sy < -1000) { continue; }
            grid[nx][nz] = grid[cx][cz] + 1;
            parent[nx][nz] = cz * PATH_GRID_SIZE + cx;
            qx[tail] = nx;
            qz[tail] = nz;
            tail++;
        }
    }

    traceCount = 0;
    cx = bestX;
    cz = bestZ;
    while ((cx != PATH_GRID_RADIUS || cz != PATH_GRID_RADIUS) && traceCount < MOB_PATH_MAX_POINTS) {
        wx = startX + cx - PATH_GRID_RADIUS;
        wz = startZ + cz - PATH_GRID_RADIUS;
        sy = MobFindStepY(m->type, wx, startY, wz);
        if (sy < -1000) { sy = startY; }
        traceX[traceCount] = wx;
        traceY[traceCount] = sy;
        traceZ[traceCount] = wz;
        traceCount++;
        p = parent[cx][cz];
        if (p < 0) { break; }
        rx = p % PATH_GRID_SIZE;
        rz = p / PATH_GRID_SIZE;
        cx = rx;
        cz = rz;
    }

    m->pathLength = 0;
    m->pathIndex = 0;
    for (i = traceCount - 1; i >= 0 && m->pathLength < MOB_PATH_MAX_POINTS; i--) {
        m->pathNodeX[m->pathLength] = traceX[i];
        m->pathNodeY[m->pathLength] = traceY[i];
        m->pathNodeZ[m->pathLength] = traceZ[i];
        m->pathLength++;
    }

    m->lastPathGoalX = goalX;
    m->lastPathGoalY = goalY;
    m->lastPathGoalZ = goalZ;
    m->pathRecalcTimer = MOB_AI_REPATH_FAR_SECONDS;

    if (m->pathLength <= 0) { return 0; }
    return 1;
}

int MobAI_GetCachedPathSteerV17(Mob *m, double goalX, double goalY, double goalZ, double dt, double *outX, double *outZ)
{
    int gx;
    int gy;
    int gz;
    int nodeX;
    int nodeY;
    int nodeZ;
    double dx;
    double dz;
    double len;
    double centerX;
    double centerZ;

    if (!m || !outX || !outZ) { return 0; }
    if (m->type == MOB_SQUID) { return 0; }

    gx = (int)floor(goalX);
    gy = (int)floor(goalY);
    gz = (int)floor(goalZ);

    if (m->pathRecalcTimer > 0.0) { m->pathRecalcTimer -= dt; }
    if (m->pathRecalcTimer < 0.0) { m->pathRecalcTimer = 0.0; }

    if (m->pathLength <= 0 || m->pathIndex >= m->pathLength ||
        m->pathRecalcTimer <= 0.0 ||
        abs(gx - m->lastPathGoalX) > 2 || abs(gz - m->lastPathGoalZ) > 2) {
        if (!MobAI_BuildPathV17(m, gx, gy, gz)) {
            MobPathSteer(m, *outX, *outZ, outX, outZ);
            return 0;
        }
        if (m->targetKind == MOB_TARGET_PLAYER) { m->pathRecalcTimer = MOB_AI_REPATH_NEAR_SECONDS; }
    }

    if (m->pathIndex < 0) { m->pathIndex = 0; }
    if (m->pathIndex >= m->pathLength) { return 0; }

    nodeX = m->pathNodeX[m->pathIndex];
    nodeY = m->pathNodeY[m->pathIndex];
    nodeZ = m->pathNodeZ[m->pathIndex];
    centerX = (double)nodeX + 0.5;
    centerZ = (double)nodeZ + 0.5;
    dx = centerX - m->x;
    dz = centerZ - m->z;
    len = sqrt(dx * dx + dz * dz);

    if (len < MOB_AI_CLOSE_NODE_DISTANCE) {
        m->pathIndex++;
        if (m->pathIndex >= m->pathLength) {
            m->pathLength = 0;
            return 0;
        }
        nodeX = m->pathNodeX[m->pathIndex];
        nodeY = m->pathNodeY[m->pathIndex];
        nodeZ = m->pathNodeZ[m->pathIndex];
        centerX = (double)nodeX + 0.5;
        centerZ = (double)nodeZ + 0.5;
        dx = centerX - m->x;
        dz = centerZ - m->z;
        len = sqrt(dx * dx + dz * dz);
    }

    if (nodeY > (int)floor(m->y) && m->onGround && m->jumpDelay <= 0.0) {
        m->vy = 4.9;
        m->jumpDelay = 0.35;
    }

    if (len > 0.001) {
        *outX = dx / len;
        *outZ = dz / len;
        return 1;
    }

    return 0;
}

void MobAI_TryJumpHelperV17(Mob *m, double dirX, double dirZ, int mobOnGround)
{
    int x;
    int y;
    int z;
    int stepX;
    int stepZ;

    if (!m || !mobOnGround || m->jumpDelay > 0.0) { return; }
    if (m->type == MOB_SQUID || m->type == MOB_CHICKEN) { return; }

    x = (int)floor(m->x);
    y = (int)floor(m->y);
    z = (int)floor(m->z);
    stepX = (int)floor(m->x + dirX * 0.80);
    stepZ = (int)floor(m->z + dirZ * 0.80);

    if ((IsSolidBlock(GetBlock(stepX, y, stepZ)) || IsSolidBlock(GetBlock(stepX, y + 1, stepZ))) &&
        !IsSolidBlock(GetBlock(stepX, y + 2, stepZ)) &&
        IsSolidBlock(GetBlock(stepX, y - 1, stepZ))) {
        m->vy = 4.8;
        m->jumpDelay = 0.45;
    }
}

void MobAI_ApplyFluidMotionV17(Mob *m, double dt)
{
    if (!m) { return; }

    if (m->inLava) {
        m->vx *= MOB_AI_LAVA_DRAG;
        m->vy *= MOB_AI_LAVA_DRAG;
        m->vz *= MOB_AI_LAVA_DRAG;
        if (m->vy < -1.0) { m->vy = -1.0; }
        m->vy += 1.30 * dt;
        m->burning = 1;
        m->burnTimer += dt;
        if (m->burnTimer > 0.85) {
            m->burnTimer = 0.0;
            m->health -= 3;
            SpawnMobEffectParticles(m, BLOCK_FIRE, 5);
            if (m->health <= 0) {
                KillMobRenderable(m);
            }
        }
        return;
    }

    if (m->inWater) {
        m->vx *= MOB_AI_WATER_DRAG;
        m->vz *= MOB_AI_WATER_DRAG;
        m->vy *= 0.74;
        m->vy -= GRAVITY * dt * 0.18;
        if (m->type != MOB_SQUID) {
            m->vy += 1.10 * dt;
        }
        if (m->vy < -2.0) { m->vy = -2.0; }
    }
}


/* ENTITY_LIVING_V18:
   This pass fills in the next layer from Entity.java/EntityLiving.java/
   EntityCreature.java.  It keeps the V17 path cache but adds Java-style living
   environment state: ladders, webs, fire, drowning, fall distance, entity push,
   and axis-ordered AABB collision resolution. */
int MobAI_FireImmuneV18(int type)
{
    /* No current overworld mob in this clone is truly fire-immune.  Keep the
       helper so later Nether-style mobs can opt in without changing callers. */
    (void)type;
    return 0;
}

int MobAI_IsLadderBlockV18(int block)
{
    if (block == BLOCK_LADDER) { return 1; }
    return 0;
}

int MobAI_IsWebBlockV18(int block)
{
    if (block == BLOCK_WEB) { return 1; }
    return 0;
}

int MobAI_IsFireBlockV18(int block)
{
    if (block == BLOCK_FIRE || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { return 1; }
    return 0;
}

void MobAI_RefreshEntityStateV18(Mob *m)
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
    double half;
    double height;

    if (!m) { return; }

    MobAI_RefreshFluidStateV17(m);

    half = (double)MobWidth(m->type) * 0.50;
    height = (double)MobHeight(m->type);
    minX = (int)floor(m->x - half);
    maxX = (int)floor(m->x + half);
    minY = (int)floor(m->y);
    maxY = (int)floor(m->y + height * 0.95);
    minZ = (int)floor(m->z - half);
    maxZ = (int)floor(m->z + half);

    m->onLadder = 0;
    m->inWeb = 0;
    m->inFire = 0;

    for (x = minX; x <= maxX; x++) {
        for (y = minY; y <= maxY; y++) {
            for (z = minZ; z <= maxZ; z++) {
                block = GetBlock(x, y, z);
                if (MobAI_IsLadderBlockV18(block)) { m->onLadder = 1; }
                if (MobAI_IsWebBlockV18(block)) { m->inWeb = 1; }
                if (MobAI_IsFireBlockV18(block)) { m->inFire = 1; }
            }
        }
    }
}

void MobAI_DamageSelfV18(Mob *m, int amount, int effectBlock)
{
    if (!m || !m->active || m->deathTime > 0.0) { return; }
    if (amount <= 0) { return; }
    m->health -= amount;
    m->hurtTime = 0.45;
    m->fleeTimer = 2.5;
    SpawnMobEffectParticles(m, effectBlock, 6);
    PlayMobHurtSoundNear(m);
    if (m->health <= 0) {
        KillMobRenderable(m);
    }
}

void MobAI_ApplyLivingEnvironmentV18(Mob *m, double dt)
{
    int headBlock;

    if (!m || !m->active) { return; }

    if (m->inWeb && m->type != MOB_SPIDER) {
        m->vx *= 0.25;
        m->vz *= 0.25;
        if (m->vy < -0.25) { m->vy = -0.25; }
        m->vy *= 0.30;
    }

    if (m->onLadder && m->type != MOB_SQUID) {
        if (m->vy < -0.15) { m->vy = -0.15; }
        if (m->vy > 0.25) { m->vy = 0.25; }
        m->fallDistance = 0.0;
    }

    if ((m->inFire || m->burning) && !MobAI_FireImmuneV18(m->type)) {
        m->fireTimer += dt;
        m->burning = 1;
        if (m->fireTimer >= MOB_AI_FIRE_TICK_SECONDS) {
            m->fireTimer = 0.0;
            SpawnFlameParticlesV24(m->x, m->y + 0.8, m->z, 2);
            MobAI_DamageSelfV18(m, 2, BLOCK_FIRE);
        }
    } else if (!m->inLava) {
        m->fireTimer = 0.0;
    }

    headBlock = GetBlock((int)floor(m->x), (int)floor(m->y + (double)MobHeight(m->type) * 0.85), (int)floor(m->z));
    if (headBlock == BLOCK_WATER && m->type != MOB_SQUID) {
        m->airTimer -= dt;
        if (m->airTimer <= 0.0) {
            m->drownTimer -= dt;
            if (m->drownTimer <= 0.0) {
                SpawnWaterBubbleParticles(m->x, m->y + 0.9, m->z, 3);
                MobAI_DamageSelfV18(m, 2, BLOCK_WATER);
                m->drownTimer = MOB_AI_DROWN_INTERVAL;
            }
        }
    } else {
        m->airTimer = MOB_AI_AIR_SECONDS;
        m->drownTimer = MOB_AI_DROWN_INTERVAL;
    }
}

void MobAI_ApplyEntityPushesV18(int index, Mob *m, double dt)
{
    int j;
    Mob *o;
    double dx;
    double dz;
    double dist2;
    double minDist;
    double len;
    double push;

    if (!m || !m->active || m->type == MOB_SQUID) { return; }

    for (j = 0; j < MAX_MOBS; j++) {
        if (j == index) { continue; }
        o = &mobs[j];
        if (!o->active || o->deathTime > 0.0 || o->type == MOB_SQUID) { continue; }
        if (fabs(o->y - m->y) > 1.25) { continue; }
        dx = m->x - o->x;
        dz = m->z - o->z;
        dist2 = dx * dx + dz * dz;
        minDist = ((double)MobWidth(m->type) + (double)MobWidth(o->type)) * 0.45;
        if (dist2 <= 0.0001 || dist2 > minDist * minDist) { continue; }
        len = sqrt(dist2);
        push = (minDist - len) * MOB_AI_PUSH_STRENGTH * dt;
        if (push > 0.08) { push = 0.08; }
        dx /= len;
        dz /= len;
        m->vx += dx * push;
        m->vz += dz * push;
        o->vx -= dx * push;
        o->vz -= dz * push;
    }
}

void MobAI_ResolveAxisSweepV18(Mob *m, double *newX, double *newY, double *newZ, int *mobOnGround)
{
    double oldX;
    double oldY;
    double oldZ;
    double tryX;
    double tryY;
    double tryZ;
    int landed;
    int damage;

    if (!m || !newX || !newY || !newZ || !mobOnGround) { return; }
    if (m->type == MOB_SQUID) { return; }

    oldX = m->x;
    oldY = m->y;
    oldZ = m->z;
    tryX = *newX;
    tryY = *newY;
    tryZ = *newZ;
    landed = 0;

    if (MobAI_AabbBlockedAtV17(m->type, tryX, oldY, oldZ)) {
        tryX = oldX;
        m->vx = 0.0;
    }

    if (MobAI_AabbBlockedAtV17(m->type, tryX, tryY, oldZ)) {
        if (tryY < oldY && m->vy <= 0.0) { landed = 1; }
        tryY = oldY;
        m->vy = 0.0;
    }

    if (MobAI_AabbBlockedAtV17(m->type, tryX, tryY, tryZ)) {
        tryZ = oldZ;
        m->vz = 0.0;
    }

    if (!m->inWater && !m->inLava && !m->onLadder && !m->inWeb) {
        if (!landed && m->vy < 0.0) {
            m->fallDistance += -m->vy * 0.055;
        }
        if (landed) {
            if (m->fallDistance > MOB_AI_FALL_SAFE_DISTANCE) {
                damage = (int)(m->fallDistance - MOB_AI_FALL_SAFE_DISTANCE + 0.5);
                if (damage > 0) { MobAI_DamageSelfV18(m, damage, BLOCK_STONE); }
            }
            m->fallDistance = 0.0;
        }
    } else {
        m->fallDistance = 0.0;
    }

    if (landed) { *mobOnGround = 1; }
    *newX = tryX;
    *newY = tryY;
    *newZ = tryZ;
}


/* ENTITY_AI_PATHING_V24:
   Deeper Java-style EntityLiving/EntityCreature/Pathfinder pass.
   This layer keeps the V17/V18 fixed-size data model but adds line-of-sight
   target reacquisition, weighted path node costs, safer step/water/lava/door
   decisions, hurt-resistant timers, attack cooldowns, knockback, improved
   entity push separation, and a tighter AxisAlignedBB-style sweep. */
int MobAI_IsDoorLikeBlockV24(int block)
{
    if (block == BLOCK_WOOD_DOOR || block == BLOCK_TRAPDOOR) { return 1; }
    if (block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL) { return 1; }
    return 0;
}

int MobAI_IsHazardBlockV24(int block)
{
    if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA || block == BLOCK_FIRE) { return 1; }
    if (block == BLOCK_CACTUS) { return 1; }
    return 0;
}

int MobAI_PathPenaltyV24(Mob *m, int x, int y, int z, int previousY)
{
    int block;
    int below;
    int cost;
    block = GetBlock(x, y, z);
    below = GetBlock(x, y - 1, z);
    cost = 0;
    if (MobAI_IsWaterBlockV17(block) || MobAI_IsWaterBlockV17(below)) {
        if (!m || (m->type != MOB_SQUID && m->type != MOB_CHICKEN)) { cost += MOB_AI_V24_PATH_WATER_PENALTY; }
    }
    if (MobAI_IsLavaBlockV17(block) || MobAI_IsLavaBlockV17(below) || MobAI_IsHazardBlockV24(block) || MobAI_IsHazardBlockV24(below)) {
        if (!m || !MobAI_FireImmuneV18(m->type)) { cost += MOB_AI_V24_PATH_LAVA_PENALTY; }
    }
    if (MobAI_IsDoorLikeBlockV24(block)) { cost += MOB_AI_V24_PATH_DOOR_PENALTY; }
    if (block == BLOCK_WEB && (!m || m->type != MOB_SPIDER)) { cost += 28; }
    if (y > previousY) { cost += MOB_AI_V24_PATH_STEP_PENALTY; }
    if (y < previousY) { cost += MOB_AI_V24_PATH_DROP_PENALTY; }
    return cost;
}

int MobAI_CanStandAtV24(Mob *m, int x, int y, int z)
{
    int block;
    int below;
    int head;
    block = GetBlock(x, y, z);
    below = GetBlock(x, y - 1, z);
    head = GetBlock(x, y + 1, z);
    if (!m) { return 0; }
    if (m->type == MOB_SQUID) {
        if (MobAI_IsWaterBlockV17(block) || MobAI_IsWaterBlockV17(head)) { return 1; }
        return 0;
    }
    if (IsSolidBlock(block) || IsSolidBlock(head)) { return 0; }
    if (block == BLOCK_WEB && m->type != MOB_SPIDER) { return 0; }
    if (!IsSolidBlock(below) && !MobAI_IsWaterBlockV17(below) && below != BLOCK_LADDER && below != BLOCK_FENCE) { return 0; }
    if (MobAI_IsHazardBlockV24(block) && !MobAI_FireImmuneV18(m->type)) { return 0; }
    return 1;
}

int MobAI_FindStepYV24(Mob *m, int wx, int startY, int wz)
{
    int dy;
    int y;
    if (!m) { return -9999; }
    for (dy = 1; dy >= -3; dy--) {
        y = startY + dy;
        if (y < 2 || y > WORLD_Y - 4) { continue; }
        if (MobAI_CanStandAtV24(m, wx, y, wz)) { return y; }
    }
    return -9999;
}

int MobAI_HasLineOfSightToPlayerV24(Mob *m)
{
    double sx;
    double sy;
    double sz;
    double ex;
    double ey;
    double ez;
    double dx;
    double dy;
    double dz;
    double len;
    double t;
    int steps;
    int i;
    int bx;
    int by;
    int bz;
    int block;
    if (!m) { return 0; }
    sx = m->x;
    sy = m->y + (double)MobHeight(m->type) * 0.72;
    sz = m->z;
    ex = playerX;
    ey = playerY + EYE_HEIGHT;
    ez = playerZ;
    dx = ex - sx;
    dy = ey - sy;
    dz = ez - sz;
    len = sqrt(dx * dx + dy * dy + dz * dz);
    if (len < 0.01) { return 1; }
    if (len > 42.0) { return 0; }
    steps = (int)(len * 3.0);
    if (steps < 3) { steps = 3; }
    if (steps > 96) { steps = 96; }
    for (i = 1; i < steps; i++) {
        t = (double)i / (double)steps;
        bx = (int)floor(sx + dx * t);
        by = (int)floor(sy + dy * t);
        bz = (int)floor(sz + dz * t);
        block = GetBlock(bx, by, bz);
        if (IsSolidBlock(block) && block != BLOCK_GLASS && block != BLOCK_LEAVES && block != BLOCK_ICE) { return 0; }
    }
    return 1;
}

int MobAI_SelectTargetKindV24(Mob *m, double dist2, double dt)
{
    int baseKind;
    int visible;
    if (!m) { return MOB_TARGET_NONE; }
    baseKind = MobAI_SelectTargetKindV17(m, dist2);
    m->lineOfSightTimer -= dt;
    if (m->lineOfSightTimer <= 0.0) {
        visible = MobAI_HasLineOfSightToPlayerV24(m);
        m->lastTargetVisible = visible;
        m->lineOfSightTimer = 0.20 + (double)((WorldHash3D((int)m->x, (int)m->y, (int)m->z, g_worldSeed + m->type) & 15)) / 100.0;
    }
    if (baseKind == MOB_TARGET_PLAYER) {
        if (m->lastTargetVisible) {
            m->targetLostTimer = 0.0;
            return MOB_TARGET_PLAYER;
        }
        m->targetLostTimer += dt;
        if (m->targetLostTimer < MOB_AI_V24_TARGET_LOST_SECONDS) { return MOB_TARGET_PLAYER; }
        return MOB_TARGET_WANDER;
    }
    if (baseKind == MOB_TARGET_FLEE_PLAYER) { return baseKind; }
    m->targetLostTimer = 0.0;
    return baseKind;
}

int MobAI_BuildPathV24(Mob *m, int goalX, int goalY, int goalZ)
{
    int startX;
    int startY;
    int startZ;
    int cost[PATH_GRID_SIZE][PATH_GRID_SIZE];
    int parent[PATH_GRID_SIZE][PATH_GRID_SIZE];
    int open[PATH_GRID_SIZE][PATH_GRID_SIZE];
    int dirs[8][2];
    int cx;
    int cz;
    int nx;
    int nz;
    int wx;
    int wz;
    int sy;
    int bestOpenX;
    int bestOpenZ;
    int bestOpenScore;
    int score;
    int bestX;
    int bestZ;
    int bestGoalScore;
    int dir;
    int loops;
    int stepCost;
    int newCost;
    int traceX[MOB_PATH_MAX_POINTS];
    int traceY[MOB_PATH_MAX_POINTS];
    int traceZ[MOB_PATH_MAX_POINTS];
    int traceCount;
    int p;
    int i;
    if (!m) { return 0; }
    if (m->type == MOB_SQUID) { return 0; }
    startX = (int)floor(m->x);
    startY = (int)floor(m->y);
    startZ = (int)floor(m->z);
    if (goalX < startX - PATH_GRID_RADIUS) { goalX = startX - PATH_GRID_RADIUS; }
    if (goalX > startX + PATH_GRID_RADIUS) { goalX = startX + PATH_GRID_RADIUS; }
    if (goalZ < startZ - PATH_GRID_RADIUS) { goalZ = startZ - PATH_GRID_RADIUS; }
    if (goalZ > startZ + PATH_GRID_RADIUS) { goalZ = startZ + PATH_GRID_RADIUS; }
    dirs[0][0] = 1; dirs[0][1] = 0;
    dirs[1][0] = -1; dirs[1][1] = 0;
    dirs[2][0] = 0; dirs[2][1] = 1;
    dirs[3][0] = 0; dirs[3][1] = -1;
    dirs[4][0] = 1; dirs[4][1] = 1;
    dirs[5][0] = -1; dirs[5][1] = 1;
    dirs[6][0] = 1; dirs[6][1] = -1;
    dirs[7][0] = -1; dirs[7][1] = -1;
    for (cx = 0; cx < PATH_GRID_SIZE; cx++) {
        for (cz = 0; cz < PATH_GRID_SIZE; cz++) {
            cost[cx][cz] = 999999;
            parent[cx][cz] = -1;
            open[cx][cz] = 0;
        }
    }
    cost[PATH_GRID_RADIUS][PATH_GRID_RADIUS] = 0;
    open[PATH_GRID_RADIUS][PATH_GRID_RADIUS] = 1;
    bestX = PATH_GRID_RADIUS;
    bestZ = PATH_GRID_RADIUS;
    bestGoalScore = 999999;
    for (loops = 0; loops < PATH_GRID_SIZE * PATH_GRID_SIZE; loops++) {
        bestOpenX = -1;
        bestOpenZ = -1;
        bestOpenScore = 999999;
        for (cx = 0; cx < PATH_GRID_SIZE; cx++) {
            for (cz = 0; cz < PATH_GRID_SIZE; cz++) {
                if (!open[cx][cz]) { continue; }
                wx = startX + cx - PATH_GRID_RADIUS;
                wz = startZ + cz - PATH_GRID_RADIUS;
                score = cost[cx][cz] + (abs(wx - goalX) + abs(wz - goalZ)) * 10;
                if (score < bestOpenScore) { bestOpenScore = score; bestOpenX = cx; bestOpenZ = cz; }
            }
        }
        if (bestOpenX < 0) { break; }
        cx = bestOpenX;
        cz = bestOpenZ;
        open[cx][cz] = 0;
        wx = startX + cx - PATH_GRID_RADIUS;
        wz = startZ + cz - PATH_GRID_RADIUS;
        score = (abs(wx - goalX) + abs(wz - goalZ)) * 10 + abs(startY - goalY) * 3 + cost[cx][cz];
        if (score < bestGoalScore) { bestGoalScore = score; bestX = cx; bestZ = cz; }
        if (wx == goalX && wz == goalZ) { bestX = cx; bestZ = cz; break; }
        for (dir = 0; dir < 8; dir++) {
            nx = cx + dirs[dir][0];
            nz = cz + dirs[dir][1];
            if (nx < 0 || nz < 0 || nx >= PATH_GRID_SIZE || nz >= PATH_GRID_SIZE) { continue; }
            wx = startX + nx - PATH_GRID_RADIUS;
            wz = startZ + nz - PATH_GRID_RADIUS;
            if (dir >= 4) {
                if (MobAI_AabbBlockedAtV17(m->type, (double)(startX + cx - PATH_GRID_RADIUS + dirs[dir][0]) + 0.5, (double)startY, (double)(startZ + cz - PATH_GRID_RADIUS) + 0.5)) { continue; }
                if (MobAI_AabbBlockedAtV17(m->type, (double)(startX + cx - PATH_GRID_RADIUS) + 0.5, (double)startY, (double)(startZ + cz - PATH_GRID_RADIUS + dirs[dir][1]) + 0.5)) { continue; }
            }
            sy = MobAI_FindStepYV24(m, wx, startY, wz);
            if (sy < -1000) { continue; }
            stepCost = (dir >= 4) ? 14 : 10;
            stepCost += MobAI_PathPenaltyV24(m, wx, sy, wz, startY);
            newCost = cost[cx][cz] + stepCost;
            if (newCost < cost[nx][nz]) {
                cost[nx][nz] = newCost;
                parent[nx][nz] = cz * PATH_GRID_SIZE + cx;
                open[nx][nz] = 1;
            }
        }
    }
    traceCount = 0;
    cx = bestX;
    cz = bestZ;
    while ((cx != PATH_GRID_RADIUS || cz != PATH_GRID_RADIUS) && traceCount < MOB_PATH_MAX_POINTS) {
        wx = startX + cx - PATH_GRID_RADIUS;
        wz = startZ + cz - PATH_GRID_RADIUS;
        sy = MobAI_FindStepYV24(m, wx, startY, wz);
        if (sy < -1000) { sy = startY; }
        traceX[traceCount] = wx;
        traceY[traceCount] = sy;
        traceZ[traceCount] = wz;
        p = parent[cx][cz];
        if (p < 0) { break; }
        cx = p % PATH_GRID_SIZE;
        cz = p / PATH_GRID_SIZE;
        traceCount++;
    }
    if (traceCount <= 0) { m->pathLength = 0; m->pathFailTimer = 1.0; return 0; }
    if (traceCount > MOB_PATH_MAX_POINTS) { traceCount = MOB_PATH_MAX_POINTS; }
    m->pathLength = traceCount;
    m->pathIndex = 0;
    for (i = 0; i < traceCount; i++) {
        p = traceCount - 1 - i;
        m->pathNodeX[i] = traceX[p];
        m->pathNodeY[i] = traceY[p];
        m->pathNodeZ[i] = traceZ[p];
    }
    m->lastPathGoalX = goalX;
    m->lastPathGoalY = goalY;
    m->lastPathGoalZ = goalZ;
    m->pathFailTimer = 0.0;
    return 1;
}

int MobAI_GetCachedPathSteerV24(Mob *m, double goalX, double goalY, double goalZ, double dt, double *outX, double *outZ)
{
    int gx;
    int gy;
    int gz;
    double nodeX;
    double nodeZ;
    double dx;
    double dz;
    double len;
    if (!m || !outX || !outZ) { return 0; }
    gx = (int)floor(goalX);
    gy = (int)floor(goalY);
    gz = (int)floor(goalZ);
    m->pathRecalcTimer -= dt;
    if (m->pathFailTimer > 0.0) { m->pathFailTimer -= dt; }
    if (m->pathLength <= 0 || m->pathIndex >= m->pathLength ||
        abs(gx - m->lastPathGoalX) > 2 || abs(gz - m->lastPathGoalZ) > 2 ||
        m->pathRecalcTimer <= 0.0) {
        if (m->pathFailTimer <= 0.0) {
            if (!MobAI_BuildPathV24(m, gx, gy, gz)) {
                MobPathSteer(m, goalX - m->x, goalZ - m->z, outX, outZ);
                m->pathRecalcTimer = MOB_AI_REPATH_FAR_SECONDS;
                return 0;
            }
        }
        m->pathRecalcTimer = (MobDistanceSquaredToPlayer(m) < 16.0 * 16.0) ? MOB_AI_REPATH_NEAR_SECONDS : MOB_AI_REPATH_FAR_SECONDS;
    }
    if (m->pathLength > 0 && m->pathIndex < m->pathLength) {
        nodeX = (double)m->pathNodeX[m->pathIndex] + 0.5;
        nodeZ = (double)m->pathNodeZ[m->pathIndex] + 0.5;
        dx = nodeX - m->x;
        dz = nodeZ - m->z;
        len = sqrt(dx * dx + dz * dz);
        if (len < MOB_AI_CLOSE_NODE_DISTANCE) {
            m->pathIndex++;
            if (m->pathIndex >= m->pathLength) { m->pathLength = 0; }
        }
        if (len > 0.001) { *outX = dx / len; *outZ = dz / len; return 1; }
    }
    MobPathSteer(m, goalX - m->x, goalZ - m->z, outX, outZ);
    return 0;
}

void MobAI_ApplyKnockbackV24(Mob *m, double srcX, double srcZ, double power, double up)
{
    double dx;
    double dz;
    double len;
    if (!m) { return; }
    dx = m->x - srcX;
    dz = m->z - srcZ;
    len = sqrt(dx * dx + dz * dz);
    if (len < 0.001) { dx = sin(g_worldTimeSeconds + (double)m->type); dz = cos(g_worldTimeSeconds + (double)m->type); len = sqrt(dx * dx + dz * dz); }
    dx /= len;
    dz /= len;
    m->knockbackX += dx * power;
    m->knockbackZ += dz * power;
    m->knockbackY += up;
    m->knockbackTimer = 0.22;
    m->vx += dx * power;
    m->vz += dz * power;
    if (m->vy < up) { m->vy = up; }
}

void MobAI_DamageSelfV24(Mob *m, int amount, int effectBlock, double srcX, double srcZ)
{
    if (!m || amount <= 0) { return; }
    if (m->hurtResistantTime > 0.0) { return; }
    m->health -= amount;
    m->hurtTime = 0.45;
    m->hurtResistantTime = MOB_AI_V24_HURT_RESIST_SECONDS;
    m->fleeTimer = 1.8;
    MobAI_ApplyKnockbackV24(m, srcX, srcZ, 1.15, 2.0);
    SpawnMobEffectParticles(m, effectBlock, 7);
    PlayMobHurtSoundNear(m);
    if (m->health <= 0) { KillMobRenderable(m); }
}

int MobAI_AttackPlayerV24(Mob *m, int amount, double cooldown)
{
    if (!m) { return 0; }
    if (m->attackCooldown > 0.0 || m->attackTimer > 0.0) { return 0; }
    if (!MobAI_HasLineOfSightToPlayerV24(m)) { return 0; }
    TakeDamage(amount);
    m->attackTimer = cooldown;
    m->attackCooldown = cooldown;
    m->animWalk += 0.8;
    SpawnMobEffectParticles(m, BLOCK_DIRT, 3);
    return 1;
}

void MobAI_ApplyEntityPushesV24(int index, Mob *m, double dt)
{
    int j;
    double dx;
    double dz;
    double dist2;
    double minDist;
    double push;
    double len;
    Mob *o;
    if (!m || m->deathTime > 0.0) { return; }
    for (j = 0; j < MAX_MOBS; j++) {
        if (j == index) { continue; }
        o = &mobs[j];
        if (!o->active || o->deathTime > 0.0) { continue; }
        dx = m->x - o->x;
        dz = m->z - o->z;
        dist2 = dx * dx + dz * dz;
        minDist = ((double)MobWidth(m->type) + (double)MobWidth(o->type)) * 0.42;
        if (dist2 > 0.0001 && dist2 < minDist * minDist) {
            len = sqrt(dist2);
            push = (minDist - len) * 0.55;
            dx /= len;
            dz /= len;
            m->vx += dx * push * dt * 4.0;
            m->vz += dz * push * dt * 4.0;
            o->vx -= dx * push * dt * 2.0;
            o->vz -= dz * push * dt * 2.0;
        }
    }
}

void MobAI_ResolveAxisSweepV24(Mob *m, double *newX, double *newY, double *newZ, int *mobOnGround)
{
    double oldX;
    double oldY;
    double oldZ;
    double tryX;
    double tryY;
    double tryZ;
    double stepY;
    int landed;
    int horizontalBlocked;
    int damage;
    if (!m || !newX || !newY || !newZ || !mobOnGround) { return; }
    oldX = m->x;
    oldY = m->y;
    oldZ = m->z;
    tryX = *newX;
    tryY = *newY;
    tryZ = *newZ;
    landed = 0;
    horizontalBlocked = 0;
    if (MobAI_AabbBlockedAtV17(m->type, tryX, oldY, oldZ)) { tryX = oldX; m->vx = 0.0; horizontalBlocked = 1; }
    if (MobAI_AabbBlockedAtV17(m->type, tryX, oldY, tryZ)) { tryZ = oldZ; m->vz = 0.0; horizontalBlocked = 1; }
    if (horizontalBlocked && *mobOnGround && m->jumpDelay <= 0.0) {
        stepY = oldY + 1.0;
        if (!MobAI_AabbBlockedAtV17(m->type, *newX, stepY, *newZ) && !MobAI_AabbBlockedAtV17(m->type, oldX, stepY, oldZ)) {
            tryX = *newX;
            tryY = stepY;
            tryZ = *newZ;
            m->jumpDelay = 0.28;
        }
    }
    if (MobAI_AabbBlockedAtV17(m->type, tryX, tryY, tryZ)) {
        if (m->vy < 0.0) {
            landed = 1;
            tryY = floor(oldY + 0.05);
            if (tryY < 2.0) { tryY = oldY; }
        } else {
            tryY = oldY;
        }
        m->vy = 0.0;
    }
    if (!landed && m->vy < -0.01) { m->fallDistance += -m->vy * 0.05; }
    if (landed) {
        if (m->fallDistance > MOB_AI_FALL_SAFE_DISTANCE) {
            damage = (int)(m->fallDistance - MOB_AI_FALL_SAFE_DISTANCE);
            if (damage > 0) { MobAI_DamageSelfV24(m, damage, BLOCK_STONE, oldX, oldZ); }
        }
        m->fallDistance = 0.0;
        *mobOnGround = 1;
    } else if (m->inWater || m->inLava || m->onLadder) {
        m->fallDistance = 0.0;
    }
    *newX = tryX;
    *newY = tryY;
    *newZ = tryZ;
}

