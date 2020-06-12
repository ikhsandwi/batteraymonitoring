#include <loraid.h>                                                    // LoRa Antares Library
#include <Adafruit_Sensor.h>                                           // Adafruit Sensor Library
#include <Adafruit_BME280.h>                                           // BME280 Sensor Library     

Adafruit_BME280 bme;                                                    // Inisialisasi BME 
long interval = 10000;                                                  // 10 s interval to send message
long previousMillis = 0;                                                // will store last time message sent
unsigned int counter = 0;                                               // message counter
float temp,hum;                                                         // Temp and Hum parameter   
int recvStatus = 1;                                                     // Message Status 
String deviceid = "1579162318409543664";                                // Device Status From Jeager 
// Parameter Kalibrasi
float kalibrasi_temp = -(1.2);
float kalibrasi_hum = 0;

// Get Sensor Value from BME Sensor
void sensorValue()
{
    temp = bme.readTemperature() + kalibrasi_temp;
    hum = bme.readHumidity() + kalibrasi_hum;
}

void setup() {
  // Setup loraid access
  lora.init();
  // Set LoRaWAN Class. CLASS_A and CLASS_C are available
  lora.setDeviceClass(CLASS_A);
  // Set Data Rate
	// SF12 to SF7 available. Also SF7_250 for 250KHz variant	
  lora.setDataRate(SF12);
  // Put your Antares account key and Lora.id device address here
  lora.setAccessKey("a08c0faa1b988b11:9ae97d4dee93a723");
  lora.setDeviceId("b9a3bf46");
  // Set I2C BME280
  bme.begin(); 
}

void loop() {
  sensorValue(); 
  unsigned long currentMillis = millis();

  // Check if more than 10 seconds
	if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis; 

 		String data = String(temp) + "," + String(hum) +","+ deviceid;   
    lora.sendToAntares(data,recvStatus);
  }
  // Check Lora RX
  lora.update();
}
