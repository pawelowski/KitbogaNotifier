/*

  3D-printed sign that shows if the Kitboga streamer is online on Twitch

  Credit for STL design goes to makkuro https://www.thingiverse.com/thing:749887

  Other rescourses used:
  https://randomnerdtutorials.com/esp32-http-get-post-arduino/
  https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectTheatreChase

*/
const String VER = "1.20.1";

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiMulti.h>
//#include <WiFiClientSecure.h>
#include "config.h"
#include "root_ca.h"
#include <Arduino_JSON.h>
#include "FastLED.h"
#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

// LED setup variables
#define DATA_PIN 2
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 29     //total number of used LEDs=60, KIT=29
#define INDICAT_LED 18  // the dot over letter i
//#define INDICAT_LED 13  // the dot over letter i
#define BRIGHTNESS 100

//Colour definitions
CRGB boga_c = CRGB(99, 6, 171);
CRGB red = CRGB(201, 0, 0);
CRGB orange = CRGB(255, 85, 5);
CRGB green = CRGB(26, 110, 3);
CRGB teal = CRGB(10, 170, 173);
CRGB blue = CRGB(25, 8, 207);
CRGB yellow = CRGB(255, 255, 200);

// =[ WiFi variables ]=
const char* ssid = "Polska_Pany";
const char* password = "PolakiCebulaki6";

// =[ Twich Helix API variables ]=
//URLs
const char* validateOAuthURL = "https://id.twitch.tv/oauth2/validate";
const char* searchStreamerURL = "https://api.twitch.tv/helix/streams?user_login=kitboga";
const char* searchChannelURL = "https://api.twitch.tv/helix/search/channels?query=kitboga";
//const char* searchStreamerURL = "https://api.twitch.tv/helix/streams?user_login=cohhcarnage";

String authURL = "https://id.twitch.tv/oauth2/token?client_id=" + clientID + "&client_secret=" + clientSecret + "&grant_type=client_credentials";
int streamersID = 32787655;  //kit
String broadcast_login = "kitboga";
//String broadcast_login = "cohhcarnage";
//int streamersID = 26610234; //cohh

// =[ Other variables ]=
unsigned long lastTime = 0;      //used for timer
unsigned long timerDelay = 120;  //seconds
bool hasBeenOnline = false;      //used to run the trail effect only for the 1st time after checking if online
bool firstBoot = true;           //used to run the checkStatus() straight after boot to omit timer once

CRGB leds[NUM_LEDS];
WiFiClientSecure client;
WiFiMulti wifiMulti;

//client.setCACert(test_root_ca);

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.printf(">> FW Version: %s\n", VER);
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  poweredOn();

  wifiMulti.addAP(ssid1, password1);
  wifiMulti.addAP(ssid2, password2);

  //WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (firstBoot) {
    Serial.println("First time check after boot...");
    checkStatus();
    firstBoot = false;
  }

  //Send an HTTP POST request every 'timerDelay' value
  if ((millis() - lastTime) > timerDelay * 1000) {
    if (wifiMulti.run() == WL_CONNECTED) {

      // if validation successful, check if streamer is live,
      // if not, then if re-run is shown,
      // otherwise offline
      checkStatus();
      // otherwise, keep displaying the indication state based on the http request result

      //auth(); //for debugging: check your auth separtely
    } else {
      Serial.print("Wifi Disconnected? Status code :");
      Serial.println(wifiMulti.run());
      errorState(blue);
    }
    lastTime = millis();
  }
}

//=[ Functions ]=
void checkStatus() {
  if (validate()) {
    if (searchStream()) {
      if (!hasBeenOnline) introTrail();
      kitOnline();
    } else if (searchChannel()) {
      kitRerun();
    } else {
      kitOffline();
    }
  }
}
bool validate() {
  //Checks if access token is valid before making a request
  Serial.print("Validating... ");
  //Serial.printf("Access Token: %s\n", access_token);
  String rawResponse = httpGETRequest(validateOAuthURL, "Authorization", access_token, "", "");
  //Serial.println("HERE" + parseJson(rawResponse));  //for debugging
  int validTime = parseJson(rawResponse)["expires_in"];
  bool valid;
  if (validTime > 0) {
    valid = true;
  } else valid = false;
  Serial.printf(" -> Keys valid for: %d sec \n", validTime);
  delay(500);
  return valid;
}

void auth() {
  //Request a new access token to make a call to the API's endpoints
  Serial.print("Authing... ");
  HTTPClient http;
  http.begin(authURL, root_ca);

  int httpResponseCode = http.POST("");

  String payload = "{}";

  // ALL GOOD
  if (httpResponseCode == 200) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();

    JSONVar twitchAuthResponse = parseJson(payload);

    //extract access token from the Twich API response
    //Serial.println(twitchAuthResponse); // for debugging
    access_token = JSON.stringify(twitchAuthResponse["access_token"]);
    //Serial.print("Raw Access Token: ");Serial.println(access_token); // for debugging

    //I couldn't figure out how to extract characters from inside of " ", so I just chop them off both ends.
    // Example: access_token = "a1b2c3d4e5f6g7h8i9j1k2l3m4n5o6";

    access_token.remove(0, 1);  //remove 1st ' " ' (character 0)
    //Serial.printf("Access token lenght: %d\n", access_token.length());
    access_token.remove(access_token.length() - 1, 1);  //remove last ' " ' (character lenght - 1)
    //Serial.print("Access Token: ");Serial.println(access_token); // for debugging
  } else {
    Serial.print("Auth Issue \n");
    errorState(orange);
  }
  http.end();
}

bool searchStream() {
  bool isLive;
  //Look up streamers based on their user_login (can do multiple at a time). See 'searchStreamerURL' variable
  Serial.print("Looking for a live stream... ");
  JSONVar twitchStreamResponse = JSON.parse(httpGETRequest(searchStreamerURL, "Authorization", access_token, "Client-Id", clientID));
  if (JSON.typeof(twitchStreamResponse) == "undefined") {
    Serial.println("Parsing input failed!");
  }

  if ((int(twitchStreamResponse["data"].length()) > 0) && (atoi(twitchStreamResponse["data"][0]["user_id"]) == streamersID) && (JSON.stringify(twitchStreamResponse["data"][0]["type"]) == (const char*)("\"live\""))) {
    isLive = true;
  } else {
    isLive = false;
    Serial.print(" -> [ No Live Stream ]\n");
  }
  return isLive;
}

bool searchChannel() {
  bool reRunStatus;
  //Search all Twitch channels based on a term. See 'searchChannelURL' variable
  Serial.print("Checking channel for a re-run... ");
  JSONVar twitchStreamResponse = JSON.parse(httpGETRequest(searchChannelURL, "Authorization", access_token, "Client-Id", clientID));
  if (JSON.typeof(twitchStreamResponse) == "undefined") {
    Serial.println("Parsing input failed!");
  }

  //int resultLength = twitchStreamResponse["data"].length(); //for debugging
  //Serial.printf("Result length: %d\n", resultLength);

  if ((int(twitchStreamResponse["data"].length()) > 0) && (JSON.stringify(twitchStreamResponse["data"][0]["broadcaster_login"]) == (const char*)("\"kitboga\""))) {
    if (JSON.stringify(twitchStreamResponse["data"][0]["is_live"]) == (const char*)("true")) {
      reRunStatus = true;
    } else {
      reRunStatus = false;
      Serial.print(" -> [ No Re-runs ]\n");
    }
  } else {
    Serial.println("Can't find the channel");
  }
  return reRunStatus;
}

JSONVar parseJson(String _stringResult) {
  JSONVar twitchResponse = JSON.parse(_stringResult);
  if (JSON.typeof(twitchResponse) == "undefined") {
    Serial.println("Parsing input failed!");
    //    return;
  }
  return twitchResponse;
}

String httpGETRequest(const char* reqPath, String _auth_h, String _auth_v, String _c_id_h, String _c_id_v) {

  HTTPClient http;
  http.begin(reqPath, root_ca);

  http.addHeader("Content-Type", "application/json");
  //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader(_c_id_h, _c_id_v);
  String auth_param = "Bearer " + _auth_v;
  http.addHeader(_auth_h, auth_param, false, false);

  int httpResponseCode = http.GET();

  String payload = "{}";

  // ALL GOOD.
  if (httpResponseCode == 200) {
    Serial.printf("{ HTTP Response code: %d }", httpResponseCode);
    payload = http.getString();

    // Unauthorized - expired keys, so re-auth.
  } else if (httpResponseCode == 401) {
    errorState(teal);
    Serial.printf("Error code: %d\n", httpResponseCode);
    Serial.println("Unauthorized. Expired keys.");
    auth();
    payload = httpGETRequest(validateOAuthURL, "Authorization", access_token, "", "");

    //Internal Server Error.
  } else if (httpResponseCode == 500) {
    errorState(red);
    Serial.printf("Error code: %d\n", httpResponseCode);
    Serial.println("Internal Server Error");
    timerDelay = 15 * 60;  // try in 15min
    Serial.printf("Will try next in %d sec\n", timerDelay);

    // Bad request, expired root CA
  } else if (httpResponseCode == 400) {
    errorState(yellow);
    Serial.printf("Error code: %d\n", httpResponseCode);
    Serial.println("Bad request... Need a new root ca");

    //Invalid creds
  } else if (httpResponseCode == 403) {
    Serial.printf("Error code: %d\n", httpResponseCode);
    Serial.println("Invalid creditentials");    
    ayeNoDot();

    //-1 error
  } else if (httpResponseCode < 0) {
    Serial.println("Unexpected... !");
    Serial.printf("Error code: %d\n", httpResponseCode);
    redK();
    delay(1000);
    ESP.restart();

  } else {
    Serial.println("Oh, what's this?! New issue, great");
    Serial.printf("Error code: %d\n", httpResponseCode);
    fatalError();
  }
  // Free resources
  http.end();

  return payload;
}

//=[ LED indication states ]=
void kitOnline() {
  hasBeenOnline = true;
  Serial.println("  \\(◦'⌣'◦)/ KIT IS LIVE POOPERS !!! GATHER AROUND! \\(◦'⌣'◦)/");
  fill_solid(leds, NUM_LEDS, boga_c);
  FastLED.show();
  delay(100);
}

void kitRerun() {
  Serial.println("  ~(˘▾˘~) WeeeWooo IT's A Re-RUN !!! (~˘▾˘)~");
  FastLED.clear(true);
  for (int k = 0; k < 14; k++) {
    leds[k] = boga_c;
  }
  FastLED.show();
  delay(100);
}

void kitOffline() {
  Serial.println("(x╭╮x) KIT IT IS OFFILINE (x╭╮x)");
  FastLED.clear(true);
  leds[INDICAT_LED] = boga_c;
  FastLED.show();
}

void errorState(CRGB colour) {
  FastLED.clear(true);
  leds[INDICAT_LED] = colour;
  FastLED.show();
}
void fatalError() {
  FastLED.clear(true);
  fill_solid(leds, NUM_LEDS, red);
  FastLED.show();
}

void unknownError() {
  FastLED.clear(true);
  leds[INDICAT_LED] = red;
  FastLED.show();
}
void poweredOn() {
  FastLED.clear(true);
  leds[INDICAT_LED] = green;
  FastLED.show();
}
void redK(){
  FastLED.clear(true);
  for (int k = 0; k < 14; k++) {
    leds[k] = red;
  }
  FastLED.show();
}
void ayeNoDot(){
  FastLED.clear(true);
  for (int k = 14; k < 18; k++) {
    leds[k] = red;
  }
  FastLED.show();
}
void introTrail() {
  //(colour, length, decay, randomnes in decay, drawing speed delay)
  meteorRain(boga_c, 4, 64, true, 60);
}
//The bellow code if total copy from https://www.tweaking4all.com/ (full link at the top of the code)
void meteorRain(CRGB colour, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {
  FastLED.clear(true);

  for (int i = 0; i < NUM_LEDS + NUM_LEDS; i++) {
    // fade brightness all LEDs one step
    for (int j = 0; j < NUM_LEDS; j++) {
      if ((!meteorRandomDecay) || (random(10) > 5)) {
        fadeToBlack(j, meteorTrailDecay);
      }
    }
    // draw meteor
    for (int j = 0; j < meteorSize; j++) {
      if ((i - j < NUM_LEDS) && (i - j >= 0)) {
        leds[i - j] = colour;
      }
    }
    FastLED.show();
    delay(SpeedDelay);
  }
}
void fadeToBlack(int ledNo, byte fadeValue) {
  leds[ledNo].fadeToBlackBy(fadeValue);
}