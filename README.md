# Galactic Defender: Interactive 2D Space Survival Simulation

---

| Field | Details |
|---|---|
| **Institution** | Metropolitan University, Sylhet |
| **Department** | Computer Science and Engineering |
| **Course** | CSE 402 — Computer Graphics |
| **Submitted To** | Shrabanti Chowdhury, Senior Lecturer |
| **Batch / Section** | 58th Batch, Section B |

### Team Members

| Name | Student ID |
|---|---|
| Eshrath Jahan Esha | 231-115-041 |
| Niyaz Ahmad Khan | 231-115-052 |

---

## 🎮 Live View

> **Insert your GitHub Pages URL below after the first successful Actions run.**

```
https://<your-github-username>.github.io/galactic-defender/
```

[![Build & Deploy](https://github.com/<your-github-username>/galactic-defender/actions/workflows/build.yml/badge.svg)](https://github.com/<your-github-username>/galactic-defender/actions/workflows/build.yml)

---

## Abstract

Galactic Defender is a real-time, interactive 2D space survival game developed in **C++** using the **FreeGLUT** and **OpenGL** libraries. The project demonstrates practical applications of computer graphics theory — including 2D geometric primitive rendering, affine transformations, bounding-box collision detection, and fixed-timestep game loops. The game is compiled to **WebAssembly** via the Emscripten toolchain, enabling browser-based play without plugins.

---

## Features

### 1. 2D Primitive Rendering (OpenGL Fixed-Function Pipeline)

| Entity | Primitive | Colour |
|---|---|---|
| Player Spaceship | `GL_TRIANGLES` | Cyan (`0, 1, 1`) |
| Enemy Asteroids | `GL_QUADS` | Red (`1, 0.25, 0.25`) |
| Laser Projectiles | `GL_LINES` | Yellow (`1, 1, 0`) |
| Explosion Particles | `GL_POINTS` | Orange fade |
| HUD Bars | `GL_QUADS` | Green → Red (health) |
| Starfield | `GL_POINTS` | White (varying brightness) |

All geometry is rendered through the OpenGL fixed-function pipeline using `glBegin` / `glEnd` blocks and vertex submission via `glVertex2f`.

### 2. Dynamic Kinematics

- **Linear velocity model**: each entity updates its position by a constant velocity vector each frame (`position += velocity × Δt`).
- **Fixed timestep**: the update loop runs at a constant `DT = 1/60 s` regardless of display refresh rate, ensuring deterministic simulation.
- **Difficulty scaling**: asteroid speed and spawn frequency increase continuously as a function of elapsed time, creating a progressive challenge curve.

### 3. Interactive Keyboard Controls

| Key | Action |
|---|---|
| `A` | Move spaceship left |
| `D` | Move spaceship right |
| `SPACE` | Fire laser (hold for continuous fire with 0.25 s cooldown) |
| `R` | Restart game (from Game Over screen) |
| `ESC` | Quit |

Key presses are processed using GLUT's `glutKeyboardFunc` (key-down) and `glutKeyboardUpFunc` (key-up) callbacks, enabling smooth hold-to-move and hold-to-fire mechanics.

### 4. Bounding-Box Collision Detection (AABB)

Collision detection uses Axis-Aligned Bounding Box (AABB) testing for all entity pairs:

```
Overlap if:  |Ax − Bx| < (Ahw + Bhw)  AND  |Ay − By| < (Ahh + Bhh)
```

| Pair | Consequence |
|---|---|
| Laser ↔ Asteroid | Asteroid destroyed, score increases, explosion spawned |
| Ship ↔ Asteroid | Hull damage −20 HP, explosion spawned |
| Asteroid exits screen | Hull damage −10 HP (missed asteroid penalty) |

### 5. HUD Overlay

The Heads-Up Display (HUD) is rendered using `glutBitmapCharacter` (via the `drawString` helper) and `GL_QUADS` geometry:

- **Score counter** — top-left, live update each frame
- **Hull indicator label** — top-right
- **Animated health bar** — colour-shifts from green (100 % HP) to red (0 % HP)
- **Game Over screen** — dimmed full-screen overlay with final score and restart prompt

### 6. WebAssembly Portability

The `main()` function contains an `#ifdef __EMSCRIPTEN__` block to switch between the two execution models:

```cpp
#ifdef __EMSCRIPTEN__
    // Web: browser drives the loop via requestAnimationFrame
    emscripten_set_main_loop(emLoop, 0, 1);
#else
    // Desktop: GLUT timer fires every 16 ms (~60 fps)
    glutTimerFunc(16, timerCB, 0);
    glutMainLoop();
#endif
```

This single-source approach allows the identical C++ codebase to compile for both native desktop and web (WebAssembly) targets.

---

## Project Structure

```
galactic-defender/
├── main.cpp                        ← Core C++ game engine (OpenGL / FreeGLUT)
├── README.md                       ← This document
└── .github/
    └── workflows/
        └── build.yml               ← GitHub Actions CI/CD pipeline
```

---

## Build Instructions

### Desktop (Linux / macOS)

**Prerequisites:** `g++`, `freeglut3-dev`, `libgl1-mesa-dev`, `libglu1-mesa-dev`

```bash
# Ubuntu / Debian
sudo apt-get install freeglut3-dev

# Compile
g++ main.cpp -o galactic_defender -lGL -lGLU -lglut

# Run
./galactic_defender
```

### Desktop (Windows — MinGW)

```bash
g++ main.cpp -o galactic_defender.exe -lfreeglut -lopengl32 -lglu32
```

### Web — Manual Emscripten Build

**Prerequisites:** [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)

```bash
mkdir -p public
emcc main.cpp \
     -o public/index.html \
     -s USE_FREEGLUT=1 \
     -s LEGACY_GL_EMULATION=1 \
     -O2

# Serve locally (Python 3)
cd public && python3 -m http.server 8080
# Open: http://localhost:8080/index.html
```

### Web — Automated (GitHub Actions)

Push any commit to the `main` branch. The workflow defined in
`.github/workflows/build.yml` will:

1. Install the Emscripten SDK via `mymindstorm/setup-emsdk@v12`
2. Compile `main.cpp` → WebAssembly
3. Deploy the output to the `gh-pages` branch via `peaceiris/actions-gh-pages@v3`

The game becomes available at:
```
https://<your-github-username>.github.io/galactic-defender/
```

---

## Technical Design

### Rendering Pipeline

```
main() → glutMainLoop / emscripten_set_main_loop
           │
           ├── update()          ← physics, AI, collision, state
           └── display()
                 ├── renderStars()       GL_POINTS
                 ├── renderParticles()   GL_POINTS  (alpha-blended)
                 ├── renderBullets()     GL_LINES   (yellow)
                 ├── renderAsteroids()   GL_QUADS   (red)
                 ├── renderPlayer()      GL_TRIANGLES (cyan)
                 └── renderHUD()         GL_QUADS + glutBitmapCharacter
```

### Memory Model

All entity arrays are statically allocated fixed-size pools to avoid heap fragmentation and `std::vector` overhead in the tight 60-fps loop:

| Pool | Capacity |
|---|---|
| `g_bullets[]` | 64 |
| `g_asteroids[]` | 128 |
| `g_particles[]` | 256 |
| `g_stars[]` | 150 |

### Difficulty Curve

```
diffMult(t)   = clamp(1.0 + 1.5 × (t / 120), 1.0, 2.5)
spawnInterval = clamp(1.2 / diffMult(t), 0.40, 1.2)   seconds
```

At game start the speed multiplier is **1.0×** and spawn period is **1.2 s**.
After 120 seconds the multiplier caps at **2.5×** with a spawn period of **0.48 s**.

---

## References

1. OpenGL 1.x Reference Manual — [https://www.opengl.org/documentation/](https://www.opengl.org/documentation/)
2. FreeGLUT API Documentation — [https://freeglut.sourceforge.net/docs/api.php](https://freeglut.sourceforge.net/docs/api.php)
3. Emscripten Documentation — [https://emscripten.org/docs/](https://emscripten.org/docs/)
4. GitHub Actions Documentation — [https://docs.github.com/en/actions](https://docs.github.com/en/actions)
5. Foley, J. D. et al. *Computer Graphics: Principles and Practice* (2nd ed.). Addison-Wesley, 1996.

---

*© 2026 — Eshrath Jahan Esha & Niyaz Ahmad Khan — Metropolitan University, Sylhet*
