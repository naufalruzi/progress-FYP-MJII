#include <Adafruit_GFX.h>        //OLED  libraries
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "MAX30105.h"           //MAX3010x library
#include "heartRate.h"          //Heart rate  calculating algorithm
#include <WiFi.h>
#include <HTTPClient.h>

MAX30105 particleSensor;

const byte RATE_SIZE  = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array  of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last  beat occurred
float beatsPerMinute;
int beatAvg;
String beatAVGdb = String(beatAvg);
const int sampleSizeToFavoriot = 5;
int sensorValuesToFavoriot[sampleSizeToFavoriot];
int sampleIndexToFavoriot = 0;

#define SCREEN_WIDTH  128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display  height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino  reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  //Declaring the display name (display)

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

char ssid[] = "WiFi-Pelajar"; //  your network SSID (name)
char pass[] = "Student@mjiI";    // your network password (use for WPA, or use as key for WEP)

const String DEVICE = "sleep_monitor@naufal3003"; // Replace with the id_developer of your device
//String beatAVGdb; // Text to send

int status = WL_IDLE_STATUS;

void updateToFavoriot(int average){
  HTTPClient http;
  // 1. HTTP URL
  http.begin("http://apiv2.favoriot.com/v2/streams");
  // 2. HTTP Header
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Apikey", "RuX2xA0tsjaDXWkiFhlCABZWDacRMytA");
  // 3. HTTP Body
  String httpBody = "{\"device_developer_id\":\"sleep_monitor@naufal3003\",\"data\":{\"heartbeat\":\""+String(average)+"\"}}";
  // 4. HTTP Method
  int httpCode = http.POST(httpBody);

  if(httpCode > 0){
    Serial.println(http.getString());
  }
  else{
    Serial.println("HTTP Request Connection Error!");
  }

  http.end();
  
}

void setup() {  

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  
  // check for the presence of the shield:
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  
  display.begin(SSD1306_SWITCHCAPVCC,  0x3C); //Start the OLED display
  display.display();
  delay(3000);
  //  Initialize sensor
  particleSensor.begin(Wire, I2C_SPEED_FAST); //Use default  I2C port, 400kHz speed
  particleSensor.setup(); //Configure sensor with default  settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to  indicate sensor is running

}

void loop() {
  //Reading the IR value it will permit us to know if there's a finger on the  sensor or not
  //Also detecting a heartbeat
  
  long irValue = particleSensor.getIR();  
                                        
  //If a finger is detected
  if(irValue  > 7000){                                           
    display.clearDisplay();                                   //Clear the display
    display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);       //Draw the first bmp  picture (little heart)
    display.setTextSize(2);                                   //Near  it display the average BPM you can display the BPM if you want
    display.setTextColor(WHITE);  
    display.setCursor(50,0);                
    display.println("BPM");             
    display.setCursor(50,18);                
    display.println(beatAvg);  
    display.display();
    
    if (checkForBeat(irValue) == true)                        //If  a heart beat is detected
    {
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
      //noTone(3);                                          //Deactivate the buzzer  to have the effect of a "bip"
      //We sensed a beat!
      long delta = millis()  - lastBeat;                   //Measure duration between two beats
      lastBeat  = millis();
  
      beatsPerMinute = 60 / (delta / 1000.0);           //Calculating  the BPM

      if (beatsPerMinute < 150 && beatsPerMinute > 60)               //To  calculate the average we strore some values (4) then do some math to calculate the  average
      {
        rates[rateSpot++] = (byte)beatsPerMinute; //Store this  reading in the array
        rateSpot %= RATE_SIZE; //Wrap variable
  
        //Take  average of readings
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE  ; x++)
          beatAvg += rates[x];
        
        beatAvg /= RATE_SIZE;

        sensorValuesToFavoriot[sampleIndexToFavoriot] = beatAvg;  // Store sensor value in the array
        sampleIndexToFavoriot++;  // Increment the current index
      
        if (sampleIndexToFavoriot == sampleSizeToFavoriot) {
          // Calculate the average when the desired sample size is reached
          int total = 0;  // Running sum of sensor readings
          for (int i = 0; i < sampleSizeToFavoriot; i++) {
            total += sensorValuesToFavoriot[i];  // Add each sensor value to the sum
          }
      
          int average = total / sampleSizeToFavoriot;  // Calculate the average
          Serial.print("Average Heart Rate: ");
          Serial.println(average);

          updateToFavoriot(average);
      
          // Reset the current index to start storing values from the beginning of the array
          sampleIndexToFavoriot = 0;
        }
      }
    }
  }

  //If no finger is detected it inform  the user and put the average BPM to 0 or it will be stored for the next measure
  if (irValue < 7000){       
    beatAvg=0;
    display.clearDisplay();
    display.setTextSize(1);                    
    display.setTextColor(WHITE);
    display.setCursor(5,1);                
    display.println("Please wait for 30s ");              
    display.setCursor(30,10);                
    display.println("after place "); 
    display.setCursor(30,20);
    display.println("your finger ");  
    display.display();
    //noTone(3);
  }
}
