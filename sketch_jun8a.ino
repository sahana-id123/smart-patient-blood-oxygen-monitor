#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 1000
#define WIFISSID "Galaxy M21 2021 Edition 255A" // Put your WifiSSID here
#define PASSWORD "kxos2030"                    // Put your wifi password here
#define TOKEN "BBUS-527oBpLUK4MO59pvviQ28WU6y4vQSD" // Put your Ubidots' TOKEN
#define DEVICE_LABEL "oxygen-monitor"                  // Put the device label
#define VARIABLE_LABEL_1 "heartrate"                   // Put the variable label
#define VARIABLE_LABEL_2 "SPo2"                        // Put the variable label
#define MQTT_CLIENT_NAME "EI_OXMO"      


PulseOximeter pox;
uint32_t tsLastReport = 0;

char mqttBroker[] = "industrial.api.ubidots.com";
char payload[700];
char topic[150];

// Space to store values to send
char str_val_1[6];
char str_val_2[6];

int flag = 0;
float BMP1=6.00;
int SPO21=97;

ESP8266WiFiMulti WiFiMulti;
WiFiClient ubidots;
PubSubClient client(ubidots);

void onBeatDetected()
{
  ;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  WiFiMulti.addAP(WIFISSID, PASSWORD);
  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);

  Serial.print("Initializing pulse oximeter..");
  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
    digitalWrite(1, HIGH);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);

  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);

}

void loop(){
if (flag == 0)
{
  client.connect(MQTT_CLIENT_NAME, TOKEN, "");
  Serial.println("MQTT connected again");
  flag = 1;
}
  if (!client.connected()) {
    Serial.print("Reconnecting ... ");
    reconnect();
  }
  // Make sure to call update as fast as possible
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

    Serial.print("BPM:");
    Serial.print(pox.getHeartRate());
    //blue.println("\n");

    Serial.print(" SpO2: ");
    Serial.print(pox.getSpO2());
    Serial.print("%");
    Serial.println("\n");

 
   dtostrf(pox.getHeartRate(), 4, 2, str_val_1);
    dtostrf(pox.getSpO2(), 4, 2, str_val_2);

    

    Serial.println(payload);
    Serial.println(topic);

    client.publish(topic, payload);
    client.loop();

    tsLastReport = millis();

  }
}
