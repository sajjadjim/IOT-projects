// ========================================================
// BLYNK CLOUD CREDENTIALS
// These connect your ESP32 to your specific Blynk Dashboard
// ========================================================
#define BLYNK_TEMPLATE_ID "TMPL6vLHwhwNE"
#define BLYNK_TEMPLATE_NAME "IOT PROJECT"
#define BLYNK_AUTH_TOKEN "MDruTO1wKKncJrVqw_pm7mByoymdlIQh"
#define BLYNK_PRINT Serial // Prints Blynk connection status to Serial Monitor

// ========================================================
// LIBRARIES
// Include the necessary code packages for all components
// ========================================================
#include <WiFi.h>              // Connect to Wi-Fi
#include <BlynkSimpleEsp32.h>  // Connect to Blynk App
#include <ESP32Servo.h>        // Control the Servo motor
#include <NewPing.h>           // Read the Ultrasonic sensor
#include <Wire.h>              // I2C communication for LCD
#include <LiquidCrystal_I2C.h> // Control the LCD screen
#include <WidgetRTC.h>         // Real-Time Clock for scheduling
#include <DHT.h>               // Read the Temperature/Humidity sensor

// ========================================================
// WI-FI CREDENTIALS
// ========================================================
char ssid[] = "PET";
char pass[] = "12345678";

// ========================================================
// PIN DEFINITIONS (Where components plug into the ESP32)
// ========================================================
#define SERVO_FEED_PIN 18  // Servo motor for the food gate
#define TRIG_PIN 5         // Ultrasonic sensor (Trigger)
#define ECHO_PIN 4         // Ultrasonic sensor (Echo)
#define PIR_PIN 15         // Motion sensor
#define DHT_PIN 19         // Temperature and Humidity sensor
#define DC_MOTOR_PIN 25    // Relay switch for the 3V cooling fan
#define LED_RED 12         // Red LED (Food is empty)
#define LED_GREEN 13       // Green LED (Food is full)

// ========================================================
// SENSOR SETTINGS & CONSTANTS
// ========================================================
#define MAX_DISTANCE 100   // Maximum distance the ultrasonic sensor will measure (cm)
#define BOX_DEPTH 20       // Depth of the food container (cm)
#define DHTTYPE DHT11      // Specify that we are using the DHT11 model

// Scheduled feeding times (24-hour format)
const int morningFeed = 8;  // 8:00 AM
const int eveningFeed = 17; // 5:00 PM
const int nightFeed = 21;   // 9:00 PM

// ========================================================
// INITIALIZE COMPONENTS
// ========================================================
Servo feedServo;
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Standard I2C address for a 16x2 LCD
DHT dht(DHT_PIN, DHTTYPE);
BlynkTimer timer; // Runs functions at specific intervals without pausing the code
WidgetRTC rtc;    // Internet clock

// ========================================================
// STATE VARIABLES (Keeps track of what the system is doing)
// ========================================================
bool isFeeding = false;          // Prevents feeding twice at the exact same time
bool fanIsOn = false;            // Tracks if the cooling fan is currently running
unsigned long fanStartTime = 0;  // Remembers when the fan turned on
unsigned long lastMotionTime = 0;// Remembers when motion was last detected
bool fedMorning = false, fedEvening = false, fedNight = false; // Tracks daily meals

// ========================================================
// MANUAL FEED BUTTON (Triggered from Blynk App Virtual Pin V0)
// ========================================================
BLYNK_WRITE(V0) {
  if (param.asInt() == 1) { // If the button is pressed
    dispenseFood("Manual"); // Call the feeding function
  }
}

// ========================================================
// FUNCTION: DISPENSE FOOD
// Opens the servo gate, updates the screen, and logs to Blynk
// ========================================================
void dispenseFood(String reason) {
  if (isFeeding) return; // Stop if already feeding
  isFeeding = true;
  
  // 1. Physically drop the food
  feedServo.write(90); // Open the gate
  delay(1500);         // Wait 1.5 seconds for food to fall
  feedServo.write(0);  // Close the gate
  
  // 2. Get the current time from the internet
  String timeString = String(hour()) + ":" + (minute() < 10 ? "0" : "") + String(minute());
  String msg = reason + " @ " + timeString; // Example: "Manual @ 14:30"
  
  // 3. Send log to Blynk Terminal (Virtual Pin V4)
  Blynk.virtualWrite(V4, msg + "\n"); 
  
  // 4. Show feeding status on the physical LCD screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fed: " + reason);
  lcd.setCursor(0, 1);
  lcd.print("Time: " + timeString);
  delay(3000); // Keep message on screen for 3 seconds
  lcd.clear();
  
  isFeeding = false; // Reset feeding state
}

// ========================================================
// FUNCTION: CHECK SENSORS (Food Level, Temp, Humidity, Fan)
// ========================================================
void checkSensors() {
  // --- 1. FOOD LEVEL SENSOR (Ultrasonic) ---
  int distance = sonar.ping_cm();
  if (distance == 0 || distance > MAX_DISTANCE) distance = MAX_DISTANCE; // Fix errors
  
  // Send distance to Serial Monitor and Blynk (Virtual Pin V1)
  Serial.print("Sending Distance to Blynk (V1): ");
  Serial.println(distance);
  Blynk.virtualWrite(V1, distance); 
  
  // Update physical LEDs based on food distance
  if (distance > (BOX_DEPTH - 5)) { // If distance is deep (box is nearly empty)
    digitalWrite(LED_RED, HIGH);    // Turn Red LED ON
    digitalWrite(LED_GREEN, LOW);
  } else {                          // Box has plenty of food
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);  // Turn Green LED ON
  }

  // --- 2. TEMPERATURE & HUMIDITY SENSOR (DHT11) ---
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Temperature in Celsius
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Warning: DHT sensor read failed!");
  } else {
    // Send Temp to V2 and Humidity to V3 on Blynk
    Serial.print("Sending Temp to Blynk (V2): ");
    Serial.println(t);
    Serial.print("Sending Humidity to Blynk (V3): ");
    Serial.println(h);
    
    Blynk.virtualWrite(V2, t); 
    Blynk.virtualWrite(V3, h); 
    
    // --- 3. COOLING FAN LOGIC ---
    // Turn the fan ON if temp goes above 30C and it isn't already running
    if (t > 30.0 && !fanIsOn) {
      digitalWrite(DC_MOTOR_PIN, HIGH); // Click the relay ON
      fanIsOn = true;
      fanStartTime = millis(); // Start a timer
      Serial.println("Fan Turned ON!");
    }
  }

  // Turn the fan OFF automatically after 5 minutes (300,000 milliseconds)
  if (fanIsOn && (millis() - fanStartTime >= 300000)) {
    digitalWrite(DC_MOTOR_PIN, LOW); // Click the relay OFF
    fanIsOn = false;
    Serial.println("Fan Turned OFF!");
  }
}

// ========================================================
// FUNCTION: CHECK MOTION (PIR Security Sensor)
// Sends a notification if the pet is waiting for food
// ========================================================
void checkPIR() {
  int motion = digitalRead(PIR_PIN);
  
  if (motion == HIGH) {
    // Only send notification if 10 minutes (600,000 ms) have passed since the last one
    if (millis() - lastMotionTime > 600000 || lastMotionTime == 0) { 
      Blynk.logEvent("pet_hungry", "Your pet is waiting near the feeder!"); // Trigger Blynk Push Notification
      lastMotionTime = millis();
    }
  }
}

// ========================================================
// FUNCTION: CHECK AUTOMATIC SCHEDULE
// Automatically feeds the pet at specific hours
// ========================================================
void checkSchedule() {
  int currentHour = hour(); // Get current hour from internet clock
  
  // Reset daily feeding flags exactly at midnight
  if (currentHour == 0) {
    fedMorning = false; fedEvening = false; fedNight = false;
  }

  // Check and trigger scheduled meals
  if (currentHour == morningFeed && !fedMorning) {
    dispenseFood("Morning");
    fedMorning = true;
  }
  if (currentHour == eveningFeed && !fedEvening) {
    dispenseFood("Evening");
    fedEvening = true;
  }
  if (currentHour == nightFeed && !fedNight) {
    dispenseFood("Night");
    fedNight = true;
  }
}

// ========================================================
// FUNCTION: UPDATE LCD DISPLAY
// Shows live temperature and food distance on the screen
// ========================================================
void updateLCD() {
  if (isFeeding) return; // Do not interrupt screen if food is dropping
  
  // Top Row: Temperature and Fan status
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print((int)dht.readTemperature());
  lcd.print("C ");
  if (fanIsOn) lcd.print("FAN:ON ");
  else lcd.print("       ");

  // Bottom Row: Food depth distance
  lcd.setCursor(0, 1);
  lcd.print("Food:");
  lcd.print(sonar.ping_cm());
  lcd.print("cm  ");
}

// ========================================================
// MAIN SETUP (Runs exactly ONE time when board turns on)
// ========================================================
void setup() {
  Serial.begin(115200); // Start serial communication for debugging
  
  // Set all pins to output (sending power/signals) or input (reading signals)
  pinMode(DC_MOTOR_PIN, OUTPUT); digitalWrite(DC_MOTOR_PIN, LOW); // Relay off
  pinMode(LED_RED, OUTPUT);      digitalWrite(LED_RED, LOW);      // Red LED off
  pinMode(LED_GREEN, OUTPUT);    digitalWrite(LED_GREEN, LOW);    // Green LED off
  pinMode(PIR_PIN, INPUT);                                        // Read Motion
  
  // Connect Servo and set it to closed position
  feedServo.attach(SERVO_FEED_PIN); 
  feedServo.write(0);

  // Start Sensors and LCD
  dht.begin();
  Wire.begin(21, 22); // I2C pins for ESP32
  lcd.init();
  lcd.backlight();
  lcd.print("Connecting Wi-Fi");

  // Connect to Wi-Fi and Blynk Cloud
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  rtc.begin(); // Sync internet time
  
  lcd.clear();
  lcd.print("System Ready!");
  delay(2000);

  // Set the Timer to run background tasks at different speeds
  timer.setInterval(2000L, checkSensors);   // Run every 2 seconds
  timer.setInterval(1000L, checkPIR);       // Run every 1 second
  timer.setInterval(60000L, checkSchedule); // Run every 60 seconds (1 minute)
  timer.setInterval(2000L, updateLCD);      // Run every 2 seconds
}

// ========================================================
// MAIN LOOP (Runs continuously forever)
// ========================================================
void loop() {
  Blynk.run(); // Keep connected to Blynk server
  timer.run(); // Keep the background timers running
}
