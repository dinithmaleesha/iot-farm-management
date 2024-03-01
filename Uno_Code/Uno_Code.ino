int waterPumpPin = 2;  // Digital pin connected to the water pump
char receivedData;
int pumpStatus = 0;

void setup() {
  Serial.begin(9600);
  pinMode(waterPumpPin, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
      receivedData = Serial.read();
      Serial.print("Received: "); Serial.println(receivedData);

      if (receivedData == '*') {
        Serial.println("Received: Watering is needed.");
        digitalWrite(waterPumpPin, HIGH);
        delay(3000);
      } else if (receivedData == '/') {
        Serial.println("Received: No watering needed.");
        digitalWrite(waterPumpPin, LOW);
      } else {
        Serial.println("Received: Other char.");
        //digitalWrite(waterPumpPin, LOW);
      }
      
  }
}
