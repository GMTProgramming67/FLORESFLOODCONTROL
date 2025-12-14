#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NewPing.h>
#include <TimerFreeTone.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TRIG_PIN 9
#define ECHO_PIN 10
#define MAX_DISTANCE 600
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

int buzz = 8;
const int maxd = 100;
const unsigned long interval = 5000; //time interval

float distanceA = 0, distanceB = 0, velocity = 0; //values for reading a, b then rate of rise

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

void loop() ;
    distanceA = getAverageDistance();
    delay(interval); 
    distanceB = getAverageDistance();

    float diff = distanceA - distanceB;
    velocity = diff / (interval / 1000.0);

float estimatedTime = 0;
  if (velocity > 0.1) estimatedTime = distanceB / velocity;
  else estimatedTime = 0;

    showOLED(distanceB, velocity,estimatedTime);

    // === Buzzer Alerts ===
    if (distanceB > maxd) {
      Serial.println(" DANGER LEVEL >100cm!");
      nonBlockingBuzz(1500, 8, 300);
    } 
    else if (distanceB > 80) {
      Serial.println(" Risky level (>80cm)");
      nonBlockingBuzz(1000, 5, 300);
    } 
    else if (distanceB > 30) {
      Serial.println(" Moderate level (30–80cm)");
      nonBlockingBuzz(800, 3, 200);
    } 
    else {
      Serial.println(" Safe level (<30cm)");
      nonBlockingBuzz(500, 1, 100);
    }

    Serial.print(F("Dist A: ")); Serial.println(distanceA);
    Serial.print(F("Dist B: ")); Serial.println(distanceB);
    Serial.print(F("Velocity: ")); Serial.println(velocity);
  Serial.print(F("Estimated Time: ")); Serial.println(estimatedTime);
    Serial.println(F("--------------------------"));
  }
}

// === OLED Display ===
void showOLED(float dist, float vel, float estimatedTime) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Dynamic header based on flood level 
  display.setCursor(10, 0);
  if (dist > 100) display.print(F(" DANGER LEVEL >100cm"));
  else if (dist > 80) display.print(F(" Risky level >80cm"));
  else if (dist > 30) display.print(F(" Moderate level"));
  else display.print(F(" Safe level <30cm"));

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.drawLine(64, 10, 64, 63, SSD1306_WHITE);
  display.drawLine(64, 35, 127, 35, SSD1306_WHITE);
  display.drawLine(0, 21, 64, 21, SSD1306_WHITE);
  display.drawLine(64, 55, 127, 55, SSD1306_WHITE);

  // Left box
display.setCursor(2, 13);
display.print(F("Delay: "));
display.setCursor(33, 13);
display.print(interval);


  display.setCursor(2, 25);
  display.print(F("Estimated "));
  display.setCursor(2, 36);
  display.print(F("Time:"));
  display.setTextSize(2);
  display.setCursor(3, 47);
  display.print(estimatedTime);
  display.setTextSize(1);

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


// === Distance Average ===
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

//Buzzer code because Oled distrupts the buzzer, used Timer free tone and new ping
void nonBlockingBuzz(int freq, int count, int duration) {
  unsigned long buzzStart = millis();
  for (int i = 0; i < count; i++) {
    TimerFreeTone(buzz, freq, duration);
    while (millis() - buzzStart < (i + 1) * (duration + 50)) {}
  }
}
