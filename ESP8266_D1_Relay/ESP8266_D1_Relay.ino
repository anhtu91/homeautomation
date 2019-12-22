#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const char* ssid = "Wireless Network";
const char* password = "greenzoo@150";
const char* mqtt_server = "192.168.1.2"; 
const char* mqtt_user = "admin2";
const char* mqtt_pass= "admin2";

const unsigned int pirPin = 14;
const unsigned int relayPin = 5;
const unsigned int lightPin = A0;
const unsigned int remotePin = 4;
int lightValue = 0;
int pirValue = 0;

const unsigned int delay_relay = 300000; //5 minutes stop

IRrecv irrecv(remotePin);
decode_results results;

WiFiClient espClient;
PubSubClient client(espClient);

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

void reconnect() {
  // Loop until we're reconnected
  Serial.println("In reconnect...\n");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...\n");
    // Attempt to connect
    if (client.connect("ESP8266 Relay", mqtt_user, mqtt_pass)) {
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
  pinMode(pirPin, INPUT);
  pinMode(relayPin, OUTPUT);
  irrecv.enableIRIn(); //Start the remote receiver
  client.setServer(mqtt_server, 1883);
}

void loop() { 
  if (!client.connected()) {
    reconnect();
  }else{
    pirValue = digitalRead(pirPin);
    lightValue = analogRead(lightPin);

    if(irrecv.decode(&results)){
      int str_value = results.value;

      if(str_value == 16724175){
        Serial.println("Enter 1");
        digitalWrite(relayPin, LOW); //TURN ON LIGHT
        delay(delay_relay);
      }else if(str_value == 16718055){
        Serial.println("Enter 2");
        digitalWrite(relayPin, HIGH); //TURN OFF LIGHT
      }
      
      irrecv.resume();      
    }

    if(pirValue == HIGH) { //MOVE
      Serial.println("Move");
      
      if(lightValue < 50){ //DARK
        client.publish("light", "Dark");
        Serial.println("Dark");
        digitalWrite(relayPin, LOW); //TURN ON LIGHT
      }else{ //LIGHT
        client.publish("light", "Light");
        Serial.println("Light");
      }
      
      client.publish("move", "1");  
    }else{ //NOT MOVE
       Serial.println("Not move");
       digitalWrite(relayPin, HIGH);
       
       if(lightValue < 50){ //DARK
         client.publish("light", "Dark");
         Serial.println("Dark");
       }else{ //LIGHT
         client.publish("light", "Light");
         Serial.println("Light");
       }
       
       client.publish("move", "0");
    }
  }
}
