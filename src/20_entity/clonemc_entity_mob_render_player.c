/* ============================================================
   CloneMC V51 section: ENTITY / MOB MODELS / PLAYER CONTROL / CAMERA / WORLD ENTRY RENDER CALL
   ============================================================ */

                IsSolidBlock(GetBlock(testX, testY + 1, testZ))) {
                avoidX = -targetZ;
                avoidZ = targetX;

                testX = (int)floor(m->x + avoidX * 0.85);
                testZ = (int)floor(m->z + avoidZ * 0.85);

                if (!IsSolidBlock(GetBlock(testX, testY, testZ)) &&
                    !IsSolidBlock(GetBlock(testX, testY + 1, testZ))) {
                    targetX = avoidX;
                    targetZ = avoidZ;
                } else if (mobOnGround) {
                    m->vy = 4.2;
                }
            }
        }

        if (targetX != 0.0 || targetZ != 0.0) {
            if (targetKind == MOB_TARGET_FLEE_PLAYER) {
                goalX = m->x + targetX * 6.0;
                goalY = m->y;
                goalZ = m->z + targetZ * 6.0;
            } else if (targetKind == MOB_TARGET_PLAYER) {
                goalX = playerX;
                goalY = playerY;
                goalZ = playerZ;
            } else {
                goalX = m->x + targetX * 5.0;
                goalY = m->y;
                goalZ = m->z + targetZ * 5.0;
            }
            MobAIV53_PathToGoal(m, goalX, goalY, goalZ, dt, &targetX, &targetZ);
            MobAI_TryJumpHelperV17(m, targetX, targetZ, mobOnGround);
            MobApproachFacing(m, targetX, targetZ, dt, 520.0);
            m->vx += targetX * speed * dt * 3.0;
            m->vz += targetZ * speed * dt * 3.0;
        }

        if (m->type == MOB_SPIDER && mobOnGround && targetKind == MOB_TARGET_PLAYER && dist2 < 72.0 && dist2 > 3.0 && m->attackTimer <= 0.15) {
            m->vy = 5.1;
            m->vx += targetX * 1.8;
            m->vz += targetZ * 1.8;
            m->attackTimer = 0.90;
        }

        if (m->type == MOB_SLIME && mobOnGround && m->thinkTimer <= 0.15) {
            m->vy = 4.4;
            PlayMobStepSoundNear(m, blockBelow);
        }

        MobV39_ApplyFriction(m, blockBelow, dt);

        maxSpeed = speed;
        if (maxSpeed < 0.25) {
            maxSpeed = 0.25;
        }

        if (m->vx > maxSpeed) { m->vx = maxSpeed; }
        if (m->vx < -maxSpeed) { m->vx = -maxSpeed; }
        if (m->vz > maxSpeed) { m->vz = maxSpeed; }
        if (m->vz < -maxSpeed) { m->vz = -maxSpeed; }

        if (m->type == MOB_CHICKEN && m->vy < -3.0) {
            m->vy = -3.0;
        }

        if (m->inWater || m->inLava) {
            MobAI_ApplyFluidMotionV17(m, dt);
        } else if (m->type != MOB_SQUID) {
            m->vy -= GRAVITY * dt;
        }

        if (m->deathTime > 0.0) {
            continue;
        }

        MobAI_ApplyEntityPushesV24(i, m, dt);

        newX = m->x + m->vx * dt;
        newY = m->y + m->vy * dt;
        newZ = m->z + m->vz * dt;

        if (newX < 2.0 || newX > WORLD_X - 3.0) {
            m->vx = -m->vx * 0.3;
            newX = m->x;
        }
        if (newZ < 2.0 || newZ > WORLD_Z - 3.0) {
            m->vz = -m->vz * 0.3;
            newZ = m->z;
        }

        oldX = m->x;
        oldZ = m->z;
        MobV39_MoveEntity(m, newX - m->x, newY - m->y, newZ - m->z, dt, &mobOnGround);
        newX = m->x;
        newY = m->y;
        newZ = m->z;
        blockBelow = GetBlock((int)floor(m->x), (int)floor(m->y - 0.08), (int)floor(m->z));

        if (m->type == MOB_SQUID) {
            if (GetBlock((int)floor(newX), (int)floor(newY), (int)floor(newZ)) != BLOCK_WATER &&
                GetBlock((int)floor(newX), (int)floor(newY + 1.0), (int)floor(newZ)) != BLOCK_WATER) {
                if (newY > GEN_WATER_LEVEL + 1) {
                    m->y = GEN_WATER_LEVEL;
                    newY = m->y;
                }
            }
        }

        horSpeed = sqrt((newX - oldX) * (newX - oldX) + (newZ - oldZ) * (newZ - oldZ)) / (dt > 0.0001 ? dt : 0.0001);
        if (mobOnGround && horSpeed > 0.10 && m->type != MOB_SQUID) {
            if (m->stepTimer <= 0.0) {
                PlayMobStepSoundNear(m, blockBelow);
                stepDelay = 0.48;
                if (m->type == MOB_CHICKEN || m->type == MOB_SPIDER) { stepDelay = 0.28; }
                else if (m->type == MOB_ZOMBIE || m->type == MOB_SKELETON) { stepDelay = 0.55; }
                else if (m->type == MOB_WOLF) { stepDelay = 0.34; }
                else if (m->type == MOB_SLIME) { stepDelay = 0.90; }
                m->stepTimer = stepDelay;
            }
        }

        if (horSpeed > 0.012) {
            m->animWalk += horSpeed * dt * 7.2;
            MobApproachFacing(m, newX - oldX, newZ - oldZ, dt, 520.0);
            m->stuckTimer = 0.0;
        } else {
            m->vx *= 0.50;
            m->vz *= 0.50;
            if (targetKind == MOB_TARGET_PLAYER || targetKind == MOB_TARGET_FLEE_PLAYER) {
                m->stuckTimer += dt;
                if (m->stuckTimer > MOB_AI_STUCK_SECONDS) {
                    m->pathLength = 0;
                    m->pathRecalcTimer = 0.0;
                    m->stuckTimer = 0.0;
                    if (mobOnGround) { m->vy = 4.8; }
                }
            }
        }

        m->lastMoveX = newX;
        m->lastMoveZ = newZ;
        m->x = newX;
        m->y = newY;
        m->z = newZ;
    }
}


void DrawMobBillboard(Mob *m, GLuint tex, float width, float height)
{
    float cx;
    float cy;
    float cz;
    float halfW;
    float rightX;
    float rightZ;
    float yawRad;
    float bright;
    float pulse;

    if (!m || !tex) {
        return;
    }

    cx = (float)m->x;
    cy = (float)m->y;
    cz = (float)m->z;

    /* The uploaded mob PNGs are bitmap skin/model cards, so draw them larger. */
    halfW = width * 0.72f;
    if (m->type == MOB_CHICKEN) {
        halfW = 0.55f;
        height = 1.15f;
    } else if (m->type == MOB_SPIDER) {
        halfW = 1.10f;
        height = 0.90f;
    } else if (m->type == MOB_SLIME) {
        halfW = 0.80f;
        height = 1.05f;
    } else if (m->type == MOB_COW || m->type == MOB_SHEEP ||
               m->type == MOB_PIG || m->type == MOB_WOLF) {
        halfW = 0.92f;
        height = 1.35f;
    }

    yawRad = (float)(yaw * PI / 180.0);
    rightX = cos(yawRad) * halfW;
    rightZ = sin(yawRad) * halfW;

    bright = ApplyGammaBoost(0.68f + g_daySkyBrightness * 0.42f);
    if (bright > 1.0f) {
        bright = 1.0f;
    }

    if (m->burning) {
        pulse = (float)(0.75 + 0.25 * sin(g_worldTimeSeconds * 18.0));
        glColor4f(1.0f, pulse * 0.60f, pulse * 0.30f, 1.0f);
    } else {
        glColor4f(bright, bright, bright, 1.0f);
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    /* Existing TGA loader converts to top-left row order, so this UV orientation is upright. */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(cx - rightX, cy, cz - rightZ);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(cx + rightX, cy, cz + rightZ);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(cx + rightX, cy + height, cz + rightZ);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(cx - rightX, cy + height, cz - rightZ);
    glEnd();

    /* Cross-card second plane, like old sprite vegetation, so mobs remain visible from side angles. */
    rightX = -sin(yawRad) * halfW;
    rightZ =  cos(yawRad) * halfW;
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(cx - rightX, cy, cz - rightZ);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(cx + rightX, cy, cz + rightZ);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(cx + rightX, cy + height, cz + rightZ);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(cx - rightX, cy + height, cz - rightZ);
    glEnd();
}

void DrawMobShadow(Mob *m)
{
    int gx;
    int gz;
    int gy;
    float x;
    float z;
    float y;
    float size;
    if (!m) {
        return;
    }
    gx = (int)floor(m->x);
    gz = (int)floor(m->z);
    if (gx < 1 || gx >= WORLD_X - 1 || gz < 1 || gz >= WORLD_Z - 1) {
        return;
    }
    gy = (int)floor(m->y);
    while (gy > 1 && !IsSolidBlock(GetBlock(gx, gy - 1, gz))) {
        gy--;
    }
    y = (float)gy + 0.025f;
    x = (float)m->x;
    z = (float)m->z;
    size = MobWidth(m->type) * 0.60f;
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (texBetaShadow) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texBetaShadow);
        glColor4f(1.0f, 1.0f, 1.0f, 0.42f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.0f, 0.0f, 0.0f, 0.32f);
    }
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x - size, y, z - size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + size, y, z - size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + size, y, z + size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x - size, y, z + size);
    glEnd();
}



void MobSkinUV(int px, int py, int pw, int ph, float *u0, float *v0, float *u1, float *v1)
{
    float pad;

    pad = 0.30f;
    *u0 = ((float)px + pad) / 64.0f;
    *v0 = ((float)py + pad) / 32.0f;
    *u1 = ((float)(px + pw) - pad) / 64.0f;
    *v1 = ((float)(py + ph) - pad) / 32.0f;
}

void EmitMobSkinQuad(float x0, float y0, float z0,
                     float x1, float y1, float z1,
                     float x2, float y2, float z2,
                     float x3, float y3, float z3,
                     int px, int py, int pw, int ph)
{
    float u0;
    float v0;
    float u1;
    float v1;

    MobSkinUV(px, py, pw, ph, &u0, &v0, &u1, &v1);

    glTexCoord2f(u0, v1); glVertex3f(x0, y0, z0);
    glTexCoord2f(u1, v1); glVertex3f(x1, y1, z1);
    glTexCoord2f(u1, v0); glVertex3f(x2, y2, z2);
    glTexCoord2f(u0, v0); glVertex3f(x3, y3, z3);
}

void DrawMobSkinBoxPart(float cx, float cy, float cz,
                        float sx, float sy, float sz,
                        GLuint tex, float shade, float alpha,
                        int tx, int ty, int pw, int ph, int pd)
{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;

    if (!tex) {
        return;
    }

    x0 = cx - sx * 0.5f;
    x1 = cx + sx * 0.5f;
    y0 = cy - sy * 0.5f;
    y1 = cy + sy * 0.5f;
    z0 = cz - sz * 0.5f;
    z1 = cz + sz * 0.5f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor4f(shade * g_mobTintR, shade * g_mobTintG, shade * g_mobTintB, alpha);

    glBegin(GL_QUADS);

    EmitMobSkinQuad(x0, y0, z0, x1, y0, z0, x1, y1, z0, x0, y1, z0,
                    tx + pd, ty + pd, pw, ph);
    EmitMobSkinQuad(x1, y0, z1, x0, y0, z1, x0, y1, z1, x1, y1, z1,
                    tx + pd + pw + pd, ty + pd, pw, ph);
    EmitMobSkinQuad(x0, y0, z1, x0, y0, z0, x0, y1, z0, x0, y1, z1,
                    tx, ty + pd, pd, ph);
    EmitMobSkinQuad(x1, y0, z0, x1, y0, z1, x1, y1, z1, x1, y1, z0,
                    tx + pd + pw, ty + pd, pd, ph);
    EmitMobSkinQuad(x0, y1, z0, x1, y1, z0, x1, y1, z1, x0, y1, z1,
                    tx + pd, ty, pw, pd);
    EmitMobSkinQuad(x0, y0, z1, x1, y0, z1, x1, y0, z0, x0, y0, z0,
                    tx + pd + pw, ty, pw, pd);

    glEnd();
}


void DrawMobSkinBoxPartRot(float px, float py, float pz,
                           float cx, float cy, float cz,
                           float sx, float sy, float sz,
                           GLuint tex, float shade, float alpha,
                           int tx, int ty, int pw, int ph, int pd,
                           float rotX, float rotY, float rotZ)
{
    glPushMatrix();
    glTranslatef(px, py, pz);
    if (rotZ != 0.0f) { glRotatef(rotZ, 0.0f, 0.0f, 1.0f); }
    if (rotY != 0.0f) { glRotatef(rotY, 0.0f, 1.0f, 0.0f); }
    if (rotX != 0.0f) { glRotatef(rotX, 1.0f, 0.0f, 0.0f); }
    DrawMobSkinBoxPart(cx, cy, cz, sx, sy, sz, tex, shade, alpha, tx, ty, pw, ph, pd);
    glPopMatrix();
}


/* ------------------------------------------------------------ */
/* Java ModelRenderer/TexturedQuad/PositionTextureVertex port    */
/* ------------------------------------------------------------ */
/*
   This block converts the behavior of the uploaded Java
   ModelRenderer.java, TexturedQuad.java, and PositionTextureVertex.java
   into immediate-mode OpenGL 1.1 C.  It intentionally avoids display
   lists/VBOs so the result stays simple for Open Watcom and Windows 98.
   The important behavior reproduced here is:
     - addBox vertex order
     - per-face UV rectangles from textureOffsetX/Y
     - mirror flipping by swapping X bounds and reversing face vertices
     - render() transform order: translate, rotate Z, rotate Y, rotate X
     - 64x32 mob skin UV padding matching TexturedQuad.java
*/

typedef struct JavaPTV_C {
    float x;
    float y;
    float z;
    float u;
    float v;
} JavaPTV_C;

typedef struct JavaQuad_C {
    JavaPTV_C v[4];
} JavaQuad_C;

JavaPTV_C JavaPTV_Make(float x, float y, float z, float u, float v)
{
    JavaPTV_C p;
    p.x = x;
    p.y = y;
    p.z = z;
    p.u = u;
    p.v = v;
    return p;
}

JavaPTV_C JavaPTV_SetTexturePosition(JavaPTV_C p, float u, float v)
{
    p.u = u;
    p.v = v;
    return p;
}

void JavaQuad_SetTextureRect(JavaQuad_C *q, int left, int top, int right, int bottom)
{
    float f;
    float f1;
    if (!q) { return; }
    f = 0.0015625f;
    f1 = 0.003125f;
    q->v[0] = JavaPTV_SetTexturePosition(q->v[0], (float)right / 64.0f - f, (float)top / 32.0f + f1);
    q->v[1] = JavaPTV_SetTexturePosition(q->v[1], (float)left / 64.0f + f, (float)top / 32.0f + f1);
    q->v[2] = JavaPTV_SetTexturePosition(q->v[2], (float)left / 64.0f + f, (float)bottom / 32.0f - f1);
    q->v[3] = JavaPTV_SetTexturePosition(q->v[3], (float)right / 64.0f - f, (float)bottom / 32.0f - f1);
}

void JavaQuad_FlipFace(JavaQuad_C *q)
{
    JavaPTV_C t;
    if (!q) { return; }
    t = q->v[0]; q->v[0] = q->v[3]; q->v[3] = t;
    t = q->v[1]; q->v[1] = q->v[2]; q->v[2] = t;
}

void JavaQuad_Draw(JavaQuad_C *q)
{
    int i;
    if (!q) { return; }
    for (i = 0; i < 4; i++) {
        glTexCoord2f(q->v[i].u, q->v[i].v);
        glVertex3f(q->v[i].x, q->v[i].y, q->v[i].z);
    }
}

void JavaModel_DrawBoxImmediate(int texOffX, int texOffY,
                                float x, float y, float z,
                                int w, int h, int d,
                                float inflate, int mirror)
{
    JavaPTV_C p0;
    JavaPTV_C p1;
    JavaPTV_C p2;
    JavaPTV_C p3;
    JavaPTV_C p4;
    JavaPTV_C p5;
    JavaPTV_C p6;
    JavaPTV_C p7;
    JavaQuad_C q[6];
    float x2;
    float y2;
    float z2;
    float tmp;
    int i;

    x2 = x + (float)w;
    y2 = y + (float)h;
    z2 = z + (float)d;
    x -= inflate;
    y -= inflate;
    z -= inflate;
    x2 += inflate;
    y2 += inflate;
    z2 += inflate;

    if (mirror) {
        tmp = x2;
        x2 = x;
        x = tmp;
    }

    p0 = JavaPTV_Make(x,  y,  z,  0.0f, 0.0f);
    p1 = JavaPTV_Make(x2, y,  z,  0.0f, 8.0f);
    p2 = JavaPTV_Make(x2, y2, z,  8.0f, 8.0f);
    p3 = JavaPTV_Make(x,  y2, z,  8.0f, 0.0f);
    p4 = JavaPTV_Make(x,  y,  z2, 0.0f, 0.0f);
    p5 = JavaPTV_Make(x2, y,  z2, 0.0f, 8.0f);
    p6 = JavaPTV_Make(x2, y2, z2, 8.0f, 8.0f);
    p7 = JavaPTV_Make(x,  y2, z2, 8.0f, 0.0f);

    q[0].v[0] = p5; q[0].v[1] = p1; q[0].v[2] = p2; q[0].v[3] = p6;
    JavaQuad_SetTextureRect(&q[0], texOffX + d + w,     texOffY + d, texOffX + d + w + d,     texOffY + d + h);

    q[1].v[0] = p0; q[1].v[1] = p4; q[1].v[2] = p7; q[1].v[3] = p3;
    JavaQuad_SetTextureRect(&q[1], texOffX + 0,         texOffY + d, texOffX + d,             texOffY + d + h);

    q[2].v[0] = p5; q[2].v[1] = p4; q[2].v[2] = p0; q[2].v[3] = p1;
    JavaQuad_SetTextureRect(&q[2], texOffX + d,         texOffY + 0, texOffX + d + w,         texOffY + d);

    q[3].v[0] = p2; q[3].v[1] = p3; q[3].v[2] = p7; q[3].v[3] = p6;
    JavaQuad_SetTextureRect(&q[3], texOffX + d + w,     texOffY + 0, texOffX + d + w + w,     texOffY + d);

    q[4].v[0] = p1; q[4].v[1] = p0; q[4].v[2] = p3; q[4].v[3] = p2;
    JavaQuad_SetTextureRect(&q[4], texOffX + d,         texOffY + d, texOffX + d + w,         texOffY + d + h);

    q[5].v[0] = p4; q[5].v[1] = p5; q[5].v[2] = p6; q[5].v[3] = p7;
    JavaQuad_SetTextureRect(&q[5], texOffX + d + w + d, texOffY + d, texOffX + d + w + d + w, texOffY + d + h);

    if (mirror) {
        for (i = 0; i < 6; i++) {
            JavaQuad_FlipFace(&q[i]);
        }
    }

    for (i = 0; i < 6; i++) {
        JavaQuad_Draw(&q[i]);
    }
}

void JavaModel_RenderPart(GLuint tex, float shade, float alpha,
                          int texOffX, int texOffY,
                          float boxX, float boxY, float boxZ,
                          int boxW, int boxH, int boxD,
                          float inflate, int mirror,
                          float rotPointX, float rotPointY, float rotPointZ,
                          float rotX, float rotY, float rotZ)
{
    if (!tex) { return; }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor4f(shade * g_mobTintR, shade * g_mobTintG, shade * g_mobTintB, alpha);
    glPushMatrix();
    glTranslatef(rotPointX, rotPointY, rotPointZ);
    if (rotZ != 0.0f) { glRotatef(rotZ * 57.29578f, 0.0f, 0.0f, 1.0f); }
    if (rotY != 0.0f) { glRotatef(rotY * 57.29578f, 0.0f, 1.0f, 0.0f); }
    if (rotX != 0.0f) { glRotatef(rotX * 57.29578f, 1.0f, 0.0f, 0.0f); }
    glBegin(GL_QUADS);
    JavaModel_DrawBoxImmediate(texOffX, texOffY, boxX, boxY, boxZ, boxW, boxH, boxD, inflate, mirror);
    glEnd();
    glPopMatrix();
}

void DrawJavaModelBoxV4(float rpX, float rpY, float rpZ,
                        float boxX, float boxY, float boxZ,
                        float boxW, float boxH, float boxD,
                        GLuint tex, float shade, float alpha,
                        int tx, int ty, int tw, int th, int td,
                        float rotX, float rotY, float rotZ)
{
    (void)tw;
    (void)th;
    (void)td;
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 0.0f);
    glScalef(0.0625f, -0.0625f, 0.0625f);
    JavaModel_RenderPart(tex, shade, alpha, tx, ty,
                         boxX, boxY, boxZ,
                         (int)boxW, (int)boxH, (int)boxD,
                         0.0f, 0,
                         rpX, rpY, rpZ,
                         rotX * 0.0174532925f,
                         rotY * 0.0174532925f,
                         rotZ * 0.0174532925f);
    glPopMatrix();
}

float JavaModel_GetWalkAmount(Mob *m)
{
    float walkAmount;
    if (!m) { return 0.0f; }
    walkAmount = (float)(sqrt(m->vx * m->vx + m->vz * m->vz) / 1.35);
    if (walkAmount > 1.0f) { walkAmount = 1.0f; }
    if (walkAmount < 0.04f) { walkAmount = 0.0f; }
    if (m->type == 0) {
        walkAmount = g_playerWalkAmount > 0.04 ? (float)g_playerWalkAmount : 0.0f;
    }
    if (m->targetKind == MOB_TARGET_FLEE_PLAYER && walkAmount < 0.82f) { walkAmount = 0.82f; }
    if (m->targetKind == MOB_TARGET_PLAYER && walkAmount < 0.58f) { walkAmount = 0.58f; }
    if (m->hurtTime > 0.0 && walkAmount < 0.66f) { walkAmount = 0.66f; }
    if (walkAmount > 1.15f) { walkAmount = 1.15f; }
    return walkAmount;
}

void JavaModel_BeginEntity(void)
{
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 0.0f);
    glScalef(0.0625f, -0.0625f, 0.0625f);
}

void JavaModel_EndEntity(void)
{
    glPopMatrix();
}

void JavaModel_RenderBipedCore(Mob *m, GLuint tex, float shade, float alpha,
                               int skeletonLike, int heldItem, float headPitch,
                               int zombieArms, float inflate, int drawHeadwear)
{
    float walk;
    float walkAmount;
    float idle;
    float headX;
    float headY;
    float armRX;
    float armLX;
    float armRZ;
    float armLZ;
    float legRX;
    float legLX;
    float bodyY;
    float armY;
    float legY;
    int armW;
    int armD;
    int legW;
    int legD;
    float armBoxXRight;
    float armBoxXLeft;
    float armBoxZ;
    float legBoxX;
    float legBoxZ;
    float swing;

    if (!m || !tex) { return; }
    walk = (float)m->animWalk;
    walkAmount = JavaModel_GetWalkAmount(m);
    idle = (float)g_worldTimeSeconds;
    headX = headPitch / 57.29578f;
    if (headX > 1.309f) { headX = 1.309f; }
    if (headX < -1.309f) { headX = -1.309f; }
    headY = 0.0f;

    armRX = (float)cos((double)walk * 0.6662 + PI) * 2.0f * walkAmount * 0.5f;
    armLX = (float)cos((double)walk * 0.6662) * 2.0f * walkAmount * 0.5f;
    legRX = (float)cos((double)walk * 0.6662) * 1.4f * walkAmount;
    legLX = (float)cos((double)walk * 0.6662 + PI) * 1.4f * walkAmount;
    armRZ = (float)cos((double)idle * 0.09) * 0.05f + 0.05f;
    armLZ = -((float)cos((double)idle * 0.09) * 0.05f + 0.05f);
    armRX += (float)sin((double)idle * 0.067) * 0.05f;
    armLX -= (float)sin((double)idle * 0.067) * 0.05f;

    if (zombieArms) {
        armRX = -1.570796f;
        armLX = -1.570796f;
        armRZ = (float)cos((double)idle * 0.09) * 0.05f + 0.05f;
        armLZ = -armRZ;
    }

    bodyY = 0.0f;
    armY = 2.0f;
    legY = 12.0f;
    armW = skeletonLike ? 2 : 4;
    armD = skeletonLike ? 2 : 4;
    legW = skeletonLike ? 2 : 4;
    legD = skeletonLike ? 2 : 4;
    armBoxXRight = skeletonLike ? -1.0f : -3.0f;
    armBoxXLeft = skeletonLike ? -1.0f : -1.0f;
    armBoxZ = skeletonLike ? -1.0f : -2.0f;
    legBoxX = skeletonLike ? -1.0f : -2.0f;
    legBoxZ = skeletonLike ? -1.0f : -2.0f;

    if (m->hurtTime > 0.0) { shade = 1.0f; }

    JavaModel_RenderPart(tex, shade, alpha, 0, 0,
                         -4.0f, -8.0f, -4.0f,
                         8, 8, 8, inflate, 0,
                         0.0f, bodyY, 0.0f, headX, headY, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.96f, alpha, 16, 16,
                         -4.0f, 0.0f, -2.0f,
                         8, 12, 4, inflate, 0,
                         0.0f, bodyY, 0.0f, 0.0f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.92f, alpha, 40, 16,
                         armBoxXRight, -2.0f, armBoxZ,
                         armW, 12, armD, inflate, 0,
                         -5.0f, armY, 0.0f, armRX, 0.0f, armRZ);
    JavaModel_RenderPart(tex, shade * 0.92f, alpha, 40, 16,
                         armBoxXLeft, -2.0f, armBoxZ,
                         armW, 12, armD, inflate, 1,
                         5.0f, armY, 0.0f, armLX, 0.0f, armLZ);
    JavaModel_RenderPart(tex, shade * 0.86f, alpha, 0, 16,
                         legBoxX, 0.0f, legBoxZ,
                         legW, 12, legD, inflate, 0,
                         -2.0f, legY, 0.0f, legRX, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.86f, alpha, 0, 16,
                         legBoxX, 0.0f, legBoxZ,
                         legW, 12, legD, inflate, 1,
                         2.0f, legY, 0.0f, legLX, 0.0f, 0.0f);

    if (drawHeadwear && inflate <= 0.001f) {
        JavaModel_RenderPart(tex, shade, alpha * 0.72f, 32, 0,
                             -4.0f, -8.0f, -4.0f,
                             8, 8, 8, 0.5f, 0,
                             0.0f, bodyY, 0.0f, headX, headY, 0.0f);
    }

    if (heldItem != ITEM_NONE) {
        swing = handSwingTimer > 0.0 ? (float)(handSwingTimer / HAND_SWING_TIME) : 0.0f;
        RenderItemCombatV6_Cube(0.56, 1.05 - (double)(0.10f * swing), -0.28, 0.18, 0.18, 0.55, 0.85f, 0.85f, 0.75f);
    }
}

void JavaModel_RenderQuadrupedParts(GLuint tex, float shade, float alpha,
                                    int headTexX, int headTexY,
                                    float headBoxX, float headBoxY, float headBoxZ,
                                    int headW, int headH, int headD, float headInflate,
                                    int bodyTexX, int bodyTexY,
                                    float bodyBoxX, float bodyBoxY, float bodyBoxZ,
                                    int bodyW, int bodyH, int bodyD, float bodyInflate,
                                    int legTexX, int legTexY,
                                    int legH, float legInflate,
                                    float leg1x, float leg1z,
                                    float leg2x, float leg2z,
                                    float leg3x, float leg3z,
                                    float leg4x, float leg4z,
                                    float walk, float walkAmount,
                                    float headRotX, float headRotY)
{
    float bodyRot;
    float leg1Rot;
    float leg2Rot;
    float leg3Rot;
    float leg4Rot;
    float headPointY;
    float headPointZ;
    float bodyPointY;
    float bodyPointZ;
    float legPointY;

    bodyRot = 1.570796f;
    leg1Rot = (float)cos((double)walk * 0.6662) * 1.4f * walkAmount;
    leg2Rot = (float)cos((double)walk * 0.6662 + PI) * 1.4f * walkAmount;
    leg3Rot = leg2Rot;
    leg4Rot = leg1Rot;

    /* Default ModelQuadruped.java rotation points.  The older V23 conversion
       passed Java box coordinates as if they were world-space origins; that
       made passive animals too tall/wide and shifted their heads/legs. */
    headPointY = 18.0f - (float)legH;
    headPointZ = -6.0f;
    bodyPointY = 17.0f - (float)legH;
    bodyPointZ = 2.0f;
    legPointY = 24.0f - (float)legH;

    JavaModel_RenderPart(tex, shade, alpha, headTexX, headTexY,
                         headBoxX, headBoxY, headBoxZ, headW, headH, headD, headInflate, 0,
                         0.0f, headPointY, headPointZ, headRotX, headRotY, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.96f, alpha, bodyTexX, bodyTexY,
                         bodyBoxX, bodyBoxY, bodyBoxZ, bodyW, bodyH, bodyD, bodyInflate, 0,
                         0.0f, bodyPointY, bodyPointZ, bodyRot, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, legTexX, legTexY,
                         -2.0f, 0.0f, -2.0f, 4, legH, 4, legInflate, 0,
                         leg1x, legPointY, leg1z, leg1Rot, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, legTexX, legTexY,
                         -2.0f, 0.0f, -2.0f, 4, legH, 4, legInflate, 0,
                         leg2x, legPointY, leg2z, leg2Rot, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, legTexX, legTexY,
                         -2.0f, 0.0f, -2.0f, 4, legH, 4, legInflate, 0,
                         leg3x, legPointY, leg3z, leg3Rot, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, legTexX, legTexY,
                         -2.0f, 0.0f, -2.0f, 4, legH, 4, legInflate, 0,
                         leg4x, legPointY, leg4z, leg4Rot, 0.0f, 0.0f);
}

void JavaModel_RenderCow(Mob *m, GLuint tex, float shade, float alpha)
{
    float walk;
    float walkAmount;
    float headRotX;
    float headRotY;
    float l1;
    float l2;
    float l3;
    float l4;

    walk = m ? (float)m->animWalk : 0.0f;
    walkAmount = JavaModel_GetWalkAmount(m);
    headRotX = 0.0f;
    headRotY = 0.0f;
    l1 = (float)cos((double)walk * 0.6662) * 1.4f * walkAmount;
    l2 = (float)cos((double)walk * 0.6662 + PI) * 1.4f * walkAmount;
    l3 = l2;
    l4 = l1;

    /* ModelCow.java exact pixel dimensions/rotation points. */
    JavaModel_RenderPart(tex, shade, alpha, 0, 0,
                         -4.0f, -4.0f, -6.0f, 8, 8, 6, 0.0f, 0,
                         0.0f, 4.0f, -8.0f, headRotX, headRotY, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.96f, alpha, 18, 4,
                         -6.0f, -10.0f, -7.0f, 12, 18, 10, 0.0f, 0,
                         0.0f, 5.0f, 2.0f, 1.570796f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, 0, 16,
                         -2.0f, 0.0f, -2.0f, 4, 12, 4, 0.0f, 0,
                         -4.0f, 12.0f, 7.0f, l1, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, 0, 16,
                         -2.0f, 0.0f, -2.0f, 4, 12, 4, 0.0f, 0,
                         4.0f, 12.0f, 7.0f, l2, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, 0, 16,
                         -2.0f, 0.0f, -2.0f, 4, 12, 4, 0.0f, 0,
                         -4.0f, 12.0f, -6.0f, l3, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, 0, 16,
                         -2.0f, 0.0f, -2.0f, 4, 12, 4, 0.0f, 0,
                         4.0f, 12.0f, -6.0f, l4, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade, alpha, 22, 0,
                         -4.0f, -5.0f, -4.0f, 1, 3, 1, 0.0f, 0,
                         0.0f, 3.0f, -7.0f, headRotX, headRotY, 0.0f);
    JavaModel_RenderPart(tex, shade, alpha, 22, 0,
                         3.0f, -5.0f, -4.0f, 1, 3, 1, 0.0f, 0,
                         0.0f, 3.0f, -7.0f, headRotX, headRotY, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.90f, alpha, 52, 0,
                         -2.0f, -3.0f, 0.0f, 4, 6, 2, 0.0f, 0,
                         0.0f, 14.0f, 6.0f, 1.570796f, 0.0f, 0.0f);
}

void JavaModel_RenderPig(Mob *m, GLuint tex, float shade, float alpha)
{
    float walk;
    float walkAmount;
    walk = m ? (float)m->animWalk : 0.0f;
    walkAmount = JavaModel_GetWalkAmount(m);
    JavaModel_RenderQuadrupedParts(tex, shade, alpha,
                                   0, 0, -4.0f, -4.0f, -8.0f, 8, 8, 8, 0.0f,
                                   28, 8, -5.0f, -10.0f, -7.0f, 10, 16, 8, 0.0f,
                                   0, 16, 6, 0.0f,
                                   -3.0f, 7.0f, 3.0f, 7.0f, -3.0f, -5.0f, 3.0f, -5.0f,
                                   walk, walkAmount, 0.0f, 0.0f);
}

void JavaModel_RenderSheepBase(Mob *m, GLuint tex, float shade, float alpha)
{
    float walk;
    float walkAmount;
    walk = m ? (float)m->animWalk : 0.0f;
    walkAmount = JavaModel_GetWalkAmount(m);
    /* ModelSheep2.java-style sheared sheep body/head.  V29 renders this once
       through the corrected ModelQuadruped rotation-point path.  Earlier V23
       drew explicit head/body and then the quadruped again, which made sheep
       look doubled and oversized. */
    JavaModel_RenderQuadrupedParts(tex, shade, alpha,
                                   0, 0, -3.0f, -4.0f, -6.0f, 6, 6, 8, 0.0f,
                                   28, 8, -4.0f, -10.0f, -7.0f, 8, 16, 6, 0.0f,
                                   0, 16, 12, 0.0f,
                                   -3.0f, 7.0f, 3.0f, 7.0f, -3.0f, -5.0f, 3.0f, -5.0f,
                                   walk, walkAmount, 0.0f, 0.0f);
}

void JavaModel_RenderSheepFur(Mob *m, GLuint tex, float shade, float alpha)
{
    float walk;
    float walkAmount;
    walk = m ? (float)m->animWalk : 0.0f;
    walkAmount = JavaModel_GetWalkAmount(m);
    JavaModel_RenderQuadrupedParts(tex, shade, alpha,
                                   0, 0, -3.0f, -4.0f, -4.0f, 6, 6, 6, 0.60f,
                                   28, 8, -4.0f, -10.0f, -7.0f, 8, 16, 6, 1.75f,
                                   0, 16, 6, 0.50f,
                                   -3.0f, 7.0f, 3.0f, 7.0f, -3.0f, -5.0f, 3.0f, -5.0f,
                                   walk, walkAmount, 0.0f, 0.0f);
}

void JavaModel_RenderChicken(Mob *m, GLuint tex, float shade, float alpha)
{
    float walk;
    float walkAmount;
    float wing;
    float headRotX;
    float headRotY;
    walk = m ? (float)m->animWalk : 0.0f;
    walkAmount = JavaModel_GetWalkAmount(m);
    wing = (float)((sin((double)walk * 1.7) + 1.0) * (0.35 + 0.45 * walkAmount));
    headRotX = 0.0f;
    headRotY = 0.0f;
    JavaModel_RenderPart(tex, shade, alpha, 0, 0, -2.0f, -6.0f, -2.0f, 4, 6, 3, 0.0f, 0, 0.0f, 15.0f, -4.0f, headRotX, headRotY, 0.0f);
    JavaModel_RenderPart(tex, shade, alpha, 14, 0, -2.0f, -4.0f, -4.0f, 4, 2, 2, 0.0f, 0, 0.0f, 15.0f, -4.0f, headRotX, headRotY, 0.0f);
    JavaModel_RenderPart(tex, shade, alpha, 14, 4, -1.0f, -2.0f, -3.0f, 2, 2, 2, 0.0f, 0, 0.0f, 15.0f, -4.0f, headRotX, headRotY, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.96f, alpha, 0, 9, -3.0f, -4.0f, -3.0f, 6, 8, 6, 0.0f, 0, 0.0f, 16.0f, 0.0f, 1.570796f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.78f, alpha, 26, 0, -1.0f, 0.0f, -3.0f, 3, 5, 3, 0.0f, 0, -2.0f, 19.0f, 1.0f, (float)cos((double)walk * 0.6662) * 1.4f * walkAmount, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.78f, alpha, 26, 0, -1.0f, 0.0f, -3.0f, 3, 5, 3, 0.0f, 0, 1.0f, 19.0f, 1.0f, (float)cos((double)walk * 0.6662 + PI) * 1.4f * walkAmount, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.92f, alpha, 24, 13, 0.0f, 0.0f, -3.0f, 1, 4, 6, 0.0f, 0, -4.0f, 13.0f, 0.0f, 0.0f, 0.0f, wing);
    JavaModel_RenderPart(tex, shade * 0.92f, alpha, 24, 13, -1.0f, 0.0f, -3.0f, 1, 4, 6, 0.0f, 0, 4.0f, 13.0f, 0.0f, 0.0f, 0.0f, -wing);
}

void JavaModel_RenderSpider(Mob *m, GLuint tex, float shade, float alpha)
{
    float walk;
    float walkAmount;
    float f6;
    float f7;
    float f8;
    float f9;
    float f10;
    float f11;
    float f12;
    float f13;
    float f14;
    float f15;
    float f16;
    float ly[8];
    float lz[8];
    float rpX;
    float rpZ;
    float bx;
    int i;
    walk = m ? (float)m->animWalk : 0.0f;
    walkAmount = JavaModel_GetWalkAmount(m);
    f6 = 0.7853982f;
    ly[0] = 0.3926991f * 2.0f; ly[1] = -0.3926991f * 2.0f;
    ly[2] = 0.3926991f;        ly[3] = -0.3926991f;
    ly[4] = -0.3926991f;       ly[5] = 0.3926991f;
    ly[6] = -0.3926991f * 2.0f;ly[7] = 0.3926991f * 2.0f;
    lz[0] = -f6; lz[1] = f6; lz[2] = -f6 * 0.74f; lz[3] = f6 * 0.74f;
    lz[4] = -f6 * 0.74f; lz[5] = f6 * 0.74f; lz[6] = -f6; lz[7] = f6;
    f9  = -((float)cos((double)walk * 0.6662 * 2.0 + 0.0) * 0.4f) * walkAmount;
    f10 = -((float)cos((double)walk * 0.6662 * 2.0 + PI) * 0.4f) * walkAmount;
    f11 = -((float)cos((double)walk * 0.6662 * 2.0 + PI * 0.5) * 0.4f) * walkAmount;
    f12 = -((float)cos((double)walk * 0.6662 * 2.0 + PI * 1.5) * 0.4f) * walkAmount;
    f13 = (float)fabs(sin((double)walk * 0.6662 + 0.0) * 0.4) * walkAmount;
    f14 = (float)fabs(sin((double)walk * 0.6662 + PI) * 0.4) * walkAmount;
    f15 = (float)fabs(sin((double)walk * 0.6662 + PI * 0.5) * 0.4) * walkAmount;
    f16 = (float)fabs(sin((double)walk * 0.6662 + PI * 1.5) * 0.4) * walkAmount;
    ly[0] += f9;  ly[1] += -f9;
    ly[2] += f10; ly[3] += -f10;
    ly[4] += f11; ly[5] += -f11;
    ly[6] += f12; ly[7] += -f12;
    lz[0] += f13; lz[1] += -f13;
    lz[2] += f14; lz[3] += -f14;
    lz[4] += f15; lz[5] += -f15;
    lz[6] += f16; lz[7] += -f16;

    JavaModel_RenderPart(tex, shade, alpha, 32, 4, -4.0f, -4.0f, -8.0f, 8, 8, 8, 0.0f, 0, 0.0f, 15.0f, -3.0f, 0.0f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.96f, alpha, 0, 0, -3.0f, -3.0f, -3.0f, 6, 6, 6, 0.0f, 0, 0.0f, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade, alpha, 0, 12, -5.0f, -4.0f, -6.0f, 10, 8, 12, 0.0f, 0, 0.0f, 15.0f, 9.0f, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < 8; i++) {
        rpX = (i & 1) ? 4.0f : -4.0f;
        rpZ = 2.0f;
        if (i == 2 || i == 3) { rpZ = 1.0f; }
        if (i == 4 || i == 5) { rpZ = 0.0f; }
        if (i == 6 || i == 7) { rpZ = -1.0f; }
        bx = (i & 1) ? -1.0f : -15.0f;
        JavaModel_RenderPart(tex, shade * 0.78f, alpha, 18, 0,
                             bx, -1.0f, -1.0f, 16, 2, 2, 0.0f, 0,
                             rpX, 15.0f, rpZ, 0.0f, ly[i], lz[i]);
    }
}

void JavaModel_RenderWolf(Mob *m, GLuint tex, float shade, float alpha)
{
    float walk;
    float walkAmount;
    float tailYaw;
    float tailPitch;
    float headX;
    float headY;
    walk = m ? (float)m->animWalk : 0.0f;
    walkAmount = JavaModel_GetWalkAmount(m);
    if (m && m->angry == 1) { tailYaw = 0.0f; }
    else { tailYaw = (float)cos((double)walk * 0.6662) * 1.4f * walkAmount; }
    tailPitch = 0.65f;
    headX = 0.0f;
    headY = 0.0f;
    JavaModel_RenderPart(tex, shade, alpha, 0, 0, -3.0f, -3.0f, -2.0f, 6, 6, 4, 0.0f, 0, -1.0f, 13.5f, -7.0f, headX, headY, 0.0f);
    JavaModel_RenderPart(tex, shade, alpha, 16, 14, -3.0f, -5.0f, 0.0f, 2, 2, 1, 0.0f, 0, -1.0f, 13.5f, -7.0f, headX, headY, 0.0f);
    JavaModel_RenderPart(tex, shade, alpha, 16, 14, 1.0f, -5.0f, 0.0f, 2, 2, 1, 0.0f, 0, -1.0f, 13.5f, -7.0f, headX, headY, 0.0f);
    JavaModel_RenderPart(tex, shade, alpha, 0, 10, -2.0f, 0.0f, -5.0f, 3, 3, 4, 0.0f, 0, -0.5f, 13.5f, -7.0f, headX, headY, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.96f, alpha, 18, 14, -4.0f, -2.0f, -3.0f, 6, 9, 6, 0.0f, 0, 0.0f, 14.0f, 2.0f, 1.570796f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade, alpha, 21, 0, -4.0f, -3.0f, -3.0f, 8, 6, 7, 0.0f, 0, -1.0f, 14.0f, -3.0f, 1.570796f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, 0, 18, -1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f, 0, -2.5f, 16.0f, 7.0f, (float)cos((double)walk * 0.6662) * 1.4f * walkAmount, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, 0, 18, -1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f, 0, 0.5f, 16.0f, 7.0f, (float)cos((double)walk * 0.6662 + PI) * 1.4f * walkAmount, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, 0, 18, -1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f, 0, -2.5f, 16.0f, -4.0f, (float)cos((double)walk * 0.6662 + PI) * 1.4f * walkAmount, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.82f, alpha, 0, 18, -1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f, 0, 0.5f, 16.0f, -4.0f, (float)cos((double)walk * 0.6662) * 1.4f * walkAmount, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade, alpha, 9, 18, -1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f, 0, -1.0f, 12.0f, 8.0f, tailPitch, tailYaw, 0.0f);
}


void JavaModel_RenderCreeperV53(Mob *m, GLuint tex, float shade, float alpha)
{
    float walk;
    float walkAmount;
    float leg1;
    float leg2;
    walk = m ? (float)m->animWalk : 0.0f;
    walkAmount = JavaModel_GetWalkAmount(m);
    leg1 = (float)cos((double)walk * 0.6662) * 1.4f * walkAmount;
    leg2 = (float)cos((double)walk * 0.6662 + PI) * 1.4f * walkAmount;
    JavaModel_RenderPart(tex, shade, alpha, 0, 0, -4.0f, -8.0f, -4.0f, 8, 8, 8, 0.0f, 0, 0.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.96f, alpha, 16, 16, -4.0f, 0.0f, -2.0f, 8, 12, 4, 0.0f, 0, 0.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.84f, alpha, 0, 16, -2.0f, 0.0f, -2.0f, 4, 6, 4, 0.0f, 0, -2.0f, 18.0f, 4.0f, leg1, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.84f, alpha, 0, 16, -2.0f, 0.0f, -2.0f, 4, 6, 4, 0.0f, 0, 2.0f, 18.0f, 4.0f, leg2, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.84f, alpha, 0, 16, -2.0f, 0.0f, -2.0f, 4, 6, 4, 0.0f, 0, -2.0f, 18.0f, -4.0f, leg2, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.84f, alpha, 0, 16, -2.0f, 0.0f, -2.0f, 4, 6, 4, 0.0f, 0, 2.0f, 18.0f, -4.0f, leg1, 0.0f, 0.0f);
}

void JavaModel_RenderSlimeV53(Mob *m, GLuint tex, float shade, float alpha)
{
    float squish;
    float scaleY;
    float scaleXZ;
    squish = 1.0f;
    if (m) { squish = 1.0f + (float)(sin(m->animWalk * 1.45) * 0.12); }
    scaleY = squish;
    scaleXZ = 2.0f - squish;
    glPushMatrix();
    glScalef(scaleXZ, scaleY, scaleXZ);
    JavaModel_RenderPart(tex, shade, alpha * 0.72f, 0, 0, -8.0f, 0.0f, -8.0f, 16, 16, 16, 0.0f, 0, 0.0f, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 1.08f, alpha, 0, 16, -6.0f, 2.0f, -6.0f, 12, 12, 12, 0.0f, 0, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.45f, alpha, 32, 0, -3.25f, 6.0f, -8.1f, 2, 2, 1, 0.0f, 0, 0.0f, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    JavaModel_RenderPart(tex, shade * 0.45f, alpha, 32, 4, 1.25f, 6.0f, -8.1f, 2, 2, 1, 0.0f, 0, 0.0f, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    glPopMatrix();
}

void JavaModel_RenderSquidV53(Mob *m, GLuint tex, float shade, float alpha)
{
    float walk;
    float wave;
    int i;
    float angle;
    float x;
    float z;
    walk = m ? (float)m->animWalk : (float)g_worldTimeSeconds;
    JavaModel_RenderPart(tex, shade, alpha, 0, 0, -6.0f, -8.0f, -6.0f, 12, 16, 12, 0.0f, 0, 0.0f, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < 8; i++) {
        angle = (float)i * 0.7853982f;
        x = (float)cos((double)angle) * 5.0f;
        z = (float)sin((double)angle) * 5.0f;
        wave = (float)sin((double)walk * 0.75 + (double)i) * 0.35f + 0.65f;
        JavaModel_RenderPart(tex, shade * 0.82f, alpha, 48, 0, -1.0f, 0.0f, -1.0f, 2, 18, 2, 0.0f, 0, x, 16.0f, z, wave, angle, 0.0f);
    }
}

int JavaModel_CanRenderExactType(int type)
{
    if (type == MOB_CHICKEN) { return 1; }
    if (type == MOB_COW) { return 1; }
    if (type == MOB_SHEEP) { return 1; }
    if (type == MOB_PIG) { return 1; }
    if (type == MOB_WOLF) { return 1; }
    if (type == MOB_SPIDER) { return 1; }
    if (type == MOB_ZOMBIE || type == MOB_SKELETON) { return 1; }
    if (type == MOB_CREEPER || type == MOB_SLIME || type == MOB_SQUID) { return 1; }
    return 0;
}

void JavaModel_RenderExactMob(Mob *m, GLuint tex, float alpha)
{
    float shade;
    float deathAmount;
    float creeperPulse;
    float scale;
    int bx;
    int by;
    int bz;
    int localLight;
    if (!m || !tex) { return; }
    if (!JavaModel_CanRenderExactType(m->type)) {
        RenderMobModelTex(m, tex, alpha);
        return;
    }

    bx = (int)floor(m->x);
    by = (int)floor(m->y + 0.8);
    bz = (int)floor(m->z);
    localLight = GetLightLevel(bx, by, bz);
    shade = 0.34f + (float)localLight / 15.0f * 0.58f + g_daySkyBrightness * 0.12f;
    shade = ApplyGammaBoost(shade);
    if (shade > 1.0f) { shade = 1.0f; }
    if (shade < 0.22f) { shade = 0.22f; }

    if (m->hurtTime > 0.0) {
        g_mobTintR = 1.0f;
        g_mobTintG = 0.28f + (float)(0.20 * sin(g_worldTimeSeconds * 38.0));
        g_mobTintB = 0.28f;
    } else if (m->burning) {
        g_mobTintR = 1.0f;
        g_mobTintG = 0.55f;
        g_mobTintB = 0.22f;
    } else {
        g_mobTintR = 1.0f;
        g_mobTintG = 1.0f;
        g_mobTintB = 1.0f;
    }

    glPushMatrix();
    glTranslatef((float)m->x, (float)m->y, (float)m->z);
    glRotatef((float)-m->renderYawOffset, 0.0f, 1.0f, 0.0f);

    if (m->deathTime > 0.0) {
        deathAmount = (float)((1.4 - m->deathTime) / 1.4);
        if (deathAmount < 0.0f) { deathAmount = 0.0f; }
        if (deathAmount > 1.0f) { deathAmount = 1.0f; }
        glRotatef(deathAmount * 90.0f, 0.0f, 0.0f, 1.0f);
    }

    if (m->type == MOB_CREEPER && m->fuseTimer > 0.0) {
        creeperPulse = (float)(m->fuseTimer / 1.6);
        if (creeperPulse > 1.0f) { creeperPulse = 1.0f; }
        scale = 1.0f + (float)sin((double)creeperPulse * 100.0) * creeperPulse * 0.01f;
        glScalef(scale, 1.0f + creeperPulse * 0.10f, scale);
        shade += creeperPulse * 0.35f;
        if (shade > 1.0f) { shade = 1.0f; }
    }

    glDisable(GL_CULL_FACE);
    JavaModel_BeginEntity();

    if (m->type == MOB_CHICKEN) {
        JavaModel_RenderChicken(m, tex, shade, alpha);
    } else if (m->type == MOB_COW) {
        JavaModel_RenderCow(m, tex, shade, alpha);
    } else if (m->type == MOB_PIG) {
        JavaModel_RenderPig(m, tex, shade, alpha);
    } else if (m->type == MOB_SHEEP) {
        JavaModel_RenderSheepBase(m, texMobSheep ? texMobSheep : tex, shade, alpha);
        if (!m->sheared && texMobSheepFur) {
            JavaModel_RenderSheepFur(m, texMobSheepFur, shade, alpha * 0.95f);
        }
    } else if (m->type == MOB_WOLF) {
        JavaModel_RenderWolf(m, tex, shade, alpha);
    } else if (m->type == MOB_SPIDER) {
        JavaModel_RenderSpider(m, tex, shade, alpha);
    } else if (m->type == MOB_ZOMBIE || m->type == MOB_SKELETON) {
        JavaModel_RenderBipedCore(m, tex, shade, alpha, m->type == MOB_SKELETON, ITEM_NONE, 0.0f, m->type == MOB_ZOMBIE, 0.0f, 0);
    } else if (m->type == MOB_CREEPER) {
        JavaModel_RenderCreeperV53(m, tex, shade, alpha);
    } else if (m->type == MOB_SLIME) {
        JavaModel_RenderSlimeV53(m, tex, shade, alpha);
    } else if (m->type == MOB_SQUID) {
        JavaModel_RenderSquidV53(m, tex, shade, alpha);
    }

    JavaModel_EndEntity();
    glPopMatrix();
    g_mobTintR = 1.0f;
    g_mobTintG = 1.0f;
    g_mobTintB = 1.0f;
}

void RenderMobModelJavaV53(Mob *m, GLuint tex, float alpha)
{
    JavaModel_RenderExactMob(m, tex, alpha);
}

/* Clean-room blocky mob model renderer.
   The first mob pass used large billboard cards.  This pass replaces that
   with simple OpenGL 1.1 cuboid models so mobs read like voxel creatures
   while still compiling in Open Watcom and Win98-era OpenGL. */

void DrawMobBoxPart(float cx, float cy, float cz,
                    float sx, float sy, float sz,
                    GLuint tex, float shade, float alpha)
{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;

    if (!tex) {
        return;
    }

    x0 = cx - sx * 0.5f;
    x1 = cx + sx * 0.5f;
    y0 = cy - sy * 0.5f;
    y1 = cy + sy * 0.5f;
    z0 = cz - sz * 0.5f;
    z1 = cz + sz * 0.5f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor4f(shade * g_mobTintR, shade * g_mobTintG, shade * g_mobTintB, alpha);

    glBegin(GL_QUADS);

    /* Front */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y1, z0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y1, z0);

    /* Back */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x0, y1, z1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y1, z1);

    /* Left */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x0, y0, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x0, y1, z0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y1, z1);

    /* Right */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y1, z0);

    /* Top */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y1, z0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y1, z0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y1, z1);

    /* Bottom */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x0, y0, z1);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, y0, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, y0, z0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x0, y0, z0);

    glEnd();
}

void RenderMobModelTex(Mob *m, GLuint tex, float alpha)
{
    float bright;
    float pulse;
    float shade;
    float walk;
    float bodyBob;
    float legDeg;
    float legDegOpp;
    float armDeg;
    float wingDeg;
    float scale;
    float side;
    float zpos;
    float yrot;
    float zrot;
    float walkFactor;
    int s;
    int t;

    if (!m || !tex) { return; }

    bright = ApplyGammaBoost(0.62f + g_daySkyBrightness * 0.48f);
    if (bright > 1.0f) { bright = 1.0f; }

    if (m->burning) {
        pulse = (float)(0.70 + 0.30 * sin(g_worldTimeSeconds * 20.0));
        shade = pulse;
    } else {
        shade = bright;
    }

    if (alpha > 1.0f) { alpha = 1.0f; }
    if (alpha < 0.05f) { alpha = 0.05f; }
    if (m->hurtTime > 0.0) {
        pulse = (float)(0.65 + 0.35 * sin(g_worldTimeSeconds * 48.0));
        shade = 1.0f;
        alpha = alpha * pulse;
    }
    if (m->deathTime > 0.0) {
        alpha = alpha * (float)(m->deathTime / 0.70);
        if (alpha < 0.10f) { alpha = 0.10f; }
    }

    /* Converted from Java model animation ideas: limb swing is cosine based,
       arms/legs rotate around the shoulder/hip instead of sliding through blocks. */
    walk = (float)m->animWalk;
    walkFactor = (float)(sqrt(m->vx * m->vx + m->vz * m->vz) / 1.45);
    if (walkFactor > 1.0f) { walkFactor = 1.0f; }
    if (walkFactor < 0.04f && m->type != MOB_SQUID) { walkFactor = 0.0f; }

    legDeg = (float)cos((double)walk * 0.6662) * 38.0f * walkFactor;
    legDegOpp = (float)cos((double)walk * 0.6662 + PI) * 38.0f * walkFactor;
    armDeg = (float)cos((double)walk * 0.6662 + PI) * 30.0f * walkFactor;
    wingDeg = (float)sin((double)walk * 1.7) * 35.0f * (walkFactor > 0.0f ? walkFactor : 0.15f);
    bodyBob = (float)(fabs(sin((double)walk)) * 0.025 * walkFactor);

    glPushMatrix();
    glTranslatef((float)m->x, (float)m->y, (float)m->z);
    if (m->deathTime > 0.0) {
        glRotatef((float)((1.0 - m->deathTime / 0.70) * 82.0), 0.0f, 0.0f, 1.0f);
    }
    /* Match the player renderer convention: model front is -Z, positive yaw
       turns the camera/player, so negate mob yaw for world-facing cuboids. */
    glRotatef((float)-m->yaw, 0.0f, 1.0f, 0.0f);
    glDisable(GL_CULL_FACE);

    if (m->type == MOB_ZOMBIE || m->type == MOB_SKELETON) {
        scale = 1.0f;
        if (m->type == MOB_SKELETON) { scale = 0.86f; }
        DrawMobSkinBoxPart(0.0f, 1.74f + bodyBob, 0.0f, 0.50f * scale, 0.50f, 0.50f * scale, tex, shade, alpha, 0, 0, 8, 8, 8);
        DrawMobSkinBoxPart(0.0f, 1.12f + bodyBob, 0.0f, 0.50f * scale, 0.75f, 0.25f * scale, tex, shade, alpha, 16, 16, 8, 12, 4);
        DrawMobSkinBoxPartRot(-0.38f * scale, 1.45f + bodyBob, 0.0f, 0.0f, -0.36f, 0.0f, 0.22f * scale, 0.74f, 0.22f * scale, tex, shade * 0.93f, alpha, 40, 16, 4, 12, 4, armDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.38f * scale, 1.45f + bodyBob, 0.0f, 0.0f, -0.36f, 0.0f, 0.22f * scale, 0.74f, 0.22f * scale, tex, shade * 0.93f, alpha, 40, 16, 4, 12, 4, -armDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot(-0.13f * scale, 0.75f, 0.0f, 0.0f, -0.36f, 0.0f, 0.22f * scale, 0.74f, 0.22f * scale, tex, shade * 0.84f, alpha, 0, 16, 4, 12, 4, -legDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.13f * scale, 0.75f, 0.0f, 0.0f, -0.36f, 0.0f, 0.22f * scale, 0.74f, 0.22f * scale, tex, shade * 0.84f, alpha, 0, 16, 4, 12, 4, legDeg, 0.0f, 0.0f);
    } else if (m->type == MOB_CREEPER) {
        DrawMobSkinBoxPart(0.0f, 1.60f + bodyBob, 0.0f, 0.64f, 0.64f, 0.64f, tex, shade, alpha, 0, 0, 8, 8, 8);
        DrawMobSkinBoxPart(0.0f, 0.96f + bodyBob, 0.0f, 0.56f, 0.86f, 0.30f, tex, shade, alpha, 16, 16, 8, 12, 4);
        DrawMobSkinBoxPartRot(-0.20f, 0.54f, -0.25f, 0.0f, -0.24f, 0.0f, 0.22f, 0.48f, 0.22f, tex, shade * 0.84f, alpha, 0, 16, 4, 6, 4, legDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.20f, 0.54f, -0.25f, 0.0f, -0.24f, 0.0f, 0.22f, 0.48f, 0.22f, tex, shade * 0.84f, alpha, 0, 16, 4, 6, 4, legDegOpp, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot(-0.20f, 0.54f,  0.25f, 0.0f, -0.24f, 0.0f, 0.22f, 0.48f, 0.22f, tex, shade * 0.84f, alpha, 0, 16, 4, 6, 4, legDegOpp, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.20f, 0.54f,  0.25f, 0.0f, -0.24f, 0.0f, 0.22f, 0.48f, 0.22f, tex, shade * 0.84f, alpha, 0, 16, 4, 6, 4, legDeg, 0.0f, 0.0f);
    } else if (m->type == MOB_PIG || m->type == MOB_COW || m->type == MOB_SHEEP || m->type == MOB_WOLF) {
        GLuint bodyTex;
        GLuint headTex;
        float bodyY;
        float bodyW;
        float bodyH;
        float bodyD;
        float headY;
        float headZ;
        float headW;
        float headH;
        float headD;
        float legH;
        float legW;
        float legY;
        float legX;
        float legZFront;
        float legZBack;

        bodyTex = tex;
        headTex = tex;
        bodyY = 0.82f + bodyBob;
        bodyW = 0.68f; bodyH = 0.50f; bodyD = 0.98f;
        headY = 1.05f + bodyBob; headZ = -0.54f; headW = 0.50f; headH = 0.50f; headD = 0.50f;
        legH = 0.52f; legW = 0.16f;
        legX = 0.28f; legZFront = -0.32f; legZBack = 0.42f;

        if (m->type == MOB_COW) {
            /* Java ModelCow.java: head addBox(-4,-4,-6,8,8,6) at 0,4,-8;
               body addBox(-6,-10,-7,12,18,10) rotated on X; horns use 22,0;
               udder uses 52,0.  The C renderer keeps the same UV rectangles
               but scales them into compact OpenGL world units. */
            bodyY = 1.00f + bodyBob;
            bodyW = 0.78f; bodyH = 0.62f; bodyD = 1.16f;
            headY = 1.24f + bodyBob; headZ = -0.62f; headW = 0.52f; headH = 0.52f; headD = 0.40f;
            legH = 0.72f; legW = 0.18f;
            legX = 0.32f; legZFront = -0.34f; legZBack = 0.44f;

            DrawMobSkinBoxPart(0.0f, bodyY, 0.10f,
                               bodyW, bodyH, bodyD,
                               bodyTex, shade, alpha,
                               18, 4, 12, 18, 10);
            DrawMobSkinBoxPartRot(0.0f, headY, headZ,
                                  0.0f, 0.0f, 0.0f,
                                  headW, headH, headD,
                                  headTex, shade, alpha,
                                  0, 0, 8, 8, 6,
                                  0.0f, 0.0f, 0.0f);
            DrawMobSkinBoxPart(-0.22f, headY + 0.28f, headZ - 0.05f,
                               0.07f, 0.20f, 0.07f,
                               tex, shade * 0.98f, alpha,
                               22, 0, 1, 3, 1);
            DrawMobSkinBoxPart( 0.22f, headY + 0.28f, headZ - 0.05f,
                               0.07f, 0.20f, 0.07f,
                               tex, shade * 0.98f, alpha,
                               22, 0, 1, 3, 1);
            DrawMobSkinBoxPart(0.0f, bodyY - 0.34f, 0.28f,
                               0.26f, 0.10f, 0.20f,
                               tex, shade * 0.90f, alpha,
                               52, 0, 4, 6, 2);
        } else {
            if (m->type == MOB_SHEEP) {
                bodyW = 0.76f; bodyH = 0.62f; bodyD = 1.08f;
                bodyY = 0.94f + bodyBob;
                headY = 1.17f + bodyBob; headW = 0.50f; headH = 0.50f; headD = 0.56f;
                legH = 0.62f; bodyTex = m->sheared ? texMobSheep : texMobSheepFur; headTex = bodyTex;
            }
            if (m->type == MOB_WOLF) {
                bodyW = 0.48f; bodyH = 0.44f; bodyD = 0.86f;
                bodyY = 0.82f + bodyBob;
                headY = 1.10f + bodyBob; headZ = -0.60f; headW = 0.48f; headH = 0.42f; headD = 0.44f;
                legH = 0.55f; legW = 0.11f; legX = 0.22f; legZFront = -0.28f; legZBack = 0.34f;
            }

            DrawMobSkinBoxPart(0.0f, bodyY, 0.10f, bodyW, bodyH, bodyD, bodyTex, shade, alpha, 28, 8, 10, 16, 8);
            DrawMobSkinBoxPart(0.0f, headY, headZ, headW, headH, headD, headTex, shade, alpha, 0, 0, 8, 8, 8);
        }

        legY = legH;
        DrawMobSkinBoxPartRot(-legX, legY, legZFront, 0.0f, -legH * 0.5f, 0.0f, legW, legH, legW, tex, shade * 0.82f, alpha, 0, 16, 4, 12, 4, legDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( legX, legY, legZFront, 0.0f, -legH * 0.5f, 0.0f, legW, legH, legW, tex, shade * 0.82f, alpha, 0, 16, 4, 12, 4, legDegOpp, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot(-legX, legY, legZBack,  0.0f, -legH * 0.5f, 0.0f, legW, legH, legW, tex, shade * 0.82f, alpha, 0, 16, 4, 12, 4, legDegOpp, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( legX, legY, legZBack,  0.0f, -legH * 0.5f, 0.0f, legW, legH, legW, tex, shade * 0.82f, alpha, 0, 16, 4, 12, 4, legDeg, 0.0f, 0.0f);
    } else if (m->type == MOB_CHICKEN) {
        DrawMobSkinBoxPart(0.0f, 0.58f + bodyBob, 0.05f, 0.38f, 0.50f, 0.38f, tex, shade, alpha, 0, 9, 6, 8, 6);
        DrawMobSkinBoxPart(0.0f, 1.00f + bodyBob, -0.20f, 0.25f, 0.38f, 0.20f, tex, shade, alpha, 0, 0, 4, 6, 3);
        DrawMobSkinBoxPart(0.0f, 0.96f + bodyBob, -0.38f, 0.25f, 0.12f, 0.14f, tex, shade, alpha, 14, 0, 4, 2, 2);
        DrawMobSkinBoxPart(0.0f, 0.82f + bodyBob, -0.35f, 0.12f, 0.12f, 0.12f, tex, shade, alpha, 14, 4, 2, 2, 2);
        DrawMobSkinBoxPartRot(-0.13f, 0.36f, 0.02f, 0.0f, -0.18f, 0.0f, 0.08f, 0.36f, 0.08f, tex, shade * 0.78f, alpha, 26, 0, 3, 5, 3, legDeg, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot( 0.13f, 0.36f, 0.02f, 0.0f, -0.18f, 0.0f, 0.08f, 0.36f, 0.08f, tex, shade * 0.78f, alpha, 26, 0, 3, 5, 3, legDegOpp, 0.0f, 0.0f);
        DrawMobSkinBoxPartRot(-0.32f, 0.72f, 0.08f, 0.0f, -0.15f, 0.0f, 0.08f, 0.30f, 0.38f, tex, shade * 0.92f, alpha, 24, 13, 1, 4, 6, 0.0f, 0.0f, wingDeg);
        DrawMobSkinBoxPartRot( 0.32f, 0.72f, 0.08f, 0.0f, -0.15f, 0.0f, 0.08f, 0.30f, 0.38f, tex, shade * 0.92f, alpha, 24, 13, 1, 4, 6, 0.0f, 0.0f, -wingDeg);
    } else if (m->type == MOB_SPIDER) {
        DrawMobSkinBoxPart(0.0f, 0.48f + bodyBob, 0.12f, 0.92f, 0.36f, 0.92f, tex, shade, alpha, 0, 12, 10, 8, 12);
        DrawMobSkinBoxPart(0.0f, 0.52f + bodyBob, -0.42f, 0.50f, 0.32f, 0.44f, tex, shade, alpha, 0, 0, 6, 6, 6);
        DrawMobSkinBoxPart(0.0f, 0.58f + bodyBob, -0.72f, 0.58f, 0.42f, 0.42f, tex, shade, alpha, 32, 4, 8, 8, 8);
        for (s = 0; s < 2; s++) {
            side = -1.0f; if (s == 1) { side = 1.0f; }
            for (t = 0; t < 4; t++) {
                zpos = -0.38f + (float)t * 0.28f;
                yrot = side * (55.0f - (float)t * 8.0f) + (float)sin((double)walk * 1.3324 + (double)t) * 18.0f * side;
                zrot = side * (34.0f + (float)(t % 2) * 10.0f);
                DrawMobSkinBoxPartRot(side * 0.36f, 0.36f, zpos, side * 0.42f, 0.0f, 0.0f, 0.84f, 0.10f, 0.10f, tex, shade * 0.78f, alpha, 18, 0, 16, 2, 2, 0.0f, yrot, zrot);
            }
        }
    } else if (m->type == MOB_SLIME) {
        scale = 1.0f + (float)fabs(sin((double)walk)) * 0.09f;
        DrawMobSkinBoxPart(0.0f, 0.55f + bodyBob, 0.0f, 1.05f * scale, 1.05f / scale, 1.05f * scale, tex, shade, 0.78f * alpha, 0, 0, 8, 8, 8);
        DrawMobSkinBoxPart(0.0f, 0.55f + bodyBob, 0.0f, 0.62f, 0.62f, 0.62f, tex, shade * 1.08f, 0.92f * alpha, 0, 16, 6, 6, 6);
        DrawMobSkinBoxPart(-0.18f, 0.70f + bodyBob, -0.34f, 0.13f, 0.13f, 0.06f, tex, shade * 0.55f, alpha, 32, 0, 2, 2, 2);
        DrawMobSkinBoxPart( 0.18f, 0.70f + bodyBob, -0.34f, 0.13f, 0.13f, 0.06f, tex, shade * 0.55f, alpha, 32, 4, 2, 2, 2);
    } else if (m->type == MOB_SQUID) {
        DrawMobSkinBoxPart(0.0f, 0.95f + bodyBob, 0.0f, 0.82f, 0.82f, 0.82f, tex, shade, alpha, 0, 0, 12, 16, 12);
        for (t = 0; t < 8; t++) {
            yrot = (float)t * 45.0f;
            DrawMobSkinBoxPartRot(0.0f, 0.48f + bodyBob, 0.0f, 0.0f, -0.36f, 0.34f, 0.10f, 0.72f, 0.10f, tex, shade * 0.82f, alpha, 48, 0, 2, 18, 2, (float)sin((double)walk + (double)t) * 16.0f, yrot, 0.0f);
        }
    } else {
        DrawMobSkinBoxPart(0.0f, 0.80f + bodyBob, 0.0f, 0.85f, 1.25f, 0.50f, tex, shade, alpha, 0, 0, 8, 8, 8);
    }

    glEnable(GL_CULL_FACE);
    glPopMatrix();
}

void RenderMobs(void)
{
    int i;
    GLuint tex;
    double dx;
    double dz;
    double dist2;
    double fullDist2;
    double alpha;
    Mob renderMobV32;

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    fullDist2 = (double)g_mobFullModelDistanceBlocks * (double)g_mobFullModelDistanceBlocks;

    for (i = 0; i < MAX_MOBS; i++) {
        if (!mobs[i].active) { continue; }
        if (!IsMobInsideLoadedWindow(&mobs[i])) { continue; }

        dx = mobs[i].x - playerX;
        dz = mobs[i].z - playerZ;
        dist2 = dx * dx + dz * dz;

        if (dist2 > (double)GetMobRenderDistanceBlocks() * (double)GetMobRenderDistanceBlocks()) {
            g_renderProfileV33.mobsDistanceCulled++;
            continue;
        }
        if (!IsPointProbablyInView(mobs[i].x, mobs[i].z, 14.0, -0.32)) {
            g_renderProfileV33.mobsFrustumCulled++;
            continue;
        }
        alpha = g_mobRenderPartialTicks;
        MobInterp_BuildRenderMobV32(&mobs[i], &renderMobV32, alpha);
        tex = GetMobTexture(&renderMobV32);
        DrawMobShadow(&renderMobV32);

        RenderMobModelJavaV53(&renderMobV32, tex, 1.0f);
        g_renderProfileV33.mobsRendered++;
        if (renderMobV32.type == MOB_SPIDER && !IsDaylightForMobs()) {
            RenderMobModelJavaV53(&renderMobV32, texMobSpiderEyes, 0.88f);
        }
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_CULL_FACE);
}

void StartHandSwing(void)
{
    handSwingLength = HAND_SWING_TIME;
    handSwingTimer = HAND_SWING_TIME;
}

void StartHandUse(void)
{
    handUseLength = HAND_USE_TIME;
    handUseTimer = HAND_USE_TIME;
}

void ItemRendererV38_UpdateEquippedItem(double dt)
{
    InventorySlot *slot;
    int currentItem;
    int currentDamage;
    int same;
    double target;
    double delta;
    double maxStep;

    itemRendererPrevEquippedProgressV38 = itemRendererEquippedProgressV38;
    currentItem = ITEM_NONE;
    currentDamage = 0;
    if (selectedHotbarSlot >= 0 && selectedHotbarSlot < HOTBAR_SLOTS) {
        slot = &hotbar[selectedHotbarSlot];
        if (slot->item != ITEM_NONE && slot->count > 0) { currentItem = slot->item; currentDamage = slot->damage; }
    }
    same = (currentItem == itemRendererHeldItemV38 && currentDamage == itemRendererHeldDamageV38 && selectedHotbarSlot == itemRendererHeldSlotV38);
    target = same ? 1.0 : 0.0;
    delta = target - itemRendererEquippedProgressV38;
    maxStep = dt * 8.0;
    if (maxStep > 0.40) { maxStep = 0.40; }
    if (delta < -maxStep) { delta = -maxStep; }
    if (delta > maxStep) { delta = maxStep; }
    itemRendererEquippedProgressV38 += delta;
    if (itemRendererEquippedProgressV38 < 0.10) {
        itemRendererHeldItemV38 = currentItem;
        itemRendererHeldDamageV38 = currentDamage;
        itemRendererHeldSlotV38 = selectedHotbarSlot;
    }
}

float ItemRendererV38_GetEquipProgress(void)
{
    return (float)itemRendererEquippedProgressV38;
}

void UpdatePlayerHandAnimation(double dt)
{
    if (handSwingTimer > 0.0) {
        handSwingTimer -= dt;
        if (handSwingTimer < 0.0) { handSwingTimer = 0.0; }
    }

    if (handUseTimer > 0.0) {
        handUseTimer -= dt;
        if (handUseTimer < 0.0) { handUseTimer = 0.0; }
    }
    ItemRendererV38_UpdateEquippedItem(dt);
}

float GetHandProgress(double timer, double length)
{
    double p;

    if (length <= 0.0 || timer <= 0.0) {
        return 0.0f;
    }

    p = 1.0 - timer / length;

    if (p < 0.0) {
        p = 0.0;
    }

    if (p > 1.0) {
        p = 1.0;
    }

    return (float)p;
}

void DrawHeldItemFirstPersonV29(int item)
{
    int block;
    float swingP;
    float swingSin;
    float swingRoot;
    float equip;
    float f5;

    if (item == ITEM_NONE) { return; }
    block = ItemToBlock(item);
    swingP = GetHandProgress(handSwingTimer, handSwingLength);
    swingSin = (float)sin((double)swingP * PI);
    swingRoot = (float)sin(sqrt((double)swingP) * PI);
    equip = ItemRendererV38_GetEquipProgress();
    f5 = 0.8f;

    glPushMatrix();
    /* V38: Java ItemRenderer-style held item transform.  The equippedProgress
       value slides newly selected items up into view instead of snapping them,
       and the sqrt/squared swing rotations match the early Java renderer feel. */
    glTranslatef(-swingRoot * 0.4f, (float)sin(sqrt((double)swingP) * PI * 2.0) * 0.2f, -swingSin * 0.2f);
    glTranslatef(0.70f * f5, -0.65f * f5 - (1.0f - equip) * 0.60f, -0.90f * f5);
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(-((float)sin((double)swingP * (double)swingP * PI)) * 20.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(-swingRoot * 20.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(-swingRoot * 80.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.40f, 0.40f, 0.40f);

    if (block != BLOCK_AIR && !ItemRenderV46_ShouldPreferItemIcon(item) && ShouldRenderBlockItemAs3DV27(block)) {
        DrawHeldBlockCubeLocal(block, 1.0f);
    } else {
        if (block != BLOCK_AIR) { glScalef(1.20f, 1.20f, 1.20f); }
        DrawHeldItemQuadLocal(item, 0.72f);
    }
    glPopMatrix();
}


void RenderFirstPersonPlayerArmV41(float swing, float usePush, float equip, float bobX, float bobY)
{
    float swingP;
    float armRX;
    float armRZ;

    swingP = GetHandProgress(handSwingTimer, handSwingLength);
    armRX = -1.12f + swing * 0.78f + usePush * 0.10f;
    armRZ = 0.18f + swing * 0.22f;

    if (!texMobPlayer) {
        DrawPlayerArmPrism();
        return;
    }

    /* V41: render the first-person hand from the same ModelBiped right-arm
       cuboid used by the Java player model: texture offset 40,16; box -3,-2,-2;
       size 4x12x4; rotation point -5,2,0.  The outer transform keeps the item
       renderer's first-person placement while the actual mesh/UVs are no
       longer a custom hardcoded prism. */
    glPushMatrix();
    glTranslatef(0.74f + bobX, -0.82f + bobY - (1.0f - equip) * 0.30f, -1.10f + usePush * 0.12f);
    glRotatef(28.0f - swing * 18.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(8.0f + swing * 12.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(-10.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.0625f, -0.0625f, 0.0625f);
    JavaModel_RenderPart(texMobPlayer, 1.0f, 1.0f, 40, 16,
                         -3.0f, -2.0f, -2.0f,
                         4, 12, 4,
                         0.0f, 0,
                         -5.0f, 2.0f, 0.0f,
                         armRX, 0.0f, armRZ);
    glPopMatrix();
}

void RenderPlayerHand(void)
{
    float swingP;
    float useP;
    float swing;
    float usePush;
    float bobX;
    float bobY;
    float equip;
    int heldItem;

    swingP = GetHandProgress(handSwingTimer, handSwingLength);
    useP = GetHandProgress(handUseTimer, handUseLength);
    swing = (float)sin((double)swingP * PI);
    usePush = (float)sin((double)useP * PI);
    equip = ItemRendererV38_GetEquipProgress();
    bobX = (float)(cos(handBob * 0.5) * 0.018);
    bobY = (float)(sin(handBob) * 0.032);
    heldItem = GetHeldHotbarItem();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    RenderFirstPersonPlayerArmV41(swing, usePush, equip, bobX, bobY);
    DrawHeldItemFirstPersonV29(heldItem);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
}

void DrawPlayerArmPrism(void)
{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;

    /* First-person arm from player skin texture; fallback is colored cuboid. */
    if (texMobPlayer) {
        DrawMobSkinBoxPart(0.0f, 0.0f, 0.0f,
                           0.30f, 1.04f, 0.44f,
                           texMobPlayer, 1.0f, 1.0f,
                           40, 16, 4, 12, 4);
        return;
    }

    x0 = -0.14f;
    x1 =  0.16f;
    y0 = -0.54f;
    y1 =  0.18f;
    z0 = -0.14f;
    z1 =  0.14f;

    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 0);
    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 1);
    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 2);
    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 3);
    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 4);
    DrawPlayerArmFace(x0, x1, y0, y1, z0, z1, 5);
}

void DrawPlayerArmFace(float x0, float x1, float y0, float y1, float z0, float z1, int face)
{
    float shade;

    if (face == 0) {
        shade = 1.00f;
    } else if (face == 1) {
        shade = 0.58f;
    } else if (face == 2 || face == 3) {
        shade = 0.82f;
    } else {
        shade = 0.70f;
    }

    glColor3f(0.82f * shade, 0.56f * shade, 0.43f * shade);

    glBegin(GL_QUADS);

    if (face == 0) {
        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z0);
    } else if (face == 1) {
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y0, z1);
    } else if (face == 2) {
        glVertex3f(x1, y0, z0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);
    } else if (face == 3) {
        glVertex3f(x0, y0, z1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x0, y1, z1);
    } else if (face == 4) {
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y1, z0);
    } else if (face == 5) {
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1);
    }

    glEnd();

    if (face == 5) {
        glColor3f(0.62f * shade, 0.38f * shade, 0.30f * shade);

        glBegin(GL_QUADS);
        glVertex3f(x1 + 0.002f, y0 + 0.12f, z0 + 0.03f);
        glVertex3f(x1 + 0.002f, y0 + 0.12f, z1 - 0.03f);
        glVertex3f(x1 + 0.002f, y0 + 0.34f, z1 - 0.03f);
        glVertex3f(x1 + 0.002f, y0 + 0.34f, z0 + 0.03f);
        glEnd();
    }
}




/* ------------------------------------------------------------ */
/* World helpers                                                */
/* ------------------------------------------------------------ */

int IsInsideWorld(int x, int y, int z)
{
    if (x < 0 || x >= WORLD_X) {
        return 0;
    }

    if (y < 0 || y >= WORLD_Y) {
        return 0;
    }

    if (z < 0 || z >= WORLD_Z) {
        return 0;
    }

    return 1;
}

int GetBlock(int x, int y, int z)
{
    if (!IsInsideWorld(x, y, z)) {
        return BLOCK_BORDER;
    }

    return world[x][y][z];
}

void SetBlock(int x, int y, int z, int block)
{
    int oldBlock;
    if (!IsInsideWorld(x, y, z)) {
        return;
    }

    if (world[x][y][z] == BLOCK_BORDER) {
        return;
    }

    oldBlock = world[x][y][z];
    world[x][y][z] = block;
    if (block == BLOCK_AIR) { g_blockMeta[x][y][z] = 0; g_redstonePower[x][y][z] = 0; }
    if (block == BLOCK_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { g_blockMeta[x][y][z] &= (unsigned char)(FLUID_V42_LEVEL_MASK | FLUID_V42_FALLING_FLAG); }
    RebuildColumnTopAt(x, z);
    InvalidateTerrainChunkMeshAt(x, z);
    InvalidateTerrainChunkMeshAt(x + 1, z);
    InvalidateTerrainChunkMeshAt(x - 1, z);
    InvalidateTerrainChunkMeshAt(x, z + 1);
    InvalidateTerrainChunkMeshAt(x, z - 1);
    EnqueueLightUpdateV42(x, y, z, 12);
    if (oldBlock != block) {
        BlockV50_NotifyNeighborsOfChange(x, y, z, oldBlock, block);
        BlockV52_OnBlockChanged(x, y, z, oldBlock, block);
    }
}

int IsSolidBlock(int block)
{
    return BlockV49_BlocksMovement(block);
}


/* ------------------------------------------------------------ */
/* Player movement                                              */
/* ------------------------------------------------------------ */

void CenterMouse(void)
{
    POINT p;

    p.x = g_windowWidth / 2;
    p.y = g_windowHeight / 2;

    ClientToScreen(g_hwnd, &p);
    SetCursorPos(p.x, p.y);

    ignoreNextMouseDelta = 1;
}


void UpdateMouseLook(void)
{
    POINT center;
    POINT mouse;
    int dx;
    int dy;

    if (!mouseLocked) {
        return;
    }

    if (GetForegroundWindow() != g_hwnd) {
        return;
    }

    center.x = g_windowWidth / 2;
    center.y = g_windowHeight / 2;

    ClientToScreen(g_hwnd, &center);

    GetCursorPos(&mouse);

    dx = mouse.x - center.x;
    dy = mouse.y - center.y;

    /*
        After forcing the mouse to the center, skip one frame so the
        teleport itself does not become camera movement.
    */
    if (ignoreNextMouseDelta) {
        ignoreNextMouseDelta = 0;
        SetCursorPos(center.x, center.y);
        return;
    }

    /*
        Delta-counting camera movement:
            yaw   = left/right mouse movement
            pitch = up/down mouse movement
    */
    yaw += (double)dx * MOUSE_SPEED;
    pitch -= (double)dy * MOUSE_SPEED;

    if (pitch > 89.0) {
        pitch = 89.0;
    }

    if (pitch < -89.0) {
        pitch = -89.0;
    }

    /*
        Lock physical cursor back to the screen center.
        The visual crosshair is the only cursor the player sees.
    */
    SetCursorPos(center.x, center.y);
}



void HandleGameInput(double dt)
{
    double speed;
    double yawRad;
    double forwardX;
    double forwardZ;
    double rightX;
    double rightZ;
    double moveX;
    double moveZ;
    double len;
    int inWater;

    inWater = IsPlayerInWater();
    speed = MOVE_SPEED * dt;
    if (inWater) {
        speed *= WATER_HORIZONTAL_SCALE;
    }

    yawRad = yaw * PI / 180.0;
    forwardX = -sin(yawRad);
    forwardZ = -cos(yawRad);
    rightX = cos(yawRad);
    rightZ = -sin(yawRad);

    moveX = 0.0;
    moveZ = 0.0;

    if (keyForward) { moveX += forwardX; moveZ += forwardZ; }
    if (keyBack) { moveX -= forwardX; moveZ -= forwardZ; }
    if (keyRight) { moveX += rightX; moveZ += rightZ; }
    if (keyLeft) { moveX -= rightX; moveZ -= rightZ; }

    len = sqrt(moveX * moveX + moveZ * moveZ);
    if (len > 0.0001) {
        moveX = (moveX / len) * speed;
        moveZ = (moveZ / len) * speed;
    }

    MovePlayerAxis(moveX, 0.0, 0.0);
    MovePlayerAxis(0.0, 0.0, moveZ);

    if (keyJump) {
        if (inWater) {
            onGround = 0;
        } else if (onGround) {
            velocityY = JUMP_SPEED;
            onGround = 0;
        }
    }
}




/* ------------------------------------------------------------ */
/* Collision detection                                          */
/* ------------------------------------------------------------ */

int PlayerCollidesAt(double x, double y, double z)
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

    minX = (int)floor(x - PLAYER_RADIUS);
    maxX = (int)floor(x + PLAYER_RADIUS);
    minY = (int)floor(y);
    maxY = (int)floor(y + PLAYER_HEIGHT);
    minZ = (int)floor(z - PLAYER_RADIUS);
    maxZ = (int)floor(z + PLAYER_RADIUS);

    for (bx = minX; bx <= maxX; bx++) {
        for (by = minY; by <= maxY; by++) {
            for (bz = minZ; bz <= maxZ; bz++) {
                if (IsSolidBlockAtV5(bx, by, bz)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}


int MovePlayerAxis(double dx, double dy, double dz)
{
    double newX;
    double newY;
    double newZ;

    newX = playerX + dx;
    newY = playerY + dy;
    newZ = playerZ + dz;

    if (!PlayerCollidesAt(newX, newY, newZ)) {
        playerX = newX;
        playerY = newY;
        playerZ = newZ;
        return 0;
    }
    return 1;
}

int PlayerV22_MoveAxisSweep(double dx, double dy, double dz)
{
    double remaining;
    double step;
    double tryStep;
    double nx;
    double ny;
    double nz;
    int collided;

    remaining = fabs(dx) + fabs(dy) + fabs(dz);
    if (remaining <= 0.000001) { return 0; }

    collided = 0;
    step = PLAYER_SWEEP_STEP_V22;
    if (remaining < step) { step = remaining; }

    while (remaining > 0.000001) {
        tryStep = step;
        if (tryStep > remaining) { tryStep = remaining; }
        nx = playerX;
        ny = playerY;
        nz = playerZ;
        if (dx > 0.0) { nx += tryStep; } else if (dx < 0.0) { nx -= tryStep; }
        if (dy > 0.0) { ny += tryStep; } else if (dy < 0.0) { ny -= tryStep; }
        if (dz > 0.0) { nz += tryStep; } else if (dz < 0.0) { nz -= tryStep; }
        if (PlayerV22_AabbCollidesAt(nx, ny, nz)) { collided = 1; break; }
        playerX = nx;
        playerY = ny;
        playerZ = nz;
        remaining -= tryStep;
    }
    return collided;
}

int PlayerV22_MoveHorizontal(double dx, double dz)
{
    double oldX;
    double oldY;
    double oldZ;
    double stepY;
    int collidedNormal;
    int collidedStep;

    if (fabs(dx) < 0.000001 && fabs(dz) < 0.000001) { return 0; }
    oldX = playerX;
    oldY = playerY;
    oldZ = playerZ;

    if (g_playerSneakingV22 && onGround) {
        if (!PlayerV22_HasGroundSupport(playerX + dx, playerY, playerZ + dz)) {
            velocityX *= 0.25;
            velocityZ *= 0.25;
            return 1;
        }
    }

    collidedNormal = PlayerV22_MoveHorizontalRaw(dx, dz);
    if (!collidedNormal) { return 0; }

    if (!onGround || g_playerInWaterV22 || g_playerInLavaV22 || g_playerInWebV22) {
        if (dx != 0.0 && fabs(playerX - oldX) < fabs(dx) * 0.5) { velocityX = 0.0; }
        if (dz != 0.0 && fabs(playerZ - oldZ) < fabs(dz) * 0.5) { velocityZ = 0.0; }
        return 1;
    }

    playerX = oldX;
    playerY = oldY;
    playerZ = oldZ;
    stepY = PLAYER_STEP_HEIGHT_V22;
    if (!PlayerV22_MoveAxisSweep(0.0, stepY, 0.0)) {
        collidedStep = PlayerV22_MoveHorizontalRaw(dx, dz);
        if (!collidedStep) {
            PlayerV22_MoveAxisSweep(0.0, -stepY, 0.0);
            return 0;
        }
    }

    playerX = oldX;
    playerY = oldY;
    playerZ = oldZ;
    if (dx != 0.0) { velocityX = 0.0; }
    if (dz != 0.0) { velocityZ = 0.0; }
    return 1;
}

void PlayerV22_RefreshEnvironment(void)
{
    int bx;
    int by;
    int bz;
    int minX;
    int maxX;
    int minY;
    int maxY;
    int minZ;
    int maxZ;
    int block;

    g_playerInWaterV22 = 0;
    g_playerHeadWaterV22 = 0;
    g_playerInLavaV22 = 0;
    g_playerOnLadderV22 = 0;
    g_playerInWebV22 = 0;
    g_playerInFireV22 = 0;

    minX = (int)floor(playerX - PLAYER_RADIUS);
    maxX = (int)floor(playerX + PLAYER_RADIUS);
    minY = (int)floor(playerY);
    maxY = (int)floor(playerY + PLAYER_HEIGHT);
    minZ = (int)floor(playerZ - PLAYER_RADIUS);
    maxZ = (int)floor(playerZ + PLAYER_RADIUS);

    for (bx = minX; bx <= maxX; bx++) {
        for (by = minY; by <= maxY; by++) {
            for (bz = minZ; bz <= maxZ; bz++) {
                block = GetBlock(bx, by, bz);
                if (block == BLOCK_WATER) { g_playerInWaterV22 = 1; }
                if (block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA) { g_playerInLavaV22 = 1; }
                if (block == BLOCK_LADDER) { g_playerOnLadderV22 = 1; }
                if (block == BLOCK_WEB) { g_playerInWebV22 = 1; }
                if (block == BLOCK_FIRE || block == BLOCK_CACTUS) { g_playerInFireV22 = 1; }
            }
        }
    }
    block = GetBlock((int)floor(playerX), (int)floor(playerY + PlayerV22_GetEyeHeight()), (int)floor(playerZ));
    if (block == BLOCK_WATER) { g_playerHeadWaterV22 = 1; }
}

double PlayerV22_GetEyeHeight(void)
{
    if (g_playerSneakingV22) { return EYE_HEIGHT - 0.10; }
    return EYE_HEIGHT;
}

void PlayerV22_ApproachVelocity(double targetX, double targetZ, double accel, double dt)
{
    double f;
    f = accel * dt;
    if (f > 1.0) { f = 1.0; }
    velocityX += (targetX - velocityX) * f;
    velocityZ += (targetZ - velocityZ) * f;
}

void PlayerV22_ApplyFriction(double amount, double dt)
{
    double f;
    f = 1.0 - amount * dt;
    if (f < 0.0) { f = 0.0; }
    if (f > 1.0) { f = 1.0; }
    velocityX *= f;
    velocityZ *= f;
}

void PlayerV22_HandleMovementInput(double dt)
{
    double yawRad;
    double forwardX;
    double forwardZ;
    double rightX;
    double rightZ;
    double inputX;
    double inputZ;
    double len;
    double speed;
    double accel;

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

    speed = MOVE_SPEED;
    accel = onGround ? PLAYER_GROUND_ACCEL_V22 : PLAYER_AIR_ACCEL_V22;
    if (g_playerSneakingV22 && onGround) { speed *= PLAYER_SNEAK_SPEED_SCALE_V22; }
    if (g_playerInWaterV22) { speed *= WATER_HORIZONTAL_SCALE; accel = PLAYER_WATER_ACCEL_V22; }
    if (g_playerInLavaV22) { speed *= PLAYER_LAVA_SCALE_V22; accel = PLAYER_WATER_ACCEL_V22 * 0.55; }
    if (g_playerInWebV22) { speed *= PLAYER_WEB_SCALE_V22; accel = PLAYER_WATER_ACCEL_V22 * 0.45; }

    if (len > 0.0001) { PlayerV22_ApproachVelocity(inputX * speed, inputZ * speed, accel, dt); }
    else {
        if (onGround) { PlayerV22_ApplyFriction(PLAYER_GROUND_FRICTION_V22, dt); }
        else if (g_playerInWaterV22 || g_playerInLavaV22) { PlayerV22_ApplyFriction(PLAYER_WATER_FRICTION_V22, dt); }
        else { PlayerV22_ApplyFriction(PLAYER_AIR_FRICTION_V22, dt); }
    }

    if (g_playerInWebV22) { velocityX *= 0.55; velocityZ *= 0.55; if (velocityY < -0.65) { velocityY = -0.65; } }

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

    PlayerV22_MoveHorizontal(velocityX * dt, velocityZ * dt);
}

void PlayerV22_UpdatePhysics(double dt)
{
    double oldVelY;
    int hitY;
    if (dt <= 0.0) { return; }
    if (dt > 0.10) { dt = 0.10; }
    PlayerV22_RefreshEnvironment();
    oldVelY = velocityY;

    if (g_playerInWaterV22) {
        velocityY *= 1.0 - ClampDouble(dt * 3.8, 0.0, 0.45);
        if (!keyJump) { velocityY -= WATER_PASSIVE_SINK_ACCEL * dt; }
        if (velocityY < -WATER_DOWN_MAX_SPEED) { velocityY = -WATER_DOWN_MAX_SPEED; }
        g_playerFallDistanceV22 = 0.0;
    } else if (g_playerInLavaV22) {
        velocityY *= 1.0 - ClampDouble(dt * 4.5, 0.0, 0.55);
        if (!keyJump) { velocityY -= 1.25 * dt; }
        if (velocityY < -1.25) { velocityY = -1.25; }
        g_playerFallDistanceV22 = 0.0;
    } else if (g_playerOnLadderV22) {
        if (velocityY < -2.0) { velocityY = -2.0; }
        if (keySneak && velocityY < 0.0) { velocityY = 0.0; }
        g_playerFallDistanceV22 = 0.0;
    } else { velocityY -= GRAVITY * dt; }

    if (g_playerInWebV22 && velocityY < -0.55) { velocityY = -0.55; }

    onGround = 0;
    hitY = PlayerV22_MoveAxisSweep(0.0, velocityY * dt, 0.0);
    if (hitY) {
        if (oldVelY < 0.0) {
            onGround = 1;
            if (!g_playerInWaterV22 && !g_playerInLavaV22 && !g_playerOnLadderV22 && g_playerFallDistanceV22 > 3.0) {
                TakeDamage((int)(g_playerFallDistanceV22 - 3.0));
            }
            g_playerFallDistanceV22 = 0.0;
        }
        velocityY = 0.0;
    } else if (velocityY < 0.0 && !g_playerInWaterV22 && !g_playerInLavaV22 && !g_playerOnLadderV22) {
        g_playerFallDistanceV22 += -velocityY * dt;
    }

    if (g_playerHeadWaterV22) {
        g_playerAirTimer -= dt;
        if (((int)(g_playerAirTimer * 4.0)) != ((int)((g_playerAirTimer + dt) * 4.0))) { SpawnWaterBubbleParticles(playerX, playerY + PlayerV22_GetEyeHeight() - 0.20, playerZ, 3); }
        if (g_playerAirTimer <= 0.0) {
            g_drownDamageTimer -= dt;
            if (g_drownDamageTimer <= 0.0) { TakeDamage(2); SpawnWaterBubbleParticles(playerX, playerY + PlayerV22_GetEyeHeight() - 0.20, playerZ, 9); g_drownDamageTimer = 1.0; }
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

    if (g_playerUseCooldownV22 > 0.0 && !g_playerInFireV22) { g_playerUseCooldownV22 -= dt; if (g_playerUseCooldownV22 < 0.0) { g_playerUseCooldownV22 = 0.0; } }
}

int PlayerV22_CanAttachToBlock(int x, int y, int z)
{
    return PlayerV22_BlockHasCollision(GetBlock(x, y, z));
}

int PlayerV22_CanPlaceBlockAt(int block, int x, int y, int z, int hitX, int hitY, int hitZ)
{
    int oldBlock;
    int support;
    oldBlock = GetBlock(x, y, z);
    if (!IsInsideWorld(x, y, z)) { return 0; }
    if (!BlockV57_IsReplaceableAt(oldBlock, x, y, z)) { return 0; }
    if (!BlockV50_CanPlaceBlockAt(block, x, y, z, hitX, hitY, hitZ)) { return 0; }
    if (block == BLOCK_CACTUS) {
        if (GetBlock(x, y - 1, z) != BLOCK_SAND) { return 0; }
        if (PlayerV22_BlockHasCollision(GetBlock(x + 1, y, z))) { return 0; }
        if (PlayerV22_BlockHasCollision(GetBlock(x - 1, y, z))) { return 0; }
        if (PlayerV22_BlockHasCollision(GetBlock(x, y, z + 1))) { return 0; }
        if (PlayerV22_BlockHasCollision(GetBlock(x, y, z - 1))) { return 0; }
    }
    if (block == BLOCK_REED) {
        support = GetBlock(x, y - 1, z);
        if (support != BLOCK_GRASS && support != BLOCK_DIRT && support != BLOCK_SAND) { return 0; }
        if (GetBlock(x + 1, y - 1, z) != BLOCK_WATER && GetBlock(x - 1, y - 1, z) != BLOCK_WATER &&
            GetBlock(x, y - 1, z + 1) != BLOCK_WATER && GetBlock(x, y - 1, z - 1) != BLOCK_WATER) { return 0; }
    }
    if (block == BLOCK_TORCH || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) { if (!PlayerV22_CanAttachToBlock(hitX, hitY, hitZ)) { return 0; } }
    if (block == BLOCK_LADDER || block == BLOCK_SIGN_WALL) { if (!PlayerV22_CanAttachToBlock(hitX, hitY, hitZ)) { return 0; } }
    if (block == BLOCK_WOOD_DOOR) {
        if (!PlayerV22_BlockHasCollision(GetBlock(x, y - 1, z))) { return 0; }
        if (!IsInsideWorld(x, y + 1, z)) { return 0; }
        if (!PlayerV22_IsReplaceableBlock(GetBlock(x, y + 1, z))) { return 0; }
    }
    g_blockV50SuppressNeighborNotify++;
    SetBlock(x, y, z, block);
    if (PlayerV22_AabbCollidesAt(playerX, playerY, playerZ)) { SetBlock(x, y, z, oldBlock); g_blockV50SuppressNeighborNotify--; return 0; }
    SetBlock(x, y, z, oldBlock);
    g_blockV50SuppressNeighborNotify--;
    return 1;
}

void PlayerV22_ApplyPlacementMetadata(int block, int x, int y, int z, int hitX, int hitY, int hitZ)
{
    int sideX;
    int sideY;
    int sideZ;
    unsigned char meta;
    sideX = x - hitX;
    sideY = y - hitY;
    sideZ = z - hitZ;
    meta = 0;
    if (block == BLOCK_TORCH || block == BLOCK_REDSTONE_TORCH_ON || block == BLOCK_REDSTONE_TORCH_OFF) {
        if (sideY > 0) { meta = 5; }
        else if (sideX > 0) { meta = 1; }
        else if (sideX < 0) { meta = 2; }
        else if (sideZ > 0) { meta = 3; }
        else if (sideZ < 0) { meta = 4; }
        g_blockMeta[x][y][z] = meta;
    } else if (block == BLOCK_LADDER || block == BLOCK_SIGN_WALL) {
        if (sideZ > 0) { meta = 2; }
        else if (sideZ < 0) { meta = 3; }
        else if (sideX > 0) { meta = 4; }
        else if (sideX < 0) { meta = 5; }
        g_blockMeta[x][y][z] = meta;
    } else if (block == BLOCK_WOOD_DOOR) {
        if (fabs(sin(yaw * PI / 180.0)) > fabs(cos(yaw * PI / 180.0))) { meta = sin(yaw * PI / 180.0) > 0.0 ? 1 : 3; }
        else { meta = cos(yaw * PI / 180.0) > 0.0 ? 2 : 0; }
        g_blockMeta[x][y][z] = meta;
        if (IsInsideWorld(x, y + 1, z)) { SetBlock(x, y + 1, z, BLOCK_WOOD_DOOR); g_blockMeta[x][y + 1][z] = (unsigned char)(meta | 8); }
    }
    BlockV50_ApplyPlacementMetadata(block, x, y, z, hitX, hitY, hitZ);
}


/* ------------------------------------------------------------ */
/* Block mining and placing                                     */
/* ------------------------------------------------------------ */

void BreakBlockRaycast(void)
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
    double vx;
    double vz;
    int bx;
    int by;
    int bz;
    int block;
    int item;
    int dropCount;
    int hash;

    if (AttackMobRaycast()) {
        StartHandSwing();
        return;
    }

    yawRad = yaw * PI / 180.0;
    pitchRad = pitch * PI / 180.0;

    dirX = -sin(yawRad) * cos(pitchRad);
    dirY = sin(pitchRad);
    dirZ = -cos(yawRad) * cos(pitchRad);

    startX = playerX;
    startY = playerY + EYE_HEIGHT;
    startZ = playerZ;

    for (t = 0.0; t < RAY_DISTANCE; t += RAY_STEP) {
        bx = (int)floor(startX + dirX * t);
        by = (int)floor(startY + dirY * t);
        bz = (int)floor(startZ + dirZ * t);

        block = GetBlock(bx, by, bz);

        if (block != BLOCK_AIR &&
            block != BLOCK_BORDER &&
            block != BLOCK_WATER) {

            if (!ItemCombatV6_ShouldFinishMine(block, bx, by, bz)) {
                PlayBlockBreakSound(block);
                SpawnBlockBreakParticles(bx, by, bz, block);
                StartHandSwing();
                return;
            }

            dropCount = 1;
            item = GetBlockDropItemAt(block, bx, by, bz, &dropCount);
            if (!ItemCombatV6_CanHarvestBlock(block, hotbar[selectedHotbarSlot].item)) {
                item = ITEM_NONE;
                dropCount = 0;
            }

            PlayBlockBreakSound(block);
            SpawnBlockBreakParticles(bx, by, bz, block);
            RemoveTileEntityAt(bx, by, bz);
            SetBlock(bx, by, bz, BLOCK_AIR);
            SpawnFallingBlockIfNeededV6(bx, by + 1, bz);
            DamageHeldTool(1);
            UpdateRedstoneAround(bx, by, bz);

            if (item != ITEM_NONE && dropCount > 0) {
                hash = WorldHash3D(bx, by, bz, g_worldSeed + 55310);
                vx = (((hash & 255) / 255.0) - 0.5) * 1.4;
                vz = ((((hash >> 8) & 255) / 255.0) - 0.5) * 1.4;
                /* Java Block.dropBlockAsItem_do spawns inside the block with a small random offset. */
                AddDroppedItem(item, dropCount,
                               (double)bx + 0.25 + (double)((hash >> 16) & 255) / 510.0,
                               (double)by + 0.25 + (double)((hash >> 24) & 127) / 254.0,
                               (double)bz + 0.25 + (double)((hash >> 4) & 255) / 510.0,
                               vx, 1.85, vz);
            }

            RecomputeLegacyLightingLocal(bx, by, bz, 18);

            StartHandSwing();

            return;
        }
    }
}





void PlaceBlockRaycast(void)
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

    int bx;
    int by;
    int bz;

    int lastAirX;
    int lastAirY;
    int lastAirZ;

    int block;
    int oldBlock;
    int placeBlock;

    InventorySlot *selectedSlot;

    selectedSlot = &hotbar[selectedHotbarSlot];

    yawRad = yaw * PI / 180.0;
    pitchRad = pitch * PI / 180.0;
    dirX = -sin(yawRad) * cos(pitchRad);
    dirY = sin(pitchRad);
    dirZ = -cos(yawRad) * cos(pitchRad);
    startX = playerX;
    startY = playerY + EYE_HEIGHT;
    startZ = playerZ;

    /* First handle interactive blocks.  This lets an empty hand open the
       crafting table when the crosshair points at one. */
    for (t = 0.0; t < RAY_DISTANCE; t += RAY_STEP) {
        bx = (int)floor(startX + dirX * t);
        by = (int)floor(startY + dirY * t);
        bz = (int)floor(startZ + dirZ * t);
        block = GetBlock(bx, by, bz);

        if (TryActivateTileOrRedstoneV5(bx, by, bz)) {
            return;
        }

        if (block != BLOCK_AIR && block != BLOCK_WATER) {
            break;
        }
    }

    if (selectedSlot->item == ITEM_NONE || selectedSlot->count <= 0) {
        return;
    }

    if (ItemCombatV6_TryUseSelectedItem()) {
        return;
    }

    placeBlock = ItemToBlock(selectedSlot->item);

    if (placeBlock == BLOCK_AIR || placeBlock == BLOCK_BORDER) {
        return;
    }

    lastAirX = -1;
    lastAirY = -1;
    lastAirZ = -1;

    for (t = 0.0; t < RAY_DISTANCE; t += RAY_STEP) {
        bx = (int)floor(startX + dirX * t);
        by = (int)floor(startY + dirY * t);
        bz = (int)floor(startZ + dirZ * t);

        block = GetBlock(bx, by, bz);

        if (block == BLOCK_AIR || block == BLOCK_WATER) {
            lastAirX = bx;
            lastAirY = by;
            lastAirZ = bz;
        } else {
            /* V57 Priority 2: Java BlockStep/BlockSnow same-block placement. */
            if (placeBlock == BLOCK_STEP && block == BLOCK_STEP) {
                if (IsInsideWorld(bx, by, bz) && !(g_blockMeta[bx][by][bz] & 8)) {
                    SetBlock(bx, by, bz, BLOCK_DOUBLE_STEP);
                    RemoveItemFromSelectedHotbar(1);
                    RecomputeLegacyLightingLocal(bx, by, bz, 12);
                    StartHandUse();
                }
                return;
            }
            if (placeBlock == BLOCK_SNOW && block == BLOCK_SNOW) {
                if (IsInsideWorld(bx, by, bz) && (g_blockMeta[bx][by][bz] & 7) < 7) {
                    g_blockMeta[bx][by][bz] = (unsigned char)((g_blockMeta[bx][by][bz] & 7) + 1);
                    InvalidateTerrainChunkMeshAt(bx, bz);
                    RemoveItemFromSelectedHotbar(1);
                    StartHandUse();
                }
                return;
            }
            if (lastAirX >= 0) {
                oldBlock = GetBlock(lastAirX, lastAirY, lastAirZ);

                if (!PlayerV22_CanPlaceBlockAt(placeBlock, lastAirX, lastAirY, lastAirZ, bx, by, bz)) {
                    return;
                }

                SetBlock(lastAirX, lastAirY, lastAirZ, placeBlock);
                PlayerV22_ApplyPlacementMetadata(placeBlock, lastAirX, lastAirY, lastAirZ, bx, by, bz);
                if (!BlockV50_CanBlockStayAt(placeBlock, lastAirX, lastAirY, lastAirZ)) { SetBlock(lastAirX, lastAirY, lastAirZ, oldBlock); return; }
                SpawnFallingBlockIfNeededV6(lastAirX, lastAirY + 1, lastAirZ);
                if (placeBlock == BLOCK_PISTON || placeBlock == BLOCK_PISTON_STICKY) {
                    if (fabs(dirX) > fabs(dirZ)) { g_blockMeta[lastAirX][lastAirY][lastAirZ] = dirX < 0.0 ? 0 : 1; }
                    else { g_blockMeta[lastAirX][lastAirY][lastAirZ] = dirZ < 0.0 ? 2 : 3; }
                }
                EnsureTileEntityForBlock(placeBlock, lastAirX, lastAirY, lastAirZ);
                UpdateRedstoneAround(lastAirX, lastAirY, lastAirZ);

                if (PlayerCollidesAt(playerX, playerY, playerZ)) {
                    SetBlock(lastAirX, lastAirY, lastAirZ, oldBlock);
                    return;
                }

                RemoveItemFromSelectedHotbar(1);
                RecomputeLegacyLightingLocal(lastAirX, lastAirY, lastAirZ, 18);
                StartHandUse();
            }

            return;
        }
    }
}



void PlayerV22_ApplyCameraBob(void)
{
    double walk;
    double bob;
    double bob2;
    if (g_cameraMode != CAMERA_FIRST_PERSON) { return; }
    if (g_playerSneakingV22) { return; }
    walk = g_playerWalkAmount;
    if (walk <= 0.001) { return; }
    if (walk > 1.0) { walk = 1.0; }
    bob = sin(handBob * 0.50) * walk;
    bob2 = cos(handBob) * walk;
    glTranslatef((float)(bob * 0.035), (float)(fabs(bob2) * -0.035), 0.0f);
    glRotatef((float)(bob * 1.6), 0.0f, 0.0f, 1.0f);
    glRotatef((float)(fabs(bob2) * 1.2), 1.0f, 0.0f, 0.0f);
}

void PlayerV22_ClipThirdPersonCamera(double eyeX, double eyeY, double eyeZ, double *camX, double *camY, double *camZ)
{
    double dx;
    double dy;
    double dz;
    double t;
    double step;
    double sx;
    double sy;
    double sz;
    double lastX;
    double lastY;
    double lastZ;
    int bx;
    int by;
    int bz;
    int block;
    dx = *camX - eyeX;
    dy = *camY - eyeY;
    dz = *camZ - eyeZ;
    lastX = eyeX;
    lastY = eyeY;
    lastZ = eyeZ;
    step = 0.18;
    for (t = step; t <= 1.0; t += step) {
        sx = eyeX + dx * t;
        sy = eyeY + dy * t;
        sz = eyeZ + dz * t;
        bx = (int)floor(sx);
        by = (int)floor(sy);
        bz = (int)floor(sz);
        block = GetBlock(bx, by, bz);
        if (PlayerV22_BlockHasCollision(block)) {
            *camX = lastX;
            *camY = lastY;
            *camZ = lastZ;
            return;
        }
        lastX = sx;
        lastY = sy;
        lastZ = sz;
    }
}

/* ------------------------------------------------------------ */
/* Camera and world rendering                                   */
/* ------------------------------------------------------------ */

void SetupCamera(void)
{
    double yawRad;
    double pitchRad;
    double dirX;
    double dirY;
    double dirZ;
    double eyeX;
    double eyeY;
    double eyeZ;
    double camX;
    double camY;
    double camZ;
    double dist;

    if (g_cameraMode == CAMERA_FIRST_PERSON) {
        ApplyDamageCameraWobble();
        glRotatef((float)-pitch, 1.0f, 0.0f, 0.0f);
        glRotatef((float)-yaw, 0.0f, 1.0f, 0.0f);
        glTranslatef((float)-playerX, (float)-(playerY + EYE_HEIGHT), (float)-playerZ);
        return;
    }

    yawRad = yaw * PI / 180.0;
    pitchRad = pitch * PI / 180.0;
    dirX = -sin(yawRad) * cos(pitchRad);
    dirY = sin(pitchRad);
    dirZ = -cos(yawRad) * cos(pitchRad);

    eyeX = playerX;
    eyeY = playerY + EYE_HEIGHT;
    eyeZ = playerZ;
    dist = 5.0;

    if (g_cameraMode == CAMERA_THIRD_BACK) {
        camX = eyeX - dirX * dist;
        camY = eyeY - dirY * dist + 0.85;
        camZ = eyeZ - dirZ * dist;
    } else {
        camX = eyeX + dirX * dist;
        camY = eyeY + 0.60;
        camZ = eyeZ + dirZ * dist;
    }

    if (camY < playerY + 0.8) {
        camY = playerY + 0.8;
    }

    gluLookAt(camX, camY, camZ, eyeX, eyeY, eyeZ, 0.0, 1.0, 0.0);
}

void RenderWorld(void)
{
    int px;
    int pz;
    int pcx;
    int pcz;
    int renderChunks;
    int builds;
    int budget;
    int i;
    int cx;
    int cz;
    int dx;
    int dz;
    int mustBuildNear;
    DWORD passStartMs;
    DWORD nowMs;

    px = (int)playerX;
    pz = (int)playerZ;
    pcx = px / CHUNK_SIZE;
    pcz = pz / CHUNK_SIZE;

    renderChunks = g_renderDistanceChunks;
    if (renderChunks < RENDER_DISTANCE_MIN_CHUNKS) { renderChunks = RENDER_DISTANCE_MIN_CHUNKS; }
    if (renderChunks > RENDER_DISTANCE_MAX_CHUNKS) { renderChunks = RENDER_DISTANCE_MAX_CHUNKS; }

    if (terrainChunkMeshOriginX != worldOriginBlockX ||
        terrainChunkMeshOriginZ != worldOriginBlockZ) {
        if (g_renderV47DisableStaleAfterOriginShift) {
            DeleteTerrainChunkMeshes();
            g_renderV47InitialBuildBoostFrames = 8;
        }
        InvalidateAllTerrainChunkMeshes();
    }

    RendererV33_BeginFrame();
    RendererV47_UpdateFrustumFromOpenGL();
    RendererV33_BuildChunkQueues(pcx, pcz, renderChunks);
    budget = RendererV33_GetMeshBuildBudget();
    if (g_renderV47InitialBuildBoostFrames > 0) {
        budget += 4;
        g_renderV47InitialBuildBoostFrames--;
    }
    builds = 0;
    passStartMs = GetTickCount();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glEnable(GL_CULL_FACE);

    /* RenderSorter/WorldRenderer style: sorted nearest first for solid terrain.
       Dirty chunks with existing display lists are drawn stale when the budget
       is gone; missing nearby chunks are allowed to use the budget first. */
    for (i = 0; i < g_renderOpaqueCountV33; i++) {
        cx = g_renderOpaqueQueueV33[i].cx;
        cz = g_renderOpaqueQueueV33[i].cz;
        dx = cx - pcx;
        dz = cz - pcz;
        mustBuildNear = (dx * dx + dz * dz <= RENDER_V33_NEAR_BUILD_RADIUS_CHUNKS * RENDER_V33_NEAR_BUILD_RADIUS_CHUNKS) ? 1 : 0;

        if (terrainChunkDirty[cx][cz] || !terrainChunkLists[cx][cz]) {
            if ((builds < budget || (mustBuildNear && !terrainChunkLists[cx][cz] && builds < budget + 2)) &&
