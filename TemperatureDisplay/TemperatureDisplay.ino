// michaelpeterswa

#include <GxEPD2_BW.h>                // Include B/W E-Paper library
#include <Fonts/FreeMonoBold9pt7b.h>  // Include Font 1
#include <Fonts/FreeMonoBold24pt7b.h> // Include Font 2

#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>  // Include the HTTPClient library
#include <ArduinoJson.h>        // Include the JSON library
#include <WifiUdp.h>            // Include the WifiUDP library
#include <NTPClient.h>          // Include the NTPClient library

// display constructor for NodeMCU and 296x128 2.9" E-Paper panel
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 5, /*BUSY=D2*/ 4)); // GDEH029A1

const long utcOffsetInSeconds = -28800;
WiFiUDP ntpUDP;  // Define NTP Client to get time
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const char* ssid     = "";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "";               // The password of the Wi-Fi network

const String coord_lat = "";                         // Latitude for weather report
const String coord_long = "";                      // Longitude for weather report
const String api_key = "";  // API Key for OpenWeatherMap

void setup()
{
  //~~~~SERIAL INITIALIZATION~~~~
  Serial.begin(115200);
  delay(5);
  Serial.println("\n");

  //~~~~WIFI INITIALIZATION~~~~
  Serial.println("Starting Connection:");
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer

  //~~~~DISPLAY INITIALIZATION~~~~

  handleDisplay();
  
  Serial.println("entering deep sleep");
  ESP.deepSleep(3e8);
  
}

const char CurrentTemp[] = "Temperature:";

void printTempData(String temperature){
  int16_t tempbx, tempby; uint16_t tempbw, tempbh;

  display.setFont(&FreeMonoBold24pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.getTextBounds(temperature, 0, 0, &tempbx, &tempby, &tempbw, &tempbh);

  uint16_t x2 = ((display.width() - tempbw) / 2) - tempbx;
  uint16_t y2 = ((display.height() - tempbh) / 2) - tempby;

  display.setCursor(x2, y2); 
  display.print(temperature);
}

void printTempTitle() {
  Serial.println("Printing Text: Temperature");
  
  int16_t tbx, tby; uint16_t tbw, tbh;
  uint16_t offx, offy;
  offx = 10; // 10 pixel offset x
  offy = 10; // 10 pixel offset y
  
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.getTextBounds(CurrentTemp, 0, 0, &tbx, &tby, &tbw, &tbh);

  uint16_t x = ((display.width() - tbw) / 2) - tbx;;
  uint16_t y = abs(tby) + offy;
  
  display.setCursor(x, y);
  display.print(CurrentTemp);

  printHorizLine(x, tbh + offy + 5, tbw);
}

void printHorizLine(uint16_t x, uint16_t y, uint16_t width) {
  display.writeFastHLine(x, y, width, 0);
}

String getTempData() {
  
   double temp = -1;
   String tempDataURL = "http://api.openweathermap.org/data/2.5/weather?lat=" + coord_lat + "&lon=" + coord_long + "&appid=" + api_key + "&units=imperial";
   Serial.println(tempDataURL);
   HTTPClient http; //Object of class HTTPClient
   http.begin(tempDataURL);
   int httpCode = http.GET();
   Serial.println(httpCode);
   if(httpCode == 200) {
    Serial.println(http.getString());
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, http.getString());

    temp = doc["main"]["temp"];
    Serial.println(temp);
   }
   return String(temp) + " F";
}

void printDateTime() {
  timeClient.update();
  Serial.println(timeClient.getFormattedTime()); 
  Serial.println("Printing Text: Time");
  
  int16_t tbx, tby; uint16_t tbw, tbh;
  uint16_t offx, offy;
  offx = 10; // 10 pixel offset x
  offy = 103; // 10 pixel offset y
  
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.getTextBounds(timeClient.getFormattedTime(), 0, 0, &tbx, &tby, &tbw, &tbh);

  uint16_t x = ((display.width() - tbw) / 2) - tbx;;
  uint16_t y = abs(tby) + offy;
  
  display.setCursor(x, y);
  display.print(timeClient.getFormattedTime());

  printHorizLine(x, y - tbh - 5, tbw);
}

void handleDisplay() {
  
  display.init();
  Serial.println("Setting Rotation: 3 (UPSIDE DOWN)");
  display.setRotation(3);
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);

  printTempData(getTempData());
  printTempTitle();
  printDateTime();

  display.display();
  display.hibernate();
}

void loop() {};
