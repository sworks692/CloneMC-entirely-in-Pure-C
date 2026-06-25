/* ============================================================
   CloneMC V51 section: SAVE / TILE ENTITIES / REGION / NBT-LIKE PERSISTENCE
   ============================================================ */

void DrawBiomeTintOverlayForBlock(int block, int x, int z)
{
    /* PATCH_NO_GREEN_TINT:
       The earlier biome-color pass multiplied grass/leaves/water by green and
       swamp/desert hues.  That looked like a broken overlay with the uploaded
       terrain atlas.  Keep Java/Beta-style blocky light and face shading, but
       force texture color back to neutral white so the PNG/TGA pixels show with
       their original hue. */
    (void)block;
    (void)x;
    (void)z;
    g_vertexTintR = 1.0f;
    g_vertexTintG = 1.0f;
    g_vertexTintB = 1.0f;
}


int IsTileEntityBlock(int block)
{
    /* Java/Beta-style tile-entity blocks that carry extra state beyond
       a simple block id.  Keeping this in one helper prevents missing
       linker symbols and makes it easy to add/remove tile blocks later. */
    switch (block) {
    case BLOCK_CHEST:
    case BLOCK_FURNACE:
    case BLOCK_FURNACE_LIT:
    case BLOCK_SIGN_POST:
    case BLOCK_SIGN_WALL:
    case BLOCK_DISPENSER:
    case BLOCK_MOB_SPAWNER:
    case BLOCK_NOTE:
    case BLOCK_PISTON:
    case BLOCK_PISTON_STICKY:
    case BLOCK_JUKEBOX:
        return 1;
    default:
        return 0;
    }
}

int FindTileEntityAt(int x, int y, int z)
{
    int i;
    for (i = 0; i < MAX_TILE_ENTITIES; i++) {
        if (tileEntities[i].active && tileEntities[i].x == x && tileEntities[i].y == y && tileEntities[i].z == z) {
            return i;
        }
    }
    return -1;
}

void EnsureTileEntityForBlock(int block, int x, int y, int z)
{
    int i;
    int idx;
    if (!IsTileEntityBlock(block)) { return; }
    idx = FindTileEntityAt(x, y, z);
    if (idx >= 0) { tileEntities[idx].type = block; return; }
    for (i = 0; i < MAX_TILE_ENTITIES; i++) {
        if (!tileEntities[i].active) {
            ZeroMemory(&tileEntities[i], sizeof(TileEntityState));
            tileEntities[i].active = 1;
            tileEntities[i].type = block;
            tileEntities[i].x = x;
            tileEntities[i].y = y;
            tileEntities[i].z = z;
            if (block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL) { strcpy(tileEntities[i].text, "CloneMC sign"); }
            if (block == BLOCK_MOB_SPAWNER) { tileEntities[i].power = MOB_ZOMBIE; tileEntities[i].cookTime = 6.0; }
            if (block == BLOCK_NOTE) { tileEntities[i].power = 0; }
            return;
        }
    }
}

void RemoveTileEntityAt(int x, int y, int z)
{
    int idx;
    idx = FindTileEntityAt(x, y, z);
    if (idx >= 0) { ZeroMemory(&tileEntities[idx], sizeof(TileEntityState)); }
}

void SaveCurrentTileEntities(void)
{
    FILE *f;
    char path[128];
    int i;
    int j;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    wsprintf(path, "saves\\world%d_tiles.dat", currentWorldSlot);
    f = fopen(path, "w");
    if (!f) { return; }
    fprintf(f, "CLONEMC_TILEENTITIES_V1\n");
    for (i = 0; i < MAX_TILE_ENTITIES; i++) {
        if (!tileEntities[i].active) { continue; }
        fprintf(f, "tile %d %d %d %d %.3f %.3f %d %s\n",
                tileEntities[i].type, tileEntities[i].x, tileEntities[i].y, tileEntities[i].z,
                tileEntities[i].burnTime, tileEntities[i].cookTime, tileEntities[i].power,
                tileEntities[i].text);
        for (j = 0; j < 27; j++) {
            if (tileEntities[i].slots[j].item != ITEM_NONE && tileEntities[i].slots[j].count > 0) {
                fprintf(f, "slot %d %d %d %d\n", j, tileEntities[i].slots[j].item,
                        tileEntities[i].slots[j].count, tileEntities[i].slots[j].damage);
            }
        }
    }
    fclose(f);
}

int LoadCurrentTileEntities(void)
{
    FILE *f;
    char path[128];
    char key[64];
    char text[64];
    int type;
    int x;
    int y;
    int z;
    int power;
    int slot;
    int item;
    int count;
    int damage;
    int idx;
    double burn;
    double cook;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    wsprintf(path, "saves\\world%d_tiles.dat", currentWorldSlot);
    f = fopen(path, "r");
    if (!f) { return 0; }
    if (fscanf(f, "%63s", key) != 1 || strcmp(key, "CLONEMC_TILEENTITIES_V1") != 0) { fclose(f); return 0; }
    ZeroMemory(tileEntities, sizeof(tileEntities));
    idx = -1;
    while (fscanf(f, "%63s", key) == 1) {
        if (strcmp(key, "tile") == 0) {
            text[0] = 0;
            if (fscanf(f, "%d %d %d %d %lf %lf %d %63s", &type, &x, &y, &z, &burn, &cook, &power, text) >= 7) {
                EnsureTileEntityForBlock(type, x, y, z);
                idx = FindTileEntityAt(x, y, z);
                if (idx >= 0) {
                    tileEntities[idx].burnTime = burn;
                    tileEntities[idx].cookTime = cook;
                    tileEntities[idx].power = power;
                    if (text[0]) { strncpy(tileEntities[idx].text, text, 63); tileEntities[idx].text[63] = 0; }
                }
            }
        } else if (strcmp(key, "slot") == 0) {
            damage = 0;
            if (fscanf(f, "%d %d %d %d", &slot, &item, &count, &damage) >= 3) {
                if (idx >= 0 && slot >= 0 && slot < 27) {
                    tileEntities[idx].slots[slot].item = item;
                    tileEntities[idx].slots[slot].count = count;
                    tileEntities[idx].slots[slot].damage = damage;
                }
            }
        }
    }
    fclose(f);
    return 1;
}

void SaveCurrentRegionLite(void)
{
    FILE *f;
    char path[128];
    int x;
    int y;
    int z;
    unsigned short b;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    wsprintf(path, "saves\\world%d_region_lite.mcr", currentWorldSlot);
    f = fopen(path, "wb");
    if (!f) { return; }
    fwrite("CMCR", 1, 4, f);
    fwrite(&worldOriginBlockX, sizeof(int), 1, f);
    fwrite(&worldOriginBlockZ, sizeof(int), 1, f);
    fwrite(&g_worldSeed, sizeof(int), 1, f);
    fwrite(&g_worldTimeSeconds, sizeof(double), 1, f);
    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                b = (unsigned short)world[x][y][z];
                fwrite(&b, sizeof(unsigned short), 1, f);
            }
        }
    }
    fclose(f);
}

int LoadCurrentRegionLite(void)
{
    FILE *f;
    char path[128];
    char magic[4];
    int ox;
    int oz;
    int seed;
    int x;
    int y;
    int z;
    unsigned short b;
    double wt;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    wsprintf(path, "saves\\world%d_region_lite.mcr", currentWorldSlot);
    f = fopen(path, "rb");
    if (!f) { return 0; }
    if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "CMCR", 4) != 0) { fclose(f); return 0; }
    if (fread(&ox, sizeof(int), 1, f) != 1 || fread(&oz, sizeof(int), 1, f) != 1 ||
        fread(&seed, sizeof(int), 1, f) != 1 || fread(&wt, sizeof(double), 1, f) != 1) { fclose(f); return 0; }
    if (ox != worldOriginBlockX || oz != worldOriginBlockZ) { fclose(f); return 0; }
    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                if (fread(&b, sizeof(unsigned short), 1, f) != 1) { fclose(f); return 0; }
                world[x][y][z] = (int)b;
            }
        }
    }
    fclose(f);
    g_worldSeed = seed;
    g_worldTimeSeconds = wt;
    RebuildColumnTops();
    InvalidateAllTerrainChunkMeshes();
    return 1;
}


/* ------------------------------------------------------------ */
/* Java NBT / Region / SaveHandler conversion pass              */
/* ------------------------------------------------------------ */

typedef struct CmcNbtWriter {
    FILE *f;
    int ok;
} CmcNbtWriter;

typedef struct CmcNbtReader {
    FILE *f;
    int ok;
} CmcNbtReader;

void SavePath_WorldDir(int slot, char *path)
{
    sprintf(path, "saves\\World%d", slot + 1);
}

void SavePath_RegionDir(int slot, char *path)
{
    sprintf(path, "saves\\World%d\\region", slot + 1);
}

void SavePath_LevelDat(int slot, char *path)
{
    sprintf(path, "saves\\World%d\\level.dat", slot + 1);
}

void SavePath_PlayerDat(int slot, char *path)
{
    sprintf(path, "saves\\World%d\\player.dat", slot + 1);
}

void SavePath_EntitiesDat(int slot, char *path)
{
    sprintf(path, "saves\\World%d\\entities.dat", slot + 1);
}

void SavePath_TileEntitiesDat(int slot, char *path)
{
    sprintf(path, "saves\\World%d\\tileentities.dat", slot + 1);
}

void SavePath_LegacyRegionFileV29(int slot, char *path)
{
    sprintf(path, "saves\\World%d\\region\\%s", slot + 1, SAVE_REGION_FILE_NAME);
}

void SavePath_RegionFile(int slot, char *path)
{
    /* Legacy V29/V33 full-window fallback path.  V34 writes the newer
       chunk-addressed r.<regionX>.<regionZ>.mcr files below, but keeping
       this path lets old saves load. */
    sprintf(path, "saves\\World%d\\region\\r_%d_%d.cmr",
            slot + 1, worldOriginBlockX, worldOriginBlockZ);
}

void SavePath_RegionFileV34(int slot, int regionX, int regionZ, char *path)
{
    sprintf(path, "saves\\World%d\\region\\r.%d.%d.mcr",
            slot + 1, regionX, regionZ);
}

void SaveHandler_DeleteRegionFilesV34(int slot)
{
    char path[260];
    int rx;
    int rz;
    if (slot < 0 || slot >= MAX_WORLD_SLOTS) { return; }
    for (rx = -8; rx <= 8; rx++) {
        for (rz = -8; rz <= 8; rz++) {
            SavePath_RegionFileV34(slot, rx, rz, path);
            remove(path);
        }
    }
}

void SaveHandler_CreateWorldLayout(int slot)
{
    char path[260];
    EnsureSaveDirectory();
    SavePath_WorldDir(slot, path);
    CreateDirectory(path, NULL);
    SavePath_RegionDir(slot, path);
    CreateDirectory(path, NULL);
}

int SaveHandler_SafeCommit(const char *tmpPath, const char *finalPath)
{
    char bakPath[300];
    sprintf(bakPath, "%s.bak", finalPath);
    remove(bakPath);
    rename(finalPath, bakPath);
    if (rename(tmpPath, finalPath) != 0) {
        rename(bakPath, finalPath);
        return 0;
    }
    remove(bakPath);
    return 1;
}

void CmcWriteU8(CmcNbtWriter *w, int v)
{
    if (!w->ok) { return; }
    if (fputc(v & 255, w->f) == EOF) { w->ok = 0; }
}

void CmcWriteU16(CmcNbtWriter *w, int v)
{
    CmcWriteU8(w, (v >> 8) & 255);
    CmcWriteU8(w, v & 255);
}

void CmcWriteI32(CmcNbtWriter *w, int v)
{
    unsigned long u;
    u = (unsigned long)v;
    CmcWriteU8(w, (int)((u >> 24) & 255));
    CmcWriteU8(w, (int)((u >> 16) & 255));
    CmcWriteU8(w, (int)((u >> 8) & 255));
    CmcWriteU8(w, (int)(u & 255));
}

void CmcWriteDouble(CmcNbtWriter *w, double d)
{
    union { double d; unsigned char b[8]; } u;
    int i;
    u.d = d;
    for (i = 7; i >= 0; i--) { CmcWriteU8(w, u.b[i]); }
}

void CmcWriteBytes(CmcNbtWriter *w, const unsigned char *data, int len)
{
    if (!w->ok) { return; }
    if (len > 0 && fwrite(data, 1, len, w->f) != (size_t)len) { w->ok = 0; }
}

void CmcWriteUtfRaw(CmcNbtWriter *w, const char *s)
{
    int len;
    if (!s) { s = ""; }
    len = (int)strlen(s);
    if (len > 65535) { len = 65535; }
    CmcWriteU16(w, len);
    CmcWriteBytes(w, (const unsigned char *)s, len);
}

void CmcWriteTagHeader(CmcNbtWriter *w, int type, const char *name)
{
    CmcWriteU8(w, type);
    if (type != 0) { CmcWriteUtfRaw(w, name); }
}

void CmcWriteBeginFile(CmcNbtWriter *w, FILE *f, const char *rootName)
{
    w->f = f;
    w->ok = 1;
    CmcWriteBytes(w, (const unsigned char *)CMC_NBT_MAGIC, 4);
    CmcWriteU8(w, CMC_SAVE_VERSION);
    CmcWriteTagHeader(w, 10, rootName);
}

void CmcWriteEndCompound(CmcNbtWriter *w)
{
    CmcWriteU8(w, 0);
}

void CmcWriteIntTag(CmcNbtWriter *w, const char *name, int v)
{
    CmcWriteTagHeader(w, 3, name);
    CmcWriteI32(w, v);
}

void CmcWriteDoubleTag(CmcNbtWriter *w, const char *name, double v)
{
    CmcWriteTagHeader(w, 6, name);
    CmcWriteDouble(w, v);
}

void CmcWriteStringTag(CmcNbtWriter *w, const char *name, const char *v)
{
    CmcWriteTagHeader(w, 8, name);
    CmcWriteUtfRaw(w, v);
}

void CmcWriteByteArrayTag(CmcNbtWriter *w, const char *name, const unsigned char *data, int len)
{
    CmcWriteTagHeader(w, 7, name);
    CmcWriteI32(w, len);
    CmcWriteBytes(w, data, len);
}

int CmcReadU8(CmcNbtReader *r)
{
    int c;
    if (!r->ok) { return 0; }
    c = fgetc(r->f);
    if (c == EOF) { r->ok = 0; return 0; }
    return c & 255;
}

int CmcReadU16(CmcNbtReader *r)
{
    int a;
    int b;
    a = CmcReadU8(r);
    b = CmcReadU8(r);
    return (a << 8) | b;
}

int CmcReadInt(CmcNbtReader *r);

int CmcReadI32(CmcNbtReader *r)
{
    unsigned long a;
    unsigned long b;
    unsigned long c;
    unsigned long d;
    a = (unsigned long)CmcReadU8(r);
    b = (unsigned long)CmcReadU8(r);
    c = (unsigned long)CmcReadU8(r);
    d = (unsigned long)CmcReadU8(r);
    return (int)((a << 24) | (b << 16) | (c << 8) | d);
}

int CmcReadInt(CmcNbtReader *r)
{
    /* Open Watcom V25 compile fix: older patches accidentally called
       CmcReadInt() even though the NBT helper was named CmcReadI32().
       Keep this alias so existing save-loading code links and still reads
       Java/NBT-style big-endian 32-bit TAG_Int payloads. */
    return CmcReadI32(r);
}

double CmcReadDouble(CmcNbtReader *r)
{
    union { double d; unsigned char b[8]; } u;
    int i;
    for (i = 7; i >= 0; i--) { u.b[i] = (unsigned char)CmcReadU8(r); }
    return u.d;
}

void CmcSkipBytes(CmcNbtReader *r, int len)
{
    int i;
    for (i = 0; i < len && r->ok; i++) { CmcReadU8(r); }
}

void CmcReadUtfPayload(CmcNbtReader *r, char *out, int maxLen)
{
    int len;
    int copy;
    int i;
    len = CmcReadU16(r);
    copy = len;
    if (copy >= maxLen) { copy = maxLen - 1; }
    if (copy < 0) { copy = 0; }
    for (i = 0; i < copy; i++) { out[i] = (char)CmcReadU8(r); }
    out[copy] = 0;
    CmcSkipBytes(r, len - copy);
}

void CmcReadTagName(CmcNbtReader *r, char *name, int maxLen)
{
    CmcReadUtfPayload(r, name, maxLen);
}

int CmcReadBeginFile(CmcNbtReader *r, FILE *f, const char *rootName)
{
    char magic[5];
    char name[64];
    int version;
    int type;
    r->f = f;
    r->ok = 1;
    if (fread(magic, 1, 4, f) != 4) { return 0; }
    magic[4] = 0;
    if (strcmp(magic, CMC_NBT_MAGIC) != 0) { return 0; }
    version = CmcReadU8(r);
    if (version < 1 || version > CMC_SAVE_VERSION) { return 0; }
    type = CmcReadU8(r);
    if (type != 10) { return 0; }
    CmcReadTagName(r, name, sizeof(name));
    if (!r->ok || strcmp(name, rootName) != 0) { return 0; }
    return 1;
}

void CmcSkipPayload(CmcNbtReader *r, int type)
{
    int len;
    int i;
    int t;
    char name[80];
    if (!r->ok) { return; }
    if (type == 1) { CmcSkipBytes(r, 1); }
    else if (type == 2) { CmcSkipBytes(r, 2); }
    else if (type == 3) { CmcSkipBytes(r, 4); }
    else if (type == 4) { CmcSkipBytes(r, 8); }
    else if (type == 5) { CmcSkipBytes(r, 4); }
    else if (type == 6) { CmcSkipBytes(r, 8); }
    else if (type == 7) { len = CmcReadI32(r); CmcSkipBytes(r, len); }
    else if (type == 8) { len = CmcReadU16(r); CmcSkipBytes(r, len); }
    else if (type == 9) {
        t = CmcReadU8(r);
        len = CmcReadI32(r);
        for (i = 0; i < len; i++) { CmcSkipPayload(r, t); }
    } else if (type == 10) {
        while (r->ok) {
            t = CmcReadU8(r);
            if (t == 0) { break; }
            CmcReadTagName(r, name, sizeof(name));
            CmcSkipPayload(r, t);
        }
    }
}

void CmcPutI32(unsigned char *p, int v)
{
    unsigned long u;
    u = (unsigned long)v;
    p[0] = (unsigned char)((u >> 24) & 255);
    p[1] = (unsigned char)((u >> 16) & 255);
    p[2] = (unsigned char)((u >> 8) & 255);
    p[3] = (unsigned char)(u & 255);
}

int CmcGetI32(const unsigned char *p)
{
    unsigned long u;
    u = ((unsigned long)p[0] << 24) | ((unsigned long)p[1] << 16) | ((unsigned long)p[2] << 8) | (unsigned long)p[3];
    return (int)u;
}

void CmcPutDouble(unsigned char *p, double d)
{
    union { double d; unsigned char b[8]; } u;
    int i;
    u.d = d;
    for (i = 0; i < 8; i++) { p[i] = u.b[7 - i]; }
}

double CmcGetDouble(const unsigned char *p)
{
    union { double d; unsigned char b[8]; } u;
    int i;
    for (i = 0; i < 8; i++) { u.b[7 - i] = p[i]; }
    return u.d;
}

void SaveSlotToBytes(InventorySlot *slot, unsigned char *p)
{
    CmcPutI32(p + 0, slot->item);
    CmcPutI32(p + 4, slot->count);
    CmcPutI32(p + 8, slot->damage);
}

void LoadSlotFromBytes(InventorySlot *slot, const unsigned char *p)
{
    slot->item = CmcGetI32(p + 0);
    slot->count = CmcGetI32(p + 4);
    slot->damage = CmcGetI32(p + 8);
    if (slot->count <= 0) { slot->item = ITEM_NONE; slot->count = 0; slot->damage = 0; }
    if (slot->count > MAX_STACK) { slot->count = MAX_STACK; }
}

void SaveSlotsToByteArray(InventorySlot *slots, int count, unsigned char *data)
{
    int i;
    for (i = 0; i < count; i++) { SaveSlotToBytes(&slots[i], data + i * SAVE_INV_SLOT_BYTES); }
}

void LoadSlotsFromByteArray(InventorySlot *slots, int count, const unsigned char *data, int len)
{
    int i;
    int maxSlots;
    maxSlots = len / SAVE_INV_SLOT_BYTES;
    if (maxSlots > count) { maxSlots = count; }
    for (i = 0; i < maxSlots; i++) { LoadSlotFromBytes(&slots[i], data + i * SAVE_INV_SLOT_BYTES); }
}

int SaveHandler_SaveWorldInfoV2(int slot)
{
    FILE *f;
    CmcNbtWriter w;
    char path[260];
    char tmp[300];
    WorldSaveInfo *info;
    if (slot < 0 || slot >= MAX_WORLD_SLOTS) { return 0; }
    info = &worldSaves[slot];
    if (!info->exists) { return 0; }
    SaveHandler_CreateWorldLayout(slot);
    SavePath_LevelDat(slot, path);
    sprintf(tmp, "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f) { return 0; }
    CmcWriteBeginFile(&w, f, "Data");
    CmcWriteStringTag(&w, "LevelName", info->name);
    CmcWriteStringTag(&w, "SeedText", info->seedText);
    CmcWriteIntTag(&w, "RandomSeed", info->seed);
    CmcWriteIntTag(&w, "SpawnX", (int)info->playerGlobalX);
    CmcWriteIntTag(&w, "SpawnY", (int)info->playerY);
    CmcWriteIntTag(&w, "SpawnZ", (int)info->playerGlobalZ);
    CmcWriteIntTag(&w, "WorldSize", info->worldSize);
    CmcWriteIntTag(&w, "SaveVersion", CMC_SAVE_VERSION);
    CmcWriteIntTag(&w, "WindowOriginX", worldOriginBlockX);
    CmcWriteIntTag(&w, "WindowOriginZ", worldOriginBlockZ);
    CmcWriteDoubleTag(&w, "PlayerGlobalX", info->playerGlobalX);
    CmcWriteDoubleTag(&w, "PlayerY", info->playerY);
    CmcWriteDoubleTag(&w, "PlayerGlobalZ", info->playerGlobalZ);
    CmcWriteDoubleTag(&w, "Time", info->worldTime);
    CmcWriteEndCompound(&w);
    fclose(f);
    if (!w.ok) { remove(tmp); return 0; }
    return SaveHandler_SafeCommit(tmp, path);
}

int SaveHandler_LoadWorldInfoV2(int slot, WorldSaveInfo *info)
{
    FILE *f;
    CmcNbtReader r;
    char path[260];
    char name[80];
    int type;
    if (!info || slot < 0 || slot >= MAX_WORLD_SLOTS) { return 0; }
    SavePath_LevelDat(slot, path);
    f = fopen(path, "rb");
    if (!f) { return 0; }

    ZeroMemory(info, sizeof(WorldSaveInfo));
    info->seed = DEFAULT_WORLDGEN_SEED;
    info->worldSize = WORLD_SIZE_INFINITE;
    strcpy(info->seedText, "173773");
    info->playerGlobalX = 0.5;
    info->playerY = 80.0;
    info->playerGlobalZ = 0.5;
    info->worldTime = 300.0;
    if (!CmcReadBeginFile(&r, f, "Data")) { fclose(f); return 0; }
    while (r.ok) {
        type = CmcReadU8(&r);
        if (type == 0) { break; }
        CmcReadTagName(&r, name, sizeof(name));
        if (type == 8 && strcmp(name, "LevelName") == 0) { CmcReadUtfPayload(&r, info->name, WORLD_NAME_LEN); }
        else if (type == 8 && strcmp(name, "SeedText") == 0) { CmcReadUtfPayload(&r, info->seedText, WORLD_SEED_LEN); }
        else if (type == 3 && strcmp(name, "RandomSeed") == 0) { info->seed = CmcReadI32(&r); }
        else if (type == 3 && strcmp(name, "WorldSize") == 0) { info->worldSize = CmcReadI32(&r); }
        else if (type == 6 && strcmp(name, "PlayerGlobalX") == 0) { info->playerGlobalX = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "PlayerY") == 0) { info->playerY = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "PlayerGlobalZ") == 0) { info->playerGlobalZ = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "Time") == 0) { info->worldTime = CmcReadDouble(&r); }
        else { CmcSkipPayload(&r, type); }
    }
    fclose(f);
    if (!r.ok) { return 0; }
    if (info->name[0] == 0) { sprintf(info->name, "World %d", slot + 1); }
    if (info->seedText[0] == 0) { sprintf(info->seedText, "%d", info->seed); }
    if (info->worldSize < 0) { info->worldSize = WORLD_SIZE_INFINITE; }
    if (info->worldSize > 0 && info->worldSize < FINITE_WORLD_SIZE_SMALL) { info->worldSize = FINITE_WORLD_SIZE_SMALL; }
    info->exists = 1;
    return 1;
}

int SaveHandler_SavePlayerV2(void)
{
    FILE *f;
    CmcNbtWriter w;
    char path[260];
    char tmp[300];
    unsigned char invData[INVENTORY_SLOTS * SAVE_INV_SLOT_BYTES];
    unsigned char hotData[HOTBAR_SLOTS * SAVE_INV_SLOT_BYTES];
    unsigned char armorData[4 * SAVE_INV_SLOT_BYTES];
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    SaveHandler_CreateWorldLayout(currentWorldSlot);
    SaveSlotsToByteArray(hotbar, HOTBAR_SLOTS, hotData);
    SaveSlotsToByteArray(inventory, INVENTORY_SLOTS, invData);
    SaveSlotsToByteArray(g_armorSlotsV6, 4, armorData);
    SavePath_PlayerDat(currentWorldSlot, path);
    sprintf(tmp, "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f) { return 0; }
    CmcWriteBeginFile(&w, f, "Player");
    CmcWriteIntTag(&w, "Health", playerHealth);
    CmcWriteIntTag(&w, "PrevHealth", playerPrevHealth);
    CmcWriteIntTag(&w, "SelectedHotbar", selectedHotbarSlot);
    CmcWriteDoubleTag(&w, "HeartsLife", playerHeartsLife);
    CmcWriteDoubleTag(&w, "Yaw", yaw);
    CmcWriteDoubleTag(&w, "Pitch", pitch);
    CmcWriteDoubleTag(&w, "Air", g_playerAirTimer);
    CmcWriteDoubleTag(&w, "GlobalX", GetPlayerGlobalX());
    CmcWriteDoubleTag(&w, "Y", playerY);
    CmcWriteDoubleTag(&w, "GlobalZ", GetPlayerGlobalZ());
    CmcWriteDoubleTag(&w, "VelocityX", velocityX);
    CmcWriteDoubleTag(&w, "VelocityY", velocityY);
    CmcWriteDoubleTag(&w, "VelocityZ", velocityZ);
    CmcWriteDoubleTag(&w, "FallDistance", g_playerFallDistanceV22);
    CmcWriteIntTag(&w, "Sneaking", g_playerSneakingV22);
    CmcWriteByteArrayTag(&w, "Hotbar", hotData, sizeof(hotData));
    CmcWriteByteArrayTag(&w, "Inventory", invData, sizeof(invData));
    CmcWriteByteArrayTag(&w, "Armor", armorData, sizeof(armorData));
    CmcWriteEndCompound(&w);
    fclose(f);
    if (!w.ok) { remove(tmp); return 0; }
    return SaveHandler_SafeCommit(tmp, path);
}

int SaveHandler_LoadPlayerV2(void)
{
    FILE *f;
    CmcNbtReader r;
    char path[260];
    char name[80];
    unsigned char data[INVENTORY_SLOTS * SAVE_INV_SLOT_BYTES];
    int type;
    int len;
    int copy;
    double gx;
    double gy;
    double gz;
    gx = g_startGlobalX;
    gy = g_startPlayerY;
    gz = g_startGlobalZ;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    SavePath_PlayerDat(currentWorldSlot, path);
    f = fopen(path, "rb");
    if (!f) { return 0; }
    if (!CmcReadBeginFile(&r, f, "Player")) { fclose(f); return 0; }
    while (r.ok) {
        type = CmcReadU8(&r);
        if (type == 0) { break; }
        CmcReadTagName(&r, name, sizeof(name));
        if (type == 3 && strcmp(name, "Health") == 0) { playerHealth = CmcReadI32(&r); }
        else if (type == 3 && strcmp(name, "PrevHealth") == 0) { playerPrevHealth = CmcReadI32(&r); }
        else if (type == 3 && strcmp(name, "SelectedHotbar") == 0) { selectedHotbarSlot = CmcReadI32(&r); }
        else if (type == 6 && strcmp(name, "HeartsLife") == 0) { playerHeartsLife = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "Yaw") == 0) { yaw = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "Pitch") == 0) { pitch = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "Air") == 0) { g_playerAirTimer = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "GlobalX") == 0) { gx = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "Y") == 0) { gy = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "GlobalZ") == 0) { gz = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "VelocityX") == 0) { velocityX = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "VelocityY") == 0) { velocityY = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "VelocityZ") == 0) { velocityZ = CmcReadDouble(&r); }
        else if (type == 6 && strcmp(name, "FallDistance") == 0) { g_playerFallDistanceV22 = CmcReadDouble(&r); }
        else if (type == 3 && strcmp(name, "Sneaking") == 0) { g_playerSneakingV22 = CmcReadInt(&r); }
        else if (type == 2 && strcmp(name, "Sneaking") == 0) { g_playerSneakingV22 = CmcReadU16(&r) ? 1 : 0; }
        else if (type == 7 && strcmp(name, "Hotbar") == 0) {
            len = CmcReadI32(&r);
            copy = len;
            if (copy > (int)sizeof(data)) { copy = sizeof(data); }
            fread(data, 1, copy, r.f);
            CmcSkipBytes(&r, len - copy);
            LoadSlotsFromByteArray(hotbar, HOTBAR_SLOTS, data, copy);
        } else if (type == 7 && strcmp(name, "Inventory") == 0) {
            len = CmcReadI32(&r);
            copy = len;
            if (copy > (int)sizeof(data)) { copy = sizeof(data); }
            fread(data, 1, copy, r.f);
            CmcSkipBytes(&r, len - copy);
            LoadSlotsFromByteArray(inventory, INVENTORY_SLOTS, data, copy);
        } else if (type == 7 && strcmp(name, "Armor") == 0) {
            len = CmcReadI32(&r);
            copy = len;
            if (copy > (int)sizeof(data)) { copy = sizeof(data); }
            fread(data, 1, copy, r.f);
            CmcSkipBytes(&r, len - copy);
            LoadSlotsFromByteArray(g_armorSlotsV6, 4, data, copy);
        } else { CmcSkipPayload(&r, type); }
    }
    fclose(f);
    if (playerHealth < 0) { playerHealth = 0; }
    if (playerHealth > MAX_HEALTH) { playerHealth = MAX_HEALTH; }
    if (selectedHotbarSlot < 0) { selectedHotbarSlot = 0; }
    if (selectedHotbarSlot >= HOTBAR_SLOTS) { selectedHotbarSlot = HOTBAR_SLOTS - 1; }
    playerX = gx - (double)worldOriginBlockX;
    playerY = gy;
    playerZ = gz - (double)worldOriginBlockZ;
    return r.ok;
}

void SaveMobToBytes(Mob *m, unsigned char *p)
{
    CmcPutI32(p + 0, m->type);
    CmcPutI32(p + 4, m->health);
    CmcPutI32(p + 8, m->angry);
    CmcPutI32(p + 12, m->sheared);
    CmcPutI32(p + 16, m->burning);
    CmcPutI32(p + 20, m->deathDropsDone);
    CmcPutDouble(p + 24, (double)worldOriginBlockX + m->x);
    CmcPutDouble(p + 32, m->y);
    CmcPutDouble(p + 40, (double)worldOriginBlockZ + m->z);
    CmcPutDouble(p + 48, m->vx);
    CmcPutDouble(p + 56, m->vy);
    CmcPutDouble(p + 64, m->vz);
    CmcPutDouble(p + 72, m->yaw);
    CmcPutDouble(p + 80, m->fuseTimer);
    CmcPutDouble(p + 88, m->hurtTime);
    CmcPutDouble(p + 96, m->fallDistance);
    CmcPutDouble(p + 104, m->airTimer);
    CmcPutDouble(p + 112, m->fireTimer);
    CmcPutI32(p + 120, m->inWater);
    CmcPutI32(p + 124, m->inLava);
    CmcPutDouble(p + 128, m->targetLostTimer);
    CmcPutDouble(p + 136, m->entityAge);
    CmcPutDouble(p + 144, m->knockbackX);
    CmcPutDouble(p + 152, m->knockbackY);
    CmcPutDouble(p + 160, m->knockbackZ);
    CmcPutDouble(p + 168, m->knockbackTimer);
    CmcPutDouble(p + 176, m->pathFailTimer);
    CmcPutI32(p + 184, m->lastTargetVisible);
    CmcPutI32(p + 188, m->persistent);
}

void SaveDropToBytes(DroppedItem *d, unsigned char *p)
{
    CmcPutI32(p + 0, d->item);
    CmcPutI32(p + 4, d->count);
    CmcPutDouble(p + 8, (double)worldOriginBlockX + d->x);
    CmcPutDouble(p + 16, d->y);
    CmcPutDouble(p + 24, (double)worldOriginBlockZ + d->z);
    CmcPutDouble(p + 32, d->vx);
    CmcPutDouble(p + 40, d->vy);
    CmcPutDouble(p + 48, d->vz);
    CmcPutDouble(p + 56, d->age);
    CmcPutDouble(p + 64, d->spin);
    CmcPutI32(p + 72, d->damage);
    CmcPutI32(p + 76, d->health);
}

/* V44_SPECIAL_ENTITIES: Java EntityArrow/Boat/Minecart/Painting/Fish/TNT
   persistence record.  The global X/Z storage follows V34's region-save
   convention so moving the active window does not shift special entities. */
void SaveSpecialToBytesV44(SpecialEntityV6 *e, unsigned char *p)
{
    CmcPutI32(p + 0, e->type);
    CmcPutI32(p + 4, e->item);
    CmcPutI32(p + 8, e->block);
    CmcPutI32(p + 12, e->damage);
    CmcPutI32(p + 16, e->stuck);
    CmcPutI32(p + 20, e->meta);
    CmcPutDouble(p + 24, e->x + (double)worldOriginBlockX);
    CmcPutDouble(p + 32, e->y);
    CmcPutDouble(p + 40, e->z + (double)worldOriginBlockZ);
    CmcPutDouble(p + 48, e->vx);
    CmcPutDouble(p + 56, e->vy);
    CmcPutDouble(p + 64, e->vz);
    CmcPutDouble(p + 72, e->yaw);
    CmcPutDouble(p + 80, e->pitch);
    CmcPutDouble(p + 88, e->age);
    CmcPutDouble(p + 96, e->life);
    CmcPutDouble(p + 104, e->fuse);
    CmcPutDouble(p + 112, 0.0);
    CmcPutDouble(p + 120, 0.0);
    CmcPutI32(p + 128, 0);
    CmcPutI32(p + 132, 0);
}

int LoadSpecialFromBytesV44(unsigned char *p, int bytes)
{
    int idx;
    int type;
    int item;
    int block;
    double gx;
    double gz;
    if (!p || bytes < 112) { return -1; }
    type = CmcGetI32(p + 0);
    item = CmcGetI32(p + 4);
    block = CmcGetI32(p + 8);
    gx = CmcGetDouble(p + 24);
    gz = CmcGetDouble(p + 40);
    idx = SpawnSpecialEntityV6(type, item, block,
                               gx - (double)worldOriginBlockX,
                               CmcGetDouble(p + 32),
                               gz - (double)worldOriginBlockZ,
                               CmcGetDouble(p + 48),
                               CmcGetDouble(p + 56),
                               CmcGetDouble(p + 64),
                               CmcGetDouble(p + 104));
    if (idx >= 0) {
        g_specialEntitiesV6[idx].damage = CmcGetI32(p + 12);
        g_specialEntitiesV6[idx].stuck = CmcGetI32(p + 16);
        g_specialEntitiesV6[idx].meta = CmcGetI32(p + 20);
        g_specialEntitiesV6[idx].yaw = CmcGetDouble(p + 72);
        g_specialEntitiesV6[idx].pitch = CmcGetDouble(p + 80);
        g_specialEntitiesV6[idx].age = CmcGetDouble(p + 88);
        g_specialEntitiesV6[idx].life = CmcGetDouble(p + 96);
    }
    return idx;
}

int SaveHandler_SaveEntitiesV2(void)
{
    FILE *f;
    CmcNbtWriter w;
    char path[260];
    char tmp[300];
    unsigned char *mobData;
    unsigned char *dropData;
    unsigned char *specialData;
    int i;
    int mobCount;
    int dropCount;
    int specialCount;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    SaveHandler_CreateWorldLayout(currentWorldSlot);
    mobCount = 0;
    dropCount = 0;
    specialCount = 0;
    for (i = 0; i < MAX_MOBS; i++) { if (mobs[i].active) { mobCount++; } }
    for (i = 0; i < MAX_DROPPED_ITEMS; i++) { if (droppedItems[i].active) { dropCount++; } }
    for (i = 0; i < MAX_SPECIAL_ENTITIES_V6; i++) { if (g_specialEntitiesV6[i].active) { specialCount++; } }
    mobData = (unsigned char *)malloc(mobCount * SAVE_MOB_RECORD_BYTES + 1);
    dropData = (unsigned char *)malloc(dropCount * SAVE_DROP_RECORD_BYTES + 1);
    specialData = (unsigned char *)malloc(specialCount * SAVE_SPECIAL_RECORD_BYTES_V44 + 1);
    if (!mobData || !dropData || !specialData) { if (mobData) { free(mobData); } if (dropData) { free(dropData); } if (specialData) { free(specialData); } return 0; }
    mobCount = 0;
    for (i = 0; i < MAX_MOBS; i++) { if (mobs[i].active) { SaveMobToBytes(&mobs[i], mobData + mobCount * SAVE_MOB_RECORD_BYTES); mobCount++; } }
    dropCount = 0;
    for (i = 0; i < MAX_DROPPED_ITEMS; i++) { if (droppedItems[i].active) { SaveDropToBytes(&droppedItems[i], dropData + dropCount * SAVE_DROP_RECORD_BYTES); dropCount++; } }
    specialCount = 0;
    for (i = 0; i < MAX_SPECIAL_ENTITIES_V6; i++) { if (g_specialEntitiesV6[i].active) { SaveSpecialToBytesV44(&g_specialEntitiesV6[i], specialData + specialCount * SAVE_SPECIAL_RECORD_BYTES_V44); specialCount++; } }
    SavePath_EntitiesDat(currentWorldSlot, path);
    sprintf(tmp, "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f) { free(mobData); free(dropData); free(specialData); return 0; }
    CmcWriteBeginFile(&w, f, "Entities");
    CmcWriteIntTag(&w, "MobRecordBytes", SAVE_MOB_RECORD_BYTES);
    CmcWriteIntTag(&w, "MobCount", mobCount);
    CmcWriteByteArrayTag(&w, "Mobs", mobData, mobCount * SAVE_MOB_RECORD_BYTES);
    CmcWriteIntTag(&w, "DropRecordBytes", SAVE_DROP_RECORD_BYTES);
    CmcWriteIntTag(&w, "DropCount", dropCount);
    CmcWriteByteArrayTag(&w, "Drops", dropData, dropCount * SAVE_DROP_RECORD_BYTES);
    CmcWriteIntTag(&w, "SpecialRecordBytes", SAVE_SPECIAL_RECORD_BYTES_V44);
    CmcWriteIntTag(&w, "SpecialCount", specialCount);
    CmcWriteByteArrayTag(&w, "Specials", specialData, specialCount * SAVE_SPECIAL_RECORD_BYTES_V44);
    CmcWriteEndCompound(&w);
    fclose(f);
    free(mobData);
    free(dropData);
    free(specialData);
    if (!w.ok) { remove(tmp); return 0; }
    return SaveHandler_SafeCommit(tmp, path);
}

int SaveHandler_LoadEntitiesV2(void)
{
    FILE *f;
    CmcNbtReader r;
    char path[260];
    char name[80];
    unsigned char *mobData;
    unsigned char *dropData;
    unsigned char *specialData;
    unsigned char *rec;
    int type;
    int len;
    int mobRecordBytes;
    int dropRecordBytes;
    int specialRecordBytes;
    int mobBytes;
    int dropBytes;
    int specialBytes;
    int i;
    int count;
    int idx;
    int dropDamage;
    int dropHealth;
    double gx;
    double gz;
    mobData = NULL;
    dropData = NULL;
    specialData = NULL;
    mobRecordBytes = SAVE_MOB_RECORD_BYTES;
    dropRecordBytes = SAVE_DROP_RECORD_BYTES;
    specialRecordBytes = SAVE_SPECIAL_RECORD_BYTES_V44;
    mobBytes = 0;
    dropBytes = 0;
    specialBytes = 0;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    SavePath_EntitiesDat(currentWorldSlot, path);
    f = fopen(path, "rb");
    if (!f) { return 0; }
    if (!CmcReadBeginFile(&r, f, "Entities")) { fclose(f); return 0; }
    while (r.ok) {
        type = CmcReadU8(&r);
        if (type == 0) { break; }
        CmcReadTagName(&r, name, sizeof(name));
        if (type == 3 && strcmp(name, "MobRecordBytes") == 0) { mobRecordBytes = CmcReadI32(&r); }
        else if (type == 3 && strcmp(name, "DropRecordBytes") == 0) { dropRecordBytes = CmcReadI32(&r); }
        else if (type == 3 && strcmp(name, "SpecialRecordBytes") == 0) { specialRecordBytes = CmcReadI32(&r); }
        else if (type == 7 && strcmp(name, "Mobs") == 0) {
            len = CmcReadI32(&r);
            mobData = (unsigned char *)malloc(len + 1);
            if (!mobData) { fclose(f); return 0; }
            if (fread(mobData, 1, len, r.f) != (size_t)len) { r.ok = 0; }
            mobBytes = len;
        } else if (type == 7 && strcmp(name, "Drops") == 0) {
            len = CmcReadI32(&r);
            dropData = (unsigned char *)malloc(len + 1);
            if (!dropData) { fclose(f); if (mobData) { free(mobData); } return 0; }
            if (fread(dropData, 1, len, r.f) != (size_t)len) { r.ok = 0; }
            dropBytes = len;
        } else if (type == 7 && strcmp(name, "Specials") == 0) {
            len = CmcReadI32(&r);
            specialData = (unsigned char *)malloc(len + 1);
            if (!specialData) { fclose(f); if (mobData) { free(mobData); } if (dropData) { free(dropData); } return 0; }
            if (fread(specialData, 1, len, r.f) != (size_t)len) { r.ok = 0; }
            specialBytes = len;
        } else { CmcSkipPayload(&r, type); }
    }
    fclose(f);
    if (!r.ok) { if (mobData) { free(mobData); } if (dropData) { free(dropData); } if (specialData) { free(specialData); } return 0; }
    InitMobs();
    InitDroppedItems();
    memset(g_specialEntitiesV6, 0, sizeof(g_specialEntitiesV6));
    g_fishingHookIndexV6 = -1;
    if (mobData && mobRecordBytes >= SAVE_MOB_RECORD_BYTES) {
        count = mobBytes / mobRecordBytes;
        if (count > MAX_MOBS) { count = MAX_MOBS; }
        for (i = 0; i < count; i++) {
            rec = mobData + i * mobRecordBytes;
            gx = CmcGetDouble(rec + 24);
            gz = CmcGetDouble(rec + 40);
            idx = AddMob(CmcGetI32(rec + 0), gx - (double)worldOriginBlockX, CmcGetDouble(rec + 32), gz - (double)worldOriginBlockZ);
            if (idx >= 0) {
                mobs[idx].health = CmcGetI32(rec + 4);
                mobs[idx].angry = CmcGetI32(rec + 8);
                mobs[idx].sheared = CmcGetI32(rec + 12);
                mobs[idx].burning = CmcGetI32(rec + 16);
                mobs[idx].deathDropsDone = CmcGetI32(rec + 20);
                mobs[idx].vx = CmcGetDouble(rec + 48);
                mobs[idx].vy = CmcGetDouble(rec + 56);
                mobs[idx].vz = CmcGetDouble(rec + 64);
                mobs[idx].yaw = CmcGetDouble(rec + 72);
                mobs[idx].fuseTimer = CmcGetDouble(rec + 80);
                mobs[idx].hurtTime = CmcGetDouble(rec + 88);
                if (mobRecordBytes >= 128) {
                    mobs[idx].fallDistance = CmcGetDouble(rec + 96);
                    mobs[idx].airTimer = CmcGetDouble(rec + 104);
                    mobs[idx].fireTimer = CmcGetDouble(rec + 112);
                    mobs[idx].inWater = CmcGetI32(rec + 120);
                    mobs[idx].inLava = CmcGetI32(rec + 124);
                }
                if (mobRecordBytes >= 192) {
                    mobs[idx].targetLostTimer = CmcGetDouble(rec + 128);
                    mobs[idx].entityAge = CmcGetDouble(rec + 136);
                    mobs[idx].knockbackX = CmcGetDouble(rec + 144);
                    mobs[idx].knockbackY = CmcGetDouble(rec + 152);
                    mobs[idx].knockbackZ = CmcGetDouble(rec + 160);
                    mobs[idx].knockbackTimer = CmcGetDouble(rec + 168);
                    mobs[idx].pathFailTimer = CmcGetDouble(rec + 176);
                    mobs[idx].lastTargetVisible = CmcGetI32(rec + 184);
                    mobs[idx].persistent = CmcGetI32(rec + 188);
                }
                mobs[idx].spawnGraceTimer = 0.25;
                mobs[idx].prevX = mobs[idx].x;
                mobs[idx].prevY = mobs[idx].y;
                mobs[idx].prevZ = mobs[idx].z;
                mobs[idx].prevYaw = mobs[idx].yaw;
                mobs[idx].prevRenderYawOffset = mobs[idx].renderYawOffset;
                mobs[idx].prevAnimWalk = mobs[idx].animWalk;
            }
        }
    }
    if (dropData && dropRecordBytes >= 72) {
        count = dropBytes / dropRecordBytes;
        if (count > MAX_DROPPED_ITEMS) { count = MAX_DROPPED_ITEMS; }
        for (i = 0; i < count; i++) {
            rec = dropData + i * dropRecordBytes;
            dropDamage = 0;
            dropHealth = 5;
            if (dropRecordBytes >= 80) { dropDamage = CmcGetI32(rec + 72); dropHealth = CmcGetI32(rec + 76); }
            idx = AddDroppedItemStackV38(CmcGetI32(rec + 0), CmcGetI32(rec + 4), dropDamage, CmcGetDouble(rec + 8) - (double)worldOriginBlockX,
                                 CmcGetDouble(rec + 16), CmcGetDouble(rec + 24) - (double)worldOriginBlockZ,
                                 CmcGetDouble(rec + 32), CmcGetDouble(rec + 40), CmcGetDouble(rec + 48));
            if (idx >= 0) { droppedItems[idx].age = CmcGetDouble(rec + 56); droppedItems[idx].spin = CmcGetDouble(rec + 64); droppedItems[idx].hoverStart = droppedItems[idx].spin / 57.29578; droppedItems[idx].health = dropHealth; droppedItems[idx].pickupDelay = 0.50; }
        }
    }
    if (specialData && specialRecordBytes >= SAVE_SPECIAL_RECORD_BYTES_V44) {
        count = specialBytes / specialRecordBytes;
        if (count > MAX_SPECIAL_ENTITIES_V6) { count = MAX_SPECIAL_ENTITIES_V6; }
        for (i = 0; i < count; i++) {
            idx = LoadSpecialFromBytesV44(specialData + i * specialRecordBytes, specialRecordBytes);
            if (idx >= 0 && g_specialEntitiesV6[idx].type == ENTITY_V6_FISH_HOOK) { g_fishingHookIndexV6 = idx; }
        }
    }
    if (mobData) { free(mobData); }
    if (dropData) { free(dropData); }
    if (specialData) { free(specialData); }
    return 1;
}
void SaveTileToBytesV34(TileEntityState *t, unsigned char *p, int globalCoords)
{
    int i;
    int sx;
    int sz;
    sx = t->x;
    sz = t->z;
    if (globalCoords) {
        sx += worldOriginBlockX;
        sz += worldOriginBlockZ;
    }
    CmcPutI32(p + 0, t->type);
    CmcPutI32(p + 4, sx);
    CmcPutI32(p + 8, t->y);
    CmcPutI32(p + 12, sz);
    CmcPutI32(p + 16, t->power);
    CmcPutDouble(p + 20, t->burnTime);
    CmcPutDouble(p + 28, t->cookTime);
    ZeroMemory(p + 36, 64);
    strncpy((char *)(p + 36), t->text, 63);
    for (i = 0; i < 27; i++) { SaveSlotToBytes(&t->slots[i], p + 100 + i * SAVE_INV_SLOT_BYTES); }
}

void SaveTileToBytes(TileEntityState *t, unsigned char *p)
{
    SaveTileToBytesV34(t, p, 0);
}

int LoadTileFromBytesV34(const unsigned char *rec, int recordBytes, int globalCoords)
{
    int tx;
    int ty;
    int tz;
    int type;
    int idx;
    int j;
    if (recordBytes < SAVE_TILE_RECORD_BYTES) { return 0; }
    type = CmcGetI32(rec + 0);
    tx = CmcGetI32(rec + 4);
    ty = CmcGetI32(rec + 8);
    tz = CmcGetI32(rec + 12);
    if (globalCoords) {
        tx -= worldOriginBlockX;
        tz -= worldOriginBlockZ;
    }
    if (!IsInsideWorld(tx, ty, tz)) { return 0; }
    EnsureTileEntityForBlock(type, tx, ty, tz);
    idx = FindTileEntityAt(tx, ty, tz);
    if (idx < 0) { return 0; }
    tileEntities[idx].type = type;
    tileEntities[idx].power = CmcGetI32(rec + 16);
    tileEntities[idx].burnTime = CmcGetDouble(rec + 20);
    tileEntities[idx].cookTime = CmcGetDouble(rec + 28);
    strncpy(tileEntities[idx].text, (char *)(rec + 36), 63);
    tileEntities[idx].text[63] = 0;
    for (j = 0; j < 27; j++) { LoadSlotFromBytes(&tileEntities[idx].slots[j], rec + 100 + j * SAVE_INV_SLOT_BYTES); }
    return 1;
}

int SaveHandler_SaveTileEntitiesV2(void)
{
    FILE *f;
    CmcNbtWriter w;
    char path[260];
    char tmp[300];
    unsigned char *data;
    int i;
    int count;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    count = 0;
    for (i = 0; i < MAX_TILE_ENTITIES; i++) { if (tileEntities[i].active) { count++; } }
    data = (unsigned char *)malloc(count * SAVE_TILE_RECORD_BYTES + 1);
    if (!data) { return 0; }
    count = 0;
    for (i = 0; i < MAX_TILE_ENTITIES; i++) { if (tileEntities[i].active) { SaveTileToBytesV34(&tileEntities[i], data + count * SAVE_TILE_RECORD_BYTES, 1); count++; } }
    SaveHandler_CreateWorldLayout(currentWorldSlot);
    SavePath_TileEntitiesDat(currentWorldSlot, path);
    sprintf(tmp, "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f) { free(data); return 0; }
    CmcWriteBeginFile(&w, f, "TileEntities");
    CmcWriteIntTag(&w, "RecordBytes", SAVE_TILE_RECORD_BYTES);
    CmcWriteIntTag(&w, "GlobalCoords", 1);
    CmcWriteIntTag(&w, "TileCount", count);
    CmcWriteByteArrayTag(&w, "Tiles", data, count * SAVE_TILE_RECORD_BYTES);
    CmcWriteEndCompound(&w);
    fclose(f);
    free(data);
    if (!w.ok) { remove(tmp); return 0; }
    return SaveHandler_SafeCommit(tmp, path);
}

int SaveHandler_LoadTileEntitiesV2(void)
{
    FILE *f;
    CmcNbtReader r;
    char path[260];
    char name[80];
    unsigned char *data;
    unsigned char *rec;
    int type;
    int recordBytes;
    int globalCoords;
    int bytes;
    int len;
    int count;
    int i;
    int j;
    int idx;
    data = NULL;
    recordBytes = SAVE_TILE_RECORD_BYTES;
    globalCoords = 0;
    bytes = 0;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    SavePath_TileEntitiesDat(currentWorldSlot, path);
    f = fopen(path, "rb");
    if (!f) { return 0; }
    if (!CmcReadBeginFile(&r, f, "TileEntities")) { fclose(f); return 0; }
    while (r.ok) {
        type = CmcReadU8(&r);
        if (type == 0) { break; }
        CmcReadTagName(&r, name, sizeof(name));
        if (type == 3 && strcmp(name, "RecordBytes") == 0) { recordBytes = CmcReadI32(&r); }
        else if (type == 3 && strcmp(name, "GlobalCoords") == 0) { globalCoords = CmcReadI32(&r); }
        else if (type == 7 && strcmp(name, "Tiles") == 0) {
            len = CmcReadI32(&r);
            data = (unsigned char *)malloc(len + 1);
            if (!data) { fclose(f); return 0; }
            if (fread(data, 1, len, r.f) != (size_t)len) { r.ok = 0; }
            bytes = len;
        } else { CmcSkipPayload(&r, type); }
    }
    fclose(f);
    if (!r.ok || !data || recordBytes < SAVE_TILE_RECORD_BYTES) { if (data) { free(data); } return 0; }
    ZeroMemory(tileEntities, sizeof(tileEntities));
    count = bytes / recordBytes;
    if (count > MAX_TILE_ENTITIES) { count = MAX_TILE_ENTITIES; }
    for (i = 0; i < count; i++) {
        rec = data + i * recordBytes;
        LoadTileFromBytesV34(rec, recordBytes, globalCoords);
    }
    free(data);
    return 1;
}

void Region_WriteByteArrayFromLight(FILE *f, unsigned char arr[WORLD_X][WORLD_Y][WORLD_Z], int x0, int z0)
{
    CmcNbtWriter w;
    int x;
    int y;
    int z;
    w.f = f;
    w.ok = 1;
    CmcWriteI32(&w, SAVE_CHUNK_BLOCK_COUNT);
    for (x = x0; x < x0 + CHUNK_SIZE; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = z0; z < z0 + CHUNK_SIZE; z++) { CmcWriteU8(&w, arr[x][y][z]); }
        }
    }
}

void Region_WriteBiomeBytes(FILE *f, int x0, int z0)
{
    CmcNbtWriter w;
    int x;
    int z;
    w.f = f;
    w.ok = 1;
    CmcWriteI32(&w, CHUNK_SIZE * CHUNK_SIZE);
    for (x = x0; x < x0 + CHUNK_SIZE; x++) {
        for (z = z0; z < z0 + CHUNK_SIZE; z++) { CmcWriteU8(&w, biomeMap[x][z]); }
    }
}

int Region_CountBlockRuns(int x0, int z0)
{
    int x;
    int y;
    int z;
    int runs;
    int last;
    int cur;
    int first;
    first = 1;
    runs = 0;
    last = 0;
    for (x = x0; x < x0 + CHUNK_SIZE; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = z0; z < z0 + CHUNK_SIZE; z++) {
                cur = world[x][y][z];
                if (first || cur != last) { runs++; last = cur; first = 0; }
            }
        }
    }
    return runs;
}

void Region_WriteBlockRuns(FILE *f, int x0, int z0)
{
    CmcNbtWriter w;
    int x;
    int y;
    int z;
    int runs;
    int last;
    int cur;
    int run;
    int first;
    w.f = f;
    w.ok = 1;
    runs = Region_CountBlockRuns(x0, z0);
    CmcWriteI32(&w, runs);
    first = 1;
    last = 0;
    run = 0;
    for (x = x0; x < x0 + CHUNK_SIZE; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = z0; z < z0 + CHUNK_SIZE; z++) {
                cur = world[x][y][z];
                if (first) { first = 0; last = cur; run = 1; }
                else if (cur == last && run < 2147483000) { run++; }
                else { CmcWriteI32(&w, last); CmcWriteI32(&w, run); last = cur; run = 1; }
            }
        }
    }
    if (!first) { CmcWriteI32(&w, last); CmcWriteI32(&w, run); }
}

int Region3_LocalIndexV34(int globalChunkX, int globalChunkZ)
{
    int rx;
    int rz;
    int lx;
    int lz;
    rx = FloorDivInt(globalChunkX, 32);
    rz = FloorDivInt(globalChunkZ, 32);
    lx = globalChunkX - rx * 32;
    lz = globalChunkZ - rz * 32;
    if (lx < 0 || lx >= 32 || lz < 0 || lz >= 32) { return -1; }
    return lx + lz * 32;
}

void Region3_ChunkRegionV34(int globalChunkX, int globalChunkZ, int *regionX, int *regionZ)
{
    *regionX = FloorDivInt(globalChunkX, 32);
    *regionZ = FloorDivInt(globalChunkZ, 32);
}

int Region3_ReadTableIntV34(FILE *f, long base, int index, int *out)
{
    CmcNbtReader r;
    if (index < 0 || index >= CMC_REGION3_ENTRY_COUNT) { return 0; }
    if (fseek(f, base + (long)index * 4L, SEEK_SET) != 0) { return 0; }
    r.f = f;
    r.ok = 1;
    *out = CmcReadI32(&r);
    return r.ok;
}

int Region3_WriteTableIntV34(FILE *f, long base, int index, int value)
{
    CmcNbtWriter w;
    if (index < 0 || index >= CMC_REGION3_ENTRY_COUNT) { return 0; }
    if (fseek(f, base + (long)index * 4L, SEEK_SET) != 0) { return 0; }
    w.f = f;
    w.ok = 1;
    CmcWriteI32(&w, value);
    return w.ok;
}

int Region3_WriteHeaderV34(FILE *f, int regionX, int regionZ)
{
    CmcNbtWriter w;
    int i;
    w.f = f;
    w.ok = 1;
    CmcWriteBytes(&w, (const unsigned char *)"CMR3", 4);
    CmcWriteI32(&w, CMC_REGION_VERSION);
    CmcWriteI32(&w, regionX);
    CmcWriteI32(&w, regionZ);
    for (i = 0; i < CMC_REGION3_ENTRY_COUNT * 3; i++) { CmcWriteI32(&w, 0); }
    return w.ok;
}

int Region3_ReadHeaderV34(FILE *f, int regionX, int regionZ)
{
    CmcNbtReader r;
    char magic[5];
    int version;
    int rx;
    int rz;
    if (fseek(f, 0L, SEEK_SET) != 0) { return 0; }
    if (fread(magic, 1, 4, f) != 4) { return 0; }
    magic[4] = 0;
    if (strcmp(magic, "CMR3") != 0) { return 0; }
    r.f = f;
    r.ok = 1;
    version = CmcReadI32(&r);
    rx = CmcReadI32(&r);
    rz = CmcReadI32(&r);
    if (!r.ok || version > CMC_REGION_VERSION || rx != regionX || rz != regionZ) { return 0; }
    return 1;
}

int Region3_EnsureFileV34(const char *path, int regionX, int regionZ)
{
    FILE *f;
    f = fopen(path, "rb");
    if (f) {
        if (Region3_ReadHeaderV34(f, regionX, regionZ)) { fclose(f); return 1; }
        fclose(f);
        remove(path);
    }
    f = fopen(path, "wb");
    if (!f) { return 0; }
    if (!Region3_WriteHeaderV34(f, regionX, regionZ)) { fclose(f); remove(path); return 0; }
    fclose(f);
    return 1;
}

void Region3_ClearTileEntitiesForChunkV34(int x0, int z0)
{
    int i;
    for (i = 0; i < MAX_TILE_ENTITIES; i++) {
        if (!tileEntities[i].active) { continue; }
        if (tileEntities[i].x >= x0 && tileEntities[i].x < x0 + CHUNK_SIZE &&
            tileEntities[i].z >= z0 && tileEntities[i].z < z0 + CHUNK_SIZE) {
            ZeroMemory(&tileEntities[i], sizeof(TileEntityState));
        }
    }
}

void Region3_WriteTileEntitiesForChunkV34(FILE *f, int x0, int z0)
{
    CmcNbtWriter w;
    unsigned char rec[SAVE_TILE_RECORD_BYTES];
    int i;
    int count;
    w.f = f;
    w.ok = 1;
    count = 0;
    for (i = 0; i < MAX_TILE_ENTITIES; i++) {
        if (!tileEntities[i].active) { continue; }
        if (tileEntities[i].x >= x0 && tileEntities[i].x < x0 + CHUNK_SIZE &&
            tileEntities[i].z >= z0 && tileEntities[i].z < z0 + CHUNK_SIZE) {
            count++;
        }
    }
    CmcWriteI32(&w, count);
    CmcWriteI32(&w, SAVE_TILE_RECORD_BYTES);
    for (i = 0; i < MAX_TILE_ENTITIES; i++) {
        if (!tileEntities[i].active) { continue; }
        if (tileEntities[i].x >= x0 && tileEntities[i].x < x0 + CHUNK_SIZE &&
            tileEntities[i].z >= z0 && tileEntities[i].z < z0 + CHUNK_SIZE) {
            SaveTileToBytesV34(&tileEntities[i], rec, 1);
            CmcWriteBytes(&w, rec, SAVE_TILE_RECORD_BYTES);
        }
    }
}

int Region3_SaveChunkV34(int slot, int cx, int cz)
{
    FILE *f;
    CmcNbtWriter w;
    char path[260];
    int globalChunkX;
    int globalChunkZ;
    int regionX;
    int regionZ;
    int index;
    int x0;
    int z0;
    long start;
    long end;
    int length;
    int stamp;
    if (slot < 0 || slot >= MAX_WORLD_SLOTS) { return 0; }
    if (cx < 0 || cx >= WORLD_CHUNKS_X || cz < 0 || cz >= WORLD_CHUNKS_Z) { return 0; }
    x0 = cx * CHUNK_SIZE;
    z0 = cz * CHUNK_SIZE;
    globalChunkX = FloorDivInt(worldOriginBlockX + x0, CHUNK_SIZE);
    globalChunkZ = FloorDivInt(worldOriginBlockZ + z0, CHUNK_SIZE);
    Region3_ChunkRegionV34(globalChunkX, globalChunkZ, &regionX, &regionZ);
    index = Region3_LocalIndexV34(globalChunkX, globalChunkZ);
    if (index < 0) { return 0; }
    SavePath_RegionFileV34(slot, regionX, regionZ, path);
    if (!Region3_EnsureFileV34(path, regionX, regionZ)) { return 0; }
    f = fopen(path, "r+b");
    if (!f) { return 0; }
    if (!Region3_ReadHeaderV34(f, regionX, regionZ)) { fclose(f); return 0; }
    if (fseek(f, 0L, SEEK_END) != 0) { fclose(f); return 0; }
    start = ftell(f);
    if (start < CMC_REGION3_HEADER_BYTES) {
        if (fseek(f, CMC_REGION3_HEADER_BYTES, SEEK_SET) != 0) { fclose(f); return 0; }
        start = CMC_REGION3_HEADER_BYTES;
    }
    w.f = f;
    w.ok = 1;
    CmcWriteBytes(&w, (const unsigned char *)"CHK3", 4);
    CmcWriteI32(&w, CMC_REGION_VERSION);
    CmcWriteI32(&w, globalChunkX);
    CmcWriteI32(&w, globalChunkZ);
    CmcWriteI32(&w, worldOriginBlockX);
    CmcWriteI32(&w, worldOriginBlockZ);
    CmcWriteI32(&w, WORLD_Y);
    CmcWriteI32(&w, CHUNK_SIZE);
    CmcWriteI32(&w, g_worldSeed);
    CmcWriteDouble(&w, g_worldTimeSeconds);
    CmcWriteI32(&w, CMC_REGION3_COMPRESSION_NONE);
    Region_WriteBlockRuns(f, x0, z0);
    Region_WriteByteArrayFromLight(f, skyLight, x0, z0);
    Region_WriteByteArrayFromLight(f, blockLight, x0, z0);
    Region_WriteBiomeBytes(f, x0, z0);
    Region3_WriteTileEntitiesForChunkV34(f, x0, z0);
    end = ftell(f);
    length = (int)(end - start);
    if (!w.ok || length <= 0 || length > CMC_REGION3_MAX_RECORD_BYTES) { fclose(f); return 0; }
    stamp = (int)(GetTickCount() / 1000);
    if (!Region3_WriteTableIntV34(f, CMC_REGION3_OFFSET_TABLE, index, (int)start) ||
        !Region3_WriteTableIntV34(f, CMC_REGION3_LENGTH_TABLE, index, length) ||
        !Region3_WriteTableIntV34(f, CMC_REGION3_TIME_TABLE, index, stamp)) {
        fclose(f);
        return 0;
    }
    fflush(f);
    fclose(f);
    return 1;
}

int SaveHandler_SaveRegionWindowV2(void)
{
    int cx;
    int cz;
    int ok;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    SaveHandler_CreateWorldLayout(currentWorldSlot);
    ok = 1;
    for (cx = 0; cx < WORLD_CHUNKS_X; cx++) {
        for (cz = 0; cz < WORLD_CHUNKS_Z; cz++) {
            if (!Region3_SaveChunkV34(currentWorldSlot, cx, cz)) { ok = 0; }
        }
    }
    return ok;
}

int Region_ReadBlockRuns(FILE *f, int x0, int z0)
{
    CmcNbtReader r;
    int pairs;
    int i;
    int n;
    int idx;
    int block;
    int run;
    int x;
    int y;
    int z;
    r.f = f;
    r.ok = 1;
    pairs = CmcReadI32(&r);
    idx = 0;
    for (i = 0; i < pairs && r.ok; i++) {
        block = CmcReadI32(&r);
        run = CmcReadI32(&r);
        for (n = 0; n < run && idx < SAVE_CHUNK_BLOCK_COUNT; n++, idx++) {
            z = idx % CHUNK_SIZE;
            y = (idx / CHUNK_SIZE) % WORLD_Y;
            x = idx / (CHUNK_SIZE * WORLD_Y);
            world[x0 + x][y][z0 + z] = block;
        }
    }
    return r.ok && idx == SAVE_CHUNK_BLOCK_COUNT;
}

int Region_ReadLight(FILE *f, unsigned char arr[WORLD_X][WORLD_Y][WORLD_Z], int x0, int z0)
{
    CmcNbtReader r;
    int len;
    int idx;
    int x;
    int y;
    int z;
    r.f = f;
    r.ok = 1;
    len = CmcReadI32(&r);
    if (len != SAVE_CHUNK_BLOCK_COUNT) { CmcSkipBytes(&r, len); return 0; }
    for (idx = 0; idx < len; idx++) {
        z = idx % CHUNK_SIZE;
        y = (idx / CHUNK_SIZE) % WORLD_Y;
        x = idx / (CHUNK_SIZE * WORLD_Y);
        arr[x0 + x][y][z0 + z] = (unsigned char)CmcReadU8(&r);
    }
    return r.ok;
}

int Region_ReadBiome(FILE *f, int x0, int z0)
{
    CmcNbtReader r;
    int len;
    int idx;
    int x;
    int z;
    r.f = f;
    r.ok = 1;
    len = CmcReadI32(&r);
    if (len != CHUNK_SIZE * CHUNK_SIZE) { CmcSkipBytes(&r, len); return 0; }
    for (idx = 0; idx < len; idx++) {
        z = idx % CHUNK_SIZE;
        x = idx / CHUNK_SIZE;
        biomeMap[x0 + x][z0 + z] = (unsigned char)CmcReadU8(&r);
    }
    return r.ok;
}

int Region3_LoadTileEntitiesForChunkV34(FILE *f, int x0, int z0, long recordEnd)
{
    CmcNbtReader r;
    unsigned char rec[SAVE_TILE_RECORD_BYTES];
    int count;
    int recordBytes;
    int i;
    long pos;
    r.f = f;
    r.ok = 1;
    pos = ftell(f);
    if (pos < 0 || pos >= recordEnd) { return 1; }
    count = CmcReadI32(&r);
    recordBytes = CmcReadI32(&r);
    if (!r.ok || count < 0 || count > MAX_TILE_ENTITIES || recordBytes < SAVE_TILE_RECORD_BYTES || recordBytes > 4096) { return 0; }
    Region3_ClearTileEntitiesForChunkV34(x0, z0);
    for (i = 0; i < count; i++) {
        if (ftell(f) + recordBytes > recordEnd) { return 0; }
        if (fread(rec, 1, SAVE_TILE_RECORD_BYTES, f) != SAVE_TILE_RECORD_BYTES) { return 0; }
        if (recordBytes > SAVE_TILE_RECORD_BYTES) { CmcSkipBytes(&r, recordBytes - SAVE_TILE_RECORD_BYTES); }
        LoadTileFromBytesV34(rec, SAVE_TILE_RECORD_BYTES, 1);
        g_regionTilesLoadedV34 = 1;
    }
    return 1;
}

int Region3_LoadChunkV34(int slot, int cx, int cz)
{
    FILE *f;
    CmcNbtReader r;
    char path[260];
    char magic[5];
    int globalChunkX;
    int globalChunkZ;
    int fileChunkX;
    int fileChunkZ;
    int originX;
    int originZ;
    int regionX;
    int regionZ;
    int index;
    int offset;
    int length;
    int version;
    int chunkHeight;
    int chunkSize;
    int seed;
    int compression;
    int x0;
    int z0;
    long recordEnd;
    double savedTime;
    if (slot < 0 || slot >= MAX_WORLD_SLOTS) { return 0; }
    if (cx < 0 || cx >= WORLD_CHUNKS_X || cz < 0 || cz >= WORLD_CHUNKS_Z) { return 0; }
    x0 = cx * CHUNK_SIZE;
    z0 = cz * CHUNK_SIZE;
    globalChunkX = FloorDivInt(worldOriginBlockX + x0, CHUNK_SIZE);
    globalChunkZ = FloorDivInt(worldOriginBlockZ + z0, CHUNK_SIZE);
    Region3_ChunkRegionV34(globalChunkX, globalChunkZ, &regionX, &regionZ);
    index = Region3_LocalIndexV34(globalChunkX, globalChunkZ);
    if (index < 0) { return 0; }
    SavePath_RegionFileV34(slot, regionX, regionZ, path);
    f = fopen(path, "rb");
    if (!f) { return 0; }
    if (!Region3_ReadHeaderV34(f, regionX, regionZ)) { fclose(f); return 0; }
    if (!Region3_ReadTableIntV34(f, CMC_REGION3_OFFSET_TABLE, index, &offset) ||
        !Region3_ReadTableIntV34(f, CMC_REGION3_LENGTH_TABLE, index, &length)) { fclose(f); return 0; }
    if (offset < CMC_REGION3_HEADER_BYTES || length <= 0 || length > CMC_REGION3_MAX_RECORD_BYTES) { fclose(f); return 0; }
    if (fseek(f, (long)offset, SEEK_SET) != 0) { fclose(f); return 0; }
    recordEnd = (long)offset + (long)length;
    if (fread(magic, 1, 4, f) != 4) { fclose(f); return 0; }
    magic[4] = 0;
    if (strcmp(magic, "CHK3") != 0) { fclose(f); return 0; }
    r.f = f;
    r.ok = 1;
    version = CmcReadI32(&r);
    fileChunkX = CmcReadI32(&r);
    fileChunkZ = CmcReadI32(&r);
    originX = CmcReadI32(&r);
    originZ = CmcReadI32(&r);
    chunkHeight = CmcReadI32(&r);
    chunkSize = CmcReadI32(&r);
    seed = CmcReadI32(&r);
    savedTime = CmcReadDouble(&r);
    compression = CmcReadI32(&r);
    if (!r.ok || version > CMC_REGION_VERSION || fileChunkX != globalChunkX || fileChunkZ != globalChunkZ ||
        chunkHeight != WORLD_Y || chunkSize != CHUNK_SIZE || compression != CMC_REGION3_COMPRESSION_NONE) {
        fclose(f);
        return 0;
    }
    if (!Region_ReadBlockRuns(f, x0, z0)) { fclose(f); return 0; }
    if (!Region_ReadLight(f, skyLight, x0, z0)) { fclose(f); return 0; }
    if (!Region_ReadLight(f, blockLight, x0, z0)) { fclose(f); return 0; }
    if (!Region_ReadBiome(f, x0, z0)) { fclose(f); return 0; }
    if (!Region3_LoadTileEntitiesForChunkV34(f, x0, z0, recordEnd)) { fclose(f); return 0; }
    fclose(f);
    g_worldSeed = seed;
    g_worldTimeSeconds = savedTime;
    return 1;
}

int SaveHandler_LoadRegionWindowLegacyV33(void);

int SaveHandler_LoadRegionWindowV2(void)
{
    int cx;
    int cz;
    int loaded;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    loaded = 0;
    g_regionTilesLoadedV34 = 0;
    for (cx = 0; cx < WORLD_CHUNKS_X; cx++) {
        for (cz = 0; cz < WORLD_CHUNKS_Z; cz++) {
            if (Region3_LoadChunkV34(currentWorldSlot, cx, cz)) { loaded++; }
        }
    }
    if (loaded > 0) {
        RebuildColumnTops();
        InvalidateAllTerrainChunkMeshes();
        return 1;
    }
    return SaveHandler_LoadRegionWindowLegacyV33();
}

int SaveHandler_LoadRegionWindowLegacyV33(void)
{
    FILE *f;
    CmcNbtReader r;
    char path[260];
    char magic[5];
    char chk[5];
    int version;
    int ox;
    int oz;
    int sx;
    int sy;
    int sz;
    int chunkSize;
    int seed;
    int chunkCount;
    int i;
    int cx;
    int cz;
    int gcx;
    int gcz;
    int x0;
    int z0;
    double time;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    SavePath_RegionFile(currentWorldSlot, path);
    f = fopen(path, "rb");
    if (!f) {
        /* Backwards compatibility with V2-V28 one-file region saves. */
        SavePath_LegacyRegionFileV29(currentWorldSlot, path);
        f = fopen(path, "rb");
    }
    if (!f) { return 0; }
    if (fread(magic, 1, 4, f) != 4) { fclose(f); return 0; }
    magic[4] = 0;
    if (strcmp(magic, "CMR2") != 0) { fclose(f); return 0; }
    r.f = f;
    r.ok = 1;
    version = CmcReadI32(&r);
    ox = CmcReadI32(&r);
    oz = CmcReadI32(&r);
    sx = CmcReadI32(&r);
    sy = CmcReadI32(&r);
    sz = CmcReadI32(&r);
    chunkSize = CmcReadI32(&r);
    seed = CmcReadI32(&r);
    time = CmcReadDouble(&r);
    chunkCount = CmcReadI32(&r);
    if (!r.ok || version > CMC_REGION_VERSION || ox != worldOriginBlockX || oz != worldOriginBlockZ || sx != WORLD_X || sy != WORLD_Y || sz != WORLD_Z || chunkSize != CHUNK_SIZE) {
        fclose(f);
        return 0;
    }
    for (i = 0; i < chunkCount; i++) {
        if (fread(chk, 1, 4, f) != 4) { fclose(f); return 0; }
        chk[4] = 0;
        if (strcmp(chk, "CHK2") != 0) { fclose(f); return 0; }
        cx = CmcReadI32(&r);
        cz = CmcReadI32(&r);
        gcx = CmcReadI32(&r);
        gcz = CmcReadI32(&r);
        x0 = cx * CHUNK_SIZE;
        z0 = cz * CHUNK_SIZE;
        if (cx < 0 || cz < 0 || cx >= WORLD_CHUNKS_X || cz >= WORLD_CHUNKS_Z) { fclose(f); return 0; }
        if (gcx != FloorDivInt(worldOriginBlockX + x0, CHUNK_SIZE) || gcz != FloorDivInt(worldOriginBlockZ + z0, CHUNK_SIZE)) { fclose(f); return 0; }
        if (!Region_ReadBlockRuns(f, x0, z0)) { fclose(f); return 0; }
        if (!Region_ReadLight(f, skyLight, x0, z0)) { fclose(f); return 0; }
        if (!Region_ReadLight(f, blockLight, x0, z0)) { fclose(f); return 0; }
        if (!Region_ReadBiome(f, x0, z0)) { fclose(f); return 0; }
    }
    fclose(f);
    g_worldSeed = seed;
    g_worldTimeSeconds = time;
    RebuildColumnTops();
    InvalidateAllTerrainChunkMeshes();
    return 1;
}

int SaveHandler_SaveCurrentWorldV2(void)
{
    int ok;
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    worldSaves[currentWorldSlot].seed = g_worldSeed;
    worldSaves[currentWorldSlot].worldSize = g_worldSizeBlocks;
    worldSaves[currentWorldSlot].playerGlobalX = GetPlayerGlobalX();
    worldSaves[currentWorldSlot].playerY = playerY;
    worldSaves[currentWorldSlot].playerGlobalZ = GetPlayerGlobalZ();
    worldSaves[currentWorldSlot].worldTime = g_worldTimeSeconds;
    ok = SaveHandler_SaveWorldInfoV2(currentWorldSlot);
    if (!SaveHandler_SavePlayerV2()) { ok = 0; }
    if (!SaveHandler_SaveEntitiesV2()) { ok = 0; }
    if (!SaveHandler_SaveTileEntitiesV2()) { ok = 0; }
    if (!SaveHandler_SaveRegionWindowV2()) { ok = 0; }
    return ok;
}

void SaveHandler_DeleteWorldV2(int slot)
{
    char path[260];
    if (slot < 0 || slot >= MAX_WORLD_SLOTS) { return; }
    SavePath_LevelDat(slot, path); remove(path);
    sprintf(path, "saves\\World%d\\level.dat.bak", slot + 1); remove(path);
    SavePath_PlayerDat(slot, path); remove(path);
    sprintf(path, "saves\\World%d\\player.dat.bak", slot + 1); remove(path);
    SavePath_EntitiesDat(slot, path); remove(path);
    sprintf(path, "saves\\World%d\\entities.dat.bak", slot + 1); remove(path);
    SavePath_TileEntitiesDat(slot, path); remove(path);
    sprintf(path, "saves\\World%d\\tileentities.dat.bak", slot + 1); remove(path);
    SavePath_RegionFile(slot, path); remove(path);
    SaveHandler_DeleteRegionFilesV34(slot);
    sprintf(path, "saves\\World%d\\region\\%s.bak", slot + 1, SAVE_REGION_FILE_NAME); remove(path);
    SavePath_RegionDir(slot, path); RemoveDirectory(path);
    SavePath_WorldDir(slot, path); RemoveDirectory(path);
}

