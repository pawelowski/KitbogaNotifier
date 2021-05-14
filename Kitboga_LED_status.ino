/*

  3D-printed sign that shows if the Kitboga streamer is online on Twitch

  Credit for STL design goes to makkuro https://www.thingiverse.com/thing:749887

  Other rescourses used:
  https://randomnerdtutorials.com/esp32-http-get-post-arduino/

  /*
   Error Codes:
   # 1 red led = Unauthorized/Expired keys
   # 2 red leds = Auth Issue
   # 3 red leds = Internal Server Error
   # ALL red = other unknown request failure

*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "FastLED.h"
#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

// LED setup variables
#define DATA_PIN 2
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 60    //total number of used LEDs
#define INDICAT_LED 18 // the dot over letter i
#define BRIGHTNESS 120

//Colour definitions
CHSV yellow = CHSV(40, 240, 120);
CHSV boga_c = CHSV(190, 239, 155);
CHSV red = CHSV(255, 255, 130);
CHSV orange = CHSV(23, 233, 120);
CHSV green = CHSV(72, 213, 100);
// =[ WiFi variables ]=
const char *ssid = "Polska_Pany";
const char *password = "PolakiCebulaki6";

// =[ Twich Helix API variables ]=
//URLs
const char *validateOAuthURL = "https://id.twitch.tv/oauth2/validate";
const char *searchStreamerURL = "https://api.twitch.tv/helix/streams?user_login=kitboga";

int streamersID = 32787655; //kitboga stream ID

//Tokens
String clientSecret = "";
String clientID = "";
String authURL = "https://id.twitch.tv/oauth2/token?client_id=" + clientID + "&client_secret=" + clientSecret + "&grant_type=client_credentials";
String access_token = "";

// =[ Other variables ]=
unsigned long lastTime = 0;
unsigned long timerDelay = 15; //seconds

CRGB leds[NUM_LEDS];

void setup()
{
  Serial.begin(115200);
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  poweredOn();
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  //Make the 1st requests, don't wait for the interval of "timerDelay" variable
  validate();
  searchStream();
}

void loop()
{

  //Send an HTTP POST request every 'timerDelay' value
  if ((millis() - lastTime) > timerDelay * 1000)
  {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {

      // if validation successful, go and search the stream up
      if (validate())
        searchStream();
      // otherwise, keep displaying the indication state based on the http request result

      //auth(); //for debugging: check your auth separtely
    }
    else
    {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

//=[ Functions ]=
bool validate()
{
  //Checks if access token is valid before making a request
  Serial.print("Validating... ");
  //Serial.printf("Access Token: %s\n", access_token);
  String rawResponse = httpGETRequest(validateOAuthURL, "Authorization", access_token, "", "");
  int validTime = parseJson(rawResponse)["expires_in"];
  bool valid;
  if (validTime > 0)
  {
    valid = true;
  }
  else
    valid = false;
  Serial.printf("Keys valid for: %d sec \n", validTime);
  delay(500);
  return valid;
}

void auth()
{
  //Request a new access token to make a call to the API's endpoints
  Serial.print("Authing... ");
  HTTPClient http;
  http.begin(authURL);

  int httpResponseCode = http.POST("");

  String payload = "{}";

  // ALL GOOD
  if (httpResponseCode == 200)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();

    JSONVar twitchAuthResponse = parseJson(payload);

    //extract access token from the Twich API response
    access_token = JSON.stringify(twitchAuthResponse["access_token"]);
    //Serial.println(access_token); // for debugging

    //I couldn't figure out how to extract characters from inside of " ", so I just chop them off both ends.
    // Example: access_token = "a1b2c3d4e5f6g7h8i9j1k2l3m4n5o6";

    access_token.remove(0, 1); //remove 1st " (character 0)
    //Serial.printf("Access token lenght: %d\n", access_token.length());
    access_token.remove(access_token.length() - 1, 1); //remove last " (character lenght - 1)
    //Serial.printf("Access Token: %s\n", access_token); // for debugging
  }
  else
  {
    Serial.print("Auth Issue");
    errorState(1, orange);
  }
  http.end();
}

void searchStream()
{
  //Look up streamers based on their user_login (can do multiple at a time). See 'searchStreamerURL' variable
  Serial.print("Looking up stream... ");
  JSONVar twitchStreamResponse = JSON.parse(httpGETRequest(searchStreamerURL, "Authorization", access_token, "Client-Id", clientID));
  if (JSON.typeof(twitchStreamResponse) == "undefined")
  {
    Serial.println("Parsing input failed!");
  }

  //int resultLength = twitchStreamResponse["data"].length();

  if ((int(twitchStreamResponse["data"].length()) > 0) && (atoi(twitchStreamResponse["data"][0]["user_id"]) == streamersID) && (JSON.stringify(twitchStreamResponse["data"][0]["type"]) == (const char *)("\"live\"")))
  {
    //if ( (resultLength > 0) && (atoi(twitchStreamResponse["data"][0]["user_id"]) == streamersID ) && (JSON.stringify(twitchStreamResponse["data"][0]["type"]) == (const char*)("\"live\"") )) {
    Serial.println("=^= KIT IS LIVE BITCHES !!! GATHER AROUND! =^=");
    kitOnline();
  }
  else
  {
    Serial.println("KIT IS OFFILINE =[");
    kitOffline();
  }
}

JSONVar parseJson(String _stringResult)
{
  JSONVar twitchResponse = JSON.parse(_stringResult);
  if (JSON.typeof(twitchResponse) == "undefined")
  {
    Serial.println("Parsing input failed!");
    //    return;
  }
  return twitchResponse;
}

String httpGETRequest(const char *reqPath, String _auth_h, String _auth_v, String _c_id_h, String _c_id_v)
{

  HTTPClient http;
  http.begin(reqPath);

  //http.addHeader("Content-Type", "text/plain");
  http.addHeader(_c_id_h, _c_id_v);
  String auth_param = "Bearer " + _auth_v;
  http.addHeader(_auth_h, auth_param, false, false);

  int httpResponseCode = http.GET();

  String payload = "{}";

  // ALL GOOD.
  if (httpResponseCode == 200)
  {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    payload = http.getString();

    // Unauthorized - expired keys, so re-auth.
  }
  else if (httpResponseCode == 401)
  {
    errorState(1, yellow); //yellow
    Serial.printf("Error code: %d\n", httpResponseCode);
    Serial.println("Unauthorized.");
    auth();

    //Internal Server Error.
  }
  else if (httpResponseCode == 500)
  {
    errorState(1, red);
    Serial.printf("Error code: %d\n", httpResponseCode);
    Serial.println("Internal Server Error");
    timerDelay = 15 * 60; // try in 15min
    Serial.printf("Will try next in %d sec\n", timerDelay);
  }
  else
  {
    Serial.println("Unexpected... !");
    Serial.printf("Error code: %d\n", httpResponseCode);
    errorState(NUM_LEDS, red);
  }

  // Free resources
  http.end();

  return payload;
}

//=[ LED indication states ]=
void kitOnline()
{
  fill_solid(leds, NUM_LEDS, boga_c);
  FastLED.show();
  delay(100);
}

void kitOffline()
{
  FastLED.clear(true);
  leds[INDICAT_LED] = boga_c;
  FastLED.show();
}

void errorState(int num, CHSV colour)
{
  FastLED.clear(true);
  fill_solid(leds, num, colour);
  FastLED.show();
}
void poweredOn()
{
  FastLED.clear(true);
  leds[INDICAT_LED] = green;
  FastLED.show();
}