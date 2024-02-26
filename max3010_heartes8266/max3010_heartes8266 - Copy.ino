#include <Wire.h>
#include "MAX30105.h"
#include <ESP8266WiFi.h>
#include "heartRate.h"
#include <ESP8266HTTPClient.h>

MAX30105 particleSensor;

const byte RATE_SIZE = 4;  //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];     //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;  //Time at which the last beat occurred
String ssid = "UUMWiFi_Guest", pass = "";
bool constatus = false;
float beatsPerMinute;
int beatAvg;
bool fingerSt = false;
WiFiClient wifiClient;
const long TWO_MINUTES_IN_MILLISECONDS = 120 * 1000;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");
  Serial.println("Connecting to WiFi " + ssid);
  if (testWifi()) {
    Serial.println("WiFi Connected!!!");
    constatus = true;
    Serial.println("Connecting to sensor");
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST))  //Use default I2C port, 400kHz speed
    {
      Serial.println("MAX30102 was not found. Please check wiring/power. ");
      while (1)
        ;
    }
    Serial.println("Place your index finger on the sensor with steady pressure.");
    particleSensor.setup();                     //Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A);  //Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0);   //Turn off Green LED
  } else {
    constatus = false;
    Serial.println("Unable to connect to wifi");
  }
  // Initialize sensor
}

void loop() {
  if (constatus) {
    getBeat();
  }
}

void getBeat() {
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    if (fingerSt) {
      unsigned long start_time = millis();
      while (millis() - start_time < TWO_MINUTES_IN_MILLISECONDS) {
        // Do nothing.
        //We sensed a beat!
        long delta = millis() - lastBeat;
        lastBeat = millis();

        beatsPerMinute = 60 / (delta / 1000.0);

        if (beatsPerMinute < 255 && beatsPerMinute > 20) {
          rates[rateSpot++] = (byte)beatsPerMinute;  //Store this reading in the array
          rateSpot %= RATE_SIZE;                     //Wrap variable

          //Take average of readings
          beatAvg = 0;

          for (byte x = 0; x < RATE_SIZE; x++) {
            beatAvg += rates[x];
          }
        }
        beatAvg /= RATE_SIZE;
        Serial.println(beatAvg);
        Serial.println("Upload");
      }

      if (beatAvg != 0) {
        uploadBeat(int beatval);
        beatAvg = 0;
      }
      fingerSt = false;
    }
  }

  // Serial.print("IR=");
  // Serial.print(irValue);
  // Serial.print(", BPM=");
  // Serial.print(beatsPerMinute);
  // Serial.print(", Avg BPM=");
  // Serial.print(beatAvg);

  if (irValue < 50000) {
    fingerSt = false;
  }else{
    fingerSt = true;
  }
  //Serial.print(" No finger?");

  //Serial.println();
}

boolean testWifi() {  //Test WiFi connection function
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  int c = 0;
  while (c < 30) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.setAutoReconnect(true);
      WiFi.persistent(true);
      Serial.println(WiFi.status());
      Serial.println(WiFi.localIP());
      delay(100);
      //digitalWrite(LED, HIGH);
      return true;
    }
    Serial.print(".");
    yield();
    delay(1000);
    c++;
  }
  //digitalWrite(LED, LOW);
  Serial.println("Connection time out...");
  return false;
}

void uploadBeat(int beatval) {
  String request = "http://slumberjer.com/beat/insert.php?beat=" + String(beatval);
  Serial.println(request);
  HTTPClient http;
  http.begin(wifiClient, request);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String responseBody = http.getString();
    Serial.println(responseBody);
  } else {
    Serial.println("Error: " + String(httpCode));
  }
  http.end();
}
