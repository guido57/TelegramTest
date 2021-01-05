#include <WiFi.h>
#include <telegram.h>
#include <TaskScheduler.h>
#include "credentials.h"

// Wifi network station credentials
/* Create a file credentials.h and add:
#include <Arduino.h>
String WIFI_SSID = "the SSID of your WIFI";
String WIFI_PASSWORD = "the password of your WIFI";

*/
extern String WIFI_SSID; 
extern String WIFI_PASSWORD; 

// defined here because other Tasks need to use it
Scheduler myScheduler;

void setup()
{
  Serial.begin(115200);
  Serial.println();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // setup up the telegram communicator
  telegram_setup(&myScheduler);
  
}

void loop()
{
  myScheduler.execute();
  // don't put anything here
}