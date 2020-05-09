#include "secrets.h"
#include "queue.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <EmonLib.h>

//baud rate is 9600 by default

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "ESP32-Mk1/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "ESP32-Mk1/sub"
#define SAMPLE_INTERVAL 5000

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);
EnergyMonitor emonl;

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  unsigned long time_t = millis();
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    if (millis()-time_t > SAMPLE_INTERVAL) {
      Serial.print("Fail to");
      return ;
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi Connection Timeout!");
      return;
  }
  Serial.println("Connected to Wi-Fi!");

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  //WARNING: Don't forget to attach policy in the IoT console, otherwise connection always fail, check secrets.h for details
  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT: ");
  Serial.println(THINGNAME);

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage(const float realPower,
                    const float apparentPower,
                    const float powerFactor,
                    const float Vrms,
                    const float Irms )
{
   StaticJsonDocument<200> doc;
   doc["room"] = "eceb2020";
   doc["realPower"] = realPower;
   doc["apparentPower"] = apparentPower;
   doc["powerFactor"] = powerFactor;
   doc["Vrms"] = Vrms;
   doc["Irms"] = Irms;
  
  
   char jsonBuffer[512];
   serializeJson(doc, jsonBuffer); // print to client
  
   client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
   Serial.println("Message Published!");
//  Serial.print("realPower="); Serial.println(realPower);
//  Serial.print("apparentPower="); Serial.println(apparentPower);
//  Serial.print("powerFActor ="); Serial.println(powerFactor);
//  Serial.print("Vrms="); Serial.println(Vrms);
//  Serial.print("Irms ="); Serial.println(Irms);
//  Serial.println("************************Next Loop**************");
  }

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
}

void setup() {
  Serial.begin(9600);
  emonl.voltage(35, 1.84521649422, -7);
  emonl.current(34, 0.69661151385);
  Serial.println("Emonlib Set!");
//  connectAWS();

}

float glitch_filtered_sum(float data[], int count) {
  switch (count) {
    case 0:
      return 0.0;
    case 1:
      return data[0];
    case 2:
      return (data[0]+data[1])/2.0;
    default:
      float sum = 0.0, max = 0.0, min = data[0];
      for (int i = 0; i < count; ++i) {
        if (data[i] > max) max = data[i];
        if (data[i] < min) min = data[i];
        sum += data[i];
      }
      return (sum-max-min) / (float)(count-2);
  }
}

void loop() {
  struct Queue* q = createQueue();
  unsigned long time_t;
  emonl.calcVI(20,2000);

  float realPower[30];
  float apparentPower[30];
  float powerFactor[30];
  float Vrms[30];
  float Irms[30];

  int entry_count;
  while (true) {
    /* main loop */
    time_t = millis();
    Serial.println("Sampling!");

    entry_count = 0;
    do {
      /* sample and average
          no need to clear array because we keep track of index
       */
      emonl.calcVI(20,2000);
      realPower[entry_count] = emonl.realPower;
      apparentPower[entry_count] = emonl.apparentPower;
      powerFactor[entry_count] = emonl.powerFactor;
      Vrms[entry_count] = emonl.Vrms;
      Irms[entry_count] = emonl.Irms;
      entry_count++;
//      delay(500);
    } while(millis()-time_t <= SAMPLE_INTERVAL);
    Serial.print("Sampling complete with counts:");
    Serial.println(entry_count);
    enQueue(q,
              glitch_filtered_sum(realPower, entry_count),
              glitch_filtered_sum(apparentPower, entry_count),
              glitch_filtered_sum(powerFactor, entry_count),
              glitch_filtered_sum(Vrms, entry_count),
              glitch_filtered_sum(Irms, entry_count));

    if (WiFi.status() == WL_CONNECTED) {

      if (client.connected()) {
        publishMessage(read_realPower(q), read_apparentPower(q), read_powerFactor(q), read_Vrms(q), read_Irms(q));
        deQueue(q);
        /* code */
      } else{
        net.setCACert(AWS_CERT_CA);
        net.setCertificate(AWS_CERT_CRT);
        net.setPrivateKey(AWS_CERT_PRIVATE);
        client.begin(AWS_IOT_ENDPOINT, 8883, net);
        client.onMessage(messageHandler);
        time_t = millis();
        Serial.println("reconnecting to AWS");
        while (!client.connect(THINGNAME) && millis() - time_t <= 5000) {
          Serial.print(".");
          delay(100);
        }
      }
    } else {
//      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    client.loop();
  }
  delay(10000);
}
