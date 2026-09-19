# GitHub update guide

Repository: `https://github.com/chyonadhikary/ESP8266-Home-Automation`

## Method A — GitHub website (সবচেয়ে সহজ)

1. GitHub repository খুলুন।
2. **Add file → Upload files** নির্বাচন করুন।
3. এই folder-এর সব file upload করুন:
   - `SmartHome_ArduinoCloud_Provisioned.ino`
   - `config.h`
   - `storage.h`
   - `README.md`
   - `README_OFFLINE_MODE.md`
   - `circuit.mmd`
   - `circuit.png`
   - `circuit_schematic.svg`
4. পুরোনো একই নামের file replace হবে; নতুন `README_OFFLINE_MODE.md` যোগ হবে।
5. নিচে commit message লিখুন:

   `Add password-protected OTA and automatic offline/online mode`

6. **Commit changes** চাপুন।

### গুরুত্বপূর্ণ folder rule

Arduino IDE-এর জন্য `.ino`, `config.h`, এবং `storage.h` একই folder-এ থাকতে হবে। GitHub root-এ সরাসরি রাখলে এই repository structure বজায় থাকবে:

```text
ESP8266-Home-Automation/
├── SmartHome_ArduinoCloud_Provisioned.ino
├── config.h
├── storage.h
├── README.md
├── README_OFFLINE_MODE.md
├── circuit.mmd
├── circuit.png
└── circuit_schematic.svg
```

## Method B — Git command line

প্রথমবার:

```bash
git clone https://github.com/chyonadhikary/ESP8266-Home-Automation.git
cd ESP8266-Home-Automation
```

এই package-এর উপরের আটটি project file repository folder-এ copy/replace করুন। তারপর:

```bash
git status
git add SmartHome_ArduinoCloud_Provisioned.ino config.h storage.h README.md README_OFFLINE_MODE.md circuit.mmd circuit.png circuit_schematic.svg
git commit -m "Add password-protected OTA and automatic offline/online mode"
git push origin main
```

GitHub login চাইলে নিজের GitHub authentication ব্যবহার করবেন। Password-এর বদলে GitHub-এর browser login, GitHub CLI, অথবা Personal Access Token ব্যবহার করতে হতে পারে।

## New firmware behavior

- Saved router Wi-Fi available: Online / Arduino Cloud mode.
- Router unavailable for 60 seconds: ESP8266 starts `SmartHome-XXXX` AP.
- Offline AP password: `SETUP_AP_PASSWORD` in `config.h`, default `12345678`.
- Offline dashboard: `http://192.168.4.1/`.
- Offline mode: local mobile control and physical buttons work; Cloud/Google Home do not.
- Offline mode retries the saved router every 30 seconds.
- Router returns: ESP8266 restarts and returns to Online / Cloud mode.
- Firmware version: `2.3.0-offline-online`.

## OTA upload after compiling

1. Arduino IDE-তে `SmartHome_ArduinoCloud_Provisioned.ino` খুলুন।
2. Board: **NodeMCU 1.0 (ESP-12E Module)**.
3. `config.h`-এ `OTA_PASSWORD` নিজের password দিন।
4. প্রথমবার USB upload করুন।
5. পরবর্তী build-এর জন্য **Sketch → Export Compiled Binary** নির্বাচন করুন।
6. তৈরি মূল `.ino.bin` file Web OTA page-এর **Firmware** field-এ দিন।
7. **FileSystem** field খালি রাখুন।
8. Update চাপুন এবং restart শেষ হওয়া পর্যন্ত power বন্ধ করবেন না।

## Security

- `OTA_PASSWORD` অবশ্যই default value থেকে বদলান।
- Port forwarding করে `/update` internet-এ প্রকাশ করবেন না।
- `storage.h` ও `config.h` একই sketch folder-এ রাখুন।
- 110/220 V relay wiring qualified electrician দিয়ে যাচাই করান।
