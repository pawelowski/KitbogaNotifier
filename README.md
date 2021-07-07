\> Work In Progress <

Still documenting the repo

# Kitboga Twitch Status

Notification sign for Kitboga status on Twitch.

### TO DO:

[ ] save `access_token` to EEPROM memory

[ ] fix error codes

## Description

Project uses an ESP32 to call the TwitchTV Helix API to check status of Kitboga's stream. The result is displayed on a 3D-printed sign with addressible RGBs and FastLED library.

NOTE: The STLs are a small remix of a great design by makkuro [thing:749887](https://www.thingiverse.com/thing:749887).

# Details

When my fav streamer Kitboga (https://www.twitch.tv/kitboga) comes up live I get notified with all the LEDs lighting up in the stream's colour.

![Kit-online](https://github.com/pawelowski/KitbogaTwitchStatus/blob/main/photos/KIT_BOGA_online-part.jpg)

When he's offline, only 1 LED over _i_ illuminates.

![Kit-offline](https://github.com/pawelowski/KitbogaTwitchStatus/blob/main/photos/KIT_offline.jpg)

Also error states are displayed.

### Error Codes

- :yellow_circle: = Unauthorized/Expired keys
- :orange_circle: = Auth Issue
- :red_circle: = Internal Server Error
- :cursing_face: ALL red = other unknown request failure (including response `-1`)

## Hardware BoM

- ESP32
- WS2811 addressible RGBs
- 3D-printed letters (printed separately)
- resistor, capacitor, diode, power connector, wires

## Libriaries and other necessary resources

- access to Twich Helix API https://dev.twitch.tv/docs/api/
- HTTPClient https://www.arduino.cc/reference/en/libraries/httpclient/
- Arduino JSON https://www.arduino.cc/reference/en/libraries/arduino_json/
- FastLED https://www.arduino.cc/reference/en/libraries/fastled/

## ESP32 setup

![circuit_diagram](https://github.com/pawelowski/KitbogaTwitchStatus/blob/main/photos/circuit_diagram.JPG)

- diode D1 provents power being drawn from the ESP32
- capacitor C1 fixes [Timed out waiting for packet header](https://randomnerdtutorials.com/solved-failed-to-connect-to-esp32-timed-out-waiting-for-packet-header/)

## API setup

1. created a Twitch developer account to obtain access keys https://dev.twitch.tv/docs/api/

2. in Arduino IDE, filled `clientID` & `clientSecret` accordingly. Line #156 to prints auth token in Serial Monitor.
   Uploaded code to the the μC.

NOTE: Also used programes clients like Postman or Insomnia to preview the token myself.

3. Auth token was generated from the API and printed in the Serial Monitor. Grabed it and pasted to `access_token` in Arduino IDE (it lasts for around 60 days, this prevents creating a new one when I often power cycle the device in early stages). Commented back the #156 line, re-uploaded the code with a valid `auth_token`.

Erros are shown in the Serial Monitor for debugging. Further lines from the code can be uncommented to print important variables.

## STL and printing details

![KiT_STL](https://raw.githubusercontent.com/pawelowski/KitbogaTwitchStatus/main/photos/KiT_SLT_screenshot.JPG?v4&s=1260px)
![boga_STL](https://raw.githubusercontent.com/pawelowski/KitbogaTwitchStatus/main/photos/boga_STL_screenshot.JPG)

- nothing special here, regular 0.2mm nozzle and low infil 15%

# Know issue

The Twich API requires HTTPS requests. However, I only managed to make this work with HTTP via HTTPClient. This, once in a while, causes the ESP32 to fail to call the API - generates http response code `-1`. A power cycle or reboot of the ESP seems to mostly fix it.. this is horrible and annoying so if you want to help and develop proper HTTPS calls (maybe with [WiFiClientSecure](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiClientSecure) lib) then plz get involved <3

# More photos

![](https://github.com/pawelowski/KitbogaTwitchStatus/blob/main/photos/STL_LED_inside.jpg)
