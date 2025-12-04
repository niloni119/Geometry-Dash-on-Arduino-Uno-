#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// --------------------------------------------------------------------------
// U8G2 LIBRARY CONFIGURATION
// Constructor for SSD1306/SH1106 128x64 with hardware I2C. 
// Wiring: SDA (A4) and SCL (A5) on Arduino Uno.
// --------------------------------------------------------------------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// --- BUTTON CONFIGURATION ---
// Pin for the "Jump/Restart" button. Connected to D7 and GND.
const int BUTTON_PIN = 7; 

// --- GAME VARIABLES AND DIMENSIONS ---
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int GROUND_Y = SCREEN_HEIGHT - 5; // Y-coordinate for the ground level

// Object dimensions
const int CUBE_SIZE = 10;      // Player cube size
const int SPIKE_WIDTH = 8;     // Width of the spike obstacle
const int SPIKE_HEIGHT = 12;   // Height of the spike obstacle

// Motion and Physics Parameters
const int BASE_SPIKE_SPEED = 5;    // Initial speed of obstacles (pixels/frame)
const int JUMP_POWER = 12;         // Initial vertical jump velocity
const int GRAVITY = 2;             // Gravitational acceleration (simulated)
const int BASE_SPAWN_CHANCE = 4;   // Initial chance of a spike appearing (in %)

// --- GAME STATE ---
// Cube (Player) state
int cube_x = 10;
int cube_y = GROUND_Y - CUBE_SIZE; // Cube starts on the ground

bool is_jumping = false;
int jump_velocity = 0; // Current vertical velocity during jump

// Spike (Obstacle) state
int spike_x = SCREEN_WIDTH; // Spike position on the X-axis
bool spike_active = false; // Is the spike currently on screen

// Difficulty and Score
int current_spike_speed = BASE_SPIKE_SPEED;
int current_spawn_chance = BASE_SPAWN_CHANCE;
int score = 0; 
long game_progress = 0; // Total progress used to scale difficulty

// --------------------------------------------------------------------------
//                          FUNCTION PROTOTYPES
// Prototypes must be declared before functions are called in setup() or loop()
// --------------------------------------------------------------------------
void gameOver();
void drawSpike(int x, int y); 
void updateDifficulty(); 

// --------------------------------------------------------------------------
//                                 SETUP
// Initializes the Arduino and display hardware.
// --------------------------------------------------------------------------

void setup() {
    Serial.begin(9600);
    u8g2.begin(); // Initialize the OLED display
    // Configure button pin with internal pull-up resistor.
    // Pressing the button pulls the pin LOW.
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // Initial Welcome Screen
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    u8g2.drawStr(5, 20, "Geometry Dash LITE");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(5, 40, "Press Button to Start");
    u8g2.sendBuffer();
    
    // Seed the random number generator
    randomSeed(analogRead(A0));
    delay(2000);
}

// --------------------------------------------------------------------------
//                         DIFFICULTY UPDATE LOGIC
// Increases game speed and spike spawn frequency based on progress.
// --------------------------------------------------------------------------
void updateDifficulty() {
    long normalized_progress = game_progress / 100;

    // 1. INCREASE GAME SPEED (Max speed 9)
    int speed_increase = normalized_progress / 10; 
    current_spike_speed = BASE_SPIKE_SPEED + speed_increase;
    if (current_spike_speed > 9) { 
        current_spike_speed = 9;
    }
    
    // 2. INCREASE SPAWN CHANCE (Max chance 8%)
    int chance_increase = normalized_progress / 500;
    current_spawn_chance = BASE_SPAWN_CHANCE + chance_increase;
    if (current_spawn_chance > 8) { 
        current_spawn_chance = 8;
    }
}

// --------------------------------------------------------------------------
//                               DRAW SPIKE FUNCTION
// Draws the triangle shape representing the spike obstacle.
// --------------------------------------------------------------------------
void drawSpike(int x, int y) {
    // Defines the three points of the triangle (x1,y1) to (x2,y2) to (x3,y3)
    int x1 = x;
    int y1 = y;
    int x2 = x + SPIKE_WIDTH;
    int y2 = y; 
    
    int x3 = x + (SPIKE_WIDTH / 2);
    int y3 = y - SPIKE_HEIGHT; 
    
    u8g2.drawTriangle(x1, y1, x2, y2, x3, y3);
    // Draw a small base on the ground line
    u8g2.drawBox(x, y - 1, SPIKE_WIDTH, 1); 
}

// --------------------------------------------------------------------------
//                                 LOOP (Main Game Loop)
// Handles input, physics, collision, and drawing for each frame.
// --------------------------------------------------------------------------

void loop() {
    
    updateDifficulty();
    
    // 1. Input Handling (Jump)
    // Check if the button is pressed (LOW) and the cube is on the ground.
    if (digitalRead(BUTTON_PIN) == LOW && !is_jumping) {
        is_jumping = true;
        jump_velocity = JUMP_POWER;
    }

    // 2. Cube State Update (Physics)
    if (is_jumping) {
        cube_y -= jump_velocity; // Apply vertical movement
        jump_velocity -= GRAVITY; // Apply gravity (slows upward speed, increases downward speed)
        
        // Landing Check
        if (cube_y >= (GROUND_Y - CUBE_SIZE)) {
            cube_y = GROUND_Y - CUBE_SIZE; // Snap to the ground level
            is_jumping = false;
            jump_velocity = 0;
        }
    }

    // 3. Spike State Update (Movement and Spawn)
    if (spike_active) {
        spike_x -= current_spike_speed; // Move the spike left
        game_progress++; // Increase progress
        
        // CHECK FOR OBJECT PASSING AND SCORE INCREMENT
        if (spike_x < -SPIKE_WIDTH) {
            spike_active = false; // Spike is off-screen
            score++; // Increase score
        }
    } else {
        // Chance to spawn a new spike (checked when the screen is clear of the previous one)
        if (random(0, 100) < current_spawn_chance) { 
            spike_active = true;
            spike_x = SCREEN_WIDTH; // Spawn spike on the right edge
        }
    }

    // 4. Collision Check (GAME OVER)
    if (spike_active) {
        // Collision on X-axis: Cube is horizontally aligned with the spike
        bool hit_x = (cube_x + CUBE_SIZE > spike_x) && (cube_x < spike_x + SPIKE_WIDTH);
        // Collision on Y-axis: Cube is low enough to hit the spike's height
        bool hit_y = (cube_y + CUBE_SIZE > GROUND_Y - SPIKE_HEIGHT); 
        
        if (hit_x && hit_y) {
            gameOver();
            return; // Exit loop iteration to wait in gameOver
        }
    }

    // 5. U8G2 Drawing
    u8g2.clearBuffer(); // Clear the display buffer
    u8g2.drawLine(0, GROUND_Y, SCREEN_WIDTH, GROUND_Y); // Draw the ground line

    // Draw the Cube (Player)
    u8g2.drawBox(cube_x, cube_y, CUBE_SIZE, CUBE_SIZE);
    
    // Draw the spike
    if (spike_active) {
        drawSpike(spike_x, GROUND_Y);
    }
    
    // Draw the score and current speed
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(SCREEN_WIDTH - 55, 10);
    u8g2.print(F("SCORE:"));
    u8g2.print(score); 
    
    u8g2.setCursor(0, 10);
    u8g2.print(F("SPD:"));
    u8g2.print(current_spike_speed);
    
    u8g2.sendBuffer(); // Send the buffer content to the display
    
    delay(10); // Frame rate control (approx. 100 FPS)
}

// --------------------------------------------------------------------------
//                               GAME OVER FUNCTION
// Displays the final score and waits for input to restart.
// --------------------------------------------------------------------------

void gameOver() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_unifont_t_symbols); 
    u8g2.setCursor(5, 20);
    u8g2.print(F("GAME OVER"));
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(5, 35);
    u8g2.print(F("Final Score: "));
    u8g2.print(score);
    u8g2.setCursor(5, 45);
    u8g2.print(F("Press Button to Retry"));
    
    u8g2.sendBuffer();

    // Wait until the button is pressed again to restart the game
    while (digitalRead(BUTTON_PIN) != LOW) {
        delay(10);
    }
    
    // Reset all game variables for a new round
    cube_y = GROUND_Y - CUBE_SIZE;
    is_jumping = false;
    spike_active = false;
    score = 0;
    game_progress = 0; 
    spike_x = SCREEN_WIDTH;
    current_spike_speed = BASE_SPIKE_SPEED;
    current_spawn_chance = BASE_SPAWN_CHANCE;
    
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}
