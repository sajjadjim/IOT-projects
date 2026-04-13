#include <Servo.h>

const int trigPin = 10;
const int echoPin = 11;
Servo myServo;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600); // Start talking to the computer
  myServo.attach(9);  // Servo on Pin 9
}

void loop() {
  // Sweep from 15 to 165 degrees
  for(int i = 15; i <= 165; i++) {
    myServo.write(i);
    delay(30); // Speed of the radar sweep
    int distance = calculateDistance();
    
    // Send data to computer format: "Angle,Distance."
    Serial.print(i); 
    Serial.print(","); 
    Serial.print(distance); 
    Serial.println(".");
  }
  
  // Sweep back from 165 to 15 degrees
  for(int i = 165; i > 15; i--) {
    myServo.write(i);
    delay(30);
    int distance = calculateDistance();
    
    Serial.print(i); 
    Serial.print(","); 
    Serial.print(distance); 
    Serial.println(".");
  }
}

// Function to fire the sensor and calculate distance in cm
int calculateDistance() {
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;
  return distance;
}