#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "config.h"
#include "storage.h"

ESP8266WebServer web(80);
DNSServer dns;
CloudConfig cfg;
WiFiConnectionHandler *cloudConnection = nullptr;

bool setupMode = false;
bool cloudStarted = false;
bool rebootPending = false;
unsigned long cloudStartedAt = 0;
unsigned long rebootAt = 0;

bool mainLight = false;
bool ledLight = false;
bool fan = false;
bool socket = false;

bool rawButton[4] = {HIGH, HIGH, HIGH, HIGH};
bool stableButton[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long changedAt[4] = {0, 0, 0, 0};

String esc(String value) {
  value.replace("&", "&amp;");
  value.replace("<", "&lt;");
  value.replace(">", "&gt;");
  value.replace("\"", "&quot;");
  return value;
}

String pageStart(const String &title) {
  String h = F("<!doctype html><html><head>");
  h += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  h += F("<title>");
  h += esc(title);
  h += F("</title><style>");
  h += F("body{font-family:Arial,sans-serif;background:#f1f4f9;color:#172033;margin:0}");
  h += F("header{background:#152342;color:white;padding:22px}");
  h += F("main{max-width:720px;margin:auto;padding:16px}");
  h += F(".card{background:white;padding:18px;border-radius:15px;margin:12px 0;box-shadow:0 4px 15px #0001}");
  h += F("h1{margin:0;font-size:1.35rem}h2{margin-top:0;font-size:1.05rem;color:#465777}");
  h += F(".relay{display:flex;align-items:center;justify-content:space-between;border-top:1px solid #e8ecf4;padding:14px 0}");
  h += F(".state{font-weight:bold;color:#68758f}.on{color:#14935f}");
  h += F("label{font-weight:bold;display:block;margin:12px 0 5px}");
  h += F("input{width:100%;box-sizing:border-box;padding:12px;border:1px solid #ccd3e0;border-radius:8px;font-size:1rem}");
  h += F("button{background:#315bef;color:white;border:0;padding:11px 15px;border-radius:8px;font-weight:bold;margin:4px 3px 0 0}");
  h += F("button.off{background:#e7ebf5;color:#253453}.danger{background:#b42335}");
  h += F(".muted{color:#68758f;font-size:.9rem}.notice{background:#eaf1ff;padding:12px;border-radius:8px}");
  h += F(".warn{background:#fff4d5;padding:12px;border-radius:8px}");
  h += F("</style></head><body><header><h1>SMART HOME</h1>");
  h += F("<div>ESP8266 Home Controller</div></header><main>");
  return h;
}

String pageEnd() {
  return F("</main></body></html>");
}

String setupPage(const String &message = "") {
  String h = pageStart("Device Setup");
  if (message.length()) {
    h += F("<div class='card warn'>");
    h += esc(message);
    h += F("</div>");
  }
  h += F("<div class='card'><h2>First-time setup</h2>");
  h += F("<p>Connect this controller to the customer's Wi-Fi and Arduino Cloud.</p>");
  h += F("<form method='post' action='/save'>");
  h += F("<label>Home Wi-Fi SSID</label><input name='ssid' required value='");
  h += esc(cfg.ssid);
  h += F("'><label>Home Wi-Fi password</label><input name='wifi' type='password' required>");
  h += F("<label>Arduino Cloud Device ID</label><input name='device' required value='");
  h += esc(cfg.deviceId);
  h += F("'><label>Arduino Cloud Secret Device Key</label><input name='secret' type='password' required>");
  h += F("<label>Arduino Cloud Thing ID</label><input name='thing' required value='");
  h += esc(cfg.thingId);
  h += F("'><button>Save &amp; Connect</button></form>");
  h += F("<p class='muted'>Setup Wi-Fi: <b>");
  h += apName();
  h += F("</b> · Password: <b>");
  h += SETUP_AP_PASSWORD;
  h += F("</b></p></div>");
  h += pageEnd();
  return h;
}

String dashboard() {
  String h = pageStart("Smart Home Dashboard");
  String wifiText = WiFi.status() == WL_CONNECTED ? String("Connected") : String("Disconnected");
  String cloudText = ArduinoCloud.connected() ? String("Connected") : String("Connecting / Offline");
  h += F("<div class='card'><h2>Device status</h2><div class='notice'>Wi-Fi: ");
  h += wifiText;
  h += F("<br>Local IP: <b>");
  h += WiFi.localIP().toString();
  h += F("</b><br>Arduino Cloud: ");
  h += cloudText;
  h += F("<br>Firmware: ");
  h += FIRMWARE_VERSION;
  h += F("</div>");

  const char *names[4] = {"Main Light", "LED Light", "Fan", "Socket"};
  bool states[4] = {mainLight, ledLight, fan, socket};
  for (uint8_t i = 0; i < 4; i++) {
    h += F("<div class='relay'><div><b>");
    h += names[i];
    h += F("</b><div class='state ");
    if (states[i]) h += F("on");
    h += F("'>");
    h += states[i] ? F("ON") : F("OFF");
    h += F("</div></div><div><a href='/relay?ch=");
    h += String(i);
    h += F("&v=1'><button>ON</button></a><a href='/relay?ch=");
    h += String(i);
    h += F("&v=0'><button class='off'>OFF</button></a></div></div>");
  }
  h += F("</div><div class='card'><h2>Arduino Cloud / Google Home</h2>");
  h += F("<p class='muted'>Use Arduino Cloud Dashboard or IoT Remote. Google Home variables must be Smart Home Switch types.</p>");
  h += F("<a href='/settings'><button>Settings</button></a>");
  h += F("<a href='/setup'><button class='off'>Reconfigure</button></a></div>");
  h += pageEnd();
  return h;
}

void safeRelays() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(RELAY_PINS[i], HIGH);
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], HIGH);
  }
}

void applyRelays() {
  digitalWrite(RELAY_PINS[0], mainLight ? LOW : HIGH);
  digitalWrite(RELAY_PINS[1], ledLight ? LOW : HIGH);
  digitalWrite(RELAY_PINS[2], fan ? LOW : HIGH);
  digitalWrite(RELAY_PINS[3], socket ? LOW : HIGH);
}

void onMainLightChange() { digitalWrite(RELAY_PINS[0], mainLight ? LOW : HIGH); }
void onLedLightChange()  { digitalWrite(RELAY_PINS[1], ledLight ? LOW : HIGH); }
void onFanChange()       { digitalWrite(RELAY_PINS[2], fan ? LOW : HIGH); }
void onSocketChange()    { digitalWrite(RELAY_PINS[3], socket ? LOW : HIGH); }

void toggleFromButton(uint8_t index) {
  if (index == 0) { mainLight = !mainLight; onMainLightChange(); }
  else if (index == 1) { ledLight = !ledLight; onLedLightChange(); }
  else if (index == 2) { fan = !fan; onFanChange(); }
  else { socket = !socket; onSocketChange(); }
}

void readButtons() {
  unsigned long now = millis();
  for (uint8_t i = 0; i < 4; i++) {
    bool reading = digitalRead(BUTTON_PINS[i]);
    if (reading != rawButton[i]) {
      rawButton[i] = reading;
      changedAt[i] = now;
    }
    if ((now - changedAt[i]) >= 40 && stableButton[i] != reading) {
      bool previous = stableButton[i];
      stableButton[i] = reading;
      if (previous == HIGH && reading == LOW) toggleFromButton(i);
    }
  }
}

void startSetupAP() {
  setupMode = true;
  cloudStarted = false;
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName().c_str(), SETUP_AP_PASSWORD);
  dns.start(53, "*", WiFi.softAPIP());
}

void startCloud() {
  setupMode = false;
  cloudStartedAt = millis();
  cloudConnection = new WiFiConnectionHandler(cfg.ssid.c_str(), cfg.wifiPassword.c_str());
  ArduinoCloud.setThingId(cfg.thingId);
  ArduinoCloud.setBoardId(cfg.deviceId);
  ArduinoCloud.setSecretDeviceKey(cfg.secretKey);
  ArduinoCloud.addProperty(mainLight, READWRITE, ON_CHANGE, onMainLightChange);
  ArduinoCloud.addProperty(ledLight, READWRITE, ON_CHANGE, onLedLightChange);
  ArduinoCloud.addProperty(fan, READWRITE, ON_CHANGE, onFanChange);
  ArduinoCloud.addProperty(socket, READWRITE, ON_CHANGE, onSocketChange);
  ArduinoCloud.begin(*cloudConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  cloudStarted = true;
}

void redirectHome() {
  web.sendHeader("Location", "/");
  web.send(303, "text/plain", "");
}

void setupRoutes() {
  web.onNotFound([]() {
    web.sendHeader("Location", setupMode ? "/setup" : "/");
    web.send(302, "text/plain", "");
  });
  web.on("/generate_204", HTTP_GET, []() { web.send(200, "text/html", setupMode ? setupPage() : dashboard()); });
  web.on("/hotspot-detect.html", HTTP_GET, []() { web.send(200, "text/html", setupMode ? setupPage() : dashboard()); });
  web.on("/", HTTP_GET, []() { web.send(200, "text/html", setupMode ? setupPage() : dashboard()); });
  web.on("/setup", HTTP_GET, []() { web.send(200, "text/html", setupPage()); });

  web.on("/save", HTTP_POST, []() {
    String s = web.arg("ssid");
    String w = web.arg("wifi");
    String d = web.arg("device");
    String k = web.arg("secret");
    String t = web.arg("thing");
    if (!s.length() || !w.length() || !d.length() || !k.length() || !t.length()) {
      web.send(400, "text/html", setupPage("All fields are required."));
      return;
    }
    cfg.ssid = s; cfg.wifiPassword = w; cfg.deviceId = d;
    cfg.secretKey = k; cfg.thingId = t; cfg.provisioned = true;
    saveConfig(cfg);
    web.send(200, "text/html", pageStart("Saved") + F("<div class='card'><h2>Saved</h2><p>Device is restarting and will connect to Wi-Fi and Arduino Cloud.</p></div>") + pageEnd());
    rebootPending = true;
    rebootAt = millis() + 1500;
  });

  web.on("/relay", HTTP_GET, []() {
    uint8_t index = (uint8_t)web.arg("ch").toInt();
    bool value = web.arg("v") == "1";
    if (index == 0) { mainLight = value; onMainLightChange(); }
    else if (index == 1) { ledLight = value; onLedLightChange(); }
    else if (index == 2) { fan = value; onFanChange(); }
    else if (index == 3) { socket = value; onSocketChange(); }
    redirectHome();
  });

  web.on("/settings", HTTP_GET, []() {
    String h = pageStart("Settings");
    h += F("<div class='card'><h2>Saved configuration</h2><p>SSID: ");
    h += esc(cfg.ssid);
    h += F("</p><p>Device ID: ");
    h += esc(cfg.deviceId);
    h += F("<br>Thing ID: ");
    h += esc(cfg.thingId);
    h += F("<br>Secret key: ********</p><p class='muted'>Use Reconfigure to replace Wi-Fi or Cloud credentials.</p>");
    h += F("<form method='post' action='/reset' onsubmit=\"return confirm('Erase all saved configuration and restart?')\"><button class='danger'>Factory Reset</button></form></div>");
    h += pageEnd();
    web.send(200, "text/html", h);
  });

  web.on("/reset", HTTP_POST, []() {
    eraseConfig(cfg);
    web.send(200, "text/html", pageStart("Reset") + F("<div class='card'><h2>Reset complete</h2><p>Reconnect to the setup Wi-Fi and configure the device again.</p></div>") + pageEnd());
    rebootPending = true;
    rebootAt = millis() + 1500;
  });
  web.begin();
}

void setup() {
  safeRelays();
  for (uint8_t i = 0; i < 4; i++) pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  Serial.begin(115200);
  delay(300);
  loadConfig(cfg);
  mainLight = false; ledLight = false; fan = false; socket = false;
  applyRelays();
  setupRoutes();
  if (cfg.provisioned) startCloud(); else startSetupAP();
}

void loop() {
  if (setupMode) dns.processNextRequest();
  web.handleClient();
  if (!setupMode && cloudStarted) {
    ArduinoCloud.update();
    if (WiFi.status() != WL_CONNECTED && millis() - cloudStartedAt > WIFI_TIMEOUT_MS) startSetupAP();
  }
  readButtons();
  if (rebootPending && millis() > rebootAt) ESP.restart();
  yield();
}
