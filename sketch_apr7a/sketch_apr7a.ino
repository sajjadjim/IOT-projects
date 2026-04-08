// Pin Definitions
const int trigPin = 2;
const int echoPin = 3;
const int moisturePin = A0; // Soil sensor connected to Analog 0

// Threshold for Soil (Adjust this based on your sensor)
// Lower values = Wet, Higher values = Dry
const int dryThreshold = 700; 

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.begin(9600); // Initialize Serial Monitor
  Serial.println("--- Dual Measurement System Starting ---");
}

void loop() {
  // 1. ULTRASONIC MEASUREMENT
  long duration;
  int distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Convert to cm

  // 2. SOIL MOISTURE MEASUREMENT
  int moistureValue = analogRead(moisturePin);
  String soilStatus;

  if (moistureValue < dryThreshold) {
    soilStatus = "WET (Water Detected)";
  } else {
    soilStatus = "DRY (Needs Water)";
  }

  // 3. DISPLAY RESULTS TO SERIAL MONITOR
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm  |  ");
  
  Serial.print("Moisture Level: ");
  Serial.print(moistureValue);
  Serial.print("  |  Status: ");
  Serial.println(soilStatus);

  delay(500); // Wait half a second before next reading
}