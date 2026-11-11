# Galactic Defender: Interactive 2D Space Survival Simulation

<div align="center">

[![Play Game](https://img.shields.io/badge/🎮%20PLAY%20NOW-LIVE%20DEMO-00ffcc?style=for-the-badge&logo=googlechrome&logoColor=black)](https://theniyazkhan.github.io/galactic-defender/)
[![Deploy Status](https://img.shields.io/badge/GitHub%20Pages-Active-brightgreen?style=for-the-badge&logo=github)](https://theniyazkhan.github.io/galactic-defender/)

**Direct Link to Play:** [https://theniyazkhan.github.io/galactic-defender/](https://theniyazkhan.github.io/galactic-defender/)

</div>

---

### Academic Information

| Field | Details |
|---|---|
| **Institution** | Metropolitan University, Sylhet |
| **Department** | Computer Science and Engineering |
| **Course** | CSE 402 — Computer Graphics |
| **Submitted To** | Shrabanti Chowdhury, Senior Lecturer |
| **Batch / Section** | 58th Batch, Section B |

### Project Contributors

| Name | Student ID | Role |
|---|---|---|
| **Niyaz Ahmad Khan** | 231-115-052 | Game Engine, Kinematics & Deployment |
| **Eshrath Jahan Esha** | 231-115-041 | Graphics Design, Visuals & Collision Detection |

---

## 🚀 About the Project

**Galactic Defender** is a fast-paced, real-time 2D space survival game built with **C++** using **OpenGL** and **FreeGLUT**, featuring a parallel modern **HTML5 Canvas / Web Audio edition** for seamless cross-platform play directly in any web browser.

The project demonstrates core concepts in computer graphics:
- **Primitive rendering**: Custom polygonal models constructed via vertex buffers (`GL_TRIANGLES`, `GL_QUADS`, `GL_POLYGON`, and `GL_LINES`).
- **3D-shaded isometric voxel aesthetics**: Minecraft-styled meteorites featuring dynamic multi-angle lighting, blocky crater depressions, and incandescent ore inclusions.
- **Particle & thruster dynamics**: Dynamic alpha-blended particle explosions and real-time oscillating jet engine exhaust flames.
- **Bounding-Box Collision Detection (AABB)**: High-performance overlap calculations between lasers, spaceships, and asteroids.
- **Deterministic 60 FPS loop**: Fixed-timestep updates independent of display refresh rates.

---

## 🎮 Game Controls

| Key | Alternative | Action |
|:---:|:---:|---|
| <kbd>←</kbd> | <kbd>A</kbd> | Move Spaceship Left |
| <kbd>→</kbd> | <kbd>D</kbd> | Move Spaceship Right |
| <kbd>SPACE</kbd> | <kbd>↑</kbd> | Fire Laser Cannons (Hold to auto-fire) |
| <kbd>R</kbd> | — | Restart Game (from Game Over screen) |
| <kbd>ESC</kbd> | — | Exit Game (Desktop build) |

---

## 🎯 Gameplay Mechanics & Rules

- **Destroy Meteorites**: Each destroyed asteroid earns score points scaled by current game difficulty: $\text{Points} = 10 \times \text{DifficultyMultiplier}$.
- **Hull Damage**:
  - **Direct collision with an asteroid**: $-20\%$ Hull damage + ship explosion effect.
  - **Allowing an asteroid to slip past screen**: $-10\%$ Hull damage penalty (missed threat).
- **Dynamic Difficulty Scaling**:
  - Asteroid velocity and spawn frequency progressively ramp up over 120 seconds.
  - Speed multiplier scales from $1.0\times$ up to $2.5\times$.
  - Spawn interval tightens from $1.2\text{ s}$ down to a fast $0.4\text{ s}$.
- **Game Over**: When hull health reaches $0\%$, a final score screen appears with an option to press <kbd>R</kbd> to play again.

---

## 🎨 Visual Assets & Design Details

| Entity | Graphic Style | Technical Implementation |
|---|---|---|
| **Spaceship** | Cyan futuristic fighter with swept wings & cockpit | Multi-tier `GL_TRIANGLES` + wing navigation beacons + animated oscillating thrusters |
| **Asteroids** | Minecraft-styled space meteorites | 3D-shaded isometric facets (sunlit top-left, shaded bottom-right), blocky craters, and redstone ore veins |
| **Lasers** | High-energy yellow beam projectiles | Dual-mount `GL_LINES` with glow bloom |
| **Explosions** | Dispersing debris particles | Radial velocity particle system with linear alpha fade |
| **Starfield** | Deep space parallax background | Star particle pool with variable brightness |
| **HUD** | Live score & dynamic health bar | Gradient health bar color-shifting from green ($100\%$) $\rightarrow$ yellow $\rightarrow$ red ($0\%$) |

---

## 📁 Repository Structure

```
galactic-defender/
├── main.cpp                 # C++ OpenGL / FreeGLUT desktop game engine
├── public/
│   └── index.html           # Web game edition (HTML5 Canvas + Web Audio API)
├── .github/
│   └── workflows/
│       └── build.yml        # Automated GitHub Actions deployment pipeline
└── README.md                # Project documentation & presentation guide
```

---

## 💻 How to Run Locally

### 1. Web Version (Instant in any browser)
Simply open [`public/index.html`](file:///c:/Users/NIyaz/galactic-defender/public/index.html) in your favorite web browser (Chrome, Edge, Firefox, Safari), or click the live link above!

### 2. Desktop C++ Build (Windows MinGW)
Make sure you have `g++` and `FreeGLUT` installed:
```bash
g++ main.cpp -o galactic_defender.exe -DFREEGLUT_STATIC -lfreeglut_static -lopengl32 -lglu32 -lwinmm -lgdi32
.\galactic_defender.exe
```

### 3. Desktop C++ Build (Linux / Ubuntu)
```bash
sudo apt-get install freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev
g++ main.cpp -o galactic_defender -lGL -lGLU -lglut
./galactic_defender
```

---

<div align="center">

*Submitted for evaluation in **CSE 402: Computer Graphics** — Metropolitan University, Sylhet.*

*© 2026 Eshrath Jahan Esha & Niyaz Ahmad Khan. All rights reserved.*

</div>
