# SmartHome ESP8266 — Provisioning, Local Dashboard, Arduino Cloud ও Google Home

এই firmware-এর সঠিক flow হলো: প্রথম boot-এ setup AP, setup web page, credentials save, reboot; এরপর router-এর IP address-এ local dashboard এবং একই সঙ্গে Arduino Cloud/IoT Remote/Google Home control।

## ১. প্রথমবার customer setup

1. Device-এ power দিন।
2. ফোনে `SmartHome-XXXX` Wi-Fi connect করুন। Password: `12345678`।
3. Captive page না খুললে `http://192.168.4.1/` খুলুন।
4. Home Wi-Fi SSID ও password দিন।
5. Arduino Cloud-এর Device ID, Secret Device Key এবং Thing ID দিন।
6. **Save & Connect** চাপুন। Device restart হবে।
7. ফোনটিকে আবার home router-এর Wi-Fi-তে connect করুন।
8. Router-এর দেওয়া ESP8266 IP browser-এ খুলুন, যেমন `http://192.168.1.50/`। এই firmware-এ এখন local dashboard আছে।

## ২. Local IP কীভাবে জানবেন

Router admin page-এর connected devices/DHCP clients list-এ ESP8266 খুঁজুন। Arduino Cloud-এ connected হলে Serial Monitor-এ Wi-Fi status/debug information-ও দেখা যাবে। প্রথম firmware version-এ mDNS নয়, router IP ব্যবহার করাই নির্ভরযোগ্য।

Local dashboard-এ দেখা যাবে:

- Wi-Fi connected/disconnected
- Local IP address
- Arduino Cloud connected/connecting
- চার relay-এর বর্তমান ON/OFF state
- Local ON/OFF buttons
- Settings ও Factory Reset

Local dashboard থেকে relay বদলালে সংশ্লিষ্ট Cloud variable-ও বদলাবে, কারণ dashboard এবং physical buttons একই in-memory Cloud property ব্যবহার করে।

## ৩. Arduino Cloud Thing configuration

Arduino Cloud-এ Thing খুলে ESP8266 device-টি Associated Device হিসেবে যুক্ত করুন। নিচের চারটি variable তৈরি করুন:

| নাম | Type | Permission | Update policy |
|---|---|---|---|
| `mainLight` | **Smart Home → Switch** | Read & Write | On change |
| `ledLight` | **Smart Home → Switch** | Read & Write | On change |
| `fan` | **Smart Home → Switch** | Read & Write | On change |
| `socket` | **Smart Home → Switch** | Read & Write | On change |

**Google Home-এর জন্য Type হিসেবে সাধারণ Boolean নয়, `Smart Home → Switch` নির্বাচন করুন।** Arduino-এর official Google Home guide এই ধরনের compatible variable ব্যবহার করে।

## ৪. Arduino Cloud Dashboard এবং IoT Remote

Dashboard-এ চারটি Switch widget যোগ করুন এবং link করুন:

- Main Light → `mainLight`
- LED Light → `ledLight`
- Fan → `fan`
- Socket → `socket`

Arduino IoT Remote app-এ একই Arduino account দিয়ে login করলে এই dashboard দেখা যাবে।

## ৫. Google Home সংযোগ

1. Arduino Cloud Thing-এর চারটি variable `Smart Home → Switch` হিসেবে save করুন।
2. Google Home app খুলুন।
3. **Devices → Add → Works with Google Home** নির্বাচন করুন।
4. Search করুন `Arduino`।
5. Arduino account link করুন এবং অনুমতি দিন।
6. পাওয়া চারটি device Google Home-এর room-এ assign করুন।
7. Device নাম সহজ রাখুন, যেমন `Main Light`, `Fan`, `Socket`।

তারপর বলতে পারবেন:

```text
Hey Google, turn on Main Light
Hey Google, turn off Fan
```

## ৬. Physical buttons ও wiring

Relay inputs:

- D1/GPIO5 → Relay IN1 → Main Light
- D2/GPIO4 → Relay IN2 → LED Light
- D5/GPIO14 → Relay IN3 → Fan
- D6/GPIO12 → Relay IN4 → Socket

Momentary buttons:

- D3/GPIO0 → S1, অন্য প্রান্ত GND
- D4/GPIO2 → S2, অন্য প্রান্ত GND
- D7/GPIO13 → S3, অন্য প্রান্ত GND
- D8/GPIO15 → S4, অন্য প্রান্ত GND

`INPUT_PULLUP` ব্যবহৃত হয়েছে: button ছেড়ে দিলে HIGH, চাপলে LOW। D3/D4/D8 boot-sensitive pin; reset বা power-on-এর সময় button চেপে রাখবেন না। সম্পূর্ণ pin-to-wire electrical schematic হিসেবে `circuit_schematic.svg` এবং `circuit.png` দেখুন।

## ৭. Reconnect behavior

Power ফিরে এলে firmware প্রথমে সব relay OFF করে safety state-এ boot করবে, saved Wi-Fi দিয়ে Cloud connection শুরু করবে, তারপর synchronized Cloud value অনুযায়ী relay update হবে। Wi-Fi disconnect হলে Cloud library reconnect করার চেষ্টা করবে। দীর্ঘ সময় Wi-Fi না এলে setup AP আবার চালু হবে যাতে customer configuration ঠিক করতে পারেন।

## ৮. Factory reset

Local IP dashboard → **Settings → Factory Reset**। এতে Wi-Fi ও Cloud credentials মুছে যাবে এবং device আবার `SmartHome-XXXX` setup AP mode-এ যাবে।

## ৯. Upload-এর জন্য libraries

Arduino IDE Library Manager-এ install করুন:

- ArduinoIoTCloud
- Arduino_ConnectionHandler
- ArduinoJson

Board: **NodeMCU 1.0 (ESP-12E Module)**।

## নিরাপত্তা

110/220 V wiring breadboard-এ করবেন না। Relay rating, fuse, isolation, enclosure, earthing, creepage/clearance এবং স্থানীয় electrical code qualified electrician দিয়ে যাচাই করুন।

## Official references

- Google Home: https://docs.arduino.cc/arduino-cloud/guides/google-home/
- Cloud Variables: https://docs.arduino.cc/arduino-cloud/cloud-interface/variables/
- IoT Remote: https://docs.arduino.cc/arduino-cloud/iot-remote-app/getting-started


## ১০. Local Web Firmware Update (OTA)

এই version-এ local browser থেকে firmware update করা যায়। Arduino IDE কেবল প্রথমবার firmware install এবং নতুন `.bin` file তৈরি করার জন্য লাগবে। পরবর্তী update-এর সময় USB দিয়ে board-এ upload করার প্রয়োজন নেই।

### প্রথমবার প্রস্তুতি

1. `config.h` খুলে `OTA_PASSWORD`-এর default password পরিবর্তন করুন।
2. Arduino IDE-তে Board হিসেবে **NodeMCU 1.0 (ESP-12E Module)** নির্বাচন করুন।
3. প্রথমবার USB দিয়ে sketch upload করুন।
4. Arduino IDE-এর **Sketch → Export Compiled Binary** নির্বাচন করুন।
5. তৈরি হওয়া `.bin` file নিরাপদে রাখুন।

### Browser থেকে update

1. ESP8266 এবং ফোন/কম্পিউটারকে একই Wi-Fi network-এ রাখুন।
2. Dashboard খুলুন, যেমন `http://192.168.1.50/`।
3. **Firmware Update** button চাপুন, অথবা সরাসরি `http://192.168.1.50/update` খুলুন।
4. Username হিসেবে `admin` এবং `config.h`-এ সেট করা OTA password দিন।
5. NodeMCU 1.0-এর জন্য তৈরি `.bin` file নির্বাচন করে upload করুন।
6. Upload শেষ হলে board নিজে restart হবে। Wi-Fi ও Arduino Cloud credentials LittleFS-এ আগের মতো থাকবে।

### OTA-এর গুরুত্বপূর্ণ নিয়ম

- `OTA_PASSWORD` অবশ্যই প্রথম upload-এর আগে পরিবর্তন করুন।
- শুধু এই board এবং একই project-এর জন্য তৈরি `.bin` upload করুন।
- OTA update-এর সময় relay সাময়িকভাবে restart হয়ে safety state-এ OFF হবে।
- Update চলাকালে power বা Wi-Fi বন্ধ করবেন না।
- Router-এ port forwarding করে update page internet-এ প্রকাশ করবেন না।
- OTA update page local network-এর জন্য; এটি cloud থেকে firmware update নয়।
- `Erase Flash: All Flash Contents` ব্যবহার করলে saved Wi-Fi/Cloud configuration মুছে যেতে পারে।
- নতুন firmware release হলে `FIRMWARE_VERSION` বাড়িয়ে দিন, যেমন `2.2.0-ota` থেকে `2.3.0`।

### বর্তমান OTA credentials

| Setting | Value |
|---|---|
| Update URL | `http://<ESP8266-IP>/update` |
| Username | `admin` |
| Password | `config.h`-এ নির্ধারিত `OTA_PASSWORD` |

এই local OTA ব্যবস্থা password-protected হলেও HTTP encryption ব্যবহার করে না। তাই এটি কেবল trusted local Wi-Fi-তে ব্যবহার করুন এবং update password অন্যের সঙ্গে শেয়ার করবেন না।
