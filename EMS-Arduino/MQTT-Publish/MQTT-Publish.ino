#include <WiFi.h>                                                      // WIFI Library
#include <ESP32Ping.h>                                                 // ESP32 Ping Library
#include <PubSubClient.h>                                              // MQTT Library (Version 2.7.0)
#include <Wire.h>                                                      // I2C Library
#include <Adafruit_Sensor.h>                                           // Adafruit Sensor Library
#include <Adafruit_BME280.h>                                           // Temperature and Humidity Library (Version 1.1.0)
#include <driver/adc.h>                                                // ADC Library
#include <strings.h>                                                   // String Library
#include <EEPROM.h>                                                    // EEPROM Library
#include <BlynkSimpleEsp32.h>                                          // Blynk Library
#include <RTClib.h>                                                    // RTC Library
#include <DNSServer.h>                                                 // DNS Server Library (https://github.com/zhouhan0126/DNSServer---esp32)
#include <WebServer.h>                                                 // Web Server Library (https://github.com/zhouhan0126/WebServer-esp32)
#include <WiFiManager.h>                                               // WiFi Manager Library (https://github.com/Brunez3BD/WIFIMANAGER-ESP32)

//ESP32 Wake Up Push Button
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex

//Blynk Server
const char*  auth = "sPwSKszxzM4OuYZck4nRynOzN4rH4n9T";                // Authentification Code to access blynk App
const char*  host = "34.87.51.72";                                     // Blynk Server Addres
int port = 8080;                                                       // Blynk Server Port
// DeepSleep Parameter
#define uS_TO_S_FACTOR 1000000                                         // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  10                                             // Time ESP32 will go to sleep (in seconds)
// Wifi Parameter
const char* ssid = "GASPACE";                                          // Wifi SSID name
const char* password = "gaspace46";                                    // Wifi SSID password
// Wifi Manager
const char* AP = "Jeager-Node";                                        // Wifi SSID name
const char* pass = "12345678";                                         // Wifi SSID password
bool wifistatus;
// MQTT Parameter
const char* mqttServer = "34.87.119.216";                              // MQTT Server Addres
const int mqttPort = 1883;                                             // MQTT Server Port
const char* mqttUser = "jeager";                                       // MQTT Server Username
const char* mqttPassword = "Telkom123";                                // MQTT Server Password
const char* topic = "environment_sensor";                              // MQTT App Topic
char data[1024];
// JSON Parameter
char* client_id = "5df30e114ccd8b1af4e5cc7b";                          // Jeager App Client ID                     
char* device_id = "5df30e114ccd8b1af4e5cc7b-1578380179048816021";      // Jeager App Device ID
char  times[10];                                                       // Buffer for times parameter
char  date[20];                                                        // Buffer for date parameter
// Timer Parameter 
const char* ntpServer = "pool.ntp.org";                                // NTP Server Address
const long  gmtOffset_sec = 25200;                                     // GMT offset in second (GMT+1 = 3600)(GMT-1 = -3600)(GMT+2=7200)
const int   daylightOffset_sec = 3600;                                 // Daylight offet default is 0 or 3600
// Sensor Parameter
const float battery_max = 4.1;                                         // Maximum voltage of battery
const float battery_min = 3.0;                                         // Minimum voltage of battery before shutdown
float temp,hum,pres,alt,battery,Tmax,Tmin,Hmax,Hmin;
int waktu_addr=6,
    Tmax_addr=7,
    Tmin_addr=8,
    Hmax_addr=9,
    Hmin_addr=10;

// Define Parameter
RTC_DS3231 rtc;
Adafruit_BME280 bme;                                                  // Call Adafruit BME280 Sensor as bme
WiFiManager wifiManager;                                              // Call WiFiManager Library as wifimanager
#define SEALEVELPRESSURE_HPA (1013.25)                                // Sealevel setup parameter

// MQTT PubSub Connection
WiFiClient espClient;
PubSubClient client(espClient);

//
void wakeupSetAP(){
  // This statement will declare pin 15 as digital output 
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  if ( wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
     Serial.println("Reset");
     if(!wifiManager.startConfigPortal(AP, pass) ){
        Serial.println("Failed to Connect");
        delay(2000);
        ESP.restart();
        delay(1000);
        }
     }
}

// Check Wifi Connection
void wifiConnection(){
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
 
}

// Check MQTT Connection
void mqttConnection(){
    client.setServer(mqttServer, mqttPort);
    while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");
 
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

// Callback Function for Wifi Manager Setup
void configModeCallback (WiFiManager *myWiFiManager) {  
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

// Callback Function for Save Wifi Manager Setup
void saveConfigCallback () {
  Serial.println("Should save config");
  Serial.println(WiFi.softAPIP()); 
}

// Function WiFi Manager Access Point Mode
void wifiAPMode(){
  if ( digitalRead(5) == LOW) {
      Serial.println("Reset");
      wifistatus = false;
      if(!wifiManager.startConfigPortal(AP, pass) ){
        Serial.println("Failed to Connect");
        delay(2000);
        ESP.restart();
        delay(1000);
      }
  }
}
// Get Sensor Value from BME Sensor
void sensorValue()
{
    temp = bme.readTemperature();
    pres = bme.readPressure() / 100.0F;
    alt = bme.readAltitude(SEALEVELPRESSURE_HPA);
    hum = bme.readHumidity();
}

// Get Battery Level Value from Resistor
float batteryValue()
{
    long sum = 0;                  // sum of samples taken
    float voltage = 0.0;           // calculated voltage
    float output = 0.0;            // output value
    float R1 = 200000.0;           // resistance of R1 (200K)
    float R2 = 200000.0;           // resistance of R2(200K)

    for (int i = 0; i < 100; i++)
    {
        sum += adc1_get_voltage(ADC1_CHANNEL_0);
        delayMicroseconds(1000);
    }
    // calculate the voltage
    voltage = sum / (float)100;
    voltage = (voltage * 1.1) / 2047.0; //for internal 1.1v reference
    voltage = voltage / (R2/(R1+R2));
    output = ((voltage - battery_min) / (battery_max - battery_min)) * 100;
    if (output < 100)
        return output;
    else
        return 100.0f;
}

//Function Read EEPROM
void readEEPROM(){
  Tmax = EEPROM.read(Tmax_addr);
  Tmin = EEPROM.read(Tmin_addr);
  Hmax = EEPROM.read(Hmax_addr);
  Hmin = EEPROM.read(Hmin_addr);
}

// Function Platform Blynk
void platformBlynk(){
// Blynk Setup
  Blynk.begin(auth, ssid, password, host, port);
  Blynk.run(); 
// Blynk Get Sensor Parameter
  Blynk.virtualWrite(V0,temp);
  Blynk.virtualWrite(V1,pres);
  Blynk.virtualWrite(V2,alt);
  Blynk.virtualWrite(V3,hum);
  Blynk.virtualWrite(V10,battery);

// Blynk Treshold Configuration
  Blynk.virtualWrite(V20,Tmax);
  Blynk.virtualWrite(V21,Tmin);
  Blynk.virtualWrite(V22,Hmax);
  Blynk.virtualWrite(V23,Hmin);

// Blynk Notification Configuration
  if(temp>Tmax){
    Blynk.notify("Temperature is HIGH !!");
  }
  if(temp<Tmin){
    Blynk.notify("Temperature is LOW !!");
  }
  if(hum>Hmax){
    Blynk.notify("Humidity is HIGH !!");
  }
  if(hum<Hmin){
    Blynk.notify("Humidity is LOW !!");
  }
}

// Input threshold sensor from Blynk App and Write to EEPROM
  BLYNK_WRITE(V4)
  {
    Tmax = param.asInt(); 
    EEPROM.write(Tmax_addr,Tmax);
    EEPROM.commit();
  }
  BLYNK_WRITE(V5)
  {
    Tmin = param.asInt();
    EEPROM.write(Tmin_addr,Tmin);
    EEPROM.commit();
  }
  BLYNK_WRITE(V6)
  {
    Hmax = param.asInt(); 
    EEPROM.write(Hmax_addr,Hmax);
    EEPROM.commit();
  }
  BLYNK_WRITE(V7)
  {
    Hmin = param.asInt();
    EEPROM.write(Hmin_addr,Hmin);
    EEPROM.commit();
  }


// Get Local Time from NTP Server
void getTime()
{
  struct tm timeinfo;
  int jam,menit,detik,tanggal,bulan,tahun;
// Check Localtime Data
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
// Parse Time Object
  jam =  timeinfo.tm_hour;
  menit  = timeinfo.tm_min;
  detik  = timeinfo.tm_sec;
  tanggal = timeinfo.tm_mday;
  bulan = timeinfo.tm_mon + 1;
  tahun = timeinfo.tm_year +1900;
// Build time parameter as string object
  sprintf(times,"%02d:%02d:%02d.000", jam, menit, detik);
  sprintf(date,"%04d-%02d-%02d", tahun, bulan, tanggal);
}

// Get Time from RTC device 
void RTC(){
  int jam,menit,detik,tanggal,bulan,tahun;
  DateTime now = rtc.now();
  jam = now.hour();
  menit = now.minute();
  detik = now.second();
  tanggal = now.day();
  bulan = now.month();
  tahun = now.year();
//  sprintf(waktu, "%02d/%02d/%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());  
  sprintf(times,"%02d:%02d:%02d.000", jam, menit, detik);
  sprintf(date,"%04d-%02d-%02d", tahun, bulan, tanggal);
//  Serial.print(F("Time: "));
//  Serial.println(waktu);
//  delay(1000);
}


// Convert String to Char
void mqttPublish(String topic, String value) {
    client.publish((char * ) topic.c_str(), (char * ) value.c_str());
}

// Formating data to JSON
String jsonData(String client_id, String device_id, String temperature, String humidity, String times, String date){
    String json = "{\"client_id\": \"" + client_id + "\", \"device_id\": \"" + device_id + "\", \"sensors\": [{\"sensor_name\": \"temperatur\", \"value\": "+ temperature +"}, {\"sensor_name\": \"kelembapan\", \"value\": " + humidity +"}], \"time\": \"" + date  +" " + times +" +0700\"}";
    return json ;
}

// Init Function
void setup() {
  Serial.begin(115200);                                                                                   // Set Baudrate for Serial Communication
  wifiManager.autoConnect(AP,pass);                                                                       // Autoconnect with recent value from eeprom
  wifiManager.setAPCallback(configModeCallback);                                                          // WiFi Manager mode CallBack
  wifiManager.setSaveConfigCallback(saveConfigCallback);                                                  // WiFi Manager Save CallBack
  wakeupSetAP();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,1);                                                            // Wake up ESP32 with push button pin 15, 1 = High, 0 = Low
//  WiFi.begin(ssid, password);                                                                             // Set WiFi username and Password
//  bme.begin();                                                                                            //inisialisasi sensor BME280
//   EEPROM.begin(512);                                                                                      //inisialisasi EEPROM
//  wifiConnection();                                                                                       // Check WiFi connection
   mqttConnection();                                                                                       // Check MQTT connection
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);                                               // Set Local Time parameter
   esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);                                          // Set timer to deep sleep
}

// Main Function
void loop() {
  client.loop();                                                           
//   readEEPROM();                                                                                           // Get EEPROM Value
//  sensorValue();                                                                                          // Get Sensor Value
//  battery = batteryValue();                                                                               // Get Battery Level
//   platformBlynk();                                                                                        // Platform Blynk Function    
   temp = random(20, 35);
   hum = random(50, 100);
//   RTC();
  getTime();                                                                                              // Get Local Time From NTP Server
  String payload = jsonData(client_id,device_id,String(temp),String(hum),times,date);                     // Build payload in JSON Format
  mqttPublish(topic, payload);                                                                            // Publish payload to mqtt server
//  delay (5000);
  Serial.println(payload);
//  Serial.print(temp);
//  Serial.print("°C ");
//  Serial.print(hum);
//  Serial.println("%");
//  Serial.println(battery);
//  Serial.flush();
  esp_deep_sleep_start();                                                                                 //ESP deep sleep start
}
