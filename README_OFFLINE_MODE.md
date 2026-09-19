# Offline / Online Automatic Mode

This firmware keeps the existing relay pins, push buttons, Arduino Cloud, local dashboard, LittleFS credentials, and password-protected Web OTA update.

## Offline operation

If the saved router Wi-Fi is unavailable for 60 seconds, the ESP8266 starts its own access point:

- Wi-Fi name: `SmartHome-XXXX`
- Password: the value of `SETUP_AP_PASSWORD` in `config.h` (default `12345678`)
- Dashboard: `http://192.168.4.1/`

The four relays can then be controlled locally from a phone connected directly to this Wi-Fi. Physical push buttons continue to work. Arduino Cloud, Google Home, and remote Internet control are unavailable without Internet.

If there are no saved credentials on first boot, the same offline dashboard starts immediately. Open `/setup` later if you want to configure router and Arduino Cloud credentials.

## Automatic return online

While in Offline AP mode, the firmware retries the saved router Wi-Fi every 30 seconds. When the router becomes available, the ESP8266 restarts and returns to normal Online / Cloud mode automatically.

## OTA update

Build this sketch for **NodeMCU 1.0 (ESP-12E Module)**, export the compiled `.bin`, then upload it on the existing authenticated `/update` page. Leave the FileSystem field empty.
