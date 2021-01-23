#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include "settings.h"

int LEDRELAY = D2;

/* Configure client WiFi */
WiFiClient espClient;

/* Configure MQTT */
PubSubClient clientMqtt(espClient);
char msg[50];
String mqttcommand = String(14);

/* Flow sensor configuration */
int FlowSensorState = 0;
float CountFlow = 0;
float CountLitre = 0;
long CountStart = 0;

void setup() {
  Serial.begin(115200);

  // GPIO7 is set OUTPUT

  pinMode(LEDRELAY, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  /* setup wifi */
  setup_wifi();
  clientMqtt.setServer(mqtt_server, mqtt_port);
  clientMqtt.setCallback(callback);

  /* define sensor */
  pinMode(FlowSensorPin, INPUT);
}

void setup_wifi() {

  delay(10);

  // The process of connecting to the WiFi network begins
  Serial.println();
  Serial.print("[WIFI] Connecting to ");
  Serial.println(ssid);

  // Station mode
  WiFi.mode(WIFI_STA);
  // Inicio WiFi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("[WIFI]WiFi Connected");
  Serial.print("[WIFI]IP: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[MQTT] Message received (");
  Serial.print(topic);
  Serial.print(") ");
  mqttcommand = "";
  for (int i = 0; i < length; i++) {
    mqttcommand += (char)payload[i];
  }
  Serial.print(mqttcommand);
  Serial.println();
  // Switch on the water pump if mqtt command recived
  if (mqttcommand == "utilWaterON") {
    Serial.println("utilWaterON");
    digitalWrite(LEDRELAY, HIGH); // LED/RELAY ON
  } else if (mqttcommand == "utilWaterOFF") {
    Serial.println("utilWaterOFF");
    digitalWrite(LEDRELAY, LOW); // LED/RELAY OFF
  }
}

void reconnect() {
  Serial.print("[MQTT] Trying to connect to MQTT server ... ");
  // Loop until connection is made
  while (!clientMqtt.connected()) {
    Serial.print(".");
    // Start the MQTT Connection...
    if (clientMqtt.connect("MQTT_NAME", "MQTT_USERNAME", "MQTT_PASSWORD")) { // Be careful, for more than one device change the name to avoid conflict
      Serial.println("");
      Serial.println("[MQTT] Connected to the MQTT server");
      // Once connected, publish an announcement...
      clientMqtt.publish(mqtt_sub_topic_healthcheck, "starting");
      // ... and subscribe
      clientMqtt.subscribe(mqtt_sub_topic_operation);
    } else {
      Serial.print("[MQTT]Error, rc=");
      Serial.print(clientMqtt.state());
      Serial.println("[MQTT] Trying to connect in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!clientMqtt.connected()) {
    reconnect();
  }
  clientMqtt.loop();

  FlowSensorState = digitalRead(FlowSensorPin);
  if (FlowSensorState == HIGH) {
    CountFlow += 1;
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  } else {
    digitalWrite(LED_BUILTIN, HIGH) ;    // turn the LED off by making the voltage LOW
  }
  long now_sensors = millis();
  if (now_sensors - CountStart > update_time_sensors) {
    CountStart = now_sensors;
    Serial.println("Resume--------------------------------------");
    if (CountFlow > 1500000 || CountFlow == 0) {
      clientMqtt.publish(mqtt_pub_topic_waterflow, "0");
    } else {
      //Flow pulse characteristics (6.6*L/Min)
      CountLitre = CountFlow * 6.6 / 2815562;
      snprintf (msg, 10, "%6.2f", CountLitre);
      clientMqtt.publish(mqtt_pub_topic_waterflow, msg);
      Serial.print(msg);
      Serial.println(" liters");
    }
    Serial.println(CountFlow);
    Serial.println("");
    CountFlow = 0;
  }
}
