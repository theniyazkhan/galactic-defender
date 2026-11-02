#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Game State & Screen Dimensions
int score = 0;
int health = 100;
bool gameOver = false;
const int WIDTH = 800;
const int HEIGHT = 600;

// Entities
struct Player { float x, y, size, speed; };
struct Asteroid { float x, y, size, speed; bool active; };
struct Bullet { float x, y, speed; bool active; };

Player ship = {WIDTH / 2.0f, 50.0f, 20.0f, 15.0f};
std::vector<Asteroid> asteroids;
std::vector<Bullet> bullets;

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

void display() {
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!gameOver) {
        // Draw Player (Cyan Triangle)
        glColor3f(0.0f, 1.0f, 0.8f);
        glBegin(GL_TRIANGLES);
        glVertex2f(ship.x, ship.y + ship.size);
        glVertex2f(ship.x - ship.size, ship.y - ship.size);
        glVertex2f(ship.x + ship.size, ship.y - ship.size);
        glEnd();

        // Draw Asteroids (Red Squares)
        glColor3f(1.0f, 0.3f, 0.3f);
        for (size_t i = 0; i < asteroids.size(); i++) {
            if (asteroids[i].active) {
                glBegin(GL_QUADS);
                glVertex2f(asteroids[i].x - asteroids[i].size, asteroids[i].y - asteroids[i].size);
                glVertex2f(asteroids[i].x + asteroids[i].size, asteroids[i].y - asteroids[i].size);
                glVertex2f(asteroids[i].x + asteroids[i].size, asteroids[i].y + asteroids[i].size);
                glVertex2f(asteroids[i].x - asteroids[i].size, asteroids[i].y + asteroids[i].size);
                glEnd();
            }
        }

        // Draw Bullets (Yellow Lines)
        glColor3f(1.0f, 1.0f, 0.0f);
        for (size_t i = 0; i < bullets.size(); i++) {
            if (bullets[i].active) {
                glBegin(GL_LINES);
                glVertex2f(bullets[i].x, bullets[i].y);
                glVertex2f(bullets[i].x, bullets[i].y + 10.0f);
                glEnd();
            }
        }

        // HUD Overlay
        glColor3f(1.0f, 1.0f, 1.0f);
        std::string sText = "Score: " + std::to_string(score);
        std::string hText = "Health: " + std::to_string(health);
        drawText(10, HEIGHT - 25, sText.c_str());
        drawText(10, HEIGHT - 50, hText.c_str());
    } else {
        glColor3f(1.0f, 0.0f, 0.0f);
        std::string finalScore = "Final Score: " + std::to_string(score);
        drawText(WIDTH / 2.0f - 50, HEIGHT / 2.0f, "GAME OVER");
        drawText(WIDTH / 2.0f - 100, HEIGHT / 2.0f - 30, finalScore.c_str());
    }

    glutSwapBuffers();
}

void update() {
    if (gameOver) return;

    for (size_t i = 0; i < bullets.size(); i++) {
        if (bullets[i].active) {
            bullets[i].y += bullets[i].speed;
            if (bullets[i].y > HEIGHT) bullets[i].active = false;
        }
    }

    if (rand() % 100 < 5) {
        Asteroid a = {(float)(rand() % WIDTH), (float)HEIGHT, (float)(10 + rand() % 20), 3.0f + (score * 0.1f), true};
        asteroids.push_back(a);
    }

    for (size_t i = 0; i < asteroids.size(); i++) {
        if (asteroids[i].active) {
            asteroids[i].y -= asteroids[i].speed;
            if (asteroids[i].y < 0) { asteroids[i].active = false; health -= 10; }

            for (size_t j = 0; j < bullets.size(); j++) {
                if (bullets[j].active && std::abs(bullets[j].x - asteroids[i].x) < asteroids[i].size && std::abs(bullets[j].y - asteroids[i].y) < asteroids[i].size) {
                    bullets[j].active = false;
                    asteroids[i].active = false;
                    score += 10;
                }
            }

            if (std::abs(ship.x - asteroids[i].x) < (ship.size + asteroids[i].size) && std::abs(ship.y - asteroids[i].y) < (ship.size + asteroids[i].size)) {
                asteroids[i].active = false;
                health -= 25;
            }
        }
    }

    if (health <= 0) gameOver = true;
    glutPostRedisplay();
}

// Timer wrapper for desktop C++
void timerUpdate(int value) {
    update();
    glutTimerFunc(16, timerUpdate, 0);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'a' && ship.x > ship.size) ship.x -= ship.speed;
    if (key == 'd' && ship.x < WIDTH - ship.size) ship.x += ship.speed;
    if (key == ' ') {
        Bullet b = {ship.x, ship.y + ship.size, 10.0f, true};
        bullets.push_back(b);
    }
    if (key == 'r' && gameOver) {
        score = 0; health = 100; gameOver = false;
        asteroids.clear(); bullets.clear();
    }
}

// Emscripten requires a loop function without arguments
void emscripten_loop() {
    update();
    display();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Galactic Defender");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscripten_loop, 0, 1);
#else
    glutTimerFunc(16, timerUpdate, 0);
    glutMainLoop();
#endif

    return 0;
}