// Smart Sorting Bin with LED Indicators
// Optimized for SajjadJIM - Arduino Uno R3

#include <Servo.h>

// Pin Definitions
const int trigPin = 5;
const int echoPin = 6;
const int sensorPin = 4; // Soil Moisture / IR Sensor
const int servoPin = 7;
const int greenLed = 2;  // Green LED for DRY
const int redLed = 3;    // Red LED for WET

Servo s1;
long duration;
int distanceCm;
int sensorValue;

void setup() {
  Serial.begin(9600); 
  
  s1.attach(servoPin);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(sensorPin, INPUT);
  
  // LED Setup
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  
  // Initial State: LEDs off, Servo neutral
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
  s1.write(50);
}

void loop() {
  // 1. Measure Distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * 0.034 / 2;

  // 2. Read Sensor Value
  sensorValue = digitalRead(sensorPin);

  // 3. Logic Control
  // Only operate if an object is within 20cm
  if (distanceCm < 20 && distanceCm > 1) {
    
    if (sensorValue == LOW) { 
      // WET CONDITION
      digitalWrite(redLed, HIGH);   // Red ON
      digitalWrite(greenLed, LOW);  // Green OFF
      s1.write(100);                // Open Wet Side
      Serial.println("Status: WET - Red LED ON");
    } 
    else { 
      // DRY CONDITION
      digitalWrite(greenLed, HIGH); // Green ON
      digitalWrite(redLed, LOW);    // Red OFF
      s1.write(0);                  // Open Dry Side
      Serial.println("Status: DRY - Green LED ON");
    }
    delay(1000); // Wait for servo to move and user to see LED
  } 
  else {
    // IDLE STATE (No object nearby)
    s1.write(50);
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, LOW);
  }

  delay(100); 
}