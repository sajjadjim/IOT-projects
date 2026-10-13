

#include <Servo.h> // The standard built-in Arduino Servo Library

// --- PIN DEFINITIONS (Arduino Uno R3) ---
#define ENTRY_SENSOR_PIN 2  // Digital Pin 2
#define EXIT_SENSOR_PIN 3   // Digital Pin 3
#define GATE_SERVO_PIN 9    // Digital Pin 9 (PWM)

// --- SYSTEM SETTINGS ---
int totalSpaces = 5;      // Total spots in the parking lot
int availableSpaces = 5;  // Spots currently empty

Servo gateServo;

void setup() {
  // 9600 is the standard baud rate for Arduino Uno
  Serial.begin(9600); 
  
  // Set sensors to read data
  pinMode(ENTRY_SENSOR_PIN, INPUT);
  pinMode(EXIT_SENSOR_PIN, INPUT);
  
  // Attach servo and make sure gate starts CLOSED (0 degrees)
  gateServo.attach(GATE_SERVO_PIN);
  gateServo.write(0); 
  
  Serial.println("Arduino Smart Parking System Ready!");
  Serial.print("Available Spaces: ");
  Serial.println(availableSpaces);
}

void loop() {
  // Read both IR sensors (LOW means a car is detected blocking the IR light)
  int entryCarDetected = digitalRead(ENTRY_SENSOR_PIN);
  int exitCarDetected = digitalRead(EXIT_SENSOR_PIN);

  // =======================================
  // LOGIC 1: A CAR IS ENTERING
  // =======================================
  if (entryCarDetected == LOW && availableSpaces > 0) {
    Serial.println("Car Entering! Opening Gate...");
    
    gateServo.write(90); // Open the gate (up)
    delay(3000);         // Wait 3 seconds for the car to drive through
    gateServo.write(0);  // Close the gate (down)
    
    availableSpaces--;   // Subtract 1 from available spaces
    
    Serial.print("Spaces left: ");
    Serial.println(availableSpaces);
    delay(1000);         // Cooldown so it doesn't trigger twice fast
  }
  
  // If lot is full, don't open the gate!
  else if (entryCarDetected == LOW && availableSpaces == 0) {
    Serial.println("Parking Lot is FULL! Gate stays closed.");
    delay(1000);
  }

  // =======================================
  // LOGIC 2: A CAR IS EXITING
  // =======================================
  if (exitCarDetected == LOW && availableSpaces < totalSpaces) {
    Serial.println("Car Exiting! Opening Gate...");
    
    gateServo.write(90); // Open the gate (up)
    delay(3000);         // Wait 3 seconds for the car to drive out
    gateServo.write(0);  // Close the gate (down)
    
    availableSpaces++;   // Add 1 back to available spaces
    
    Serial.print("Spaces left: ");
    Serial.println(availableSpaces);
    delay(1000);         // Cooldown
  }
}