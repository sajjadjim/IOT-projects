#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "Servo.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo flapServo;

// Pin Definitions
const int SERVO_PIN      = 9;
const int TRIG_PERSON    = 2;
const int ECHO_PERSON    = 3;
const int TRIG_WET_BIN   = 4;
const int ECHO_WET_BIN   = 5;
const int TRIG_DRY_BIN   = 7;
const int ECHO_DRY_BIN   = 8;
const int RED_LED_ACTIVE = 11;
const int GREEN_LED_WET  = 12;
const int GREEN_LED_DRY  = 13;
const int MOISTURE_PIN   = A0;

// Angles for flap
const int NEUTRAL_ANGLE  = 90;
const int WET_ANGLE      = 35;
const int DRY_ANGLE      = 145;

// Function to read HC-SR04 sonar safely
int readSonarDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 18ms timeout (~3 meters max distance)
  long duration = pulseIn(echoPin, HIGH, 18000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

// Function to blink LED for 5 seconds (5000 ms)
void blinkGreenLed(int pin) {
  for (int i = 0; i < 10; i++) {
    digitalWrite(pin, HIGH);
    delay(250);
    digitalWrite(pin, LOW);
    delay(250);
  }
}

void setup() {
  Serial.begin(9600);

  // Setup LEDs
  pinMode(RED_LED_ACTIVE, OUTPUT);
  pinMode(GREEN_LED_WET, OUTPUT);
  pinMode(GREEN_LED_DRY, OUTPUT);
  digitalWrite(RED_LED_ACTIVE, HIGH); // System active indicator

  // Setup Sonar Pins
  pinMode(TRIG_PERSON, OUTPUT);
  pinMode(ECHO_PERSON, INPUT);
  pinMode(TRIG_WET_BIN, OUTPUT);
  pinMode(ECHO_WET_BIN, INPUT);
  pinMode(TRIG_DRY_BIN, OUTPUT);
  pinMode(ECHO_DRY_BIN, INPUT);

  // Setup Servo
  flapServo.attach(SERVO_PIN);
  flapServo.write(NEUTRAL_ANGLE);

  // Setup LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMART DUSTBIN");
  lcd.setCursor(0, 1);
  lcd.print("READY TO USE");
  delay(1500);
  lcd.clear();
}

void loop() {
  // Read person detection sensor
  int personDist = readSonarDistance(TRIG_PERSON, ECHO_PERSON);

  // Person comes within 10 CM
  if (personDist > 0 && personDist <= 10) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("USE SMART");
    lcd.setCursor(0, 1);
    lcd.print("DUST BIN !!!");

    delay(2000); // Wait 2 seconds for waste to be placed on moisture sensor

    // Read moisture sensor safely
    int moistureRaw = analogRead(MOISTURE_PIN);

    // If moisture sensor is wet (raw value drops significantly, e.g. < 700)
    if (moistureRaw < 700) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Type: WET WASTE");
      lcd.setCursor(0, 1);
      lcd.print("Sorting to WET..");

      flapServo.write(WET_ANGLE);
      blinkGreenLed(GREEN_LED_WET); // 5 seconds blink
      flapServo.write(NEUTRAL_ANGLE);
    } else {
      // Default to DRY waste (even if moisture sensor is open/disconnected)
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Type: DRY WASTE");
      lcd.setCursor(0, 1);
      lcd.print("Sorting to DRY..");

      flapServo.write(DRY_ANGLE);
      blinkGreenLed(GREEN_LED_DRY); // 5 seconds blink
      flapServo.write(NEUTRAL_ANGLE);
    }

    lcd.clear();
  } else {
    // Normal Standby Mode: Show level of both bins
    int wetDist = readSonarDistance(TRIG_WET_BIN, ECHO_WET_BIN);
    int dryDist = readSonarDistance(TRIG_DRY_BIN, ECHO_DRY_BIN);

    // Wet Bin display check (>= 13 cm is Empty)
    lcd.setCursor(0, 0);
    lcd.print("Wet: ");
    if (wetDist >= 13 || wetDist == 999) {
      lcd.print("Empty    ");
    } else if (wetDist <= 3) {
      lcd.print("FULL!    ");
    } else {
      lcd.print(wetDist);
      lcd.print("cm left ");
    }

    // Dry Bin display check (>= 13 cm is Empty)
    lcd.setCursor(0, 1);
    lcd.print("Dry: ");
    if (dryDist >= 13 || dryDist == 999) {
      lcd.print("Empty    ");
    } else if (dryDist <= 3) {
      lcd.print("FULL!    ");
    } else {
      lcd.print(dryDist);
      lcd.print("cm left ");
    }

    delay(300);
  }
}