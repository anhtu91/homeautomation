#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wireless Network";
const char* password = "greenzoo@150";
const char* mqtt_server = "192.168.1.2"; 
const char* mqtt_user = "admin1";
const char* mqtt_pass= "admin1";

const unsigned int move_sensor = 13;
//const unsigned int rain_sensor = A0;

int IsMove;
char strmove[8];

//int IsRain;
//char strrain[8];

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  // Connecting to a WiFi network
  WiFi.hostname("ESP8266 Motion Detector");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected\n");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()+"\n");
}

void reconnect() {
  // Loop until we're reconnected
  Serial.println("In reconnect...\n");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...\n");
    // Attempt to connect
    if (client.connect("ESP8266 Movement", mqtt_user, mqtt_pass)) {
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
  pinMode(move_sensor, INPUT); 
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }else{
    //Movement Sensor
    if(digitalRead(move_sensor) == HIGH) {
       IsMove = 1;
       Serial.print("Move\n");
    }else{
       IsMove = 0;
       Serial.print("No Move\n");
    }
      
    dtostrf(IsMove, 6, 2, strmove);
    client.publish("move", strmove);
    
    //Rain Sensor
    /*IsRain = analogRead(rain_sensor);
    IsRain = constrain(IsRain, 150, 440); 
    IsRain = map(IsRain, 150, 440, 1023, 0); 

    dtostrf(IsRain, 6, 2, strrain); //If IsRain >= 20 => Rain
    client.publish("rain", strrain);*/
    delay(200);
  }
}
