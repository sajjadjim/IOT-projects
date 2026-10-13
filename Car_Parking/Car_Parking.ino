#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo dustbinServo;

// Pins
const int SERVO_PIN = 9;

const int PERSON_TRIG = 2;
const int PERSON_ECHO = 3;

const int WET_TRIG = 4;
const int WET_ECHO = 5;

const int DRY_TRIG = 7;
const int DRY_ECHO = 8;

const int RED_LED = 11;
const int WET_LED = 12;
const int DRY_LED = 13;

const int MOISTURE_PIN = A0;

// Servo angles
const int SERVO_NEUTRAL = 90;
const int SERVO_WET     = 35;
const int SERVO_DRY     = 145;

// CALIBRATION SETTINGS
const int MOISTURE_THRESHOLD = 500;
const bool LOWER_VALUE_MEANS_WET = true;

// Distances in cm
const int PERSON_DISTANCE   = 10;
const int BIN_FULL_DISTANCE = 5;
const int SERVO_OPEN_TIME   = 3000;

bool waitingForPersonToLeave = false;
unsigned long lastStandbyUpdate = 0;

// Read ultrasonic distance
long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, 20000); // 20ms timeout (~3.4m)
  if (duration == 0) return -1;

  return duration * 0.0343 / 2.0;
}

// Average moisture reading
int readMoisture() {
  long total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(MOISTURE_PIN);
    delay(15);
  }
  return total / 10;
}

bool isBinFull(int trigPin, int echoPin) {
  long distance = getDistance(trigPin, echoPin);
  if (distance < 0) return false;
  return distance <= BIN_FULL_DISTANCE;
}

void allSortingLedsOff() {
  digitalWrite(WET_LED, LOW);
  digitalWrite(DRY_LED, LOW);
}

// Non-flickering real-time standby screen
void updateStandbyDisplay() {
  long wetDistance = getDistance(WET_TRIG, WET_ECHO);
  long dryDistance = getDistance(DRY_TRIG, DRY_ECHO);

  lcd.setCursor(0, 0);
  lcd.print("Smart Dustbin   ");

  lcd.setCursor(0, 1);
  lcd.print("W:");
  if (wetDistance < 0 || wetDistance >= 13) lcd.print("Empty ");
  else if (wetDistance <= BIN_FULL_DISTANCE) lcd.print("FULL! ");
  else { lcd.print(wetDistance); lcd.print("cm   "); }

  lcd.setCursor(8, 1);
  lcd.print(" D:");
  if (dryDistance < 0 || dryDistance >= 13) lcd.print("Empty ");
  else if (dryDistance <= BIN_FULL_DISTANCE) lcd.print("FULL! ");
  else { lcd.print(dryDistance); lcd.print("cm   "); }
}

void showFullMessage(const char* message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  delay(1800);
}

void openBin(int angle, int ledPin, const char* label, int moistureValue) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(label);
  lcd.setCursor(0, 1);
  lcd.print("Val: ");
  lcd.print(moistureValue);

  // Turn on designated classification LED
  digitalWrite(ledPin, HIGH);

  dustbinServo.write(angle);
  delay(SERVO_OPEN_TIME);

  dustbinServo.write(SERVO_NEUTRAL);
  delay(500);

  digitalWrite(ledPin, LOW);
}

void setup() {
  Serial.begin(9600);

  pinMode(PERSON_TRIG, OUTPUT);
  pinMode(PERSON_ECHO, INPUT);
  pinMode(WET_TRIG, OUTPUT);
  pinMode(WET_ECHO, INPUT);
  pinMode(DRY_TRIG, OUTPUT);
  pinMode(DRY_ECHO, INPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(WET_LED, OUTPUT);
  pinMode(DRY_LED, OUTPUT);

  // Red LED stays permanently ON as power/active indicator
  digitalWrite(RED_LED, HIGH);
  allSortingLedsOff();

  dustbinServo.attach(SERVO_PIN);
  dustbinServo.write(SERVO_NEUTRAL);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Smart Dustbin");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(1500);
  lcd.clear();

  Serial.println("Smart Dustbin ready");
}

void loop() {
  long personDistance = getDistance(PERSON_TRIG, PERSON_ECHO);

  // Wait until person clears the area before allowing another trigger
  if (waitingForPersonToLeave) {
    if (personDistance < 0 || personDistance > PERSON_DISTANCE) {
      waitingForPersonToLeave = false;
      lcd.clear();
    }
    delay(150);
    return;
  }

  // Person arrives within 10 cm
  if (personDistance > 0 && personDistance <= PERSON_DISTANCE) {
    waitingForPersonToLeave = true;

    // Visual prompt on screen
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("USE SMART");
    lcd.setCursor(0, 1);
    lcd.print("DUST BIN !!!");

    // 2-second window allowing the user to place the waste
    delay(2000);

    bool wetFull = isBinFull(WET_TRIG, WET_ECHO);
    bool dryFull = isBinFull(DRY_TRIG, DRY_ECHO);

    if (wetFull && dryFull) {
      showFullMessage("Both Bins Full!");
      lcd.clear();
      return;
    }

    int moistureValue = readMoisture();
    Serial.print("Moisture value: ");
    Serial.println(moistureValue);

    bool isWet = LOWER_VALUE_MEANS_WET ? (moistureValue < MOISTURE_THRESHOLD) 
                                       : (moistureValue > MOISTURE_THRESHOLD);

    Serial.println(isWet ? "Classification: WET" : "Classification: DRY");

    if (isWet) {
      if (wetFull) {
        showFullMessage("Wet bin is full!");
      } else {
        openBin(SERVO_WET, WET_LED, "WET WASTE", moistureValue);
      }
    } else {
      if (dryFull) {
        showFullMessage("Dry bin is full!");
      } else {
        openBin(SERVO_DRY, DRY_LED, "DRY WASTE", moistureValue);
      }
    }

    allSortingLedsOff();
    lcd.clear();
  } else {
    // Refresh standby status smoothly every 300 ms without flicker
    if (millis() - lastStandbyUpdate > 300) {
      lastStandbyUpdate = millis();
      updateStandbyDisplay();
    }
  }

  delay(50);
}