//This program only support for ESP32
#define DEBUG 1                                                        // 1 DEBUG , 0 PRODUCTION
#include <WiFi.h>                                                      // WIFI Library
#include <PubSubClient.h>                                              // MQTT Library (Version 2.7.0)
#include <Wire.h>                                                      // I2C Library
#include <Adafruit_Sensor.h>                                           // Adafruit Sensor Library
#include <Adafruit_BME280.h>                                           // Temperature and Humidity Library (Version 1.1.0)
#include <strings.h>                                                   // String Library
#include <RTClib.h>                                                    // RTC Library

// Wifi Parameter
const char* ssid = "GASPACE";                                          // Wifi SSID name
const char* password = "gaspace46";                                    // Wifi SSID password
// MQTT Parameter
const char* mqttServer = "34.87.119.216";                              // MQTT Server Addres
const int mqttPort = 1883;                                             // MQTT Server Port
const char* mqttUser = "jeager";                                       // MQTT Server Username
const char* mqttPassword = "Telkom123";                                // MQTT Server Password
const char* topic = "environment_sensor";                              // MQTT App Topic
char data[1024];
// JSON Parameter ESP32
//char* client_id = "5df30e114ccd8b1af4e5cc7b";                          // Jeager App Client ID                     
//char* device_id = "5df30e114ccd8b1af4e5cc7b-1578380179048816021";      // Jeager App Device ID
// JSON Parameter ESP32 Baterai
char* client_id = "5df30e114ccd8b1af4e5cc7b";                          // Jeager App Client ID                     
char* device_id = "5df30e114ccd8b1af4e5cc7b-1579058369820517973";      // Jeager App Device ID
char  times[10];                                                       // Buffer for times parameter
char  date[20];                                                        // Buffer for date parameter
// Timer Parameter 
const char* ntpServer = "pool.ntp.org";                                // NTP Server Address
const long  gmtOffset_sec = 25200;                                     // GMT offset in second (GMT+1 = 3600)(GMT-1 = -3600)(GMT+2=7200)
const int   daylightOffset_sec = 3600;                                 // Daylight offet default is 0 or 3600
float temp,hum;
// Parameter Kalibrasi
float kalibrasi_temp = -(1.2);
float kalibrasi_hum = 0;
// Define Parameter
RTC_DS3231 rtc;
Adafruit_BME280 bme;                                                  // Call Adafruit BME280 Sensor as bme
#define SEALEVELPRESSURE_HPA (1013.25)                                // Sealevel setup parameter
int waktuKirim = 5000;

// MQTT PubSub Connection
WiFiClient espClient;
PubSubClient client(espClient);

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
 
    if (client.connect(device_id, mqttUser, mqttPassword )) {
 
      Serial.println("connected");
 
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

// Get Sensor Value from BME Sensor
void sensorValue()
{
    temp = bme.readTemperature() + kalibrasi_temp;
    hum = bme.readHumidity() + kalibrasi_hum;
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
  sprintf(times,"%02d:%02d:%02d.000", jam, menit, detik);
  sprintf(date,"%04d-%02d-%02d", tahun, bulan, tanggal);
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
  #ifdef DEBUG
    Serial.begin(115200);                                                                                   // Set Baudrate for Serial Communication.
    WiFi.begin(ssid, password);                                                                             // Set WiFi username and Password
    bme.begin();                                                                                            // Inisialisasi sensor BME280
    wifiConnection();                                                                                       // Check WiFi connection
    mqttConnection();                                                                                       // Check MQTT connection
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);                                               // Konfigurasi untuk NTP Server
  #else
   WiFi.begin(ssid, password);                                                                             // Set WiFi username and Password
   bme.begin();                                                                                            //inisialisasi sensor BME280
   wifiConnection();                                                                                       // Check WiFi connection
   mqttConnection();                                                                                       // Check MQTT connection
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);                                               // Konfigurasi untuk NTP Server
  #endif
}

// Main Function
void loop() {
  #ifdef DEBUG
  client.loop();                                                                                           // Make sure connection with MQTT broker                                                          
  RTC();                                                                                                   // Get Time from RTC
  sensorValue();                                                                                          // Get Sensor Value
//  getTime();                                                                                              // Get Local Time From NTP Server
  String payload = jsonData(client_id,device_id,String(temp),String(hum),times,date);                     // Build payload in JSON Format
  mqttPublish(topic, payload);                                                                            // Publish payload to mqtt server
  Serial.println(payload);
  delay (waktuKirim);
  #else
  client.loop();                                                                                           // Make sure connection with MQTT broker                                                          
  sensorValue();                                                                                          // Get Sensor Value
  getTime();                                                                                              // Get Local Time From NTP Server
  String payload = jsonData(client_id,device_id,String(temp),String(hum),times,date);                     // Build payload in JSON Format
  mqttPublish(topic, payload);                                                                            // Publish payload to mqtt server
  delay (waktuKirim);
  #endif
}
