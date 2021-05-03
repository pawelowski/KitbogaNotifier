# Kitboga Twitch Status

3D-printed sign that shows if the Kitboga streamer is online on Twitch

## Description

Project uses an ESP32 to call the TwitchTV Helix API to check status of Kitboga's stream. The result is displayed on a 3D-printed sign with addressible RGBs and FastLED library.

NOTE: This is a small remix of a great design by makkuro [thing:749887](https://www.thingiverse.com/thing:749887).

## Hardware BoM

- ESP32
- WS2811 addressible RGBs
- 3D-printed letters

## Libriaries and other necessary resources

- access to Twich Helix API https://dev.twitch.tv/docs/api/
- HTTPClient
- Arduino JSON
- FastLED https://www.arduino.cc/reference/en/libraries/fastled/

## Know issue

Some times, the Http request returns -1 which I can't figure out what it means or why it happens. When that occurs, the sign will display critical error state (all LEDs show red). Rebooting the ESP32 solves the issue. If you have any ideas why my Http response is -1, please let me know.
