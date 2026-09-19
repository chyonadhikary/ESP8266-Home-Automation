#pragma once
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config.h"
inline void defaults(CloudConfig &c){c.ssid="";c.wifiPassword="";c.deviceId="";c.secretKey="";c.thingId="";c.provisioned=false;}
inline bool loadConfig(CloudConfig &c){defaults(c);if(!LittleFS.begin())return false;if(!LittleFS.exists(CONFIG_FILE))return true;File f=LittleFS.open(CONFIG_FILE,"r");if(!f)return false;DynamicJsonDocument d(1024);DeserializationError e=deserializeJson(d,f);f.close();if(e||!d["schema"].is<int>()){defaults(c);return false;}c.ssid=String(d["ssid"]|"");c.wifiPassword=String(d["wifiPassword"]|"");c.deviceId=String(d["deviceId"]|"");c.secretKey=String(d["secretKey"]|"");c.thingId=String(d["thingId"]|"");c.provisioned=d["provisioned"]|(c.ssid.length()>0&&c.deviceId.length()>0&&c.secretKey.length()>0&&c.thingId.length()>0);return true;}
inline bool saveConfig(const CloudConfig &c){DynamicJsonDocument d(1024);d["schema"]=1;d["ssid"]=c.ssid;d["wifiPassword"]=c.wifiPassword;d["deviceId"]=c.deviceId;d["secretKey"]=c.secretKey;d["thingId"]=c.thingId;d["provisioned"]=c.provisioned;File f=LittleFS.open(CONFIG_FILE,"w");if(!f)return false;bool ok=serializeJson(d,f)>0;f.close();return ok;}
inline void eraseConfig(CloudConfig &c){defaults(c);LittleFS.remove(CONFIG_FILE);saveConfig(c);}
