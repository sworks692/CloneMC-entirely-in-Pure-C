/* ============================================================
   CloneMC V51 section: RENDER / CHUNK QUEUES / SPECIAL ENTITIES / FIRST-PERSON ITEMS
   ============================================================ */

                else { AddDroppedItem(e->item, 1, e->x, e->y, e->z, 0.0, 1.0, 0.0); }
                e->active = 0;
            }
            continue;
        }
        if (e->type == ENTITY_V6_FISH_HOOK) {
            b = GetBlock((int)floor(e->x), (int)floor(e->y), (int)floor(e->z));
            if (b == BLOCK_WATER) {
                e->vx *= 0.75;
                e->vz *= 0.75;
                e->vy *= 0.18;
                if (e->meta == 0) {
                    e->meta = 1;
                    e->fuse = 3.0 + (double)(WorldHash3D((int)e->x, (int)e->y, (int)e->z, g_worldSeed + 47300) & 255) / 32.0;
                } else if (e->meta == 1) {
                    e->fuse -= dt;
                    if (e->fuse <= 0.0) {
                        e->meta = 2;
                        e->fuse = 1.35;
                        SpawnSplashParticlesV24(e->x, e->y, e->z, 8);
                        PlaySoundAtV35("assets\\sounds\\liquid\\splash.wav", e->x, e->y, e->z, 0.60, 1.15, SOUND_DEFAULT_RANGE_V35);
                    }
                } else if (e->meta == 2) {
                    e->fuse -= dt;
                    e->vy = -0.02;
                    if (e->fuse <= 0.0) {
                        e->meta = 1;
                        e->fuse = 2.0 + (double)(WorldHash3D((int)(e->x + e->age), (int)e->y, (int)e->z, g_worldSeed + 47301) & 127) / 28.0;
                    }
                }
            } else { e->vy -= GRAVITY * 0.45 * dt; }
            dx = playerX - e->x;
            dz = playerZ - e->z;
            if (dx * dx + dz * dz > 64.0) { e->active = 0; if (g_fishingHookIndexV6 == i) { g_fishingHookIndexV6 = -1; } continue; }
        } else {
            e->vy -= GRAVITY * 0.18 * dt;
        }
        e->x += e->vx * dt;
        e->y += e->vy * dt;
        e->z += e->vz * dt;
        bx = (int)floor(e->x);
        by = (int)floor(e->y);
        bz = (int)floor(e->z);
        if (!IsInsideWorld(bx, by, bz)) { e->active = 0; continue; }
        b = GetBlock(bx, by, bz);
        if (b != BLOCK_AIR && b != BLOCK_WATER && b != BLOCK_FIRE) {
            if (e->type == ENTITY_V6_ARROW) { AddDroppedItem(ITEM_ARROW, 1, e->x, e->y, e->z, 0.0, 0.7, 0.0); }
            else if (e->type == ENTITY_V6_FIREBALL) { SpawnExplosionV6(e->x, e->y, e->z, 3.0, 1); }
            e->active = 0;
            continue;
        }
        ItemCombatV6_CheckProjectileMobHit(e);
    }
}

void RenderItemCombatV6_Cube(double x, double y, double z, double sx, double sy, double sz, float r, float g, float b)
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(r, g, b);
    glPushMatrix();
    glTranslatef((float)x, (float)y, (float)z);
    glScalef((float)sx, (float)sy, (float)sz);
    glBegin(GL_QUADS);
    glVertex3f(-0.5f,-0.5f,-0.5f); glVertex3f(0.5f,-0.5f,-0.5f); glVertex3f(0.5f,0.5f,-0.5f); glVertex3f(-0.5f,0.5f,-0.5f);
    glVertex3f(0.5f,-0.5f,0.5f); glVertex3f(-0.5f,-0.5f,0.5f); glVertex3f(-0.5f,0.5f,0.5f); glVertex3f(0.5f,0.5f,0.5f);
    glVertex3f(-0.5f,-0.5f,0.5f); glVertex3f(-0.5f,-0.5f,-0.5f); glVertex3f(-0.5f,0.5f,-0.5f); glVertex3f(-0.5f,0.5f,0.5f);
    glVertex3f(0.5f,-0.5f,-0.5f); glVertex3f(0.5f,-0.5f,0.5f); glVertex3f(0.5f,0.5f,0.5f); glVertex3f(0.5f,0.5f,-0.5f);
    glVertex3f(-0.5f,0.5f,-0.5f); glVertex3f(0.5f,0.5f,-0.5f); glVertex3f(0.5f,0.5f,0.5f); glVertex3f(-0.5f,0.5f,0.5f);
    glVertex3f(-0.5f,-0.5f,0.5f); glVertex3f(0.5f,-0.5f,0.5f); glVertex3f(0.5f,-0.5f,-0.5f); glVertex3f(-0.5f,-0.5f,-0.5f);
    glEnd();
    glPopMatrix();
}

void RenderItemCombatV6(void)
{
    int i;
    SpecialEntityV6 *e;
    for (i = 0; i < MAX_SPECIAL_ENTITIES_V6; i++) {
        if (!g_specialEntitiesV6[i].active) { continue; }
        e = &g_specialEntitiesV6[i];
        if (e->type == ENTITY_V6_ARROW) { RenderItemCombatV6_Cube(e->x, e->y, e->z, 0.08, 0.08, 0.80, 0.85f, 0.75f, 0.55f); }
        else if (e->type == ENTITY_V6_EGG) { RenderItemCombatV6_Cube(e->x, e->y, e->z, 0.24, 0.30, 0.24, 0.95f, 0.95f, 0.80f); }
        else if (e->type == ENTITY_V6_SNOWBALL) { RenderItemCombatV6_Cube(e->x, e->y, e->z, 0.24, 0.24, 0.24, 1.0f, 1.0f, 1.0f); }
        else if (e->type == ENTITY_V6_FIREBALL) { RenderItemCombatV6_Cube(e->x, e->y, e->z, 0.36, 0.36, 0.36, 1.0f, 0.35f, 0.05f); }
        else if (e->type == ENTITY_V6_FISH_HOOK) { RenderItemCombatV6_Cube(e->x, e->y, e->z, 0.12, 0.12, 0.12, 0.25f, 0.25f, 0.25f); }
        else if (e->type == ENTITY_V6_BOAT) { RenderItemCombatV6_Cube(e->x, e->y, e->z, 1.20, 0.32, 1.80, 0.55f, 0.32f, 0.14f); }
        else if (e->type == ENTITY_V6_PAINTING) { RenderItemCombatV6_Cube(e->x, e->y, e->z, 1.00, 1.00, 0.08, 0.75f, 0.55f, 0.30f); }
        else if (e->type == ENTITY_V6_TNT) { RenderItemCombatV6_Cube(e->x, e->y, e->z, 0.85, 0.85, 0.85, 0.95f, 0.15f, 0.10f); }
        else if (e->type == ENTITY_V6_FALLING_BLOCK) { glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, texTerrain); glPushMatrix(); glTranslatef((float)e->x, (float)e->y, (float)e->z); glScalef(0.55f, 0.55f, 0.55f); DrawDroppedBlockCube(e->block); glPopMatrix(); }
        else if (e->type == ENTITY_V6_LIGHTNING) {
            glDisable(GL_TEXTURE_2D);
            glColor3f(0.75f, 0.85f, 1.0f);
            glBegin(GL_LINES);
            glVertex3f((float)e->x, (float)(WORLD_Y - 1), (float)e->z);
            glVertex3f((float)e->x, (float)e->y, (float)e->z);
            glEnd();
        }
    }
}

void InitMobProjectiles(void)
{
    int i;
    for (i = 0; i < MAX_MOB_PROJECTILES; i++) { mobProjectiles[i].active = 0; }
}

int SpawnSkeletonArrowProjectile(Mob *m)
{
    int i;
    double dx;
    double dy;
    double dz;
    double len;
    if (!m) { return -1; }
    dx = playerX - m->x;
    dy = (playerY + EYE_HEIGHT * 0.70) - (m->y + 1.35);
    dz = playerZ - m->z;
    len = sqrt(dx * dx + dy * dy + dz * dz);
    if (len < 0.001) { return -1; }
    dx /= len; dy /= len; dz /= len;
    for (i = 0; i < MAX_MOB_PROJECTILES; i++) {
        if (!mobProjectiles[i].active) {
            mobProjectiles[i].active = 1;
            mobProjectiles[i].type = ENTITY_V6_ARROW;
            mobProjectiles[i].shooterType = MOB_SKELETON;
            mobProjectiles[i].x = m->x;
            mobProjectiles[i].y = m->y + 1.35;
            mobProjectiles[i].z = m->z;
            mobProjectiles[i].vx = dx * 9.0;
            mobProjectiles[i].vy = dy * 9.0 + 0.8;
            mobProjectiles[i].vz = dz * 9.0;
            mobProjectiles[i].age = 0.0;
            mobProjectiles[i].life = 12.0;
            PlayOneShotMP3("assets\\sounds\\random\\bow.mp3");
            return i;
        }
    }
    return -1;
}

void UpdateMobProjectiles(double dt)
{
    int i;
    int bx;
    int by;
    int bz;
    double dx;
    double dy;
    double dz;
    MobProjectile *p;
    for (i = 0; i < MAX_MOB_PROJECTILES; i++) {
        if (!mobProjectiles[i].active) { continue; }
        p = &mobProjectiles[i];
        p->age += dt;
        if (p->age > p->life) { p->active = 0; continue; }
        p->vy -= GRAVITY * 0.16 * dt;
        p->x += p->vx * dt;
        p->y += p->vy * dt;
        p->z += p->vz * dt;
        bx = (int)floor(p->x);
        by = (int)floor(p->y);
        bz = (int)floor(p->z);
        if (!IsInsideWorld(bx, by, bz)) { p->active = 0; continue; }
        if (GetBlock(bx, by, bz) != BLOCK_AIR && GetBlock(bx, by, bz) != BLOCK_WATER) { p->active = 0; continue; }
        dx = p->x - playerX;
        dy = p->y - (playerY + EYE_HEIGHT * 0.55);
        dz = p->z - playerZ;
        if (dx * dx + dy * dy + dz * dz < 0.42) {
            TakeDamage(3);
            p->active = 0;
        }
    }
}

void RenderMobProjectiles(void)
{
    int i;
    for (i = 0; i < MAX_MOB_PROJECTILES; i++) {
        if (!mobProjectiles[i].active) { continue; }
        RenderItemCombatV6_Cube(mobProjectiles[i].x, mobProjectiles[i].y, mobProjectiles[i].z, 0.08, 0.08, 0.80, 0.9f, 0.85f, 0.65f);
    }
}



/* ------------------------------------------------------------ */
/* RENDER_PIPELINE_V33 implementation                           */
/* ------------------------------------------------------------ */

void RendererV47_NormalizePlane(int i)
{
    float x;
    float y;
    float z;
    float len;
    x = g_frustumPlanesV47[i][0];
    y = g_frustumPlanesV47[i][1];
    z = g_frustumPlanesV47[i][2];
    len = (float)sqrt((double)(x * x + y * y + z * z));
    if (len < 0.00001f) { len = 1.0f; }
    g_frustumPlanesV47[i][0] /= len;
    g_frustumPlanesV47[i][1] /= len;
    g_frustumPlanesV47[i][2] /= len;
    g_frustumPlanesV47[i][3] /= len;
}

void RendererV47_UpdateFrustumFromOpenGL(void)
{
    float proj[16];
    float modl[16];
    float clip[16];

    if (!g_renderV47UsePlaneFrustum) { g_frustumValidV47 = 0; return; }

    glGetFloatv(GL_PROJECTION_MATRIX, proj);
    glGetFloatv(GL_MODELVIEW_MATRIX, modl);

    clip[0] = modl[0] * proj[0] + modl[1] * proj[4] + modl[2] * proj[8] + modl[3] * proj[12];
    clip[1] = modl[0] * proj[1] + modl[1] * proj[5] + modl[2] * proj[9] + modl[3] * proj[13];
    clip[2] = modl[0] * proj[2] + modl[1] * proj[6] + modl[2] * proj[10] + modl[3] * proj[14];
    clip[3] = modl[0] * proj[3] + modl[1] * proj[7] + modl[2] * proj[11] + modl[3] * proj[15];
    clip[4] = modl[4] * proj[0] + modl[5] * proj[4] + modl[6] * proj[8] + modl[7] * proj[12];
    clip[5] = modl[4] * proj[1] + modl[5] * proj[5] + modl[6] * proj[9] + modl[7] * proj[13];
    clip[6] = modl[4] * proj[2] + modl[5] * proj[6] + modl[6] * proj[10] + modl[7] * proj[14];
    clip[7] = modl[4] * proj[3] + modl[5] * proj[7] + modl[6] * proj[11] + modl[7] * proj[15];
    clip[8] = modl[8] * proj[0] + modl[9] * proj[4] + modl[10] * proj[8] + modl[11] * proj[12];
    clip[9] = modl[8] * proj[1] + modl[9] * proj[5] + modl[10] * proj[9] + modl[11] * proj[13];
    clip[10] = modl[8] * proj[2] + modl[9] * proj[6] + modl[10] * proj[10] + modl[11] * proj[14];
    clip[11] = modl[8] * proj[3] + modl[9] * proj[7] + modl[10] * proj[11] + modl[11] * proj[15];
    clip[12] = modl[12] * proj[0] + modl[13] * proj[4] + modl[14] * proj[8] + modl[15] * proj[12];
    clip[13] = modl[12] * proj[1] + modl[13] * proj[5] + modl[14] * proj[9] + modl[15] * proj[13];
    clip[14] = modl[12] * proj[2] + modl[13] * proj[6] + modl[14] * proj[10] + modl[15] * proj[14];
    clip[15] = modl[12] * proj[3] + modl[13] * proj[7] + modl[14] * proj[11] + modl[15] * proj[15];

    g_frustumPlanesV47[0][0] = clip[3] - clip[0]; g_frustumPlanesV47[0][1] = clip[7] - clip[4]; g_frustumPlanesV47[0][2] = clip[11] - clip[8]; g_frustumPlanesV47[0][3] = clip[15] - clip[12];
    g_frustumPlanesV47[1][0] = clip[3] + clip[0]; g_frustumPlanesV47[1][1] = clip[7] + clip[4]; g_frustumPlanesV47[1][2] = clip[11] + clip[8]; g_frustumPlanesV47[1][3] = clip[15] + clip[12];
    g_frustumPlanesV47[2][0] = clip[3] + clip[1]; g_frustumPlanesV47[2][1] = clip[7] + clip[5]; g_frustumPlanesV47[2][2] = clip[11] + clip[9]; g_frustumPlanesV47[2][3] = clip[15] + clip[13];
    g_frustumPlanesV47[3][0] = clip[3] - clip[1]; g_frustumPlanesV47[3][1] = clip[7] - clip[5]; g_frustumPlanesV47[3][2] = clip[11] - clip[9]; g_frustumPlanesV47[3][3] = clip[15] - clip[13];
    g_frustumPlanesV47[4][0] = clip[3] - clip[2]; g_frustumPlanesV47[4][1] = clip[7] - clip[6]; g_frustumPlanesV47[4][2] = clip[11] - clip[10]; g_frustumPlanesV47[4][3] = clip[15] - clip[14];
    g_frustumPlanesV47[5][0] = clip[3] + clip[2]; g_frustumPlanesV47[5][1] = clip[7] + clip[6]; g_frustumPlanesV47[5][2] = clip[11] + clip[10]; g_frustumPlanesV47[5][3] = clip[15] + clip[14];
    RendererV47_NormalizePlane(0); RendererV47_NormalizePlane(1); RendererV47_NormalizePlane(2);
    RendererV47_NormalizePlane(3); RendererV47_NormalizePlane(4); RendererV47_NormalizePlane(5);
    g_frustumValidV47 = 1;
}

int RendererV47_IsAABBInFrustum(double minX, double minY, double minZ, double maxX, double maxY, double maxZ)
{
    int i;
    float *p;
    if (!g_frustumValidV47) { return 1; }
    for (i = 0; i < 6; i++) {
        p = g_frustumPlanesV47[i];
        if (p[0] * minX + p[1] * minY + p[2] * minZ + p[3] <= 0.0 &&
            p[0] * maxX + p[1] * minY + p[2] * minZ + p[3] <= 0.0 &&
            p[0] * minX + p[1] * maxY + p[2] * minZ + p[3] <= 0.0 &&
            p[0] * maxX + p[1] * maxY + p[2] * minZ + p[3] <= 0.0 &&
            p[0] * minX + p[1] * minY + p[2] * maxZ + p[3] <= 0.0 &&
            p[0] * maxX + p[1] * minY + p[2] * maxZ + p[3] <= 0.0 &&
            p[0] * minX + p[1] * maxY + p[2] * maxZ + p[3] <= 0.0 &&
            p[0] * maxX + p[1] * maxY + p[2] * maxZ + p[3] <= 0.0) {
            return 0;
        }
    }
    return 1;
}

int RendererV47_IsChunkVisibleAABB(int cx, int cz)
{
    double minX;
    double maxX;
    double minZ;
    double maxZ;
    double pad;
    if (cx < 0 || cx >= WORLD_CHUNKS_X || cz < 0 || cz >= WORLD_CHUNKS_Z) { return 0; }
    pad = 4.0;
    minX = (double)(cx * CHUNK_SIZE) - pad;
    maxX = (double)((cx + 1) * CHUNK_SIZE) + pad;
    minZ = (double)(cz * CHUNK_SIZE) - pad;
    maxZ = (double)((cz + 1) * CHUNK_SIZE) + pad;
    return RendererV47_IsAABBInFrustum(minX, -pad, minZ, maxX, (double)WORLD_Y + pad, maxZ);
}

int RendererV47_TimeBudgetAllows(DWORD startMs, int builds, int budget)
{
    DWORD nowMs;
    if (builds <= 0) { return 1; }
    if (builds < budget && g_renderV47InitialBuildBoostFrames > 0) { return 1; }
    nowMs = GetTickCount();
    if ((int)(nowMs - startMs) <= g_renderV47MaxBuildMs) { return 1; }
    return 0;
}

void RendererV47_DrawFallbackChunkShell(int cx, int cz)
{
    int x;
    int z;
    int y;
    int minX;
    int maxX;
    int minZ;
    int maxZ;
    int topY;
    int block;
    int y0;

    minX = cx * CHUNK_SIZE;
    maxX = minX + CHUNK_SIZE - 1;
    minZ = cz * CHUNK_SIZE;
    maxZ = minZ + CHUNK_SIZE - 1;
    if (maxX >= WORLD_X) { maxX = WORLD_X - 1; }
    if (maxZ >= WORLD_Z) { maxZ = WORLD_Z - 1; }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    if (texTerrain) { TessellatorV8_BeginTerrain(texTerrain); }
    for (x = minX; x <= maxX; x++) {
        for (z = minZ; z <= maxZ; z++) {
            topY = columnTop[x][z];
            if (topY < 1) { continue; }
            if (topY >= WORLD_Y) { topY = WORLD_Y - 1; }
            y0 = topY - 2;
            if (y0 < 1) { y0 = 1; }
            for (y = y0; y <= topY; y++) {
                block = world[x][y][z];
                if (block == BLOCK_AIR) { continue; }
                if (RendererV8_IsTranslucentBlock(block)) { continue; }
                if (block == BLOCK_TORCH || IsSpecialBlockV5(block) || WorldGenV3_IsCrossPlantBlock(block)) { continue; }
                if (ShouldDrawFace(x, y + 1, z, block) || ShouldDrawFace(x, y, z - 1, block) ||
                    ShouldDrawFace(x, y, z + 1, block) || ShouldDrawFace(x - 1, y, z, block) ||
                    ShouldDrawFace(x + 1, y, z, block)) {
                    DrawBlock(x, y, z, block);
                }
            }
        }
    }
    if (g_tessellatorActiveV8) { TessellatorV8_End(); }
}

void BuildTerrainChunkTransMeshV47(int cx, int cz)
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
    DWORD buildStartMs;

    buildStartMs = GetTickCount();
    if (cx < 0 || cx >= WORLD_CHUNKS_X || cz < 0 || cz >= WORLD_CHUNKS_Z) { return; }
    if (!terrainChunkTransLists[cx][cz]) {
        terrainChunkTransLists[cx][cz] = glGenLists(1);
        if (!terrainChunkTransLists[cx][cz]) { return; }
    }

    minX = cx * CHUNK_SIZE;
    maxX = minX + CHUNK_SIZE - 1;
    minZ = cz * CHUNK_SIZE;
    maxZ = minZ + CHUNK_SIZE - 1;
    if (maxX >= WORLD_X) { maxX = WORLD_X - 1; }
    if (maxZ >= WORLD_Z) { maxZ = WORLD_Z - 1; }
    emitted = 0;

    glNewList(terrainChunkTransLists[cx][cz], GL_COMPILE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);

    for (x = minX; x <= maxX; x++) {
        for (z = minZ; z <= maxZ; z++) {
            topY = columnTop[x][z] + 3;
            if (topY < 1) { topY = 1; }
            if (topY >= WORLD_Y) { topY = WORLD_Y - 1; }
            startY = RendererV41_GetColumnStartY(cx, cz, topY);
            for (y = startY; y <= topY; y++) {
                block = world[x][y][z];
                if (block == BLOCK_AIR) { continue; }
                if (!RendererV8_IsTranslucentBlock(block)) { continue; }
                if (block == BLOCK_WATER) { glColor4f(0.85f, 0.90f, 1.0f, 0.66f); }
                else if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { glColor4f(1.0f, 0.75f, 0.35f, 0.78f); }
                else if (block == BLOCK_PORTAL || block == BLOCK_FIRE) { glColor4f(1.0f, 1.0f, 1.0f, 0.60f); }
                else { glColor4f(1.0f, 1.0f, 1.0f, 0.72f); }
                DrawBlock(x, y, z, block);
                emitted = 1;
                g_renderProfileV33.transparentBlocks++;
            }
        }
    }

    glEndList();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    terrainChunkTransDirty[cx][cz] = 0;
    terrainChunkSkipPassTransV47[cx][cz] = emitted ? 0 : 1;
    terrainChunkLastTransBuildFrameV47[cx][cz] = g_rendererFrameIdV33;
    g_renderProfileV33.meshBuildMs += (double)(GetTickCount() - buildStartMs);
}

void RendererV33_ResetFrameProfile(void)
{
    ZeroMemory(&g_renderProfileV33, sizeof(g_renderProfileV33));
}

void RendererV33_BeginFrame(void)
{
    int cx;
    int cz;
    RendererV33_ResetFrameProfile();
    g_renderOpaqueCountV33 = 0;
    g_renderTransCountV33 = 0;
    g_rendererFrameIdV33++;
    if (g_rendererFrameIdV33 == 0) { g_rendererFrameIdV33 = 1; }
    for (cx = 0; cx < WORLD_CHUNKS_X; cx++) {
        for (cz = 0; cz < WORLD_CHUNKS_Z; cz++) {
            terrainChunkWasVisibleV33[cx][cz] = 0;
        }
    }
}

int RendererV33_CompareNearToFar(const void *a, const void *b)
{
    const RendererV33ChunkEntry *aa;
    const RendererV33ChunkEntry *bb;
    aa = (const RendererV33ChunkEntry *)a;
    bb = (const RendererV33ChunkEntry *)b;
    if (aa->distSq < bb->distSq) { return -1; }
    if (aa->distSq > bb->distSq) { return 1; }
    if (aa->cx != bb->cx) { return aa->cx - bb->cx; }
    return aa->cz - bb->cz;
}

int RendererV33_CompareFarToNear(const void *a, const void *b)
{
    const RendererV33ChunkEntry *aa;
    const RendererV33ChunkEntry *bb;
    aa = (const RendererV33ChunkEntry *)a;
    bb = (const RendererV33ChunkEntry *)b;
    if (aa->distSq > bb->distSq) { return -1; }
    if (aa->distSq < bb->distSq) { return 1; }
    if (aa->cx != bb->cx) { return bb->cx - aa->cx; }
    return bb->cz - aa->cz;
}

int RendererV33_GetMeshBuildBudget(void)
{
    int budget;
    budget = g_chunkMeshBuildBudget;
    if (budget < 1) { budget = 1; }

    /* V41: give enough budget to fill render-distance holes, but still bound
       display-list compilation.  Missing chunks are built nearest-first in
       RenderWorld; stale chunks continue drawing until replaced. */
    if (g_currentFPS > 0 && g_currentFPS < 22 && budget > 2) { budget = 2; }
    else if (g_currentFPS > 0 && g_currentFPS < 35 && budget > 3) { budget = 3; }
    else if (g_currentFPS > 0 && g_currentFPS < 50 && budget > 5) { budget = 5; }
    else if (budget > 8) { budget = 8; }
    return budget;
}

int RendererV33_IsChunkInFrustum(int cx, int cz, int distSq, float *outDot)
{
    double minX;
    double maxX;
    double minZ;
    double maxZ;
    double centerX;
    double centerZ;
    double dx;
    double dz;
    double len;
    double yawRad;
    double forwardX;
    double forwardZ;
    double dot;
    double radius;
    int nearAlways;

    if (cx < 0 || cx >= WORLD_CHUNKS_X || cz < 0 || cz >= WORLD_CHUNKS_Z) { return 0; }

    minX = (double)(cx * CHUNK_SIZE);
    maxX = minX + (double)CHUNK_SIZE;
    minZ = (double)(cz * CHUNK_SIZE);
    maxZ = minZ + (double)CHUNK_SIZE;
    centerX = (minX + maxX) * 0.5;
    centerZ = (minZ + maxZ) * 0.5;
    dx = centerX - playerX;
    dz = centerZ - playerZ;
    len = sqrt(dx * dx + dz * dz);

    if (len < 0.001) {
        if (outDot) { *outDot = 1.0f; }
        return 1;
    }

    nearAlways = GetVeryNearTerrainRenderDistanceBlocks() + CHUNK_SIZE * 2;
    if (g_renderDistanceChunks <= 3 || distSq <= nearAlways * nearAlways) {
        if (outDot) { *outDot = 1.0f; }
        return 1;
    }

    if (g_renderV47UsePlaneFrustum && g_frustumValidV47) {
        if (!RendererV47_IsChunkVisibleAABB(cx, cz)) {
            g_renderProfileV33.chunksPlaneFrustumCulled++;
            if (outDot) { *outDot = -1.0f; }
            return 0;
        }
        if (outDot) { *outDot = 1.0f; }
        return 1;
    }

    yawRad = yaw * PI / 180.0;
    forwardX = -sin(yawRad);
    forwardZ = -cos(yawRad);
    dot = (dx * forwardX + dz * forwardZ) / len;

    /* A chunk is not a point.  Add a radius/far-distance margin so edge chunks
       do not pop when turning, matching the conservative idea behind Java's
       Frustrum.isBoxInFrustum on a rendererBoundingBox. */
    radius = (double)CHUNK_SIZE * 0.72;
    dot += radius / len;

    if (outDot) { *outDot = (float)dot; }
    /* V41: widen the acceptance edge.  The old -0.20 threshold culled side
       chunks too early and looked like broken render distance when turning. */
    if (dot > -0.55) { return 1; }
    return 0;
}

void RendererV33_BuildChunkQueues(int pcx, int pcz, int renderChunks)
{
    int cx;
    int cz;
    int dx;
    int dz;
    int distSq;
    int renderChunkSq;
    float dot;
    RendererV33ChunkEntry e;

    g_renderOpaqueCountV33 = 0;
    g_renderTransCountV33 = 0;
    renderChunkSq = renderChunks * renderChunks;

    for (cx = 0; cx < WORLD_CHUNKS_X; cx++) {
        for (cz = 0; cz < WORLD_CHUNKS_Z; cz++) {
            dx = cx - pcx;
            dz = cz - pcz;
            distSq = dx * dx + dz * dz;
            if (distSq > renderChunkSq) { continue; }
            g_renderProfileV33.chunksConsidered++;
            if (!RendererV33_IsChunkInFrustum(cx, cz, distSq * CHUNK_SIZE * CHUNK_SIZE, &dot)) {
                g_renderProfileV33.chunksFrustumCulled++;
                continue;
            }
            terrainChunkWasVisibleV33[cx][cz] = 1;
            e.cx = cx;
            e.cz = cz;
            e.distSq = distSq;
            e.dot = dot;
            if (g_renderOpaqueCountV33 < RENDER_V33_MAX_CHUNK_QUEUE) {
                g_renderOpaqueQueueV33[g_renderOpaqueCountV33++] = e;
            }
            if (g_renderTransCountV33 < RENDER_V33_MAX_CHUNK_QUEUE) {
                g_renderTransQueueV33[g_renderTransCountV33++] = e;
            }
        }
    }

    qsort(g_renderOpaqueQueueV33, (size_t)g_renderOpaqueCountV33, sizeof(RendererV33ChunkEntry), RendererV33_CompareNearToFar);
    qsort(g_renderTransQueueV33, (size_t)g_renderTransCountV33, sizeof(RendererV33ChunkEntry), RendererV33_CompareFarToNear);
}

void RendererV33_DrawProfilerOverlay(void)
{
    char line[192];
    int y;
    if (!g_debugRenderProfileV33) { return; }
    y = 42;
    DrawRect2D(8, 38, 660, 136, 0.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(line, "Render V47: solid draw/built/defer/stale/fallback %ld/%ld/%ld/%ld/%ld culled %ld plane %ld",
            g_renderProfileV33.chunksDrawn, g_renderProfileV33.chunksBuilt,
            g_renderProfileV33.chunksDeferred, g_renderProfileV33.chunksDrawnStale,
            g_renderProfileV33.chunksFallbackDrawn, g_renderProfileV33.chunksFrustumCulled,
            g_renderProfileV33.chunksPlaneFrustumCulled);
    DrawText2D(fontBaseNormal, 16, y + 16, line); y += 18;
    sprintf(line, "Trans list draw/built/defer %ld/%ld/%ld blocks %ld  special chunks/blocks %ld/%ld",
            g_renderProfileV33.transparentListsDrawn, g_renderProfileV33.transparentListsBuilt,
            g_renderProfileV33.transparentListsDeferred, g_renderProfileV33.transparentBlocks,
            g_renderProfileV33.specialChunksScanned, g_renderProfileV33.specialBlocksDrawn);
    DrawText2D(fontBaseNormal, 16, y + 16, line); y += 18;
    sprintf(line, "Pass ms: solid %.1f  mesh %.1f  trans %.1f special %.1f drawCalls %ld",
            (float)g_renderProfileV33.solidPassMs, (float)g_renderProfileV33.meshBuildMs,
            (float)g_renderProfileV33.translucentPassMs, (float)g_renderProfileV33.specialPassMs,
            g_renderProfileV33.drawCalls);
    DrawText2D(fontBaseNormal, 16, y + 16, line); y += 18;
    sprintf(line, "Entities: mobs %ld culled %ld/%ld  particles %ld culled %ld/%ld skipped %ld",
            g_renderProfileV33.mobsRendered, g_renderProfileV33.mobsDistanceCulled,
            g_renderProfileV33.mobsFrustumCulled, g_renderProfileV33.particlesDrawn,
            g_renderProfileV33.particlesDistanceCulled, g_renderProfileV33.particlesFrustumCulled,
            g_renderProfileV33.particlesSpawnSkipped);
    DrawText2D(fontBaseNormal, 16, y + 16, line); y += 18;
    sprintf(line, "Lighting V48: queue %d processed %ld changed %ld skipped %ld skySub %d",
            g_lightQueueCountV48, g_lightV48ProcessedBoxes, g_lightV48ChangedCells,
            g_lightV48SkippedLarge, g_skyLightSubtractedV48);
    DrawText2D(fontBaseNormal, 16, y + 16, line);
}

/* ------------------------------------------------------------ */
/* RENDER_PIPELINE_V8 implementation                            */
/* ------------------------------------------------------------ */

void RendererV8_ResetStats(void)
{
    ZeroMemory(&g_rendererStatsV8, sizeof(g_rendererStatsV8));
}

void RendererV8_RegisterSpecial(int type, RendererV8SpecialRenderFn fn)
{
    if (g_renderDispatchCountV8 >= RENDER_V8_MAX_DISPATCH) {
        return;
    }
    g_renderDispatchV8[g_renderDispatchCountV8].type = type;
    g_renderDispatchV8[g_renderDispatchCountV8].renderFn = fn;
    g_renderDispatchCountV8++;
}

RendererV8SpecialRenderFn RendererV8_FindSpecialRenderer(int type)
{
    int i;
    for (i = 0; i < g_renderDispatchCountV8; i++) {
        if (g_renderDispatchV8[i].type == type) {
            return g_renderDispatchV8[i].renderFn;
        }
    }
    return 0;
}

void RendererV8_Init(void)
{
    if (g_rendererV8Initialized) {
        return;
    }
    g_renderDispatchCountV8 = 0;
    RendererV8_RegisterSpecial(ENTITY_V6_ARROW, RendererV8_RenderArrowEntity);
    RendererV8_RegisterSpecial(ENTITY_V6_EGG, RendererV8_RenderThrowableEntity);
    RendererV8_RegisterSpecial(ENTITY_V6_SNOWBALL, RendererV8_RenderThrowableEntity);
    RendererV8_RegisterSpecial(ENTITY_V6_FIREBALL, RendererV8_RenderFireballEntity);
    RendererV8_RegisterSpecial(ENTITY_V6_FISH_HOOK, RendererV8_RenderFishHookEntity);
    RendererV8_RegisterSpecial(ENTITY_V6_BOAT, RendererV8_RenderBoatEntity);
    RendererV8_RegisterSpecial(ENTITY_V6_PAINTING, RendererV8_RenderPaintingEntity);
    RendererV8_RegisterSpecial(ENTITY_V6_TNT, RendererV8_RenderTNTEntity);
    RendererV8_RegisterSpecial(ENTITY_V6_FALLING_BLOCK, RendererV8_RenderFallingBlockEntity);
    RendererV8_RegisterSpecial(ENTITY_V6_LIGHTNING, RendererV8_RenderLightningEntity);
    g_rendererV8Initialized = 1;
}

void TessellatorV8_BeginTerrain(GLuint tex)
{
    if (g_tessellatorActiveV8) {
        TessellatorV8_End();
    }
    g_tessellatorActiveV8 = 1;
    g_tessellatorTextureV8 = tex;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
}

void TessellatorV8_End(void)
{
    if (!g_tessellatorActiveV8) {
        return;
    }
    glEnd();
    g_tessellatorActiveV8 = 0;
    g_rendererStatsV8.tessFlushes++;
    glColor3f(1.0f, 1.0f, 1.0f);
}

int RendererV8_IsTranslucentBlock(int block)
{
    BlockDefV19 *d;
    d = BlockRegistryV19_Get(block);
    if (d->translucent) { return 1; }
    return 0;
}


void RendererV8_PostWorldEffects(void)
{
    if (g_rendererV8SelectionOutlineEnabled) {
        RendererV8_RenderSelectionOutline();
    }
    if (g_rendererV8BreakingOverlayEnabled) {
        RendererV8_RenderBreakingOverlay();
    }
    if (!g_skipWeatherRenderV13 && g_weatherMode != 0 && g_rainStrength > 0.02) {
        RendererV8_RenderWeather3D();
    }
}

void RendererV8_DrawBlockWireBox(int x, int y, int z, float expand, float r, float g, float b, float a)
{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;
    x0 = (float)x - expand;
    y0 = (float)y - expand;
    z0 = (float)z - expand;
    x1 = (float)x + 1.0f + expand;
    y1 = (float)y + 1.0f + expand;
    z1 = (float)z + 1.0f + expand;
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);
    glBegin(GL_LINES);
    glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1);
    glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1);
    glVertex3f(x0, y0, z1); glVertex3f(x0, y0, z0);
    glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z0); glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
    glVertex3f(x0, y1, z1); glVertex3f(x0, y1, z0);
    glVertex3f(x0, y0, z0); glVertex3f(x0, y1, z0);
    glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0);
    glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1);
    glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void RendererV8_RenderSelectionOutline(void)
{
    int hx;
    int hy;
    int hz;
    int px;
    int py;
    int pz;
    int block;
    (void)px;
    (void)py;
    (void)pz;
    if (!ItemCombatV6_RaycastBlock(&hx, &hy, &hz, &px, &py, &pz)) {
        return;
    }
    block = GetBlock(hx, hy, hz);
    if (block == BLOCK_AIR || block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_FIRE) {
        return;
    }
    glLineWidth(2.0f);
    RendererV8_DrawBlockWireBox(hx, hy, hz, 0.003f, 0.0f, 0.0f, 0.0f, 0.70f);
    glLineWidth(1.0f);
}

void RendererV11_RenderBreakingTextureFace(int face, float x0, float y0, float z0, float x1, float y1, float z1, float u0, float v0, float u1, float v1)
{
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
    } else {
        glTexCoord2f(u0, v0); glVertex3f(x1, y0, z1);
        glTexCoord2f(u1, v0); glVertex3f(x1, y0, z0);
        glTexCoord2f(u1, v1); glVertex3f(x1, y1, z0);
        glTexCoord2f(u0, v1); glVertex3f(x1, y1, z1);
    }
}

void RendererV8_RenderBreakingOverlay(void)
{
    float p;
    float e;
    int stage;
    int face;
    float x0;
    float y0;
    float z0;
    float x1;
    float y1;
    float z1;
    float u0;
    float v0;
    float u1;
    float v1;

    if (g_miningX_V6 < 0 || g_miningY_V6 < 0 || g_miningZ_V6 < 0) { return; }
    if (g_miningProgressV6 <= 0.02) { return; }
    if (!IsInsideWorld(g_miningX_V6, g_miningY_V6, g_miningZ_V6)) { return; }

    p = (float)g_miningProgressV6;
    if (p > 1.0f) { p = 1.0f; }
    stage = (int)(p * 10.0f);
    if (stage < 0) { stage = 0; }
    if (stage > 9) { stage = 9; }

    e = 0.0075f;
    x0 = (float)g_miningX_V6 - e;
    y0 = (float)g_miningY_V6 - e;
    z0 = (float)g_miningZ_V6 - e;
    x1 = (float)g_miningX_V6 + 1.0f + e;
    y1 = (float)g_miningY_V6 + 1.0f + e;
    z1 = (float)g_miningZ_V6 + 1.0f + e;

    if (texBlockBreakV11) {
        u0 = ((float)(stage * 16) + 0.5f) / 160.0f;
        v0 = 0.5f / 16.0f;
        u1 = ((float)((stage + 1) * 16) - 0.5f) / 160.0f;
        v1 = 15.5f / 16.0f;
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texBlockBreakV11);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glColor4f(1.0f, 1.0f, 1.0f, 0.72f);
        glBegin(GL_QUADS);
        for (face = 0; face < 6; face++) {
            RendererV11_RenderBreakingTextureFace(face, x0, y0, z0, x1, y1, z1, u0, v0, u1, v1);
        }
        glEnd();
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        return;
    }

    /* Fallback if the texture is missing. */
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.20f + p * 0.35f);
    RendererV8_DrawBlockWireBox(g_miningX_V6, g_miningY_V6, g_miningZ_V6, e, 0.0f, 0.0f, 0.0f, 0.20f + p * 0.35f);
    glEnable(GL_CULL_FACE);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void RendererV8_RenderTransparentBlocks(void)
{
    int i;
    int cx;
    int cz;
    int builds;
    int budget;
    DWORD passStartMs;

    if (g_renderTransCountV33 <= 0) { return; }
    passStartMs = GetTickCount();
    builds = 0;
    budget = g_renderV47TransBuildBudget;
    if (g_renderV47InitialBuildBoostFrames > 0) { budget += 2; }
    if (budget < 1) { budget = 1; }
    if (g_currentFPS > 0 && g_currentFPS < 28 && budget > 1) { budget = 1; }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    /* V47: Java WorldRenderer.skipRenderPass[1]-style cached translucent pass.
       Older builds scanned every visible column every frame for water/glass/fire.
       At render distance 4-6 that was the main hitch source.  Translucent
       geometry now gets its own dirty display list per chunk and is drawn
       far-to-near through the already sorted RenderSorter queue. */
    for (i = 0; i < g_renderTransCountV33; i++) {
        cx = g_renderTransQueueV33[i].cx;
        cz = g_renderTransQueueV33[i].cz;
        if (terrainChunkTransDirty[cx][cz] || !terrainChunkTransLists[cx][cz]) {
            if (builds < budget && RendererV47_TimeBudgetAllows(passStartMs, builds, budget + 1)) {
                BuildTerrainChunkTransMeshV47(cx, cz);
                builds++;
                g_renderProfileV33.transparentListsBuilt++;
            } else {
                g_renderProfileV33.transparentListsDeferred++;
            }
        }
        if (terrainChunkTransLists[cx][cz] && !terrainChunkSkipPassTransV47[cx][cz]) {
            glCallList(terrainChunkTransLists[cx][cz]);
            g_renderProfileV33.transparentListsDrawn++;
            g_renderProfileV33.drawCalls++;
        }
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    g_renderProfileV33.translucentPassMs += (double)(GetTickCount() - passStartMs);
}

void RendererV8_RenderArrowEntity(SpecialEntityV6 *e)
{
    double yawDeg;
    double pitchDeg;
    double len;
    len = sqrt(e->vx * e->vx + e->vy * e->vy + e->vz * e->vz);
    yawDeg = atan2(e->vx, e->vz) * 180.0 / PI;
    pitchDeg = 0.0;
    if (len > 0.001) { pitchDeg = asin(e->vy / len) * 180.0 / PI; }
    glPushMatrix();
    glTranslatef((float)e->x, (float)e->y, (float)e->z);
    glRotatef((float)yawDeg, 0.0f, 1.0f, 0.0f);
    glRotatef((float)-pitchDeg, 1.0f, 0.0f, 0.0f);
    RenderItemCombatV6_Cube(0.0, 0.0, 0.0, 0.055, 0.055, 0.80, 0.78f, 0.66f, 0.45f);
    glPopMatrix();
}

void RendererV8_RenderThrowableEntity(SpecialEntityV6 *e)
{
    float r;
    float g;
    float b;
    r = 0.96f; g = 0.96f; b = 0.88f;
    if (e->type == ENTITY_V6_SNOWBALL) { r = 1.0f; g = 1.0f; b = 1.0f; }
    RenderItemCombatV6_Cube(e->x, e->y, e->z, 0.22, 0.22, 0.22, r, g, b);
}

void RendererV8_RenderFireballEntity(SpecialEntityV6 *e)
{
    double s;
    s = 0.34 + sin(e->age * 18.0) * 0.04;
    RenderItemCombatV6_Cube(e->x, e->y, e->z, s, s, s, 1.0f, 0.42f, 0.08f);
}

void RendererV8_RenderFishHookEntity(SpecialEntityV6 *e)
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.08f, 0.08f, 0.08f);
    glBegin(GL_LINES);
    glVertex3f((float)playerX, (float)(playerY + EYE_HEIGHT - 0.35), (float)playerZ);
    glVertex3f((float)e->x, (float)e->y, (float)e->z);
    glEnd();
    RenderItemCombatV6_Cube(e->x, e->y, e->z, 0.12, 0.12, 0.12, 0.22f, 0.22f, 0.22f);
}

void RendererV8_DrawBoatModel(SpecialEntityV6 *e)
{
    glPushMatrix();
    glTranslatef((float)e->x, (float)(e->y + 0.22), (float)e->z);
    glRotatef((float)(e->yaw), 0.0f, 1.0f, 0.0f);
    RenderItemCombatV6_Cube(0.0, 0.0, 0.0, 1.35, 0.18, 1.90, 0.52f, 0.30f, 0.12f);
    RenderItemCombatV6_Cube(-0.72, 0.22, 0.0, 0.10, 0.55, 1.92, 0.42f, 0.23f, 0.09f);
    RenderItemCombatV6_Cube(0.72, 0.22, 0.0, 0.10, 0.55, 1.92, 0.42f, 0.23f, 0.09f);
    RenderItemCombatV6_Cube(0.0, 0.22, -0.98, 1.35, 0.55, 0.10, 0.42f, 0.23f, 0.09f);
    RenderItemCombatV6_Cube(0.0, 0.22, 0.98, 1.35, 0.55, 0.10, 0.42f, 0.23f, 0.09f);
    glPopMatrix();
}

void RendererV8_RenderBoatEntity(SpecialEntityV6 *e)
{
    RendererV8_DrawBoatModel(e);
}

void RendererV8_DrawPaintingByMeta(SpecialEntityV6 *e)
{
    float sx;
    float sy;
    float u0;
    float v0;
    float u1;
    float v1;
    int art;
    art = e->meta & 7;
    sx = 1.0f;
    sy = 1.0f;
    if (art == 1 || art == 2) { sx = 2.0f; }
    if (art == 3 || art == 4) { sy = 2.0f; }
    if (art >= 5) { sx = 2.0f; sy = 2.0f; }
    glPushMatrix();
    glTranslatef((float)e->x, (float)e->y, (float)e->z);
    glRotatef((float)e->yaw, 0.0f, 1.0f, 0.0f);
    if (texCompatArtKz) {
        u0 = (float)((art % 4) * 16) / 256.0f;
        v0 = (float)((art / 4) * 16) / 256.0f;
        u1 = u0 + (sx * 16.0f) / 256.0f;
        v1 = v0 + (sy * 16.0f) / 256.0f;
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texCompatArtKz);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(u0, v1); glVertex3f(-sx * 0.5f, -sy * 0.5f, 0.02f);
        glTexCoord2f(u1, v1); glVertex3f( sx * 0.5f, -sy * 0.5f, 0.02f);
        glTexCoord2f(u1, v0); glVertex3f( sx * 0.5f,  sy * 0.5f, 0.02f);
        glTexCoord2f(u0, v0); glVertex3f(-sx * 0.5f,  sy * 0.5f, 0.02f);
        glEnd();
    } else {
        RenderItemCombatV6_Cube(0.0, 0.0, 0.0, sx, sy, 0.06, 0.75f, 0.55f, 0.30f);
    }
    glPopMatrix();
}

void RendererV8_RenderPaintingEntity(SpecialEntityV6 *e)
{
    RendererV8_DrawPaintingByMeta(e);
}

void RendererV8_RenderTNTEntity(SpecialEntityV6 *e)
{
    float pulse;
    pulse = 1.0f + (float)(sin(e->age * 18.0) * 0.06);
    glPushMatrix();
    glTranslatef((float)e->x, (float)e->y, (float)e->z);
    glScalef(pulse, pulse, pulse);
    if (texTerrain) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texTerrain);
        DrawDroppedBlockCube(BLOCK_TNT);
    } else {
        RenderItemCombatV6_Cube(0.0, 0.0, 0.0, 0.85, 0.85, 0.85, 0.95f, 0.15f, 0.10f);
    }
    glPopMatrix();
}

void RendererV8_RenderFallingBlockEntity(SpecialEntityV6 *e)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glPushMatrix();
    glTranslatef((float)e->x, (float)e->y, (float)e->z);
    glRotatef((float)(e->age * 80.0), 0.0f, 1.0f, 0.0f);
    glScalef(0.55f, 0.55f, 0.55f);
    DrawDroppedBlockCube(e->block);
    glPopMatrix();
}

void RendererV8_RenderLightningEntity(SpecialEntityV6 *e)
{
    int i;
    float x;
    float z;
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glLineWidth(3.0f);
    glColor4f(0.75f, 0.85f, 1.0f, 0.95f);
    glBegin(GL_LINE_STRIP);
    x = (float)e->x;
    z = (float)e->z;
    glVertex3f(x, (float)(WORLD_Y - 1), z);
    for (i = 0; i < 8; i++) {
        x += (float)((WorldHash3D(i, (int)e->age, (int)e->x, 9911) % 100) - 50) / 90.0f;
        z += (float)((WorldHash3D(i, (int)e->age, (int)e->z, 9912) % 100) - 50) / 90.0f;
        glVertex3f(x, (float)(WORLD_Y - 1 - i * 8), z);
    }
    glVertex3f((float)e->x, (float)e->y, (float)e->z);
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_CULL_FACE);
}

void RendererV8_RenderSpecialEntities(void)
{
    int i;
    RendererV8SpecialRenderFn fn;
    RendererV8_Init();
    for (i = 0; i < MAX_SPECIAL_ENTITIES_V6; i++) {
        if (!g_specialEntitiesV6[i].active) { continue; }
        fn = RendererV8_FindSpecialRenderer(g_specialEntitiesV6[i].type);
        if (fn) { fn(&g_specialEntitiesV6[i]); }
        else { RenderItemCombatV6_Cube(g_specialEntitiesV6[i].x, g_specialEntitiesV6[i].y, g_specialEntitiesV6[i].z, 0.30, 0.30, 0.30, 1.0f, 1.0f, 1.0f); }
        g_rendererStatsV8.renderedEntities++;
    }
}

void RendererV8_RenderMobProjectiles(void)
{
    int i;
    SpecialEntityV6 tmp;
    ZeroMemory(&tmp, sizeof(tmp));
    for (i = 0; i < MAX_MOB_PROJECTILES; i++) {
        if (!mobProjectiles[i].active) { continue; }
        tmp.type = ENTITY_V6_ARROW;
        tmp.x = mobProjectiles[i].x;
        tmp.y = mobProjectiles[i].y;
        tmp.z = mobProjectiles[i].z;
        tmp.vx = mobProjectiles[i].vx;
        tmp.vy = mobProjectiles[i].vy;
        tmp.vz = mobProjectiles[i].vz;
        RendererV8_RenderArrowEntity(&tmp);
        g_rendererStatsV8.renderedEntities++;
    }
}

void RendererV8_DrawBlockItemMini(int block, float size)
{
    if (block == BLOCK_AIR) { return; }
    if (!ShouldRenderBlockItemAs3DV27(block)) {
        DrawTerrainItemCardLocalV27(block, size, size * 0.08f);
        return;
    }
    glPushMatrix();
    glScalef(size, size, size);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    DrawDroppedBlockCube(block);
    glPopMatrix();
}

void RendererV8_DrawTexturedItemQuad(int item, float size)
{
    DrawItemCardLocalV27(item, size, size * 0.08f);
}

void RendererV8_RenderFirstPersonItem(void)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(70.0, (double)g_windowWidth / (double)g_windowHeight, 0.05, 16.0);
    glMatrixMode(GL_MODELVIEW);
    RenderPlayerHand();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void RendererV8_RenderWeather3D(void)
{
    int i;
    int gx;
    int gz;
    int lx;
    int lz;
    int top;
    int mode;
    float x;
    float z;
    float y0;
    float y1;
    float a;
    float driftX;
    float driftZ;
    float halfW;
    float u0;
    float v0;
    float u1;
    float v1;
    GLuint tex;

    mode = g_weatherMode;
    if (mode == 0 || g_rainStrength <= 0.02) { return; }

    tex = (mode == 2) ? texBetaSnow : texBetaRain;
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    if (tex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    a = (float)(0.18 + g_rainStrength * 0.42);
    if (mode == 2) { glColor4f(0.92f, 0.96f, 1.0f, a); }
    else { glColor4f(0.55f, 0.64f, 0.82f, a); }

    glBegin(GL_QUADS);
    for (i = 0; i < RENDER_V8_RAIN_COLUMNS; i++) {
        gx = (int)floor(GetPlayerGlobalX()) - 24 + (WorldHash2D(i, (int)g_weatherScroll, g_worldSeed + 771) % 49);
        gz = (int)floor(GetPlayerGlobalZ()) - 24 + (WorldHash2D(i, (int)g_weatherScroll, g_worldSeed + 772) % 49);
        lx = GlobalToLocalBlockX(gx);
        lz = GlobalToLocalBlockZ(gz);
        if (lx < 0 || lx >= WORLD_X || lz < 0 || lz >= WORLD_Z) { continue; }
        top = columnTop[lx][lz] + 1;
        if (top < 1) { top = 1; }
        x = (float)lx + 0.5f;
        z = (float)lz + 0.5f;
        y0 = (float)(top + 8 + (WorldHash2D(i, top, g_worldSeed + 773) & 7));
        if (y0 > (float)(WORLD_Y - 2)) { y0 = (float)(WORLD_Y - 2); }

        if (mode == 2) {
            y1 = y0 - 0.95f;
            halfW = 0.18f;
            driftX = g_weatherWindX_V24 * 0.20f;
            driftZ = g_weatherWindZ_V24 * 0.20f;
        } else {
            y1 = y0 - 3.6f;
            halfW = 0.07f;
            driftX = g_weatherWindX_V24 * 0.55f - 0.22f;
            driftZ = g_weatherWindZ_V24 * 0.55f + 0.10f;
        }

        u0 = 0.0f; v0 = 0.0f; u1 = 1.0f; v1 = 1.0f;
        glTexCoord2f(u0, v0); glVertex3f(x - halfW, y0, z - halfW);
        glTexCoord2f(u1, v0); glVertex3f(x + halfW, y0, z + halfW);
        glTexCoord2f(u1, v1); glVertex3f(x + halfW + driftX, y1, z + halfW + driftZ);
        glTexCoord2f(u0, v1); glVertex3f(x - halfW + driftX, y1, z - halfW + driftZ);
        g_rendererStatsV8.weatherQuads++;
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void RendererV8_DrawMinecartModel(double x, double y, double z, double yawDeg)
{
    glPushMatrix();
    glTranslatef((float)x, (float)(y + 0.25), (float)z);
    glRotatef((float)yawDeg, 0.0f, 1.0f, 0.0f);
    RenderItemCombatV6_Cube(0.0, 0.0, 0.0, 0.95, 0.18, 1.25, 0.28f, 0.28f, 0.30f);
    RenderItemCombatV6_Cube(-0.52, 0.28, 0.0, 0.08, 0.46, 1.25, 0.20f, 0.20f, 0.22f);
    RenderItemCombatV6_Cube( 0.52, 0.28, 0.0, 0.08, 0.46, 1.25, 0.20f, 0.20f, 0.22f);
    RenderItemCombatV6_Cube(0.0, 0.28, -0.66, 0.95, 0.46, 0.08, 0.20f, 0.20f, 0.22f);
    RenderItemCombatV6_Cube(0.0, 0.28, 0.66, 0.95, 0.46, 0.08, 0.20f, 0.20f, 0.22f);
    glPopMatrix();
}

void RendererV8_RenderMinecarts(void)
{
    int i;
    double yawDeg;
    for (i = 0; i < MAX_MINECARTS_V5; i++) {
        if (!g_minecartsV5[i].active) { continue; }
        yawDeg = atan2(g_minecartsV5[i].vx, g_minecartsV5[i].vz) * 180.0 / PI;
        RendererV8_DrawMinecartModel(g_minecartsV5[i].x, g_minecartsV5[i].y, g_minecartsV5[i].z, yawDeg);
    }
}

/* ------------------------------------------------------------ */
/* Crosshair                                                    */
/* ------------------------------------------------------------ */

void DrawCrosshair(void)
{
    int cx;
    int cy;

    Setup2D();

    cx = g_windowWidth / 2;
    cy = g_windowHeight / 2;

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_LINES);

    glVertex2i(cx - 8, cy);
    glVertex2i(cx + 8, cy);

    glVertex2i(cx, cy - 8);
    glVertex2i(cx, cy + 8);

    glEnd();
}

/* ------------------------------------------------------------ */
/* Survival system                                              */
/* ------------------------------------------------------------ */

