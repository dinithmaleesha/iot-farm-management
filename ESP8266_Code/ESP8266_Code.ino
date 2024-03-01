#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Servo.h>
#include <DHT.h>

#define FIREBASE_HOST "" // Firebase host
#define FIREBASE_AUTH "" //Firebase Auth code
#define WIFI_SSID "" //Enter your wifi Name
#define WIFI_PASSWORD "" // Enter your password

// LED
#define LedPin D0
#define LedSwitch D1

// Servo
Servo myservo;
#define servoPin D2
#define servoSwitch D3
int servoPosition = 0;
int stepDelay = 15;

// Flame Detector
#define flameDetector D4

// DHT
#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Soil Moisture
//#define moisturePin A0
const int moisturePin = A0;

void setup() {
  // Serial Communication
  Serial.begin(9600);

  // LED
  pinMode(LedPin, OUTPUT);
  pinMode(LedSwitch, INPUT_PULLUP);

  // Servo Motor
  myservo.attach(servoPin);
  pinMode(servoSwitch, INPUT_PULLUP);
  myservo.write(servoPosition);

  // Flame Detector
  pinMode(flameDetector, INPUT);

  // DHT
  dht.begin();

  // Soil Moisture
  
  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected.");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // define Firebase variable
  // LED
  Firebase.setString("/LED/ledStatus", "0");
  Firebase.setString("/LED/LedIs", "OFF");

  // Servo
  Firebase.setString("/SERVO/servoStatus" , "0");
  Firebase.setString("/SERVO/gateIs", "CLOSE");

  // Flame Detector
  Firebase.setString("/FIRE/fireStatus" , "Not-Detected");

  // DHT
  Firebase.setFloat("/DHT/temperature" , 0.0);
  Firebase.setFloat("/DHT/humidity" , 0.0);

  // Soil Moisture
  Firebase.setInt("/SoilMoisture/sensorValue", 0);
  Firebase.setInt("/SoilMoisture/moisturePercentage", 0);
  Firebase.setString("/SoilMoisture/soilStatus", "N/A");
  Firebase.setString("/SoilMoisture/waterPump", "OFF");
}

void loop() {
  // LED Control
  bool swithchState = digitalRead(LedSwitch);
  String stringLedStatus = Firebase.getString("/LED/ledStatus");

  int ledStatus = stringLedStatus.toInt();
  Serial.println("Switch State: " +String(swithchState));
  Serial.println("LED State   : " + String(ledStatus));

  if(hasStatusChanged(swithchState, ledStatus)){
    Serial.println("LED Status has changed");
    controlLED(swithchState, ledStatus);
  }

  // Servo Control
  bool servoSwitchState = digitalRead(servoSwitch);
  String stringServoStatus = Firebase.getString("/SERVO/servoStatus");

  int servoStatus = stringServoStatus.toInt();

  if(hasServoChanged(servoSwitchState, servoStatus)){
    Serial.println("Servo Status has changed");
    controlServo(servoSwitchState, servoStatus);
  }
 
  // Flame Detector
  flameDetectorMethod();

  // DHT
  DHTvalue();

  // Soil Moisture
  soilMoisture();
}

//check LED switch or firebase status has changed
bool hasStatusChanged(bool currentSwitchState, int currentLedStatus){
  static bool previousSwitchState = false;
  static int previousLedStatus = 0;

  if (currentSwitchState != previousSwitchState || currentLedStatus != previousLedStatus) {
    // Status has changed
    previousSwitchState = currentSwitchState;
    previousLedStatus = currentLedStatus;
    return true;
  }
  return false;
}

//check Servo switch or firebase statuc has changed
bool hasServoChanged(bool currentServoSwitchState, int currentServoStatus){
  static bool previousServoSwitchState = false;
  static int previousServoStatus = 0;

  if(previousServoSwitchState != currentServoSwitchState || currentServoStatus != previousServoStatus){
    // Status has changed
    previousServoSwitchState = currentServoSwitchState;
    previousServoStatus = currentServoStatus;
    return true;
  }
  return false;
}

// Control LED main Logic
void controlLED(bool switchState, int ledStatus){
  Serial.println("Control LED:");
  Serial.println("    Switch State   : " + String(switchState));
  Serial.println("    Firebase State : " + String(ledStatus));
  

  if(switchState == 0){
    if(ledStatus == 0){
      digitalWrite(LedPin, LOW);
      Firebase.setString("/LED/LedIs", "OFF");
      Serial.println("    LED : OFF");

    } else if(ledStatus == 1){
      digitalWrite(LedPin, HIGH);
      Firebase.setString("/LED/LedIs", "ON");
      Serial.println("    LED : ON");
    }
  } else{
    if(ledStatus == 0){
      digitalWrite(LedPin, HIGH);
      Firebase.setString("/LED/LedIs", "ON");
      Serial.println("    LED : ON");

    } else if(ledStatus == 1){
      digitalWrite(LedPin, LOW);
      Firebase.setString("/LED/LedIs", "OFF");
      Serial.println("    LED : OFF");
    }
  }
  Serial.println("");
}

// Control SERVO main Logic
void controlServo(bool switchState, int servoStatus){
  Serial.println("Control Servo:");
  Serial.println("    Switch State   : " + String(switchState));
  Serial.println("    Firebase State : " + String(servoStatus));
  

  if(switchState == 0){
    if(servoStatus == 0){
      Firebase.setString("/SERVO/gateIs", "CLOSE");
      Serial.println("    Servo : OFF");
      servoRotate(90, 0);

    } else if (servoStatus == 1){
      Firebase.setString("/SERVO/gateIs", "OPEN");
      Serial.println("    Servo : ON");
      servoRotate(0, 90);

    }
  } else{
    if(servoStatus == 0){
      Firebase.setString("/SERVO/gateIs", "OPEN");
      Serial.println("    Servo : ON");
      servoRotate(0, 90);

    } else if (servoStatus == 1){
      Firebase.setString("/SERVO/gateIs", "CLOSE");
      Serial.println("    Servo : OFF");
      servoRotate(90, 0);

    }
  }
  Serial.println("");
}

// Servo rotate
void servoRotate(int from, int to) {
  Serial.println("Rotate Servo:");
  if (from < to) {
    Serial.println("    Rotate to: 0 to 90");
    for (int pos = from; pos <= to; pos++) {
      myservo.write(pos);
      delay(stepDelay);
    }
    Serial.println("      Rotated");

  } else {
    Serial.println("    Rotate to: 90 to 0");
    for (int pos = from; pos >= to; pos--) {
      myservo.write(pos);
      delay(stepDelay);
    }
    Serial.println("      Rotated");

  }
  Serial.println("");
}

// Flame Detector
void flameDetectorMethod(){
  Serial.println("Check Fire:");
  int fire = digitalRead(flameDetector);
  if(fire == 0){
    Firebase.setString("/FIRE/fireStatus", "Detected");
    Serial.println("    Fire: Detected");
  } else if(fire == 1){
    Firebase.setString("/FIRE/fireStatus", "Not-Detected");
    Serial.println("    Fire: Not-Detected");
  }
  Serial.println("");
}

// DHT
void DHTvalue(){
  Serial.println("Check DHT11 value:");
  
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("    Failed to read from DHT sensor");
    Firebase.setFloat("/DHT/temperature" , 0);
    Firebase.setFloat("/DHT/humidity" , 0);
  } else {
    Serial.print("    Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    // update Firebase value
    Firebase.setFloat("/DHT/temperature" , temperature);
    Firebase.setFloat("/DHT/humidity" , humidity);
  }
  Serial.println("");
}

// Soil Moisture
void soilMoisture(){
  Serial.println("Check Soil Moisture value:");
  

  int sensorValue = analogRead(moisturePin);
  int moisturePercentage = map(sensorValue, 0, 1023, 0, 100);
  
  Serial.print("    Soil Moisture: ");
  Serial.print(moisturePercentage);
  Serial.println("%");
  Serial.print("    Sensor value: ");
  Serial.println(sensorValue);

  Firebase.setInt("/SoilMoisture/sensorValue", sensorValue);
  Firebase.setInt("/SoilMoisture/moisturePercentage", moisturePercentage);

  if(sensorValue>900){
    Firebase.setString("/SoilMoisture/soilStatus", "Dry");
    Firebase.setString("/SoilMoisture/waterPump", "ON");
    Serial.println("    Watering the plant...");
    Serial.println("*"); // Send "1" to Uno board
  } else {
    Firebase.setString("/SoilMoisture/soilStatus", "Wet");
    Firebase.setString("/SoilMoisture/waterPump", "OFF");
    Serial.println("    Moisture level is sufficient.");
    Serial.println("/"); // Send "0" to Uno board
  }
  Serial.println("");
}