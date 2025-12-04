#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// --------------------------------------------------------------------------
// КОНФИГУРАЦИЯ U8G2 ДЛЯ SH1106 (1.3" OLED)
// --------------------------------------------------------------------------
// Предполагается I2C: SDA на A4, SCL на A5 (для Uno)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// --- КОНФИГУРАЦИЯ КНОПКИ ---
const int BUTTON_PIN = 7; 

// --- ПЕРЕМЕННЫЕ ИГРЫ И РАЗМЕРЫ ---
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int GROUND_Y = SCREEN_HEIGHT - 5; 

// Размеры объектов
const int CUBE_SIZE = 10;      
const int SPIKE_WIDTH = 8;    
const int SPIKE_HEIGHT = 12;   

// Параметры движения и физики
const int BASE_SPIKE_SPEED = 5;    
const int JUMP_POWER = 12;         
const int GRAVITY = 2;             
const int BASE_SPAWN_CHANCE = 4;   

// Состояние куба (игрока)
int cube_x = 10;
int cube_y = GROUND_Y - CUBE_SIZE; 

// Состояние прыжка
bool is_jumping = false;
int jump_velocity = 0;

// Состояние шипа (препятствия)
int spike_x = SCREEN_WIDTH;
bool spike_active = false;

// ПЕРЕМЕННЫЕ СЛОЖНОСТИ
int current_spike_speed = BASE_SPIKE_SPEED;
int current_spawn_chance = BASE_SPAWN_CHANCE;

// СЧЕТ: ТЕПЕРЬ СЧИТАЕТ КОЛИЧЕСТВО ПЕРЕПРЫГНУТЫХ ОБЪЕКТОВ
int score = 0; 
long game_progress = 0; // Для динамики сложности

// --- ФУНКЦИИ ---
void gameOver();
void drawSpike(int x, int y); 
void updateDifficulty(); 

// --------------------------------------------------------------------------
//                                 SETUP
// --------------------------------------------------------------------------

void setup() {
    Serial.begin(9600);
    u8g2.begin(); 
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // Приветствие
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    u8g2.drawStr(5, 20, "Geometry Dash LITE");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(5, 40, "Score: Per Jump!");
    u8g2.sendBuffer();
    
    // Инициализация генератора случайных чисел
    randomSeed(analogRead(A0));
    delay(2000);
}

// --------------------------------------------------------------------------
//                         ОБНОВЛЕНИЕ СЛОЖНОСТИ
// --------------------------------------------------------------------------
void updateDifficulty() {
    // ВАЖНО: сложность теперь зависит от game_progress, а не score
    long normalized_progress = game_progress / 100;

    // 1. УСКОРЕНИЕ ИГРЫ
    // Скорость увеличивается на 1 каждые 1000 единиц прогресса
    int speed_increase = normalized_progress / 10; 
    current_spike_speed = BASE_SPIKE_SPEED + speed_increase;

    // Ограничение максимальной скорости
    if (current_spike_speed > 9) { 
        current_spike_speed = 9;
    }
    
    // 2. УВЕЛИЧЕНИЕ ЧАСТОТЫ ПОЯВЛЕНИЯ
    // Шанс увеличивается на 1% каждые 500 единиц прогресса
    int chance_increase = normalized_progress / 500;
    current_spawn_chance = BASE_SPAWN_CHANCE + chance_increase;

    // Ограничение максимального шанса
    if (current_spawn_chance > 8) { 
        current_spawn_chance = 8;
    }
}

// --------------------------------------------------------------------------
//                               РИСОВАНИЕ ШИПА
// --------------------------------------------------------------------------
void drawSpike(int x, int y) {
    // Шип - это треугольник, нарисованный на земле
    int x1 = x;
    int y1 = y;
    int x2 = x + SPIKE_WIDTH;
    int y2 = y; 
    
    int x3 = x + (SPIKE_WIDTH / 2);
    int y3 = y - SPIKE_HEIGHT; 
    
    u8g2.drawTriangle(x1, y1, x2, y2, x3, y3);
    // Небольшая полоска на земле, чтобы шип не "висел"
    u8g2.drawBox(x, y - 1, SPIKE_WIDTH, 1); 
}

// --------------------------------------------------------------------------
//                                 LOOP
// --------------------------------------------------------------------------

void loop() {
    
    updateDifficulty();
    
    // 1. Обработка Ввода (Прыжок)
    // Кнопка подключена как INPUT_PULLUP, поэтому нажатие - это LOW
    if (digitalRead(BUTTON_PIN) == LOW && !is_jumping) {
        is_jumping = true;
        jump_velocity = JUMP_POWER;
    }

    // 2. Обновление состояния Куба (Физика)
    if (is_jumping) {
        cube_y -= jump_velocity;
        jump_velocity -= GRAVITY;
        
        // Приземление
        if (cube_y >= (GROUND_Y - CUBE_SIZE)) {
            cube_y = GROUND_Y - CUBE_SIZE;
            is_jumping = false;
            jump_velocity = 0;
        }
    }

    // 3. Обновление состояния Шипа
    if (spike_active) {
        spike_x -= current_spike_speed;
        game_progress++; // Увеличиваем прогресс для сложности
        
        // ПРОВЕРКА ПРОХОЖДЕНИЯ ОБЪЕКТА И ДОБАВЛЕНИЕ СЧЕТА
        if (spike_x < -SPIKE_WIDTH) {
            spike_active = false;
            score++; // СЧЕТ ДОБАВЛЯЕТСЯ ТОЛЬКО ОДИН РАЗ, КОГДА ОБЪЕКТ УШЕЛ
        }
    } else {
        // Шанс появления нового шипа
        if (random(0, 100) < current_spawn_chance) { 
            spike_active = true;
            spike_x = SCREEN_WIDTH;
        }
    }

    // 4. Проверка Коллизий (GAME OVER)
    if (spike_active) {
        // Проверка столкновения по X-координате
        bool hit_x = (cube_x + CUBE_SIZE > spike_x) && (cube_x < spike_x + SPIKE_WIDTH);
        // Проверка столкновения по Y-координате
        bool hit_y = (cube_y + CUBE_SIZE > GROUND_Y - SPIKE_HEIGHT); 
        
        if (hit_x && hit_y) {
            gameOver();
            return; 
        }
    }

    // 5. Отрисовка U8G2
    u8g2.clearBuffer();
    // Рисуем линию земли
    u8g2.drawLine(0, GROUND_Y, SCREEN_WIDTH, GROUND_Y); 
    u8g2.setBitmapMode(1); 

    // Отрисовка Куба
    u8g2.drawBox(cube_x, cube_y, CUBE_SIZE, CUBE_SIZE);
    
    // Рисуем шип
    if (spike_active) {
        drawSpike(spike_x, GROUND_Y);
    }
    
    // Рисуем счет и текущую скорость
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(SCREEN_WIDTH - 55, 10);
    u8g2.print(F("SCORE:"));
    u8g2.print(score); 
    
    u8g2.setCursor(0, 10);
    u8g2.print(F("SPD:"));
    u8g2.print(current_spike_speed);
    
    u8g2.sendBuffer();
    
    delay(10); // Задержка для управления частотой кадров
}

// --------------------------------------------------------------------------
//                               ФУНКЦИЯ GAME OVER
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

    // Ждем нажатия кнопки для перезапуска
    while (digitalRead(BUTTON_PIN) != LOW) {
        delay(10);
    }
    
    // Сброс всех игровых переменных
    cube_y = GROUND_Y - CUBE_SIZE;
    is_jumping = false;
    spike_active = false;
    score = 0;
    game_progress = 0; // Сбрасываем прогресс
    spike_x = SCREEN_WIDTH;
    current_spike_speed = BASE_SPIKE_SPEED;
    current_spawn_chance = BASE_SPAWN_CHANCE;
    
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}
