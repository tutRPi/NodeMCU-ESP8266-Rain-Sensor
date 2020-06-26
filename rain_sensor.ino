#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define USE_SERIAL Serial


const char* WIFISSID = "Wifi Name"; // SSID name
const char* WIFIPASS = "Wifi Passwort"; // password
const char* MQTT_BROKER = "192.168.1.220"; // IP of the MQTT Broker
const char* MQTT_CHANNEL = "/RainDrop/data"; // in which Channel should the results be published
const char* MQTT_CLIENT_NAME = "ESP8266ClientRain1";

bool firstLoop = true; // Indicates that this is the first loop

int triggerPin = 14;    // The stick reading about it tipped.
int totalCount   = 0;    // a simple calculator that adds how many tipping it has done since its start.

const char* mmPerSquareMeter = "0.094175";// 1.25 ml per dipper change, 94.175 ml per m^2 (=1.25*75.34), = 0.094175mm/m^2
                                          // diameter of raingauge is about 130mm, r = 65mm, surface = pi*r^2 = 132.73 cm^2 
                                          // therefore per m^2 (=10000cm^2) we have a factor of about 75.34

bool lastState = false;    // Specifies which mode was last time.
bool currentState = false; // Specifies the state of the loop now
unsigned long lastChanged = 0; // when did we change the status the last time

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(MQTT_BROKER, 1883);
}
 
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFISSID);
 
    WiFi.begin(WIFISSID, WIFIPASS);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
 
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
 

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    if (client.connect(MQTT_CLIENT_NAME)) {
    //if (client.connect(MQTT_CLIENT_NAME, mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

    // connect to MQTT broker
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
  
    int currentState = digitalRead(triggerPin);
    if (firstLoop){
        lastState = currentState;
        firstLoop = false;       // Sets that it has now run the startup.

    };  //  If it is the first startup then it should not send a dump
     
    // Checks the value of the stick, translates it to a bool value.
    if (currentState == 1){currentState = true;} else {currentState = false;}

    // Checks whether the previous loop has the same loop as the current loop. Will not go into this first loop.
    if (lastState != currentState){

      lastState = currentState;
      
      // between two changes, there should be 1 sec difference
      if ((millis() - lastChanged) > 1000 ) {
        totalCount++;
        lastChanged = millis();
  
        client.publish(MQTT_CHANNEL, mmPerSquareMeter );
  
        Serial.print("Number of times:");
        Serial.println(totalCount);
      } 
    }
    

    delay(150);        // small delay to avoid false readings.

}
