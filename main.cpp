// =============================================================================
//  Galactic Defender — main.cpp
//  2D Space Survival Game | C++ + OpenGL (FreeGLUT)
//
//  Course    : CSE 402 — Computer Graphics
//  Institution: Metropolitan University, Sylhet
//  Submitted  : Shrabanti Chowdhury, Senior Lecturer
//  Team       : Eshrath Jahan Esha (231-115-041)
//               Niyaz Ahmad Khan   (231-115-052)
//
//  Controls
//  --------
//    A / D   — Move spaceship left / right
//    SPACE   — Fire laser
//    R       — Restart after Game Over
//    ESC     — Quit
//
//  Build (Desktop / Linux / macOS)
//  --------------------------------
//    g++ main.cpp -o galactic_defender -lGL -lGLU -lglut
//
//  Build (WebAssembly via Emscripten)
//  ------------------------------------
//    emcc main.cpp -o public/index.html \
//         -s USE_FREEGLUT=1             \
//         -s LEGACY_GL_EMULATION=1
// =============================================================================

#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// =============================================================================
// Global constants
// =============================================================================
static const int   WIN_W       = 800;
static const int   WIN_H       = 600;
static const float PI          = 3.14159265f;
static const int   MAX_HEALTH  = 100;

// Per-entity array capacities (fixed pools — no heap allocation at runtime)
static const int CAP_BULLETS   = 64;
static const int CAP_ASTEROIDS = 128;
static const int CAP_PARTICLES = 256;
static const int CAP_STARS     = 150;

// Timing
static const float DT               = 1.0f / 60.0f;   // fixed timestep (s)
static const float SHOOT_COOLDOWN   = 0.25f;           // minimum seconds between shots
static const float SPAWN_INTERVAL_0 = 1.2f;            // initial asteroid spawn period (s)
static const float SPAWN_INTERVAL_MIN = 0.40f;         // fastest spawn rate cap (s)

// =============================================================================
// Utility helpers
// =============================================================================

// Draw a bitmap string at 2-D position (x, y).
static void drawString(float x, float y, const char* text,
                       void* font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (const char* c = text; *c; ++c)
        glutBitmapCharacter(font, *c);
}

// Axis-Aligned Bounding Box overlap test.
// Returns true if box A (centre ax,ay, half-extents ahw,ahh)
// overlaps box B (centre bx,by, half-extents bhw,bhh).
static bool aabbOverlap(float ax, float ay, float ahw, float ahh,
                        float bx, float by, float bhw, float bhh)
{
    return (fabsf(ax - bx) < ahw + bhw) &&
           (fabsf(ay - by) < ahh + bhh);
}

// =============================================================================
// Starfield (decorative background)
// =============================================================================
struct Star { float x, y, b; };   // b = brightness [0..1]
static Star g_stars[CAP_STARS];

static void initStars()
{
    for (int i = 0; i < CAP_STARS; ++i) {
        g_stars[i].x = (float)(rand() % WIN_W);
        g_stars[i].y = (float)(rand() % WIN_H);
        g_stars[i].b = 0.3f + 0.7f * ((float)rand() / RAND_MAX);
    }
}

static void renderStars()
{
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < CAP_STARS; ++i) {
        glColor3f(g_stars[i].b, g_stars[i].b, g_stars[i].b);
        glVertex2f(g_stars[i].x, g_stars[i].y);
    }
    glEnd();
    glPointSize(1.0f);
}

// =============================================================================
// Player Spaceship
//   Shape    : GL_TRIANGLES  (cyan triangle)
//   Bounding : axis-aligned box [-halfW, +halfW] x [-halfH, +halfH]
// =============================================================================
struct Player {
    float x, y;          // centre position
    float halfW, halfH;  // bounding half-extents
    float speed;
    bool  moveLeft, moveRight;
};
static Player g_ship;

static void initPlayer()
{
    g_ship.x         = WIN_W / 2.0f;
    g_ship.y         = 50.0f;
    g_ship.halfW     = 20.0f;
    g_ship.halfH     = 25.0f;
    g_ship.speed     = 5.0f;
    g_ship.moveLeft  = false;
    g_ship.moveRight = false;
}

// Render a cyan triangle for the player spaceship.
// The triangle points upward; its three vertices are:
//   Top      : (cx, cy + halfH)
//   Bot-left : (cx - halfW, cy - halfH)
//   Bot-right: (cx + halfW, cy - halfH)
static void renderPlayer()
{
    float cx = g_ship.x, cy = g_ship.y;
    float hw = g_ship.halfW, hh = g_ship.halfH;

    // --- Main hull: GL_TRIANGLES (cyan) ---
    glColor3f(0.0f, 1.0f, 1.0f);          // cyan
    glBegin(GL_TRIANGLES);
        glVertex2f(cx,      cy + hh);      // nose tip
        glVertex2f(cx - hw, cy - hh);     // bottom-left
        glVertex2f(cx + hw, cy - hh);     // bottom-right
    glEnd();

    // --- Cockpit accent: smaller darker triangle ---
    glColor3f(0.0f, 0.5f, 0.8f);
    glBegin(GL_TRIANGLES);
        glVertex2f(cx,           cy + hh * 0.3f);
        glVertex2f(cx - hw * 0.35f, cy - hh * 0.1f);
        glVertex2f(cx + hw * 0.35f, cy - hh * 0.1f);
    glEnd();

    // --- Outline: GL_LINE_LOOP ---
    glColor3f(0.5f, 1.0f, 1.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(cx,      cy + hh);
        glVertex2f(cx - hw, cy - hh);
        glVertex2f(cx + hw, cy - hh);
    glEnd();
    glLineWidth(1.0f);
}

// =============================================================================
// Laser Projectiles
//   Shape : GL_LINES  (yellow)
// =============================================================================
struct Bullet {
    float x, y;
    float vy;      // upward velocity (px/frame)
    bool  active;
};
static Bullet g_bullets[CAP_BULLETS];

static void initBullets()
{
    for (int i = 0; i < CAP_BULLETS; ++i) g_bullets[i].active = false;
}

static void spawnBullet(float x, float y)
{
    for (int i = 0; i < CAP_BULLETS; ++i) {
        if (!g_bullets[i].active) {
            g_bullets[i] = {x, y, 12.0f, true};
            return;
        }
    }
}

// Render each active laser as a yellow GL_LINES segment.
static void renderBullets()
{
    glLineWidth(2.5f);
    for (int i = 0; i < CAP_BULLETS; ++i) {
        if (!g_bullets[i].active) continue;
        glColor3f(1.0f, 1.0f, 0.0f);          // yellow
        glBegin(GL_LINES);
            glVertex2f(g_bullets[i].x, g_bullets[i].y);
            glVertex2f(g_bullets[i].x, g_bullets[i].y + 14.0f);
        glEnd();
    }
    glLineWidth(1.0f);
}

// =============================================================================
// Asteroids
//   Shape : GL_QUADS  (red squares)
// =============================================================================
struct Asteroid {
    float x, y;
    float half;       // half side length (bounding box = half*2 per axis)
    float vy;         // downward speed (px/frame)
    bool  active;
};
static Asteroid g_asteroids[CAP_ASTEROIDS];

static void initAsteroids()
{
    for (int i = 0; i < CAP_ASTEROIDS; ++i) g_asteroids[i].active = false;
}

static void spawnAsteroid(float speedMult)
{
    for (int i = 0; i < CAP_ASTEROIDS; ++i) {
        if (!g_asteroids[i].active) {
            float h = 10.0f + (float)(rand() % 18);
            g_asteroids[i].x      = h + (float)(rand() % (int)(WIN_W - 2.0f * h));
            g_asteroids[i].y      = (float)WIN_H + h;
            g_asteroids[i].half   = h;
            g_asteroids[i].vy     = (1.5f + (float)(rand() % 25) * 0.1f) * speedMult;
            g_asteroids[i].active = true;
            return;
        }
    }
}

// Render each active asteroid as a red GL_QUADS square.
static void renderAsteroids()
{
    for (int i = 0; i < CAP_ASTEROIDS; ++i) {
        if (!g_asteroids[i].active) continue;
        float ax = g_asteroids[i].x, ay = g_asteroids[i].y;
        float h  = g_asteroids[i].half;

        glColor3f(1.0f, 0.25f, 0.25f);        // red
        glBegin(GL_QUADS);
            glVertex2f(ax - h, ay - h);        // bottom-left
            glVertex2f(ax + h, ay - h);        // bottom-right
            glVertex2f(ax + h, ay + h);        // top-right
            glVertex2f(ax - h, ay + h);        // top-left
        glEnd();

        // Lighter border for visual depth
        glColor3f(1.0f, 0.55f, 0.55f);
        glLineWidth(1.2f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(ax - h, ay - h);
            glVertex2f(ax + h, ay - h);
            glVertex2f(ax + h, ay + h);
            glVertex2f(ax - h, ay + h);
        glEnd();
        glLineWidth(1.0f);
    }
}

// =============================================================================
// Explosion Particles (visual feedback on hit / death)
// =============================================================================
struct Particle {
    float x, y, vx, vy, life;   // life: 1→0
    float r, g, b;
    bool  active;
};
static Particle g_particles[CAP_PARTICLES];

static void initParticles()
{
    for (int i = 0; i < CAP_PARTICLES; ++i) g_particles[i].active = false;
}

static void spawnExplosion(float x, float y,
                           float r, float g, float b, int count)
{
    int n = 0;
    for (int i = 0; i < CAP_PARTICLES && n < count; ++i) {
        if (!g_particles[i].active) {
            float angle = 2.0f * PI * ((float)rand() / RAND_MAX);
            float spd   = 0.5f + 3.5f * ((float)rand() / RAND_MAX);
            g_particles[i] = {x, y,
                               spd * cosf(angle), spd * sinf(angle),
                               1.0f, r, g, b, true};
            ++n;
        }
    }
}

static void updateParticles()
{
    for (int i = 0; i < CAP_PARTICLES; ++i) {
        if (!g_particles[i].active) continue;
        g_particles[i].x    += g_particles[i].vx;
        g_particles[i].y    += g_particles[i].vy;
        g_particles[i].life -= 0.030f;
        if (g_particles[i].life <= 0.0f) g_particles[i].active = false;
    }
}

static void renderParticles()
{
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < CAP_PARTICLES; ++i) {
        if (!g_particles[i].active) continue;
        float l = g_particles[i].life;
        glColor4f(g_particles[i].r, g_particles[i].g, g_particles[i].b, l);
        glVertex2f(g_particles[i].x, g_particles[i].y);
    }
    glEnd();
    glPointSize(1.0f);
}

// =============================================================================
// HUD (Heads-Up Display)
//   Draws score and health overlay using glutBitmapCharacter.
// =============================================================================
static void renderHUD(int score, int health)
{
    char buf[64];

    // --- Top bar background ---
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
        glVertex2f(0,       WIN_H);
        glVertex2f(WIN_W,   WIN_H);
        glVertex2f(WIN_W,   WIN_H - 38.0f);
        glVertex2f(0,       WIN_H - 38.0f);
    glEnd();

    // Score (left) — uses glutBitmapCharacter internally via drawString
    glColor3f(0.3f, 1.0f, 0.9f);
    snprintf(buf, sizeof(buf), "SCORE  %d", score);
    drawString(12.0f, WIN_H - 25.0f, buf, GLUT_BITMAP_HELVETICA_18);

    // Health label (right)
    glColor3f(1.0f, 1.0f, 1.0f);
    drawString(WIN_W - 215.0f, WIN_H - 25.0f, "HULL", GLUT_BITMAP_HELVETICA_18);

    // Numeric health value
    snprintf(buf, sizeof(buf), "%d%%", health);
    glColor3f(health > 50 ? 0.3f : 1.0f,
              health > 50 ? 1.0f : 0.3f, 0.0f);
    drawString(WIN_W - 55.0f, WIN_H - 25.0f, buf, GLUT_BITMAP_HELVETICA_18);

    // Health bar background
    float bx = WIN_W - 155.0f, by = WIN_H - 32.0f, bw = 90.0f, bh = 13.0f;
    glColor3f(0.25f, 0.25f, 0.25f);
    glBegin(GL_QUADS);
        glVertex2f(bx,      by);
        glVertex2f(bx + bw, by);
        glVertex2f(bx + bw, by + bh);
        glVertex2f(bx,      by + bh);
    glEnd();

    // Health bar fill — colour shifts green → red
    float pct  = (float)health / MAX_HEALTH;
    float fill = bw * pct;
    glColor3f(1.0f - pct, pct * 0.85f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(bx,        by);
        glVertex2f(bx + fill, by);
        glVertex2f(bx + fill, by + bh);
        glVertex2f(bx,        by + bh);
    glEnd();

    // Health bar border
    glColor3f(0.6f, 0.6f, 0.6f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(bx,      by);
        glVertex2f(bx + bw, by);
        glVertex2f(bx + bw, by + bh);
        glVertex2f(bx,      by + bh);
    glEnd();
}

static void renderGameOver(int score)
{
    // Dim overlay
    glColor4f(0.0f, 0.0f, 0.05f, 0.80f);
    glBegin(GL_QUADS);
        glVertex2f(0,     0);
        glVertex2f(WIN_W, 0);
        glVertex2f(WIN_W, WIN_H);
        glVertex2f(0,     WIN_H);
    glEnd();

    char buf[64];

    // Title
    glColor3f(1.0f, 0.15f, 0.15f);
    drawString(WIN_W / 2.0f - 90.0f, WIN_H / 2.0f + 30.0f,
               "GAME  OVER", GLUT_BITMAP_TIMES_ROMAN_24);

    // Final score
    glColor3f(1.0f, 1.0f, 1.0f);
    snprintf(buf, sizeof(buf), "Final Score: %d", score);
    drawString(WIN_W / 2.0f - 80.0f, WIN_H / 2.0f - 5.0f,
               buf, GLUT_BITMAP_HELVETICA_18);

    // Restart hint
    glColor3f(0.55f, 0.55f, 0.55f);
    drawString(WIN_W / 2.0f - 100.0f, WIN_H / 2.0f - 38.0f,
               "Press  R  to  restart", GLUT_BITMAP_HELVETICA_12);
}

// =============================================================================
// Game State
// =============================================================================
static int   g_score        = 0;
static int   g_health       = MAX_HEALTH;
static bool  g_gameOver     = false;
static float g_time         = 0.0f;   // elapsed seconds
static float g_shootTimer   = 0.0f;   // time since last shot
static float g_spawnTimer   = 0.0f;   // time since last asteroid spawn
static bool  g_holdFire     = false;  // is SPACE currently held?

// ---------------------------------------------------------------------------
// Difficulty scaling
//   Asteroid speed multiplier grows from 1.0× to 2.5× over 120 s.
//   Spawn interval shrinks from 1.2 s to a floor of 0.4 s.
// ---------------------------------------------------------------------------
static float diffMult()
{
    float d = 1.0f + (g_time / 120.0f) * 1.5f;
    return d > 2.5f ? 2.5f : d;
}

static float spawnInterval()
{
    float iv = SPAWN_INTERVAL_0 / diffMult();
    return iv < SPAWN_INTERVAL_MIN ? SPAWN_INTERVAL_MIN : iv;
}

// ---------------------------------------------------------------------------
// Full game reset
// ---------------------------------------------------------------------------
static void resetGame()
{
    g_score      = 0;
    g_health     = MAX_HEALTH;
    g_gameOver   = false;
    g_time       = 0.0f;
    g_shootTimer = 0.0f;
    g_spawnTimer = 0.0f;
    g_holdFire   = false;
    initPlayer();
    initBullets();
    initAsteroids();
    initParticles();
}

// =============================================================================
// Core update loop  (called at ~60 fps)
// =============================================================================
static void update()
{
    if (g_gameOver) return;

    g_time       += DT;
    g_shootTimer += DT;
    g_spawnTimer += DT;

    // --- Player movement (smooth hold-to-move) ---
    if (g_ship.moveLeft  && g_ship.x - g_ship.halfW > 0)
        g_ship.x -= g_ship.speed;
    if (g_ship.moveRight && g_ship.x + g_ship.halfW < WIN_W)
        g_ship.x += g_ship.speed;

    // --- Auto-fire while SPACE held (respects cooldown) ---
    if (g_holdFire && g_shootTimer >= SHOOT_COOLDOWN) {
        spawnBullet(g_ship.x, g_ship.y + g_ship.halfH);
        g_shootTimer = 0.0f;
    }

    // --- Bullet kinematics ---
    for (int i = 0; i < CAP_BULLETS; ++i) {
        if (!g_bullets[i].active) continue;
        g_bullets[i].y += g_bullets[i].vy;                    // linear velocity
        if (g_bullets[i].y > WIN_H + 20.0f)
            g_bullets[i].active = false;
    }

    // --- Spawn new asteroids ---
    if (g_spawnTimer >= spawnInterval()) {
        spawnAsteroid(diffMult());
        g_spawnTimer = 0.0f;
    }

    // --- Asteroid kinematics + collision detection ---
    for (int i = 0; i < CAP_ASTEROIDS; ++i) {
        if (!g_asteroids[i].active) continue;

        // Linear downward velocity
        g_asteroids[i].y -= g_asteroids[i].vy;

        // Passed below screen → player takes hull damage (missed asteroid)
        if (g_asteroids[i].y + g_asteroids[i].half < 0.0f) {
            g_asteroids[i].active = false;
            g_health -= 10;
            continue;
        }

        // --- Bounding-box collision: bullet vs asteroid ---
        for (int j = 0; j < CAP_BULLETS; ++j) {
            if (!g_bullets[j].active) continue;
            if (aabbOverlap(g_bullets[j].x,    g_bullets[j].y + 7.0f, 2.0f, 7.0f,
                            g_asteroids[i].x,  g_asteroids[i].y,
                            g_asteroids[i].half, g_asteroids[i].half))
            {
                g_bullets[j].active   = false;
                g_asteroids[i].active = false;
                g_score += (int)(10.0f * diffMult());
                spawnExplosion(g_asteroids[i].x, g_asteroids[i].y,
                               1.0f, 0.55f, 0.1f, 16);
                break;
            }
        }

        // --- Bounding-box collision: ship vs asteroid ---
        if (g_asteroids[i].active &&
            aabbOverlap(g_ship.x, g_ship.y,
                        g_ship.halfW * 0.80f, g_ship.halfH * 0.80f,
                        g_asteroids[i].x, g_asteroids[i].y,
                        g_asteroids[i].half * 0.80f, g_asteroids[i].half * 0.80f))
        {
            g_asteroids[i].active = false;
            g_health -= 20;
            spawnExplosion(g_ship.x, g_ship.y, 0.9f, 0.4f, 0.0f, 12);
        }
    }

    // --- Particle system update ---
    updateParticles();

    // --- Death check ---
    if (g_health <= 0) {
        g_health  = 0;
        g_gameOver = true;
        spawnExplosion(g_ship.x, g_ship.y, 1.0f, 0.2f, 0.0f, 40);
    }

    glutPostRedisplay();
}

// =============================================================================
// GLUT Display callback
// =============================================================================
static void display()
{
    glClearColor(0.03f, 0.03f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderStars();
    renderParticles();

    if (!g_gameOver) {
        renderBullets();          // GL_LINES  — yellow lasers
        renderAsteroids();        // GL_QUADS  — red squares
        renderPlayer();           // GL_TRIANGLES — cyan triangle
        renderHUD(g_score, g_health);
    } else {
        renderAsteroids();        // still drift in background
        renderGameOver(g_score);
    }

    glutSwapBuffers();
}

// =============================================================================
// GLUT Keyboard callbacks
// =============================================================================
static void keyDown(unsigned char key, int /*x*/, int /*y*/)
{
    switch (key) {
        case 'a': case 'A': g_ship.moveLeft  = true;  break;
        case 'd': case 'D': g_ship.moveRight = true;  break;
        case ' ':
            if (!g_gameOver && g_shootTimer >= SHOOT_COOLDOWN) {
                spawnBullet(g_ship.x, g_ship.y + g_ship.halfH);
                g_shootTimer = 0.0f;
            }
            g_holdFire = true;
            break;
        case 'r': case 'R':
            if (g_gameOver) resetGame();
            break;
        case 27:   // ESC
            exit(0);
    }
}

static void keyUp(unsigned char key, int /*x*/, int /*y*/)
{
    switch (key) {
        case 'a': case 'A': g_ship.moveLeft  = false; break;
        case 'd': case 'D': g_ship.moveRight = false; break;
        case ' ':            g_holdFire       = false; break;
    }
}

// =============================================================================
// Desktop-only timer callback — drives the update loop at ~60 fps
// =============================================================================
static void timerCB(int /*val*/)
{
    update();
    glutTimerFunc(16, timerCB, 0);
}

// =============================================================================
// Emscripten main-loop shim
//   emscripten_set_main_loop requires a void(*)(void) callback.
//   It replaces glutMainLoop() in the web build so that the browser
//   drives animation via requestAnimationFrame.
// =============================================================================
static void emLoop()
{
    update();
    display();
}

// =============================================================================
// main
// =============================================================================
int main(int argc, char** argv)
{
    srand((unsigned int)time(nullptr));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Galactic Defender");

    // 2-D orthographic projection — one unit equals one pixel
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Alpha blending for particles and overlay effects
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Seed game world
    initStars();
    resetGame();

    // Register GLUT callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);

    // ------------------------------------------------------------------
    // Platform-specific main loop
    //
    //   #ifdef __EMSCRIPTEN__
    //     Use emscripten_set_main_loop() so the browser can call our loop
    //     via requestAnimationFrame.  glutMainLoop() must NOT be called on
    //     the web because it would block the browser's event thread.
    //
    //   #else
    //     Desktop: register a GLUT timer for ~60 fps and enter the
    //     standard glutMainLoop() which never returns.
    // ------------------------------------------------------------------
#ifdef __EMSCRIPTEN__
    // fps = 0  → let the browser choose (typically 60 fps via rAF)
    // simulate_infinite_loop = 1 → keep main() alive (do not call exit)
    emscripten_set_main_loop(emLoop, 0, 1);
#else
    glutTimerFunc(16, timerCB, 0);   // kick off ~60 fps update
    glutMainLoop();                  // blocks until window is closed
#endif

    return 0;
}