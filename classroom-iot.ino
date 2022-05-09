#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#include "RootManager.h"

/* Define the WiFi credentials */
#define WIFI_SSID "zero-2"
#define WIFI_PASSWORD "24071961"

/* Define the RTDB URL and database secret */
#define DATABASE_URL "classroom-iot-default-rtdb.asia-southeast1.firebasedatabase.app"
#define DATABASE_SECRET "MGKSPJ3xFGWyI0cHAO6leSWYbStCGi40HMgAhdp1"

RootManager *root_manager;

void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  Firebase.reconnectWiFi(true);

  Firebase.begin(DATABASE_URL, DATABASE_SECRET);

  root_manager = new RootManager();
}

void loop()
{
  root_manager->run();
}
