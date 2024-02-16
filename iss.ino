
/*
 *  ESP8266 JSON Decode of server response
 *  -Manoj R. Thkuar
 *  https://circuits4you.com
 */
#include <time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>     // Core graphics library
//#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <SPI.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#define FS_NO_GLOBALS
#include <FS.h>
// Call up the TFT library
#include <TFT_eSPI.h> // Hardware-specific library for ESP8266


// Invoke TFT library
TFT_eSPI tft = TFT_eSPI();


float p = 3.1415926;

// Variables de travail
unsigned long epoch = 0;
int nujour = 0; //numero jour de la semaine avec 0 pour dimanche
String jour = "mon jour"; // dimanche, lundi, etc.
String heure = "mon heure ..";
char buffer[80]; // Stockage de la date complete


// ST7789 TFT module connections
#define TFT_RST   15     // TFT RST pin is connected to NodeMCU pin D8 (GPIO15)
#define TFT_DC    2     // TFT DC  pin is connected to NodeMCU pin D4 (GPIO2)
#define TFT_CS    -1     // TFT CS  pin is directly connected to GND

// initialize ST7789 TFT library with hardware SPI module
// SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
// MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//Creation objet WIFI UDP puis du client NTP
WiFiUDP ntpUDP;
//Creation objet client NTP avec les parametres suivants :
// - pool de serveurs NTP
// - en option le décalage horaire en secondes, ici 3600 pour GMT+1, pour GMT+8 mettre 28800, etc.
// - en option l intervalle de mise à jour en millisecondes par défaut à 60 secondes
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

 
void setup(void) {
       
  //tft.init(240, 240, SPI_MODE2);  
  uint16_t time = millis();

  time = millis() - time;

   tft.begin();
   
  // Demarrage client NTP
  timeClient.begin(); 

   WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
    Serial.begin(115200);
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    //wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
     res = wm.autoConnect("SPACE CONNECTION"); // anonymous ap
    //res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!res) {
      tft.fillScreen(TFT_BLACK);
      delay(1000);
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);  // set text color to white and black background    
      tft.setTextSize(3); 
      tft.setCursor(0,80);  
      tft.println("wifi:");
      tft.setCursor(0,140);
      tft.setTextSize(2);
      tft.setTextColor(TFT_MAGENTA);  
      tft.println("SPACE CONNECTION");
     
    } 
    else {
        //if you get here you have connected to the WiFi    
        
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);  // set text color to white and black background    
      tft.setTextSize(3); 
  tft.setCursor(0,100);   
  tft.print("Connecté");
  delay(1000);
  
  }
   
}

void loop() { 
 
  // Recup heure puis affichage
  timeClient.update();
  epoch = timeClient.getEpochTime(); // Heure Unix
  nujour = timeClient.getDay();    // jour de la semaine
  heure = timeClient.getFormattedTime(); // heure
  // Calcul de la date en convertissant le temps UNIX epoch
  time_t timestamp = epoch;
  struct tm * pTime = localtime( & timestamp );
  strftime( buffer,80, "%d/%m/%Y", pTime );
  //Serial.println(buffer);

  switch (nujour) { // on determine le jour
      case 0: 
        jour = "dimanche";
        break;
      case 1:
        jour = "lundi";
        break;
      case 2: 
        jour = "mardi";
        break;
      case 3: 
        jour = "mercredi";
        break;
      case 4: 
        jour = "jeudi";
        break;
      case 5: 
        jour = "vendredi";
        break;
       case 6: 
        jour = "samedi";
        break;
    }
tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0); 
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);    
  tft.print(jour);
  tft.setCursor(20, 50); 
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(3);    
  tft.print(buffer);
  tft.setTextSize(4);   
  tft.setCursor(0, 100);
  tft.setTextColor(TFT_MAGENTA);
  tft.print("----------");
  tft.setCursor(0, 160); 
  tft.setTextColor(TFT_GREEN);  
  tft.setTextSize(5);    
  tft.print(heure);
  delay(4000); // tempo de 4 sec

tft.fillScreen(TFT_BLACK);
  
  HTTPClient http;    //Declare object of class HTTPClient
  
  http.begin("http://api.open-notify.org/astros.json");     //Specify request destination
  
  int httpCode = http.GET();            //Send the request
  String payload = http.getString();    //Get the response payload from server
 
  if(httpCode == 200)
  {
    // Allocate JsonBuffer
    // Use arduinojson.org/assistant to compute the capacity.
    //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 1024;
    // Stream& input;

StaticJsonDocument<1024> doc;

DeserializationError error = deserializeJson(doc, payload);

if (error) {
  tft.print(F("deserializeJson() failed: "));
  tft.println(error.f_str());
  return;
}

int number = doc["number"]; // 10 
tft.fillScreen(TFT_BLACK);
  tft.setTextWrap(true);
  tft.setTextSize(3); 
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0,0);
  tft.print("-ASTRONAUTES-");
  tft.setCursor(0,35);
  tft.print("dans l'espace");
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(7);  
  tft.setCursor(80,80);
  tft.print(number);
  tft.drawRect (1,155,85,240,TFT_BLACK); 

  //tft.println(payload);    //Print request response payload
  //delay(6000);

for (JsonObject people_item : doc["people"].as<JsonArray>()) {

  const char* people_item_craft = people_item["craft"]; // "Shenzhou 13", "Shenzhou 13", "Shenzhou 13", ...
  const char* people_item_name = people_item["name"]; // "Zhai Zhigang", "Wang Yaping", "Ye Guangfu", ...

  for ( int j = 0; j < number; ++j ) // output each array element's value {
      tft.setTextWrap(true);
      tft.setTextColor(TFT_WHITE);  
      tft.setTextSize(3); 
      tft.setCursor(0,160); 
      tft.print (people_item_name) ;
      tft.setTextColor(TFT_YELLOW);   
      tft.setTextSize(2); 
      tft.setCursor(0,210);
      tft.print (people_item_craft);
      delay(2500);
      tft.fillRect (1, 155, 240, 85, TFT_BLACK);           
}

}
  http.end();  //Close connection
  
  http.begin("http://api.open-notify.org/iss-now.json");     //Specify request destination
  
  int httpCodeb = http.GET();            //Send the request
  String payloadb = http.getString();    //Get the response payload from server
  
  
  if(httpCodeb == 200)
  {
    // Allocate JsonBuffer
    // Use arduinojson.org/assistant to compute the capacity.
    const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3)+ 192;
    // Stream& input;

// Stream& input;

StaticJsonDocument<192> docb;

DeserializationError error = deserializeJson(docb, payloadb);

if (error) {
  tft.print(F("deserializeJson() failed: "));
  tft.println(error.f_str());
  return;
}

long timestamp = docb["timestamp"]; // 1649869660
const char* messageb = docb["message"]; // "success"

const char* iss_position_longitude = docb["iss_position"]["longitude"]; // "-121.8565"
const char* iss_position_latitude = docb["iss_position"]["latitude"]; // "39.6452"

tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_MAGENTA);  // set text color to white and black background    
  tft.setTextSize(3);  
  tft.setCursor(0,0);
  tft.setTextWrap(true);  
  tft.print("ISS POSITION:");
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3); 
  tft.print("-------------");
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(4);  
  tft.setCursor(0,60);
  tft.print("Longitude:");
  tft.setTextSize(3);
  tft.setCursor(30,110);
  tft.setTextColor(TFT_WHITE);  
  tft.print(iss_position_longitude);
  tft.setTextSize(4); 
  tft.setCursor(0,160);
  tft.setTextColor(TFT_GREEN);
  tft.print("Latitude:");
  tft.setTextSize(3); 
  tft.setCursor(30,210);
  tft.setTextColor(TFT_WHITE);
  tft.print(iss_position_latitude);
  delay(4000); 
 tft.fillScreen(TFT_BLACK);
 }
  else
  {
    tft.println("Error in response ISS");
    delay(1500);
  }
  http.end();  //Close connection
    }
    
