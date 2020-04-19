#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "DHTesp.h"
#include <PubSubClient.h>

//include passwords
#include "credentials.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER_IP;
const char* user= MQTT_USER;
const char* passw= MQTT_PASSWORD;

//Wifi
WiFiClient espBedCabinet;
PubSubClient client(espBedCabinet);

//mqtt messages
#define MSG_BUFFER_SIZE	(50)
char msg_temp[MSG_BUFFER_SIZE];
char msg_humidity[MSG_BUFFER_SIZE];
char msg_door[MSG_BUFFER_SIZE];
String receivedString;

//DHT11 sensor
#define dhtpin 0
DHTesp dht;

//door-sensor
#define sensor 2
bool doorStatus = false;
bool doorStatusOld = false;

//timer for sensor-publish
unsigned long timer = 0;
const long interval = 20000; //20 Sek.


//-------------------------------------functions----------------------------------

//reconnect for mqtt-broker
void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect("espBedCabinet", user, passw)) {
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


//-------------------------------MAIN------------------------------------------------
void setup() {
  
  Serial.begin(9600);

  //DHT11
  dht.setup(dhtpin, DHTesp::DHT11);

  //Wifi
  WiFi.hostname("espBedCabinet");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  //mqtt
  client.setServer(mqtt_server, 1883);

  //door-sensor
  pinMode(sensor, INPUT);

}

void loop() {

  //check mqtt-connection
  if (!client.connected()){
    reconnect();
  }
  client.loop();

  //timer
  unsigned long currentTime = millis();

  if (currentTime - timer >= interval){

    Serial.println("send data");

    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();

    char helpval[8];
    dtostrf(temperature, 6, 2, helpval);
    snprintf (msg_temp, MSG_BUFFER_SIZE, helpval);
    client.publish("home/bedroom/bed/temperature", msg_temp);
    delay(100);

    dtostrf(humidity, 6, 2, helpval);
    snprintf (msg_humidity, MSG_BUFFER_SIZE, helpval);
    client.publish("home/bedroom/bed/humidity", msg_humidity);
    delay(100);

    timer = currentTime;

  }

  doorStatus = digitalRead(sensor);

  if (doorStatus != doorStatusOld) {

    if (doorStatus == HIGH) {
      client.publish("home/bedroom/bed/door", "OPEN");
      delay(100);
    }
    else{
      client.publish("home/bedroom/bed/door", "CLOSED");
      delay(100);
    }

    doorStatusOld = doorStatus;

  }


}