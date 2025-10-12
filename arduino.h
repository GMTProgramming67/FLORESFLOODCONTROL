#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NewPing.h>
#include <TimerFreeTone.h>

// === OLED Setup ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === Ultrasonic Setup ===
#define TRIG_PIN 9
#define ECHO_PIN 10
#define MAX_DISTANCE 600
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

int buzz = 8;
const int maxd = 100;
const unsigned long interval = 10000; // 10 seconds
unsigned long previousMillis = 0;

// === Global readings ===
float distanceA = 0, distanceB = 0, velocity = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Starting JSN-SR04T Flood + OLED Test..."));
  pinMode(buzz, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {}
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // === Read distances ===
    distanceA = getAverageDistance();
    delay(10000);  // Keep ultrasonic stable between readings
    distanceB = getAverageDistance();

    float diff = distanceA - distanceB;
    velocity = diff / (interval / 1000.0);

    // === Display on OLED ===
    showOLED(distanceB, velocity);

    // === Buzzer Alerts ===
    if (distanceB > maxd) {
      nonBlockingBuzz(1500, 8, 300);
    } else if (distanceB > 80) {
      nonBlockingBuzz(1000, 5, 300);
    } else if (distanceB > 30) {
      nonBlockingBuzz(800, 3, 200);
    } else {
      nonBlockingBuzz(500, 1, 100);
    }

    // === Serial Debug ===
    Serial.print(F("Dist A: ")); Serial.println(distanceA);
    Serial.print(F("Dist B: ")); Serial.println(distanceB);
    Serial.print(F("Velocity: ")); Serial.println(velocity);
    Serial.println(F("--------------------------"));
  }
}

// === OLED Display Function ===
void showOLED(float dist, float vel) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Header
  display.setCursor(30, 0);
  display.print(F("SAFE: RISKY"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.drawLine(64, 10, 64, 63, SSD1306_WHITE);
  display.drawLine(64, 35, 127, 35, SSD1306_WHITE);
  display.drawLine(0, 21, 64, 21, SSD1306_WHITE);
  display.drawLine(64, 55, 127, 55, SSD1306_WHITE);

  // Left box
  display.setCursor(2, 13);
  display.print(F("ETime: "));
  if (vel > 0) display.print((maxd - dist) / vel, 0);
  else display.print(F("--"));
  display.print(F("s"));

  display.setCursor(2, 25);
  display.print(F("Estimated "));
  display.setCursor(2, 36);
  display.print(F("Time:"));
  display.setTextSize(2);
  display.setCursor(3, 47);
  display.print((int)dist);
  display.setTextSize(1);
  display.setCursor(40, 53);
  display.print(F("cm"));

  // Right box
  display.setCursor(67, 15);
  display.print(F("Velocity:"));
  display.setCursor(68, 26);
  display.print(vel, 1);
  display.print(F(" cm/s"));
  display.setCursor(68, 38);
  display.print(F("Current"));
  display.setCursor(67, 46);
  display.print(F("level:"));
  display.setCursor(103, 46);
  display.print((int)dist);
  display.print(F("cm"));

  display.setCursor(80, 57);
  if (vel > 0) display.print(F("RISING"));
  else if (vel < 0) display.print(F("DROPPING"));
  else display.print(F("STABLE"));

  display.display();
}

// === Distance Averaging ===
float getAverageDistance() {
  const int samples = 5;
  unsigned int total = 0;
  int valid = 0;
  for (int i = 0; i < samples; i++) {
    unsigned int reading = sonar.ping_cm();
    if (reading > 0) { total += reading; valid++; }
    delay(30);
  }
  if (valid == 0) return 0;
  return total / (float)valid;
}

// === Non-blocking buzzer ===
void nonBlockingBuzz(int freq, int count, int duration) {
  unsigned long buzzStart = millis();
  for (int i = 0; i < count; i++) {
    TimerFreeTone(buzz, freq, duration);
    while (millis() - buzzStart < (i + 1) * (duration + 50)) {}
  }
}
