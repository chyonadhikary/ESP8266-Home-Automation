#pragma once
#include <Arduino.h>
#define FIRMWARE_VERSION "2.1.0"
#define CONFIG_FILE "/cloud_config.json"
#define SETUP_AP_PASSWORD "12345678"
#define WIFI_TIMEOUT_MS 60000UL
#define MAX_FIELD 120
static const uint8_t RELAY_PINS[4] = {D1, D2, D5, D6};
static const uint8_t BUTTON_PINS[4] = {D3, D4, D7, D8};
struct CloudConfig { String ssid, wifiPassword, deviceId, secretKey, thingId; bool provisioned; };
inline String apName() { char b[12]; snprintf(b,sizeof(b),"%04X",(unsigned)(ESP.getChipId()&0xFFFF)); return String("SmartHome-")+b; }
