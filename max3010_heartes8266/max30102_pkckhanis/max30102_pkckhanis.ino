#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>
#include "MAX30105.h"
#include <ESP8266WiFi.h>
#include "heartRate.h"
#include <ESP8266HTTPClient.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels
#define OLED_RESET    -1  // Reset pin # (or -1 if sharing reset pin)


MAX30105 particleSensor;

const byte RATE_SIZE = 4;  //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];     //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;  //Time at which the last beat occurred
String ssid = "Galaxy S10b3f4", pass = "nopal3003";
bool constatus = false;
float beatsPerMinute;
int beatAvg;
bool fingerSt = false;
WiFiClient wifiClient;
const long TWO_MINUTES_IN_MILLISECONDS = 10 * 1000;
int upload = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const unsigned char PROGMEM  logo2_bmp[] =
{ 0x03, 0xC0, 0xF0, 0x06, 0x71, 0x8C, 0x0C, 0x1B, 0x06, 0x18, 0x0E,  0x02, 0x10, 0x0C, 0x03, 0x10,              //Logo2 and Logo3 are two bmp pictures  that display on the OLED if called
0x04, 0x01, 0x10, 0x04, 0x01, 0x10, 0x40,  0x01, 0x10, 0x40, 0x01, 0x10, 0xC0, 0x03, 0x08, 0x88,
0x02, 0x08, 0xB8, 0x04,  0xFF, 0x37, 0x08, 0x01, 0x30, 0x18, 0x01, 0x90, 0x30, 0x00, 0xC0, 0x60,
0x00,  0x60, 0xC0, 0x00, 0x31, 0x80, 0x00, 0x1B, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x04, 0x00,  };

static const unsigned char PROGMEM logo3_bmp[] =
{ 0x01, 0xF0, 0x0F,  0x80, 0x06, 0x1C, 0x38, 0x60, 0x18, 0x06, 0x60, 0x18, 0x10, 0x01, 0x80, 0x08,
0x20,  0x01, 0x80, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x08,  0x03,
0x80, 0x00, 0x08, 0x01, 0x80, 0x00, 0x18, 0x01, 0x80, 0x00, 0x1C, 0x01,  0x80, 0x00, 0x14, 0x00,
0x80, 0x00, 0x14, 0x00, 0x80, 0x00, 0x14, 0x00, 0x40,  0x10, 0x12, 0x00, 0x40, 0x10, 0x12, 0x00,
0x7E, 0x1F, 0x23, 0xFE, 0x03, 0x31,  0xA0, 0x04, 0x01, 0xA0, 0xA0, 0x0C, 0x00, 0xA0, 0xA0, 0x08,
0x00, 0x60, 0xE0,  0x10, 0x00, 0x20, 0x60, 0x20, 0x06, 0x00, 0x40, 0x60, 0x03, 0x00, 0x40, 0xC0,
0x01,  0x80, 0x01, 0x80, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x30, 0x0C,  0x00,
0x00, 0x08, 0x10, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x03, 0xC0, 0x00,  0x00, 0x01, 0x80, 0x00  };


void setup() {
  Serial.begin(115200);
  SPI.begin(); // init SPI bus
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

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
 if(irValue  > 7000){                                           //If a finger is detected
    display.clearDisplay();                                   //Clear the display
    display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);       //Draw the first bmp  picture (little heart)
    display.setTextSize(2);                                   //Near  it display the average BPM you can display the BPM if you want
    display.setTextColor(WHITE);  
    display.setCursor(50,0);                
    display.println("BPM");             
    display.setCursor(50,18);                
    display.println(beatAvg);  
    display.display(); 
  if (checkForBeat(irValue) == true) {
    
    display.clearDisplay();                                //Clear  the display
    display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE);    //Draw  the second picture (bigger heart)
    display.setTextSize(2);                                //And  still displays the average BPM
    display.setTextColor(WHITE);             
    display.setCursor(50,0);                
    display.println("BPM");             
    display.setCursor(50,18);                
    display.println(beatAvg); 
    display.display();
    //tone(3,1000);                                        //And  tone the buzzer for a 100ms you can reduce it it will be better
    delay(100);
    unsigned long start_time = millis();
    // Do nothing.
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;  //Store this reading in the array
      rateSpot %= RATE_SIZE;                     //Wrap variable

      //Take average of readings

      for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
      }
    }
    beatAvg /= RATE_SIZE;
    Serial.println("BEAT AVG:" + String(beatAvg));
    if (upload==10){
      uploadBeat(beatAvg);
      upload = 0;
    }else{
      upload++;
    }
    

  } 


  // Serial.print("IR=");
  // Serial.print(irValue);
  // Serial.print(", BPM=");
  // Serial.print(beatsPerMinute);
  // Serial.print(", Avg BPM=");
  // Serial.print(beatAvg);

  if (irValue < 70000){
    Serial.println(" No finger?");
  //Serial.println();
     display.clearDisplay();
     display.setTextSize(1);                    
     display.setTextColor(WHITE);             
     display.setCursor(30,5);                
     display.println("Please Place "); 
     display.setCursor(30,15);
     display.println("Your Finger ");  
     display.display();
  }
}
}

//void drawScreen(String a, String b, String c, String d, String e)
//{
//  display.display();
//  delay(10);
//  display.clearDisplay();
//  display.setTextColor(WHITE);
//  display.setTextSize(1);
//  display.setCursor(15, 5);
//  display.print(a);
//  display.setCursor(0, 20);
//  display.print(b);
//  display.setCursor(0, 30);
//  display.print(c);
//  display.setCursor(0, 40);
//  display.print(d);
//  display.setCursor(0, 50);
//  display.print(e);
//  display.display();
//}

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
  String request = "http://naufal3003.000webhostapp.com/insertbpm.php?bpm=" + String(beatval);
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
