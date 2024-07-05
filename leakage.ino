#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
String apiKey = "P3NSW2BG5JPXQQ8C";
const char *ssid = "mi";
const char *pass = "00000000";
const char* server = "api.thingspeak.com";
 
#define LED_BUILTIN 16
#define SENSOR1  12
#define SENSOR2  13// Assuming sensor 2 is connected to GPIO pin 5
 
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
long flowStartTime1 = 0;
long flowStartTime2 = 0;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount1;
volatile byte pulseCount2;
byte pulse1Sec1 = 0;
byte pulse1Sec2 = 0;
float flowRate1;
float flowRate2;
unsigned long flowMilliLitres1;
unsigned long flowMilliLitres2;
unsigned int totalMilliLitres1;
unsigned int totalMilliLitres2;
float flowLitres1;
float flowLitres2;
float totalLitres1;
float totalLitres2;
float volumeDifference;
 
void IRAM_ATTR pulseCounter1()
{
  pulseCount1++;
}

void IRAM_ATTR pulseCounter2()
{
  pulseCount2++;
}
 
WiFiClient client;
 
void connectToWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 500) {
    delay(100);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Wi-Fi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Failed to connect to Wi-Fi. Please check your credentials.");
  }
}


void setup()
{
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();                                                                                         
  delay(10);
 
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR1, INPUT_PULLUP);
  pinMode(SENSOR2, INPUT_PULLUP);

 connectToWiFi();
  pulseCount1 = 0;
  pulseCount2 = 0;
  flowRate1 = 0.0;
  flowRate2 = 0.0;
  flowMilliLitres1 = 0;
  flowMilliLitres2 = 0;
  totalMilliLitres1 = 0;
  totalMilliLitres2 = 0;
  previousMillis = 0;
  volumeDifference=0;
 
 attachInterrupt(digitalPinToInterrupt(SENSOR1), pulseCounter1, FALLING);
 attachInterrupt(digitalPinToInterrupt(SENSOR2), pulseCounter2, FALLING);
}

 
void loop()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) 
  {
    
    pulse1Sec1 = pulseCount1;
    pulseCount1 = 0;
    pulse1Sec2 = pulseCount2;
    pulseCount2 = 0;
 
    flowRate1 = ((1000.0 / (millis() - previousMillis)) * pulse1Sec1) / calibrationFactor;
    flowRate2 = ((1000.0 / (millis() - previousMillis)) * pulse1Sec2) / calibrationFactor;
    previousMillis = millis();
 
    flowMilliLitres1 = (flowRate1 / 60) * 1000;
    flowMilliLitres2 = (flowRate2 / 60) * 1000;
    flowLitres1 = (flowRate1 / 60);
    flowLitres2 = (flowRate2 / 60);
 
    totalMilliLitres1 += flowMilliLitres1;
    totalMilliLitres2 += flowMilliLitres2;
    totalLitres1 += flowLitres1;
    totalLitres2 += flowLitres2;

    Serial.print("Flow rate: ");
    Serial.print(float(flowRate1));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space
    Serial.print("Flow rate 2: ");
    Serial.print(float(flowRate2));  // Print the integer part of the variable
    Serial.println("L/min");
 
    display.clearDisplay();
    
    display.setCursor(10,0);  //oled display
        display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("Water Flow Meter");
    
    display.setCursor(0,20);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("R1:");
    display.print(float(flowRate1));
    display.setCursor(50,20);  //oled display
    display.setTextSize(1);
    display.print("L/M");

    display.setCursor(0,40);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("R2:");
    display.print(float(flowRate2));
    display.setCursor(50,40);  //oled display
    display.setTextSize(1);
    display.print("L/M");
 
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity 1: ");
    Serial.print(totalMilliLitres1);
    Serial.print("mL / ");
    Serial.print(totalLitres1);
    Serial.println("L");
    Serial.print("Output Liquid Quantity 2: ");
    Serial.print(totalMilliLitres2);
    Serial.print("mL / ");
    Serial.print(totalLitres2);
    Serial.println("L");
 
 
    display.setCursor(75,20);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("V1:");
    display.print(totalLitres1);
    display.print("L");
    display.setCursor(75,40);  //oled display
    display.print("V2:");
    display.print(totalLitres2);
    display.print("L");
    display.display();
  }

if (flowRate1 == 0.0) {
      // If the flow rate is zero, update the flow start time
      flowStartTime1 = currentMillis;
      
    }
 if (flowRate2 == 0.0) {
    // If the flow rate is zero for sensor 2, update the flow start time
    flowStartTime2 = currentMillis;
  }

  volumeDifference = totalLitres1 - totalLitres2 ;

      // Display the time since the flow rate became non-zero in seconds
  Serial.print("Time Since Flow Start 1: ");
  Serial.print((currentMillis - flowStartTime1) / 1000); // Convert to seconds
  Serial.println(" seconds");

  // Display the time since the flow rate became non-zero for sensor 2 in seconds
  Serial.print("Time Since Flow Start 2: ");
  Serial.print((currentMillis - flowStartTime2) / 1000); // Convert to seconds
  Serial.println(" seconds");


  if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
  {
     String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(float(flowRate1));
      postStr += "&field4=";
      postStr += String(float(flowRate2));
      postStr += "&field2=";
      postStr += String(totalLitres1);
      postStr += "&field5=";
      postStr += String(totalLitres2);
      postStr += "&field3=";
      postStr += String((currentMillis - flowStartTime1) / 1000);
      postStr += "&field6=";
      postStr += String((currentMillis - flowStartTime2) / 1000);
      postStr += "&field7=";  // Assuming field 9 in ThingSpeak channel is reserved for volume difference
      postStr += String(volumeDifference);
      postStr += "\r\n\r\n";
    
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
   
  }
    client.stop();
}