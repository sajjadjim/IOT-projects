#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// --- 1. Hardware Setup ---
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD Display
Servo myServo;                      // Create a servo object

// --- 2. Pin Definitions ---
const int trigPin = 5;
const int echoPin = 18;
const int pirPin = 19;
const int servoPin = 13;

// --- 3. Variables ---
long duration;
int distanceCm;

void setup() {
  // Start the screen
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Starting!!");
  
  // Setup Sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT);
  
  // Setup Servo
  myServo.attach(servoPin);
  myServo.write(0); // Tell the servo to go to the 0-degree starting position
  
  // Wait 3 seconds to let the motion sensor warm up
  delay(3000); 
  lcd.clear();
}

void loop() {
  // --- READ DISTANCE SENSOR ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * 0.034 / 2;
  
  // --- READ MOTION SENSOR ---
  int motionDetected = digitalRead(pirPin);
  
  // --- LOGIC: CONTROL THE SERVO ---
  // If motion IS detected AND distance is LESS than 10cm
  if (motionDetected == HIGH && distanceCm < 10) {
    myServo.write(90); // Open! Turn servo to 90 degrees
  } else {
    myServo.write(0);  // Closed! Keep servo at 0 degrees
  }
  
  // --- UPDATE THE LCD SCREEN ---
  lcd.setCursor(0, 0); 
  lcd.print("Dist: ");
  lcd.print(distanceCm);
  lcd.print(" cm    "); 
  
  lcd.setCursor(0, 1); 
  if (motionDetected == HIGH) {
    lcd.print("Motion: DETECTED!");
  } else {
    lcd.print("Motion: None    "); 
  }

  // Brief pause before checking again
  delay(200);
}