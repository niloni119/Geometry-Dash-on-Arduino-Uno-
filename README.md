# Geometry Dash on Arduino Uno (Minimalist Version)

A minimalist, high-speed version of the popular platformer game, **Geometry Dash**, implemented on an **Arduino Uno** and an **OLED screen**. Players control a jumping square to avoid incoming spikes, with the difficulty dynamically increasing as the game progresses.

## Features

* **Dynamic Difficulty:** Spike speed and spawn rate increase with `game_progress`.
* **Simple Controls:** A single button controls the jump action.
* **Collision Detection:** Accurate hit-box detection for the cube and spike.
* **Score Tracking:** Score increases every time a spike is successfully jumped over.

## Components Needed

* **Arduino Uno** (or compatible board)
* **1.3" OLED Display** (SH1106, 128x64, I2C interface) - *Note: The code uses the `U8G2_SH1106_128X64_NONAME_F_HW_I2C` constructor.*
* **Push Button**
* **Breadboard & Jumper Wires**

## Wiring Diagram

### Connections

| Component | Pin on Arduino Uno |
| :--- | :--- |
| **OLED (GND)** | GND |
| **OLED (VCC)** | 5V |
| **OLED (SCL)** | A5 (I2C Clock) |
| **OLED (SDA)** | A4 (I2C Data) |
| **Push Button** | Digital Pin **7** |
| **Push Button** | GND (The code uses `INPUT_PULLUP` to simplify wiring, so the other side of the button goes to GND) |

 

## Software & Libraries

1.  **Arduino IDE:** Ensure you have the latest version.
2.  **U8g2 Library:** This library is essential for controlling the OLED display. Install it via the Arduino Library Manager:
    * `Sketch` -> `Include Library` -> `Manage Libraries...`
    * Search for **`U8g2`** and install the latest version by **olikraus**.

## How to Run

1.  Connect the components according to the wiring table above.
2.  Open the `Geometry_Dash_Arduino.ino` file in the Arduino IDE.
3.  Ensure you have selected the correct **Board** (`Arduino Uno`) and **Port**.
4.  Upload the code to your Arduino.
5.  Press the button to start playing!
   
## The main goal of the project 

is to demonstrate the possibility of creating a simple, dynamic gameplay (platformer runner)

based on basic and affordable Arduino components, focusing on the efficient use of limited resources.:

**Graphics:** Using monochrome OLED display (128x64) and U8g2 library for rendering graphic primitives (cube, spikes, earth).

**Logic:** Implementation of basic game mechanics such as gravity, jumping, obstacle generation and collision checking.

**Performance:** Achieve high frame rate (FPS) for smooth gameplay, despite the low power of the microcontroller.
