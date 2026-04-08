/*
Arduino Smart Parking System
Upgraded: 7 Slots, Traffic Lights, and Dual Intro Screens
*/

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

/* ------------------- LCD ------------------- */
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* ------------------- SERVO ----------------- */
Servo gateServo;
const int SERVO_PIN = 4;
const int GATE_OPEN = 65;   // Adjusted open angle
const int GATE_CLOSE = 155; // Your custom close angle

/* ------------------- IR SENSORS ------------ */
const int IR_ENTRY = 2;
const int IR_EXIT = 3;

/* ------------------- LED PINS -------------- */
const int LED_GREEN = 5;
const int LED_YELLOW = 6;
const int LED_RED = 7;

/* ------------------- PARKING --------------- */
int slotsAvailable = 7; // Total parking slots updated to 7

bool entryDetected = false;
bool exitDetected = false;

/* ------------------- SETUP ----------------- */
void setup() {
  Serial.begin(9600);

  // Setup IR Sensors
  pinMode(IR_ENTRY, INPUT);
  pinMode(IR_EXIT, INPUT);

  // Setup LEDs
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Setup Servo
  gateServo.attach(SERVO_PIN);
  gateServo.write(GATE_CLOSE);

  // Setup LCD
  lcd.init();
  lcd.backlight();

  // --- INTRO SCREEN 1: School Name ---
  lcd.setCursor(0, 0);
  lcd.print(" Daulatpur Pilot"); 
  lcd.setCursor(0, 1);
  lcd.print("  Girl's School "); 
  delay(3000); // Hold for 3 seconds
  lcd.clear();

  // --- INTRO SCREEN 2: Project Name ---
  lcd.setCursor(0, 0);
  lcd.print("    ARDUINO     ");
  lcd.setCursor(0, 1);
  lcd.print(" PARKING SYSTEM ");
  delay(2000); // Hold for 2 seconds
  lcd.clear();
  // -----------------------------------

  // Turn on the correct LED immediately when the system starts
  updateLEDs();
}

/* ------------------- LOOP ------------------ */
void loop() {

  /* -------- ENTRY SENSOR -------- */
  if (digitalRead(IR_ENTRY) == LOW && !entryDetected) {
    entryDetected = true;

    if (slotsAvailable > 0) {
      if (!exitDetected) {
        gateServo.write(GATE_OPEN);
        slotsAvailable--;
        updateLEDs(); // Instantly update lights
      }
    } else {
      // Lot is full, reject the car
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   SORRY :(     ");
      lcd.setCursor(0, 1);
      lcd.print("  Parking Full  ");
      delay(3000);
      lcd.clear();
    }
  }

  /* -------- EXIT SENSOR -------- */
  if (digitalRead(IR_EXIT) == LOW && !exitDetected) {
    exitDetected = true;

    if (!entryDetected) {
      gateServo.write(GATE_OPEN);
      slotsAvailable++;
      if (slotsAvailable > 7) slotsAvailable = 7; // Max capacity safety check
      updateLEDs(); // Instantly update lights
    }
  }

  /* -------- RESET AFTER PASS -------- */
  if (entryDetected && exitDetected) {
    delay(1000); // Give the car 1 second to clear the gate completely
    gateServo.write(GATE_CLOSE);
    entryDetected = false;
    exitDetected = false;
  }

  /* -------- LCD DISPLAY ---------- */
  lcd.setCursor(0, 0);
  lcd.print("    WELCOME!    ");
  lcd.setCursor(0, 1);
  lcd.print("Slot Left: ");
  lcd.print(slotsAvailable);
  lcd.print("    "); // Extra spaces to wipe old numbers
}

/* ------------------- LED LOGIC ------------- */
void updateLEDs() {
  // First, turn ALL lights off to clear previous state
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  // Then, turn on only the right one based on slots left
  if (slotsAvailable == 0) {
    // 0 Slots = RED
    digitalWrite(LED_RED, HIGH);
  } 
  else if (slotsAvailable > 0 && slotsAvailable <= 3) {
    // 1 to 3 Slots = YELLOW (Almost Full)
    digitalWrite(LED_YELLOW, HIGH);
  } 
  else {
    // 4 to 7 Slots = GREEN (Plenty of Space)
    digitalWrite(LED_GREEN, HIGH);
  }
}