/* ============================================================
   CloneMC V51 section: UI / PLATFORM / OPENGL STARTUP / TEXTURES / MENUS / LEGACY SAVE UI
   ============================================================ */

void DrawBuildingTerrainScreen(const char *message);
void DrawHearts(void);
void DrawHotbar(void);
void DrawInventoryScreen(void);
void DrawInventorySlot(int x, int y, InventorySlot slot, int selected);
int GetInventorySlotAtPoint(int mx, int my, int *isHotbar);
void DrawCarriedInventoryStack(void);
void HandleInventoryClick(int mx, int my);
void HandleInventoryRightClick(int mx, int my);
void InventoryMouseRightClick(int mx, int my);

void DrawHealthHearts(void)
{
    DrawHearts();
}

void DrawHearts(void);
void DrawHotbar(void);
void DrawInventoryScreen(void);

void DrawInventorySlot(int x, int y, InventorySlot slot, int selected);



/* ------------------------------------------------------------ */
/* Main program                                                 */
/* ------------------------------------------------------------ */

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    if (!InitWindow(hInstance, nCmdShow)) {
        MessageBox(NULL, "Failed to create window.", "Error", MB_OK);
        return 0;
    }

    if (!InitOpenGL()) {
        MessageBox(NULL, "Failed to initialize OpenGL.", "Error", MB_OK);
        return 0;
    }

    LoadGameTextures();
    CreateFonts();

    LoadUserOptionsV13B();
    LoadWorldList();
    EnterMenu();

    MainLoop();

    DeleteFonts();
    DeleteGameTextures();
    ShutdownOpenGL();

    return 0;
}

/* ------------------------------------------------------------ */
/* Win32 window creation                                        */
/* ------------------------------------------------------------ */

int InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASS wc;

    g_hInst = hInstance;

    ZeroMemory(&wc, sizeof(wc));

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "CloneMCWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        return 0;
    }

    g_hwnd = CreateWindow(
        "CloneMCWindow",
        "CloneMC",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!g_hwnd) {
        return 0;
    }

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    return 1;
}


/* PATCH_F11_MOB_GUI_FULLSCREEN_IMPL
   F11 fullscreen/windowed toggle.
   Uses only old Win32 calls available to Open Watcom/Win98 headers. */
void ApplyWindowResizeState(void)
{
    if (g_windowWidth <= 0) { g_windowWidth = WINDOW_WIDTH; }
    if (g_windowHeight <= 0) { g_windowHeight = WINDOW_HEIGHT; }
    glViewport(0, 0, g_windowWidth, g_windowHeight);
    LayoutBetaMenus();
}

void ToggleFullscreen(void)
{
    int sw;
    int sh;
    int ww;
    int wh;

    if (!g_hwnd) {
        return;
    }

    if (!g_isFullscreen) {
        g_windowedStyle = (DWORD)GetWindowLong(g_hwnd, GWL_STYLE);
        GetWindowRect(g_hwnd, &g_windowedRect);

        sw = GetSystemMetrics(SM_CXSCREEN);
        sh = GetSystemMetrics(SM_CYSCREEN);
        if (sw <= 0) { sw = WINDOW_WIDTH; }
        if (sh <= 0) { sh = WINDOW_HEIGHT; }

        SetWindowLong(g_hwnd, GWL_STYLE,
                      (LONG)(g_windowedStyle & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU)));
        SetWindowPos(g_hwnd, HWND_TOP, 0, 0, sw, sh,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        g_windowWidth = sw;
        g_windowHeight = sh;
        g_isFullscreen = 1;
    } else {
        SetWindowLong(g_hwnd, GWL_STYLE, (LONG)g_windowedStyle);
        ww = g_windowedRect.right - g_windowedRect.left;
        wh = g_windowedRect.bottom - g_windowedRect.top;
        if (ww <= 0) { ww = WINDOW_WIDTH; }
        if (wh <= 0) { wh = WINDOW_HEIGHT; }
        SetWindowPos(g_hwnd, HWND_TOP,
                     g_windowedRect.left, g_windowedRect.top, ww, wh,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        g_windowWidth = ww;
        g_windowHeight = wh;
        g_isFullscreen = 0;
    }

    ApplyWindowResizeState();
    if (g_state == STATE_GAME && !inventoryOpen) {
        LockMouseForGame();
    }
}

/* ------------------------------------------------------------ */
/* OpenGL initialization                                        */
/* ------------------------------------------------------------ */

int InitOpenGL(void)
{
    PIXELFORMATDESCRIPTOR pfd;
    int pixelFormat;

    g_hdc = GetDC(g_hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL |
                  PFD_DOUBLEBUFFER;

    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    pixelFormat = ChoosePixelFormat(g_hdc, &pfd);

    if (!pixelFormat) {
        return 0;
    }

    if (!SetPixelFormat(g_hdc, pixelFormat, &pfd)) {
        return 0;
    }

    g_glrc = wglCreateContext(g_hdc);

    if (!g_glrc) {
        return 0;
    }

    if (!wglMakeCurrent(g_hdc, g_glrc)) {
        return 0;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.45f, 0.70f, 1.0f, 1.0f);

    ResourceV10_DetectCaps();

    return 1;
}

void ShutdownOpenGL(void)
{
    StopAllMusic();
    StopMobSounds();
    UnlockMouseFromGame();

    if (g_glrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_glrc);
        g_glrc = NULL;
    }

    if (g_hwnd && g_hdc) {
        ReleaseDC(g_hwnd, g_hdc);
        g_hdc = NULL;
    }
}


/* ------------------------------------------------------------ */
/* Bitmap font setup                                            */
/* ------------------------------------------------------------ */

void CreateFonts(void)
{
    HFONT normalFont;
    HFONT titleFont;
    HFONT oldFont;

    fontBaseNormal = glGenLists(96);
    fontBaseTitle = glGenLists(96);

    normalFont = CreateFont(
        22, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "Arial"
    );

    oldFont = (HFONT)SelectObject(g_hdc, normalFont);
    wglUseFontBitmaps(g_hdc, 32, 96, fontBaseNormal);
    SelectObject(g_hdc, oldFont);
    DeleteObject(normalFont);

    titleFont = CreateFont(
        64, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "Arial"
    );

    oldFont = (HFONT)SelectObject(g_hdc, titleFont);
    wglUseFontBitmaps(g_hdc, 32, 96, fontBaseTitle);
    SelectObject(g_hdc, oldFont);
    DeleteObject(titleFont);
}

void DeleteFonts(void)
{
    if (fontBaseNormal) {
        glDeleteLists(fontBaseNormal, 96);
        fontBaseNormal = 0;
    }

    if (fontBaseTitle) {
        glDeleteLists(fontBaseTitle, 96);
        fontBaseTitle = 0;
    }
}

void DrawText2D(GLuint base, int x, int y, const char *text)
{
    int len;
    if (!text) { return; }

    /* V26_ARIAL_REVERT:
       Always render GUI/HUD text through the Win32 Arial display-list font.
       The imported bitmap font remains loadable for assets, but it no longer
       overrides the readable Arial renderer. */
    len = (int)strlen(text);
    glRasterPos2i(x, y);
    glListBase(base - 32);
    glCallLists(len, GL_UNSIGNED_BYTE, text);
}



/* ------------------------------------------------------------ */
/* Beta-style pixel font                                        */
/* ------------------------------------------------------------ */

int BlockyScaleForBase(GLuint base)
{
    if (base == fontBaseTitle) {
        return 7;
    }
    return 2;
}

int BlockyCharWidth(GLuint base)
{
    return 8 * BlockyScaleForBase(base);
}

int BlockyGlyphRow(char c, int row)
{
    static const int sp[7] = {0,0,0,0,0,0,0};
    static const int qn[7] = {14,17,1,2,4,0,4};
    static const int ex[7] = {4,4,4,4,4,0,4};
    static const int dt[7] = {0,0,0,0,0,0,4};
    static const int co[7] = {0,4,4,0,4,4,0};
    static const int da[7] = {0,0,0,31,0,0,0};
    static const int sl[7] = {1,1,2,4,8,16,16};
    static const int us[7] = {0,0,0,0,0,0,31};
    static const int pl[7] = {0,4,4,31,4,4,0};
    static const int cm[7] = {0,0,0,0,0,4,8};
    static const int ap[7] = {4,4,8,0,0,0,0};
    static const int n0[7] = {14,17,19,21,25,17,14};
    static const int n1[7] = {4,12,4,4,4,4,14};
    static const int n2[7] = {14,17,1,2,4,8,31};
    static const int n3[7] = {30,1,1,14,1,1,30};
    static const int n4[7] = {2,6,10,18,31,2,2};
    static const int n5[7] = {31,16,16,30,1,1,30};
    static const int n6[7] = {6,8,16,30,17,17,14};
    static const int n7[7] = {31,1,2,4,8,8,8};
    static const int n8[7] = {14,17,17,14,17,17,14};
    static const int n9[7] = {14,17,17,15,1,2,12};
    static const int A[7] = {14,17,17,31,17,17,17};
    static const int B[7] = {30,17,17,30,17,17,30};
    static const int C[7] = {14,17,16,16,16,17,14};
    static const int D[7] = {30,17,17,17,17,17,30};
    static const int E[7] = {31,16,16,30,16,16,31};
    static const int F[7] = {31,16,16,30,16,16,16};
    static const int G[7] = {14,17,16,23,17,17,14};
    static const int H[7] = {17,17,17,31,17,17,17};
    static const int I[7] = {14,4,4,4,4,4,14};
    static const int J[7] = {7,2,2,2,18,18,12};
    static const int K[7] = {17,18,20,24,20,18,17};
    static const int L[7] = {16,16,16,16,16,16,31};
    static const int M[7] = {17,27,21,21,17,17,17};
    static const int N[7] = {17,25,21,19,17,17,17};
    static const int O[7] = {14,17,17,17,17,17,14};
    static const int P[7] = {30,17,17,30,16,16,16};
    static const int Q[7] = {14,17,17,17,21,18,13};
    static const int R[7] = {30,17,17,30,20,18,17};
    static const int S[7] = {15,16,16,14,1,1,30};
    static const int T[7] = {31,4,4,4,4,4,4};
    static const int U[7] = {17,17,17,17,17,17,14};
    static const int V[7] = {17,17,17,17,17,10,4};
    static const int W[7] = {17,17,17,21,21,21,10};
    static const int X[7] = {17,17,10,4,10,17,17};
    static const int Y[7] = {17,17,10,4,4,4,4};
    static const int Z[7] = {31,1,2,4,8,16,31};
    const int *p;

    if (row < 0 || row >= 7) {
        return 0;
    }

    if (c >= 'a' && c <= 'z') {
        c = (char)(c - 'a' + 'A');
    }

    p = qn;
    switch (c) {
    case ' ': p = sp; break;
    case '?': p = qn; break;
    case '!': p = ex; break;
    case '.': p = dt; break;
    case ':': p = co; break;
    case '-': p = da; break;
    case '/': p = sl; break;
    case '_': p = us; break;
    case '+': p = pl; break;
    case ',': p = cm; break;
    case '\'': p = ap; break;
    case '0': p = n0; break;
    case '1': p = n1; break;
    case '2': p = n2; break;
    case '3': p = n3; break;
    case '4': p = n4; break;
    case '5': p = n5; break;
    case '6': p = n6; break;
    case '7': p = n7; break;
    case '8': p = n8; break;
    case '9': p = n9; break;
    case 'A': p = A; break;
    case 'B': p = B; break;
    case 'C': p = C; break;
    case 'D': p = D; break;
    case 'E': p = E; break;
    case 'F': p = F; break;
    case 'G': p = G; break;
    case 'H': p = H; break;
    case 'I': p = I; break;
    case 'J': p = J; break;
    case 'K': p = K; break;
    case 'L': p = L; break;
    case 'M': p = M; break;
    case 'N': p = N; break;
    case 'O': p = O; break;
    case 'P': p = P; break;
    case 'Q': p = Q; break;
    case 'R': p = R; break;
    case 'S': p = S; break;
    case 'T': p = T; break;
    case 'U': p = U; break;
    case 'V': p = V; break;
    case 'W': p = W; break;
    case 'X': p = X; break;
    case 'Y': p = Y; break;
    case 'Z': p = Z; break;
    default: p = qn; break;
    }

    return p[row];
}



void DrawBetaFontGlyph2D(int ch, int x1, int y1, int x2, int y2)
{
    int sx;
    int sy;
    float u0;
    float v0;
    float u1;
    float v1;

    if (!texBetaFont) {
        return;
    }

    ch = ch & 255;
    sx = (ch & 15) * 8;
    sy = ((ch >> 4) & 15) * 8;

    u0 = (float)sx / 128.0f;
    v0 = (float)sy / 128.0f;
    u1 = (float)(sx + 8) / 128.0f;
    v1 = (float)(sy + 8) / 128.0f;

    glTexCoord2f(u0, v0); glVertex2i(x1, y1);
    glTexCoord2f(u1, v0); glVertex2i(x2, y1);
    glTexCoord2f(u1, v1); glVertex2i(x2, y2);
    glTexCoord2f(u0, v1); glVertex2i(x1, y2);
}

void DrawBetaFontText2D(GLuint base, int x, int y, const char *text)
{
    int scale;
    int i;
    int px;
    int top;

    if (!text || !texBetaFont) {
        return;
    }

    scale = BlockyScaleForBase(base);
    px = x;
    top = y - 8 * scale;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texBetaFont);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    for (i = 0; text[i] != '\0'; i++) {
        DrawBetaFontGlyph2D((int)((unsigned char)text[i]),
                            px, top, px + 8 * scale, top + 8 * scale);
        px += 8 * scale;
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void DrawBlockyText2D(GLuint base, int x, int y, const char *text)
{
    int scale;
    int i;
    int row;
    int col;
    int mask;
    int px;
    int top;
    int x0;
    int y0;

    if (!text) {
        return;
    }

    if (texBetaFont) {
        DrawBetaFontText2D(base, x, y, text);
        return;
    }

    scale = BlockyScaleForBase(base);
    px = x;
    top = y - 7 * scale;

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    for (i = 0; text[i] != '\0'; i++) {
        for (row = 0; row < 7; row++) {
            mask = BlockyGlyphRow(text[i], row);
            for (col = 0; col < 5; col++) {
                if (mask & (1 << (4 - col))) {
                    x0 = px + col * scale;
                    y0 = top + row * scale;
                    glVertex2i(x0, y0);
                    glVertex2i(x0 + scale, y0);
                    glVertex2i(x0 + scale, y0 + scale);
                    glVertex2i(x0, y0 + scale);
                }
            }
        }
        px += 8 * scale;
    }

    glEnd();
}

/* ------------------------------------------------------------ */
/* Texture loading and atlas helpers                            */
/* ------------------------------------------------------------ */

GLuint LoadTGATexture(const char *filename)
{
    FILE *file;
    unsigned char header[18];
    int width;
    int height;
    int bpp;
    int bytesPerPixel;
    int imageSize;
    unsigned char *rawData;
    unsigned char *rgbaData;
    int x;
    int y;
    int srcIndex;
    int dstIndex;
    int srcY;
    int originTop;
    GLuint texID;

    file = fopen(filename, "rb");

    if (!file) {
        MessageBox(NULL, filename, "Could not open TGA texture", MB_OK);
        return 0;
    }

    if (fread(header, 1, 18, file) != 18) {
        fclose(file);
        MessageBox(NULL, filename, "Invalid TGA header", MB_OK);
        return 0;
    }

    /*
        TGA image type 2 = uncompressed true-color.
    */
    if (header[2] != 2) {
        fclose(file);
        MessageBox(NULL, filename, "Only uncompressed TGA files are supported", MB_OK);
        return 0;
    }

    width = header[12] | (header[13] << 8);
    height = header[14] | (header[15] << 8);
    bpp = header[16];

    if (bpp != 24 && bpp != 32) {
        fclose(file);
        MessageBox(NULL, filename, "Only 24-bit or 32-bit TGA supported", MB_OK);
        return 0;
    }

    bytesPerPixel = bpp / 8;
    imageSize = width * height * bytesPerPixel;

    rawData = (unsigned char *)malloc(imageSize);

    if (!rawData) {
        fclose(file);
        MessageBox(NULL, filename, "Could not allocate TGA memory", MB_OK);
        return 0;
    }

    fread(rawData, 1, imageSize, file);
    fclose(file);

    rgbaData = (unsigned char *)malloc(width * height * 4);

    if (!rgbaData) {
        free(rawData);
        MessageBox(NULL, filename, "Could not allocate RGBA memory", MB_OK);
        return 0;
    }

    /*
        TGA stores pixels as BGR/BGRA.
        OpenGL wants RGBA.
        This also converts bottom-left TGA origin to top-left atlas rows.
    */
    originTop = (header[17] & 0x20) != 0;

    for (y = 0; y < height; y++) {
        if (originTop) {
            srcY = y;
        } else {
            srcY = height - 1 - y;
        }

        for (x = 0; x < width; x++) {
            srcIndex = (srcY * width + x) * bytesPerPixel;
            dstIndex = (y * width + x) * 4;

            rgbaData[dstIndex + 0] = rawData[srcIndex + 2];
            rgbaData[dstIndex + 1] = rawData[srcIndex + 1];
            rgbaData[dstIndex + 2] = rawData[srcIndex + 0];

            if (bytesPerPixel == 4) {
                rgbaData[dstIndex + 3] = rawData[srcIndex + 3];
            } else {
                rgbaData[dstIndex + 3] = 255;
            }
        }
    }

    free(rawData);

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /*
        GL_CLAMP works on OpenGL 1.1.
    */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        rgbaData
    );

    free(rgbaData);

    return texID;
}


/* ------------------------------------------------------------ */
/* RESOURCE_PACK_V10 implementation                             */
/* ------------------------------------------------------------ */

void ResourceV10_SafeCopy(char *dst, const char *src, int maxLen)
{
    int i;
    if (!dst || maxLen <= 0) { return; }
    if (!src) { dst[0] = 0; return; }
    for (i = 0; i < maxLen - 1 && src[i] != 0; i++) { dst[i] = src[i]; }
    dst[i] = 0;
}

void ResourceV10_NormalizeSlashes(char *s)
{
    int i;
    if (!s) { return; }
    for (i = 0; s[i] != 0; i++) { if (s[i] == '\\') { s[i] = '/'; } }
}

int ResourceV10_FileExists(const char *path)
{
    FILE *f;
    if (!path || path[0] == 0) { return 0; }
    f = fopen(path, "rb");
    if (f) { fclose(f); return 1; }
    return 0;
}

int ResourceV10_HasZipExtension(const char *path)
{
    int n;
    if (!path) { return 0; }
    n = (int)strlen(path);
    if (n < 4) { return 0; }
    if ((path[n - 4] == '.' || path[n - 4] == '.') &&
        (path[n - 3] == 'z' || path[n - 3] == 'Z') &&
        (path[n - 2] == 'i' || path[n - 2] == 'I') &&
        (path[n - 1] == 'p' || path[n - 1] == 'P')) { return 1; }
    return 0;
}

unsigned long ResourceV10_ReadLE32(FILE *f)
{
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4) { return 0; }
    return ((unsigned long)b[0]) | ((unsigned long)b[1] << 8) | ((unsigned long)b[2] << 16) | ((unsigned long)b[3] << 24);
}

unsigned int ResourceV10_ReadLE16(FILE *f)
{
    unsigned char b[2];
    if (fread(b, 1, 2, f) != 2) { return 0; }
    return ((unsigned int)b[0]) | ((unsigned int)b[1] << 8);
}

void ResourceV10_MakeCachePath(const char *logical, char *outPath, int outMax)
{
    char temp[RESOURCEV10_MAX_PATH];
    int i;
    ResourceV10_SafeCopy(temp, logical, RESOURCEV10_MAX_PATH);
    ResourceV10_NormalizeSlashes(temp);
    for (i = 0; temp[i] != 0; i++) {
        if (temp[i] == '/') { temp[i] = '_'; }
        if (temp[i] == ':' || temp[i] == '*') { temp[i] = '_'; }
    }
    CreateDirectory("assets\\texturepacks", NULL);
    CreateDirectory("assets\\texturepacks\\_cache", NULL);
    sprintf(outPath, "assets/texturepacks/_cache/%s", temp);
}

int ResourceV10_ZipEntryNameMatches(const char *entryName, const char *logical)
{
    char a[RESOURCEV10_MAX_PATH];
    char b[RESOURCEV10_MAX_PATH];
    char c[RESOURCEV10_MAX_PATH];
    ResourceV10_SafeCopy(a, entryName, RESOURCEV10_MAX_PATH);
    ResourceV10_SafeCopy(b, logical, RESOURCEV10_MAX_PATH);
    ResourceV10_NormalizeSlashes(a);
    ResourceV10_NormalizeSlashes(b);
    if (strcmp(a, b) == 0) { return 1; }
    sprintf(c, "assets/%s", b);
    if (strcmp(a, c) == 0) { return 1; }
    sprintf(c, "textures and the music folder/%s", b);
    if (strcmp(a, c) == 0) { return 1; }
    return 0;
}

int ResourceV10_ExtractStoredZipEntry(const char *zipPath, const char *logical, char *outPath, int outMax)
{
    FILE *f;
    FILE *o;
    unsigned long sig;
    unsigned int versionNeeded;
    unsigned int flags;
    unsigned int method;
    unsigned int modTime;
    unsigned int modDate;
    unsigned long crc;
    unsigned long compSize;
    unsigned long uncompSize;
    unsigned int nameLen;
    unsigned int extraLen;
    char name[RESOURCEV10_MAX_PATH];
    unsigned long left;
    unsigned char buffer[512];
    int toRead;
    int got;
    int match;
    char cachePath[RESOURCEV10_MAX_PATH];

    f = fopen(zipPath, "rb");
    if (!f) { return 0; }

    while (1) {
        sig = ResourceV10_ReadLE32(f);
        if (sig != RESOURCEV10_ZIP_LOCAL_SIG) { break; }
        versionNeeded = ResourceV10_ReadLE16(f);
        flags = ResourceV10_ReadLE16(f);
        method = ResourceV10_ReadLE16(f);
        modTime = ResourceV10_ReadLE16(f);
        modDate = ResourceV10_ReadLE16(f);
        crc = ResourceV10_ReadLE32(f);
        compSize = ResourceV10_ReadLE32(f);
        uncompSize = ResourceV10_ReadLE32(f);
        nameLen = ResourceV10_ReadLE16(f);
        extraLen = ResourceV10_ReadLE16(f);
        if (nameLen >= RESOURCEV10_MAX_PATH) { fclose(f); return 0; }
        if (fread(name, 1, nameLen, f) != nameLen) { fclose(f); return 0; }
        name[nameLen] = 0;
        if (extraLen > 0) { fseek(f, (long)extraLen, SEEK_CUR); }
        match = ResourceV10_ZipEntryNameMatches(name, logical);
        if (match) {
            if (method != 0 || (flags & 8)) {
                g_resourceV10DeflatedZipMisses++;
                fclose(f);
                return 0;
            }
            ResourceV10_MakeCachePath(logical, cachePath, RESOURCEV10_MAX_PATH);
            o = fopen(cachePath, "wb");
            if (!o) { fclose(f); return 0; }
            left = compSize;
            while (left > 0) {
                toRead = left > sizeof(buffer) ? (int)sizeof(buffer) : (int)left;
                got = (int)fread(buffer, 1, toRead, f);
                if (got <= 0) { fclose(o); fclose(f); return 0; }
                fwrite(buffer, 1, got, o);
                left -= (unsigned long)got;
            }
            fclose(o);
            ResourceV10_SafeCopy(outPath, cachePath, outMax);
            fclose(f);
            return 1;
        }
        fseek(f, (long)compSize, SEEK_CUR);
        versionNeeded = versionNeeded; modTime = modTime; modDate = modDate; crc = crc; uncompSize = uncompSize;
    }
    fclose(f);
    return 0;
}

const char *ResourceV10_AliasFor(const char *logical, int index)
{
    if (!logical) { return NULL; }
    if (strcmp(logical, "beta/items.tga") == 0) {
        if (index == 0) { return "gui/items.tga"; }
        if (index == 1) { return "items.tga"; }
    }
    if (strcmp(logical, "beta/gui.tga") == 0) {
        if (index == 0) { return "gui/gui.tga"; }
        if (index == 1) { return "gui/container.tga"; }
    }
    if (strcmp(logical, "beta/inventory.tga") == 0 && index == 0) { return "gui/inventory.tga"; }
    if (strcmp(logical, "beta/crafting.tga") == 0 && index == 0) { return "gui/crafting.tga"; }
    if (strcmp(logical, "beta/furnace.tga") == 0 && index == 0) { return "gui/furnace.tga"; }
    if (strcmp(logical, "beta/icons.tga") == 0 && index == 0) { return "gui/icons.tga"; }
    if (strcmp(logical, "beta/particles.tga") == 0 && index == 0) { return "gui/particles.tga"; }
    if (strcmp(logical, "beta/clouds.tga") == 0 && index == 0) { return "environment/clouds.tga"; }
    if (strcmp(logical, "beta/rain.tga") == 0 && index == 0) { return "environment/rain.tga"; }
    if (strcmp(logical, "beta/snow.tga") == 0 && index == 0) { return "environment/snow.tga"; }
    if (strcmp(logical, "beta/font_default.tga") == 0 && index == 0) { return "font/default.tga"; }
    /* V58_PRIORITY3_TEXTURE_DEFAULT_REPAIR:
       Treat terrain.tga as the canonical runtime terrain sheet.  Texture packs
       normally provide terrain.tga, and the built-in assets now provide the
       same logical name, so the default pack no longer goes untextured. */
    if (strcmp(logical, "terrain.tga") == 0) {
        if (index == 0) { return "terrain_v58_canonical.tga"; }
    }
    if (strcmp(logical, "terrain_v58_canonical.tga") == 0) {
        if (index == 0) { return "terrain.tga"; }
    }

    /* CLONEMC_DIRECT_JAVA_ASSET_PATCH:
       The imported Java/resource packs use both mob/ and mobs/ spellings.
       The previous code asked for mobs/*.tga but the local zip stores the
       files under assets\mob\*.tga, causing texMobCow/texMobPlayer to be 0
       and forcing untextured fallback models.  Keep the old logical names for
       texture-pack compatibility, but resolve them to the local folder. */
    if (index == 0) {
        if (strcmp(logical, "mobs/player.tga") == 0) { return "mob/char.tga"; }
        if (strcmp(logical, "mobs/char.tga") == 0) { return "mob/char.tga"; }
        if (strcmp(logical, "mobs/chicken.tga") == 0) { return "mob/chicken.tga"; }
        if (strcmp(logical, "mobs/cow.tga") == 0) { return "mob/cow.tga"; }
        if (strcmp(logical, "mobs/sheep.tga") == 0) { return "mob/sheep.tga"; }
        if (strcmp(logical, "mobs/sheep_fur.tga") == 0) { return "mob/sheep_fur.tga"; }
        if (strcmp(logical, "mobs/wolf.tga") == 0) { return "mob/wolf.tga"; }
        if (strcmp(logical, "mobs/wolf_angry.tga") == 0) { return "mob/wolf_angry.tga"; }
        if (strcmp(logical, "mobs/wolf_tame.tga") == 0) { return "mob/wolf_tame.tga"; }
        if (strcmp(logical, "mobs/squid.tga") == 0) { return "mob/squid.tga"; }
        if (strcmp(logical, "mobs/zombie.tga") == 0) { return "mob/zombie.tga"; }
        if (strcmp(logical, "mobs/skeleton.tga") == 0) { return "mob/skeleton.tga"; }
        if (strcmp(logical, "mobs/creeper.tga") == 0) { return "mob/creeper.tga"; }
        if (strcmp(logical, "mobs/spider.tga") == 0) { return "mob/spider.tga"; }
        if (strcmp(logical, "mobs/spider_eyes.tga") == 0) { return "mob/spider_eyes.tga"; }
        if (strcmp(logical, "mobs/slime.tga") == 0) { return "mob/slime.tga"; }
        if (strcmp(logical, "mobs/pig.tga") == 0) { return "mob/pig.tga"; }
    }
    return NULL;
}

int ResourceV10_TryResolveFromPack(const char *logical, char *outPath, int outMax)
{
    char candidate[RESOURCEV10_MAX_PATH];
    const char *pack;
    pack = g_resourceV10ActivePackPath;
    if (!pack || pack[0] == 0 || strcmp(pack, "assets") == 0) { return 0; }
    if (ResourceV10_HasZipExtension(pack)) {
        if (ResourceV10_ExtractStoredZipEntry(pack, logical, outPath, outMax)) { return 1; }
        return 0;
    }
    sprintf(candidate, "%s/%s", pack, logical);
    if (ResourceV10_FileExists(candidate)) { ResourceV10_SafeCopy(outPath, candidate, outMax); return 1; }
    return 0;
}

int ResourceV10_ResolveTextureFile(const char *logical, char *outPath, int outMax)
{
    const char *alias;
    char candidate[RESOURCEV10_MAX_PATH];
    int i;
    g_resourceV10ResolvedFromPack = 0;
    g_resourceV10LastFallback[0] = 0;

    if (ResourceV10_TryResolveFromPack(logical, outPath, outMax)) {
        g_resourceV10ResolvedFromPack = 1;
        ResourceV10_SafeCopy(g_resourceV10ResolvedPath, outPath, RESOURCEV10_MAX_PATH);
        return 1;
    }
    for (i = 0; i < RESOURCEV10_MAX_ALIASES; i++) {
        alias = ResourceV10_AliasFor(logical, i);
        if (!alias) { break; }
        if (ResourceV10_TryResolveFromPack(alias, outPath, outMax)) {
            g_resourceV10ResolvedFromPack = 1;
            ResourceV10_SafeCopy(g_resourceV10ResolvedPath, outPath, RESOURCEV10_MAX_PATH);
            return 1;
        }
    }

    sprintf(candidate, "assets/%s", logical);
    if (ResourceV10_FileExists(candidate)) { ResourceV10_SafeCopy(outPath, candidate, outMax); return 1; }
    for (i = 0; i < RESOURCEV10_MAX_ALIASES; i++) {
        alias = ResourceV10_AliasFor(logical, i);
        if (!alias) { break; }
        sprintf(candidate, "assets/%s", alias);
        if (ResourceV10_FileExists(candidate)) { ResourceV10_SafeCopy(outPath, candidate, outMax); ResourceV10_SafeCopy(g_resourceV10LastFallback, candidate, RESOURCEV10_MAX_PATH); return 1; }
    }
    return 0;
}

GLuint ResourceV10_LoadTGATexture(const char *logical)
{
    char path[RESOURCEV10_MAX_PATH];
    if (!ResourceV10_ResolveTextureFile(logical, path, RESOURCEV10_MAX_PATH)) { return 0; }
    return LoadTGATexture(path);
}

void ResourceV10_DetectCaps(void)
{
    const GLubyte *s;
    GLint maxTex;
    s = glGetString(GL_VENDOR);
    if (s) { ResourceV10_SafeCopy(g_resourceV10Vendor, (const char *)s, sizeof(g_resourceV10Vendor)); }
    s = glGetString(GL_RENDERER);
    if (s) { ResourceV10_SafeCopy(g_resourceV10Renderer, (const char *)s, sizeof(g_resourceV10Renderer)); }
    s = glGetString(GL_VERSION);
    if (s) { ResourceV10_SafeCopy(g_resourceV10Version, (const char *)s, sizeof(g_resourceV10Version)); }
    maxTex = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTex);
    g_resourceV10MaxTextureSize = (int)maxTex;
}

void ResourceV10_RegisterAnimation(int block, int baseCol, int baseRow, int frames, double speed)
{
    ResourceV10AnimatedTexture *a;
    if (g_resourceV10AnimCount >= RESOURCEV10_MAX_ANIM) { return; }
    a = &g_resourceV10Anim[g_resourceV10AnimCount++];
    a->block = block;
    a->baseCol = baseCol;
    a->baseRow = baseRow;
    a->frames = frames;
    a->speed = speed;
    a->frame = 0;
    a->timer = 0.0;
}

void ResourceV10_InitResourceSystem(void)
{
    if (g_resourceV10AnimCount > 0) { return; }

    /* V54_PRIORITY8_ANIMATED_TEXTURES
       Java TextureWaterFX/TextureLavaFX/TextureFlamesFX/TexturePortalFX update
       pixels directly.  For this OpenGL 1.1/Open Watcom C build, keep the
       renderer stable by selecting adjacent atlas frames instead of uploading
       generated pixels every frame.  The atlas-frame system is cheap, works on
       old cards, and still gives water/lava/fire/portal visible motion. */
    ResourceV10_RegisterAnimation(BLOCK_WATER,            0, 13, 4, 0.16);
    ResourceV10_RegisterAnimation(BLOCK_LAVA,             2, 13, 4, 0.22);
    ResourceV10_RegisterAnimation(BLOCK_STATIONARY_LAVA,  2, 13, 4, 0.26);
    ResourceV10_RegisterAnimation(BLOCK_FIRE,             3, 13, 4, 0.08);
    ResourceV10_RegisterAnimation(BLOCK_PORTAL,           27, 8, 8, 0.10);
}

void ResourceV10_SetActivePackPath(const char *path)
{
    ResourceV10_SafeCopy(g_resourceV10ActivePackPath, path && path[0] ? path : "assets", RESOURCEV10_MAX_PATH);
    ResourceV10_NormalizeSlashes(g_resourceV10ActivePackPath);
    g_resourceV10PackType = RESOURCEV10_PACK_FOLDER;
    if (ResourceV10_HasZipExtension(g_resourceV10ActivePackPath)) { g_resourceV10PackType = RESOURCEV10_PACK_STORED_ZIP; }
    if (strcmp(g_resourceV10ActivePackPath, "assets") == 0) { g_resourceV10PackType = RESOURCEV10_PACK_NONE; }
}

void ResourceV10_ReloadTextures(void)
{
    DeleteGameTextures();
    LoadGameTextures();
    DeleteTerrainChunkMeshes();
    g_resourceV10TexturesReloaded++;
}

void ResourceV10_UpdateAnimations(double dt)
{
    int i;
    ResourceV10AnimatedTexture *a;

    ResourceV10_InitResourceSystem();
    if (dt < 0.0) { dt = 0.0; }
    if (dt > 0.10) { dt = 0.10; }
    g_textureFxTimeV54 += dt;

    for (i = 0; i < g_resourceV10AnimCount; i++) {
        a = &g_resourceV10Anim[i];
        a->timer += dt;
        while (a->timer >= a->speed && a->speed > 0.001) {
            a->timer -= a->speed;
            a->frame++;
            if (a->frame >= a->frames) { a->frame = 0; }
        }
    }

    /* Keep named frame globals for debug/diagnostics and for any render path
       that wants a direct Java TextureFX-style phase. */
    g_textureFxWaterFrameV54 = (int)(g_textureFxTimeV54 * 6.0) & 3;
    g_textureFxLavaFrameV54 = (int)(g_textureFxTimeV54 * 4.0) & 3;
    g_textureFxFireFrameV54 = (int)(g_textureFxTimeV54 * 12.0) & 3;
    g_textureFxPortalFrameV54 = (int)(g_textureFxTimeV54 * 9.0) & 7;
}

int ResourceV10_GetAnimatedTile(int block, int baseCol, int baseRow, int *col, int *row)
{
    int i;
    int frame;
    ResourceV10AnimatedTexture *a;

    for (i = 0; i < g_resourceV10AnimCount; i++) {
        a = &g_resourceV10Anim[i];
        if (a->block == block) {
            frame = a->frame;
            if (block == BLOCK_WATER) { frame = g_textureFxWaterFrameV54; }
            else if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { frame = g_textureFxLavaFrameV54; }
            else if (block == BLOCK_FIRE) { frame = g_textureFxFireFrameV54; }
            else if (block == BLOCK_PORTAL) { frame = g_textureFxPortalFrameV54; }
            if (a->frames > 0) { frame = frame % a->frames; }
            *col = a->baseCol + frame;
            *row = a->baseRow;
            return 1;
        }
    }
    *col = baseCol;
    *row = baseRow;
    return 0;
}


/* ------------------------------------------------------------ */
/* V56_PRIORITY1_CANONICAL_TERRAIN_ATLAS                         */
/* ------------------------------------------------------------ */

typedef struct ResourceV56TerrainTileMapTag {
    int javaIndex;
    int col;
    int row;
    const char *name;
} ResourceV56TerrainTileMap;

/*
   This table is the single source of truth for Java terrain.png indices.
   The Java Block*.java files request vanilla/Beta tile indices, but the local
   CloneMC terrain sheet is a 512x256, 32-column atlas built from uploaded
   resources.  Earlier versions mixed old hardcoded guesses with fallback
   index%16 math, which made grass, dirt, sand, furnaces, crops, doors and
   held/dropped block icons pull the wrong tiles.  Every world, inventory,
   held-item, dropped-item and particle block path now uses this map.
*/
static ResourceV56TerrainTileMap g_resourceV56TerrainMap[] = {
    {   0,  2,  0, "grass_top" },
    {   1,  0,  0, "stone" },
    {   2,  3,  0, "dirt" },
    {   3,  1,  0, "grass_side" },
    {   4,  4,  0, "planks" },
    {   5, 17,  4, "stone_slab_side" },
    {   6, 20,  4, "stone_slab_top" },
    {   7, 29,  1, "brick" },
    {   8, 30,  1, "tnt_side" },
    {   9, 30,  1, "tnt_top" },
    {  10, 31,  1, "tnt_bottom" },
    {  11,  2,  2, "web" },
    {  12,  4,  2, "red_flower" },
    {  13,  3,  2, "yellow_flower" },
    {  14,  0, 13, "water_still" },
    {  15, 27,  2, "oak_sapling" },
    {  16,  1,  5, "cobblestone" },
    {  17,  0,  1, "bedrock" },
    {  18,  5,  1, "sand" },
    {  19, 26,  4, "gravel" },
    {  20,  3,  3, "log_side" },
    {  21,  4,  3, "log_top" },
    {  22, 22,  0, "iron_block" },
    {  23, 16,  3, "gold_block" },
    {  24, 17,  3, "diamond_block" },
    {  25, 28,  5, "chest_top" },
    {  26, 29,  5, "chest_side" },
    {  27, 28,  5, "chest_front" },
    {  28, 29,  3, "red_mushroom" },
    {  29, 30,  3, "brown_mushroom" },
    {  30,  2, 13, "lava_still" },
    {  31,  3, 13, "lava_flow_fire" },
    {  32, 28,  4, "gold_ore" },
    {  33,  0,  4, "iron_ore" },
    {  34,  1,  4, "coal_ore" },
    {  35,  1,  2, "bookshelf" },
    {  36,  6,  7, "mossy_cobble" },
    {  37,  3,  1, "obsidian" },
    {  38,  1,  0, "grass_side_overlay_safe" },
    {  39, 10,  9, "tall_grass" },
    {  43, 12,  4, "workbench_top" },
    {  44, 15,  4, "furnace_front_idle" },
    {  45, 18,  4, "furnace_side" },
    {  46, 11,  5, "dispenser_front" },
    {  49, 24,  4, "glass" },
    {  50,  3,  4, "diamond_ore" },
    {  51,  4,  4, "redstone_ore" },
    {  52,  7,  5, "leaves_oak" },
    {  55,  8,  4, "dead_bush" },
    {  59, 13,  4, "workbench_side" },
    {  60, 14,  4, "workbench_front" },
    {  61, 16,  4, "furnace_front_lit" },
    {  62, 18,  4, "furnace_top" },
    {  63, 27,  2, "sapling_birch_safe" },
    {  64, 15,  3, "white_wool" },
    {  65, 11,  5, "mob_spawner" },
    {  66, 15,  3, "snow" },
    {  67, 13,  5, "ice" },
    {  68,  1,  0, "snowy_grass_side" },
    {  69, 17,  5, "cactus_top" },
    {  70, 15,  5, "cactus_side" },
    {  71, 16,  5, "cactus_bottom" },
    {  72, 21,  5, "clay" },
    {  73, 18,  6, "reeds" },
    {  74, 19,  5, "note_block" },
    {  75, 20,  5, "jukebox_side" },
    {  80, 25,  5, "torch" },
    {  83, 17,  7, "ladder" },
    {  84, 11,  6, "trapdoor" },
    {  87, 22,  5, "farmland_dry" },
    {  88, 18,  6, "crop_stage_0" },
    {  89, 19,  6, "crop_stage_1" },
    {  90, 20,  6, "crop_stage_2" },
    {  91, 21,  6, "crop_stage_3" },
    {  92, 22,  6, "crop_stage_4" },
    {  93, 23,  6, "crop_stage_5" },
    {  94, 24,  6, "crop_stage_6" },
    {  95, 25,  6, "crop_stage_7" },
    {  96,  0,  9, "lever" },
    {  97, 30,  5, "wood_door" },
    {  99, 26,  5, "redstone_torch_on" },
    { 102, 27,  6, "pumpkin_top" },
    { 103,  0,  7, "netherrack" },
    { 104,  2,  7, "soul_sand" },
    { 105,  5,  7, "glowstone" },
    { 106, 27,  6, "piston_side" },
    { 107, 28,  6, "piston_top" },
    { 108, 27,  6, "piston_inner_side" },
    { 109, 28,  6, "piston_inner" },
    { 110, 28,  6, "piston_extended_top" },
    { 115, 27,  5, "redstone_torch_off" },
    { 118, 28,  6, "pumpkin_side" },
    { 119, 29,  6, "pumpkin_face" },
    { 120, 30,  6, "jack_o_lantern_face" },
    { 121, 12,  8, "cake_top" },
    { 122, 13,  8, "cake_side" },
    { 123, 14,  8, "cake_inside" },
    { 124, 14,  8, "cake_bottom" },
    { 128, 18,  7, "rail" },
    { 131, 20,  7, "repeater_off" },
    { 144, 19,  3, "lapis_block" },
    { 147, 21,  7, "repeater_on" },
    { 160,  2,  4, "lapis_ore" },
    { 163, 20,  8, "powered_rail_off" },
    { 164, 13,  7, "redstone_wire" },
    { 176,  9,  1, "sandstone_top" },
    { 179, 20,  7, "powered_rail_on" },
    { 192,  8,  1, "sandstone_side" },
    { 195, 22,  7, "detector_rail" },
    { 208, 10,  1, "sandstone_bottom" },
    { 9999, 0, 0, NULL }
};

int ResourceV56_FindTerrainTile(int index, int *col, int *row)
{
    int i;
    if (index < 0) { index = -index; }
    for (i = 0; g_resourceV56TerrainMap[i].name != NULL; i++) {
        if (g_resourceV56TerrainMap[i].javaIndex == index) {
            if (col) { *col = g_resourceV56TerrainMap[i].col; }
            if (row) { *row = g_resourceV56TerrainMap[i].row; }
            return 1;
        }
    }
    return 0;
}

void ResourceV10_JavaTileToAtlas(int index, int *col, int *row)
{
    int safeIndex;

    if (!col || !row) { return; }
    safeIndex = index;
    if (safeIndex < 0) { safeIndex = -safeIndex; }

    if (ResourceV56_FindTerrainTile(safeIndex, col, row)) { return; }

    /* Last-resort fallback: still draw something instead of returning an
       invisible tile.  Known Java terrain indices are mapped above, so this
       should only be reached for later/custom blocks. */
    *col = safeIndex & 15;
    *row = (safeIndex >> 4) & 15;
    if (*col < 0) { *col = 0; }
    if (*row < 0) { *row = 0; }
    if (*col >= (TERRAIN_ATLAS_WIDTH / TILE_SIZE)) { *col = 0; }
    if (*row >= (TERRAIN_ATLAS_HEIGHT / TILE_SIZE)) { *row = 0; }
}

int ResourceV10_GetJavaBlockTexture(int block, int face, int *col, int *row)
{
    int index;
    int side;
    index = -1;

    /* V11 texture audit from local Java source:
       - Block.java constructor texture indices are Java terrain.png indices.
       - Java side 1 is top, side 0 is bottom; this C renderer uses face 0 top and face 1 bottom.
       - The uploaded terrain.png is 512x256, but its first 16 columns are the Java 16-column sheet;
         the right half contains extra/custom tiles.  Therefore Java indices still map with index%16. */
    if (face == 0) { side = 1; }
    else if (face == 1) { side = 0; }
    else { side = face; }

    if (block == BLOCK_GRASS) { if (side == 1) { index = 0; } else if (side == 0) { index = 2; } else { index = 3; } }
    else if (block == BLOCK_STONE) { index = 1; }
    else if (block == BLOCK_DIRT) { index = 2; }
    else if (block == BLOCK_COBBLESTONE) { index = 16; }
    else if (block == BLOCK_PLANKS) { index = 4; }
    else if (block == BLOCK_SAPLING) { index = 15; }
    else if (block == BLOCK_BEDROCK || block == BLOCK_BORDER) { index = 17; }
    else if (block == BLOCK_WATER) { if (side <= 1) { index = 14; } else { index = 15; } }
    else if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { if (side <= 1) { index = 30; } else { index = 31; } }
    else if (block == BLOCK_SAND) { index = 18; }
    else if (block == BLOCK_GRAVEL) { index = 19; }
    else if (block == BLOCK_GOLD_ORE) { index = 32; }
    else if (block == BLOCK_IRON_ORE) { index = 33; }
    else if (block == BLOCK_COAL_ORE) { index = 34; }
    else if (block == BLOCK_WOOD) { if (side == 1 || side == 0) { index = 21; } else { index = 20; } }
    else if (block == BLOCK_LEAVES) { index = 52; }
    else if (block == BLOCK_GLASS) { index = 49; }
    else if (block == BLOCK_LAPIS_ORE) { index = 160; }
    else if (block == BLOCK_SANDSTONE) { if (side == 1) { index = 176; } else if (side == 0) { index = 208; } else { index = 192; } }
    else if (block == BLOCK_DISPENSER) { if (side == 1 || side == 0) { index = 62; } else if (side == 3) { index = 46; } else { index = 45; } }
    else if (block == BLOCK_NOTE) { index = 74; }
    else if (block == BLOCK_RAIL) { index = 128; }
    else if (block == BLOCK_DETECTOR_RAIL) { index = 195; }
    else if (block == BLOCK_WEB) { index = 11; }
    else if (block == BLOCK_TALL_GRASS) { index = 39; }
    else if (block == BLOCK_DEAD_BUSH) { index = 55; }
    else if (block == BLOCK_FLOWER_YELLOW) { index = 13; }
    else if (block == BLOCK_FLOWER_RED) { index = 12; }
    else if (block == BLOCK_MUSHROOM_BROWN) { index = 29; }
    else if (block == BLOCK_MUSHROOM_RED) { index = 28; }
    else if (block == BLOCK_GOLD_BLOCK) { index = 23; }
    else if (block == BLOCK_IRON_BLOCK) { index = 22; }
    else if (block == BLOCK_WOOL) { index = 64; }
    else if (block == BLOCK_STEP || block == BLOCK_DOUBLE_STEP) { if (side == 1 || side == 0) { index = 6; } else { index = 5; } }
    else if (block == BLOCK_BRICK) { index = 7; }
    else if (block == BLOCK_TNT) { if (side == 1) { index = 10; } else if (side == 0) { index = 9; } else { index = 8; } }
    else if (block == BLOCK_BOOKSHELF) { if (side == 1 || side == 0) { index = 4; } else { index = 35; } }
    else if (block == BLOCK_MOSSY_COBBLESTONE) { index = 36; }
    else if (block == BLOCK_OBSIDIAN) { index = 37; }
    else if (block == BLOCK_TORCH) { index = 80; }
    else if (block == BLOCK_FIRE) { index = 31; }
    else if (block == BLOCK_MOB_SPAWNER) { index = 65; }
    else if (block == BLOCK_CHEST || block == BLOCK_LOCKED_CHEST) { if (side == 1 || side == 0) { index = 25; } else if (side == 3) { index = 27; } else { index = 26; } }
    else if (block == BLOCK_REDSTONE_WIRE) { index = 164; }
    else if (block == BLOCK_DIAMOND_ORE) { index = 50; }
    else if (block == BLOCK_DIAMOND_BLOCK) { index = 24; }
    else if (block == BLOCK_WORKBENCH) { if (side == 1) { index = 43; } else if (side == 0) { index = 4; } else if (side == 2 || side == 4) { index = 60; } else { index = 59; } }
    else if (block == BLOCK_CROPS) { index = 88; }
    else if (block == BLOCK_FARMLAND) { if (side == 1) { index = 87; } else { index = 2; } }
    else if (block == BLOCK_FURNACE || block == BLOCK_LIT_FURNACE || block == BLOCK_FURNACE_LIT) { if (side == 1 || side == 0) { index = 62; } else if (side == 3) { index = (block == BLOCK_LIT_FURNACE || block == BLOCK_FURNACE_LIT) ? 61 : 44; } else { index = 45; } }
    else if (block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL) { index = 4; }
    else if (block == BLOCK_WOOD_DOOR) { index = 97; }
    else if (block == BLOCK_LADDER) { index = 83; }
    else if (block == BLOCK_LEVER) { index = 96; }
    else if (block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_STONE_BUTTON) { index = 1; }
    else if (block == BLOCK_WOOD_PRESSURE_PLATE) { index = 4; }
    else if (block == BLOCK_REDSTONE_ORE) { index = 51; }
    else if (block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) { index = block == BLOCK_REDSTONE_TORCH_ON ? 99 : 115; }
    else if (block == BLOCK_SNOW || block == BLOCK_SNOW_BLOCK) { index = 66; }
    else if (block == BLOCK_ICE) { index = 67; }
    else if (block == BLOCK_CACTUS) { if (side == 1) { index = 69; } else if (side == 0) { index = 71; } else { index = 70; } }
    else if (block == BLOCK_CLAY) { index = 72; }
    else if (block == BLOCK_REED) { index = 73; }
    else if (block == BLOCK_JUKEBOX) { if (side == 1 || side == 0) { index = 74; } else { index = 75; } }
    else if (block == BLOCK_FENCE) { index = 4; }
    else if (block == BLOCK_PUMPKIN || block == BLOCK_JACK_O_LANTERN) { if (side == 1 || side == 0) { index = 102; } else if (side == 3) { index = block == BLOCK_JACK_O_LANTERN ? 120 : 119; } else { index = 118; } }
    else if (block == BLOCK_NETHERRACK) { index = 103; }
    else if (block == BLOCK_SOULSAND) { index = 104; }
    else if (block == BLOCK_GLOWSTONE) { index = 105; }
    else if (block == BLOCK_PORTAL) { index = 14; }
    else if (block == BLOCK_CAKE) { if (side == 1) { index = 121; } else if (side == 0) { index = 124; } else { index = 122; } }
    else if (block == BLOCK_REPEATER_OFF) { index = 131; }
    else if (block == BLOCK_REPEATER_ON) { index = 147; }
    else if (block == BLOCK_TRAPDOOR) { index = 84; }
    else if (block == BLOCK_PISTON || block == BLOCK_PISTON_STICKY || block == BLOCK_PISTON_EXTENSION) { if (side == 1) { index = 107; } else { index = 106; } }

    if (index >= 0) {
        ResourceV10_JavaTileToAtlas(index, col, row);
        return 1;
    }
    return 0;
}



/* ------------------------------------------------------------ */
/* V58_PRIORITY3_TEXTURE_DEFAULT_REPAIR                         */
/* ------------------------------------------------------------ */

GLuint ResourceV58_LoadTerrainTexture(void)
{
    GLuint tex;

    /* First ask for the Java/pack logical name.  Custom packs normally supply
       terrain.tga, while the built-in pack now also includes terrain.tga as a
       copy of the canonical terrain sheet. */
    tex = ResourceV10_LoadTGATexture("terrain.tga");
    if (tex) { return tex; }

    /* Built-in fallback for users who copied only the canonical terrain file
       into the project folder. */
    tex = ResourceV10_LoadTGATexture("terrain_v58_canonical.tga");
    if (tex) { return tex; }

    return 0;
}

void LoadGameTextures(void)
{
    ResourceV10_InitResourceSystem();
    texTerrain = ResourceV58_LoadTerrainTexture();

    /*
        Keep icons.tga loaded for any older UI/icon functions.
        The new hearts do not use this atlas.
    */
    texIcons = ResourceV10_LoadTGATexture("icons.tga");

    /*
        These two files are created from your uploaded heart PNGs.
        Full heart = health present.
        Empty heart = missing health.
    */
    texHeartFull = ResourceV10_LoadTGATexture("heart_full.tga");
    texHeartEmpty = ResourceV10_LoadTGATexture("heart_empty.tga");

    texBetaLogo = ResourceV10_LoadTGATexture("beta/mclogo.tga");
    texBetaMenuBackground = ResourceV10_LoadTGATexture("beta/menu_background.tga");
    texBetaGui = ResourceV10_LoadTGATexture("beta/gui.tga");
    texBetaInventory = ResourceV10_LoadTGATexture("beta/inventory.tga");
    texBetaCrafting = ResourceV10_LoadTGATexture("beta/crafting.tga");
    texBetaItems = ResourceV10_LoadTGATexture("beta/items.tga");
    texBetaIcons = ResourceV10_LoadTGATexture("beta/icons.tga");
    texBetaParticles = ResourceV10_LoadTGATexture("beta/particles.tga");
    texBlockBreakV11 = ResourceV10_LoadTGATexture("beta/breaking.tga");
    texBetaSun = ResourceV10_LoadTGATexture("beta/sun.tga");
    texBetaMoon = ResourceV10_LoadTGATexture("beta/moon.tga");
    texBetaRain = ResourceV10_LoadTGATexture("beta/rain.tga");
    texBetaSnow = ResourceV10_LoadTGATexture("beta/snow.tga");
    texBetaShadow = ResourceV10_LoadTGATexture("beta/shadow.tga");
    texBetaVignette = ResourceV10_LoadTGATexture("beta/vignette.tga");
    texBetaWater = ResourceV10_LoadTGATexture("beta/water.tga");
    texBetaFont = ResourceV10_LoadTGATexture("beta/font_default.tga");


    /* CLONEMC_JAVA_ASSET_COMPLETION_PASS: selected extra resources from uploaded texture folders. */
    texCompatTitleLogo = ResourceV10_LoadTGATexture("title/mclogo.tga");
    texCompatTitleBlack = ResourceV10_LoadTGATexture("title/black.tga");
    texCompatTitleMojang = ResourceV10_LoadTGATexture("title/mojang.tga");
    texCompatAchievementBg = ResourceV10_LoadTGATexture("achievement/bg.tga");
    texCompatAchievementIcons = ResourceV10_LoadTGATexture("achievement/icons.tga");
    texCompatArtKz = ResourceV10_LoadTGATexture("art/kz.tga");
    texCompatItemBoat = ResourceV10_LoadTGATexture("item/boat.tga");
    texCompatItemCart = ResourceV10_LoadTGATexture("item/cart.tga");
    texCompatItemDoor = ResourceV10_LoadTGATexture("item/door.tga");
    texCompatItemSign = ResourceV10_LoadTGATexture("item/sign.tga");
    texCompatArmorLeather1 = ResourceV10_LoadTGATexture("armor/cloth_1.tga");
    texCompatArmorLeather2 = ResourceV10_LoadTGATexture("armor/cloth_2.tga");
    texCompatArmorChain1 = ResourceV10_LoadTGATexture("armor/chain_1.tga");
    texCompatArmorChain2 = ResourceV10_LoadTGATexture("armor/chain_2.tga");
    texCompatArmorIron1 = ResourceV10_LoadTGATexture("armor/iron_1.tga");
    texCompatArmorIron2 = ResourceV10_LoadTGATexture("armor/iron_2.tga");
    texCompatArmorGold1 = ResourceV10_LoadTGATexture("armor/gold_1.tga");
    texCompatArmorGold2 = ResourceV10_LoadTGATexture("armor/gold_2.tga");
    texCompatArmorDiamond1 = ResourceV10_LoadTGATexture("armor/diamond_1.tga");
    texCompatArmorDiamond2 = ResourceV10_LoadTGATexture("armor/diamond_2.tga");


    texMobChicken = ResourceV10_LoadTGATexture("mobs/chicken.tga");
    texMobCow = ResourceV10_LoadTGATexture("mobs/cow.tga");
    texMobSheep = ResourceV10_LoadTGATexture("mobs/sheep.tga");
    texMobSheepFur = ResourceV10_LoadTGATexture("mobs/sheep_fur.tga");
    texMobWolf = ResourceV10_LoadTGATexture("mobs/wolf.tga");
    texMobWolfAngry = ResourceV10_LoadTGATexture("mobs/wolf_angry.tga");
    texMobWolfTame = ResourceV10_LoadTGATexture("mobs/wolf_tame.tga");
    texMobSquid = ResourceV10_LoadTGATexture("mobs/squid.tga");
    texMobZombie = ResourceV10_LoadTGATexture("mobs/zombie.tga");
    texMobSkeleton = ResourceV10_LoadTGATexture("mobs/skeleton.tga");
    texMobCreeper = ResourceV10_LoadTGATexture("mobs/creeper.tga");
    texMobSpider = ResourceV10_LoadTGATexture("mobs/spider.tga");
    texMobSpiderEyes = ResourceV10_LoadTGATexture("mobs/spider_eyes.tga");
    texMobSlime = ResourceV10_LoadTGATexture("mobs/slime.tga");
    texMobPig = ResourceV10_LoadTGATexture("mobs/pig.tga");
    texMobPlayer = ResourceV10_LoadTGATexture("mobs/player.tga");

    glEnable(GL_TEXTURE_2D);
}


void DeleteGameTextures(void)
{
    DeleteTerrainChunkMeshes();

    if (texTerrain) {
        glDeleteTextures(1, &texTerrain);
        texTerrain = 0;
    }

    if (texIcons) {
        glDeleteTextures(1, &texIcons);
        texIcons = 0;
    }

    if (texBetaLogo) { glDeleteTextures(1, &texBetaLogo); texBetaLogo = 0; }
    if (texBetaMenuBackground) { glDeleteTextures(1, &texBetaMenuBackground); texBetaMenuBackground = 0; }
    if (texBetaGui) { glDeleteTextures(1, &texBetaGui); texBetaGui = 0; }
    if (texBetaInventory) { glDeleteTextures(1, &texBetaInventory); texBetaInventory = 0; }
    if (texBetaCrafting) { glDeleteTextures(1, &texBetaCrafting); texBetaCrafting = 0; }
    if (texBetaItems) { glDeleteTextures(1, &texBetaItems); texBetaItems = 0; }
    if (texBetaIcons) { glDeleteTextures(1, &texBetaIcons); texBetaIcons = 0; }
    if (texBetaParticles) { glDeleteTextures(1, &texBetaParticles); texBetaParticles = 0; }
    if (texBlockBreakV11) { glDeleteTextures(1, &texBlockBreakV11); texBlockBreakV11 = 0; }
    if (texBetaSun) { glDeleteTextures(1, &texBetaSun); texBetaSun = 0; }
    if (texBetaMoon) { glDeleteTextures(1, &texBetaMoon); texBetaMoon = 0; }
    if (texBetaRain) { glDeleteTextures(1, &texBetaRain); texBetaRain = 0; }
    if (texBetaSnow) { glDeleteTextures(1, &texBetaSnow); texBetaSnow = 0; }
    if (texBetaShadow) { glDeleteTextures(1, &texBetaShadow); texBetaShadow = 0; }
    if (texBetaVignette) { glDeleteTextures(1, &texBetaVignette); texBetaVignette = 0; }
    if (texBetaWater) { glDeleteTextures(1, &texBetaWater); texBetaWater = 0; }
    if (texBetaFont) { glDeleteTextures(1, &texBetaFont); texBetaFont = 0; }


    if (texCompatTitleLogo) { glDeleteTextures(1, &texCompatTitleLogo); texCompatTitleLogo = 0; }
    if (texCompatTitleBlack) { glDeleteTextures(1, &texCompatTitleBlack); texCompatTitleBlack = 0; }
    if (texCompatTitleMojang) { glDeleteTextures(1, &texCompatTitleMojang); texCompatTitleMojang = 0; }
    if (texCompatAchievementBg) { glDeleteTextures(1, &texCompatAchievementBg); texCompatAchievementBg = 0; }
    if (texCompatAchievementIcons) { glDeleteTextures(1, &texCompatAchievementIcons); texCompatAchievementIcons = 0; }
    if (texCompatArtKz) { glDeleteTextures(1, &texCompatArtKz); texCompatArtKz = 0; }
    if (texCompatItemBoat) { glDeleteTextures(1, &texCompatItemBoat); texCompatItemBoat = 0; }
    if (texCompatItemCart) { glDeleteTextures(1, &texCompatItemCart); texCompatItemCart = 0; }
    if (texCompatItemDoor) { glDeleteTextures(1, &texCompatItemDoor); texCompatItemDoor = 0; }
    if (texCompatItemSign) { glDeleteTextures(1, &texCompatItemSign); texCompatItemSign = 0; }
    if (texCompatArmorLeather1) { glDeleteTextures(1, &texCompatArmorLeather1); texCompatArmorLeather1 = 0; }
    if (texCompatArmorLeather2) { glDeleteTextures(1, &texCompatArmorLeather2); texCompatArmorLeather2 = 0; }
    if (texCompatArmorChain1) { glDeleteTextures(1, &texCompatArmorChain1); texCompatArmorChain1 = 0; }
    if (texCompatArmorChain2) { glDeleteTextures(1, &texCompatArmorChain2); texCompatArmorChain2 = 0; }
    if (texCompatArmorIron1) { glDeleteTextures(1, &texCompatArmorIron1); texCompatArmorIron1 = 0; }
    if (texCompatArmorIron2) { glDeleteTextures(1, &texCompatArmorIron2); texCompatArmorIron2 = 0; }
    if (texCompatArmorGold1) { glDeleteTextures(1, &texCompatArmorGold1); texCompatArmorGold1 = 0; }
    if (texCompatArmorGold2) { glDeleteTextures(1, &texCompatArmorGold2); texCompatArmorGold2 = 0; }
    if (texCompatArmorDiamond1) { glDeleteTextures(1, &texCompatArmorDiamond1); texCompatArmorDiamond1 = 0; }
    if (texCompatArmorDiamond2) { glDeleteTextures(1, &texCompatArmorDiamond2); texCompatArmorDiamond2 = 0; }

    if (texHeartFull) {
        glDeleteTextures(1, &texHeartFull);
        texHeartFull = 0;
    }

    if (texHeartEmpty) {
        glDeleteTextures(1, &texHeartEmpty);
        texHeartEmpty = 0;
    }


    if (texMobChicken) { glDeleteTextures(1, &texMobChicken); texMobChicken = 0; }
    if (texMobCow) { glDeleteTextures(1, &texMobCow); texMobCow = 0; }
    if (texMobSheep) { glDeleteTextures(1, &texMobSheep); texMobSheep = 0; }
    if (texMobSheepFur) { glDeleteTextures(1, &texMobSheepFur); texMobSheepFur = 0; }
    if (texMobWolf) { glDeleteTextures(1, &texMobWolf); texMobWolf = 0; }
    if (texMobWolfAngry) { glDeleteTextures(1, &texMobWolfAngry); texMobWolfAngry = 0; }
    if (texMobWolfTame) { glDeleteTextures(1, &texMobWolfTame); texMobWolfTame = 0; }
    if (texMobSquid) { glDeleteTextures(1, &texMobSquid); texMobSquid = 0; }
    if (texMobZombie) { glDeleteTextures(1, &texMobZombie); texMobZombie = 0; }
    if (texMobSkeleton) { glDeleteTextures(1, &texMobSkeleton); texMobSkeleton = 0; }
    if (texMobCreeper) { glDeleteTextures(1, &texMobCreeper); texMobCreeper = 0; }
    if (texMobSpider) { glDeleteTextures(1, &texMobSpider); texMobSpider = 0; }
    if (texMobSpiderEyes) { glDeleteTextures(1, &texMobSpiderEyes); texMobSpiderEyes = 0; }
    if (texMobSlime) { glDeleteTextures(1, &texMobSlime); texMobSlime = 0; }
    if (texMobPig) { glDeleteTextures(1, &texMobPig); texMobPig = 0; }
    if (texMobPlayer) { glDeleteTextures(1, &texMobPlayer); texMobPlayer = 0; }
}


void GetTileUVEx(int col, int row, int atlasWidth, int atlasHeight, float *u0, float *v0, float *u1, float *v1)
{
    /*
        Converts tile coordinates into OpenGL UV coordinates.

        col,row are measured from the top-left of the atlas.
        pad avoids texture bleeding from neighboring tiles.
    */
    float pad;

    pad = 0.5f;

    *u0 = ((float)(col * TILE_SIZE) + pad) / (float)atlasWidth;
    *v0 = ((float)(row * TILE_SIZE) + pad) / (float)atlasHeight;

    *u1 = ((float)((col + 1) * TILE_SIZE) - pad) / (float)atlasWidth;
    *v1 = ((float)((row + 1) * TILE_SIZE) - pad) / (float)atlasHeight;
}

void GetTerrainTileUV(int col, int row, float *u0, float *v0, float *u1, float *v1)
{
    GetTileUVEx(col, row, TERRAIN_ATLAS_WIDTH, TERRAIN_ATLAS_HEIGHT, u0, v0, u1, v1);
}

void GetIconTileUV(int col, int row, float *u0, float *v0, float *u1, float *v1)
{
    GetTileUVEx(col, row, ICONS_ATLAS_WIDTH, ICONS_ATLAS_HEIGHT, u0, v0, u1, v1);
}


/* ------------------------------------------------------------ */
/* V19 Block.java / RenderBlocks.java central block registry     */
/* ------------------------------------------------------------ */

#define BLOCK_RENDER_AIR_V19        0
#define BLOCK_RENDER_CUBE_V19       1
#define BLOCK_RENDER_CROSS_V19      2
#define BLOCK_RENDER_FLAT_V19       3
#define BLOCK_RENDER_TORCH_V19      4
#define BLOCK_RENDER_DOOR_V19       5
#define BLOCK_RENDER_LADDER_V19     6
#define BLOCK_RENDER_CACTUS_V19     7
#define BLOCK_RENDER_SNOW_V19       8
#define BLOCK_RENDER_FLUID_V19      9
#define BLOCK_RENDER_FENCE_V19     10
#define BLOCK_RENDER_SLAB_V19      11
#define BLOCK_RENDER_FIRE_V19      12
#define BLOCK_RENDER_STAIRS_V43    13
#define BLOCK_RENDER_RAIL_V43      14
#define BLOCK_RENDER_REDSTONE_V43  15
#define BLOCK_RENDER_PRESSURE_V43  16
#define BLOCK_RENDER_BUTTON_V43    17
#define BLOCK_RENDER_LEVER_V43     18
#define BLOCK_RENDER_SIGN_POST_V43 19
#define BLOCK_RENDER_TRAPDOOR_V43  20
#define BLOCK_RENDER_CHEST_V43     21
#define BLOCK_RENDER_FURNACE_V43   22
#define BLOCK_RENDER_PISTON_V43    23
#define BLOCK_RENDER_BED_V43       24
#define BLOCK_RENDER_CAKE_V43      25
#define BLOCK_RENDER_PORTAL_V43    26

#define BLOCK_FACE_TOP_V19     0
#define BLOCK_FACE_BOTTOM_V19  1
#define BLOCK_FACE_NORTH_V19   2
#define BLOCK_FACE_SOUTH_V19   3
#define BLOCK_FACE_WEST_V19    4
#define BLOCK_FACE_EAST_V19    5

/* V49_BLOCK_MATERIAL_COLLISION_RENDER:
   Java Block.java/Material.java compatibility fields.  These values keep
   the existing one-file Open Watcom build, but give every block an explicit
   material, movement/collision behavior, harvest rule and render/culling
   identity instead of relying only on the old solid/opaque booleans. */
#define BLOCK_MATERIAL_AIR_V49       0
#define BLOCK_MATERIAL_ROCK_V49      1
#define BLOCK_MATERIAL_GRASS_V49     2
#define BLOCK_MATERIAL_GROUND_V49    3
#define BLOCK_MATERIAL_WOOD_V49      4
#define BLOCK_MATERIAL_LEAVES_V49    5
#define BLOCK_MATERIAL_PLANT_V49     6
#define BLOCK_MATERIAL_WATER_V49     7
#define BLOCK_MATERIAL_LAVA_V49      8
#define BLOCK_MATERIAL_GLASS_V49     9
#define BLOCK_MATERIAL_CLOTH_V49    10
#define BLOCK_MATERIAL_SAND_V49     11
#define BLOCK_MATERIAL_CACTUS_V49   12
#define BLOCK_MATERIAL_CIRCUIT_V49  13
#define BLOCK_MATERIAL_FIRE_V49     14
#define BLOCK_MATERIAL_ICE_V49      15
#define BLOCK_MATERIAL_SNOW_V49     16
#define BLOCK_MATERIAL_PORTAL_V49   17
#define BLOCK_MATERIAL_METAL_V49    18
#define BLOCK_MATERIAL_CAKE_V49     19

#define BLOCK_COLLISION_NONE_V49        0
#define BLOCK_COLLISION_FULL_V49        1
#define BLOCK_COLLISION_BOUNDS_V49      2
#define BLOCK_COLLISION_FENCE_V49       3
#define BLOCK_COLLISION_STAIRS_V49      4
#define BLOCK_COLLISION_DOOR_V49        5
#define BLOCK_COLLISION_TRAPDOOR_V49    6
#define BLOCK_COLLISION_LADDER_V49      7
#define BLOCK_COLLISION_RAIL_V49        8
#define BLOCK_COLLISION_BUTTON_V49      9
#define BLOCK_COLLISION_PRESSURE_V49   10
#define BLOCK_COLLISION_PISTON_V49     11
#define BLOCK_COLLISION_BED_V49        12
#define BLOCK_COLLISION_CHEST_V49      13
#define BLOCK_COLLISION_CAKE_V49       14

#define BLOCK_HARVEST_NONE_V49      0
#define BLOCK_HARVEST_PICKAXE_V49   1
#define BLOCK_HARVEST_SHOVEL_V49    2
#define BLOCK_HARVEST_AXE_V49       3
#define BLOCK_HARVEST_SHEARS_V49    4
#define BLOCK_HARVEST_SWORD_V49     5

typedef struct BlockBoxV49Tag {
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
} BlockBoxV49;

typedef struct BlockDefV19Tag {
    int id;
    int renderType;
    int opaque;
    int solid;
    int translucent;
    int lightEmit;
    int topTex;
    int bottomTex;
    int sideTex;
    int frontTex;
    int backTex;
    int leftTex;
    int rightTex;
    int overlayTex;
    int dropItem;
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
    int material;
    int collisionType;
    int blocksMovement;
    int replaceable;
    int normalCube;
    int selectable;
    int harvestTool;
    int harvestLevel;
    float hardness;
    float resistance;
    float friction;
    float slipperiness;
    int burnOdds;
    int encourageFire;
    int canSilkHarvest;
    int tickRandomly;
    int renderLayer;
} BlockDefV19;

static BlockDefV19 g_blockDefsV19[256];
static int g_blockDefsReadyV19 = 0;

void BlockRegistryV19_Set(int id, int renderType, int solid, int opaque, int translucent, int lightEmit, int topTex, int bottomTex, int sideTex, int dropItem)
{
    BlockDefV19 *d;
    if (id < 0 || id >= 256) { return; }
    d = &g_blockDefsV19[id];
    d->id = id;
    d->renderType = renderType;
    d->solid = solid;
    d->opaque = opaque;
    d->translucent = translucent;
    d->lightEmit = lightEmit;
    d->topTex = topTex;
    d->bottomTex = bottomTex;
    d->sideTex = sideTex;
    d->frontTex = -1;
    d->backTex = -1;
    d->leftTex = -1;
    d->rightTex = -1;
    d->overlayTex = -1;
    d->dropItem = dropItem;
    d->minX = 0.0f;
    d->minY = 0.0f;
    d->minZ = 0.0f;
    d->maxX = 1.0f;
    d->maxY = 1.0f;
    d->maxZ = 1.0f;
    d->material = solid ? BLOCK_MATERIAL_ROCK_V49 : BLOCK_MATERIAL_AIR_V49;
    d->collisionType = solid ? BLOCK_COLLISION_FULL_V49 : BLOCK_COLLISION_NONE_V49;
    d->blocksMovement = solid ? 1 : 0;
    d->replaceable = (id == BLOCK_AIR) ? 1 : 0;
    d->normalCube = (solid && opaque && renderType == BLOCK_RENDER_CUBE_V19) ? 1 : 0;
    d->selectable = (id == BLOCK_AIR) ? 0 : 1;
    d->harvestTool = BLOCK_HARVEST_NONE_V49;
    d->harvestLevel = 0;
    d->hardness = solid ? 1.5f : 0.0f;
    d->resistance = solid ? 10.0f : 0.0f;
    d->friction = 0.60f;
    d->slipperiness = 0.60f;
    d->burnOdds = 0;
    d->encourageFire = 0;
    d->canSilkHarvest = 0;
    d->tickRandomly = 0;
    d->renderLayer = translucent ? 1 : 0;
}

void BlockRegistryV19_SetBounds(int id, float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
    BlockDefV19 *d;
    if (id < 0 || id >= 256) { return; }
    d = &g_blockDefsV19[id];
    d->minX = minX;
    d->minY = minY;
    d->minZ = minZ;
    d->maxX = maxX;
    d->maxY = maxY;
    d->maxZ = maxZ;
}

void BlockRegistryV19_SetFaces(int id, int frontTex, int backTex, int leftTex, int rightTex)
{
    BlockDefV19 *d;
    if (id < 0 || id >= 256) { return; }
    d = &g_blockDefsV19[id];
    d->frontTex = frontTex;
    d->backTex = backTex;
    d->leftTex = leftTex;
    d->rightTex = rightTex;
}

void BlockRegistryV49_SetMaterial(int id, int material, int collisionType, int blocksMovement, int replaceable, int normalCube, int harvestTool, int harvestLevel, float hardness, float resistance, float friction, float slipperiness)
{
    BlockDefV19 *d;
    if (id < 0 || id >= 256) { return; }
    d = &g_blockDefsV19[id];
    d->material = material;
    d->collisionType = collisionType;
    d->blocksMovement = blocksMovement ? 1 : 0;
    d->replaceable = replaceable ? 1 : 0;
    d->normalCube = normalCube ? 1 : 0;
    d->selectable = (id == BLOCK_AIR) ? 0 : 1;
    d->harvestTool = harvestTool;
    d->harvestLevel = harvestLevel;
    d->hardness = hardness;
    d->resistance = resistance;
    d->friction = friction;
    d->slipperiness = slipperiness;
}

void BlockRegistryV49_SetNoCollision(int id, int material, int replaceable, float hardness)
{
    BlockRegistryV49_SetMaterial(id, material, BLOCK_COLLISION_NONE_V49, 0, replaceable, 0, BLOCK_HARVEST_NONE_V49, 0, hardness, 0.0f, 0.60f, 0.60f);
}

void BlockRegistryV50_SetFireAndLayer(int id, int burnOdds, int encourageFire, int canSilkHarvest, int tickRandomly, int renderLayer)
{
    BlockDefV19 *d;
    if (id < 0 || id >= 256) { return; }
    d = &g_blockDefsV19[id];
    d->burnOdds = burnOdds;
    d->encourageFire = encourageFire;
    d->canSilkHarvest = canSilkHarvest ? 1 : 0;
    d->tickRandomly = tickRandomly ? 1 : 0;
    d->renderLayer = renderLayer;
}

void BlockRegistryV43_SetRenderType(int id, int renderType)
{
    if (id < 0 || id >= 256) { return; }
    g_blockDefsV19[id].renderType = renderType;
}

void BlockRegistryV43_SetFaceTex(int id, int topTex, int bottomTex, int sideTex, int frontTex)
{
    BlockDefV19 *d;
    if (id < 0 || id >= 256) { return; }
    d = &g_blockDefsV19[id];
    d->topTex = topTex;
    d->bottomTex = bottomTex;
    d->sideTex = sideTex;
    d->frontTex = frontTex;
}

void BlockRegistryV19_Init(void)
{
    int i;
    BlockDefV19 *d;
    if (g_blockDefsReadyV19) { return; }
    for (i = 0; i < 256; i++) {
        d = &g_blockDefsV19[i];
        d->id = i;
        d->renderType = BLOCK_RENDER_AIR_V19;
        d->solid = 0;
        d->opaque = 0;
        d->translucent = 0;
        d->lightEmit = 0;
        d->topTex = 2;
        d->bottomTex = 2;
        d->sideTex = 2;
        d->frontTex = -1;
        d->backTex = -1;
        d->leftTex = -1;
        d->rightTex = -1;
        d->overlayTex = -1;
        d->dropItem = BLOCK_AIR;
        d->minX = 0.0f;
        d->minY = 0.0f;
        d->minZ = 0.0f;
        d->maxX = 1.0f;
        d->maxY = 1.0f;
        d->maxZ = 1.0f;
        d->material = BLOCK_MATERIAL_AIR_V49;
        d->collisionType = BLOCK_COLLISION_NONE_V49;
        d->blocksMovement = 0;
        d->replaceable = 1;
        d->normalCube = 0;
        d->selectable = 0;
        d->harvestTool = BLOCK_HARVEST_NONE_V49;
        d->harvestLevel = 0;
        d->hardness = 0.0f;
        d->resistance = 0.0f;
        d->friction = 0.60f;
        d->slipperiness = 0.60f;
        d->burnOdds = 0;
        d->encourageFire = 0;
        d->canSilkHarvest = 0;
        d->tickRandomly = 0;
        d->renderLayer = 0;
    }

    BlockRegistryV19_Set(BLOCK_STONE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 1, 1, 1, BLOCK_COBBLESTONE);
    BlockRegistryV19_Set(BLOCK_GRASS, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 0, 2, 3, BLOCK_DIRT);
    g_blockDefsV19[BLOCK_GRASS].overlayTex = 38;
    BlockRegistryV19_Set(BLOCK_DIRT, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 2, 2, 2, BLOCK_DIRT);
    BlockRegistryV19_Set(BLOCK_COBBLESTONE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 16, 16, 16, BLOCK_COBBLESTONE);
    BlockRegistryV19_Set(BLOCK_PLANKS, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 4, 4, 4, BLOCK_PLANKS);
    BlockRegistryV19_Set(BLOCK_BEDROCK, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 17, 17, 17, BLOCK_BEDROCK);
    BlockRegistryV19_Set(BLOCK_SAND, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 18, 18, 18, BLOCK_SAND);
    BlockRegistryV19_Set(BLOCK_GRAVEL, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 19, 19, 19, BLOCK_GRAVEL);
    BlockRegistryV19_Set(BLOCK_GOLD_ORE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 32, 32, 32, BLOCK_GOLD_ORE);
    BlockRegistryV19_Set(BLOCK_IRON_ORE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 33, 33, 33, BLOCK_IRON_ORE);
    BlockRegistryV19_Set(BLOCK_COAL_ORE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 34, 34, 34, BLOCK_COAL_ORE);
    BlockRegistryV19_Set(BLOCK_WOOD, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 21, 21, 20, BLOCK_WOOD);
    BlockRegistryV19_Set(BLOCK_LEAVES, BLOCK_RENDER_CUBE_V19, 1, 0, 1, 0, 52, 52, 52, BLOCK_SAPLING);
    BlockRegistryV19_Set(BLOCK_GLASS, BLOCK_RENDER_CUBE_V19, 1, 0, 1, 0, 49, 49, 49, BLOCK_GLASS);
    BlockRegistryV19_Set(BLOCK_LAPIS_ORE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 160, 160, 160, BLOCK_LAPIS_ORE);
    BlockRegistryV19_Set(BLOCK_SANDSTONE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 176, 208, 192, BLOCK_SANDSTONE);
    BlockRegistryV19_Set(BLOCK_NOTE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 74, 74, 74, BLOCK_NOTE);
    BlockRegistryV19_Set(BLOCK_DISPENSER, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 62, 62, 45, BLOCK_DISPENSER);
    BlockRegistryV19_SetFaces(BLOCK_DISPENSER, 46, 45, 45, 45);
    BlockRegistryV19_Set(BLOCK_GOLD_BLOCK, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 23, 23, 23, BLOCK_GOLD_BLOCK);
    BlockRegistryV19_Set(BLOCK_IRON_BLOCK, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 22, 22, 22, BLOCK_IRON_BLOCK);
    BlockRegistryV19_Set(BLOCK_WOOL, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 64, 64, 64, BLOCK_WOOL);
    BlockRegistryV19_Set(BLOCK_DOUBLE_STEP, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 6, 6, 5, BLOCK_STEP);
    BlockRegistryV19_Set(BLOCK_STEP, BLOCK_RENDER_SLAB_V19, 1, 1, 0, 0, 6, 6, 5, BLOCK_STEP);
    BlockRegistryV19_SetBounds(BLOCK_STEP, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
    BlockRegistryV19_Set(BLOCK_BRICK, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 7, 7, 7, BLOCK_BRICK);
    BlockRegistryV19_Set(BLOCK_TNT, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 10, 9, 8, BLOCK_TNT);
    BlockRegistryV19_Set(BLOCK_BOOKSHELF, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 4, 4, 35, BLOCK_BOOKSHELF);
    BlockRegistryV19_Set(BLOCK_MOSSY_COBBLESTONE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 36, 36, 36, BLOCK_MOSSY_COBBLESTONE);
    BlockRegistryV19_Set(BLOCK_OBSIDIAN, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 37, 37, 37, BLOCK_OBSIDIAN);
    BlockRegistryV19_Set(BLOCK_MOB_SPAWNER, BLOCK_RENDER_CUBE_V19, 1, 0, 1, 0, 65, 65, 65, BLOCK_MOB_SPAWNER);
    BlockRegistryV19_Set(BLOCK_DIAMOND_ORE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 50, 50, 50, BLOCK_DIAMOND_ORE);
    BlockRegistryV19_Set(BLOCK_DIAMOND_BLOCK, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 24, 24, 24, BLOCK_DIAMOND_BLOCK);
    BlockRegistryV19_Set(BLOCK_FARMLAND, BLOCK_RENDER_SLAB_V19, 1, 1, 0, 0, 87, 2, 2, BLOCK_DIRT);
    BlockRegistryV19_SetBounds(BLOCK_FARMLAND, 0.0f, 0.0f, 0.0f, 1.0f, 0.9375f, 1.0f);
    BlockRegistryV19_Set(BLOCK_REDSTONE_ORE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 9, 51, 51, 51, BLOCK_REDSTONE_ORE);
    BlockRegistryV19_Set(BLOCK_SNOW_BLOCK, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 66, 66, 66, BLOCK_SNOW_BLOCK);
    BlockRegistryV19_Set(BLOCK_ICE, BLOCK_RENDER_CUBE_V19, 1, 0, 1, 0, 67, 67, 67, BLOCK_ICE);
    BlockRegistryV19_Set(BLOCK_CLAY, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 72, 72, 72, BLOCK_CLAY);
    BlockRegistryV19_Set(BLOCK_JUKEBOX, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 74, 74, 75, BLOCK_JUKEBOX);
    BlockRegistryV19_Set(BLOCK_PUMPKIN, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 102, 102, 118, BLOCK_PUMPKIN);
    BlockRegistryV19_SetFaces(BLOCK_PUMPKIN, 119, 118, 118, 118);
    BlockRegistryV19_Set(BLOCK_NETHERRACK, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 103, 103, 103, BLOCK_NETHERRACK);
    BlockRegistryV19_Set(BLOCK_SOULSAND, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 104, 104, 104, BLOCK_SOULSAND);
    BlockRegistryV19_Set(BLOCK_GLOWSTONE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 15, 105, 105, 105, BLOCK_GLOWSTONE);
    BlockRegistryV19_Set(BLOCK_JACK_O_LANTERN, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 15, 102, 102, 118, BLOCK_JACK_O_LANTERN);
    BlockRegistryV19_SetFaces(BLOCK_JACK_O_LANTERN, 120, 118, 118, 118);
    BlockRegistryV19_Set(BLOCK_CAKE, BLOCK_RENDER_SLAB_V19, 1, 0, 1, 0, 121, 124, 122, BLOCK_CAKE);
    BlockRegistryV19_SetBounds(BLOCK_CAKE, 0.0625f, 0.0f, 0.0625f, 0.9375f, 0.5f, 0.9375f);
    BlockRegistryV19_Set(BLOCK_PISTON, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 107, 106, 106, BLOCK_PISTON);
    BlockRegistryV19_Set(BLOCK_PISTON_STICKY, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 107, 106, 106, BLOCK_PISTON_STICKY);
    BlockRegistryV19_Set(BLOCK_PISTON_EXTENSION, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 0, 107, 106, 106, BLOCK_PISTON_EXTENSION);

    BlockRegistryV19_Set(BLOCK_WATER, BLOCK_RENDER_FLUID_V19, 0, 0, 1, 0, 14, 14, 14, BLOCK_WATER);
    BlockRegistryV19_SetBounds(BLOCK_WATER, 0.0f, 0.0f, 0.0f, 1.0f, 0.875f, 1.0f);
    BlockRegistryV19_Set(BLOCK_LAVA, BLOCK_RENDER_FLUID_V19, 0, 0, 1, 15, 30, 30, 31, BLOCK_LAVA);
    BlockRegistryV19_Set(BLOCK_STATIONARY_LAVA, BLOCK_RENDER_FLUID_V19, 0, 0, 1, 15, 30, 30, 31, BLOCK_LAVA);
    BlockRegistryV19_SetBounds(BLOCK_LAVA, 0.0f, 0.0f, 0.0f, 1.0f, 0.875f, 1.0f);
    BlockRegistryV19_SetBounds(BLOCK_STATIONARY_LAVA, 0.0f, 0.0f, 0.0f, 1.0f, 0.875f, 1.0f);

    BlockRegistryV19_Set(BLOCK_SAPLING, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 15, 15, 15, BLOCK_SAPLING);
    BlockRegistryV19_Set(BLOCK_WEB, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 11, 11, 11, BLOCK_WEB);
    BlockRegistryV19_Set(BLOCK_TALL_GRASS, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 39, 39, 39, BLOCK_TALL_GRASS);
    BlockRegistryV19_Set(BLOCK_DEAD_BUSH, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 55, 55, 55, BLOCK_DEAD_BUSH);
    BlockRegistryV19_Set(BLOCK_FLOWER_YELLOW, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 13, 13, 13, BLOCK_FLOWER_YELLOW);
    BlockRegistryV19_Set(BLOCK_FLOWER_RED, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 12, 12, 12, BLOCK_FLOWER_RED);
    BlockRegistryV19_Set(BLOCK_MUSHROOM_BROWN, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 29, 29, 29, BLOCK_MUSHROOM_BROWN);
    BlockRegistryV19_Set(BLOCK_MUSHROOM_RED, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 28, 28, 28, BLOCK_MUSHROOM_RED);
    BlockRegistryV19_Set(BLOCK_CROPS, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 88, 88, 88, BLOCK_CROPS);
    BlockRegistryV19_Set(BLOCK_REED, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 73, 73, 73, BLOCK_REED);
    BlockRegistryV19_Set(BLOCK_FIRE, BLOCK_RENDER_FIRE_V19, 0, 0, 1, 15, 31, 31, 31, BLOCK_AIR);
    BlockRegistryV19_Set(BLOCK_PORTAL, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 11, 14, 14, 14, BLOCK_AIR);

    BlockRegistryV19_Set(BLOCK_TORCH, BLOCK_RENDER_TORCH_V19, 0, 0, 1, 14, 80, 80, 80, BLOCK_TORCH);
    BlockRegistryV19_Set(BLOCK_REDSTONE_TORCH_ON, BLOCK_RENDER_TORCH_V19, 0, 0, 1, 7, 99, 99, 99, BLOCK_REDSTONE_TORCH_ON);
    BlockRegistryV19_Set(BLOCK_REDSTONE_TORCH_OFF, BLOCK_RENDER_TORCH_V19, 0, 0, 1, 0, 115, 115, 115, BLOCK_REDSTONE_TORCH_OFF);
    BlockRegistryV19_Set(BLOCK_RAIL, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 128, 128, 128, BLOCK_RAIL);
    BlockRegistryV19_Set(BLOCK_DETECTOR_RAIL, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 195, 195, 195, BLOCK_DETECTOR_RAIL);
    BlockRegistryV19_Set(BLOCK_REDSTONE_WIRE, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 164, 164, 164, BLOCK_REDSTONE_WIRE);
    BlockRegistryV19_Set(BLOCK_STONE_PRESSURE_PLATE, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 1, 1, 1, BLOCK_STONE_PRESSURE_PLATE);
    BlockRegistryV19_Set(BLOCK_WOOD_PRESSURE_PLATE, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 4, 4, 4, BLOCK_WOOD_PRESSURE_PLATE);
    BlockRegistryV19_Set(BLOCK_REPEATER_OFF, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 131, 131, 131, BLOCK_REPEATER_OFF);
    BlockRegistryV19_Set(BLOCK_REPEATER_ON, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 7, 147, 147, 147, BLOCK_REPEATER_ON);
    BlockRegistryV19_Set(BLOCK_LEVER, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 96, 96, 96, BLOCK_LEVER);
    BlockRegistryV19_Set(BLOCK_STONE_BUTTON, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 1, 1, 1, BLOCK_STONE_BUTTON);
    BlockRegistryV19_Set(BLOCK_WOOD_DOOR, BLOCK_RENDER_DOOR_V19, 1, 0, 1, 0, 97, 97, 97, BLOCK_WOOD_DOOR);
    BlockRegistryV19_Set(BLOCK_LADDER, BLOCK_RENDER_LADDER_V19, 0, 0, 1, 0, 83, 83, 83, BLOCK_LADDER);
    BlockRegistryV19_Set(BLOCK_SIGN_POST, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 4, 4, 4, BLOCK_SIGN_POST);
    BlockRegistryV19_Set(BLOCK_SIGN_WALL, BLOCK_RENDER_LADDER_V19, 0, 0, 1, 0, 4, 4, 4, BLOCK_SIGN_WALL);
    BlockRegistryV19_Set(BLOCK_TRAPDOOR, BLOCK_RENDER_SLAB_V19, 1, 0, 1, 0, 84, 84, 84, BLOCK_TRAPDOOR);
    BlockRegistryV19_SetBounds(BLOCK_TRAPDOOR, 0.0f, 0.0f, 0.0f, 1.0f, 0.1875f, 1.0f);
    BlockRegistryV19_Set(BLOCK_SNOW, BLOCK_RENDER_SNOW_V19, 0, 0, 1, 0, 66, 66, 66, BLOCK_SNOW);
    BlockRegistryV19_SetBounds(BLOCK_SNOW, 0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
    BlockRegistryV19_Set(BLOCK_CACTUS, BLOCK_RENDER_CACTUS_V19, 1, 0, 1, 0, 69, 71, 70, BLOCK_CACTUS);
    BlockRegistryV19_SetBounds(BLOCK_CACTUS, 0.0625f, 0.0f, 0.0625f, 0.9375f, 1.0f, 0.9375f);
    BlockRegistryV19_Set(BLOCK_FENCE, BLOCK_RENDER_FENCE_V19, 1, 0, 1, 0, 4, 4, 4, BLOCK_FENCE);
    BlockRegistryV19_Set(BLOCK_LIGHT, BLOCK_RENDER_CUBE_V19, 0, 0, 1, 15, 21, 21, 21, BLOCK_LIGHT);

    /* V43 RenderBlocks.java-style special render types.  These keep old
       OpenGL 1.1 drawing but make blocks use Java-like bounds, metadata
       orientation and non-cube shapes instead of generic full cubes/cards. */
    BlockRegistryV19_Set(BLOCK_CHEST, BLOCK_RENDER_CHEST_V43, 1, 0, 1, 0, 26, 26, 26, BLOCK_CHEST);
    BlockRegistryV19_SetFaces(BLOCK_CHEST, 27, 26, 26, 26);
    BlockRegistryV19_Set(BLOCK_LOCKED_CHEST, BLOCK_RENDER_CHEST_V43, 1, 0, 1, 0, 26, 26, 26, BLOCK_LOCKED_CHEST);
    BlockRegistryV19_SetFaces(BLOCK_LOCKED_CHEST, 27, 26, 26, 26);
    BlockRegistryV19_Set(BLOCK_FURNACE, BLOCK_RENDER_FURNACE_V43, 1, 1, 0, 0, 62, 62, 45, BLOCK_FURNACE);
    BlockRegistryV19_SetFaces(BLOCK_FURNACE, 61, 45, 45, 45);
    BlockRegistryV19_Set(BLOCK_FURNACE_LIT, BLOCK_RENDER_FURNACE_V43, 1, 1, 0, 13, 62, 62, 45, BLOCK_FURNACE);
    BlockRegistryV19_SetFaces(BLOCK_FURNACE_LIT, 62, 45, 45, 45);
    BlockRegistryV19_Set(BLOCK_WORKBENCH, BLOCK_RENDER_FURNACE_V43, 1, 1, 0, 0, 43, 4, 59, BLOCK_WORKBENCH);
    BlockRegistryV19_SetFaces(BLOCK_WORKBENCH, 60, 59, 59, 59);
    BlockRegistryV43_SetRenderType(BLOCK_DISPENSER, BLOCK_RENDER_FURNACE_V43);
    BlockRegistryV43_SetRenderType(BLOCK_RAIL, BLOCK_RENDER_RAIL_V43);
    BlockRegistryV43_SetRenderType(BLOCK_DETECTOR_RAIL, BLOCK_RENDER_RAIL_V43);
    BlockRegistryV43_SetRenderType(BLOCK_REDSTONE_WIRE, BLOCK_RENDER_REDSTONE_V43);
    BlockRegistryV43_SetRenderType(BLOCK_STONE_PRESSURE_PLATE, BLOCK_RENDER_PRESSURE_V43);
    BlockRegistryV43_SetRenderType(BLOCK_WOOD_PRESSURE_PLATE, BLOCK_RENDER_PRESSURE_V43);
    BlockRegistryV43_SetRenderType(BLOCK_STONE_BUTTON, BLOCK_RENDER_BUTTON_V43);
    BlockRegistryV43_SetRenderType(BLOCK_LEVER, BLOCK_RENDER_LEVER_V43);
    BlockRegistryV43_SetRenderType(BLOCK_SIGN_POST, BLOCK_RENDER_SIGN_POST_V43);
    BlockRegistryV43_SetRenderType(BLOCK_SIGN_WALL, BLOCK_RENDER_SIGN_POST_V43);
    BlockRegistryV43_SetRenderType(BLOCK_TRAPDOOR, BLOCK_RENDER_TRAPDOOR_V43);
    BlockRegistryV43_SetRenderType(BLOCK_PISTON, BLOCK_RENDER_PISTON_V43);
    BlockRegistryV43_SetRenderType(BLOCK_PISTON_STICKY, BLOCK_RENDER_PISTON_V43);
    BlockRegistryV43_SetRenderType(BLOCK_PISTON_EXTENSION, BLOCK_RENDER_PISTON_V43);
    BlockRegistryV19_Set(BLOCK_WOOD_STAIRS, BLOCK_RENDER_STAIRS_V43, 1, 1, 0, 0, 4, 4, 4, BLOCK_WOOD_STAIRS);
    BlockRegistryV19_Set(BLOCK_COBBLESTONE_STAIRS, BLOCK_RENDER_STAIRS_V43, 1, 1, 0, 0, 16, 16, 16, BLOCK_COBBLESTONE_STAIRS);
    BlockRegistryV19_Set(BLOCK_BED, BLOCK_RENDER_BED_V43, 1, 0, 1, 0, 134, 134, 134, BLOCK_BED);
    BlockRegistryV43_SetRenderType(BLOCK_CAKE, BLOCK_RENDER_CAKE_V43);
    BlockRegistryV43_SetRenderType(BLOCK_PORTAL, BLOCK_RENDER_PORTAL_V43);

    BlockRegistryV49_ApplyBlockDefaults();
    BlockRegistryV50_ApplyPriority3Overrides();
    BlockRegistryV57_ApplyPriority2BlockBehavior();

    g_blockDefsReadyV19 = 1;
}

void BlockRegistryV49_ApplyBlockDefaults(void)
{
    int i;
    for (i = 0; i < 256; i++) {
        if (g_blockDefsV19[i].id != i) { g_blockDefsV19[i].id = i; }
        if (g_blockDefsV19[i].renderType == BLOCK_RENDER_AIR_V19) {
            g_blockDefsV19[i].material = BLOCK_MATERIAL_AIR_V49;
            g_blockDefsV19[i].collisionType = BLOCK_COLLISION_NONE_V49;
            g_blockDefsV19[i].blocksMovement = 0;
            g_blockDefsV19[i].replaceable = 1;
            g_blockDefsV19[i].normalCube = 0;
            g_blockDefsV19[i].selectable = 0;
            g_blockDefsV19[i].hardness = 0.0f;
        }
    }

    BlockRegistryV49_SetMaterial(BLOCK_AIR, BLOCK_MATERIAL_AIR_V49, BLOCK_COLLISION_NONE_V49, 0, 1, 0, BLOCK_HARVEST_NONE_V49, 0, 0.0f, 0.0f, 0.60f, 0.60f);

    /* Rock/ore/metal family: Java-like pickaxe harvesting and normal cube behavior. */
    BlockRegistryV49_SetMaterial(BLOCK_STONE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 1.5f, 10.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_COBBLESTONE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 2.0f, 10.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_BEDROCK, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 3, -1.0f, 6000000.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_COAL_ORE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 3.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_IRON_ORE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 1, 3.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_GOLD_ORE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 2, 3.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_DIAMOND_ORE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 2, 3.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_REDSTONE_ORE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 2, 3.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_LAPIS_ORE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 1, 3.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_OBSIDIAN, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 3, 10.0f, 2000.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_BRICK, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 2.0f, 30.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_SANDSTONE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 0.8f, 4.0f, 0.60f, 0.60f);

    BlockRegistryV49_SetMaterial(BLOCK_DIRT, BLOCK_MATERIAL_GROUND_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_SHOVEL_V49, 0, 0.5f, 2.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_GRASS, BLOCK_MATERIAL_GRASS_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_SHOVEL_V49, 0, 0.6f, 3.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_FARMLAND, BLOCK_MATERIAL_GROUND_V49, BLOCK_COLLISION_BOUNDS_V49, 1, 0, 0, BLOCK_HARVEST_SHOVEL_V49, 0, 0.6f, 3.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_SAND, BLOCK_MATERIAL_SAND_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_SHOVEL_V49, 0, 0.5f, 2.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_GRAVEL, BLOCK_MATERIAL_SAND_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_SHOVEL_V49, 0, 0.6f, 3.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_CLAY, BLOCK_MATERIAL_GROUND_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_SHOVEL_V49, 0, 0.6f, 3.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_SNOW_BLOCK, BLOCK_MATERIAL_SNOW_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_SHOVEL_V49, 0, 0.2f, 1.0f, 0.60f, 0.60f);

    BlockRegistryV49_SetMaterial(BLOCK_PLANKS, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_AXE_V49, 0, 2.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_WOOD, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_AXE_V49, 0, 2.0f, 10.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_BOOKSHELF, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_AXE_V49, 0, 1.5f, 7.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_CHEST, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_CHEST_V49, 1, 0, 0, BLOCK_HARVEST_AXE_V49, 0, 2.5f, 12.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_LOCKED_CHEST, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_CHEST_V49, 1, 0, 0, BLOCK_HARVEST_AXE_V49, 0, 2.5f, 12.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_WORKBENCH, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_AXE_V49, 0, 2.5f, 12.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_FENCE, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_FENCE_V49, 1, 0, 0, BLOCK_HARVEST_AXE_V49, 0, 2.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_WOOD_STAIRS, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_STAIRS_V49, 1, 0, 0, BLOCK_HARVEST_AXE_V49, 0, 2.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_COBBLESTONE_STAIRS, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_STAIRS_V49, 1, 0, 0, BLOCK_HARVEST_PICKAXE_V49, 0, 2.0f, 10.0f, 0.60f, 0.60f);

    BlockRegistryV49_SetMaterial(BLOCK_GLASS, BLOCK_MATERIAL_GLASS_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 0, BLOCK_HARVEST_NONE_V49, 0, 0.3f, 1.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_ICE, BLOCK_MATERIAL_ICE_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 0, BLOCK_HARVEST_PICKAXE_V49, 0, 0.5f, 2.5f, 0.60f, 0.98f);
    BlockRegistryV49_SetMaterial(BLOCK_LEAVES, BLOCK_MATERIAL_LEAVES_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 0, BLOCK_HARVEST_SHEARS_V49, 0, 0.2f, 1.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_WOOL, BLOCK_MATERIAL_CLOTH_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_SHEARS_V49, 0, 0.8f, 4.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_CACTUS, BLOCK_MATERIAL_CACTUS_V49, BLOCK_COLLISION_BOUNDS_V49, 1, 0, 0, BLOCK_HARVEST_NONE_V49, 0, 0.4f, 2.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_SOULSAND, BLOCK_MATERIAL_SAND_V49, BLOCK_COLLISION_BOUNDS_V49, 1, 0, 1, BLOCK_HARVEST_SHOVEL_V49, 0, 0.5f, 2.5f, 0.40f, 0.40f);

    BlockRegistryV49_SetMaterial(BLOCK_WATER, BLOCK_MATERIAL_WATER_V49, BLOCK_COLLISION_NONE_V49, 0, 1, 0, BLOCK_HARVEST_NONE_V49, 0, 100.0f, 500.0f, 0.50f, 0.50f);
    BlockRegistryV49_SetMaterial(BLOCK_LAVA, BLOCK_MATERIAL_LAVA_V49, BLOCK_COLLISION_NONE_V49, 0, 1, 0, BLOCK_HARVEST_NONE_V49, 0, 100.0f, 500.0f, 0.50f, 0.50f);
    BlockRegistryV49_SetMaterial(BLOCK_STATIONARY_LAVA, BLOCK_MATERIAL_LAVA_V49, BLOCK_COLLISION_NONE_V49, 0, 1, 0, BLOCK_HARVEST_NONE_V49, 0, 100.0f, 500.0f, 0.50f, 0.50f);
    BlockRegistryV49_SetMaterial(BLOCK_FIRE, BLOCK_MATERIAL_FIRE_V49, BLOCK_COLLISION_NONE_V49, 0, 1, 0, BLOCK_HARVEST_NONE_V49, 0, 0.0f, 0.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_PORTAL, BLOCK_MATERIAL_PORTAL_V49, BLOCK_COLLISION_NONE_V49, 0, 0, 0, BLOCK_HARVEST_NONE_V49, 0, -1.0f, 6000000.0f, 0.60f, 0.60f);

    BlockRegistryV49_SetNoCollision(BLOCK_SAPLING, BLOCK_MATERIAL_PLANT_V49, 1, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_TALL_GRASS, BLOCK_MATERIAL_PLANT_V49, 1, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_DEAD_BUSH, BLOCK_MATERIAL_PLANT_V49, 1, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_FLOWER_YELLOW, BLOCK_MATERIAL_PLANT_V49, 1, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_FLOWER_RED, BLOCK_MATERIAL_PLANT_V49, 1, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_MUSHROOM_BROWN, BLOCK_MATERIAL_PLANT_V49, 1, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_MUSHROOM_RED, BLOCK_MATERIAL_PLANT_V49, 1, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_CROPS, BLOCK_MATERIAL_PLANT_V49, 1, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_REED, BLOCK_MATERIAL_PLANT_V49, 1, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_TORCH, BLOCK_MATERIAL_CIRCUIT_V49, 0, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_REDSTONE_TORCH_ON, BLOCK_MATERIAL_CIRCUIT_V49, 0, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_REDSTONE_TORCH_OFF, BLOCK_MATERIAL_CIRCUIT_V49, 0, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_REDSTONE_WIRE, BLOCK_MATERIAL_CIRCUIT_V49, 1, 0.0f);
    BlockRegistryV49_SetNoCollision(BLOCK_RAIL, BLOCK_MATERIAL_CIRCUIT_V49, 0, 0.7f);
    BlockRegistryV49_SetNoCollision(BLOCK_DETECTOR_RAIL, BLOCK_MATERIAL_CIRCUIT_V49, 0, 0.7f);
    BlockRegistryV49_SetNoCollision(BLOCK_LADDER, BLOCK_MATERIAL_WOOD_V49, 0, 0.4f);
    BlockRegistryV49_SetNoCollision(BLOCK_WEB, BLOCK_MATERIAL_PLANT_V49, 1, 4.0f);

    BlockRegistryV49_SetMaterial(BLOCK_STEP, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_BOUNDS_V49, 1, 0, 0, BLOCK_HARVEST_PICKAXE_V49, 0, 2.0f, 10.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_DOUBLE_STEP, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 2.0f, 10.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_SNOW, BLOCK_MATERIAL_SNOW_V49, BLOCK_COLLISION_BOUNDS_V49, 0, 1, 0, BLOCK_HARVEST_SHOVEL_V49, 0, 0.1f, 0.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_WOOD_DOOR, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_DOOR_V49, 1, 0, 0, BLOCK_HARVEST_AXE_V49, 0, 3.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_IRON_DOOR, BLOCK_MATERIAL_METAL_V49, BLOCK_COLLISION_DOOR_V49, 1, 0, 0, BLOCK_HARVEST_PICKAXE_V49, 1, 5.0f, 25.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_TRAPDOOR, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_TRAPDOOR_V49, 1, 0, 0, BLOCK_HARVEST_AXE_V49, 0, 3.0f, 15.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_STONE_PRESSURE_PLATE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_PRESSURE_V49, 0, 0, 0, BLOCK_HARVEST_PICKAXE_V49, 0, 0.5f, 2.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_WOOD_PRESSURE_PLATE, BLOCK_MATERIAL_WOOD_V49, BLOCK_COLLISION_PRESSURE_V49, 0, 0, 0, BLOCK_HARVEST_AXE_V49, 0, 0.5f, 2.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_STONE_BUTTON, BLOCK_MATERIAL_CIRCUIT_V49, BLOCK_COLLISION_BUTTON_V49, 0, 0, 0, BLOCK_HARVEST_PICKAXE_V49, 0, 0.5f, 2.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_LEVER, BLOCK_MATERIAL_CIRCUIT_V49, BLOCK_COLLISION_BUTTON_V49, 0, 0, 0, BLOCK_HARVEST_PICKAXE_V49, 0, 0.5f, 2.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_PISTON, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 0.5f, 2.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_PISTON_STICKY, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 0.5f, 2.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_PISTON_EXTENSION, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_PISTON_V49, 1, 0, 0, BLOCK_HARVEST_PICKAXE_V49, 0, 0.5f, 2.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_BED, BLOCK_MATERIAL_CLOTH_V49, BLOCK_COLLISION_BED_V49, 1, 0, 0, BLOCK_HARVEST_AXE_V49, 0, 0.2f, 1.0f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_CAKE, BLOCK_MATERIAL_CAKE_V49, BLOCK_COLLISION_CAKE_V49, 1, 0, 0, BLOCK_HARVEST_NONE_V49, 0, 0.5f, 2.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_FURNACE, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 3.5f, 17.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_FURNACE_LIT, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 3.5f, 17.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_DISPENSER, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 1, BLOCK_HARVEST_PICKAXE_V49, 0, 3.5f, 17.5f, 0.60f, 0.60f);
    BlockRegistryV49_SetMaterial(BLOCK_GLOWSTONE, BLOCK_MATERIAL_GLASS_V49, BLOCK_COLLISION_FULL_V49, 1, 0, 0, BLOCK_HARVEST_PICKAXE_V49, 0, 0.3f, 1.5f, 0.60f, 0.60f);
}

BlockDefV19 *BlockRegistryV19_Get(int block)
{
    BlockRegistryV19_Init();
    if (block < 0 || block >= 256) { return &g_blockDefsV19[0]; }
    return &g_blockDefsV19[block];
}

int BlockV49_GetMaterial(int block)
{
    return BlockRegistryV19_Get(block)->material;
}

int BlockV49_IsReplaceable(int block)
{
    return BlockRegistryV19_Get(block)->replaceable ? 1 : 0;
}

int BlockV49_BlocksMovement(int block)
{
    return BlockRegistryV19_Get(block)->blocksMovement ? 1 : 0;
}

int BlockV49_IsNormalCube(int block)
{
    return BlockRegistryV19_Get(block)->normalCube ? 1 : 0;
}

int BlockV49_IsSolidRenderBlock(int block)
{
    BlockDefV19 *d;
    d = BlockRegistryV19_Get(block);
    if (!d->blocksMovement) { return 0; }
    if (d->normalCube) { return 1; }
    if (d->collisionType == BLOCK_COLLISION_FULL_V49 && d->opaque) { return 1; }
    return 0;
}

int BlockV49_IsLadderLike(int block)
{
    return (block == BLOCK_LADDER) ? 1 : 0;
}

int BlockV49_IsWebLike(int block)
{
    return (block == BLOCK_WEB) ? 1 : 0;
}

float BlockV49_GetSlipperiness(int block)
{
    return BlockRegistryV19_Get(block)->slipperiness;
}

float BlockV49_GetHardness(int block)
{
    return BlockRegistryV19_Get(block)->hardness;
}

int BlockV49_GetHarvestTool(int block)
{
    return BlockRegistryV19_Get(block)->harvestTool;
}

int BlockV49_GetHarvestLevel(int block)
{
    return BlockRegistryV19_Get(block)->harvestLevel;
}

int BlockV49_ShouldSideBeRendered(int block, int neighbor)
{
    return BlockV57_ShouldSideBeRendered(block, neighbor);
}

/* ------------------------------------------------------------ */
/* V50_PRIORITY3_DEEP_BLOCK_BEHAVIOR                             */
/* Java Block.java / Material.java behavior layer.  V49 added    */
/* material and collision data; V50 uses that data for support   */
/* validation, metadata placement, safer drops, render layers,   */
/* and neighbor-change behavior without adding new source files. */
/* ------------------------------------------------------------ */

static int g_blockV50SuppressNeighborNotify = 0;
static int g_blockV50NeighborNotifyDepth = 0;

int BlockV50_IsSupportSolid(int block)
{
    return BlockV57_IsSupportSolid(block);
}

int BlockV50_IsPlantBlock(int block)
{
    if (block == BLOCK_SAPLING || block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH || block == BLOCK_FLOWER_YELLOW || block == BLOCK_FLOWER_RED ||
        block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED || block == BLOCK_CROPS || block == BLOCK_REED) { return 1; }
    return 0;
}

int BlockV50_IsSideAttachedBlock(int block)
{
    if (block == BLOCK_LADDER || block == BLOCK_SIGN_WALL || block == BLOCK_WALL_SIGN || block == BLOCK_STONE_BUTTON || block == BLOCK_LEVER) { return 1; }
    if (block == BLOCK_TORCH || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) { return 1; }
    return 0;
}

int BlockV50_CanAttachSideByMeta(int block, int x, int y, int z)
{
    unsigned char meta;
    int dir;
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    dir = meta & 7;
    if (block == BLOCK_TORCH || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) {
        if (dir == 5) { return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z)); }
        if (dir == 1) { return BlockV50_IsSupportSolid(GetBlock(x - 1, y, z)); }
        if (dir == 2) { return BlockV50_IsSupportSolid(GetBlock(x + 1, y, z)); }
        if (dir == 3) { return BlockV50_IsSupportSolid(GetBlock(x, y, z - 1)); }
        if (dir == 4) { return BlockV50_IsSupportSolid(GetBlock(x, y, z + 1)); }
        return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z));
    }
    if (block == BLOCK_LADDER || block == BLOCK_SIGN_WALL || block == BLOCK_WALL_SIGN) {
        if (dir == 2) { return BlockV50_IsSupportSolid(GetBlock(x, y, z - 1)); }
        if (dir == 3) { return BlockV50_IsSupportSolid(GetBlock(x, y, z + 1)); }
        if (dir == 4) { return BlockV50_IsSupportSolid(GetBlock(x - 1, y, z)); }
        if (dir == 5) { return BlockV50_IsSupportSolid(GetBlock(x + 1, y, z)); }
        return 0;
    }
    if (block == BLOCK_STONE_BUTTON || block == BLOCK_LEVER) {
        if (dir == 1) { return BlockV50_IsSupportSolid(GetBlock(x - 1, y, z)); }
        if (dir == 2) { return BlockV50_IsSupportSolid(GetBlock(x + 1, y, z)); }
        if (dir == 3) { return BlockV50_IsSupportSolid(GetBlock(x, y, z - 1)); }
        if (dir == 4) { return BlockV50_IsSupportSolid(GetBlock(x, y, z + 1)); }
        if (dir == 5 || dir == 6) { return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z)); }
        return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z));
    }
    return 1;
}

int BlockV50_CanPlantStay(int block, int x, int y, int z)
{
    int below;
    below = GetBlock(x, y - 1, z);
    if (block == BLOCK_CROPS) { return below == BLOCK_FARMLAND; }
    if (block == BLOCK_REED) {
        if (below == BLOCK_REED) { return 1; }
        if (below != BLOCK_GRASS && below != BLOCK_DIRT && below != BLOCK_SAND) { return 0; }
        if (GetBlock(x + 1, y - 1, z) == BLOCK_WATER || GetBlock(x - 1, y - 1, z) == BLOCK_WATER || GetBlock(x, y - 1, z + 1) == BLOCK_WATER || GetBlock(x, y - 1, z - 1) == BLOCK_WATER) { return 1; }
        return 0;
    }
    if (block == BLOCK_CACTUS) {
        if (below != BLOCK_SAND && below != BLOCK_CACTUS) { return 0; }
        if (BlockV49_BlocksMovement(GetBlock(x + 1, y, z))) { return 0; }
        if (BlockV49_BlocksMovement(GetBlock(x - 1, y, z))) { return 0; }
        if (BlockV49_BlocksMovement(GetBlock(x, y, z + 1))) { return 0; }
        if (BlockV49_BlocksMovement(GetBlock(x, y, z - 1))) { return 0; }
        return 1;
    }
    if (block == BLOCK_DEAD_BUSH) { return below == BLOCK_SAND; }
    if (block == BLOCK_MUSHROOM_BROWN || block == BLOCK_MUSHROOM_RED) { return BlockV50_IsSupportSolid(below) || below == BLOCK_DIRT || below == BLOCK_GRASS; }
    if (below == BLOCK_GRASS || below == BLOCK_DIRT || below == BLOCK_FARMLAND) { return 1; }
    return 0;
}

int BlockV50_CanBlockStayAt(int block, int x, int y, int z)
{
    unsigned char meta;
    int lower;
    int upper;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    if (block == BLOCK_AIR || block == BLOCK_BORDER) { return 1; }
    meta = g_blockMeta[x][y][z];
    if (BlockV50_IsPlantBlock(block) || block == BLOCK_CACTUS) { return BlockV50_CanPlantStay(block, x, y, z); }
    if (block == BLOCK_RAIL || block == BLOCK_DETECTOR_RAIL) { return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z)); }
    if (block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE) { return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z)); }
    if (block == BLOCK_SNOW) { return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z)) || GetBlock(x, y - 1, z) == BLOCK_ICE || GetBlock(x, y - 1, z) == BLOCK_LEAVES; }
    if (block == BLOCK_TRAPDOOR) {
        if (meta & 4) { return BlockV50_CanAttachSideByMeta(block, x, y, z); }
        return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z)) || BlockV50_IsSupportSolid(GetBlock(x, y + 1, z));
    }
    if (BlockV50_IsSideAttachedBlock(block)) { return BlockV50_CanAttachSideByMeta(block, x, y, z); }
    if (block == BLOCK_WOOD_DOOR || block == BLOCK_IRON_DOOR) {
        if (meta & 8) { lower = GetBlock(x, y - 1, z); return lower == block; }
        upper = GetBlock(x, y + 1, z);
        return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z)) && upper == block;
    }
    if (block == BLOCK_BED) { return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z)); }
    return 1;
}

int BlockV50_CanPlaceBlockAt(int block, int x, int y, int z, int hitX, int hitY, int hitZ)
{
    int old;
    int sideX;
    int sideY;
    int sideZ;
    if (!IsInsideWorld(x, y, z)) { return 0; }
    old = GetBlock(x, y, z);
    if (!BlockV57_IsReplaceableAt(old, x, y, z)) { return 0; }
    if (block == BLOCK_AIR || block == BLOCK_BORDER) { return 0; }
    sideX = x - hitX;
    sideY = y - hitY;
    sideZ = z - hitZ;
    if (block == BLOCK_CACTUS || BlockV50_IsPlantBlock(block)) { return BlockV50_CanPlantStay(block, x, y, z); }
    if (block == BLOCK_RAIL || block == BLOCK_DETECTOR_RAIL || block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE || block == BLOCK_SNOW) { return BlockV50_IsSupportSolid(GetBlock(x, y - 1, z)); }
    if (block == BLOCK_TORCH || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) {
        if (sideY > 0) { return BlockV50_IsSupportSolid(GetBlock(hitX, hitY, hitZ)); }
        return BlockV50_IsSupportSolid(GetBlock(hitX, hitY, hitZ));
    }
    if (block == BLOCK_LADDER || block == BLOCK_SIGN_WALL || block == BLOCK_WALL_SIGN || block == BLOCK_STONE_BUTTON || block == BLOCK_LEVER) {
        if (sideX == 0 && sideZ == 0) { return 0; }
        return BlockV50_IsSupportSolid(GetBlock(hitX, hitY, hitZ));
    }
    if (block == BLOCK_WOOD_DOOR || block == BLOCK_IRON_DOOR) {
        if (!BlockV50_IsSupportSolid(GetBlock(x, y - 1, z))) { return 0; }
        if (!IsInsideWorld(x, y + 1, z)) { return 0; }
        if (!BlockV49_IsReplaceable(GetBlock(x, y + 1, z))) { return 0; }
    }
    if (block == BLOCK_BED) {
        if (!BlockV50_IsSupportSolid(GetBlock(x, y - 1, z))) { return 0; }
    }
    return 1;
}

void BlockV50_ApplyPlacementMetadata(int block, int x, int y, int z, int hitX, int hitY, int hitZ)
{
    int sideX;
    int sideY;
    int sideZ;
    int dir;
    unsigned char meta;
    sideX = x - hitX;
    sideY = y - hitY;
    sideZ = z - hitZ;
    meta = 0;
    if (!IsInsideWorld(x, y, z)) { return; }
    if (block == BLOCK_RAIL || block == BLOCK_DETECTOR_RAIL) {
        g_blockMeta[x][y][z] = (unsigned char)((fabs(sin(yaw * PI / 180.0)) > fabs(cos(yaw * PI / 180.0))) ? 1 : 0);
        return;
    }
    if (block == BLOCK_WOOD_STAIRS || block == BLOCK_COBBLESTONE_STAIRS) {
        if (fabs(sin(yaw * PI / 180.0)) > fabs(cos(yaw * PI / 180.0))) { meta = sin(yaw * PI / 180.0) > 0.0 ? 0 : 1; }
        else { meta = cos(yaw * PI / 180.0) > 0.0 ? 2 : 3; }
        if (sideY < 0) { meta |= 4; }
        g_blockMeta[x][y][z] = meta;
        return;
    }
    if (block == BLOCK_STEP) {
        if (sideY < 0) { g_blockMeta[x][y][z] = 8; }
        return;
    }
    if (block == BLOCK_TRAPDOOR) {
        if (sideZ > 0) { meta = 0; }
        else if (sideZ < 0) { meta = 1; }
        else if (sideX > 0) { meta = 2; }
        else if (sideX < 0) { meta = 3; }
        if (sideY < 0) { meta |= 8; }
        g_blockMeta[x][y][z] = meta;
        return;
    }
    if (block == BLOCK_STONE_BUTTON || block == BLOCK_LEVER) {
        if (sideX > 0) { meta = 1; }
        else if (sideX < 0) { meta = 2; }
        else if (sideZ > 0) { meta = 3; }
        else if (sideZ < 0) { meta = 4; }
        else { meta = 5; }
        g_blockMeta[x][y][z] = meta;
        return;
    }
    if (block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE) { g_blockMeta[x][y][z] = 0; return; }
    if (block == BLOCK_CHEST || block == BLOCK_FURNACE || block == BLOCK_FURNACE_LIT || block == BLOCK_DISPENSER) {
        if (fabs(sin(yaw * PI / 180.0)) > fabs(cos(yaw * PI / 180.0))) { dir = sin(yaw * PI / 180.0) > 0.0 ? 4 : 5; }
        else { dir = cos(yaw * PI / 180.0) > 0.0 ? 2 : 3; }
        g_blockMeta[x][y][z] = (unsigned char)dir;
        return;
    }
    if (block == BLOCK_BED) {
        if (fabs(sin(yaw * PI / 180.0)) > fabs(cos(yaw * PI / 180.0))) { meta = sin(yaw * PI / 180.0) > 0.0 ? 1 : 3; }
        else { meta = cos(yaw * PI / 180.0) > 0.0 ? 2 : 0; }
        g_blockMeta[x][y][z] = meta;
        return;
    }
    if (block == BLOCK_CROPS) { g_blockMeta[x][y][z] = 0; return; }
}

int BlockV50_GetDropItemAt(int block, int x, int y, int z, int *countOut)
{
    unsigned char meta;
    if (countOut) { *countOut = 1; }
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    if (block == BLOCK_WOOD_DOOR) { return (meta & 8) ? ITEM_NONE : ITEM_WOOD_DOOR; }
    if (block == BLOCK_IRON_DOOR) { return (meta & 8) ? ITEM_NONE : ITEM_IRON_DOOR; }
    if (block == BLOCK_TRAPDOOR) { return ITEM_TRAPDOOR; }
    if (block == BLOCK_BED) { return (meta & 8) ? ITEM_NONE : ITEM_BED; }
    if (block == BLOCK_CAKE) { if (countOut) { *countOut = 0; } return ITEM_NONE; }
    if (block == BLOCK_GLASS || block == BLOCK_ICE) { if (countOut) { *countOut = 0; } return ITEM_NONE; }
    if (block == BLOCK_LEAVES) { if (countOut) { *countOut = 0; } return ITEM_NONE; }
    if (block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH) { if (countOut) { *countOut = 0; } return ITEM_NONE; }
    if (block == BLOCK_REDSTONE_WIRE) { return ITEM_REDSTONE; }
    if (block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) { return ITEM_REDSTONE_TORCH; }
    if (block == BLOCK_REPEATER_ON || block == BLOCK_REPEATER_OFF) { return ITEM_REPEATER; }
    if (block == BLOCK_SIGN_POST || block == BLOCK_SIGN_WALL || block == BLOCK_WALL_SIGN) { return ITEM_SIGN; }
    if (block == BLOCK_STONE_BUTTON) { return ITEM_BUTTON; }
    if (block == BLOCK_STONE_PRESSURE_PLATE) { return ITEM_PRESSURE_PLATE_STONE; }
    if (block == BLOCK_WOOD_PRESSURE_PLATE) { return ITEM_PRESSURE_PLATE_WOOD; }
    return GetBlockDropItemAt(block, x, y, z, countOut);
}

void BlockV50_DropBlockAsItem(int x, int y, int z, int block)
{
    int count;
    int item;
    if (block == BLOCK_AIR || block == BLOCK_BORDER) { return; }
    count = 1;
    item = BlockV50_GetDropItemAt(block, x, y, z, &count);
    if (g_state == STATE_GAME && item != ITEM_NONE && count > 0) {
        AddDroppedItem(item, count, (double)x + 0.5, (double)y + 0.45, (double)z + 0.5, 0.0, 0.45, 0.0);
    }
}

void BlockV50_OnNeighborBlockChange(int x, int y, int z, int changedX, int changedY, int changedZ)
{
    int block;
    (void)changedX;
    (void)changedY;
    (void)changedZ;
    if (!IsInsideWorld(x, y, z)) { return; }
    block = GetBlock(x, y, z);
    if (block == BLOCK_AIR || block == BLOCK_BORDER) { return; }
    if (!BlockV50_CanBlockStayAt(block, x, y, z)) {
        BlockV50_DropBlockAsItem(x, y, z, block);
        g_blockV50SuppressNeighborNotify++;
        SetBlock(x, y, z, BLOCK_AIR);
        g_blockV50SuppressNeighborNotify--;
    }
}

void BlockV50_NotifyNeighborsOfChange(int x, int y, int z, int oldBlock, int newBlock)
{
    int dx;
    int dz;
    (void)oldBlock;
    (void)newBlock;
    if (g_blockV50SuppressNeighborNotify || g_blockV50NeighborNotifyDepth > 2) { return; }
    g_blockV50NeighborNotifyDepth++;
    BlockV50_OnNeighborBlockChange(x, y + 1, z, x, y, z);
    BlockV50_OnNeighborBlockChange(x, y - 1, z, x, y, z);
    for (dx = -1; dx <= 1; dx++) {
        if (dx != 0) { BlockV50_OnNeighborBlockChange(x + dx, y, z, x, y, z); }
    }
    for (dz = -1; dz <= 1; dz++) {
        if (dz != 0) { BlockV50_OnNeighborBlockChange(x, y, z + dz, x, y, z); }
    }
    BlockV50_OnNeighborBlockChange(x, y + 2, z, x, y, z);
    g_blockV50NeighborNotifyDepth--;
}

void BlockRegistryV50_ApplyPriority3Overrides(void)
{
    BlockRegistryV50_SetFireAndLayer(BLOCK_PLANKS, 5, 20, 0, 0, 0);
    BlockRegistryV50_SetFireAndLayer(BLOCK_WOOD, 5, 5, 0, 0, 0);
    BlockRegistryV50_SetFireAndLayer(BLOCK_LEAVES, 30, 60, 1, 1, 1);
    BlockRegistryV50_SetFireAndLayer(BLOCK_BOOKSHELF, 30, 20, 0, 0, 0);
    BlockRegistryV50_SetFireAndLayer(BLOCK_WOOL, 30, 60, 0, 0, 0);
    BlockRegistryV50_SetFireAndLayer(BLOCK_TNT, 15, 100, 0, 0, 0);
    BlockRegistryV50_SetFireAndLayer(BLOCK_GLASS, 0, 0, 1, 0, 1);
    BlockRegistryV50_SetFireAndLayer(BLOCK_ICE, 0, 0, 1, 0, 1);
    BlockRegistryV50_SetFireAndLayer(BLOCK_WATER, 0, 0, 0, 1, 1);
    BlockRegistryV50_SetFireAndLayer(BLOCK_LAVA, 0, 0, 0, 1, 1);
    BlockRegistryV50_SetFireAndLayer(BLOCK_STATIONARY_LAVA, 0, 0, 0, 1, 1);
    BlockRegistryV50_SetFireAndLayer(BLOCK_FIRE, 0, 0, 0, 1, 1);
    BlockRegistryV50_SetFireAndLayer(BLOCK_PORTAL, 0, 0, 0, 1, 1);
    BlockRegistryV50_SetFireAndLayer(BLOCK_CROPS, 0, 0, 0, 1, 1);
    BlockRegistryV50_SetFireAndLayer(BLOCK_REED, 0, 0, 0, 1, 1);
    BlockRegistryV50_SetFireAndLayer(BLOCK_REDSTONE_WIRE, 0, 0, 0, 1, 1);
}

/* ------------------------------------------------------------ */
/* V57_PRIORITY2_BLOCK_MATERIAL_RENDER_COMPLETION               */
/*                                                              */
/* This pass consolidates the partial V43/V49/V50 block work    */
/* into a single Java Block.java / Material.java / RenderBlocks  */
/* style registry layer.  It does not add new backup files and   */
/* keeps Open Watcom C89 compatibility.                          */
/* ------------------------------------------------------------ */

static void BlockRegistryV57_SetFlags(int id, int renderType, int solid, int opaque, int translucent, int blocksMovement, int replaceable, int normalCube, int selectable, int renderLayer)
{
    BlockDefV19 *d;
    if (id < 0 || id >= 256) { return; }
    d = &g_blockDefsV19[id];
    d->renderType = renderType;
    d->solid = solid ? 1 : 0;
    d->opaque = opaque ? 1 : 0;
    d->translucent = translucent ? 1 : 0;
    d->blocksMovement = blocksMovement ? 1 : 0;
    d->replaceable = replaceable ? 1 : 0;
    d->normalCube = normalCube ? 1 : 0;
    d->selectable = selectable ? 1 : 0;
    d->renderLayer = renderLayer;
}

static void BlockRegistryV57_SetHarvest(int id, int material, int collisionType, int harvestTool, int harvestLevel, float hardness, float resistance, float friction, float slipperiness)
{
    BlockDefV19 *d;
    if (id < 0 || id >= 256) { return; }
    d = &g_blockDefsV19[id];
    d->material = material;
    d->collisionType = collisionType;
    d->harvestTool = harvestTool;
    d->harvestLevel = harvestLevel;
    d->hardness = hardness;
    d->resistance = resistance;
    d->friction = friction;
    d->slipperiness = slipperiness;
}

static void BlockRegistryV57_SetDrop(int id, int dropItem)
{
    if (id < 0 || id >= 256) { return; }
    g_blockDefsV19[id].dropItem = dropItem;
}

void BlockRegistryV57_ApplyPriority2BlockBehavior(void)
{
    /* Full opaque cubes. */
    BlockRegistryV57_SetFlags(BLOCK_STONE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_DIRT, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_GRASS, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_SAND, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_GRAVEL, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_COBBLESTONE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_PLANKS, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_WOOD, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_BEDROCK, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_BRICK, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_MOSSY_COBBLESTONE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_SANDSTONE, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_SNOW_BLOCK, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_CLAY, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);

    /* Transparent full blocks: collide, but never act as a normal opaque cube. */
    BlockRegistryV57_SetFlags(BLOCK_GLASS, BLOCK_RENDER_CUBE_V19, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_ICE, BLOCK_RENDER_CUBE_V19, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_LEAVES, BLOCK_RENDER_CUBE_V19, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_MOB_SPAWNER, BLOCK_RENDER_CUBE_V19, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_GLOWSTONE, BLOCK_RENDER_CUBE_V19, 1, 0, 1, 1, 0, 0, 1, 1);

    /* Non-cube collision/render blocks. */
    BlockRegistryV57_SetFlags(BLOCK_STEP, BLOCK_RENDER_SLAB_V19, 1, 1, 0, 1, 0, 0, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_DOUBLE_STEP, BLOCK_RENDER_CUBE_V19, 1, 1, 0, 1, 0, 1, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_FARMLAND, BLOCK_RENDER_SLAB_V19, 1, 1, 0, 1, 0, 0, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_CACTUS, BLOCK_RENDER_CACTUS_V19, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_FENCE, BLOCK_RENDER_FENCE_V19, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_WOOD_STAIRS, BLOCK_RENDER_STAIRS_V43, 1, 1, 0, 1, 0, 0, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_COBBLESTONE_STAIRS, BLOCK_RENDER_STAIRS_V43, 1, 1, 0, 1, 0, 0, 1, 0);
    BlockRegistryV57_SetFlags(BLOCK_CHEST, BLOCK_RENDER_CHEST_V43, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_LOCKED_CHEST, BLOCK_RENDER_CHEST_V43, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_BED, BLOCK_RENDER_BED_V43, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_CAKE, BLOCK_RENDER_CAKE_V43, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_TRAPDOOR, BLOCK_RENDER_TRAPDOOR_V43, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_WOOD_DOOR, BLOCK_RENDER_DOOR_V19, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_IRON_DOOR, BLOCK_RENDER_DOOR_V19, 1, 0, 1, 1, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_PISTON_EXTENSION, BLOCK_RENDER_PISTON_V43, 1, 0, 1, 1, 0, 0, 1, 1);

    /* Flat/cross/support blocks are selectable but not movement-blocking. */
    BlockRegistryV57_SetFlags(BLOCK_SNOW, BLOCK_RENDER_SNOW_V19, 0, 0, 1, 0, 1, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_TORCH, BLOCK_RENDER_TORCH_V19, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_REDSTONE_TORCH_ON, BLOCK_RENDER_TORCH_V19, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_REDSTONE_TORCH_OFF, BLOCK_RENDER_TORCH_V19, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_LADDER, BLOCK_RENDER_LADDER_V19, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_RAIL, BLOCK_RENDER_RAIL_V43, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_DETECTOR_RAIL, BLOCK_RENDER_RAIL_V43, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_REDSTONE_WIRE, BLOCK_RENDER_REDSTONE_V43, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_REPEATER_OFF, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_REPEATER_ON, BLOCK_RENDER_FLAT_V19, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_STONE_BUTTON, BLOCK_RENDER_BUTTON_V43, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_LEVER, BLOCK_RENDER_LEVER_V43, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_STONE_PRESSURE_PLATE, BLOCK_RENDER_PRESSURE_V43, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_WOOD_PRESSURE_PLATE, BLOCK_RENDER_PRESSURE_V43, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_SIGN_POST, BLOCK_RENDER_SIGN_POST_V43, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_SIGN_WALL, BLOCK_RENDER_SIGN_POST_V43, 0, 0, 1, 0, 0, 0, 1, 1);

    BlockRegistryV57_SetFlags(BLOCK_SAPLING, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 1, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_TALL_GRASS, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 1, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_DEAD_BUSH, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 1, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_FLOWER_YELLOW, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 1, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_FLOWER_RED, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 1, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_MUSHROOM_BROWN, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 1, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_MUSHROOM_RED, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 1, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_CROPS, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 0, 0, 1, 1);
    BlockRegistryV57_SetFlags(BLOCK_REED, BLOCK_RENDER_CROSS_V19, 0, 0, 1, 0, 0, 0, 1, 1);

    /* Fluid/fire/portal layers. */
    BlockRegistryV57_SetFlags(BLOCK_WATER, BLOCK_RENDER_FLUID_V19, 0, 0, 1, 0, 1, 0, 0, 1);
    BlockRegistryV57_SetFlags(BLOCK_LAVA, BLOCK_RENDER_FLUID_V19, 0, 0, 1, 0, 1, 0, 0, 1);
    BlockRegistryV57_SetFlags(BLOCK_STATIONARY_LAVA, BLOCK_RENDER_FLUID_V19, 0, 0, 1, 0, 1, 0, 0, 1);
    BlockRegistryV57_SetFlags(BLOCK_FIRE, BLOCK_RENDER_FIRE_V19, 0, 0, 1, 0, 1, 0, 0, 1);
    BlockRegistryV57_SetFlags(BLOCK_PORTAL, BLOCK_RENDER_PORTAL_V43, 0, 0, 1, 0, 0, 0, 1, 1);

    /* Exact-ish bounds used by collision, selection, and mini-block rendering. */
    BlockRegistryV19_SetBounds(BLOCK_STEP, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
    BlockRegistryV19_SetBounds(BLOCK_FARMLAND, 0.0f, 0.0f, 0.0f, 1.0f, 0.9375f, 1.0f);
    BlockRegistryV19_SetBounds(BLOCK_SNOW, 0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
    BlockRegistryV19_SetBounds(BLOCK_CACTUS, 0.0625f, 0.0f, 0.0625f, 0.9375f, 1.0f, 0.9375f);
    BlockRegistryV19_SetBounds(BLOCK_CHEST, 0.0625f, 0.0f, 0.0625f, 0.9375f, 0.875f, 0.9375f);
    BlockRegistryV19_SetBounds(BLOCK_LOCKED_CHEST, 0.0625f, 0.0f, 0.0625f, 0.9375f, 0.875f, 0.9375f);
    BlockRegistryV19_SetBounds(BLOCK_BED, 0.0f, 0.0f, 0.0625f, 1.0f, 0.5625f, 0.9375f);
    BlockRegistryV19_SetBounds(BLOCK_CAKE, 0.0625f, 0.0f, 0.0625f, 0.9375f, 0.5f, 0.9375f);

    /* Material/harvest corrections that were previously scattered over mining code. */
    BlockRegistryV57_SetHarvest(BLOCK_GLASS, BLOCK_MATERIAL_GLASS_V49, BLOCK_COLLISION_FULL_V49, BLOCK_HARVEST_NONE_V49, 0, 0.3f, 1.5f, 0.60f, 0.60f);
    BlockRegistryV57_SetHarvest(BLOCK_ICE, BLOCK_MATERIAL_ICE_V49, BLOCK_COLLISION_FULL_V49, BLOCK_HARVEST_PICKAXE_V49, 0, 0.5f, 2.5f, 0.60f, 0.98f);
    BlockRegistryV57_SetHarvest(BLOCK_LEAVES, BLOCK_MATERIAL_LEAVES_V49, BLOCK_COLLISION_FULL_V49, BLOCK_HARVEST_SHEARS_V49, 0, 0.2f, 1.0f, 0.60f, 0.60f);
    BlockRegistryV57_SetHarvest(BLOCK_WEB, BLOCK_MATERIAL_PLANT_V49, BLOCK_COLLISION_NONE_V49, BLOCK_HARVEST_SWORD_V49, 0, 4.0f, 20.0f, 0.20f, 0.20f);
    BlockRegistryV57_SetHarvest(BLOCK_OBSIDIAN, BLOCK_MATERIAL_ROCK_V49, BLOCK_COLLISION_FULL_V49, BLOCK_HARVEST_PICKAXE_V49, 3, 50.0f, 2000.0f, 0.60f, 0.60f);
    BlockRegistryV57_SetHarvest(BLOCK_GOLD_BLOCK, BLOCK_MATERIAL_METAL_V49, BLOCK_COLLISION_FULL_V49, BLOCK_HARVEST_PICKAXE_V49, 2, 3.0f, 30.0f, 0.60f, 0.60f);
    BlockRegistryV57_SetHarvest(BLOCK_IRON_BLOCK, BLOCK_MATERIAL_METAL_V49, BLOCK_COLLISION_FULL_V49, BLOCK_HARVEST_PICKAXE_V49, 1, 5.0f, 30.0f, 0.60f, 0.60f);
    BlockRegistryV57_SetHarvest(BLOCK_DIAMOND_BLOCK, BLOCK_MATERIAL_METAL_V49, BLOCK_COLLISION_FULL_V49, BLOCK_HARVEST_PICKAXE_V49, 2, 5.0f, 30.0f, 0.60f, 0.60f);
    BlockRegistryV57_SetHarvest(BLOCK_TNT, BLOCK_MATERIAL_CLOTH_V49, BLOCK_COLLISION_FULL_V49, BLOCK_HARVEST_NONE_V49, 0, 0.0f, 0.0f, 0.60f, 0.60f);

    /* Java-like drops that must not come from the terrain render tile. */
    BlockRegistryV57_SetDrop(BLOCK_GRASS, ITEM_DIRT);
    BlockRegistryV57_SetDrop(BLOCK_STONE, ITEM_COBBLESTONE);
    BlockRegistryV57_SetDrop(BLOCK_FARMLAND, ITEM_DIRT);
    BlockRegistryV57_SetDrop(BLOCK_LAVA, ITEM_NONE);
    BlockRegistryV57_SetDrop(BLOCK_STATIONARY_LAVA, ITEM_NONE);
    BlockRegistryV57_SetDrop(BLOCK_FIRE, ITEM_NONE);
    BlockRegistryV57_SetDrop(BLOCK_PORTAL, ITEM_NONE);
}

int BlockV57_IsSupportSolid(int block)
{
    BlockDefV19 *d;
    if (block == BLOCK_AIR || block == BLOCK_BORDER) { return 0; }
    d = BlockRegistryV19_Get(block);
    if (d->normalCube && d->blocksMovement) { return 1; }
    if (block == BLOCK_FENCE || block == BLOCK_CHEST || block == BLOCK_LOCKED_CHEST) { return 1; }
    if (block == BLOCK_FURNACE || block == BLOCK_FURNACE_LIT || block == BLOCK_DISPENSER || block == BLOCK_WORKBENCH) { return 1; }
    if (block == BLOCK_JUKEBOX || block == BLOCK_NOTE || block == BLOCK_GLOWSTONE) { return 1; }
    return 0;
}

int BlockV57_ShouldSideBeRendered(int block, int neighbor)
{
    BlockDefV19 *bd;
    BlockDefV19 *nd;
    if (block == BLOCK_AIR) { return 0; }
    if (neighbor == BLOCK_AIR || neighbor == BLOCK_BORDER) { return 1; }
    bd = BlockRegistryV19_Get(block);
    nd = BlockRegistryV19_Get(neighbor);
    if (bd->renderType != BLOCK_RENDER_CUBE_V19 && bd->renderType != BLOCK_RENDER_SLAB_V19 && bd->renderType != BLOCK_RENDER_CACTUS_V19 && bd->renderType != BLOCK_RENDER_SNOW_V19) { return 1; }
    if (block == BLOCK_WATER) { return (neighbor == BLOCK_WATER) ? 0 : (nd->normalCube ? 0 : 1); }
    if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { return (neighbor == BLOCK_LAVA || neighbor == BLOCK_STATIONARY_LAVA) ? 0 : (nd->normalCube ? 0 : 1); }
    if (bd->translucent || !bd->opaque || bd->renderLayer != 0) {
        if (neighbor == block) { return 0; }
        if (neighbor == BLOCK_GLASS && block == BLOCK_GLASS) { return 0; }
        if (nd->normalCube && !nd->translucent) { return 0; }
        return 1;
    }
    if (nd->normalCube && nd->opaque) { return 0; }
    return 1;
}

int BlockV57_IsBlockSelectable(int block)
{
    BlockDefV19 *d;
    if (block == BLOCK_AIR || block == BLOCK_BORDER) { return 0; }
    d = BlockRegistryV19_Get(block);
    return d->selectable ? 1 : 0;
}

int BlockV57_GetRenderLayer(int block)
{
    return BlockRegistryV19_Get(block)->renderLayer;
}

int BlockV57_IsReplaceableAt(int block, int x, int y, int z)
{
    unsigned char meta;
    BlockDefV19 *d;
    (void)x;
    (void)y;
    (void)z;
    if (block == BLOCK_AIR || block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { return 1; }
    if (block == BLOCK_TALL_GRASS || block == BLOCK_DEAD_BUSH || block == BLOCK_FIRE || block == BLOCK_SNOW) { return 1; }
    if (block == BLOCK_SNOW && IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; if ((meta & 7) < 7) { return 1; } }
    d = BlockRegistryV19_Get(block);
    return d->replaceable ? 1 : 0;
}

int BlockV49_AddBox(AxisAlignedBBV39 *boxes, int count, int maxBoxes, int bx, int by, int bz, float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
    if (count >= maxBoxes) { return count; }
    boxes[count] = AABBV39_Make((double)bx + (double)minX, (double)by + (double)minY, (double)bz + (double)minZ, (double)bx + (double)maxX, (double)by + (double)maxY, (double)bz + (double)maxZ);
    return count + 1;
}

int BlockV49_AddDoorCollisionBoxes(int block, int bx, int by, int bz, AxisAlignedBBV39 *boxes, int maxBoxes)
{
    unsigned char meta;
    int dir;
    int open;
    float t;
    (void)block;
    meta = 0;
    if (IsInsideWorld(bx, by, bz)) { meta = g_blockMeta[bx][by][bz]; }
    dir = meta & 3;
    open = (meta & 4) ? 1 : 0;
    t = 0.1875f;
    if (open) {
        if (dir == 0) { return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.0f, t, 1.0f, 1.0f); }
        if (dir == 1) { return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, t); }
        if (dir == 2) { return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 1.0f - t, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f); }
        return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 0.0f, 1.0f - t, 1.0f, 1.0f, 1.0f);
    }
    if (dir == 0) { return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 0.0f, 1.0f - t, 1.0f, 1.0f, 1.0f); }
    if (dir == 1) { return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.0f, t, 1.0f, 1.0f); }
    if (dir == 2) { return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, t); }
    return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 1.0f - t, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

int BlockV49_AddTrapdoorCollisionBoxes(int block, int bx, int by, int bz, AxisAlignedBBV39 *boxes, int maxBoxes)
{
    unsigned char meta;
    int dir;
    float t;
    (void)block;
    meta = 0;
    if (IsInsideWorld(bx, by, bz)) { meta = g_blockMeta[bx][by][bz]; }
    dir = meta & 3;
    t = 0.1875f;
    if (meta & 4) {
        if (dir == 0) { return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 0.0f, 1.0f - t, 1.0f, 1.0f, 1.0f); }
        if (dir == 1) { return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, t); }
        if (dir == 2) { return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 1.0f - t, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f); }
        return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.0f, t, 1.0f, 1.0f);
    }
    if (meta & 8) { return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 1.0f - t, 0.0f, 1.0f, 1.0f, 1.0f); }
    return BlockV49_AddBox(boxes, 0, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.0f, 1.0f, t, 1.0f);
}

int BlockV49_AddStairCollisionBoxes(int block, int bx, int by, int bz, AxisAlignedBBV39 *boxes, int maxBoxes)
{
    unsigned char meta;
    int dir;
    int count;
    (void)block;
    meta = 0;
    if (IsInsideWorld(bx, by, bz)) { meta = g_blockMeta[bx][by][bz]; }
    dir = meta & 3;
    count = 0;
    if (meta & 4) {
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
        if (dir == 0) { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.5f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f); }
        else if (dir == 1) { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f); }
        else if (dir == 2) { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f); }
        else { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f); }
    } else {
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
        if (dir == 0) { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f); }
        else if (dir == 1) { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f); }
        else if (dir == 2) { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f); }
        else { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 0.5f); }
    }
    return count;
}

int BlockV49_AddFenceCollisionBoxes(int block, int bx, int by, int bz, AxisAlignedBBV39 *boxes, int maxBoxes)
{
    int count;
    int north;
    int south;
    int west;
    int east;
    (void)block;
    count = 0;
    north = BlockV43_ConnectsFence(GetBlock(bx, by, bz - 1));
    south = BlockV43_ConnectsFence(GetBlock(bx, by, bz + 1));
    west = BlockV43_ConnectsFence(GetBlock(bx - 1, by, bz));
    east = BlockV43_ConnectsFence(GetBlock(bx + 1, by, bz));
    if (!north && !south && !west && !east) { north = south = west = east = 1; }
    count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.375f, 0.0f, 0.375f, 0.625f, 1.5f, 0.625f);
    if (north) { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.375f, 0.0f, 0.0f, 0.625f, 1.5f, 0.5f); }
    if (south) { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.375f, 0.0f, 0.5f, 0.625f, 1.5f, 1.0f); }
    if (west) { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.0f, 0.0f, 0.375f, 0.5f, 1.5f, 0.625f); }
    if (east) { count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.5f, 0.0f, 0.375f, 1.0f, 1.5f, 0.625f); }
    return count;
}

int BlockV57_AddPistonExtensionCollisionBoxes(int block, int bx, int by, int bz, AxisAlignedBBV39 *boxes, int maxBoxes)
{
    unsigned char meta;
    int dir;
    int count;
    (void)block;
    meta = 0;
    if (IsInsideWorld(bx, by, bz)) { meta = g_blockMeta[bx][by][bz]; }
    dir = meta & 7;
    count = 0;
    if (dir == 0) {
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.25f, 0.00f, 0.25f, 0.75f, 0.75f, 0.75f);
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.00f, 0.75f, 0.00f, 1.00f, 1.00f, 1.00f);
    } else if (dir == 1) {
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.25f, 0.25f, 0.25f, 0.75f, 1.00f, 0.75f);
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.00f, 0.00f, 0.00f, 1.00f, 0.25f, 1.00f);
    } else if (dir == 2) {
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.25f, 0.25f, 0.00f, 0.75f, 0.75f, 0.75f);
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.00f, 0.00f, 0.75f, 1.00f, 1.00f, 1.00f);
    } else if (dir == 3) {
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.25f, 0.25f, 0.25f, 0.75f, 0.75f, 1.00f);
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.00f, 0.00f, 0.00f, 1.00f, 1.00f, 0.25f);
    } else if (dir == 4) {
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.00f, 0.25f, 0.25f, 0.75f, 0.75f, 0.75f);
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.75f, 0.00f, 0.00f, 1.00f, 1.00f, 1.00f);
    } else {
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.25f, 0.25f, 0.25f, 1.00f, 0.75f, 0.75f);
        count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, 0.00f, 0.00f, 0.00f, 0.25f, 1.00f, 1.00f);
    }
    return count;
}

int BlockV49_AddCollisionBoxes(int block, int bx, int by, int bz, AxisAlignedBBV39 *boxes, int maxBoxes)
{
    BlockDefV19 *d;
    unsigned char meta;
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
    int count;
    if (maxBoxes <= 0) { return 0; }
    if (block == BLOCK_AIR) { return 0; }
    d = BlockRegistryV19_Get(block);
    if (!d->blocksMovement || d->collisionType == BLOCK_COLLISION_NONE_V49) { return 0; }
    if (d->collisionType == BLOCK_COLLISION_DOOR_V49) { return BlockV49_AddDoorCollisionBoxes(block, bx, by, bz, boxes, maxBoxes); }
    if (d->collisionType == BLOCK_COLLISION_TRAPDOOR_V49) { return BlockV49_AddTrapdoorCollisionBoxes(block, bx, by, bz, boxes, maxBoxes); }
    if (d->collisionType == BLOCK_COLLISION_STAIRS_V49) { return BlockV49_AddStairCollisionBoxes(block, bx, by, bz, boxes, maxBoxes); }
    if (d->collisionType == BLOCK_COLLISION_FENCE_V49) { return BlockV49_AddFenceCollisionBoxes(block, bx, by, bz, boxes, maxBoxes); }
    if (d->collisionType == BLOCK_COLLISION_PISTON_V49 && block == BLOCK_PISTON_EXTENSION) { return BlockV57_AddPistonExtensionCollisionBoxes(block, bx, by, bz, boxes, maxBoxes); }
    meta = 0;
    if (IsInsideWorld(bx, by, bz)) { meta = g_blockMeta[bx][by][bz]; }
    minX = d->minX; minY = d->minY; minZ = d->minZ; maxX = d->maxX; maxY = d->maxY; maxZ = d->maxZ;
    if (block == BLOCK_STEP) { if (meta & 8) { minY = 0.5f; maxY = 1.0f; } else { minY = 0.0f; maxY = 0.5f; } }
    if (block == BLOCK_FARMLAND) { maxY = 0.9375f; }
    if (block == BLOCK_CACTUS) { minX = 0.0625f; minZ = 0.0625f; maxX = 0.9375f; maxZ = 0.9375f; }
    if (block == BLOCK_SNOW) { maxY = (float)(((meta & 7) + 1) * 2) / 16.0f; if (maxY < 0.125f) { maxY = 0.125f; } }
    if (block == BLOCK_CAKE) { minX = 0.0625f + (float)(meta & 7) * 0.125f; minZ = 0.0625f; maxX = 0.9375f; maxY = 0.5f; maxZ = 0.9375f; }
    if (block == BLOCK_BED) { maxY = 0.5625f; }
    if (block == BLOCK_CHEST || block == BLOCK_LOCKED_CHEST) { minX = 0.0625f; minZ = 0.0625f; maxX = 0.9375f; maxY = 0.875f; maxZ = 0.9375f; }
    if (block == BLOCK_SOULSAND) { maxY = 0.875f; }
    if (block == BLOCK_STONE_PRESSURE_PLATE || block == BLOCK_WOOD_PRESSURE_PLATE) { minX = 0.0625f; minZ = 0.0625f; maxX = 0.9375f; maxZ = 0.9375f; maxY = (meta & 8) ? 0.03125f : 0.0625f; }
    count = 0;
    count = BlockV49_AddBox(boxes, count, maxBoxes, bx, by, bz, minX, minY, minZ, maxX, maxY, maxZ);
    return count;
}

int BlockV49_GetPrimaryCollisionBounds(int block, int bx, int by, int bz, double *minX, double *minY, double *minZ, double *maxX, double *maxY, double *maxZ)
{
    AxisAlignedBBV39 tmp[8];
    int n;
    n = BlockV49_AddCollisionBoxes(block, bx, by, bz, tmp, 8);
    if (n <= 0) {
        *minX = (double)bx; *minY = (double)by; *minZ = (double)bz;
        *maxX = (double)bx + 1.0; *maxY = (double)by + 1.0; *maxZ = (double)bz + 1.0;
        return 0;
    }
    *minX = tmp[0].minX; *minY = tmp[0].minY; *minZ = tmp[0].minZ;
    *maxX = tmp[0].maxX; *maxY = tmp[0].maxY; *maxZ = tmp[0].maxZ;
    return 1;
}

int BlockV19_IsDirectionalFrontFace(int face, unsigned char meta)
{
    int dir;
    dir = meta & 7;
    if (dir == 2 && face == BLOCK_FACE_NORTH_V19) { return 1; }
    if (dir == 3 && face == BLOCK_FACE_SOUTH_V19) { return 1; }
    if (dir == 4 && face == BLOCK_FACE_WEST_V19) { return 1; }
    if (dir == 5 && face == BLOCK_FACE_EAST_V19) { return 1; }
    if (dir == 0 && face == BLOCK_FACE_SOUTH_V19) { return 1; }
    if (dir == 1 && face == BLOCK_FACE_EAST_V19) { return 1; }
    return 0;
}

int BlockV19_GetFaceJavaTileAt(int block, int x, int y, int z, int face)
{
    BlockDefV19 *d;
    unsigned char meta;
    int tex;
    d = BlockRegistryV19_Get(block);
    meta = 0;
    if (IsInsideWorld(x, y, z)) { meta = g_blockMeta[x][y][z]; }
    if (face == BLOCK_FACE_TOP_V19) { return d->topTex; }
    if (face == BLOCK_FACE_BOTTOM_V19) { return d->bottomTex; }
    if (BlockV19_IsDirectionalFrontFace(face, meta) && d->frontTex >= 0) { return d->frontTex; }
    tex = -1;
    if (face == BLOCK_FACE_NORTH_V19) { tex = d->backTex; }
    else if (face == BLOCK_FACE_SOUTH_V19) { tex = d->frontTex; }
    else if (face == BLOCK_FACE_WEST_V19) { tex = d->leftTex; }
    else if (face == BLOCK_FACE_EAST_V19) { tex = d->rightTex; }
    if (tex >= 0) { return tex; }
    return d->sideTex;
}

void BlockV19_GetTextureTileAt(int block, int x, int y, int z, int face, int *col, int *row)
{
    int index;
    int ac;
    int ar;
    index = BlockV19_GetFaceJavaTileAt(block, x, y, z, face);
    ResourceV10_JavaTileToAtlas(index, col, row);
    if (ResourceV10_GetAnimatedTile(block, *col, *row, &ac, &ar)) {
        *col = ac;
        *row = ar;
    }
}

void GetBlockTile(int block, int face, int *col, int *row)
{
    BlockV19_GetTextureTileAt(block, 0, 0, 0, face, col, row);
}


/* Open Watcom link fix: V5/V7 code calls GetBlockTextureTile, while
   the atlas mapper was implemented as GetBlockTile. Keep this wrapper
   so older call sites and newly added special block renderers link cleanly. */
void GetBlockTextureTile(int block, int face, int *col, int *row)
{
    GetBlockTile(block, face, col, row);
}

/* V12_ORIGINAL_WORLD_TEXTURE_REVERT:
   Remove lightweight plant/flower/mushroom blocks from gameplay/rendering.
   Cactus, logs and leaves are intentionally not removed. */
int IsRemovedPlantBlockV12(int block)
{
    /* V19_RENDERBLOCKS_PORT:
       V12 temporarily removed light plant blocks to avoid bad placeholder
       rendering.  RenderBlocks-style crossed planes are now implemented, so
       saplings, flowers, mushrooms, reeds, crops and tall grass should remain
       in the world and render through the central BlockDef table. */
    (void)block;
    return 0;
}


void RemovePlantsFromWorldV12(void)
{
    int x;
    int y;
    int z;
    for (x = 0; x < WORLD_X; x++) {
        for (z = 0; z < WORLD_Z; z++) {
            for (y = 1; y < WORLD_Y; y++) {
                if (IsRemovedPlantBlockV12(world[x][y][z])) {
                    world[x][y][z] = BLOCK_AIR;
                }
            }
        }
    }
}



void DrawTexturedQuad2D(GLuint texture, int atlasWidth, int atlasHeight, int col, int row, int x1, int y1, int x2, int y2)
{
    float u0;
    float v0;
    float u1;
    float v1;

    GetTileUVEx(col, row, atlasWidth, atlasHeight, &u0, &v0, &u1, &v1);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    glTexCoord2f(u0, v0); glVertex2i(x1, y1);
    glTexCoord2f(u1, v0); glVertex2i(x2, y1);
    glTexCoord2f(u1, v1); glVertex2i(x2, y2);
    glTexCoord2f(u0, v1); glVertex2i(x1, y2);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void DrawImage2D(GLuint texture, int x1, int y1, int x2, int y2, float alpha)
{
    if (!texture) {
        return;
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(x1, y1);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(x2, y1);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(x2, y2);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(x1, y2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}


void DrawImageCrop2D(GLuint texture, int atlasW, int atlasH,
                     int sx, int sy, int sw, int sh,
                     int x1, int y1, int x2, int y2, float alpha)
{
    float u0;
    float v0;
    float u1;
    float v1;

    if (!texture) {
        return;
    }

    if (atlasW <= 0 || atlasH <= 0 || sw <= 0 || sh <= 0) {
        return;
    }

    u0 = (float)sx / (float)atlasW;
    v0 = (float)sy / (float)atlasH;
    u1 = (float)(sx + sw) / (float)atlasW;
    v1 = (float)(sy + sh) / (float)atlasH;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(u0, v0); glVertex2i(x1, y1);
    glTexCoord2f(u1, v0); glVertex2i(x2, y1);
    glTexCoord2f(u1, v1); glVertex2i(x2, y2);
    glTexCoord2f(u0, v1); glVertex2i(x1, y2);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void DrawImage3DBillboard(GLuint texture, float cx, float cy, float cz, float width, float height, float alpha)
{
    float halfW;
    float halfH;
    float rightX;
    float rightZ;
    float yawRad;
    if (!texture) {
        return;
    }
    halfW = width * 0.5f;
    halfH = height * 0.5f;
    yawRad = (float)(yaw * PI / 180.0);
    rightX = cos(yawRad) * halfW;
    rightZ = sin(yawRad) * halfW;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(cx - rightX, cy - halfH, cz - rightZ);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(cx + rightX, cy - halfH, cz + rightZ);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(cx + rightX, cy + halfH, cz + rightZ);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(cx - rightX, cy + halfH, cz - rightZ);
    glEnd();
}

void DrawTerrainTile2D(int col, int row, int x1, int y1, int x2, int y2)
{
    DrawTexturedQuad2D(texTerrain, TERRAIN_ATLAS_WIDTH, TERRAIN_ATLAS_HEIGHT, col, row, x1, y1, x2, y2);
}

void DrawIconTile2D(int col, int row, int x1, int y1, int x2, int y2)
{
    DrawTexturedQuad2D(texIcons, ICONS_ATLAS_WIDTH, ICONS_ATLAS_HEIGHT, col, row, x1, y1, x2, y2);
}

/* ------------------------------------------------------------ */
/* Main loop                                                    */
/* ------------------------------------------------------------ */

void MainLoop(void)
{
    MSG msg;
    DWORD now;
    double dt;

    lastTime = GetTickCount();

    while (g_running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_running = 0;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        now = GetTickCount();
        dt = (double)(now - lastTime) / 1000.0;
        lastTime = now;

        if (dt > 0.05) {
            dt = 0.05;
        }

        UpdateFPSCounter(dt);

        /*
            Music is updated for both menu and gameplay.
        */
        UpdateMusic(dt);
        UpdateAmbientSoundsV35(dt);
        UpdateFeatureGapSystems(dt);
        ResourceV10_UpdateAnimations(dt);

        if (g_state == STATE_GAME) {
            GameUpdate(dt);
        }

        GameRender();

        Sleep(1);
    }
}

/* ------------------------------------------------------------ */
/* Input, mouse capture, safe spawn, and MP3 music              */
/* ------------------------------------------------------------ */

void ResetInputState(void)
{
    keyForward = 0;
    keyBack = 0;
    CancelHeldBlockMiningV11();
    keyLeft = 0;
    keyRight = 0;
    keyJump = 0;
    keySneak = 0;
}

void SetGameKey(WPARAM key, int down)
{
    /* GUI_LIGHT_V7: controls remapping.  The default bindings remain W/A/S/D,
       Space, E, Q, and F5, but GuiControls can change them at runtime. */
    if (key == g_keyBindForwardV7) {
        keyForward = down;
    } else if (key == g_keyBindBackV7) {
        keyBack = down;
    } else if (key == g_keyBindLeftV7) {
        keyLeft = down;
    } else if (key == g_keyBindRightV7) {
        keyRight = down;
    } else if (key == g_keyBindJumpV7) {
        keyJump = down;
    } else if (key == VK_SHIFT || key == VK_CONTROL) {
        keySneak = down;
    }
}


void HideGameCursor(void)
{
    /*
        Force cursor hidden even if Windows' internal show-count is not zero.
    */
    while (ShowCursor(FALSE) >= 0) {
    }
}

void ShowGameCursor(void)
{
    /*
        Force cursor visible again when leaving gameplay/inventory.
    */
    while (ShowCursor(TRUE) < 0) {
    }
}

void LockMouseForGame(void)
{
    if (!g_hwnd) {
        return;
    }

    mouseLocked = 1;
    SetCapture(g_hwnd);
    HideGameCursor();
    CenterMouse();
}

void UnlockMouseFromGame(void)
{
    mouseLocked = 0;
    ReleaseCapture();
    ShowGameCursor();
}

void LockMouseToGame(void)
{
    /* Open Watcom/Win32 compatibility wrapper for GUI V9 call sites.
       Older code used LockMouseForGame(); the GUI pipeline calls the
       Java-style LockMouseToGame() name when returning from text fields,
       menus, or texture-pack screens. Keep both symbols so a single-file
       build cannot link-fail. */
    LockMouseForGame();
}


int IsSkyOpenForSpawn(int bx, int by, int bz)
{
    int y;
    int block;

    if (bx < 2 || bx >= WORLD_X - 2 || bz < 2 || bz >= WORLD_Z - 2) {
        return 0;
    }

    for (y = by + 2; y < WORLD_Y - 1; y++) {
        block = GetBlock(bx, y, bz);

        /* Leaves are allowed so spawning near a tree does not fail. */
        if (block != BLOCK_AIR && block != BLOCK_LEAVES) {
            return 0;
        }
    }

    return 1;
}

void ForceSpawnPad(int x, int y, int z)
{
    int dx;
    int dz;
    int yy;
    int px;
    int pz;
    int groundBlock;

    if (y < PLAYER_SPAWN_MIN_Y) {
        y = PLAYER_SPAWN_MIN_Y;
    }

    if (y > PLAYER_SPAWN_MAX_Y) {
        y = PLAYER_SPAWN_MAX_Y;
    }

    if (y + 4 >= WORLD_Y) {
        y = WORLD_Y - 6;
    }

    for (dx = -2; dx <= 2; dx++) {
        for (dz = -2; dz <= 2; dz++) {
            px = x + dx;
            pz = z + dz;

            if (!IsInsideWorld(px, y - 3, pz)) {
                continue;
            }

            if (dx == -2 || dx == 2 || dz == -2 || dz == 2) {
                groundBlock = BLOCK_COBBLESTONE;
            } else if ((dx == 0 && dz == 0) || (WorldHash3D(px, y, pz, g_worldSeed + 28001) & 1)) {
                groundBlock = BLOCK_GRASS;
            } else {
                groundBlock = BLOCK_DIRT;
            }

            world[px][y - 1][pz] = groundBlock;
            world[px][y - 2][pz] = BLOCK_DIRT;
            world[px][y - 3][pz] = BLOCK_STONE;

            for (yy = y; yy <= y + 4 && yy < WORLD_Y; yy++) {
                world[px][yy][pz] = BLOCK_AIR;
            }

            RebuildColumnTopAt(px, pz);
        }
    }
}



int IsSpawnSpaceClear(double sx, double sy, double sz)
{
    int bx;
    int by;
    int bz;
    int feet;
    int head;
    int above;
    int ground;

    bx = (int)floor(sx);
    by = (int)floor(sy);
    bz = (int)floor(sz);

    if (by < PLAYER_SPAWN_MIN_Y || by > PLAYER_SPAWN_MAX_Y || by + 3 >= WORLD_Y) {
        return 0;
    }

    if (bx < 3 || bz < 3 || bx >= WORLD_X - 3 || bz >= WORLD_Z - 3) {
        return 0;
    }

    ground = GetBlock(bx, by - 1, bz);

    if (!IsValidSpawnGround(ground)) {
        return 0;
    }

    feet = GetBlock(bx, by, bz);
    head = GetBlock(bx, by + 1, bz);
    above = GetBlock(bx, by + 2, bz);

    if (feet != BLOCK_AIR || head != BLOCK_AIR || above != BLOCK_AIR) {
        return 0;
    }

    if (!IsSkyOpenForSpawn(bx, by, bz)) {
        return 0;
    }

    if (PlayerCollidesAt(sx, sy, sz)) {
        return 0;
    }

    return 1;
}



int FindSafeSpawn(double *sx, double *sy, double *sz)
{
    int centerX;
    int centerZ;
    int radius;
    int x;
    int y;
    int z;
    int maxRadius;
    int bestX;
    int bestY;
    int bestZ;
    int candidateY;

    centerX = WORLD_X / 2;
    centerZ = WORLD_Z / 2;

    if (WORLD_X < WORLD_Z) {
        maxRadius = WORLD_X / 2 - 4;
    } else {
        maxRadius = WORLD_Z / 2 - 4;
    }

    if (maxRadius < 8) {
        maxRadius = 8;
    }

    bestX = centerX;
    bestZ = centerZ;
    bestY = 36 + (WorldHash2D(worldOriginBlockX, worldOriginBlockZ, g_worldSeed + 28100) % 11);

    if (bestY < PLAYER_SPAWN_MIN_Y) { bestY = PLAYER_SPAWN_MIN_Y; }
    if (bestY > PLAYER_SPAWN_MAX_Y) { bestY = PLAYER_SPAWN_MAX_Y; }

    for (radius = 0; radius < maxRadius; radius++) {
        for (x = centerX - radius; x <= centerX + radius; x++) {
            for (z = centerZ - radius; z <= centerZ + radius; z++) {
                if (x < 4 || z < 4 || x >= WORLD_X - 4 || z >= WORLD_Z - 4) {
                    continue;
                }

                if (abs(x - centerX) != radius && abs(z - centerZ) != radius) {
                    continue;
                }

                for (y = PLAYER_SPAWN_MAX_Y; y >= PLAYER_SPAWN_MIN_Y; y--) {
                    if (!IsValidSpawnGround(GetBlock(x, y - 1, z))) {
                        continue;
                    }

                    if (IsSpawnSpaceClear((double)x + 0.5, (double)y, (double)z + 0.5)) {
                        *sx = (double)x + 0.5;
                        *sy = (double)y;
                        *sz = (double)z + 0.5;
                        return 1;
                    }
                }
            }
        }
    }

    candidateY = bestY;
    ForceSpawnPad(bestX, candidateY, bestZ);

    *sx = (double)bestX + 0.5;
    *sy = (double)candidateY;
    *sz = (double)bestZ + 0.5;

    return 1;
}



void StopAllMusic(void)
{
    StopMobSounds();

    mciSendString("stop menuMusic", NULL, 0, NULL);
    mciSendString("close menuMusic", NULL, 0, NULL);

    mciSendString("stop gameMusic", NULL, 0, NULL);
    mciSendString("close gameMusic", NULL, 0, NULL);

    musicMode = 0;
}

int PlayMusicFile(const char *filename, const char *aliasName, int repeat)
{
    char cmd[512]; char path[260]; const char *deviceType; int len; MCIERROR err;
    if (!SoundResolveExistingPathV35(filename, path)) { return 0; }
    sprintf(cmd, "close %s", aliasName); mciSendString(cmd, NULL, 0, NULL);
    deviceType = "mpegvideo"; len = (int)strlen(path); if (len > 4 && strcmp(path + len - 4, ".wav") == 0) { deviceType = "waveaudio"; }
    sprintf(cmd, "open \"%s\" type %s alias %s", path, deviceType, aliasName); err = mciSendString(cmd, NULL, 0, NULL); if (err != 0) { return 0; }
    SoundApplyMciControlsV35(aliasName, 1.0, 1.0, 1);
    if (repeat) { sprintf(cmd, "play %s repeat", aliasName); } else { sprintf(cmd, "play %s", aliasName); }
    err = mciSendString(cmd, NULL, 0, NULL); if (err != 0) { sprintf(cmd, "close %s", aliasName); mciSendString(cmd, NULL, 0, NULL); return 0; }
    return 1;
}


int IsMusicAliasPlaying(const char *aliasName)
{
    char cmd[128];
    char status[64];
    MCIERROR err;

    status[0] = '\0';

    sprintf(cmd, "status %s mode", aliasName);
    err = mciSendString(cmd, status, sizeof(status), NULL);

    if (err != 0) {
        return 0;
    }

    if (strcmp(status, "playing") == 0) {
        return 1;
    }

    return 0;
}

void StartMenuMusic(void)
{
    if (musicMode == 1 && IsMusicAliasPlaying("menuMusic")) {
        return;
    }

    StopAllMusic();

    /*
        Menu uses Oxygene only.
    */
    if (PlayMusicFile(MUSIC_MENU_FILE, "menuMusic", 1)) {
        musicMode = 1;
    }
}

void StartGameMusic(void)
{
    int nextSong;
    const char *songs[GAME_MUSIC_COUNT];

    songs[0] = MUSIC_GAME_FILE1;
    songs[1] = MUSIC_GAME_FILE2;
    songs[2] = MUSIC_GAME_FILE3;
    songs[3] = MUSIC_GAME_FILE4;
    songs[4] = MUSIC_GAME_FILE5;
    songs[5] = MUSIC_GAME_FILE6;
    songs[6] = MUSIC_GAME_FILE7;
    songs[7] = MUSIC_GAME_FILE8;
    songs[8] = MUSIC_GAME_FILE9;
    songs[9] = MUSIC_GAME_FILE10;
    songs[10] = MUSIC_GAME_FILE11;
    songs[11] = MUSIC_GAME_FILE12;
    songs[12] = MUSIC_GAME_FILE13;
    songs[13] = MUSIC_GAME_FILE14;
    songs[14] = MUSIC_GAME_FILE15;
    songs[15] = MUSIC_GAME_FILE16;
    songs[16] = MUSIC_GAME_FILE17;

    StopAllMusic();

    nextSong = (int)((GetTickCount() ^ WorldHash2D((int)playerX, (int)playerZ, g_worldSeed + currentGameSong * 97)) % GAME_MUSIC_COUNT);

    if (GAME_MUSIC_COUNT > 1 && nextSong == currentGameSong) {
        nextSong = (nextSong + 1 + (int)(GetTickCount() & 3)) % GAME_MUSIC_COUNT;
    }

    currentGameSong = nextSong;

    if (!PlayMusicFile(songs[currentGameSong], "gameMusic", 0)) {
        if (currentGameSong != 0) {
            PlayMusicFile(MUSIC_GAME_FILE1, "gameMusic", 0);
            currentGameSong = 0;
        }
    }

    musicMode = 2;
    gameMusicMinTimer = SOUND_MUSIC_MIN_DELAY_V35 + SoundRandUnitV35() * SOUND_MUSIC_RANDOM_DELAY_V35;
}

void UpdateMusic(double dt)
{
    if (g_state == STATE_MENU || g_state == STATE_SETTINGS ||
        g_state == STATE_WORLD_SELECT || g_state == STATE_CREATE_WORLD ||
        g_state == STATE_OPTIONS) {
        if (musicMode != 1 || !IsMusicAliasPlaying("menuMusic")) {
            StartMenuMusic();
        }

        return;
    }

    if (g_state == STATE_GAME) {
        if (musicMode != 2) {
            StartGameMusic();
            return;
        }

        if (gameMusicMinTimer > 0.0) {
            gameMusicMinTimer -= dt;
        }

        /*
            If the current song has ended and at least a minute has passed,
            choose another gameplay song.
        */
        if (gameMusicMinTimer <= 0.0 && !IsMusicAliasPlaying("gameMusic")) {
            StartGameMusic();
        }
    }
}



/* ------------------------------------------------------------ */
/* Window messages                                              */
/* ------------------------------------------------------------ */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int mx;
    int my;
    int i;

    switch (msg) {
    case WM_SIZE:
        g_windowWidth = LOWORD(lParam);
        g_windowHeight = HIWORD(lParam);

        if (g_windowWidth <= 0) {
            g_windowWidth = 1;
        }
        if (g_windowHeight <= 0) {
            g_windowHeight = 1;
        }

        LayoutBetaMenus();

        return 0;

    case WM_SETFOCUS:
        if (g_state == STATE_GAME && !inventoryOpen) {
            LockMouseForGame();
        }

        return 0;

    case WM_KILLFOCUS:
        ResetInputState();
        UnlockMouseFromGame();
        SaveCurrentWorld();
        return 0;

    case WM_CLOSE:
        SaveCurrentWorld();
        g_running = 0;
        StopAllMusic();
        PostQuitMessage(0);
        return 0;

    case WM_DESTROY:
        SaveCurrentWorld();
        g_running = 0;
        StopAllMusic();
        PostQuitMessage(0);
        return 0;

    case WM_CHAR:
        if (GuiV9_HandleChar(wParam)) { return 0; }
        if (GuiV7_HandleChar(wParam)) { return 0; }
        if (IsSignEditingV5()) {
            HandleSignCharV5(wParam);
            return 0;
        }
        if (g_state == STATE_CREATE_WORLD) {
            HandleCreateWorldChar(wParam);
            return 0;
        }

        return 0;

    case WM_KEYDOWN:
        if (IsSignEditingV5()) {
            if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_HOME || wParam == VK_END || wParam == VK_DELETE) { HandleSignKeyV36(wParam); return 0; }
        }
        if (wParam == VK_F11) {
            ToggleFullscreen();
            return 0;
        }

        if (GuiV9_HandleKeyDown(wParam)) {
            return 0;
        }

        if (GuiV7_HandleKeyDown(wParam)) {
            return 0;
        }

        if (g_state == STATE_DEATH) {
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                RespawnPlayerAtWorldSpawn();
                return 0;
            }
            if (wParam == VK_ESCAPE) {
                SaveCurrentWorld();
                EnterMenu();
                return 0;
            }
            return 0;
        }

        if (g_state == STATE_GAME) {
            SetGameKey(wParam, 1);
        }

        if (wParam == VK_ESCAPE) {
            if (g_state == STATE_GAME) {
                if (inventoryOpen) {
                    CloseActiveContainerV5();
                } else {
                    SaveCurrentWorld();
                    EnterPauseMenu();
                }
            } else if (g_state == STATE_PAUSE) {
                EnterGame();
            } else if (g_state == STATE_WORLD_SELECT) {
                EnterMenu();
            } else if (g_state == STATE_CREATE_WORLD) {
                EnterWorldSelect();
            } else if (g_state == STATE_OPTIONS || g_state == STATE_SETTINGS ||
                       g_state == STATE_VIDEO_SETTINGS || g_state == STATE_CONTROLS ||
                       g_state == STATE_ACHIEVEMENTS || g_state == STATE_STATS ||
                       g_state == STATE_RENAME_WORLD || g_state == STATE_MULTIPLAYER ||
                       g_state == STATE_CONNECTING || g_state == STATE_CONNECT_FAILED ||
                       g_state == STATE_TEXTURE_PACKS || g_state == STATE_CONFLICT_WARNING ||
                       g_state == STATE_ERROR_SCREEN || g_state == STATE_YESNO) {
                if (g_state == STATE_RENAME_WORLD) { EnterWorldSelect(); return 0; }
                if (g_state == STATE_VIDEO_SETTINGS || g_state == STATE_CONTROLS ||
                    g_state == STATE_ACHIEVEMENTS || g_state == STATE_STATS) {
                    g_state = STATE_OPTIONS;
                    LayoutBetaMenus();
                    return 0;
                }
                if (g_optionsReturnState == STATE_PAUSE) {
                    EnterPauseMenu();
                } else {
                    EnterMenu();
                }
            } else {
                SaveCurrentWorld();
                g_running = 0;
                StopAllMusic();
                PostQuitMessage(0);
            }

            return 0;
        }

        if (g_state == STATE_CREATE_WORLD) {
            if (wParam == VK_TAB) {
                createInputField = 1 - createInputField;
                return 0;
            }
        }

        if (g_state == STATE_GAME) {
            if (wParam == g_keyBindInventoryV7) {
                if (inventoryOpen) {
                    CloseActiveContainerV5();
                } else {
                    g_containerModeV5 = CONTAINER_NONE_V5;
                    g_activeTileIndexV5 = -1;
                    inventoryOpen = 1;
                    craftingOpen = 0;
                    UnlockMouseFromGame();
                }
                return 0;
            }

            if (wParam >= '1' && wParam <= '9') {
                SelectHotbarSlot((int)(wParam - '1'));
                return 0;
            }

            if (wParam == g_keyBindDropV7) {
                DropSelectedItem();
                return 0;
            }

            if (wParam == g_keyBindPerspectiveV7) {
                g_cameraMode++;
                if (g_cameraMode > CAMERA_THIRD_FRONT) {
                    g_cameraMode = CAMERA_FIRST_PERSON;
                }
                return 0;
            }

            /* Quick render-distance cycle while testing performance. */
            if (wParam == 'R') {
                ChangeRenderDistance(1);
                return 0;
            }
        }

        return 0;

    case WM_KEYUP:
        if (g_state == STATE_GAME) {
            SetGameKey(wParam, 0);
        }

        return 0;

    case WM_LBUTTONDOWN:
        mx = LOWORD(lParam);
        my = HIWORD(lParam);

        if (g_state != STATE_GAME || inventoryOpen) {
            PlayUIClickSound();
        }

        if (GuiV9_HandleMouseDown(mx, my)) {
            return 0;
        }

        if (GuiV7_HandleMouseDown(mx, my)) {
            return 0;
        }

        if (g_state == STATE_MENU) {
            if (PointInRectInt(mx, my, singleplayerButton)) {
                EnterWorldSelect();
            } else if (0 && PointInRectInt(mx, my, multiplayerButton)) {
                /* V11: multiplayer is intentionally hidden/disabled from the title menu. */
            } else if (PointInRectInt(mx, my, texturePackButton)) {
                GuiV9_EnterTexturePacks();
            } else if (PointInRectInt(mx, my, optionsButton)) {
                EnterOptions();
            } else if (PointInRectInt(mx, my, quitButton)) {
                SaveCurrentWorld();
                g_running = 0;
                StopAllMusic();
                PostQuitMessage(0);
            }
        } else if (g_state == STATE_WORLD_SELECT) {
            for (i = 0; i < MAX_WORLD_SLOTS; i++) {
                if (PointInRectInt(mx, my, worldSlotButtons[i])) {
                    selectedWorldSlot = i;
                    return 0;
                }
            }

            if (PointInRectInt(mx, my, worldPlayButton)) {
                if (selectedWorldSlot >= 0 && selectedWorldSlot < MAX_WORLD_SLOTS &&
                    worldSaves[selectedWorldSlot].exists) {
                    StartWorldSlot(selectedWorldSlot);
                }
            } else if (PointInRectInt(mx, my, worldRenameButton)) {
                if (selectedWorldSlot >= 0 && selectedWorldSlot < MAX_WORLD_SLOTS &&
                    worldSaves[selectedWorldSlot].exists) {
                    GuiV7_EnterRenameWorld();
                }
            } else if (PointInRectInt(mx, my, worldCreateButton)) {
                EnterCreateWorld();
            } else if (PointInRectInt(mx, my, worldDeleteButton)) {
                if (worldSaves[selectedWorldSlot].exists) {
                    GuiV9_EnterYesNo("Delete World", "This world will be deleted permanently.", worldSaves[selectedWorldSlot].name, 1, selectedWorldSlot);
                }
            } else if (PointInRectInt(mx, my, worldBackButton)) {
                EnterMenu();
            }
        } else if (g_state == STATE_CREATE_WORLD) {
            if (PointInRectInt(mx, my, createNameField)) {
                createInputField = 0;
            } else if (PointInRectInt(mx, my, createSeedField)) {
                createInputField = 1;
            } else if (PointInRectInt(mx, my, createWorldSizeButton)) {
                ChangeCreateWorldSize();
            } else if (PointInRectInt(mx, my, createWorldButton)) {
                if (newWorldName[0] != '\0') {
                    CreateWorldFromMenu();
                }
            } else if (PointInRectInt(mx, my, createCancelButton)) {
                EnterWorldSelect();
            }
        } else if (g_state == STATE_OPTIONS || g_state == STATE_SETTINGS) {
            if (PointInRectInt(mx, my, optionsVideoButton)) {
                GuiV7_EnterVideoSettings();
            } else if (PointInRectInt(mx, my, optionsControlsButton)) {
                GuiV7_EnterControls();
            } else if (PointInRectInt(mx, my, optionsAchievementsButton)) {
                GuiV7_EnterAchievements();
            } else if (PointInRectInt(mx, my, optionsStatsButton)) {
                GuiV7_EnterStats();
            } else if (PointInRectInt(mx, my, optionsRenderDistanceButton)) {
                SetRenderDistanceFromMouse(mx);
            } else if (PointInRectInt(mx, my, optionsDoneButton)) {
                if (g_optionsReturnState == STATE_PAUSE) {
                    EnterPauseMenu();
                } else {
                    EnterMenu();
                }
            }
        } else if (g_state == STATE_PAUSE) {
            if (PointInRectInt(mx, my, pauseContinueButton)) {
                EnterGame();
            } else if (PointInRectInt(mx, my, pauseOptionsButton)) {
                EnterOptions();
            } else if (PointInRectInt(mx, my, pauseExitButton)) {
                SaveCurrentWorld();
                EnterMenu();
            }
        } else if (g_state == STATE_DEATH) {
            if (PointInRectInt(mx, my, deathRespawnButton)) {
                RespawnPlayerAtWorldSpawn();
            } else if (PointInRectInt(mx, my, deathTitleButton)) {
                SaveCurrentWorld();
                EnterMenu();
            }
        } else if (g_state == STATE_GAME) {
            if (inventoryOpen) {
                if (g_containerModeV5 != CONTAINER_NONE_V5) { TileContainerMouseClickV5(mx, my, 0); }
                else if (craftingOpen) { CraftingMouseClick(mx, my); } else { HandleInventoryClick(mx, my); }
            } else {
                BeginHeldBlockMiningV11();
            }
        }

        return 0;

    case WM_LBUTTONUP:
        CancelHeldBlockMiningV11();
        return 0;

    case WM_MOUSEWHEEL:
        if (GuiV9_HandleMouseWheel((short)HIWORD(wParam))) { return 0; }
        if (g_state == STATE_WORLD_SELECT) {
            g_guiV21WorldScrollMax = MAX_WORLD_SLOTS - GUIV21_WORLD_VISIBLE_SLOTS;
            if (g_guiV21WorldScrollMax < 0) { g_guiV21WorldScrollMax = 0; }
            if ((short)HIWORD(wParam) < 0) { g_guiV21WorldScroll++; } else { g_guiV21WorldScroll--; }
            if (g_guiV21WorldScroll < 0) { g_guiV21WorldScroll = 0; }
            if (g_guiV21WorldScroll > g_guiV21WorldScrollMax) { g_guiV21WorldScroll = g_guiV21WorldScrollMax; }
            LayoutBetaMenus();
            return 0;
        }
        return 0;

    case WM_RBUTTONDOWN:
        if (g_state == STATE_GAME) {
            if (inventoryOpen) {
                if (g_containerModeV5 != CONTAINER_NONE_V5) { TileContainerMouseClickV5(LOWORD(lParam), HIWORD(lParam), 1); }
                else if (craftingOpen) {
                    CraftingMouseRightClick(LOWORD(lParam), HIWORD(lParam));
                } else {
                    HandleInventoryRightClick(LOWORD(lParam), HIWORD(lParam));
                }
            } else {
                PlaceBlockRaycast();
            }
        }

        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/* ------------------------------------------------------------ */
/* State switching                                              */
/* ------------------------------------------------------------ */

void LayoutBetaMenus(void)
{
    int centerX;
    int startY;
    int i;
    int visibleSlot;

    centerX = g_windowWidth / 2;

    SetRectXYWH(&singleplayerButton, centerX - 150, 250, 300, 44);
    SetRectXYWH(&multiplayerButton,  -9999, -9999, 1, 1); /* V11: hidden/disabled */
    SetRectXYWH(&texturePackButton,  centerX - 150, 304, 300, 44);
    SetRectXYWH(&optionsButton,      centerX - 150, 366, 145, 44);
    SetRectXYWH(&quitButton,         centerX +   5, 366, 145, 44);

    startY = 150;
    g_guiV21WorldScrollMax = MAX_WORLD_SLOTS - GUIV21_WORLD_VISIBLE_SLOTS;
    if (g_guiV21WorldScrollMax < 0) { g_guiV21WorldScrollMax = 0; }
    if (g_guiV21WorldScroll < 0) { g_guiV21WorldScroll = 0; }
    if (g_guiV21WorldScroll > g_guiV21WorldScrollMax) { g_guiV21WorldScroll = g_guiV21WorldScrollMax; }
    for (i = 0; i < MAX_WORLD_SLOTS; i++) {
        SetRectXYWH(&worldSlotButtons[i], -9999, -9999, 1, 1);
    }
    for (i = 0; i < GUIV21_WORLD_VISIBLE_SLOTS; i++) {
        visibleSlot = g_guiV21WorldScroll + i;
        if (visibleSlot >= 0 && visibleSlot < MAX_WORLD_SLOTS) {
            SetRectXYWH(&worldSlotButtons[visibleSlot], centerX - 260, startY + i * 54, 520, 48);
        }
    }

    SetRectXYWH(&worldPlayButton,    centerX - 260, 455, 170, 42);
    SetRectXYWH(&worldCreateButton,  centerX -  85, 455, 170, 42);
    SetRectXYWH(&worldDeleteButton,  centerX +  90, 455, 170, 42);
    SetRectXYWH(&worldBackButton,    centerX -  85, 505, 170, 42);

    SetRectXYWH(&createNameField,    centerX - 210, 220, 420, 44);
    SetRectXYWH(&createSeedField,    centerX - 210, 300, 420, 44);
    SetRectXYWH(&createWorldSizeButton, centerX - 210, 370, 420, 42);
    SetRectXYWH(&createWorldButton,  centerX - 210, 435, 200, 42);
    SetRectXYWH(&createCancelButton, centerX +  10, 435, 200, 42);

    SetRectXYWH(&optionsRenderDistanceButton, centerX + 5, 263, 250, 38);
    SetRectXYWH(&optionsDoneButton,  centerX - 100, 430, 200, 42);

    SetRectXYWH(&pauseContinueButton, centerX - 150, 210, 300, 44);
    SetRectXYWH(&pauseOptionsButton,  centerX - 150, 264, 300, 44);
    SetRectXYWH(&pauseExitButton,     centerX - 150, 318, 300, 44);

    /* GuiGameOver-style button placement. */
    SetRectXYWH(&deathRespawnButton,  centerX - 100, g_windowHeight / 4 + 72, 200, 40);
    SetRectXYWH(&deathTitleButton,    centerX - 100, g_windowHeight / 4 + 120, 200, 40);

    /* GUI_LIGHT_V7: extra menu/screen rectangles. */
    SetRectXYWH(&worldRenameButton,  centerX - 260, 505, 170, 42);
    SetRectXYWH(&worldBackButton,    centerX -  85, 555, 170, 42);
    SetRectXYWH(&optionsVideoButton, centerX - 255, 215, 250, 38);
    SetRectXYWH(&optionsControlsButton, centerX + 5, 215, 250, 38);
    SetRectXYWH(&optionsAchievementsButton, centerX - 255, 312, 250, 38);
    SetRectXYWH(&optionsStatsButton, centerX + 5, 312, 250, 38);
    SetRectXYWH(&optionsMusicSlider, centerX - 255, 263, 250, 38);
    SetRectXYWH(&optionsSoundSlider, centerX + 5, 263, 250, 38);
    SetRectXYWH(&videoRenderDistanceButton, centerX - 255, 170, 250, 38);
    SetRectXYWH(&videoBrightnessButton, centerX + 5, 170, 250, 38);
    SetRectXYWH(&videoSmoothLightingButton, centerX - 255, 218, 250, 38);
    SetRectXYWH(&videoCloudsButton, centerX + 5, 218, 250, 38);
    SetRectXYWH(&videoParticlesButton, centerX - 255, 266, 250, 38);
    SetRectXYWH(&videoFogButton, centerX + 5, 266, 250, 38);
    SetRectXYWH(&videoFullscreenButton, centerX - 125, 314, 250, 38);
    SetRectXYWH(&videoDoneButton, centerX - 100, 430, 200, 42);
    for (i = 0; i < 8; i++) {
        SetRectXYWH(&controlsButtons[i], centerX - 170, 145 + i * 38, 340, 32);
    }
    SetRectXYWH(&controlsDoneButton, centerX - 100, 470, 200, 42);
    SetRectXYWH(&renameNameField, centerX - 210, 240, 420, 44);
    SetRectXYWH(&renameDoneButton, centerX - 210, 325, 200, 42);
    SetRectXYWH(&renameCancelButton, centerX + 10, 325, 200, 42);
    SetRectXYWH(&achievementsDoneButton, centerX - 100, g_windowHeight - 70, 200, 42);
    SetRectXYWH(&statsDoneButton, centerX - 100, g_windowHeight - 70, 200, 42);
    GuiV9_Layout();
}


void SetRectXYWH(RECT *r, int x, int y, int w, int h)
{
    r->left = x;
    r->top = y;
    r->right = x + w;
    r->bottom = y + h;
}

void EnterMenu(void)
{
    g_state = STATE_MENU;
    inventoryOpen = 0;

    ResetInputState();
    UnlockMouseFromGame();
    StartMenuMusic();
    LayoutBetaMenus();
}

void EnterWorldSelect(void)
{
    g_state = STATE_WORLD_SELECT;
    inventoryOpen = 0;

    ResetInputState();
    UnlockMouseFromGame();
    StartMenuMusic();
    LoadWorldList();
    LayoutBetaMenus();
}

void EnterCreateWorld(void)
{
    int i;

    g_state = STATE_CREATE_WORLD;
    createInputField = 0;

    strcpy(newWorldName, "New World");
    newWorldSeedText[0] = '\0';

    for (i = 0; i < MAX_WORLD_SLOTS; i++) {
        if (!worldSaves[i].exists) {
            selectedWorldSlot = i;
            break;
        }
    }

    ResetInputState();
    UnlockMouseFromGame();
    StartMenuMusic();
    LayoutBetaMenus();
}

void EnterOptions(void)
{
    g_optionsReturnState = g_state;
    g_state = STATE_OPTIONS;
    ResetInputState();
    UnlockMouseFromGame();
    StartMenuMusic();
    LayoutBetaMenus();
}


void EnterSettings(void)
{
    EnterOptions();
}

void EnterPauseMenu(void)
{
    g_state = STATE_PAUSE;
    inventoryOpen = 0;

    ResetInputState();
    UnlockMouseFromGame();
    LayoutBetaMenus();
}

void RespawnPlayerAtWorldSpawn(void)
{
    double sx;
    double sy;
    double sz;

    if (FindSafeSpawn(&sx, &sy, &sz)) {
        playerX = sx;
        playerY = sy;
        playerZ = sz;
    } else {
        playerX = WORLD_X / 2;
        playerZ = WORLD_Z / 2;
        playerY = 36.0;
        ForceSpawnPad((int)playerX, (int)playerY, (int)playerZ);
    }

    playerHealth = MAX_HEALTH;
    playerPrevHealth = MAX_HEALTH;
    playerHeartsLife = 20.0;
    damageCooldown = 1.0;
    playerHurtFlash = 0.0;
    g_damageWobbleTimer = 0.0;
    velocityX = 0.0;
    velocityY = 0.0;
    velocityZ = 0.0;
    lastVelocityY = 0.0;
    g_playerFallDistanceV22 = 0.0;
    onGround = 1;
    g_playerAirTimer = 12.0;
    g_drownDamageTimer = 1.0;
    inventoryOpen = 0;
    craftingOpen = 0;
    ReturnCraftingGridToInventory();
    if (g_draggingInventory) { DropCarriedInventoryStackToWorld(0); }
    g_draggingInventory = 0;

    if (currentWorldSlot >= 0) { SaveCurrentWorld(); }
    EnterGame();
}

void EnterDeathScreen(void)
{
    g_state = STATE_DEATH;
    inventoryOpen = 0;
    craftingOpen = 0;
    ReturnCraftingGridToInventory();
    if (g_draggingInventory) { DropCarriedInventoryStackToWorld(0); }
    g_draggingInventory = 0;
    ResetInputState();
    UnlockMouseFromGame();
    playerHealth = 0;
    playerPrevHealth = 0;
    playerHeartsLife = 20.0;
    damageCooldown = 0.0;
    playerHurtFlash = 0.0;
    velocityX = 0.0;
    velocityY = 0.0;
    velocityZ = 0.0;
    lastVelocityY = 0.0;
    g_playerFallDistanceV22 = 0.0;
    LayoutBetaMenus();
}

void EnterGame(void)
{
    g_state = STATE_GAME;
    inventoryOpen = 0;

    ResetInputState();
    LockMouseForGame();
    StartGameMusic();
}


/* ------------------------------------------------------------ */
/* Beta-style save/load and seed handling                       */
/* ------------------------------------------------------------ */

void EnsureSaveDirectory(void)
{
    CreateDirectory("saves", NULL);
}

void GetWorldSavePath(int slot, char *path)
{
    sprintf(path, "saves\\World%d.dat", slot + 1);
}

void GetMobSavePath(int slot, char *path)
{
    sprintf(path, "saves\\World%d_mobs.dat", slot + 1);
}

void GetInventorySavePath(int slot, char *path)
{
    sprintf(path, "saves\\World%d_inventory.dat", slot + 1);
}

void GetDroppedItemsSavePath(int slot, char *path)
{
    sprintf(path, "saves\\World%d_drops.dat", slot + 1);
}

void GetBlockSavePath(int slot, char *path)
{
    sprintf(path, "saves\\World%d_blocks.rle", slot + 1);
}

void SanitizeWorldName(char *name)
{
    int i;

    if (!name || name[0] == '\0') {
        strcpy(name, "New World");
        return;
    }

    for (i = 0; name[i] != '\0'; i++) {
        if (name[i] == '\\' || name[i] == '/' || name[i] == ':' ||
            name[i] == '*' || name[i] == '?' || name[i] == '"' ||
            name[i] == '<' || name[i] == '>' || name[i] == '|') {
            name[i] = '_';
        }
    }
}

int SeedFromText(const char *text)
{
    unsigned int hash;
    int i;
    int sign;
    int value;
    int hasDigit;

    if (!text || text[0] == '\0') {
        return (int)(GetTickCount() ^ 0x5F3759DFu);
    }

    sign = 1;
    i = 0;
    value = 0;
    hasDigit = 0;

    if (text[0] == '-') {
        sign = -1;
        i = 1;
    }

    for (; text[i] != '\0'; i++) {
        if (text[i] < '0' || text[i] > '9') {
            hasDigit = 0;
            break;
        }

        hasDigit = 1;
        value = value * 10 + (text[i] - '0');
    }

    if (hasDigit) {
        return value * sign;
    }

    hash = 2166136261u;

    for (i = 0; text[i] != '\0'; i++) {
        hash ^= (unsigned char)text[i];
        hash *= 16777619u;
    }

    return (int)(hash & 0x7fffffff);
}

int LoadWorldSlot(int slot, WorldSaveInfo *info)
{
    FILE *f;
    char path[128];
    char line[256];
    char *p;

    if (!info) {
        return 0;
    }

    ZeroMemory(info, sizeof(WorldSaveInfo));
    info->seed = DEFAULT_WORLDGEN_SEED;
    info->worldSize = WORLD_SIZE_INFINITE;
    strcpy(info->seedText, "173773");
    info->playerGlobalX = 0.5;
    info->playerY = 80.0;
    info->playerGlobalZ = 0.5;
    info->worldTime = 300.0;

    /* V34: prefer the Java-style level.dat CNBT world metadata.
       The old text file remains as a fallback for pre-V34 saves. */
    if (SaveHandler_LoadWorldInfoV2(slot, info)) {
        return 1;
    }

    GetWorldSavePath(slot, path);
    f = fopen(path, "r");

    if (!f) {
        return 0;
    }

    while (fgets(line, sizeof(line), f)) {
        p = strchr(line, '\n');
        if (p) {
            *p = '\0';
        }

        if (strncmp(line, "name=", 5) == 0) {
            strncpy(info->name, line + 5, WORLD_NAME_LEN - 1);
            info->name[WORLD_NAME_LEN - 1] = '\0';
        } else if (strncmp(line, "seedText=", 9) == 0) {
            strncpy(info->seedText, line + 9, WORLD_SEED_LEN - 1);
            info->seedText[WORLD_SEED_LEN - 1] = '\0';
        } else if (strncmp(line, "seed=", 5) == 0) {
            info->seed = atoi(line + 5);
        } else if (strncmp(line, "worldSize=", 10) == 0) {
            info->worldSize = atoi(line + 10);
            if (info->worldSize < 0) { info->worldSize = WORLD_SIZE_INFINITE; }
            if (info->worldSize > 0 && info->worldSize < FINITE_WORLD_SIZE_SMALL) { info->worldSize = FINITE_WORLD_SIZE_SMALL; }
        } else if (strncmp(line, "playerGlobalX=", 14) == 0) {
            info->playerGlobalX = atof(line + 14);
        } else if (strncmp(line, "playerY=", 8) == 0) {
            info->playerY = atof(line + 8);
        } else if (strncmp(line, "playerGlobalZ=", 14) == 0) {
            info->playerGlobalZ = atof(line + 14);
        } else if (strncmp(line, "worldTime=", 10) == 0) {
            info->worldTime = atof(line + 10);
        }
    }

    fclose(f);

    if (info->name[0] == '\0') {
        sprintf(info->name, "World %d", slot + 1);
    }

    if (info->seedText[0] == '\0') {
        sprintf(info->seedText, "%d", info->seed);
    }

    info->exists = 1;
    return 1;
}

void LoadWorldList(void)
{
    int i;

    EnsureSaveDirectory();

    for (i = 0; i < MAX_WORLD_SLOTS; i++) {
        LoadWorldSlot(i, &worldSaves[i]);
    }

    if (selectedWorldSlot < 0) {
        selectedWorldSlot = 0;
    }

    if (selectedWorldSlot >= MAX_WORLD_SLOTS) {
        selectedWorldSlot = MAX_WORLD_SLOTS - 1;
    }
}

void SaveWorldSlotInfo(int slot)
{
    FILE *f;
    char path[128];
    WorldSaveInfo *info;

    if (slot < 0 || slot >= MAX_WORLD_SLOTS) {
        return;
    }

    info = &worldSaves[slot];

    if (!info->exists) {
        return;
    }

    EnsureSaveDirectory();
    GetWorldSavePath(slot, path);

    f = fopen(path, "w");

    if (!f) {
        return;
    }

    fprintf(f, "name=%s\n", info->name);
    fprintf(f, "seedText=%s\n", info->seedText);
    fprintf(f, "seed=%d\n", info->seed);
    fprintf(f, "worldSize=%d\n", info->worldSize);
    fprintf(f, "playerGlobalX=%.3f\n", info->playerGlobalX);
    fprintf(f, "playerY=%.3f\n", info->playerY);
    fprintf(f, "playerGlobalZ=%.3f\n", info->playerGlobalZ);
    fprintf(f, "worldTime=%.3f\n", info->worldTime);

    fclose(f);
    SaveHandler_SaveWorldInfoV2(slot);
}


void SaveCurrentMobs(void)
{
    FILE *f;
    char path[128];
    int i;
    double gx;
    double gz;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) {
        return;
    }

    EnsureSaveDirectory();
    GetMobSavePath(currentWorldSlot, path);
    f = fopen(path, "w");

    if (!f) {
        return;
    }

    fprintf(f, "# CloneMC mob save: type globalX y globalZ health angry sheared fuseTimer\n");

    for (i = 0; i < MAX_MOBS; i++) {
        if (!mobs[i].active) {
            continue;
        }

        gx = (double)worldOriginBlockX + mobs[i].x;
        gz = (double)worldOriginBlockZ + mobs[i].z;

        fprintf(f, "%d %.3f %.3f %.3f %d %d %d %.3f\n",
                mobs[i].type,
                gx,
                mobs[i].y,
                gz,
                mobs[i].health,
                mobs[i].angry,
                mobs[i].sheared,
                mobs[i].fuseTimer);
    }

    fclose(f);
}

int LoadCurrentMobs(void)
{
    FILE *f;
    char path[128];
    char line[256];
    int type;
    int health;
    int angry;
    int sheared;
    int idx;
    double gx;
    double gy;
    double gz;
    double fuse;
    double lx;
    double lz;
    int loaded;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) {
        return 0;
    }

    GetMobSavePath(currentWorldSlot, path);
    f = fopen(path, "r");

    if (!f) {
        return 0;
    }

    InitMobs();
    loaded = 0;

    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#') {
            continue;
        }

        if (sscanf(line, "%d %lf %lf %lf %d %d %d %lf",
                   &type, &gx, &gy, &gz, &health, &angry, &sheared, &fuse) != 8) {
            continue;
        }

        lx = gx - (double)worldOriginBlockX;
        lz = gz - (double)worldOriginBlockZ;

        if (lx < 1.0 || lz < 1.0 || lx > (double)(WORLD_X - 2) || lz > (double)(WORLD_Z - 2)) {
            continue;
        }

        idx = AddMob(type, lx, gy, lz);

        if (idx >= 0) {
            mobs[idx].health = health;
            mobs[idx].angry = angry;
            mobs[idx].sheared = sheared;
            mobs[idx].fuseTimer = fuse;
            mobs[idx].spawnGraceTimer = 0.5;
            loaded++;
        }
    }

    fclose(f);
    return loaded > 0;
}


/* ------------------------------------------------------------ */
/* Advanced Beta-style save components                           */
/* ------------------------------------------------------------ */

void SaveCurrentInventory(void)
{
    FILE *f;
    char path[128];
    int i;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    GetInventorySavePath(currentWorldSlot, path);
    f = fopen(path, "w");
    if (!f) { return; }

    fprintf(f, "CLONEMC_INVENTORY_V3\n");
    fprintf(f, "health %d\n", playerHealth);
    fprintf(f, "prevHealth %d\n", playerPrevHealth);
    fprintf(f, "heartsLife %.3f\n", playerHeartsLife);
    fprintf(f, "selected %d\n", selectedHotbarSlot);
    fprintf(f, "yaw %.6f\n", yaw);
    fprintf(f, "pitch %.6f\n", pitch);
    fprintf(f, "air %.3f\n", g_playerAirTimer);
    fprintf(f, "hotbar %d\n", HOTBAR_SLOTS);
    for (i = 0; i < HOTBAR_SLOTS; i++) {
        fprintf(f, "%d %d %d\n", hotbar[i].item, hotbar[i].count, hotbar[i].damage);
    }
    fprintf(f, "inventory %d\n", INVENTORY_SLOTS);
    for (i = 0; i < INVENTORY_SLOTS; i++) {
        fprintf(f, "%d %d %d\n", inventory[i].item, inventory[i].count, inventory[i].damage);
    }

    fclose(f);
}

int LoadCurrentInventory(void)
{
    FILE *f;
    char path[128];
    char key[64];
    int i;
    int count;
    int item;
    int stack;
    int damage;
    int invVersion;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    GetInventorySavePath(currentWorldSlot, path);
    f = fopen(path, "r");
    if (!f) { return 0; }

    if (fscanf(f, "%63s", key) != 1) { fclose(f); return 0; }
    invVersion = 2;
    if (strcmp(key, "CLONEMC_INVENTORY_V3") == 0) { invVersion = 3; }
    else if (strcmp(key, "CLONEMC_INVENTORY_V2") != 0) { fclose(f); return 0; }

    while (fscanf(f, "%63s", key) == 1) {
        if (strcmp(key, "health") == 0) {
            fscanf(f, "%d", &playerHealth);
            if (playerHealth < 0) { playerHealth = 0; }
            if (playerHealth > MAX_HEALTH) { playerHealth = MAX_HEALTH; }
        } else if (strcmp(key, "prevHealth") == 0) {
            fscanf(f, "%d", &playerPrevHealth);
        } else if (strcmp(key, "heartsLife") == 0) {
            fscanf(f, "%lf", &playerHeartsLife);
        } else if (strcmp(key, "selected") == 0) {
            fscanf(f, "%d", &selectedHotbarSlot);
            if (selectedHotbarSlot < 0) { selectedHotbarSlot = 0; }
            if (selectedHotbarSlot >= HOTBAR_SLOTS) { selectedHotbarSlot = HOTBAR_SLOTS - 1; }
        } else if (strcmp(key, "yaw") == 0) {
            fscanf(f, "%lf", &yaw);
        } else if (strcmp(key, "pitch") == 0) {
            fscanf(f, "%lf", &pitch);
        } else if (strcmp(key, "air") == 0) {
            fscanf(f, "%lf", &g_playerAirTimer);
        } else if (strcmp(key, "hotbar") == 0) {
            fscanf(f, "%d", &count);
            for (i = 0; i < HOTBAR_SLOTS && i < count; i++) {
                damage = 0;
                if (invVersion >= 3) {
                    if (fscanf(f, "%d %d %d", &item, &stack, &damage) != 3) { break; }
                } else {
                    if (fscanf(f, "%d %d", &item, &stack) != 2) { break; }
                }
                hotbar[i].item = item;
                hotbar[i].count = stack;
                hotbar[i].damage = damage;
                if (hotbar[i].count <= 0) { hotbar[i].item = ITEM_NONE; hotbar[i].count = 0; }
                if (hotbar[i].count > MAX_STACK) { hotbar[i].count = MAX_STACK; }
            }
            for (; i < count; i++) { if (invVersion >= 3) { fscanf(f, "%d %d %d", &item, &stack, &damage); } else { fscanf(f, "%d %d", &item, &stack); } }
        } else if (strcmp(key, "inventory") == 0) {
            fscanf(f, "%d", &count);
            for (i = 0; i < INVENTORY_SLOTS && i < count; i++) {
                damage = 0;
                if (invVersion >= 3) {
                    if (fscanf(f, "%d %d %d", &item, &stack, &damage) != 3) { break; }
                } else {
                    if (fscanf(f, "%d %d", &item, &stack) != 2) { break; }
                }
                inventory[i].item = item;
                inventory[i].count = stack;
                inventory[i].damage = damage;
                if (inventory[i].count <= 0) { inventory[i].item = ITEM_NONE; inventory[i].count = 0; }
                if (inventory[i].count > MAX_STACK) { inventory[i].count = MAX_STACK; }
            }
            for (; i < count; i++) { if (invVersion >= 3) { fscanf(f, "%d %d %d", &item, &stack, &damage); } else { fscanf(f, "%d %d", &item, &stack); } }
        }
    }

    fclose(f);
    return 1;
}

void SaveCurrentDroppedItems(void)
{
    FILE *f;
    char path[128];
    int i;
    double gx;
    double gz;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    GetDroppedItemsSavePath(currentWorldSlot, path);
    f = fopen(path, "w");
    if (!f) { return; }

    fprintf(f, "CLONEMC_DROPS_V3\n");
    for (i = 0; i < MAX_DROPPED_ITEMS; i++) {
        if (!droppedItems[i].active) { continue; }
        gx = (double)worldOriginBlockX + droppedItems[i].x;
        gz = (double)worldOriginBlockZ + droppedItems[i].z;
        fprintf(f, "%d %d %d %.4f %.4f %.4f %.5f %.5f %.5f %.3f %.3f %.5f\n",
                droppedItems[i].item, droppedItems[i].count, droppedItems[i].damage,
                gx, droppedItems[i].y, gz,
                droppedItems[i].vx, droppedItems[i].vy, droppedItems[i].vz,
                droppedItems[i].age, droppedItems[i].spin, droppedItems[i].hoverStart);
    }
    fclose(f);
}

int LoadCurrentDroppedItems(void)
{
    FILE *f;
    char path[128];
    char magic[64];
    char line[256];
    int item;
    int count;
    int damage;
    int loaded;
    double gx;
    double gy;
    double gz;
    double vx;
    double vy;
    double vz;
    double age;
    double spin;
    double hover;
    int idx;
    int n;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    GetDroppedItemsSavePath(currentWorldSlot, path);
    f = fopen(path, "r");
    if (!f) { return 0; }
    if (fscanf(f, "%63s", magic) != 1 || (strcmp(magic, "CLONEMC_DROPS_V2") != 0 && strcmp(magic, "CLONEMC_DROPS_V3") != 0)) {
        fclose(f); return 0;
    }
    fgets(line, sizeof(line), f);

    InitDroppedItems();
    loaded = 0;
    while (fgets(line, sizeof(line), f)) {
        damage = 0;
        hover = 0.0;
        if (strcmp(magic, "CLONEMC_DROPS_V3") == 0) {
            n = sscanf(line, "%d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                       &item, &count, &damage, &gx, &gy, &gz, &vx, &vy, &vz, &age, &spin, &hover);
            if (n < 11) { continue; }
        } else {
            n = sscanf(line, "%d %d %lf %lf %lf %lf %lf %lf %lf %lf",
                       &item, &count, &gx, &gy, &gz, &vx, &vy, &vz, &age, &spin);
            if (n < 10) { continue; }
        }
        idx = AddDroppedItemStackV38(item, count, damage, gx - (double)worldOriginBlockX, gy, gz - (double)worldOriginBlockZ, vx, vy, vz);
        if (idx >= 0) {
            droppedItems[idx].age = age;
            droppedItems[idx].spin = spin;
            droppedItems[idx].hoverStart = hover;
            if (hover == 0.0) { droppedItems[idx].hoverStart = spin / 57.29578; }
            droppedItems[idx].pickupDelay = 0.50;
            loaded++;
        }
    }
    fclose(f);
    return loaded > 0;
}


void SaveCurrentBlocks(void)
{
    FILE *f;
    char path[128];
    int x;
    int y;
    int z;
    int current;
    int last;
    int run;
    int first;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return; }
    EnsureSaveDirectory();
    GetBlockSavePath(currentWorldSlot, path);
    f = fopen(path, "w");
    if (!f) { return; }

    fprintf(f, "CLONEMC_BLOCKS_RLE_V2\n");
    fprintf(f, "origin %d %d size %d %d %d\n", worldOriginBlockX, worldOriginBlockZ, WORLD_X, WORLD_Y, WORLD_Z);

    first = 1;
    last = 0;
    run = 0;
    for (x = 0; x < WORLD_X; x++) {
        for (y = 0; y < WORLD_Y; y++) {
            for (z = 0; z < WORLD_Z; z++) {
                current = world[x][y][z];
                if (first) {
                    first = 0;
                    last = current;
                    run = 1;
                } else if (current == last && run < 32000) {
                    run++;
                } else {
                    fprintf(f, "%d %d\n", last, run);
                    last = current;
                    run = 1;
                }
            }
        }
    }
    if (!first) { fprintf(f, "%d %d\n", last, run); }
    fclose(f);
}

int LoadCurrentBlocks(void)
{
    FILE *f;
    char path[128];
    char magic[64];
    char key1[32];
    char key2[32];
    int savedOriginX;
    int savedOriginZ;
    int sx;
    int sy;
    int sz;
    int block;
    int run;
    int total;
    int n;
    int x;
    int y;
    int z;
    int index;

    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) { return 0; }
    GetBlockSavePath(currentWorldSlot, path);
    f = fopen(path, "r");
    if (!f) { return 0; }
    if (fscanf(f, "%63s", magic) != 1 || strcmp(magic, "CLONEMC_BLOCKS_RLE_V2") != 0) {
        fclose(f); return 0;
    }
    if (fscanf(f, "%31s %d %d %31s %d %d %d", key1, &savedOriginX, &savedOriginZ, key2, &sx, &sy, &sz) != 7) {
        fclose(f); return 0;
    }
    if (savedOriginX != worldOriginBlockX || savedOriginZ != worldOriginBlockZ ||
        sx != WORLD_X || sy != WORLD_Y || sz != WORLD_Z) {
        fclose(f); return 0;
    }

    total = WORLD_X * WORLD_Y * WORLD_Z;
    index = 0;
    while (index < total && fscanf(f, "%d %d", &block, &run) == 2) {
        for (n = 0; n < run && index < total; n++, index++) {
            z = index % WORLD_Z;
            y = (index / WORLD_Z) % WORLD_Y;
            x = index / (WORLD_Z * WORLD_Y);
            world[x][y][z] = block;
        }
    }

    fclose(f);
    RebuildColumnTops();
    InvalidateAllTerrainChunkMeshes();
    return index == total;
}

void SaveCurrentWorld(void)
{
    if (currentWorldSlot < 0 || currentWorldSlot >= MAX_WORLD_SLOTS) {
        return;
    }

    if (!worldSaves[currentWorldSlot].exists) {
        return;
    }

    worldSaves[currentWorldSlot].seed = g_worldSeed;
    worldSaves[currentWorldSlot].worldSize = g_worldSizeBlocks;
    worldSaves[currentWorldSlot].playerGlobalX = GetPlayerGlobalX();
    worldSaves[currentWorldSlot].playerY = playerY;
    worldSaves[currentWorldSlot].playerGlobalZ = GetPlayerGlobalZ();
    worldSaves[currentWorldSlot].worldTime = g_worldTimeSeconds;

    SaveHandler_SaveCurrentWorldV2();

    SaveWorldSlotInfo(currentWorldSlot);
    SaveCurrentInventory();
    SaveCurrentDroppedItems();
    SaveCurrentBlocks();
    SaveCurrentMobs();
    SaveCurrentTileEntities();
    SaveRedstoneMetaV5();
    SaveCurrentRegionLite();
}

void DeleteWorldSlot(int slot)
{
    char path[128];

    if (slot < 0 || slot >= MAX_WORLD_SLOTS) {
        return;
    }

    GetWorldSavePath(slot, path);
    remove(path);
    GetMobSavePath(slot, path);
    remove(path);
    GetInventorySavePath(slot, path);
    remove(path);
    GetDroppedItemsSavePath(slot, path);
    remove(path);
    GetBlockSavePath(slot, path);
    remove(path);
    SaveHandler_DeleteWorldV2(slot);
    wsprintf(path, "saves\\world%d_redstone_v5.dat", slot);
    remove(path);

    ZeroMemory(&worldSaves[slot], sizeof(WorldSaveInfo));

    if (currentWorldSlot == slot) {
        currentWorldSlot = -1;
    }

    LoadWorldList();
}

void StartNewWorldInSlot(int slot)
{
    WorldSaveInfo *info;

    if (slot < 0 || slot >= MAX_WORLD_SLOTS) {
        return;
    }

    info = &worldSaves[slot];
    ZeroMemory(info, sizeof(WorldSaveInfo));

    strncpy(info->name, newWorldName, WORLD_NAME_LEN - 1);
    info->name[WORLD_NAME_LEN - 1] = '\0';
    SanitizeWorldName(info->name);

    if (newWorldSeedText[0] == '\0') {
        sprintf(info->seedText, "%lu", (unsigned long)GetTickCount());
    } else {
        strncpy(info->seedText, newWorldSeedText, WORLD_SEED_LEN - 1);
        info->seedText[WORLD_SEED_LEN - 1] = '\0';
    }

    info->seed = SeedFromText(info->seedText);
    info->worldSize = g_createWorldSizeBlocks;
    info->playerGlobalX = 0.5;
    info->playerY = 80.0;
    info->playerGlobalZ = 0.5;
    info->worldTime = 300.0;
    info->exists = 1;

    selectedWorldSlot = slot;
    currentWorldSlot = slot;
    SaveHandler_DeleteWorldV2(slot);
    SaveHandler_CreateWorldLayout(slot);
    g_worldSeed = info->seed;
    g_worldSizeBlocks = info->worldSize;
    g_worldAutosaveTimer = 0.0;
    g_startFromSavedPosition = 0;

    SaveWorldSlotInfo(slot);
    DrawBuildingTerrainScreen("Building terrain");
    {
        char mobPath[128];
        GetMobSavePath(slot, mobPath);
        remove(mobPath);
        GetInventorySavePath(slot, mobPath);
        remove(mobPath);
        GetDroppedItemsSavePath(slot, mobPath);
        remove(mobPath);
        GetBlockSavePath(slot, mobPath);
        remove(mobPath);
    }

    GameInit();
    EnterGame();
}

void CreateWorldFromMenu(void)
{
    int slot;
    int i;

    slot = selectedWorldSlot;

    if (slot < 0 || slot >= MAX_WORLD_SLOTS || worldSaves[slot].exists) {
        slot = -1;

        for (i = 0; i < MAX_WORLD_SLOTS; i++) {
            if (!worldSaves[i].exists) {
                slot = i;
                break;
            }
        }
    }

    if (slot < 0) {
        MessageBox(g_hwnd, "Delete a world first. All five save slots are full.", "No Empty Save Slot", MB_OK);
        return;
    }

    StartNewWorldInSlot(slot);
}

void StartWorldSlot(int slot)
{
    WorldSaveInfo info;

    if (slot < 0 || slot >= MAX_WORLD_SLOTS) {
        return;
    }

    if (!LoadWorldSlot(slot, &info)) {
        return;
    }

    worldSaves[slot] = info;
    selectedWorldSlot = slot;
    currentWorldSlot = slot;

    g_worldSeed = info.seed;
    g_worldSizeBlocks = info.worldSize;
    g_worldAutosaveTimer = 0.0;
    g_startFromSavedPosition = 1;
    g_startGlobalX = info.playerGlobalX;
    g_startPlayerY = info.playerY;
    g_startGlobalZ = info.playerGlobalZ;
    g_worldTimeSeconds = info.worldTime;

    DrawBuildingTerrainScreen("Loading world");
    GameInit();
    EnterGame();
}

void HandleCreateWorldChar(WPARAM ch)
{
    char *target;
    int maxLen;
    int len;

    if (createInputField == 0) {
        target = newWorldName;
        maxLen = WORLD_NAME_LEN;
    } else {
        target = newWorldSeedText;
        maxLen = WORLD_SEED_LEN;
    }

    len = (int)strlen(target);

    if (ch == 8) {
        if (len > 0) {
            target[len - 1] = '\0';
        }

        return;
    }

    if (ch == 13) {
        CreateWorldFromMenu();
        return;
    }

    if (ch < 32 || ch > 126) {
        return;
    }

    if (len >= maxLen - 1) {
        return;
    }

    target[len] = (char)ch;
    target[len + 1] = '\0';
}

/* ------------------------------------------------------------ */
/* Game initialization                                          */
/* ------------------------------------------------------------ */

void GameInit(void)
{
    double sx;
    double sy;
    double sz;
    int loadingExisting;

    loadingExisting = g_startFromSavedPosition;

    GenerateWorld();

    if (loadingExisting) {
        if (!SaveHandler_LoadRegionWindowV2()) {
            if (!LoadCurrentRegionLite()) {
                LoadCurrentBlocks();
            }
        }
    }

    /* V12: clear removed plant/flower/mushroom blocks even after loading old saves. */
    RemovePlantsFromWorldV12();
    RebuildColumnTops();
    InvalidateAllTerrainChunkMeshes();

    /*
        Compute skylight and block light after world generation.
    */
    ComputeLegacyLighting();

    if (g_startFromSavedPosition) {
        playerX = g_startGlobalX - (double)worldOriginBlockX;
        playerY = g_startPlayerY;
        playerZ = g_startGlobalZ - (double)worldOriginBlockZ;

        if (!IsSpawnSpaceClear(playerX, playerY, playerZ)) {
            if (FindSafeSpawn(&sx, &sy, &sz)) {
                playerX = sx;
                playerY = sy;
                playerZ = sz;
            } else {
                playerX = WORLD_X / 2;
                playerZ = WORLD_Z / 2;
                playerY = WORLD_Y - 8;
            }
        }

        g_startFromSavedPosition = 0;
    } else {
        /*
            Find a real safe spawn instead of blindly using terrain height.
            This prevents spawning inside cliffs, under overhangs, or in water.
        */
        if (FindSafeSpawn(&sx, &sy, &sz)) {
            playerX = sx;
            playerY = sy;
            playerZ = sz;
        } else {
            /*
                Last-resort fallback: high in the air at world center.
            */
            playerX = WORLD_X / 2;
            playerZ = WORLD_Z / 2;
            playerY = WORLD_Y - 8;
        }
    }

    velocityY = 0.0;
    lastVelocityY = 0.0;

    if (!loadingExisting) {
        yaw = 0.0;
        pitch = 0.0;
    }

    g_playerLastAnimX = playerX;
    g_playerLastAnimZ = playerZ;
    handBob = 0.0;

    InitSurvival();
    if (loadingExisting) {
        if (!SaveHandler_LoadPlayerV2()) {
            LoadCurrentInventory();
        }
    }
    InitDroppedItems();
    InitParticles();
    InitMobs();
    InitMobProjectiles();
    InitFeatureGapSystems();
    if (loadingExisting) {
        if (!SaveHandler_LoadEntitiesV2()) {
            LoadCurrentDroppedItems();
            LoadCurrentMobs();
        }
        if (!g_regionTilesLoadedV34) {
            if (!SaveHandler_LoadTileEntitiesV2()) {
                LoadCurrentTileEntities();
            }
        }
        LoadRedstoneMetaV5();
    } else {
        SpawnInitialMobs();
    }
}



/* ------------------------------------------------------------ */
/* Game update                                                  */
/* ------------------------------------------------------------ */

void GameUpdate(double dt)
{
    int mobStepsV32;

    UpdateDayNightCycle(dt);
    UpdateClouds(dt);
    UpdateParticles(dt);
    UpdateDroppedItems(dt);
    ProcessScheduledBlockTicksV52(dt, SCHEDULED_TICK_V52_FRAME_BUDGET);
    UpdateMobProjectiles(dt);
    UpdateItemCombatV6(dt);
    UpdateHeldBlockMiningV11(dt);

    /* V32_MOB_SMOOTH: fixed 20 Hz mob logic plus render interpolation.
       The V31 threshold was 0.15 seconds, despite the comment saying 20 Hz,
       so mobs visibly jumped at about 6-7 updates/second. */
    g_mobUpdateAccumulator += dt;
    mobStepsV32 = 0;
    while (g_mobUpdateAccumulator >= MOB_LOGIC_STEP_SECONDS &&
           mobStepsV32 < MOB_LOGIC_MAX_STEPS_PER_FRAME) {
        UpdateMobs(MOB_LOGIC_STEP_SECONDS);
        g_mobUpdateAccumulator -= MOB_LOGIC_STEP_SECONDS;
        mobStepsV32++;
    }
    if (mobStepsV32 >= MOB_LOGIC_MAX_STEPS_PER_FRAME &&
        g_mobUpdateAccumulator > MOB_LOGIC_STEP_SECONDS) {
        g_mobUpdateAccumulator = MOB_LOGIC_STEP_SECONDS;
    }
    g_mobRenderPartialTicks = g_mobUpdateAccumulator / MOB_LOGIC_STEP_SECONDS;
    if (g_mobRenderPartialTicks < 0.0) { g_mobRenderPartialTicks = 0.0; }
    if (g_mobRenderPartialTicks > 1.0) { g_mobRenderPartialTicks = 1.0; }

    UpdatePlayerHandAnimation(dt);
    g_skyLightSubtractedV48 = (int)((1.0f - g_daySkyBrightness) * (float)LIGHT_V48_DAY_SUBTRACT_MAX);
    if (g_skyLightSubtractedV48 < 0) { g_skyLightSubtractedV48 = 0; }
    if (g_skyLightSubtractedV48 > LIGHT_V48_DAY_SUBTRACT_MAX) { g_skyLightSubtractedV48 = LIGHT_V48_DAY_SUBTRACT_MAX; }
    ProcessLightUpdatesV42(FLUID_V42_LIGHT_BUDGET + 4);

    if (g_damageWobbleTimer > 0.0) {
        g_damageWobbleTimer -= dt;
        if (g_damageWobbleTimer < 0.0) {
            g_damageWobbleTimer = 0.0;
        }
    }

    /* Update invulnerability and hurt flash timers. */
    if (damageCooldown > 0.0) {
        damageCooldown -= dt;
        if (damageCooldown < 0.0) {
            damageCooldown = 0.0;
        }
    }

    if (playerHurtFlash > 0.0) {
        playerHurtFlash -= dt;
        if (playerHurtFlash < 0.0) {
            playerHurtFlash = 0.0;
        }
    }

    if (playerHeartsLife > 0.0) {
        playerHeartsLife -= dt * 20.0;
        if (playerHeartsLife < 0.0) { playerHeartsLife = 0.0; }
    }
    g_healthHudCounter++;

    if (!inventoryOpen) {
        UpdateMouseLook();
        PlayerV39_HandleMovementInput(dt);
    } else {
        PlayerV39_ApplyNoInputFriction(dt);
    }

    /* V39_EXACT_ENTITY_MOVEMENT:
       Use one Java-style Entity.moveEntity path for the player instead of the
       older direct WASD + vertical-only collision.  The new path resolves Y, X,
       and Z against AxisAlignedBB offsets, supports stepHeight, slipperiness,
       water/lava drag and pushes, fall distance, and consistent onGround flags. */
    ProcessScheduledBlockTicksV52(0.0, SCHEDULED_TICK_V52_FRAME_BUDGET_SMALL);
    TileEntitiesRedstoneV5_Update(dt);
    ProcessLightUpdatesV42(FLUID_V42_LIGHT_BUDGET);
    PlayerV39_UpdatePhysics(dt);

    ClampPlayerToFiniteWorld();

    UpdatePlayerMovementAnimation(dt);

    UpdateInfiniteWorldStreaming();

    if (currentWorldSlot >= 0) {
        g_worldAutosaveTimer += dt;
        if (g_worldAutosaveTimer >= 45.0) {
            g_worldAutosaveTimer = 0.0;
            SaveCurrentWorld();
        }
    }
}

/* ------------------------------------------------------------ */
/* Main renderer                                                */
/* ------------------------------------------------------------ */

void GameRender(void)
{
    glViewport(0, 0, g_windowWidth, g_windowHeight);

    ApplyDayNightClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_state == STATE_MENU) {
        DrawMenu();
    } else if (g_state == STATE_WORLD_SELECT) {
        DrawWorldSelect();
    } else if (g_state == STATE_CREATE_WORLD) {
        DrawCreateWorld();
    } else if (g_state == STATE_OPTIONS) {
        DrawOptions();
    } else if (g_state == STATE_SETTINGS) {
        DrawSettings();
    } else if (g_state == STATE_VIDEO_SETTINGS) {
        GuiV7_DrawVideoSettings();
    } else if (g_state == STATE_CONTROLS) {
        GuiV7_DrawControls();
    } else if (g_state == STATE_RENAME_WORLD) {
        GuiV7_DrawRenameWorld();
    } else if (g_state == STATE_ACHIEVEMENTS) {
        GuiV7_DrawAchievements();
    } else if (g_state == STATE_STATS) {
        GuiV7_DrawStats();
    } else if (g_state == STATE_MULTIPLAYER) {
        GuiV9_DrawMultiplayer();
    } else if (g_state == STATE_CONNECTING) {
        GuiV9_DrawConnecting();
    } else if (g_state == STATE_CONNECT_FAILED) {
        GuiV9_DrawConnectFailed();
    } else if (g_state == STATE_DOWNLOAD_TERRAIN) {
        GuiV9_DrawDownloadTerrain();
    } else if (g_state == STATE_SLEEP_MP) {
        GuiV9_DrawSleepMP();
    } else if (g_state == STATE_TEXTURE_PACKS) {
        GuiV9_DrawTexturePacks();
    } else if (g_state == STATE_CONFLICT_WARNING) {
        GuiV9_DrawConflictWarning();
    } else if (g_state == STATE_ERROR_SCREEN) {
        GuiV9_DrawErrorScreen();
    } else if (g_state == STATE_YESNO) {
        GuiV9_DrawYesNo();
    } else if (g_state == STATE_GAME || g_state == STATE_PAUSE || g_state == STATE_DEATH) {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(
            70.0,
            (double)g_windowWidth / (double)g_windowHeight,
            0.05,
            100.0
        );

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        SetupCamera();
        RenderSkyBodies();
        if (g_videoFogV7) { EnableBetaFog(); }
        RendererV8_ResetStats();
        RenderWorld();
        RendererV8_RenderSpecialWorldBlocks();
        RendererV8_RenderTransparentBlocks();
        RendererV8_PostWorldEffects();
        RenderDroppedItems();
        RendererV8_RenderMinecarts();
        RendererV8_RenderMobProjectiles();
        RendererV8_RenderSpecialEntities();
        RenderMobs();
        if (g_cameraMode != CAMERA_FIRST_PERSON) {
            DrawPlayerThirdPerson();
        }
        if (g_videoCloudsV7) { RenderClouds(); }
        if (g_videoParticlesV7) { RenderParticles(); }
        if (g_videoFogV7) { DisableBetaFog(); }
        if (g_cameraMode == CAMERA_FIRST_PERSON) {
            RendererV8_RenderFirstPersonItem();
        }

        if (g_state != STATE_DEATH) {
            DrawCrosshair();
            DrawSurvivalUI();
            DrawWeather2D();
            DrawBetaVignette2D();
            DrawBetaStatus2D();
            GuiV9_DrawChatOverlay();
        }

        if (g_state == STATE_PAUSE) {
            DrawPauseMenu();
        }
        if (g_state == STATE_DEATH) {
            DrawDeathScreen();
        }
    }

    GuiV16_DrawTransitionOverlay();
    SwapBuffers(g_hdc);
}



/* ------------------------------------------------------------ */
/* GUI_PIPELINE_V9: Java GUI behavior pass                      */
/* ------------------------------------------------------------ */

void GuiV9_UpdateScaledResolution(void)
{
    int factor;
    factor = 1;
    while (factor < 4 && g_windowWidth / (factor + 1) >= 320 && g_windowHeight / (factor + 1) >= 240) {
        factor++;
    }
    g_guiV9Scale.scaleFactor = factor;
    g_guiV9Scale.scaledWidth = (g_windowWidth + factor - 1) / factor;
    g_guiV9Scale.scaledHeight = (g_windowHeight + factor - 1) / factor;
    g_guiV9Scale.guiLeft = 0;
    g_guiV9Scale.guiTop = 0;
}

void GuiV9_LoadTranslations(void)
{
    FILE *f;
    char line[320];
    char *eq;
    char *p;
    int len;
    if (g_guiV9TranslationLoaded) { return; }
    g_guiV9TranslationLoaded = 1;
    g_guiV9TranslationCount = 0;
    f = fopen("assets\\lang\\en_US.lang", "r");
    if (!f) { f = fopen("assets/lang/en_US.lang", "r"); }
    if (!f) { return; }
    while (fgets(line, sizeof(line), f) && g_guiV9TranslationCount < GUIV9_MAX_TRANSLATIONS) {
        p = line;
        while (*p == ' ' || *p == '\t') { p++; }
        if (*p == '#' || *p == '\r' || *p == '\n' || *p == 0) { continue; }
        eq = strchr(p, '=');
        if (!eq) { continue; }
        *eq = 0;
        eq++;
        len = (int)strlen(eq);
        while (len > 0 && (eq[len - 1] == '\r' || eq[len - 1] == '\n')) { eq[len - 1] = 0; len--; }
        strncpy(g_guiV9Translations[g_guiV9TranslationCount].key, p, GUIV9_MAX_TRANSLATION_KEY - 1);
        g_guiV9Translations[g_guiV9TranslationCount].key[GUIV9_MAX_TRANSLATION_KEY - 1] = 0;
        strncpy(g_guiV9Translations[g_guiV9TranslationCount].value, eq, GUIV9_MAX_TRANSLATION_VALUE - 1);
        g_guiV9Translations[g_guiV9TranslationCount].value[GUIV9_MAX_TRANSLATION_VALUE - 1] = 0;
        g_guiV9TranslationCount++;
    }
    fclose(f);
}

const char *GuiV9_Tr(const char *key, const char *fallback)
{
    int i;
    GuiV9_LoadTranslations();
    for (i = 0; i < g_guiV9TranslationCount; i++) {
        if (strcmp(g_guiV9Translations[i].key, key) == 0) {
            return g_guiV9Translations[i].value;
        }
    }
    if (fallback) { return fallback; }
    return key;
}

int GuiV9_IsAllowedChatChar(int ch)
{
    if (ch >= 32 && ch <= 126) { return 1; }
    if (ch == '\t') { return 1; }
    return 0;
}

int GuiV9_FontWidth(const char *text)
{
    if (!text) { return 0; }
    return EstimateTextWidth(fontBaseNormal, text);
}

void GuiV9_DrawString(int x, int y, const char *text, float r, float g, float b)
{
    if (!text) { return; }
    glColor3f(r, g, b);
    DrawText2D(fontBaseNormal, x, y, text);
}

void GuiV9_DrawCenteredString(int x1, int y1, int x2, int y2, const char *text, float r, float g, float b)
{
    if (!text) { return; }
    glColor3f(r, g, b);
    DrawCenteredText2D(fontBaseNormal, x1, y1, x2, y2, text);
}

void GuiV9_DrawGradientRect(int x1, int y1, int x2, int y2, float r1, float g1, float b1, float a1, float r2, float g2, float b2, float a2)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor4f(r1, g1, b1, a1);
    glVertex2i(x1, y1);
    glVertex2i(x2, y1);
    glColor4f(r2, g2, b2, a2);
    glVertex2i(x2, y2);
    glVertex2i(x1, y2);
    glEnd();
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void GuiV9_TextFieldInit(GuiV9TextField *f, RECT r, const char *initialText, int maxLen)
{
    if (!f) { return; }
    f->rect = r;
    if (initialText) {
        strncpy(f->text, initialText, GUIV9_TEXT_LEN - 1);
        f->text[GUIV9_TEXT_LEN - 1] = 0;
    } else {
        f->text[0] = 0;
    }
    f->maxLen = maxLen;
    if (f->maxLen <= 0 || f->maxLen >= GUIV9_TEXT_LEN) { f->maxLen = GUIV9_TEXT_LEN - 1; }
    f->focused = 0;
    f->cursor = (int)strlen(f->text);
    f->selectStart = f->cursor;
    f->viewOffset = 0;
    f->enabled = 1;
}

void GuiV9_TextFieldDraw(GuiV9TextField *f, const char *label)
{
    int cursorX;
    char visible[GUIV9_TEXT_LEN];
    if (!f) { return; }
    if (label && label[0]) {
        GuiV9_DrawString(f->rect.left, f->rect.top - 18, label, 0.85f, 0.85f, 0.85f);
    }
    DrawRect2D(f->rect.left - 2, f->rect.top - 2, f->rect.right + 2, f->rect.bottom + 2, 0.02f, 0.02f, 0.02f);
    if (f->focused) {
        DrawRect2D(f->rect.left, f->rect.top, f->rect.right, f->rect.bottom, 0.22f, 0.22f, 0.22f);
    } else {
        DrawRect2D(f->rect.left, f->rect.top, f->rect.right, f->rect.bottom, 0.10f, 0.10f, 0.10f);
    }
    strncpy(visible, f->text + f->viewOffset, sizeof(visible) - 1);
    visible[sizeof(visible) - 1] = 0;
    GuiV9_DrawString(f->rect.left + 8, f->rect.top + 26, visible, 1.0f, 1.0f, 1.0f);
    if (f->focused && ((GetTickCount() / 350) & 1)) {
        int showCursor;
        char prefix[GUIV9_TEXT_LEN];
        showCursor = f->cursor - f->viewOffset;
        if (showCursor < 0) { showCursor = 0; }
        if (showCursor >= GUIV9_TEXT_LEN) { showCursor = GUIV9_TEXT_LEN - 1; }
        strncpy(prefix, visible, showCursor);
        prefix[showCursor] = 0;
        cursorX = f->rect.left + 8 + GuiV9_FontWidth(prefix);
        DrawRect2D(cursorX, f->rect.top + 6, cursorX + 2, f->rect.bottom - 6, 1.0f, 1.0f, 1.0f);
    }
}

void GuiV9_TextFieldEnsureCursor(GuiV9TextField *f)
{
    int len;
    if (!f) { return; }
    len = (int)strlen(f->text);
    if (f->cursor < 0) { f->cursor = 0; }
    if (f->cursor > len) { f->cursor = len; }
    if (f->cursor < f->viewOffset) { f->viewOffset = f->cursor; }
    while (f->cursor - f->viewOffset > 38) { f->viewOffset++; }
    if (f->viewOffset < 0) { f->viewOffset = 0; }
}

int GuiV9_TextFieldChar(GuiV9TextField *f, WPARAM ch)
{
    int len;
    int i;
    if (!f || !f->focused || !f->enabled) { return 0; }
    if (ch == 8) {
        len = (int)strlen(f->text);
        if (f->cursor > 0 && len > 0) {
            for (i = f->cursor - 1; i < len; i++) { f->text[i] = f->text[i + 1]; }
            f->cursor--;
            GuiV9_TextFieldEnsureCursor(f);
        }
        return 1;
    }
    if (ch == 13 || ch == 27 || ch == 9) { return 0; }
    if (!GuiV9_IsAllowedChatChar((int)ch)) { return 1; }
    len = (int)strlen(f->text);
    if (len >= f->maxLen) { return 1; }
    for (i = len; i >= f->cursor; i--) { f->text[i + 1] = f->text[i]; }
    f->text[f->cursor] = (char)ch;
    f->cursor++;
    GuiV9_TextFieldEnsureCursor(f);
    return 1;
}

int GuiV9_TextFieldKey(GuiV9TextField *f, WPARAM key)
{
    int len;
    int i;
    if (!f || !f->focused) { return 0; }
    len = (int)strlen(f->text);
    if (key == VK_LEFT) { f->cursor--; GuiV9_TextFieldEnsureCursor(f); return 1; }
    if (key == VK_RIGHT) { f->cursor++; GuiV9_TextFieldEnsureCursor(f); return 1; }
    if (key == VK_HOME) { f->cursor = 0; GuiV9_TextFieldEnsureCursor(f); return 1; }
    if (key == VK_END) { f->cursor = len; GuiV9_TextFieldEnsureCursor(f); return 1; }
    if (key == VK_DELETE) {
        if (f->cursor < len) {
            for (i = f->cursor; i < len; i++) { f->text[i] = f->text[i + 1]; }
        }
        return 1;
    }
    return 0;
}

void GuiV9_TextFieldMouse(GuiV9TextField *f, int mx, int my)
{
    int rel;
    int len;
    if (!f) { return; }
    f->focused = PointInRectInt(mx, my, f->rect);
    if (!f->focused) { return; }
    rel = (mx - f->rect.left - 8) / 8;
    if (rel < 0) { rel = 0; }
    len = (int)strlen(f->text);
    f->cursor = f->viewOffset + rel;
    if (f->cursor > len) { f->cursor = len; }
    f->selectStart = f->cursor;
    GuiV9_TextFieldEnsureCursor(f);
}

void GuiV9_AddChatLine(const char *text)
{
    int i;
    if (!text || !text[0]) { return; }
    for (i = GUIV9_MAX_CHAT_LINES - 1; i > 0; i--) {
        g_guiV9ChatLines[i] = g_guiV9ChatLines[i - 1];
    }
    strncpy(g_guiV9ChatLines[0].text, text, GUIV9_CHAT_TEXT_LEN - 1);
    g_guiV9ChatLines[0].text[GUIV9_CHAT_TEXT_LEN - 1] = 0;
    g_guiV9ChatLines[0].timeMs = GetTickCount();
    g_guiV9ChatLines[0].updateCounter = 0;
    if (g_guiV9ChatCount < GUIV9_MAX_CHAT_LINES) { g_guiV9ChatCount++; }
}

void GuiV9_RecordChatHistory(const char *line)
{
    int i;
    if (!line || !line[0]) { return; }
    for (i = GUIV9_MAX_HISTORY - 1; i > 0; i--) {
        strcpy(g_guiV9ChatHistory[i], g_guiV9ChatHistory[i - 1]);
    }
    strncpy(g_guiV9ChatHistory[0], line, GUIV9_CHAT_TEXT_LEN - 1);
    g_guiV9ChatHistory[0][GUIV9_CHAT_TEXT_LEN - 1] = 0;
    if (g_guiV9HistoryCount < GUIV9_MAX_HISTORY) { g_guiV9HistoryCount++; }
    g_guiV9HistoryCursor = -1;
}

void GuiV9_OpenChat(void)
{
    RECT r;
    if (g_state != STATE_GAME) { return; }
    SetRectXYWH(&r, 8, g_windowHeight - 38, g_windowWidth - 16, 30);
    GuiV9_TextFieldInit(&g_guiV9ChatField, r, "", GUIV9_CHAT_TEXT_LEN - 1);
    g_guiV9ChatField.focused = 1;
    g_guiV9ChatOpen = 1;
    UnlockMouseFromGame();
}

void GuiV9_CloseChat(void)
{
    g_guiV9ChatOpen = 0;
    g_guiV9ChatField.focused = 0;
    if (g_state == STATE_GAME && !inventoryOpen) { LockMouseToGame(); }
}

void GuiV9_RunChatCommand(const char *cmd)
{
    char line[GUIV9_CHAT_TEXT_LEN];
    if (!cmd) { return; }
    if (strcmp(cmd, "/help") == 0) {
        GuiV9_AddChatLine("Commands: /help /seed /pos /time day /time night /screenshot");
    } else if (strcmp(cmd, "/seed") == 0) {
        sprintf(line, "Seed: %lu", (unsigned long)g_worldSeed);
        GuiV9_AddChatLine(line);
    } else if (strcmp(cmd, "/pos") == 0) {
        sprintf(line, "Position: %.1f %.1f %.1f", (float)(worldOriginBlockX + playerX), (float)playerY, (float)(worldOriginBlockZ + playerZ));
        GuiV9_AddChatLine(line);
    } else if (strcmp(cmd, "/time day") == 0) {
        g_worldTimeSeconds = DAY_LENGTH_SECONDS * 0.25;
        GuiV9_AddChatLine("Time set to day.");
    } else if (strcmp(cmd, "/time night") == 0) {
        g_worldTimeSeconds = DAY_LENGTH_SECONDS * 0.80;
        GuiV9_AddChatLine("Time set to night.");
    } else if (strcmp(cmd, "/screenshot") == 0) {
        if (GuiV9_SaveScreenshot()) { GuiV9_AddChatLine("Saved screenshot."); }
        else { GuiV9_AddChatLine("Could not save screenshot."); }
    } else {
        sprintf(line, "Unknown command: %s", cmd);
        GuiV9_AddChatLine(line);
    }
}

void GuiV9_DrawChatOverlay(void)
{
    int i;
    int y;
    int show;
    unsigned long nowMs;
    RECT r;
    char inputLine[GUIV9_CHAT_TEXT_LEN + 4];
    if (g_state != STATE_GAME) { return; }
    Setup2D();
    nowMs = GetTickCount();
    show = g_guiV9ChatOpen ? 16 : 8;
    if (show > g_guiV9ChatCount) { show = g_guiV9ChatCount; }
    y = g_windowHeight - (g_guiV9ChatOpen ? 68 : 38);
    for (i = 0; i < show; i++) {
        if (!g_guiV9ChatOpen && nowMs - g_guiV9ChatLines[i].timeMs > 10000UL) { continue; }
        SetRectXYWH(&r, 6, y - 20, g_windowWidth / 2 + 260, 22);
        GuiV9_DrawGradientRect(r.left, r.top, r.right, r.bottom, 0.0f, 0.0f, 0.0f, 0.35f, 0.0f, 0.0f, 0.0f, 0.35f);
        GuiV9_DrawString(10, y, g_guiV9ChatLines[i].text, 1.0f, 1.0f, 1.0f);
        y -= 24;
    }
    if (g_guiV9ChatOpen) {
        SetRectXYWH(&g_guiV9ChatField.rect, 8, g_windowHeight - 38, g_windowWidth - 16, 30);
        sprintf(inputLine, "> %s", g_guiV9ChatField.text);
        GuiV9_DrawGradientRect(6, g_windowHeight - 42, g_windowWidth - 6, g_windowHeight - 6, 0.0f, 0.0f, 0.0f, 0.55f, 0.0f, 0.0f, 0.0f, 0.55f);
        GuiV9_DrawString(12, g_windowHeight - 18, inputLine, 1.0f, 1.0f, 1.0f);
        if ((GetTickCount() / 350) & 1) {
            DrawRect2D(24 + GuiV9_FontWidth(g_guiV9ChatField.text), g_windowHeight - 32, 26 + GuiV9_FontWidth(g_guiV9ChatField.text), g_windowHeight - 12, 1.0f, 1.0f, 1.0f);
        }
    }
}

void GuiV9_Layout(void)
{
    int centerX;
    centerX = g_windowWidth / 2;
    GuiV9_UpdateScaledResolution();
    SetRectXYWH(&guiV9BackButton, centerX - 100, g_windowHeight - 65, 200, 42);
    SetRectXYWH(&guiV9DoneButton, centerX - 100, g_windowHeight - 65, 200, 42);
    SetRectXYWH(&guiV9CancelButton, centerX - 100, g_windowHeight - 65, 200, 42);
    SetRectXYWH(&guiV9ConnectButton, centerX - 155, 295, 150, 40);
    SetRectXYWH(&guiV9DirectButton, centerX + 5, 295, 150, 40);
    SetRectXYWH(&guiV9RefreshButton, centerX - 155, 345, 150, 40);
    SetRectXYWH(&guiV9TextureOpenButton, centerX + 5, 345, 150, 40);
    SetRectXYWH(&guiV9YesButton, centerX - 155, g_windowHeight / 2 + 45, 150, 40);
    SetRectXYWH(&guiV9NoButton, centerX + 5, g_windowHeight / 2 + 45, 150, 40);
    SetRectXYWH(&guiV9SleepLeaveButton, centerX - 100, g_windowHeight - 80, 200, 42);
    SetRectXYWH(&guiV9ErrorBackButton, centerX - 100, g_windowHeight / 2 + 80, 200, 42);
    SetRectXYWH(&guiV9MultiplayerAddressFieldRect, centerX - 210, 220, 420, 44);
    SetRectXYWH(&guiV9TextureSlotRect, centerX - 260, 110, 520, g_windowHeight - 205);
    g_guiV9TextureSlots.rect = guiV9TextureSlotRect;
    g_guiV9TextureSlots.itemHeight = 42;
}

void GuiV9_Init(void)
{
    RECT r;
    GuiV9_UpdateScaledResolution();
    GuiV9_LoadTranslations();
    GuiV9_LoadTexturePacks();
    SetRectXYWH(&r, g_windowWidth / 2 - 210, 220, 420, 44);
    GuiV9_TextFieldInit(&g_guiV9ServerField, r, g_guiV9ConnectAddress, 120);
    if (g_guiV9ChatCount == 0) {
        GuiV9_AddChatLine("CloneMC GUI V9 loaded. Press T for chat, F2 for screenshot.");
    }
}

void GuiV9_InitIfNeeded(void)
{
    static int initDone = 0;
    if (initDone) { return; }
    initDone = 1;
    GuiV9_Init();
}

void GuiV9_EnterMultiplayer(void)
{
    RECT r;
    GuiV9_InitIfNeeded();
    g_state = STATE_MULTIPLAYER;
    ResetInputState();
    UnlockMouseFromGame();
    StartMenuMusic();
    GuiV9_Layout();
    r = guiV9MultiplayerAddressFieldRect;
    GuiV9_TextFieldInit(&g_guiV9ServerField, r, g_guiV9ConnectAddress, 120);
    g_guiV9ServerField.focused = 1;
}

void GuiV9_EnterConnecting(const char *address)
{
    GuiV9_InitIfNeeded();
    if (address && address[0]) {
        strncpy(g_guiV9ConnectAddress, address, sizeof(g_guiV9ConnectAddress) - 1);
        g_guiV9ConnectAddress[sizeof(g_guiV9ConnectAddress) - 1] = 0;
    }
    g_state = STATE_CONNECTING;
    g_guiV9ConnectStart = GetTickCount();
    sprintf(g_guiV9StatusMessage, "Connecting to %s...", g_guiV9ConnectAddress);
    ResetInputState();
    UnlockMouseFromGame();
    GuiV9_Layout();
}

void GuiV9_EnterConnectFailed(const char *message)
{
    g_state = STATE_CONNECT_FAILED;
    if (message) { strncpy(g_guiV9StatusMessage, message, sizeof(g_guiV9StatusMessage) - 1); }
    else { strcpy(g_guiV9StatusMessage, "Could not connect to server."); }
    g_guiV9StatusMessage[sizeof(g_guiV9StatusMessage) - 1] = 0;
    ResetInputState();
    UnlockMouseFromGame();
    GuiV9_Layout();
}

void GuiV9_EnterDownloadTerrain(void)
{
    g_state = STATE_DOWNLOAD_TERRAIN;
    g_guiV9DownloadProgress = 0;
    g_guiV9LoadingStart = GetTickCount();
    ResetInputState();
    UnlockMouseFromGame();
    GuiV9_Layout();
}

void GuiV9_EnterSleepMP(void)
{
    g_state = STATE_SLEEP_MP;
    ResetInputState();
    UnlockMouseFromGame();
    GuiV9_Layout();
}

void GuiV9_EnterTexturePacks(void)
{
    GuiV9_InitIfNeeded();
    g_state = STATE_TEXTURE_PACKS;
    ResetInputState();
    UnlockMouseFromGame();
    StartMenuMusic();
    GuiV9_LoadTexturePacks();
    GuiV9_Layout();
}

void GuiV9_EnterConflictWarning(const char *line1, const char *line2)
{
    g_state = STATE_CONFLICT_WARNING;
    strncpy(g_guiV9YesNoLine1, line1 ? line1 : "Texture pack may be incompatible.", sizeof(g_guiV9YesNoLine1) - 1);
    strncpy(g_guiV9YesNoLine2, line2 ? line2 : "Continue anyway?", sizeof(g_guiV9YesNoLine2) - 1);
    g_guiV9YesNoLine1[sizeof(g_guiV9YesNoLine1) - 1] = 0;
    g_guiV9YesNoLine2[sizeof(g_guiV9YesNoLine2) - 1] = 0;
    GuiV9_Layout();
}

void GuiV9_EnterErrorScreen(const char *title, const char *message)
{
    g_state = STATE_ERROR_SCREEN;
    strncpy(g_guiV9ErrorTitle, title ? title : "Error", sizeof(g_guiV9ErrorTitle) - 1);
    strncpy(g_guiV9ErrorMessage, message ? message : "Unknown error.", sizeof(g_guiV9ErrorMessage) - 1);
    g_guiV9ErrorTitle[sizeof(g_guiV9ErrorTitle) - 1] = 0;
    g_guiV9ErrorMessage[sizeof(g_guiV9ErrorMessage) - 1] = 0;
    ResetInputState();
    UnlockMouseFromGame();
    GuiV9_Layout();
}

void GuiV9_EnterYesNo(const char *title, const char *line1, const char *line2, int action, int arg)
{
    g_state = STATE_YESNO;
    strncpy(g_guiV9YesNoTitle, title ? title : "Confirm", sizeof(g_guiV9YesNoTitle) - 1);
    strncpy(g_guiV9YesNoLine1, line1 ? line1 : "Are you sure?", sizeof(g_guiV9YesNoLine1) - 1);
    strncpy(g_guiV9YesNoLine2, line2 ? line2 : "", sizeof(g_guiV9YesNoLine2) - 1);
    g_guiV9YesNoTitle[sizeof(g_guiV9YesNoTitle) - 1] = 0;
    g_guiV9YesNoLine1[sizeof(g_guiV9YesNoLine1) - 1] = 0;
    g_guiV9YesNoLine2[sizeof(g_guiV9YesNoLine2) - 1] = 0;
    g_guiV9YesNoAction = action;
    g_guiV9YesNoArg = arg;
    ResetInputState();
    UnlockMouseFromGame();
    GuiV9_Layout();
}

void GuiV9_DrawMultiplayer(void)
{
    POINT mouse;
    GuiV9_InitIfNeeded();
    Setup2D();
    GuiV9_Layout();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    GuiV9_DrawCenteredString(0, 60, g_windowWidth, 115, GuiV9_Tr("multiplayer.title", "Play Multiplayer"), 1.0f, 1.0f, 1.0f);
    GuiV9_TextFieldDraw(&g_guiV9ServerField, "Server Address");
    GuiV9_DrawCenteredString(0, 178, g_windowWidth, 205, "Networking packets are not active yet; this screen preserves Java-style UI flow.", 0.75f, 0.75f, 0.75f);
    DrawButton2D(guiV9ConnectButton, "Connect", PointInRectInt(mouse.x, mouse.y, guiV9ConnectButton));
    DrawButton2D(guiV9DirectButton, "Direct Connect", PointInRectInt(mouse.x, mouse.y, guiV9DirectButton));
    DrawButton2D(guiV9RefreshButton, "Refresh", PointInRectInt(mouse.x, mouse.y, guiV9RefreshButton));
    DrawButton2D(guiV9BackButton, "Cancel", PointInRectInt(mouse.x, mouse.y, guiV9BackButton));
    DrawBetaVignette2D();
}

void GuiV9_DrawConnecting(void)
{
    unsigned long elapsed;
    Setup2D();
    DrawDirtMenuBackground();
    elapsed = GetTickCount() - g_guiV9ConnectStart;
    GuiV9_DrawCenteredString(0, g_windowHeight / 2 - 40, g_windowWidth, g_windowHeight / 2, "Connecting to server...", 1.0f, 1.0f, 1.0f);
    GuiV9_DrawCenteredString(0, g_windowHeight / 2, g_windowWidth, g_windowHeight / 2 + 35, g_guiV9ConnectAddress, 0.75f, 0.75f, 0.75f);
    if (elapsed > 1600UL) {
        GuiV9_EnterConnectFailed("Connection failed: networking is not enabled in this single-player Open Watcom build.");
    }
}

void GuiV9_DrawConnectFailed(void)
{
    POINT mouse;
    Setup2D();
    GuiV9_Layout();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    GuiV9_DrawCenteredString(0, 100, g_windowWidth, 155, "Failed to connect to the server", 1.0f, 1.0f, 1.0f);
    GuiV9_DrawCenteredString(100, 185, g_windowWidth - 100, 235, g_guiV9StatusMessage, 0.85f, 0.85f, 0.85f);
    DrawButton2D(guiV9BackButton, "Back to server list", PointInRectInt(mouse.x, mouse.y, guiV9BackButton));
}

void GuiV9_DrawDownloadTerrain(void)
{
    int barW;
    int fillW;
    unsigned long elapsed;
    Setup2D();
    elapsed = GetTickCount() - g_guiV9LoadingStart;
    g_guiV9DownloadProgress = (int)(elapsed / 20UL);
    if (g_guiV9DownloadProgress > 100) { g_guiV9DownloadProgress = 100; }
    DrawDirtMenuBackground();
    GuiV9_DrawCenteredString(0, g_windowHeight / 2 - 60, g_windowWidth, g_windowHeight / 2 - 20, "Downloading terrain", 1.0f, 1.0f, 1.0f);
    barW = 300;
    fillW = barW * g_guiV9DownloadProgress / 100;
    DrawRect2D(g_windowWidth / 2 - barW / 2 - 2, g_windowHeight / 2 - 2, g_windowWidth / 2 + barW / 2 + 2, g_windowHeight / 2 + 18, 0.05f, 0.05f, 0.05f);
    DrawRect2D(g_windowWidth / 2 - barW / 2, g_windowHeight / 2, g_windowWidth / 2 - barW / 2 + fillW, g_windowHeight / 2 + 16, 0.55f, 0.55f, 0.55f);
}

void GuiV9_DrawSleepMP(void)
{
    POINT mouse;
    Setup2D();
    GuiV9_Layout();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    glEnable(GL_BLEND);
    GuiV9_DrawGradientRect(0, 0, g_windowWidth, g_windowHeight, 0.0f, 0.0f, 0.20f, 0.70f, 0.0f, 0.0f, 0.0f, 0.90f);
    GuiV9_DrawCenteredString(0, 120, g_windowWidth, 175, "Sleeping", 1.0f, 1.0f, 1.0f);
    GuiV9_DrawCenteredString(0, 175, g_windowWidth, 220, "Leave bed to return to the world.", 0.80f, 0.80f, 0.80f);
    DrawButton2D(guiV9SleepLeaveButton, "Leave Bed", PointInRectInt(mouse.x, mouse.y, guiV9SleepLeaveButton));
}

void GuiV9_LoadTexturePacks(void)
{
    FILE *f;
    char line[GUIV9_PACK_FILE_LEN + GUIV9_PACK_NAME_LEN + 8];
    char *sep;
    int len;
    g_guiV9TexturePackCount = 0;
    strcpy(g_guiV9TexturePacks[0].name, "Default CloneMC textures");
    strcpy(g_guiV9TexturePacks[0].file, "assets");
    g_guiV9TexturePacks[0].active = strcmp(g_resourceV10ActivePackPath, "assets") == 0 ? 1 : 0;
    g_guiV9TexturePacks[0].incompatible = 0;
    g_guiV9TexturePackCount = 1;
    CreateDirectory("assets\\texturepacks", NULL);
    f = fopen("assets\\texturepacks\\packs.txt", "r");
    if (!f) { f = fopen("assets/texturepacks/packs.txt", "r"); }
    if (f) {
        while (fgets(line, sizeof(line), f) && g_guiV9TexturePackCount < GUIV9_MAX_PACKS) {
            len = (int)strlen(line);
            while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) { line[len - 1] = 0; len--; }
            if (line[0] == 0 || line[0] == '#') { continue; }
            sep = strchr(line, '|');
            if (sep) {
                *sep = 0;
                strncpy(g_guiV9TexturePacks[g_guiV9TexturePackCount].name, line, GUIV9_PACK_NAME_LEN - 1);
                strncpy(g_guiV9TexturePacks[g_guiV9TexturePackCount].file, sep + 1, GUIV9_PACK_FILE_LEN - 1);
            } else {
                strncpy(g_guiV9TexturePacks[g_guiV9TexturePackCount].name, line, GUIV9_PACK_NAME_LEN - 1);
                strncpy(g_guiV9TexturePacks[g_guiV9TexturePackCount].file, line, GUIV9_PACK_FILE_LEN - 1);
            }
            g_guiV9TexturePacks[g_guiV9TexturePackCount].name[GUIV9_PACK_NAME_LEN - 1] = 0;
            g_guiV9TexturePacks[g_guiV9TexturePackCount].file[GUIV9_PACK_FILE_LEN - 1] = 0;
            g_guiV9TexturePacks[g_guiV9TexturePackCount].active = strcmp(g_guiV9TexturePacks[g_guiV9TexturePackCount].file, g_resourceV10ActivePackPath) == 0 ? 1 : 0;
            if (g_guiV9TexturePacks[g_guiV9TexturePackCount].active) { g_guiV9ActiveTexturePack = g_guiV9TexturePackCount; }
            g_guiV9TexturePacks[g_guiV9TexturePackCount].incompatible = 0;
            if (strstr(g_guiV9TexturePacks[g_guiV9TexturePackCount].name, "HD") || strstr(g_guiV9TexturePacks[g_guiV9TexturePackCount].name, "128")) {
                g_guiV9TexturePacks[g_guiV9TexturePackCount].incompatible = 1;
            }
            g_guiV9TexturePackCount++;
        }
        fclose(f);
    }
    g_guiV9TextureSlots.count = g_guiV9TexturePackCount;
    if (g_guiV9TextureSlots.selected < 0 || g_guiV9TextureSlots.selected >= g_guiV9TexturePackCount) { g_guiV9TextureSlots.selected = 0; }
}

void GuiV9_ApplyTexturePack(int index)
{
    int i;
    if (index < 0 || index >= g_guiV9TexturePackCount) { return; }
    for (i = 0; i < g_guiV9TexturePackCount; i++) { g_guiV9TexturePacks[i].active = 0; }
    g_guiV9TexturePacks[index].active = 1;
    g_guiV9ActiveTexturePack = index;
    ResourceV10_SetActivePackPath(g_guiV9TexturePacks[index].file);
    ResourceV10_ReloadTextures();
    sprintf(g_guiV9StatusMessage, "Selected texture pack: %s", g_guiV9TexturePacks[index].name);
    GuiV9_AddChatLine(g_guiV9StatusMessage);
}

void GuiV9_DrawSlotListFrame(GuiV9SlotList *slot)
{
    if (!slot) { return; }
    GuiV9_DrawGradientRect(slot->rect.left, slot->rect.top, slot->rect.right, slot->rect.bottom, 0.0f, 0.0f, 0.0f, 0.45f, 0.0f, 0.0f, 0.0f, 0.65f);
    DrawRect2D(slot->rect.left, slot->rect.top, slot->rect.right, slot->rect.top + 2, 0.30f, 0.30f, 0.30f);
    DrawRect2D(slot->rect.left, slot->rect.bottom - 2, slot->rect.right, slot->rect.bottom, 0.10f, 0.10f, 0.10f);
}

void GuiV9_ScrollSlot(GuiV9SlotList *slot, int amount)
{
    int maxScroll;
    int visible;
    if (!slot) { return; }
    visible = (slot->rect.bottom - slot->rect.top) / slot->itemHeight;
    maxScroll = slot->count - visible;
    if (maxScroll < 0) { maxScroll = 0; }
    slot->scroll += amount;
    if (slot->scroll < 0) { slot->scroll = 0; }
    if (slot->scroll > maxScroll) { slot->scroll = maxScroll; }
}

void GuiV9_DrawTexturePacks(void)
{
    POINT mouse;
    int i;
    int y;
    int visible;
    int idx;
    RECT row;
    char line[192];
    Setup2D();
    GuiV9_Layout();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    GuiV9_DrawCenteredString(0, 42, g_windowWidth, 92, "Select Texture Pack", 1.0f, 1.0f, 1.0f);
    GuiV9_DrawCenteredString(0, 82, g_windowWidth, 108, "Folders and stored ZIP packs from assets\\texturepacks\\packs.txt. Missing files fall back to default assets.", 0.72f, 0.72f, 0.72f);
    sprintf(line, "Active path: %s", g_resourceV10ActivePackPath);
    GuiV9_DrawCenteredString(0, 106, g_windowWidth, 126, line, 0.62f, 0.62f, 0.62f);
    sprintf(line, "GL: %s / max texture %d", g_resourceV10Version, g_resourceV10MaxTextureSize);
    GuiV9_DrawCenteredString(0, 126, g_windowWidth, 146, line, 0.55f, 0.55f, 0.55f);
    GuiV9_DrawSlotListFrame(&g_guiV9TextureSlots);
    visible = (g_guiV9TextureSlots.rect.bottom - g_guiV9TextureSlots.rect.top) / g_guiV9TextureSlots.itemHeight;
    y = g_guiV9TextureSlots.rect.top + 4;
    for (i = 0; i < visible; i++) {
        idx = g_guiV9TextureSlots.scroll + i;
        if (idx >= g_guiV9TexturePackCount) { break; }
        SetRectXYWH(&row, g_guiV9TextureSlots.rect.left + 8, y, g_guiV9TextureSlots.rect.right - g_guiV9TextureSlots.rect.left - 16, g_guiV9TextureSlots.itemHeight - 6);
        if (idx == g_guiV9TextureSlots.selected) { DrawRect2D(row.left - 2, row.top - 2, row.right + 2, row.bottom + 2, 0.85f, 0.85f, 0.85f); }
        if (PointInRectInt(mouse.x, mouse.y, row)) { DrawRect2D(row.left, row.top, row.right, row.bottom, 0.28f, 0.28f, 0.28f); }
        else { DrawRect2D(row.left, row.top, row.right, row.bottom, 0.14f, 0.14f, 0.14f); }
        sprintf(line, "%s%s", g_guiV9TexturePacks[idx].active ? "* " : "  ", g_guiV9TexturePacks[idx].name);
        GuiV9_DrawString(row.left + 12, row.top + 24, line, 1.0f, 1.0f, 1.0f);
        if (g_guiV9TexturePacks[idx].incompatible) { GuiV9_DrawString(row.right - 150, row.top + 24, "requires confirmation", 1.0f, 0.55f, 0.35f); }
        y += g_guiV9TextureSlots.itemHeight;
    }
    DrawButton2D(guiV9DoneButton, "Done", PointInRectInt(mouse.x, mouse.y, guiV9DoneButton));
    DrawButton2D(guiV9TextureOpenButton, "Open Folder", PointInRectInt(mouse.x, mouse.y, guiV9TextureOpenButton));
}

void GuiV9_DrawConflictWarning(void)
{
    POINT mouse;
    Setup2D();
    GuiV9_Layout();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    GuiV9_DrawCenteredString(0, g_windowHeight / 2 - 80, g_windowWidth, g_windowHeight / 2 - 40, "Texture Pack Warning", 1.0f, 1.0f, 1.0f);
    GuiV9_DrawCenteredString(80, g_windowHeight / 2 - 25, g_windowWidth - 80, g_windowHeight / 2 + 5, g_guiV9YesNoLine1, 0.88f, 0.88f, 0.88f);
    GuiV9_DrawCenteredString(80, g_windowHeight / 2 + 5, g_windowWidth - 80, g_windowHeight / 2 + 35, g_guiV9YesNoLine2, 0.88f, 0.88f, 0.88f);
    DrawButton2D(guiV9YesButton, "Use anyway", PointInRectInt(mouse.x, mouse.y, guiV9YesButton));
    DrawButton2D(guiV9NoButton, "Cancel", PointInRectInt(mouse.x, mouse.y, guiV9NoButton));
}

void GuiV9_DrawErrorScreen(void)
{
    POINT mouse;
    Setup2D();
    GuiV9_Layout();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    GuiV9_DrawCenteredString(0, 110, g_windowWidth, 165, g_guiV9ErrorTitle, 1.0f, 0.35f, 0.35f);
    GuiV9_DrawCenteredString(80, 195, g_windowWidth - 80, 255, g_guiV9ErrorMessage, 0.88f, 0.88f, 0.88f);
    DrawButton2D(guiV9ErrorBackButton, "Back", PointInRectInt(mouse.x, mouse.y, guiV9ErrorBackButton));
}

void GuiV9_DrawYesNo(void)
{
    POINT mouse;
    Setup2D();
    GuiV9_Layout();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    GuiV9_DrawCenteredString(0, g_windowHeight / 2 - 90, g_windowWidth, g_windowHeight / 2 - 50, g_guiV9YesNoTitle, 1.0f, 1.0f, 1.0f);
    GuiV9_DrawCenteredString(80, g_windowHeight / 2 - 35, g_windowWidth - 80, g_windowHeight / 2 - 5, g_guiV9YesNoLine1, 0.88f, 0.88f, 0.88f);
    GuiV9_DrawCenteredString(80, g_windowHeight / 2 - 5, g_windowWidth - 80, g_windowHeight / 2 + 25, g_guiV9YesNoLine2, 0.88f, 0.88f, 0.88f);
    DrawButton2D(guiV9YesButton, "Yes", PointInRectInt(mouse.x, mouse.y, guiV9YesButton));
    DrawButton2D(guiV9NoButton, "No", PointInRectInt(mouse.x, mouse.y, guiV9NoButton));
}

int GuiV9_SaveScreenshot(void)
{
    FILE *f;
    unsigned char *pixels;
    unsigned char header[54];
    char path[160];
    int rowSize;
    int imageSize;
    int fileSize;
    int x;
    int y;
    int src;
    int dst;
    unsigned char *row;
    unsigned long nowVal;
    CreateDirectory("screenshots", NULL);
    nowVal = GetTickCount();
    sprintf(path, "screenshots\\screenshot_%lu.bmp", nowVal);
    rowSize = ((g_windowWidth * 3 + 3) / 4) * 4;
    imageSize = rowSize * g_windowHeight;
    fileSize = 54 + imageSize;
    pixels = (unsigned char *)malloc((size_t)(g_windowWidth * g_windowHeight * 3));
    row = (unsigned char *)malloc((size_t)rowSize);
    if (!pixels || !row) {
        if (pixels) { free(pixels); }
        if (row) { free(row); }
        return 0;
    }
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, g_windowWidth, g_windowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    memset(header, 0, sizeof(header));
    header[0] = 'B'; header[1] = 'M';
    header[2] = (unsigned char)(fileSize & 255); header[3] = (unsigned char)((fileSize >> 8) & 255); header[4] = (unsigned char)((fileSize >> 16) & 255); header[5] = (unsigned char)((fileSize >> 24) & 255);
    header[10] = 54;
    header[14] = 40;
    header[18] = (unsigned char)(g_windowWidth & 255); header[19] = (unsigned char)((g_windowWidth >> 8) & 255); header[20] = (unsigned char)((g_windowWidth >> 16) & 255); header[21] = (unsigned char)((g_windowWidth >> 24) & 255);
    header[22] = (unsigned char)(g_windowHeight & 255); header[23] = (unsigned char)((g_windowHeight >> 8) & 255); header[24] = (unsigned char)((g_windowHeight >> 16) & 255); header[25] = (unsigned char)((g_windowHeight >> 24) & 255);
    header[26] = 1; header[28] = 24;
    header[34] = (unsigned char)(imageSize & 255); header[35] = (unsigned char)((imageSize >> 8) & 255); header[36] = (unsigned char)((imageSize >> 16) & 255); header[37] = (unsigned char)((imageSize >> 24) & 255);
    f = fopen(path, "wb");
    if (!f) { free(pixels); free(row); return 0; }
    fwrite(header, 1, 54, f);
    for (y = 0; y < g_windowHeight; y++) {
        memset(row, 0, (size_t)rowSize);
        for (x = 0; x < g_windowWidth; x++) {
            src = (y * g_windowWidth + x) * 3;
            dst = x * 3;
            row[dst + 0] = pixels[src + 2];
            row[dst + 1] = pixels[src + 1];
            row[dst + 2] = pixels[src + 0];
        }
        fwrite(row, 1, (size_t)rowSize, f);
    }
    fclose(f);
    free(pixels);
    free(row);
    return 1;
}

int GuiV9_HandleChar(WPARAM ch)
{
    char line[GUIV9_CHAT_TEXT_LEN + 8];
    if (g_guiV9ChatOpen) {
        if (ch == 13) {
            if (g_guiV9ChatField.text[0]) {
                GuiV9_RecordChatHistory(g_guiV9ChatField.text);
                if (g_guiV9ChatField.text[0] == '/') { GuiV9_RunChatCommand(g_guiV9ChatField.text); }
                else { sprintf(line, "<Player> %s", g_guiV9ChatField.text); GuiV9_AddChatLine(line); }
            }
            GuiV9_CloseChat();
            return 1;
        }
        if (ch == 27) { GuiV9_CloseChat(); return 1; }
        return GuiV9_TextFieldChar(&g_guiV9ChatField, ch);
    }
    if (g_state == STATE_MULTIPLAYER) {
        return GuiV9_TextFieldChar(&g_guiV9ServerField, ch);
    }
    return 0;
}

int GuiV9_HandleKeyDown(WPARAM key)
{
    if (key == VK_F2) {
        if (GuiV9_SaveScreenshot()) { GuiV9_AddChatLine("Saved screenshot."); }
        else { GuiV9_AddChatLine("Could not save screenshot."); }
        return 1;
    }
    if (g_guiV9ChatOpen) {
        if (key == VK_ESCAPE) { GuiV9_CloseChat(); return 1; }
        if (key == VK_UP) {
            if (g_guiV9HistoryCount > 0) {
                g_guiV9HistoryCursor++;
                if (g_guiV9HistoryCursor >= g_guiV9HistoryCount) { g_guiV9HistoryCursor = g_guiV9HistoryCount - 1; }
                strncpy(g_guiV9ChatField.text, g_guiV9ChatHistory[g_guiV9HistoryCursor], GUIV9_TEXT_LEN - 1);
                g_guiV9ChatField.text[GUIV9_TEXT_LEN - 1] = 0;
                g_guiV9ChatField.cursor = (int)strlen(g_guiV9ChatField.text);
            }
            return 1;
        }
        if (key == VK_DOWN) {
            if (g_guiV9HistoryCursor > 0) { g_guiV9HistoryCursor--; strncpy(g_guiV9ChatField.text, g_guiV9ChatHistory[g_guiV9HistoryCursor], GUIV9_TEXT_LEN - 1); }
            else { g_guiV9HistoryCursor = -1; g_guiV9ChatField.text[0] = 0; }
            g_guiV9ChatField.text[GUIV9_TEXT_LEN - 1] = 0;
            g_guiV9ChatField.cursor = (int)strlen(g_guiV9ChatField.text);
            return 1;
        }
        return GuiV9_TextFieldKey(&g_guiV9ChatField, key);
    }
    if (g_state == STATE_GAME && (key == 'T' || key == VK_OEM_2)) {
        GuiV9_OpenChat();
        if (key == VK_OEM_2) { g_guiV9ChatField.text[0] = '/'; g_guiV9ChatField.text[1] = 0; g_guiV9ChatField.cursor = 1; }
        return 1;
    }
    if (g_state == STATE_MULTIPLAYER) {
        if (key == VK_RETURN) { GuiV9_EnterConnecting(g_guiV9ServerField.text); return 1; }
        if (GuiV9_TextFieldKey(&g_guiV9ServerField, key)) { return 1; }
    }
    if (g_state == STATE_TEXTURE_PACKS) {
        if (key == VK_UP) { g_guiV9TextureSlots.selected--; if (g_guiV9TextureSlots.selected < 0) { g_guiV9TextureSlots.selected = 0; } if (g_guiV9TextureSlots.selected < g_guiV9TextureSlots.scroll) { g_guiV9TextureSlots.scroll = g_guiV9TextureSlots.selected; } return 1; }
        if (key == VK_DOWN) { g_guiV9TextureSlots.selected++; if (g_guiV9TextureSlots.selected >= g_guiV9TexturePackCount) { g_guiV9TextureSlots.selected = g_guiV9TexturePackCount - 1; } GuiV9_ScrollSlot(&g_guiV9TextureSlots, 1); return 1; }
        if (key == VK_RETURN) { GuiV9_ApplyTexturePack(g_guiV9TextureSlots.selected); return 1; }
    }
    if (g_state == STATE_MULTIPLAYER || g_state == STATE_CONNECT_FAILED || g_state == STATE_CONNECTING ||
        g_state == STATE_TEXTURE_PACKS || g_state == STATE_CONFLICT_WARNING || g_state == STATE_ERROR_SCREEN ||
        g_state == STATE_YESNO || g_state == STATE_SLEEP_MP) {
        if (key == VK_ESCAPE) {
            if (g_state == STATE_MULTIPLAYER || g_state == STATE_TEXTURE_PACKS) { EnterMenu(); }
            else if (g_state == STATE_CONNECT_FAILED || g_state == STATE_CONNECTING) { GuiV9_EnterMultiplayer(); }
            else if (g_state == STATE_SLEEP_MP) { EnterGame(); }
            else if (g_state == STATE_ERROR_SCREEN) { EnterMenu(); }
            else { EnterMenu(); }
            return 1;
        }
    }
    return 0;
}

int GuiV9_HandleMouseWheel(int delta)
{
    if (g_state == STATE_TEXTURE_PACKS) {
        if (delta > 0) { GuiV9_ScrollSlot(&g_guiV9TextureSlots, -1); }
        else { GuiV9_ScrollSlot(&g_guiV9TextureSlots, 1); }
        return 1;
    }
    return 0;
}

int GuiV9_HandleMouseDown(int mx, int my)
{
    int i;
    int idx;
    int y;
    RECT row;
    if (g_state == STATE_MULTIPLAYER) {
        GuiV9_TextFieldMouse(&g_guiV9ServerField, mx, my);
        if (PointInRectInt(mx, my, guiV9ConnectButton) || PointInRectInt(mx, my, guiV9DirectButton)) { GuiV9_EnterConnecting(g_guiV9ServerField.text); return 1; }
        if (PointInRectInt(mx, my, guiV9RefreshButton)) { GuiV9_AddChatLine("Server list refreshed locally."); return 1; }
        if (PointInRectInt(mx, my, guiV9BackButton)) { EnterMenu(); return 1; }
        return 1;
    }
    if (g_state == STATE_CONNECT_FAILED) { if (PointInRectInt(mx, my, guiV9BackButton)) { GuiV9_EnterMultiplayer(); return 1; } return 1; }
    if (g_state == STATE_TEXTURE_PACKS) {
        y = g_guiV9TextureSlots.rect.top + 4;
        for (i = 0; i < (g_guiV9TextureSlots.rect.bottom - g_guiV9TextureSlots.rect.top) / g_guiV9TextureSlots.itemHeight; i++) {
            idx = g_guiV9TextureSlots.scroll + i;
            if (idx >= g_guiV9TexturePackCount) { break; }
            SetRectXYWH(&row, g_guiV9TextureSlots.rect.left + 8, y, g_guiV9TextureSlots.rect.right - g_guiV9TextureSlots.rect.left - 16, g_guiV9TextureSlots.itemHeight - 6);
            if (PointInRectInt(mx, my, row)) {
                g_guiV9TextureSlots.selected = idx;
                if (g_guiV9TexturePacks[idx].incompatible) { GuiV9_EnterConflictWarning("This pack looks higher resolution than the legacy renderer target.", "It may be slower on Windows 98 hardware."); }
                else { GuiV9_ApplyTexturePack(idx); }
                return 1;
            }
            y += g_guiV9TextureSlots.itemHeight;
        }
        if (PointInRectInt(mx, my, guiV9DoneButton)) { EnterMenu(); return 1; }
        if (PointInRectInt(mx, my, guiV9TextureOpenButton)) { GuiV9_AddChatLine("Texture pack folder: assets\\texturepacks"); return 1; }
        return 1;
    }
    if (g_state == STATE_CONFLICT_WARNING) {
        if (PointInRectInt(mx, my, guiV9YesButton)) { GuiV9_ApplyTexturePack(g_guiV9TextureSlots.selected); GuiV9_EnterTexturePacks(); return 1; }
        if (PointInRectInt(mx, my, guiV9NoButton)) { GuiV9_EnterTexturePacks(); return 1; }
        return 1;
    }
    if (g_state == STATE_ERROR_SCREEN) { if (PointInRectInt(mx, my, guiV9ErrorBackButton)) { EnterMenu(); return 1; } return 1; }
    if (g_state == STATE_SLEEP_MP) { if (PointInRectInt(mx, my, guiV9SleepLeaveButton)) { EnterGame(); return 1; } return 1; }
    if (g_state == STATE_YESNO) {
        if (PointInRectInt(mx, my, guiV9YesButton)) {
            if (g_guiV9YesNoAction == 1) { DeleteWorldSlot(g_guiV9YesNoArg); EnterWorldSelect(); }
            else { EnterMenu(); }
            return 1;
        }
        if (PointInRectInt(mx, my, guiV9NoButton)) { EnterWorldSelect(); return 1; }
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------ */
/* 2D setup and menu drawing                                    */
/* ------------------------------------------------------------ */

void Setup2D(void)
{
    /* GUI/menu render reset.  Several 3D render paths enable blending,
       depth, culling, and tinted glColor values; if those leak into the
       menus, the buttons/backgrounds smear or draw with wrong colors. */
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, g_windowWidth, g_windowHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int EstimateTextWidth(GLuint base, const char *text)
{
    int len;
    if (!text) { return 0; }
    len = (int)strlen(text);
    if (base == fontBaseTitle) { return len * 36; }
    return len * 12;
}


void DrawCenteredText2D(GLuint base, int x1, int y1, int x2, int y2, const char *text)
{
    int w;
    int x;
    int y;

    w = EstimateTextWidth(base, text);
    x = x1 + ((x2 - x1) - w) / 2;

    if (base == fontBaseTitle) {
        y = y1 + ((y2 - y1) / 2) + 23;
    } else {
        y = y1 + ((y2 - y1) / 2) + 8;
    }

    DrawText2D(base, x, y, text);
}

void DrawImportedBetaLogo(void)
{
    int y;

    y = 74;

    glColor3f(0.04f, 0.04f, 0.04f);
    DrawCenteredText2D(fontBaseTitle, 0, y + 4, g_windowWidth, y + 92 + 4, "CLONEMC");

    glColor3f(0.86f, 0.86f, 0.86f);
    DrawCenteredText2D(fontBaseTitle, 0, y, g_windowWidth, y + 92, "CLONEMC");
}


void DrawBetaMenuFooter(void)
{
    /* Core Beta-style menu screens only: no extra build/debug notes. */
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}


void DrawBetaButtonPanel(RECT r, int hover, int disabled)
{
    int sy;

    if (texBetaGui) {
        if (disabled) {
            sy = 46;
        } else if (hover) {
            sy = 86;
        } else {
            sy = 66;
        }

        DrawImageCrop2D(texBetaGui, 256, 256, 0, sy, 200, 20,
                        r.left, r.top, r.right, r.bottom, 1.0f);
        return;
    }

    DrawRect2D(r.left, r.top, r.right, r.bottom, 0.04f, 0.04f, 0.04f);
    if (disabled) {
        DrawRect2D(r.left + 3, r.top + 3, r.right - 3, r.bottom - 3, 0.22f, 0.22f, 0.22f);
    } else if (hover) {
        DrawRect2D(r.left + 3, r.top + 3, r.right - 3, r.bottom - 3, 0.66f, 0.66f, 0.66f);
    } else {
        DrawRect2D(r.left + 3, r.top + 3, r.right - 3, r.bottom - 3, 0.36f, 0.36f, 0.36f);
    }
}

void DrawDisabledButton2D(RECT r, const char *text)
{
    DrawBetaButtonPanel(r, 0, 1);

    if (text && text[0] != '\0') {
        glColor3f(0.18f, 0.18f, 0.18f);
        DrawCenteredText2D(fontBaseNormal, r.left + 2, r.top + 2, r.right + 2, r.bottom + 2, text);
        glColor3f(0.55f, 0.55f, 0.55f);
        DrawCenteredText2D(fontBaseNormal, r.left, r.top, r.right, r.bottom, text);
    }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_TEXTURE_2D);
}


void DrawBetaVignette2D(void)
{
    int w;
    int h;
    int band;
    float night;
    float edgeA;
    float bottomA;

    w = g_windowWidth;
    h = g_windowHeight;
    band = 96;

    if (w <= 0 || h <= 0) {
        return;
    }

    /* V41: remove the daytime/sun vignette.  Only keep a softer moon/night
       edge fade so daytime looks bright and clean. */
    night = 1.0f - g_dayNightBlend;
    if (night < 0.25f) { return; }
    edgeA = 0.22f * night;
    bottomA = 0.24f * night;

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);

    glColor4f(0.0f, 0.0f, 0.0f, edgeA); glVertex2i(0, 0); glVertex2i(w, 0);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);  glVertex2i(w, band); glVertex2i(0, band);

    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);  glVertex2i(0, h - band); glVertex2i(w, h - band);
    glColor4f(0.0f, 0.0f, 0.0f, bottomA); glVertex2i(w, h); glVertex2i(0, h);

    glColor4f(0.0f, 0.0f, 0.0f, edgeA); glVertex2i(0, 0); glVertex2i(0, h);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);  glVertex2i(band, h); glVertex2i(band, 0);

    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);  glVertex2i(w - band, 0); glVertex2i(w - band, h);
    glColor4f(0.0f, 0.0f, 0.0f, edgeA); glVertex2i(w, h); glVertex2i(w, 0);

    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawWaterOverlay2D(void)
{
    int x;
    int y;
    int tile;
    int sx;
    int sy;
    float darkAlpha;
    float overlayAlpha;

    if (g_state != STATE_GAME) {
        return;
    }

    if (IsPlayerHeadUnderWater()) {
        darkAlpha = WATER_DARKEN_ALPHA_HEAD;
        overlayAlpha = WATER_OVERLAY_ALPHA_HEAD;
    } else if (IsPlayerInWater()) {
        darkAlpha = WATER_DARKEN_ALPHA_BODY;
        overlayAlpha = WATER_OVERLAY_ALPHA_BODY;
    } else {
        return;
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.03f, 0.18f, darkAlpha);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(g_windowWidth, 0);
    glVertex2i(g_windowWidth, g_windowHeight);
    glVertex2i(0, g_windowHeight);
    glEnd();

    if (texBetaWater) {
        tile = WATER_OVERLAY_TILE_PIXELS;
        sx = (int)(g_worldTimeSeconds * 18.0) & 63;
        sy = (int)(g_worldTimeSeconds * 11.0) & 63;
        for (y = -tile; y < g_windowHeight + tile; y += tile) {
            for (x = -tile; x < g_windowWidth + tile; x += tile) {
                DrawImageCrop2D(texBetaWater, 16, 16, 0, 0, 16, 16,
                                x + sx, y + sy, x + sx + tile, y + sy + tile,
                                overlayAlpha);
            }
        }
    }

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    g_vertexTintR = 1.0f;
    g_vertexTintG = 1.0f;
    g_vertexTintB = 1.0f;
    glEnable(GL_DEPTH_TEST);
}

void DrawOxygenBubbles(void)
{
    int air;
    int full;
    int partial;
    int icons;
    int x;
    int y;
    int i;
    int sx;

    if (!IsPlayerInWater()) {
        return;
    }

    air = (int)((g_playerAirTimer / 12.0) * 300.0);
    if (air < 0) { air = 0; }
    if (air > 300) { air = 300; }

    full = (int)ceil(((double)(air - 2) * 10.0) / 300.0);
    icons = (int)ceil(((double)air * 10.0) / 300.0);
    if (full < 0) { full = 0; }
    if (full > 10) { full = 10; }
    if (icons < 0) { icons = 0; }
    if (icons > 10) { icons = 10; }
    partial = icons - full;
    if (partial < 0) { partial = 0; }

    x = g_windowWidth / 2 - 91 * 2;
    y = g_windowHeight - 104;

    for (i = 0; i < icons; i++) {
        sx = (i < full) ? 16 : 25;
        DrawImageCrop2D(texBetaIcons, 256, 256, sx, 18, 9, 9,
                        x + i * BUBBLE_ICON_SIZE, y,
                        x + i * BUBBLE_ICON_SIZE + BUBBLE_ICON_SIZE,
                        y + BUBBLE_ICON_SIZE, 1.0f);
    }
}





/* ------------------------------------------------------------ */
/* GUI_LIGHT_V7: extra Java GUI screens and full lighting bridge */
/* ------------------------------------------------------------ */

const char *GuiV7_KeyName(WPARAM key)
{
    static char name[24];
    if (key == VK_SPACE) { return "SPACE"; }
    if (key == VK_F5) { return "F5"; }
    if (key == VK_ESCAPE) { return "ESC"; }
    if (key >= 'A' && key <= 'Z') { name[0] = (char)key; name[1] = 0; return name; }
    if (key >= '0' && key <= '9') { name[0] = (char)key; name[1] = 0; return name; }
    sprintf(name, "KEY %lu", (unsigned long)key);
    return name;
}

int GuiV7_CountUnlockedAchievements(void)
{
    int i;
    int n;
    n = 0;
    for (i = 0; i < JAVA_COMPAT_ACH_COUNT; i++) {
        if (g_javaCompatAchievements[i].unlocked) { n++; }
    }
    return n;
}

void GuiV7_DrawSliderButton(RECT r, const char *label, int value, int maxValue, int hover)
{
    int barX0;
    int barX1;
    int barY0;
    int fillX;
    DrawButton2D(r, label, hover);
    if (maxValue <= 0) { maxValue = 1; }
    if (value < 0) { value = 0; }
    if (value > maxValue) { value = maxValue; }
    barX0 = r.left + 18;
    barX1 = r.right - 18;
    barY0 = r.bottom - 9;
    fillX = barX0 + ((barX1 - barX0) * value) / maxValue;
    DrawRect2D(barX0, barY0, barX1, barY0 + 4, 0.08f, 0.08f, 0.08f);
    DrawRect2D(barX0, barY0, fillX, barY0 + 4, 0.78f, 0.78f, 0.78f);
}

void GuiV7_EnterVideoSettings(void)
{
    g_state = STATE_VIDEO_SETTINGS;
    ResetInputState();
    UnlockMouseFromGame();
    LayoutBetaMenus();
}

void GuiV7_EnterControls(void)
{
    g_state = STATE_CONTROLS;
    g_guiPendingControlV7 = -1;
    ResetInputState();
    UnlockMouseFromGame();
    LayoutBetaMenus();
}

void GuiV7_EnterAchievements(void)
{
    g_state = STATE_ACHIEVEMENTS;
    ResetInputState();
    UnlockMouseFromGame();
    LayoutBetaMenus();
}

void GuiV7_EnterStats(void)
{
    g_state = STATE_STATS;
    ResetInputState();
    UnlockMouseFromGame();
    LayoutBetaMenus();
}

void GuiV7_EnterRenameWorld(void)
{
    if (selectedWorldSlot < 0 || selectedWorldSlot >= MAX_WORLD_SLOTS) { return; }
    if (!worldSaves[selectedWorldSlot].exists) { return; }
    strncpy(g_renameWorldName, worldSaves[selectedWorldSlot].name, WORLD_NAME_LEN - 1);
    g_renameWorldName[WORLD_NAME_LEN - 1] = 0;
    g_state = STATE_RENAME_WORLD;
    ResetInputState();
    UnlockMouseFromGame();
    LayoutBetaMenus();
}

void GuiV7_RenameSelectedWorld(void)
{
    if (selectedWorldSlot < 0 || selectedWorldSlot >= MAX_WORLD_SLOTS) { return; }
    if (!worldSaves[selectedWorldSlot].exists) { return; }
    if (g_renameWorldName[0] == 0) { strcpy(g_renameWorldName, "World"); }
    SanitizeWorldName(g_renameWorldName);
    strncpy(worldSaves[selectedWorldSlot].name, g_renameWorldName, WORLD_NAME_LEN - 1);
    worldSaves[selectedWorldSlot].name[WORLD_NAME_LEN - 1] = 0;
    SaveWorldSlotInfo(selectedWorldSlot);
    SaveHandler_SaveWorldInfoV2(selectedWorldSlot);
    LoadWorldList();
    EnterWorldSelect();
}

int GuiV7_HandleChar(WPARAM ch)
{
    int len;
    if (g_state != STATE_RENAME_WORLD) { return 0; }
    if (ch == 8) {
        len = (int)strlen(g_renameWorldName);
        if (len > 0) { g_renameWorldName[len - 1] = 0; }
        return 1;
    }
    if (ch == 13) { GuiV7_RenameSelectedWorld(); return 1; }
    if (ch >= 32 && ch < 127) {
        len = (int)strlen(g_renameWorldName);
        if (len < WORLD_NAME_LEN - 1) {
            g_renameWorldName[len] = (char)ch;
            g_renameWorldName[len + 1] = 0;
        }
        return 1;
    }
    return 1;
}

int GuiV7_HandleKeyDown(WPARAM key)
{
    if (g_state == STATE_CONTROLS && g_guiPendingControlV7 >= 0) {
        if (g_guiPendingControlV7 == 0) { g_keyBindForwardV7 = key; }
        else if (g_guiPendingControlV7 == 1) { g_keyBindBackV7 = key; }
        else if (g_guiPendingControlV7 == 2) { g_keyBindLeftV7 = key; }
        else if (g_guiPendingControlV7 == 3) { g_keyBindRightV7 = key; }
        else if (g_guiPendingControlV7 == 4) { g_keyBindJumpV7 = key; }
        else if (g_guiPendingControlV7 == 5) { g_keyBindInventoryV7 = key; }
        else if (g_guiPendingControlV7 == 6) { g_keyBindDropV7 = key; }
        else if (g_guiPendingControlV7 == 7) { g_keyBindPerspectiveV7 = key; }
        g_guiPendingControlV7 = -1;
        return 1;
    }
    return 0;
}

int GuiV7_HandleMouseDown(int mx, int my)
{
    int i;
    if (g_state == STATE_VIDEO_SETTINGS) {
        if (PointInRectInt(mx, my, videoRenderDistanceButton)) { SetRenderDistanceFromMouse(mx); return 1; }
        if (PointInRectInt(mx, my, videoBrightnessButton)) { g_highGamma = 1 - g_highGamma; ComputeLegacyLighting(); return 1; }
        if (PointInRectInt(mx, my, videoSmoothLightingButton)) { g_videoSmoothLightingV7 = 1 - g_videoSmoothLightingV7; return 1; }
        if (PointInRectInt(mx, my, videoCloudsButton)) { g_videoCloudsV7 = 1 - g_videoCloudsV7; return 1; }
        if (PointInRectInt(mx, my, videoParticlesButton)) { g_videoParticlesV7 = 1 - g_videoParticlesV7; return 1; }
        if (PointInRectInt(mx, my, videoFogButton)) { g_videoFogV7 = 1 - g_videoFogV7; return 1; }
        if (PointInRectInt(mx, my, videoFullscreenButton)) { ToggleFullscreen(); return 1; }
        if (PointInRectInt(mx, my, videoDoneButton)) { g_state = STATE_OPTIONS; LayoutBetaMenus(); return 1; }
        return 1;
    }
    if (g_state == STATE_CONTROLS) {
        for (i = 0; i < 8; i++) {
            if (PointInRectInt(mx, my, controlsButtons[i])) { g_guiPendingControlV7 = i; return 1; }
        }
        if (PointInRectInt(mx, my, controlsDoneButton)) { g_state = STATE_OPTIONS; LayoutBetaMenus(); return 1; }
        return 1;
    }
    if (g_state == STATE_RENAME_WORLD) {
        if (PointInRectInt(mx, my, renameDoneButton)) { GuiV7_RenameSelectedWorld(); return 1; }
        if (PointInRectInt(mx, my, renameCancelButton)) { EnterWorldSelect(); return 1; }
        return 1;
    }
    if (g_state == STATE_ACHIEVEMENTS) {
        if (PointInRectInt(mx, my, achievementsDoneButton)) { g_state = STATE_OPTIONS; LayoutBetaMenus(); return 1; }
        return 1;
    }
    if (g_state == STATE_STATS) {
        if (PointInRectInt(mx, my, statsDoneButton)) { g_state = STATE_OPTIONS; LayoutBetaMenus(); return 1; }
        return 1;
    }
    return 0;
}

void GuiV7_DrawVideoSettings(void)
{
    POINT mouse;
    char label[128];
    Setup2D();
    LayoutBetaMenus();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseTitle, 0, 72, g_windowWidth, 120, "Video Settings");
    sprintf(label, "Render Distance: %s", RenderDistanceLabel());
    GuiV7_DrawSliderButton(videoRenderDistanceButton, label, g_renderDistanceChunks - 1, RENDER_DISTANCE_MAX_CHUNKS - 1, PointInRectInt(mouse.x, mouse.y, videoRenderDistanceButton));
    sprintf(label, "Brightness: %s", g_highGamma ? "Bright" : "Normal");
    DrawButton2D(videoBrightnessButton, label, PointInRectInt(mouse.x, mouse.y, videoBrightnessButton));
    sprintf(label, "Smooth Lighting: %s", g_videoSmoothLightingV7 ? "ON" : "OFF");
    DrawButton2D(videoSmoothLightingButton, label, PointInRectInt(mouse.x, mouse.y, videoSmoothLightingButton));
    sprintf(label, "Clouds: %s", g_videoCloudsV7 ? "ON" : "OFF");
    DrawButton2D(videoCloudsButton, label, PointInRectInt(mouse.x, mouse.y, videoCloudsButton));
    sprintf(label, "Particles: %s", g_videoParticlesV7 ? "ON" : "OFF");
    DrawButton2D(videoParticlesButton, label, PointInRectInt(mouse.x, mouse.y, videoParticlesButton));
    sprintf(label, "Fog: %s", g_videoFogV7 ? "ON" : "OFF");
    DrawButton2D(videoFogButton, label, PointInRectInt(mouse.x, mouse.y, videoFogButton));
    sprintf(label, "Fullscreen: %s", g_isFullscreen ? "ON" : "OFF");
    DrawButton2D(videoFullscreenButton, label, PointInRectInt(mouse.x, mouse.y, videoFullscreenButton));
    DrawButton2D(videoDoneButton, "Done", PointInRectInt(mouse.x, mouse.y, videoDoneButton));
    DrawBetaVignette2D();
}

void GuiV7_DrawControls(void)
{
    POINT mouse;
    char label[128];
    WPARAM keys[8];
    const char *names[8];
    int i;
    keys[0] = g_keyBindForwardV7; keys[1] = g_keyBindBackV7; keys[2] = g_keyBindLeftV7; keys[3] = g_keyBindRightV7;
    keys[4] = g_keyBindJumpV7; keys[5] = g_keyBindInventoryV7; keys[6] = g_keyBindDropV7; keys[7] = g_keyBindPerspectiveV7;
    names[0] = "Forward"; names[1] = "Back"; names[2] = "Left"; names[3] = "Right";
    names[4] = "Jump"; names[5] = "Inventory"; names[6] = "Drop"; names[7] = "Perspective";
    Setup2D();
    LayoutBetaMenus();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseTitle, 0, 68, g_windowWidth, 112, "Controls");
    for (i = 0; i < 8; i++) {
        if (g_guiPendingControlV7 == i) { sprintf(label, "%s: > press key <", names[i]); }
        else { sprintf(label, "%s: %s", names[i], GuiV7_KeyName(keys[i])); }
        DrawButton2D(controlsButtons[i], label, PointInRectInt(mouse.x, mouse.y, controlsButtons[i]));
    }
    DrawButton2D(controlsDoneButton, "Done", PointInRectInt(mouse.x, mouse.y, controlsDoneButton));
    DrawBetaVignette2D();
}

void GuiV7_DrawRenameWorld(void)
{
    POINT mouse;
    Setup2D();
    LayoutBetaMenus();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseTitle, 0, 75, g_windowWidth, 125, "Rename World");
    DrawTextField2D(renameNameField, "New Name", g_renameWorldName, 1);
    DrawButton2D(renameDoneButton, "Rename", PointInRectInt(mouse.x, mouse.y, renameDoneButton));
    DrawButton2D(renameCancelButton, "Cancel", PointInRectInt(mouse.x, mouse.y, renameCancelButton));
    DrawBetaVignette2D();
}

void GuiV7_DrawAchievements(void)
{
    POINT mouse;
    int i;
    int x;
    int y;
    InventorySlot iconSlot;
    char line[128];
    Setup2D();
    LayoutBetaMenus();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(line, "Achievements (%d/%d)", GuiV7_CountUnlockedAchievements(), JAVA_COMPAT_ACH_COUNT);
    DrawCenteredText2D(fontBaseTitle, 0, 45, g_windowWidth, 90, line);
    x = g_windowWidth / 2 - 300;
    y = 105;
    for (i = 0; i < JAVA_COMPAT_ACH_COUNT; i++) {
        if (i == 6) { x = g_windowWidth / 2 + 20; y = 105; }
        DrawRect2D(x, y, x + 270, y + 42, 0.10f, 0.10f, 0.10f);
        iconSlot.item = g_javaCompatAchievements[i].iconItem;
        iconSlot.count = 1;
        iconSlot.damage = 0;
        DrawInventorySlot(x + 6, y + 3, iconSlot, 0);
        if (g_javaCompatAchievements[i].unlocked) { glColor3f(1.0f, 1.0f, 0.55f); }
        else { glColor3f(0.50f, 0.50f, 0.50f); }
        DrawText2D(fontBaseNormal, x + 50, y + 13, JavaCompat_Translate(g_javaCompatAchievements[i].key));
        y += 48;
    }
    DrawButton2D(achievementsDoneButton, "Done", PointInRectInt(mouse.x, mouse.y, achievementsDoneButton));
    DrawBetaVignette2D();
}

void GuiV7_DrawStats(void)
{
    POINT mouse;
    char line[160];
    int y;
    int activeMobs;
    int activeDrops;
    int i;
    activeMobs = 0;
    activeDrops = 0;
    for (i = 0; i < MAX_MOBS; i++) { if (mobs[i].active) { activeMobs++; } }
    for (i = 0; i < MAX_DROPPED_ITEMS; i++) { if (droppedItems[i].active) { activeDrops++; } }
    Setup2D();
    LayoutBetaMenus();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseTitle, 0, 50, g_windowWidth, 95, "Statistics");
    y = 125;
#define GUIV7_STAT_LINE(txt) DrawCenteredText2D(fontBaseNormal, 0, y, g_windowWidth, y + 25, txt); y += 30
    sprintf(line, "World: %s", currentWorldSlot >= 0 ? worldSaves[currentWorldSlot].name : "No world loaded"); GUIV7_STAT_LINE(line);
    sprintf(line, "Blocks broken: %lu", g_statsBlocksBrokenV7); GUIV7_STAT_LINE(line);
    sprintf(line, "Blocks placed: %lu", g_statsBlocksPlacedV7); GUIV7_STAT_LINE(line);
    sprintf(line, "Containers opened: %lu", g_statsContainersOpenedV7); GUIV7_STAT_LINE(line);
    sprintf(line, "Achievements: %d/%d", GuiV7_CountUnlockedAchievements(), JAVA_COMPAT_ACH_COUNT); GUIV7_STAT_LINE(line);
    sprintf(line, "Active mobs: %d / %d", activeMobs, MAX_MOBS); GUIV7_STAT_LINE(line);
    sprintf(line, "Dropped items: %d / %d", activeDrops, MAX_DROPPED_ITEMS); GUIV7_STAT_LINE(line);
    sprintf(line, "Player position: %.1f %.1f %.1f", (float)(worldOriginBlockX + playerX), (float)playerY, (float)(worldOriginBlockZ + playerZ)); GUIV7_STAT_LINE(line);
    sprintf(line, "Lighting V48: queue %d skySched %ld blockSched %ld changed %ld", g_lightQueueCountV48, g_lightV48ScheduledSky, g_lightV48ScheduledBlock, g_lightV48ChangedCells); GUIV7_STAT_LINE(line);
#undef GUIV7_STAT_LINE
    DrawButton2D(statsDoneButton, "Done", PointInRectInt(mouse.x, mouse.y, statsDoneButton));
    DrawBetaVignette2D();
}

void DrawMenu(void)
{
    POINT mouse;
    int hoverSingle;
    int hoverPack;
    int hoverOptions;
    int hoverQuit;
    Setup2D();
    LayoutBetaMenus();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    hoverSingle = PointInRectInt(mouse.x, mouse.y, singleplayerButton);
    hoverPack = PointInRectInt(mouse.x, mouse.y, texturePackButton);
    hoverOptions = PointInRectInt(mouse.x, mouse.y, optionsButton);
    hoverQuit = PointInRectInt(mouse.x, mouse.y, quitButton);
    DrawDirtMenuBackground();
    DrawImportedBetaLogo();
    DrawButton2D(singleplayerButton, "Singleplayer", hoverSingle);
    DrawButton2D(texturePackButton, "Mods and Texture Packs", hoverPack);
    DrawButton2D(optionsButton, "Options...", hoverOptions);
    DrawButton2D(quitButton, "Quit Game", hoverQuit);
    DrawBetaMenuFooter();
    DrawBetaVignette2D();
}

void DrawWorldSelect(void)
{
    POINT mouse;
    char line[128];
    int i;
    int hover;
    int canUseSelected;
    int firstVisible;
    int lastVisible;

    Setup2D();
    LayoutBetaMenus();

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);

    canUseSelected = 0;
    if (selectedWorldSlot >= 0 && selectedWorldSlot < MAX_WORLD_SLOTS &&
        worldSaves[selectedWorldSlot].exists) {
        canUseSelected = 1;
    }
    g_guiV21WorldScrollMax = MAX_WORLD_SLOTS - GUIV21_WORLD_VISIBLE_SLOTS;
    if (g_guiV21WorldScrollMax < 0) { g_guiV21WorldScrollMax = 0; }
    if (g_guiV21WorldScroll < 0) { g_guiV21WorldScroll = 0; }
    if (g_guiV21WorldScroll > g_guiV21WorldScrollMax) { g_guiV21WorldScroll = g_guiV21WorldScrollMax; }
    firstVisible = g_guiV21WorldScroll;
    lastVisible = firstVisible + GUIV21_WORLD_VISIBLE_SLOTS;
    if (lastVisible > MAX_WORLD_SLOTS) { lastVisible = MAX_WORLD_SLOTS; }

    DrawDirtMenuBackground();

    glColor3f(0.05f, 0.05f, 0.05f);
    DrawCenteredText2D(fontBaseTitle, 0, 55 + 4, g_windowWidth, 120 + 4, "Select World");

    glColor3f(0.85f, 0.85f, 0.85f);
    DrawCenteredText2D(fontBaseTitle, 0, 55, g_windowWidth, 120, "Select World");

    for (i = firstVisible; i < lastVisible; i++) {
        hover = PointInRectInt(mouse.x, mouse.y, worldSlotButtons[i]);
        DrawButtonState2D(worldSlotButtons[i], "", hover, 0, i == selectedWorldSlot);

        if (worldSaves[i].exists) {
            sprintf(line, "%s", worldSaves[i].name);
            glColor3f(1.0f, 1.0f, 1.0f);
            DrawText2D(fontBaseNormal, worldSlotButtons[i].left + 18, worldSlotButtons[i].top + 21, line);
            glColor3f(0.65f, 0.65f, 0.65f);
            sprintf(line, "Slot %d", i + 1);
            DrawText2D(fontBaseNormal, worldSlotButtons[i].right - 95, worldSlotButtons[i].top + 21, line);
        } else {
            glColor3f(0.65f, 0.65f, 0.65f);
            DrawText2D(fontBaseNormal, worldSlotButtons[i].left + 18, worldSlotButtons[i].top + 31, "Empty World Slot");
        }
    }

    DrawButtonState2D(worldPlayButton, "Play Selected", PointInRectInt(mouse.x, mouse.y, worldPlayButton), !canUseSelected, 0);
    DrawButton2D(worldCreateButton, "Create New World", PointInRectInt(mouse.x, mouse.y, worldCreateButton));
    DrawButtonState2D(worldDeleteButton, "Delete", PointInRectInt(mouse.x, mouse.y, worldDeleteButton), !canUseSelected, 0);
    DrawButtonState2D(worldRenameButton, "Rename", PointInRectInt(mouse.x, mouse.y, worldRenameButton), !canUseSelected, 0);
    if (MAX_WORLD_SLOTS > 5) {
        glColor3f(0.70f, 0.70f, 0.70f);
        DrawCenteredText2D(fontBaseNormal, 0, worldBackButton.bottom + 8, g_windowWidth, worldBackButton.bottom + 30, "Mouse wheel scrolls saved worlds");
    }
    DrawButton2D(worldBackButton, "Cancel", PointInRectInt(mouse.x, mouse.y, worldBackButton));
}

void DrawTextField2D(RECT r, const char *label, const char *value, int active)
{
    int cursorX;
    int textW;
    int blink;
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText2D(fontBaseNormal, r.left, r.top - 10, label);
    DrawRect2D(r.left - 2, r.top - 2, r.right + 2, r.bottom + 2, 0.05f, 0.05f, 0.05f);
    if (active) { DrawRect2D(r.left, r.top, r.right, r.bottom, 0.22f, 0.22f, 0.22f); }
    else { DrawRect2D(r.left, r.top, r.right, r.bottom, 0.12f, 0.12f, 0.12f); }
    if (active && g_guiV21TextSelectStart >= 0) { DrawRect2D(r.left + 8, r.top + 8, r.right - 8, r.bottom - 8, 0.10f, 0.18f, 0.42f); }
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText2D(fontBaseNormal, r.left + 10, r.top + 30, value);
    blink = ((GetTickCount() / 350) & 1);
    if (active && blink) {
        textW = EstimateTextWidth(fontBaseNormal, value);
        cursorX = r.left + 12 + textW;
        if (cursorX > r.right - 8) { cursorX = r.right - 8; }
        DrawRect2D(cursorX, r.top + 8, cursorX + 2, r.bottom - 8, 1.0f, 1.0f, 1.0f);
    }
}

void DrawCreateWorld(void)
{
    POINT mouse;

    Setup2D();
    LayoutBetaMenus();

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);

    DrawDirtMenuBackground();

    glColor3f(0.05f, 0.05f, 0.05f);
    DrawCenteredText2D(fontBaseTitle, 0, 65 + 4, g_windowWidth, 130 + 4, "Create New World");

    glColor3f(0.85f, 0.85f, 0.85f);
    DrawCenteredText2D(fontBaseTitle, 0, 65, g_windowWidth, 130, "Create New World");

    {
        char sizeLabel[96];
        DrawTextField2D(createNameField, "World Name", newWorldName, createInputField == 0);
        DrawTextField2D(createSeedField, "Seed for the World Generator", newWorldSeedText, createInputField == 1);
        sprintf(sizeLabel, "World Size: %s", WorldSizeLabel(g_createWorldSizeBlocks));
        glColor3f(0.80f, 0.80f, 0.80f);
        DrawCenteredText2D(fontBaseNormal, createWorldSizeButton.left, createWorldSizeButton.top,
                           createWorldSizeButton.right, createWorldSizeButton.bottom, sizeLabel);
    }


    DrawButtonState2D(createWorldButton, "Create New World", PointInRectInt(mouse.x, mouse.y, createWorldButton), newWorldName[0] == '\0', 0);
    DrawButton2D(createCancelButton, "Cancel", PointInRectInt(mouse.x, mouse.y, createCancelButton));
}

void DrawOptions(void)
{
    POINT mouse;
    char label[96];
    Setup2D();
    LayoutBetaMenus();
    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);
    DrawDirtMenuBackground();
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseTitle, 0, 72, g_windowWidth, 120, "Options");
    DrawButton2D(optionsVideoButton, "Video Settings...", PointInRectInt(mouse.x, mouse.y, optionsVideoButton));
    DrawButton2D(optionsControlsButton, "Controls...", PointInRectInt(mouse.x, mouse.y, optionsControlsButton));
    sprintf(label, "Music: %d%%", 100);
    GuiV7_DrawSliderButton(optionsMusicSlider, label, 100, 100, PointInRectInt(mouse.x, mouse.y, optionsMusicSlider));
    sprintf(label, "Sound: %d%%", 100);
    GuiV7_DrawSliderButton(optionsSoundSlider, label, 100, 100, PointInRectInt(mouse.x, mouse.y, optionsSoundSlider));
    DrawButton2D(optionsAchievementsButton, "Achievements", PointInRectInt(mouse.x, mouse.y, optionsAchievementsButton));
    DrawButton2D(optionsStatsButton, "Statistics", PointInRectInt(mouse.x, mouse.y, optionsStatsButton));
    sprintf(label, "Render Distance: %s", RenderDistanceLabel());
    DrawButton2D(optionsRenderDistanceButton, label, PointInRectInt(mouse.x, mouse.y, optionsRenderDistanceButton));
    DrawButton2D(optionsDoneButton, "Done", PointInRectInt(mouse.x, mouse.y, optionsDoneButton));
    DrawBetaMenuFooter();
    DrawBetaVignette2D();
}


void DrawSettings(void)
{
    DrawOptions();
}

void DrawPauseMenu(void)
{
    POINT mouse;
    char label[96];

    Setup2D();
    LayoutBetaMenus();

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(g_windowWidth, 0);
    glVertex2i(g_windowWidth, g_windowHeight);
    glVertex2i(0, g_windowHeight);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseTitle, 0, 110, g_windowWidth, 170, "Game menu");

    DrawButton2D(pauseContinueButton, "Continue", PointInRectInt(mouse.x, mouse.y, pauseContinueButton));
    DrawButton2D(pauseOptionsButton, "Options...", PointInRectInt(mouse.x, mouse.y, pauseOptionsButton));
    DrawButton2D(pauseExitButton, "Save and quit", PointInRectInt(mouse.x, mouse.y, pauseExitButton));

}

void DrawDirtMenuBackground(void)
{
    int x;
    int y;
    Setup2D();
    for (y = 0; y < g_windowHeight; y += 32) {
        for (x = 0; x < g_windowWidth; x += 32) {
            if (texTerrain) {
                /* Verified dirt tile gives the classic tiled menu background and
                   avoids corrupted gui/background.png color/state issues. */
                DrawTerrainTile2D(TILE_DIRT_COL, TILE_DIRT_ROW, x, y, x + 32, y + 32);
            } else if (texBetaMenuBackground) {
                DrawImage2D(texBetaMenuBackground, x, y, x + 32, y + 32, 1.0f);
            } else {
                DrawRect2D(x, y, x + 32, y + 32, 0.28f, 0.18f, 0.10f);
            }
        }
    }
}


void DrawDeathScreen(void)
{
    POINT mouse;

    Setup2D();
    LayoutBetaMenus();

    GetCursorPos(&mouse);
    ScreenToClient(g_hwnd, &mouse);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);

    /* Converted GuiGameOver look: red/black gradient over the frozen world. */
    glBegin(GL_QUADS);
    glColor4f(0.31f, 0.00f, 0.00f, 0.38f);
    glVertex2i(0, 0);
    glVertex2i(g_windowWidth, 0);
    glColor4f(0.08f, 0.00f, 0.00f, 0.78f);
    glVertex2i(g_windowWidth, g_windowHeight);
    glVertex2i(0, g_windowHeight);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseTitle, 0, 58, g_windowWidth, 118, "Game over!");

    glColor3f(1.0f, 1.0f, 1.0f);
    DrawCenteredText2D(fontBaseNormal, 0, 155, g_windowWidth, 185, "Score: 0");

    DrawButton2D(deathRespawnButton, "Respawn", PointInRectInt(mouse.x, mouse.y, deathRespawnButton));
    DrawButton2D(deathTitleButton, "Title menu", PointInRectInt(mouse.x, mouse.y, deathTitleButton));

    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void GuiV16_DrawSelectionBorder(RECT r)
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 0.45f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(r.left - 3, r.top - 3);
    glVertex2i(r.right + 3, r.top - 3);
    glVertex2i(r.right + 3, r.bottom + 3);
    glVertex2i(r.left - 3, r.bottom + 3);
    glEnd();
}

void DrawButtonState2D(RECT r, const char *text, int hover, int disabled, int selected)
{
    DrawBetaButtonPanel(r, hover && !disabled, disabled);

    if (selected) {
        GuiV16_DrawSelectionBorder(r);
    }

    if (text && text[0] != '\0') {
        glColor3f(0.08f, 0.08f, 0.08f);
        DrawCenteredText2D(fontBaseNormal, r.left + 2, r.top + 2, r.right + 2, r.bottom + 2, text);
        if (disabled) {
            glColor3f(0.55f, 0.55f, 0.55f);
        } else if (selected) {
            glColor3f(1.0f, 1.0f, 0.35f);
        } else if (hover) {
            glColor3f(1.0f, 1.0f, 0.55f);
        } else {
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        DrawCenteredText2D(fontBaseNormal, r.left, r.top, r.right, r.bottom, text);
    }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawButton2D(RECT r, const char *text, int hover)
{
    DrawButtonState2D(r, text, hover, 0, 0);
}

int GuiV16_GetScaleFactor(void)
{
    int scale;
    int next;
    scale = 1;
    while (scale < 4) {
        next = scale + 1;
        if (g_windowWidth / next < 320) { break; }
        if (g_windowHeight / next < 240) { break; }
        scale = next;
    }
    return scale;
}

int GuiV16_GetInventoryScale(void)
{
    int scale;
    scale = 2;
    if (g_windowWidth < 500 || g_windowHeight < 390) { scale = 1; }
    g_guiV16LastInventoryScale = scale;
    return scale;
}

int GuiV16_GetSlotPixels(void)
{
    return 18 * GuiV16_GetInventoryScale();
}

int GuiV16_IsShiftDown(void)
{
    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) { return 1; }
    return 0;
}

int GuiV16_MoveStackToSlots(InventorySlot *src, InventorySlot *dst, int dstCount)
{
    return GuiV36_MoveStackToSlotArrayRole(src, dst, dstCount, GUIV21_SLOT_PLAYER_INV);
}

int GuiV16_MoveStackToPlayer(InventorySlot *src)
{
    if (GuiV16_MoveStackToSlots(src, hotbar, HOTBAR_SLOTS)) { return 1; }
    return GuiV16_MoveStackToSlots(src, inventory, INVENTORY_SLOTS);
}

int GuiV16_MoveStackToHotbarOrInventory(InventorySlot *src, int fromHotbar)
{
    if (fromHotbar) { return GuiV16_MoveStackToSlots(src, inventory, INVENTORY_SLOTS); }
    return GuiV16_MoveStackToSlots(src, hotbar, HOTBAR_SLOTS);
}

int GuiV16_MoveStackToTileContainer(InventorySlot *src, TileEntityState *t)
{
    int limit;
    int result;
    if (!src || !t) { return 0; }
    if (src->item == ITEM_NONE || src->count <= 0) { return 0; }
    if (FurnaceV36_IsFurnaceBlock(t->type)) {
        result = 0;
        if (FurnaceV36_CanSmeltInput(src->item)) { result = GuiV36_MoveStackToSlotRole(src, &t->slots[0], GUIV21_SLOT_FURNACE_INPUT, 0); }
        if (!result && FurnaceV36_FuelBurnTicks(src->item) > 0) { result = GuiV36_MoveStackToSlotRole(src, &t->slots[1], GUIV21_SLOT_FURNACE_FUEL, 1); }
        if (result) { PlayItemPickupSound(); }
        return result;
    }
    limit = TileSlotMaxV5(t->type);
    return GuiV36_MoveStackToSlotArrayRole(src, t->slots, limit, GUIV21_SLOT_TILE_NORMAL);
}

int GuiV16_ClickTakeOnlySlot(InventorySlot *slot, int rightClick)
{
    int take;
    int space;
    int moveCount;

    if (!slot) { return 0; }
    if (slot->item == ITEM_NONE || slot->count <= 0) { return 0; }

    if (!g_draggingInventory || g_dragSlot.item == ITEM_NONE || g_dragSlot.count <= 0) {
        take = slot->count;
        if (rightClick) { take = 1; }
        g_dragSlot.item = slot->item;
        g_dragSlot.count = take;
        g_dragSlot.damage = slot->damage;
        slot->count -= take;
        if (slot->count <= 0) { slot->item = ITEM_NONE; slot->count = 0; slot->damage = 0; }
        g_draggingInventory = 1;
        return 1;
    }

    if (g_dragSlot.item == slot->item && g_dragSlot.damage == slot->damage && g_dragSlot.count < ItemV21_MaxStackSize(g_dragSlot.item)) {
        space = ItemV21_MaxStackSize(g_dragSlot.item) - g_dragSlot.count;
        moveCount = slot->count;
        if (rightClick && moveCount > 1) { moveCount = 1; }
        if (moveCount > space) { moveCount = space; }
        if (moveCount <= 0) { return 0; }
        g_dragSlot.count += moveCount;
        slot->count -= moveCount;
        if (slot->count <= 0) { slot->item = ITEM_NONE; slot->count = 0; slot->damage = 0; }
        return 1;
    }

    return 0;
}

int GuiV16_QuickCraftResultToPlayer(void)
{
    InventorySlot temp;
    int loops;
    int movedAny;

    movedAny = 0;
    for (loops = 0; loops < 64; loops++) {
        UpdateCraftingResult();
        if (craftResult.item == ITEM_NONE || craftResult.count <= 0) { break; }
        temp = craftResult;
        if (!GuiV16_MoveStackToPlayer(&temp)) { break; }
        ConsumeCraftingIngredients();
        movedAny = 1;
    }
    UpdateCraftingResult();
    return movedAny;
}

void GuiV16_DrawSlotTooltip(int mx, int my)
{
    char line[80];
    int isHotbar;
    int index;
    InventorySlot *slot;
    int w;

    slot = NULL;
    index = GetInventorySlotAtPoint(mx, my, &isHotbar);
    if (index >= 0) { slot = GuiV21_GetPlayerSlotByKind(index, isHotbar); }
    if (!slot) { return; }
    if (slot->item == ITEM_NONE || slot->count <= 0) { return; }

    if (slot->count > 1) { sprintf(line, "%s x%d", ItemV21_GetDisplayName(slot->item), slot->count); }
    else { sprintf(line, "%s", ItemV21_GetDisplayName(slot->item)); }
    w = EstimateTextWidth(fontBaseNormal, line) + 16;
    DrawRect2D(mx + 12, my + 12, mx + 12 + w, my + 38, 0.02f, 0.02f, 0.02f);
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText2D(fontBaseNormal, mx + 20, my + 31, line);
}

void GuiV16_DrawTransitionOverlay(void)
{
    unsigned long nowMs;
    unsigned long elapsed;
    float alpha;

    nowMs = GetTickCount();
    if (g_guiV16LastState != g_state) {
        g_guiV16LastState = g_state;
        g_guiV16FadeStartMs = nowMs;
    }

    elapsed = nowMs - g_guiV16FadeStartMs;
    if (elapsed >= 180) { return; }
    alpha = 1.0f - ((float)elapsed / 180.0f);
    if (alpha < 0.0f) { alpha = 0.0f; }
    if (alpha > 1.0f) { alpha = 1.0f; }

    Setup2D();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, alpha * 0.55f);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(g_windowWidth, 0);
    glVertex2i(g_windowWidth, g_windowHeight);
    glVertex2i(0, g_windowHeight);
    glEnd();
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawRect2D(int x1, int y1, int x2, int y2, float r, float g, float b)
{
    glDisable(GL_TEXTURE_2D);

    glColor3f(r, g, b);

    glBegin(GL_QUADS);
    glVertex2i(x1, y1);
    glVertex2i(x2, y1);
    glVertex2i(x2, y2);
    glVertex2i(x1, y2);
    glEnd();
}

int PointInRectInt(int x, int y, RECT r)
{
    if (x >= r.left && x <= r.right &&
        y >= r.top && y <= r.bottom) {
        return 1;
    }

    return 0;
}

void DrawWeather2D(void)
{
    int i;
    int x;
    int y;
    int len;
    int mode;
    GLuint tex;
    mode = g_weatherMode;
    if (mode == 0) {
        return;
    }
    tex = texBetaRain;
    if (mode == 2) {
        tex = texBetaSnow;
    }
    Setup2D();
    if (tex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 0.34f);
        for (i = 0; i < MAX_WEATHER_PARTICLES; i++) {
            x = (WorldHash2D(i, 17, g_worldSeed + 12345) % (g_windowWidth + 160)) - 80;
            y = (WorldHash2D(i, 23, g_worldSeed + 22345) % (g_windowHeight + 160)) - 80;
            y += (int)g_weatherScroll;
            y = y % (g_windowHeight + 160);
            y -= 80;
            len = 26;
            if (mode == 2) {
                len = 10;
            }
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2i(x, y);
            glTexCoord2f(1.0f, 0.0f); glVertex2i(x + 8, y);
            glTexCoord2f(1.0f, 1.0f); glVertex2i(x + 8, y + len);
            glTexCoord2f(0.0f, 1.0f); glVertex2i(x, y + len);
            glEnd();
        }
        glDisable(GL_TEXTURE_2D);
    }
}

void RenderSkyBodies(void)
{
    float f;
    float ang;
    float sx;
    float sy;
    float sz;
    float mx;
    float my;
    float mz;
    float alphaSun;
    float alphaMoon;
    float sunSize;
    float moonSize;

    /* V41: Java RenderGlobal.renderSky reference uses a 30F sun quad and a
       20F moon quad at the sky dome.  The old C version used tiny 8-9 block
       billboards, so it looked weak.  Keep the C billboard path for OpenGL 1.1
       compatibility but match the Java scale/brightness much more closely. */
    f = (float)(g_worldTimeSeconds / DAY_LENGTH_SECONDS);
    ang = f * 6.2831853f - 1.5707963f;

    sx = (float)playerX + (float)cos(ang) * 66.0f;
    sy = (float)playerY + 72.0f + (float)sin(ang) * 62.0f;
    sz = (float)playerZ - 88.0f;
    mx = (float)playerX - (float)cos(ang) * 66.0f;
    my = (float)playerY + 72.0f - (float)sin(ang) * 62.0f;
    mz = (float)playerZ - 88.0f;

    alphaSun = 0.92f + g_dayNightBlend * 0.08f;
    alphaMoon = 0.58f + (1.0f - g_dayNightBlend) * 0.42f;
    sunSize = 32.0f;
    moonSize = 26.0f;

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, 1);

    DrawImage3DBillboard(texBetaSun, sx, sy, sz, sunSize, sunSize, alphaSun);
    DrawImage3DBillboard(texBetaMoon, mx, my, mz, moonSize, moonSize, alphaMoon);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void EnableBetaFog(void)
{
#ifdef GL_FOG
    GLfloat fogColor[4];

    if (g_state == STATE_GAME && IsPlayerHeadUnderWater()) {
        fogColor[0] = 0.02f;
        fogColor[1] = 0.04f;
        fogColor[2] = 0.24f;
        fogColor[3] = 1.0f;
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_EXP2);
        glFogf(GL_FOG_DENSITY, 0.105f);
        glFogfv(GL_FOG_COLOR, fogColor);
        return;
    }

    fogColor[0] = 0.03f + 0.42f * g_dayNightBlend;
    fogColor[1] = 0.05f + 0.65f * g_dayNightBlend;
    fogColor[2] = 0.12f + 0.88f * g_dayNightBlend;
    fogColor[3] = 1.0f;
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, (float)(g_renderDistanceChunks * CHUNK_SIZE) * 0.62f);
    glFogf(GL_FOG_END,   (float)(g_renderDistanceChunks * CHUNK_SIZE) * 0.95f);
    glFogfv(GL_FOG_COLOR, fogColor);
#endif
}


void DisableBetaFog(void)
{
#ifdef GL_FOG
    glDisable(GL_FOG);
#endif
}

/* ------------------------------------------------------------ */
/* World generation                                             */
/* ------------------------------------------------------------ */


int WorldSizeIsFiniteV55(int size)
{
    if (size > 0) { return 1; }
    return 0;
}

int WorldIsInfiniteV55(void)
{
    return !WorldSizeIsFiniteV55(g_worldSizeBlocks);
}

int IsGlobalInsideFiniteWorld(int gx, int gz)
{
    int half;

    if (WorldIsInfiniteV55()) { return 1; }

    half = g_worldSizeBlocks / 2;

    if (gx < -half || gx >= half) {
        return 0;
    }

    if (gz < -half || gz >= half) {
        return 0;
    }

    return 1;
}

int DistanceToFiniteWorldEdge(int gx, int gz)
{
    int half;
    int dx1;
    int dx2;
    int dz1;
    int dz2;
    int d;

    if (WorldIsInfiniteV55()) { return 1073741823; }

    half = g_worldSizeBlocks / 2;
    dx1 = gx + half;
    dx2 = half - 1 - gx;
    dz1 = gz + half;
    dz2 = half - 1 - gz;

    d = dx1;
    if (dx2 < d) { d = dx2; }
    if (dz1 < d) { d = dz1; }
    if (dz2 < d) { d = dz2; }

    return d;
}

int IsGlobalInBorderOcean(int gx, int gz)
{
    if (WorldIsInfiniteV55()) { return 0; }

    if (!IsGlobalInsideFiniteWorld(gx, gz)) {
        return 1;
    }

    if (DistanceToFiniteWorldEdge(gx, gz) < FINITE_BORDER_OCEAN_WIDTH) {
        return 1;
    }

    return 0;
}

void FillFiniteOceanColumn(int lx, int lz, int gx, int gz)
{
    int y;
    int h;
    int ripple;

    ripple = WorldHash2D(gx, gz, g_worldSeed + 6060) % 3;
    h = GEN_WATER_LEVEL - 9 + ripple;
    if (h < 3) { h = 3; }

    biomeMap[lx][lz] = (unsigned char)BIOME_OCEAN;
    worldHeightMap[lx][lz] = h;

    for (y = 0; y < WORLD_Y; y++) {
        if (y == 0) {
            world[lx][y][lz] = BLOCK_BORDER;
        } else if (y <= h - 3) {
            world[lx][y][lz] = BLOCK_SANDSTONE;
        } else if (y <= h) {
            world[lx][y][lz] = BLOCK_SAND;
        } else if (y <= GEN_WATER_LEVEL) {
            world[lx][y][lz] = BLOCK_WATER;
        } else {
            world[lx][y][lz] = BLOCK_AIR;
        }
    }
}

void ClampPlayerToFiniteWorld(void)
{
    double gx;
    double gz;
    double minv;
    double maxv;

    if (WorldIsInfiniteV55()) { return; }

    minv = -(double)(g_worldSizeBlocks / 2) + (double)FINITE_BORDER_CLAMP_PAD;
    maxv =  (double)(g_worldSizeBlocks / 2) - (double)(FINITE_BORDER_CLAMP_PAD + 1);

    gx = GetPlayerGlobalX();
    gz = GetPlayerGlobalZ();

    if (gx < minv) { gx = minv; }
    if (gx > maxv) { gx = maxv; }
    if (gz < minv) { gz = minv; }
    if (gz > maxv) { gz = maxv; }

    playerX = gx - (double)worldOriginBlockX;
    playerZ = gz - (double)worldOriginBlockZ;
}

const char *WorldSizeLabel(int size)
{
    if (!WorldSizeIsFiniteV55(size)) { return "Infinite"; }
    return "862 x 862";
}

void ChangeCreateWorldSize(void)
{
    if (WorldSizeIsFiniteV55(g_createWorldSizeBlocks)) {
        g_createWorldSizeBlocks = WORLD_SIZE_INFINITE;
    } else {
        g_createWorldSizeBlocks = FINITE_WORLD_SIZE_SMALL;
    }
}




