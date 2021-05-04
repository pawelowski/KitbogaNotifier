# Kitboga Twitch Status

3D-printed sign that shows if the Kitboga streamer is online on Twitch

### TO DO:

[ ] save `access_token` to EEPROM memory

[ ] fixed error codes

## Description

---

Project uses an ESP32 to call the TwitchTV Helix API to check status of Kitboga's stream. The result is displayed on a 3D-printed sign with addressible RGBs and FastLED library.

NOTE: This is a small remix of a great design by makkuro [thing:749887](https://www.thingiverse.com/thing:749887).

![KiT_STL](https://raw.githubusercontent.com/pawelowski/KitbogaTwitchStatus/main/photos/KiT_SLT_screenshot.JPG)
![boga_STL](https://raw.githubusercontent.com/pawelowski/KitbogaTwitchStatus/main/photos/boga_STL_screenshot.JPG)

# Details

When my fav streamer Kitboga (https://www.twitch.tv/kitboga) comes up live I get notified with all the LEDs lighting up in the stream colour.

When he's offile, only 1 LED illuminates.

Also error states are displayed:

### Error Codes

- 1 red led = Unauthorized/Expired keys
- 2 red leds = Auth Issue
- 3 red leds = Internal Server Error
- ALL red = other unknown request failure

## Hardware BoM

- ESP32
- WS2811 addressible RGBs
- 3D-printed letters

## Libriaries and other necessary resources

- access to Twich Helix API https://dev.twitch.tv/docs/api/
- HTTPClient
- Arduino JSON
- FastLED https://www.arduino.cc/reference/en/libraries/fastled/

## ESP32 setup

![circuit_diagram](https://github.com/pawelowski/KitbogaTwitchStatus/blob/main/photos/circuit_diagram.JPG)

- diode D1 provents power being drawn from the ESP32
- capacitor C1 fixes [Timed out waiting for packet header](https://randomnerdtutorials.com/solved-failed-to-connect-to-esp32-timed-out-waiting-for-packet-header/)

## API setup hints

1. create a Twitch developer account to obtain access keys https://dev.twitch.tv/docs/api/

2. in Arduino IDE, fill `clientID` & `clientSecret` accordingly. Uncomment line #156 to print auth token in Serial Monitor.
   Upload code to the the μC.

NOTE: you could use programmes like Postman or Insomnia to generate the token yourself.

3. Auth token will be generated and printed in the Serial Monitor. Grab it and paste to `access_token` in Arduino IDE (it lasts for around 60 days, this will prevent creating a new one when you power cycle your device in early stages). Comment back the #156 line (optional), re-upload the code with a valid `auth_token`.

Follow erros in the Serial Monitor for debugging. Further lines from the code can be uncommented to print importand variables.

## Know issue

Some times, the Http request returns -1 which I can't figure out what it means or why it happens. When that occurs, the sign will display critical error state (all LEDs show red). Rebooting the ESP32 solves the issue. If you have any ideas why my Http response is -1, please let me know.
