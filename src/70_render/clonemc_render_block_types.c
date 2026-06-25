/* ============================================================
   CloneMC V51 section: RENDER / BLOCK RENDER TYPES / RENDERBLOCKS-STYLE GEOMETRY
   ============================================================ */

                RendererV47_TimeBudgetAllows(passStartMs, builds, budget + 2)) {
                BuildTerrainChunkMesh(cx, cz);
                builds++;
                g_renderProfileV33.chunksBuilt++;
            } else {
                g_renderProfileV33.chunksDeferred++;
                if (terrainChunkLists[cx][cz]) { g_renderProfileV33.chunksDrawnStale++; }
                else {
                    g_renderProfileV33.chunksMissingNoBudget++;
                    if (g_renderV47FallbackShell && mustBuildNear) {
                        RendererV47_DrawFallbackChunkShell(cx, cz);
                        g_renderProfileV33.chunksFallbackDrawn++;
                    }
                }
            }
        }

        if (terrainChunkLists[cx][cz] && !terrainChunkSkipPassSolidV47[cx][cz]) {
            glCallList(terrainChunkLists[cx][cz]);
            g_renderProfileV33.chunksDrawn++;
            g_renderProfileV33.drawCalls++;
        }
    }

    nowMs = GetTickCount();
    g_renderProfileV33.solidPassMs += (double)(nowMs - passStartMs);
    RenderWorldBorderOceanIllusion();
}



void RenderBlockFaceBoundsV19(int x, int y, int z, int face, int block, float bx0, float by0, float bz0, float bx1, float by1, float bz1)
{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;
    int col;
    int row;
    float u0;
    float tv0;
    float u1;
    float tv1;
    float base;
    float faceShade;
    float ao0;
    float ao1;
    float ao2;
    float ao3;
    float vb0;
    float vb1;
    float vb2;
    float vb3;

    x0 = (float)x + bx0;
    y0 = (float)y + by0;
    z0 = (float)z + bz0;
    x1 = (float)x + bx1;
    y1 = (float)y + by1;
    z1 = (float)z + bz1;

    if (!texTerrain) {
        SetBlockColorFallback(block, face);
        if (!g_tessellatorActiveV8) { glBegin(GL_QUADS); }
    } else {
        BlockV19_GetTextureTileAt(block, x, y, z, face, &col, &row);
        GetTerrainTileUV(col, row, &u0, &tv0, &u1, &tv1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texTerrain);
        base = GetLegacyFaceBrightness(x, y, z, face, block);
        faceShade = GetLegacyFaceShade(face);
        ComputeFaceAO(x, y, z, face, &ao0, &ao1, &ao2, &ao3);
        ComputeFaceVertexBrightnessV48(x, y, z, face, block, &vb0, &vb1, &vb2, &vb3);
        DrawBiomeTintOverlayForBlock(block, x, z);
        if (!g_tessellatorActiveV8) { glBegin(GL_QUADS); }
    }

    if (!texTerrain) {
        if (face == 0) {
            glVertex3f(x0, y1, z0); glVertex3f(x0, y1, z1); glVertex3f(x1, y1, z1); glVertex3f(x1, y1, z0);
        } else if (face == 1) {
            glVertex3f(x0, y0, z1); glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1);
        } else if (face == 2) {
            glVertex3f(x1, y0, z0); glVertex3f(x0, y0, z0); glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0);
        } else if (face == 3) {
            glVertex3f(x0, y0, z1); glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
        } else if (face == 4) {
            glVertex3f(x0, y0, z0); glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1); glVertex3f(x0, y1, z0);
        } else if (face == 5) {
            glVertex3f(x1, y0, z1); glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0); glVertex3f(x1, y1, z1);
        }
    } else {
        if (face == 0) {
            EmitLitVertex(u0, tv0, x0, y1, z0, vb0 * faceShade * ao0);
            EmitLitVertex(u0, tv1, x0, y1, z1, vb1 * faceShade * ao1);
            EmitLitVertex(u1, tv1, x1, y1, z1, vb2 * faceShade * ao2);
            EmitLitVertex(u1, tv0, x1, y1, z0, vb3 * faceShade * ao3);
        } else if (face == 1) {
            EmitLitVertex(u0, tv0, x0, y0, z1, vb0 * faceShade * ao0);
            EmitLitVertex(u0, tv1, x0, y0, z0, vb1 * faceShade * ao1);
            EmitLitVertex(u1, tv1, x1, y0, z0, vb2 * faceShade * ao2);
            EmitLitVertex(u1, tv0, x1, y0, z1, vb3 * faceShade * ao3);
        } else if (face == 2) {
            EmitLitVertex(u0, tv0, x1, y0, z0, vb0 * faceShade * ao0);
            EmitLitVertex(u1, tv0, x0, y0, z0, vb1 * faceShade * ao1);
            EmitLitVertex(u1, tv1, x0, y1, z0, vb2 * faceShade * ao2);
            EmitLitVertex(u0, tv1, x1, y1, z0, vb3 * faceShade * ao3);
        } else if (face == 3) {
            EmitLitVertex(u0, tv0, x0, y0, z1, vb0 * faceShade * ao0);
            EmitLitVertex(u1, tv0, x1, y0, z1, vb1 * faceShade * ao1);
            EmitLitVertex(u1, tv1, x1, y1, z1, vb2 * faceShade * ao2);
            EmitLitVertex(u0, tv1, x0, y1, z1, vb3 * faceShade * ao3);
        } else if (face == 4) {
            EmitLitVertex(u0, tv0, x0, y0, z0, vb0 * faceShade * ao0);
            EmitLitVertex(u1, tv0, x0, y0, z1, vb1 * faceShade * ao1);
            EmitLitVertex(u1, tv1, x0, y1, z1, vb2 * faceShade * ao2);
            EmitLitVertex(u0, tv1, x0, y1, z0, vb3 * faceShade * ao3);
        } else if (face == 5) {
            EmitLitVertex(u0, tv0, x1, y0, z1, vb0 * faceShade * ao0);
            EmitLitVertex(u1, tv0, x1, y0, z0, vb1 * faceShade * ao1);
            EmitLitVertex(u1, tv1, x1, y1, z0, vb2 * faceShade * ao2);
            EmitLitVertex(u0, tv1, x1, y1, z1, vb3 * faceShade * ao3);
        }
    }

    if (!g_tessellatorActiveV8) { glEnd(); }
    g_rendererStatsV8.tessVertices += 4;
    g_vertexTintR = 1.0f;
    g_vertexTintG = 1.0f;
    g_vertexTintB = 1.0f;
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void RenderGrassSideOverlayV19(int x, int y, int z, int face, float bx0, float by0, float bz0, float bx1, float by1, float bz1)
{
    int col;
    int row;
    float u0;
    float v0;
    float u1;
    float v1;
    float oldR;
    float oldG;
    float oldB;
    if (!texTerrain) { return; }
    /* V26: local atlas has no separate transparent grass-side overlay; rendering it
       previously drew the wrong tile over grass.  Grass side now uses the correct
       base tile only. */
    return;
    ResourceV10_JavaTileToAtlas(38, &col, &row);
    GetTerrainTileUV(col, row, &u0, &v0, &u1, &v1);
    oldR = g_vertexTintR;
    oldG = g_vertexTintG;
    oldB = g_vertexTintB;
    g_vertexTintR = 0.72f;
    g_vertexTintG = 0.92f;
    g_vertexTintB = 0.56f;
    if (!g_tessellatorActiveV8) { glBegin(GL_QUADS); }
    if (face == 2) {
        EmitLitVertex(u0, v0, (float)x + bx1, (float)y + by0, (float)z + bz0 - 0.001f, 0.85f);
        EmitLitVertex(u1, v0, (float)x + bx0, (float)y + by0, (float)z + bz0 - 0.001f, 0.85f);
        EmitLitVertex(u1, v1, (float)x + bx0, (float)y + by1, (float)z + bz0 - 0.001f, 0.85f);
        EmitLitVertex(u0, v1, (float)x + bx1, (float)y + by1, (float)z + bz0 - 0.001f, 0.85f);
    } else if (face == 3) {
        EmitLitVertex(u0, v0, (float)x + bx0, (float)y + by0, (float)z + bz1 + 0.001f, 0.85f);
        EmitLitVertex(u1, v0, (float)x + bx1, (float)y + by0, (float)z + bz1 + 0.001f, 0.85f);
        EmitLitVertex(u1, v1, (float)x + bx1, (float)y + by1, (float)z + bz1 + 0.001f, 0.85f);
        EmitLitVertex(u0, v1, (float)x + bx0, (float)y + by1, (float)z + bz1 + 0.001f, 0.85f);
    } else if (face == 4) {
        EmitLitVertex(u0, v0, (float)x + bx0 - 0.001f, (float)y + by0, (float)z + bz0, 0.80f);
        EmitLitVertex(u1, v0, (float)x + bx0 - 0.001f, (float)y + by0, (float)z + bz1, 0.80f);
        EmitLitVertex(u1, v1, (float)x + bx0 - 0.001f, (float)y + by1, (float)z + bz1, 0.80f);
        EmitLitVertex(u0, v1, (float)x + bx0 - 0.001f, (float)y + by1, (float)z + bz0, 0.80f);
    } else if (face == 5) {
        EmitLitVertex(u0, v0, (float)x + bx1 + 0.001f, (float)y + by0, (float)z + bz1, 0.80f);
        EmitLitVertex(u1, v0, (float)x + bx1 + 0.001f, (float)y + by0, (float)z + bz0, 0.80f);
        EmitLitVertex(u1, v1, (float)x + bx1 + 0.001f, (float)y + by1, (float)z + bz0, 0.80f);
        EmitLitVertex(u0, v1, (float)x + bx1 + 0.001f, (float)y + by1, (float)z + bz1, 0.80f);
    }
    if (!g_tessellatorActiveV8) { glEnd(); }
    g_vertexTintR = oldR;
    g_vertexTintG = oldG;
    g_vertexTintB = oldB;
}

void RenderBlockCubeBoundsV19(int x, int y, int z, int block, float bx0, float by0, float bz0, float bx1, float by1, float bz1, int cullAsFullCube)
{
    if (!cullAsFullCube || ShouldDrawFace(x, y + 1, z, block)) { RenderBlockFaceBoundsV19(x, y, z, 0, block, bx0, by0, bz0, bx1, by1, bz1); }
    if (!cullAsFullCube || ShouldDrawFace(x, y - 1, z, block)) { RenderBlockFaceBoundsV19(x, y, z, 1, block, bx0, by0, bz0, bx1, by1, bz1); }
    if (!cullAsFullCube || ShouldDrawFace(x, y, z - 1, block)) { RenderBlockFaceBoundsV19(x, y, z, 2, block, bx0, by0, bz0, bx1, by1, bz1); if (block == BLOCK_GRASS) { RenderGrassSideOverlayV19(x, y, z, 2, bx0, by0, bz0, bx1, by1, bz1); } }
    if (!cullAsFullCube || ShouldDrawFace(x, y, z + 1, block)) { RenderBlockFaceBoundsV19(x, y, z, 3, block, bx0, by0, bz0, bx1, by1, bz1); if (block == BLOCK_GRASS) { RenderGrassSideOverlayV19(x, y, z, 3, bx0, by0, bz0, bx1, by1, bz1); } }
    if (!cullAsFullCube || ShouldDrawFace(x - 1, y, z, block)) { RenderBlockFaceBoundsV19(x, y, z, 4, block, bx0, by0, bz0, bx1, by1, bz1); if (block == BLOCK_GRASS) { RenderGrassSideOverlayV19(x, y, z, 4, bx0, by0, bz0, bx1, by1, bz1); } }
    if (!cullAsFullCube || ShouldDrawFace(x + 1, y, z, block)) { RenderBlockFaceBoundsV19(x, y, z, 5, block, bx0, by0, bz0, bx1, by1, bz1); if (block == BLOCK_GRASS) { RenderGrassSideOverlayV19(x, y, z, 5, bx0, by0, bz0, bx1, by1, bz1); } }
}

void RenderFlatTopTileV19(int x, int y, int z, int block, float h)
{
    int col;
    int row;
    float u0;
    float v0;
    float u1;
    float v1;
    float yy;
    float p;
    BlockV19_GetTextureTileAt(block, x, y, z, 0, &col, &row);
    GetTerrainTileUV(col, row, &u0, &v0, &u1, &v1);
    yy = (float)y + h;
    p = 0.02f;
    if (!texTerrain) { return; }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    if (block == BLOCK_REDSTONE_WIRE) {
        p = (float)g_redstonePower[x][y][z] / 15.0f;
        glColor4f(0.35f + p * 0.65f, 0.05f, 0.05f, 1.0f);
    } else {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v0); glVertex3f((float)x, yy, (float)z);
    glTexCoord2f(u1, v0); glVertex3f((float)x + 1.0f, yy, (float)z);
    glTexCoord2f(u1, v1); glVertex3f((float)x + 1.0f, yy, (float)z + 1.0f);
    glTexCoord2f(u0, v1); glVertex3f((float)x, yy, (float)z + 1.0f);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    (void)p;
}

void RenderCrossPlanesV19(int x, int y, int z, int block, float height, float halfWidth)
{
    int col;
    int row;
    float u0;
    float v0;
    float u1;
    float v1;
    float cx;
    float cz;
    float y0;
    float y1;
    if (!texTerrain) { return; }
    BlockV19_GetTextureTileAt(block, x, y, z, 0, &col, &row);
    GetTerrainTileUV(col, row, &u0, &v0, &u1, &v1);
    cx = (float)x + 0.5f;
    cz = (float)z + 0.5f;
    y0 = (float)y;
    y1 = (float)y + height;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v1); glVertex3f(cx - halfWidth, y0, cz - halfWidth);
    glTexCoord2f(u1, v1); glVertex3f(cx + halfWidth, y0, cz + halfWidth);
    glTexCoord2f(u1, v0); glVertex3f(cx + halfWidth, y1, cz + halfWidth);
    glTexCoord2f(u0, v0); glVertex3f(cx - halfWidth, y1, cz - halfWidth);
    glTexCoord2f(u0, v1); glVertex3f(cx + halfWidth, y0, cz - halfWidth);
    glTexCoord2f(u1, v1); glVertex3f(cx - halfWidth, y0, cz + halfWidth);
    glTexCoord2f(u1, v0); glVertex3f(cx - halfWidth, y1, cz + halfWidth);
    glTexCoord2f(u0, v0); glVertex3f(cx + halfWidth, y1, cz - halfWidth);
    glEnd();
    glEnable(GL_CULL_FACE);
}

void RenderTorchBlockV19(int x, int y, int z, int block)
{
    int col;
    int row;
    float u0;
    float v0;
    float u1;
    float v1;
    float cx;
    float cz;
    float y0;
    float y1;
    float w;
    float leanX;
    float leanZ;
    unsigned char meta;
    if (!texTerrain) { return; }
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    leanX = 0.0f;
    leanZ = 0.0f;
    if ((meta & 7) == 1) { leanX = -0.18f; }
    else if ((meta & 7) == 2) { leanX = 0.18f; }
    else if ((meta & 7) == 3) { leanZ = -0.18f; }
    else if ((meta & 7) == 4) { leanZ = 0.18f; }
    BlockV19_GetTextureTileAt(block, x, y, z, 0, &col, &row);
    GetTerrainTileUV(col, row, &u0, &v0, &u1, &v1);
    cx = (float)x + 0.5f;
    cz = (float)z + 0.5f;
    y0 = (float)y + 0.05f;
    y1 = (float)y + 0.85f;
    w = 0.12f;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v1); glVertex3f(cx - w, y0, cz);
    glTexCoord2f(u1, v1); glVertex3f(cx + w, y0, cz);
    glTexCoord2f(u1, v0); glVertex3f(cx + w + leanX, y1, cz + leanZ);
    glTexCoord2f(u0, v0); glVertex3f(cx - w + leanX, y1, cz + leanZ);
    glTexCoord2f(u0, v1); glVertex3f(cx, y0, cz - w);
    glTexCoord2f(u1, v1); glVertex3f(cx, y0, cz + w);
    glTexCoord2f(u1, v0); glVertex3f(cx + leanX, y1, cz + w + leanZ);
    glTexCoord2f(u0, v0); glVertex3f(cx + leanX, y1, cz - w + leanZ);
    glEnd();
    glEnable(GL_CULL_FACE);
}

void RenderDoorBlockV19(int x, int y, int z, int block)
{
    unsigned char meta;
    unsigned char lowerMeta;
    int lowerY;
    int dir;
    int open;
    float minX;
    float maxX;
    float minZ;
    float maxZ;
    meta = 0;
    lowerMeta = 0;
    lowerY = y;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    if ((meta & 8) && IsInsideWorld(x, y - 1, z) && GetBlock(x, y - 1, z) == block) { lowerY = y - 1; }
    if (IsInsideWorld(x, lowerY, z)) { lowerMeta = g_blockMeta[x][lowerY][z]; }
    dir = lowerMeta & 3;
    /* Java BlockDoor uses bit 4 for open/closed state; low two bits are facing. */
    open = (lowerMeta & 4) ? 1 : 0;
    minX = 0.0f; maxX = 1.0f; minZ = 0.0f; maxZ = 1.0f;
    if (!open) {
        if (dir == 0) { maxZ = 0.1875f; }
        else if (dir == 1) { minX = 0.8125f; }
        else if (dir == 2) { minZ = 0.8125f; }
        else { maxX = 0.1875f; }
    } else {
        if (dir == 0) { minX = 0.8125f; }
        else if (dir == 1) { minZ = 0.8125f; }
        else if (dir == 2) { maxX = 0.1875f; }
        else { maxZ = 0.1875f; }
    }
    RenderBlockCubeBoundsV19(x, y, z, block, minX, 0.0f, minZ, maxX, 1.0f, maxZ, 0);
}

void RenderLadderBlockV19(int x, int y, int z, int block)
{
    unsigned char meta;
    float minX;
    float maxX;
    float minZ;
    float maxZ;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    minX = 0.0f;
    maxX = 1.0f;
    minZ = 0.0f;
    maxZ = 1.0f;
    if ((meta & 7) == 2) { maxZ = 0.0625f; }
    else if ((meta & 7) == 3) { minZ = 0.9375f; }
    else if ((meta & 7) == 4) { maxX = 0.0625f; }
    else { minX = 0.9375f; }
    RenderBlockCubeBoundsV19(x, y, z, block, minX, 0.0f, minZ, maxX, 1.0f, maxZ, 0);
}

void RenderFenceBlockV19(int x, int y, int z, int block)
{
    int north;
    int south;
    int west;
    int east;
    north = BlockV43_ConnectsFence(GetBlock(x, y, z - 1));
    south = BlockV43_ConnectsFence(GetBlock(x, y, z + 1));
    west = BlockV43_ConnectsFence(GetBlock(x - 1, y, z));
    east = BlockV43_ConnectsFence(GetBlock(x + 1, y, z));
    RenderBlockCubeBoundsV19(x, y, z, block, 0.375f, 0.0f, 0.375f, 0.625f, 1.5f, 0.625f, 0);
    if (north) { RenderBlockCubeBoundsV19(x, y, z, block, 0.4375f, 0.375f, 0.0f, 0.5625f, 0.75f, 0.5f, 0); RenderBlockCubeBoundsV19(x, y, z, block, 0.4375f, 0.9375f, 0.0f, 0.5625f, 1.1875f, 0.5f, 0); }
    if (south) { RenderBlockCubeBoundsV19(x, y, z, block, 0.4375f, 0.375f, 0.5f, 0.5625f, 0.75f, 1.0f, 0); RenderBlockCubeBoundsV19(x, y, z, block, 0.4375f, 0.9375f, 0.5f, 0.5625f, 1.1875f, 1.0f, 0); }
    if (west) { RenderBlockCubeBoundsV19(x, y, z, block, 0.0f, 0.375f, 0.4375f, 0.5f, 0.75f, 0.5625f, 0); RenderBlockCubeBoundsV19(x, y, z, block, 0.0f, 0.9375f, 0.4375f, 0.5f, 1.1875f, 0.5625f, 0); }
    if (east) { RenderBlockCubeBoundsV19(x, y, z, block, 0.5f, 0.375f, 0.4375f, 1.0f, 0.75f, 0.5625f, 0); RenderBlockCubeBoundsV19(x, y, z, block, 0.5f, 0.9375f, 0.4375f, 1.0f, 1.1875f, 0.5625f, 0); }
}


int BlockV43_ConnectsFence(int block)
{
    if (block == BLOCK_AIR) { return 0; }
    if (block == BLOCK_FENCE || block == BLOCK_WOOD_DOOR || block == BLOCK_IRON_DOOR) { return 1; }
    if (block == BLOCK_GLASS || block == BLOCK_LEAVES || block == BLOCK_CACTUS || block == BLOCK_TORCH) { return 0; }
    return IsSolidBlock(block);
}

void RenderBlockCubeNoCullV43(int x, int y, int z, int block, float bx0, float by0, float bz0, float bx1, float by1, float bz1)
{
    RenderBlockCubeBoundsV19(x, y, z, block, bx0, by0, bz0, bx1, by1, bz1, 0);
}

void RenderRailBlockV43(int x, int y, int z, int block)
{
    unsigned char meta;
    int col;
    int row;
    float u0;
    float v0;
    float u1;
    float v1;
    float y0;
    float y1;
    float yn;
    float ys;
    float yw;
    float ye;
    if (!texTerrain) { return; }
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    BlockV19_GetTextureTileAt(block, x, y, z, 0, &col, &row);
    GetTerrainTileUV(col, row, &u0, &v0, &u1, &v1);
    y0 = (float)y + 0.020f;
    y1 = (float)y + 1.020f;
    yn = y0; ys = y0; yw = y0; ye = y0;
    if ((meta & 7) == 2) { ye = y1; }
    else if ((meta & 7) == 3) { yw = y1; }
    else if ((meta & 7) == 4) { ys = y1; }
    else if ((meta & 7) == 5) { yn = y1; }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    if ((meta & 7) == 1 || (meta & 7) == 5 || (meta & 7) == 4) {
        glTexCoord2f(u0, v0); glVertex3f((float)x,     yn, (float)z);
        glTexCoord2f(u1, v0); glVertex3f((float)x + 1, yn, (float)z);
        glTexCoord2f(u1, v1); glVertex3f((float)x + 1, ys, (float)z + 1);
        glTexCoord2f(u0, v1); glVertex3f((float)x,     ys, (float)z + 1);
    } else {
        glTexCoord2f(u0, v0); glVertex3f((float)x,     yw, (float)z);
        glTexCoord2f(u1, v0); glVertex3f((float)x + 1, ye, (float)z);
        glTexCoord2f(u1, v1); glVertex3f((float)x + 1, ye, (float)z + 1);
        glTexCoord2f(u0, v1); glVertex3f((float)x,     yw, (float)z + 1);
    }
    glEnd();
    glEnable(GL_CULL_FACE);
}

void RenderRedstoneWireV43(int x, int y, int z, int block)
{
    int north;
    int south;
    int west;
    int east;
    float p;
    float h;
    float r;
    float g;
    float b;
    north = (GetBlock(x, y, z - 1) == BLOCK_REDSTONE_WIRE || GetBlock(x, y, z - 1) == BLOCK_REPEATER_ON || GetBlock(x, y, z - 1) == BLOCK_REPEATER_OFF || GetBlock(x, y, z - 1) == BLOCK_REDSTONE_TORCH_ON);
    south = (GetBlock(x, y, z + 1) == BLOCK_REDSTONE_WIRE || GetBlock(x, y, z + 1) == BLOCK_REPEATER_ON || GetBlock(x, y, z + 1) == BLOCK_REPEATER_OFF || GetBlock(x, y, z + 1) == BLOCK_REDSTONE_TORCH_ON);
    west = (GetBlock(x - 1, y, z) == BLOCK_REDSTONE_WIRE || GetBlock(x - 1, y, z) == BLOCK_REPEATER_ON || GetBlock(x - 1, y, z) == BLOCK_REPEATER_OFF || GetBlock(x - 1, y, z) == BLOCK_REDSTONE_TORCH_ON);
    east = (GetBlock(x + 1, y, z) == BLOCK_REDSTONE_WIRE || GetBlock(x + 1, y, z) == BLOCK_REPEATER_ON || GetBlock(x + 1, y, z) == BLOCK_REPEATER_OFF || GetBlock(x + 1, y, z) == BLOCK_REDSTONE_TORCH_ON);
    if (!north && !south && !west && !east) { north = south = west = east = 1; }
    p = 0.0f;
    if (IsInsideWorld(x, y, z)) { p = (float)g_redstonePower[x][y][z] / 15.0f; }
    r = 0.25f + p * 0.75f;
    g = 0.02f + p * 0.08f;
    b = 0.02f + p * 0.08f;
    h = 0.026f;
    glColor4f(r, g, b, 1.0f);
    RenderFlatTopTileV19(x, y, z, block, h);
    if (north) { RenderBlockCubeNoCullV43(x, y, z, block, 0.375f, h, 0.0f, 0.625f, h + 0.018f, 0.50f); }
    if (south) { RenderBlockCubeNoCullV43(x, y, z, block, 0.375f, h, 0.50f, 0.625f, h + 0.018f, 1.0f); }
    if (west) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, h, 0.375f, 0.50f, h + 0.018f, 0.625f); }
    if (east) { RenderBlockCubeNoCullV43(x, y, z, block, 0.50f, h, 0.375f, 1.0f, h + 0.018f, 0.625f); }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void RenderPressurePlateV43(int x, int y, int z, int block)
{
    unsigned char meta;
    float h;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    h = (meta & 8) ? 0.03125f : 0.0625f;
    RenderBlockCubeNoCullV43(x, y, z, block, 0.0625f, 0.0f, 0.0625f, 0.9375f, h, 0.9375f);
}

void RenderButtonV43(int x, int y, int z, int block)
{
    unsigned char meta;
    int dir;
    float depth;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    dir = meta & 7;
    depth = (meta & 8) ? 0.0625f : 0.125f;
    if (dir == 1) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.375f, 0.3125f, depth, 0.625f, 0.6875f); }
    else if (dir == 2) { RenderBlockCubeNoCullV43(x, y, z, block, 1.0f - depth, 0.375f, 0.3125f, 1.0f, 0.625f, 0.6875f); }
    else if (dir == 3) { RenderBlockCubeNoCullV43(x, y, z, block, 0.3125f, 0.375f, 0.0f, 0.6875f, 0.625f, depth); }
    else { RenderBlockCubeNoCullV43(x, y, z, block, 0.3125f, 0.375f, 1.0f - depth, 0.6875f, 0.625f, 1.0f); }
}

void RenderLeverV43(int x, int y, int z, int block)
{
    unsigned char meta;
    int dir;
    float cx0;
    float cx1;
    float cz0;
    float cz1;
    float y0;
    float y1;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    dir = meta & 7;
    cx0 = 0.375f; cx1 = 0.625f; cz0 = 0.375f; cz1 = 0.625f; y0 = 0.0f; y1 = 0.1875f;
    if (dir == 1) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.20f, 0.3125f, 0.1875f, 0.80f, 0.6875f); return; }
    if (dir == 2) { RenderBlockCubeNoCullV43(x, y, z, block, 0.8125f, 0.20f, 0.3125f, 1.0f, 0.80f, 0.6875f); return; }
    if (dir == 3) { RenderBlockCubeNoCullV43(x, y, z, block, 0.3125f, 0.20f, 0.0f, 0.6875f, 0.80f, 0.1875f); return; }
    if (dir == 4) { RenderBlockCubeNoCullV43(x, y, z, block, 0.3125f, 0.20f, 0.8125f, 0.6875f, 0.80f, 1.0f); return; }
    RenderBlockCubeNoCullV43(x, y, z, block, cx0, y0, cz0, cx1, y1, cz1);
    RenderBlockCubeNoCullV43(x, y, z, block, 0.4375f, 0.1875f, 0.4375f, 0.5625f, 0.75f, 0.5625f);
}

void RenderTrapdoorV43(int x, int y, int z, int block)
{
    unsigned char meta;
    int dir;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    dir = meta & 3;
    if (meta & 1) {
        if (dir == 0) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.0f, 0.8125f, 1.0f, 1.0f, 1.0f); }
        else if (dir == 1) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.1875f); }
        else if (dir == 2) { RenderBlockCubeNoCullV43(x, y, z, block, 0.8125f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f); }
        else { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.0f, 0.0f, 0.1875f, 1.0f, 1.0f); }
    } else {
        if (meta & 8) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.8125f, 0.0f, 1.0f, 1.0f, 1.0f); }
        else { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, 0.1875f, 1.0f); }
    }
}

void RenderSignPostV43(int x, int y, int z, int block)
{
    unsigned char meta;
    int wall;
    float thick;
    meta = 0;
    wall = (block == BLOCK_SIGN_WALL || block == BLOCK_WALL_SIGN);
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    thick = 0.0625f;
    if (wall) {
        if ((meta & 7) == 2) { RenderBlockCubeNoCullV43(x, y, z, BLOCK_PLANKS, 0.125f, 0.28125f, 0.0f, 0.875f, 0.78125f, thick); }
        else if ((meta & 7) == 3) { RenderBlockCubeNoCullV43(x, y, z, BLOCK_PLANKS, 0.125f, 0.28125f, 1.0f - thick, 0.875f, 0.78125f, 1.0f); }
        else if ((meta & 7) == 4) { RenderBlockCubeNoCullV43(x, y, z, BLOCK_PLANKS, 0.0f, 0.28125f, 0.125f, thick, 0.78125f, 0.875f); }
        else { RenderBlockCubeNoCullV43(x, y, z, BLOCK_PLANKS, 1.0f - thick, 0.28125f, 0.125f, 1.0f, 0.78125f, 0.875f); }
    } else {
        RenderBlockCubeNoCullV43(x, y, z, BLOCK_PLANKS, 0.46875f, 0.0f, 0.46875f, 0.53125f, 0.8125f, 0.53125f);
        if ((meta & 15) >= 4 && (meta & 15) < 12) { RenderBlockCubeNoCullV43(x, y, z, BLOCK_PLANKS, 0.125f, 0.75f, 0.4375f, 0.875f, 1.25f, 0.5625f); }
        else { RenderBlockCubeNoCullV43(x, y, z, BLOCK_PLANKS, 0.4375f, 0.75f, 0.125f, 0.5625f, 1.25f, 0.875f); }
    }
}

void RenderChestBlockV43(int x, int y, int z, int block)
{
    RenderBlockCubeNoCullV43(x, y, z, block, 0.0625f, 0.0f, 0.0625f, 0.9375f, 0.875f, 0.9375f);
}

void RenderFurnaceLikeBlockV43(int x, int y, int z, int block)
{
    RenderBlockCubeBoundsV19(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1);
}

void RenderStairsBlockV43(int x, int y, int z, int block)
{
    unsigned char meta;
    int dir;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    dir = meta & 3;
    if (meta & 4) {
        RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
        if (dir == 0) { RenderBlockCubeNoCullV43(x, y, z, block, 0.5f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f); }
        else if (dir == 1) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f); }
        else if (dir == 2) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f); }
        else { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f); }
    } else {
        RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
        if (dir == 0) { RenderBlockCubeNoCullV43(x, y, z, block, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f); }
        else if (dir == 1) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f); }
        else if (dir == 2) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f); }
        else { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 0.5f); }
    }
}

void RenderPistonBlockV43(int x, int y, int z, int block)
{
    unsigned char meta;
    int dir;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    dir = meta & 7;
    if (block == BLOCK_PISTON_EXTENSION) {
        if (dir == 0 || dir == 1) { RenderBlockCubeNoCullV43(x, y, z, block, 0.25f, 0.0f, 0.25f, 0.75f, 1.0f, 0.75f); }
        else if (dir == 2 || dir == 3) { RenderBlockCubeNoCullV43(x, y, z, block, 0.25f, 0.25f, 0.0f, 0.75f, 0.75f, 1.0f); }
        else { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.25f, 0.25f, 1.0f, 0.75f, 0.75f); }
        return;
    }
    RenderBlockCubeBoundsV19(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1);
}

void RenderBedBlockV43(int x, int y, int z, int block)
{
    unsigned char meta;
    int dir;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    dir = meta & 3;
    if (dir == 0 || dir == 2) { RenderBlockCubeNoCullV43(x, y, z, block, 0.0f, 0.0f, 0.0625f, 1.0f, 0.5625f, 0.9375f); }
    else { RenderBlockCubeNoCullV43(x, y, z, block, 0.0625f, 0.0f, 0.0f, 0.9375f, 0.5625f, 1.0f); }
}

void RenderCakeBlockV43(int x, int y, int z, int block)
{
    unsigned char meta;
    float eaten;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    eaten = (float)(meta & 7) * 0.125f;
    if (eaten > 0.75f) { eaten = 0.75f; }
    RenderBlockCubeNoCullV43(x, y, z, block, 0.0625f + eaten, 0.0f, 0.0625f, 0.9375f, 0.5f, 0.9375f);
}

void RenderPortalBlockV43(int x, int y, int z, int block)
{
    int col;
    int row;
    float u0;
    float v0;
    float u1;
    float v1;
    float t;
    if (!texTerrain) { return; }
    BlockV19_GetTextureTileAt(block, x, y, z, 0, &col, &row);
    GetTerrainTileUV(col, row, &u0, &v0, &u1, &v1);
    t = 0.06f;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.85f, 0.35f, 1.0f, 0.70f);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v1); glVertex3f((float)x + 0.5f - t, (float)y, (float)z);
    glTexCoord2f(u1, v1); glVertex3f((float)x + 0.5f + t, (float)y, (float)z + 1.0f);
    glTexCoord2f(u1, v0); glVertex3f((float)x + 0.5f + t, (float)y + 1.0f, (float)z + 1.0f);
    glTexCoord2f(u0, v0); glVertex3f((float)x + 0.5f - t, (float)y + 1.0f, (float)z);
    glTexCoord2f(u0, v1); glVertex3f((float)x, (float)y, (float)z + 0.5f - t);
    glTexCoord2f(u1, v1); glVertex3f((float)x + 1.0f, (float)y, (float)z + 0.5f + t);
    glTexCoord2f(u1, v0); glVertex3f((float)x + 1.0f, (float)y + 1.0f, (float)z + 0.5f + t);
    glTexCoord2f(u0, v0); glVertex3f((float)x, (float)y + 1.0f, (float)z + 0.5f - t);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_CULL_FACE);
}

void RenderFireBlockV43(int x, int y, int z, int block)
{
    int n;
    int s;
    int w;
    int e;
    RenderCrossPlanesV19(x, y, z, block, 1.0f, 0.50f);
    n = GetBlock(x, y, z - 1) != BLOCK_AIR;
    s = GetBlock(x, y, z + 1) != BLOCK_AIR;
    w = GetBlock(x - 1, y, z) != BLOCK_AIR;
    e = GetBlock(x + 1, y, z) != BLOCK_AIR;
    if (n) { RenderCrossPlanesV19(x, y, z, block, 1.0f, 0.35f); }
    if (s) { RenderCrossPlanesV19(x, y, z, block, 1.0f, 0.35f); }
    if (w) { RenderCrossPlanesV19(x, y, z, block, 1.0f, 0.35f); }
    if (e) { RenderCrossPlanesV19(x, y, z, block, 1.0f, 0.35f); }
}

void RenderSlabOrLayerBlockV57(int x, int y, int z, int block)
{
    unsigned char meta;
    float minY;
    float maxY;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    minY = 0.0f;
    maxY = 1.0f;
    if (block == BLOCK_STEP) {
        if (meta & 8) { minY = 0.5f; maxY = 1.0f; }
        else { minY = 0.0f; maxY = 0.5f; }
        RenderBlockCubeBoundsV19(x, y, z, block, 0.0f, minY, 0.0f, 1.0f, maxY, 1.0f, 0);
        return;
    }
    if (block == BLOCK_SNOW) {
        maxY = (float)(((meta & 7) + 1) * 2) / 16.0f;
        if (maxY < 0.125f) { maxY = 0.125f; }
        if (maxY > 1.0f) { maxY = 1.0f; }
        RenderBlockCubeBoundsV19(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, maxY, 1.0f, 0);
        return;
    }
    if (block == BLOCK_FARMLAND) {
        RenderBlockCubeBoundsV19(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, 0.9375f, 1.0f, 0);
        return;
    }
    if (block == BLOCK_CACTUS) {
        RenderBlockCubeBoundsV19(x, y, z, block, 0.0625f, 0.0f, 0.0625f, 0.9375f, 1.0f, 0.9375f, 0);
        return;
    }
    RenderBlockCubeBoundsV19(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0);
}

void RenderBlockAtV19(int x, int y, int z, int block)
{
    BlockDefV19 *d;
    float h;
    if (block == BLOCK_AIR) { return; }
    d = BlockRegistryV19_Get(block);
    if (d->renderType == BLOCK_RENDER_AIR_V19) { return; }
    if (d->renderType == BLOCK_RENDER_CUBE_V19) { RenderBlockCubeBoundsV19(x, y, z, block, d->minX, d->minY, d->minZ, d->maxX, d->maxY, d->maxZ, 1); return; }
    if (d->renderType == BLOCK_RENDER_SLAB_V19 || d->renderType == BLOCK_RENDER_SNOW_V19 || d->renderType == BLOCK_RENDER_CACTUS_V19) { RenderSlabOrLayerBlockV57(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_FLUID_V19) { RenderFluidBlockV42(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_CROSS_V19) {
        h = 0.90f;
        if (block == BLOCK_REED) { h = 1.0f; }
        if (block == BLOCK_CROPS) { h = 0.55f + ((float)(g_blockMeta[x][y][z] & 7) * 0.06f); }
        RenderCrossPlanesV19(x, y, z, block, h, 0.45f);
        return;
    }
    if (d->renderType == BLOCK_RENDER_FIRE_V19) { RenderFireBlockV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_FLAT_V19) { RenderFlatTopTileV19(x, y, z, block, 0.035f); return; }
    if (d->renderType == BLOCK_RENDER_TORCH_V19) { RenderTorchBlockV19(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_DOOR_V19) { RenderDoorBlockV19(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_LADDER_V19) { RenderLadderBlockV19(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_FENCE_V19) { RenderFenceBlockV19(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_STAIRS_V43) { RenderStairsBlockV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_RAIL_V43) { RenderRailBlockV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_REDSTONE_V43) { RenderRedstoneWireV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_PRESSURE_V43) { RenderPressurePlateV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_BUTTON_V43) { RenderButtonV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_LEVER_V43) { RenderLeverV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_SIGN_POST_V43) { RenderSignPostV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_TRAPDOOR_V43) { RenderTrapdoorV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_CHEST_V43) { RenderChestBlockV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_FURNACE_V43) { RenderFurnaceLikeBlockV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_PISTON_V43) { RenderPistonBlockV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_BED_V43) { RenderBedBlockV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_CAKE_V43) { RenderCakeBlockV43(x, y, z, block); return; }
    if (d->renderType == BLOCK_RENDER_PORTAL_V43) { RenderPortalBlockV43(x, y, z, block); return; }
    RenderBlockCubeBoundsV19(x, y, z, block, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1);
}

void DrawTorchBlock(int x, int y, int z)
{
    RenderTorchBlockV19(x, y, z, BLOCK_TORCH);
}

int WorldGenV3_IsCrossPlantBlock(int block)
{
    BlockDefV19 *d;
    d = BlockRegistryV19_Get(block);
    if (d->renderType == BLOCK_RENDER_CROSS_V19 || d->renderType == BLOCK_RENDER_FIRE_V19) { return 1; }
    return 0;
}

void WorldGenV3_DrawCrossPlantBlock(int x, int y, int z, int block)
{
    RenderBlockAtV19(x, y, z, block);
}

void DrawBlock(int x, int y, int z, int block)
{
    if (block == BLOCK_AIR) { return; }
    if (IsRemovedPlantBlockV12(block)) { return; }
    RenderBlockAtV19(x, y, z, block);
}

int ShouldDrawFace(int nx, int ny, int nz, int block)
{
    int neighbor;
    neighbor = GetBlock(nx, ny, nz);
    return BlockV49_ShouldSideBeRendered(block, neighbor);
}

void DrawFace(int x, int y, int z, int face, int block)
{
    RenderBlockFaceBoundsV19(x, y, z, face, block, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}


void SetBlockColorFallback(int block, int face)
{
    float shade;

    shade = 1.0f;

    if (face == 1) {
        shade = 0.55f;
    } else if (face == 2 || face == 3) {
        shade = 0.75f;
    } else if (face == 4 || face == 5) {
        shade = 0.65f;
    }

    if (block == BLOCK_GRASS) {
        if (face == 0) {
            glColor3f(0.56f * shade, 0.56f * shade, 0.56f * shade);
        } else {
            glColor3f(0.40f * shade, 0.25f * shade, 0.10f * shade);
        }
    } else if (block == BLOCK_DIRT) {
        glColor3f(0.40f * shade, 0.25f * shade, 0.10f * shade);
    } else if (block == BLOCK_STONE) {
        glColor3f(0.45f * shade, 0.45f * shade, 0.45f * shade);
    } else if (block == BLOCK_WOOD) {
        glColor3f(0.38f * shade, 0.20f * shade, 0.08f * shade);
    } else if (block == BLOCK_LEAVES) {
        glColor3f(0.44f * shade, 0.44f * shade, 0.44f * shade);
    } else if (block == BLOCK_WATER) {
        glColor3f(0.32f * shade, 0.44f * shade, 0.70f * shade);
    } else if (block == BLOCK_BORDER) {
        glColor3f(0.05f * shade, 0.05f * shade, 0.05f * shade);
    } else if (block == BLOCK_LIGHT) {
        glColor3f(0.85f * shade, 0.85f * shade, 0.85f * shade);
    } else if (block == BLOCK_WOOL) {
        glColor3f(0.88f, 0.88f, 0.84f);
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
}

/* ------------------------------------------------------------ */
/* Legacy grayscale lighting + ambient occlusion                */
/* ------------------------------------------------------------ */

