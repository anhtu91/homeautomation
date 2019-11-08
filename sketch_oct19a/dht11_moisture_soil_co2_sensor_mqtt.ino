#include "Adafruit_CCS811.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* ssid = "Wireless Network";
const char* password = "greenzoo@150";
const char* mqtt_server = "192.168.1.2"; 
const char* mqtt_user = "admin1";
const char* mqtt_pass= "admin1";
const int pin_dht11 = 2; // DHT11 Sensor PIN D4
const int DHTTYPE = DHT11;
const unsigned int delay_sensor_moisture = 180000; // Delay for Soil Sensor
const unsigned int moisture_Pin= 0; // Soil Moisture Sensor input at Analog PIN A0

int moisture_value = 0;
float humidity = 0;
float temparature = 0;
float co2 = 0;
float tvoc = 0;
float temp_ccs811 = 0;

char strhumidity[8];
char strtemparature[8];
char strmoisture[8];
char strco2[8];
char strtvoc[8];
char strtemp_ccs811[8];

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_CCS811 ccs;
DHT dht(pin_dht11, DHTTYPE); 

void setup_wifi() {
  // Connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected\n");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()+"\n");
}

void setup_sensor_ccs811(){
  Serial.println("Sensor CO2 CCS811...\n"); 
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.\n");
    while(true);
  }
  
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
}

void reconnect() {
  // Loop until we're reconnected
  Serial.println("In reconnect...\n");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...\n");
    // Attempt to connect
    if (client.connect("ESP8266 CO2", mqtt_user, mqtt_pass)) {
      Serial.println("connected\n");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds\n");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  dht.begin();
  setup_sensor_ccs811();
  client.setServer(mqtt_server, 1883);
}

void loop() {
   if (!client.connected()) {
    reconnect();
  }else{
      //For DHT11 sensor
      humidity = dht.readHumidity();
      temparature = dht.readTemperature();

      Serial.print("\nHumidity: \n");
      Serial.print(humidity);
      Serial.print("\nTemparature: \n");
      Serial.print(temparature);

      dtostrf(humidity, 6, 2, strhumidity);
      dtostrf(temparature, 6, 2, strtemparature);

      client.publish("humidity", strhumidity);
      client.publish("temparature", strtemparature);

      //For CCS811 sensor
      if(ccs.available()){
        co2 = ccs.geteCO2();
        tvoc = ccs.getTVOC();
        temp_ccs811 = ccs.calculateTemperature();
        
        if(!ccs.readData()){
          Serial.print("\nCO2: \n");
          Serial.print(co2);
          Serial.print("\nTVOC: \n");
          Serial.print(tvoc);
          Serial.print("\nTemp from CCS811: \n");
          Serial.print(temp_ccs811);

          dtostrf(co2, 6, 2, strco2);
          dtostrf(tvoc, 6, 2, strtvoc);
          dtostrf(temp_ccs811, 6, 2, strtemp_ccs811);
          
          //send to brocker
          client.publish("co2", strco2);
          client.publish("tvoc", strtvoc);
          client.publish("temp_ccs881", strtemp_ccs811);
        }
        else{
          Serial.println("ERROR!");
        }
      }

      //For soil moisture sensor
      moisture_value= analogRead(moisture_Pin);
      moisture_value= moisture_value/10;
      Serial.print("\nSoil moisture: \n");
      Serial.print(moisture_value);
      
      dtostrf(moisture_value, 6, 2, strmoisture);
      client.publish("soilmoisture", strmoisture);
      
      delay(delay_sensor_moisture);
  }
}
