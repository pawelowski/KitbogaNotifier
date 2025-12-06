/*

  3D-printed sign that shows if the Kitboga streamer is online on Twitch

  Credit for STL design goes to makkuro https://www.thingiverse.com/thing:749887

  Other rescourses used:
  https://randomnerdtutorials.com/esp32-http-get-post-arduino/
  https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectTheatreChase

*/

// =[ libraries and configs ]=
#include <Arduino.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <WiFi.h>
#include "config.h"
#include "cert.h"
#include <Arduino_JSON.h>
#include "FastLED.h"
#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

// LED setup variables
#define DATA_PIN 2
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 60    // total number of used LEDs=60, KIT=29
#define INDICAT_LED 18 // the dot over letter i
#define BRIGHTNESS 255

// Colour definitions
CRGB boga_c = CRGB(99, 6, 171); // CRGB::Amethyst 	0x9966CC
CRGB red = CRGB(201, 0, 0);
CRGB orange = CRGB(150, 50, 0); // CRGB::Chocolate 	0xD2691E
CRGB green = CRGB(26, 110, 3);
CRGB teal = CRGB(10, 170, 173);
CRGB blue = CRGB(10, 35, 207);
CRGB yellow = CRGB(250, 255, 0);

#define BASICLOG false
#define DEBUG false // detailed debug logs

const String VER = "2.5"; // version of the script

// =[ Twich Helix API variables ]=
const char *validateOAuthURL = "https://id.twitch.tv/oauth2/validate";
String authURL = "https://id.twitch.tv/oauth2/token?client_id=" + clientID + "&client_secret=" + clientSecret + "&grant_type=client_credentials";

int user_ID = 32787655;
const char *searchStreamerURL = "https://api.twitch.tv/helix/streams?user_login=kitboga";
const char *searchChannelURL = "https://api.twitch.tv/helix/search/channels?query=kitboga";
String broadcaster_login = "kitboga";

// =[ Other variables ]=
unsigned long requestDelay = 300; // seconds
unsigned long lastTime = 0;
unsigned long prevT = 0;
bool state = false;
bool hasBeenOnline = false;     // used to run the trail effect only for the 1st time after checking if online
bool DEBUG1 = BASICLOG & DEBUG; // DEBUG will only be enabled if BASICLOG is true in the 1st place

CRGB leds[NUM_LEDS];
void setup()
{

  if (BASICLOG)
  {
    Serial.begin(115200);
  }
  delay(100);
  if (BASICLOG)
  {
    Serial.printf(">>> FW Version: %s\n", VER);
  }
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear(true);
  FastLED.setBrightness(BRIGHTNESS);
  // testColours();

  if (BASICLOG)
  {
    Serial.print(">>> Connecting");
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    connecting(blue);
  }
  FastLED.clear(true);
  if (BASICLOG)
  {
    Serial.print("\n>>> Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
  }

  setClock();

  setUpDone();

  checkStatus2();
}

void loop()
{
  // wait "requestDelay" long before making a new check
  if ((millis() - lastTime) > requestDelay * 1000)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      checkStatus2();
    }
    else
    {
      if (BASICLOG)
      {
        Serial.print("Wifi Disconnected? Status code :");
        Serial.println(WiFi.status());
      }
      errorState(blue);
    }
    lastTime = millis();
  }
}

void setClock()
{
  configTime(0, 0, "pool.ntp.org");

  if (BASICLOG)
  {
    Serial.print(F(">>> Waiting for NTP time sync: "));
  }
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2)
  {
    connecting(green);
    yield();
    nowSecs = time(nullptr);
  }

  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  if (BASICLOG)
  {
    Serial.println();
    Serial.print(F(">>> Current time: "));
    Serial.print(asctime(&timeinfo));
  }
}

void auth()
{
  if (BASICLOG)
  {
    Serial.println(">>> Authing...");
  }

  NetworkClientSecure *client = new NetworkClientSecure;
  String payload;
  if (client)
  {
    client->setCACert(id_twitch_1);
    {
      HTTPClient https;
      if (https.begin(*client, authURL))
      { // HTTPS
        int httpCode = https.POST("");
        if (httpCode > 0)
        {
          if (BASICLOG)
          {
            Serial.printf(">>> [HTTPS] POST... code: %d\n", httpCode);
          }

          payload = https.getString();
          if (DEBUG1)
          {
            Serial.print(">>> Raw Auth response:");
            Serial.println(payload);
          }
          if (httpCode == HTTP_CODE_OK)
          {
            JSONVar twitchAuthResponse = parseJson(payload);

            access_token = getAccessToken(twitchAuthResponse);
            if (BASICLOG)
            {
              Serial.println("  > New access token obtained.");
            }
          }
          else
          {
            if (BASICLOG)
            {
              Serial.printf(">>> [HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
            }
          }
        }
        else
        {
          if (BASICLOG)
          {
            Serial.printf(">>> [HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }
        }
        https.end();
      }
      else
      {
        if (BASICLOG)
        {
          Serial.printf(">>> [HTTPS] Unable to connect... Auth issue X.X\n");
        }
      }
    }
    delete client;
  }
  else
  {
    if (BASICLOG)
    {
      Serial.println(">>> Unable to create client");
    }
  }
}

String httpsGETRequest(const char *reqPath, String _auth_h, String _reqType, String _auth_v, String _c_id_h, String _c_id_v)
{

  NetworkClientSecure *client = new NetworkClientSecure;
  String payload = "{}";
  if (client)
  {
    client->setCACert(id_twitch_1);
    {
      HTTPClient https;
      if (https.begin(*client, reqPath))
      { // HTTPS
        if (DEBUG1)
          Serial.printf("> Request path: %s\n", reqPath);

        // https.addHeader("Content-Type", "application/json");
        https.addHeader(_c_id_h, _c_id_v);

        String auth_param = _reqType + _auth_v;
        if (DEBUG1)
        {
          Serial.print(">>> Contructed Auth Param: ");
          Serial.println(auth_param);
        }
        https.addHeader(_auth_h, auth_param, false, false);

        int httpCode = https.GET();
        String bufferResponse = https.getString();

        if (DEBUG1)
        {
          Serial.println(">>> Raw GET response:");
          Serial.println(bufferResponse);
        }
        if (httpCode > 0)
        {
          if (httpCode == 200)
          {
            if (BASICLOG)
            {
              Serial.printf(">>> [HTTPS] GET... code: %d\n", httpCode);
            }
            payload = bufferResponse;
          }
          else if (httpCode == 400)
          {
            errorState(yellow);
            if (BASICLOG)
            {
              Serial.println(">>> Bad request... Need a new root ca?");
            }
          }
          else if (httpCode == 401)
          {
            errorState(teal);
            if (BASICLOG)
            {
              Serial.println(">>> Unauthorized. Expired keys.");
            }
            auth();
            payload = httpsGETRequest(searchStreamerURL, "Authorization", "Bearer ", access_token, "Client-Id", clientID);
          }
          else if (httpCode == 403)
          {
            if (BASICLOG)
            {
              Serial.printf(">>> Error code: %d\n", httpCode);
              Serial.println(">>> Invalid creditentials. Check your clientID & clientSecret");
            }
            ayeNoDot();
          }
          else if (httpCode == 500)
          {
            errorState(red);
            requestDelay = 15 * 60; // try in 15min
            if (BASICLOG)
            {
              Serial.printf(">>> Error code: %d\n", httpCode);
              Serial.println(">>> Internal Server Error");
              Serial.printf(">>> Will try next in %d sec\n", requestDelay);
            }
          }
          else
          {
            if (BASICLOG)
            {
              Serial.println(">>> Oh, what's this?! New issue, great");
              Serial.printf("Error code: %d\n", httpCode);
            }
            fatalError();
          }
        }
        else
        {
          redK();
          if (BASICLOG)
          {
            Serial.printf(">>> [HTTPS] GET... failed, error: %d\n", httpCode);
            Serial.println(">>> Unexpected... !");
            Serial.println(">>> Restarting...");
          }
          delay(1000);
          ESP.restart();
        }
        // Free resources
        https.end();
      }
      else
      {
        if (BASICLOG)
        {
          Serial.printf(">>> [HTTPS] Unable to connect\n");
        }
      }
    }
    // Free resources
    delete client;
  }
  else
  {
    if (BASICLOG)
    {
      Serial.println(">>> Unable to create client");
    }
  }
  return payload;
}

JSONVar parseJson(String _stringResult)
{
  JSONVar twitchResponse = JSON.parse(_stringResult);
  if (DEBUG1)
  {
    Serial.print(">>> JSON.typeof(twitchResponse) = ");
    Serial.println(JSON.typeof(twitchResponse));
  }
  if (JSON.typeof(twitchResponse) == "undefined")
  {
    Serial.println(">>> [ERROR] Parsing input failed!");
    twitchResponse = "undefined";
  }
  else if (DEBUG1)
  {
    Serial.println("Parsed Json Response: ");
    Serial.println(twitchResponse);
  }
  return twitchResponse;
}

String getAccessToken(JSONVar _response)
{
  const char *temp = _response["access_token"];
  if (DEBUG1)
  {
    Serial.print(">>> New raw access token: ");
    Serial.println(String(temp));
  }
  return temp;
}

bool searchStream()
{
  bool isLive;
  // Look up streamers based on their user_login (can do multiple at a time). See 'searchStreamerURL' variable
  if (BASICLOG)
  {
    Serial.print(">>> Looking for a live stream... ");
  }
  JSONVar twitchStreamResponse = parseJson(httpsGETRequest(searchStreamerURL, "Authorization", "Bearer ", access_token, "Client-Id", clientID));

  if (int(twitchStreamResponse["data"].length()) > 0)
  {
    if ((atoi(twitchStreamResponse["data"][0]["user_id"]) == user_ID) && (JSON.stringify(twitchStreamResponse["data"][0]["type"]) == (const char *)("\"live\"")))
    {
      isLive = true;
    }
  }
  else
  {
    isLive = false;
    if (BASICLOG)
    {
      Serial.println(">>> No live stream.");
    }
  }
  return isLive;
}

bool searchChannel()
{
  // Search all Twitch channels based on a term. See 'searchChannelURL' variable
  bool reRunOn;

  if (BASICLOG)
  {
    Serial.print(">>> Checking channel for a re-run... ");
  }

  JSONVar twitchStreamResponse = parseJson(httpsGETRequest(searchChannelURL, "Authorization", "Bearer ", access_token, "Client-Id", clientID));

  if (int(twitchStreamResponse["data"].length()) > 0)
  {
    if (JSON.stringify(twitchStreamResponse["data"][0]["broadcaster_login"]) == (const char *)("\"kitboga\""))
    {
      if (JSON.stringify(twitchStreamResponse["data"][0]["is_live"]) == (const char *)("true"))
      {
        reRunOn = true;
      }
      else
      {
        reRunOn = false;
        if (BASICLOG)
        {
          Serial.println("No re-run.");
        }
      }
    }
  }
  else
  {
    if (BASICLOG)
    {
      Serial.println(" Can't find the channel.");
    }
    reRunOn = false;
  }
  return reRunOn;
}

bool validate2()
{
  if (BASICLOG)
  {
    Serial.println(">>> Validating2...");
  }
  NetworkClientSecure *client = new NetworkClientSecure;
  String payload = "{}";
  JSONVar twitchValidateResponse;

  if (client)
  {
    client->setCACert(id_twitch_1);
    {
      HTTPClient https;
      if (https.begin(*client, validateOAuthURL))
      { // HTTPS
        String _reqType = "Authorization";
        String _auth_v = access_token;
        String auth_param = "OAuth " + _auth_v;
        https.addHeader(_reqType, auth_param, false, false);

        int httpCode = https.GET();
        String bufferResponse = https.getString();
        if (httpCode > 0)
        {
          // HTTP header has been send and Server response header has been handled
          if (BASICLOG)
          {
            Serial.printf(">>> [HTTPS] GET... code: %d\n", httpCode);
          }

          payload = bufferResponse;
          if (DEBUG1)
          {
            Serial.print(">>> Raw Validate response:");
            Serial.println(payload);
          }
          if (httpCode == HTTP_CODE_OK)
          {
            twitchValidateResponse = parseJson(payload);
          }
          else
          {
            if (BASICLOG)
            {
              Serial.printf(">>> [HTTPS] Request... failed.\n");
            }
          }
        }
        else
        {
          if (BASICLOG)
          {
            Serial.printf(">>> [HTTPS] GET... failed, error: %d\n", httpCode);
          }
        }
        https.end();
      }
      else
      {
        if (BASICLOG)
        {
          Serial.printf(">>> [HTTPS] Unable to connect... X.X\n");
        }
      }
    }
    delete client;
  }
  else
  {
    if (BASICLOG)
    {
      Serial.println(">>> Unable to create client");
    }
  }
  bool valid;

  String _clientID = twitchValidateResponse["client_id"];
  if (_clientID)
  {
    long int validTime = twitchValidateResponse["expires_in"];
    if (validTime > 0)
    {
      valid = true;
    }
    else
      valid = false;
    if (BASICLOG)
    {
      Serial.printf("  > Keys valid for: %d sec \n", validTime);
    }
  }
  else
  {
    if (BASICLOG)
    {
      Serial.println(">>> Failed to validate");
    }
  }
  return valid;
}

void checkStatus2()
{
  if (!validate2())
  {
    auth();
  }
  if (searchStream())
  {
    if (!hasBeenOnline)
      introTrail();
    kitOnline();
  }
  else if (searchChannel())
  {
    kitRerun();
  }
  else
  {
    kitOffline();
  }
  if (BASICLOG)
  {
    Serial.printf(">>> Next check in %d seconds.\n", requestDelay);
  }
}

//=[ LED indication states ]=
void kitOnline()
{
  hasBeenOnline = true;
  if (BASICLOG)
  {
    Serial.println("  \\(◦'⌣'◦)/ KIT IS LIVE POOPERS !!! GATHER AROUND! \\(◦'⌣'◦)/");
  }
  fill_solid(leds, NUM_LEDS, boga_c);
  FastLED.show();
  delay(100);
}

void kitRerun()
{
  if (BASICLOG)
  {
    Serial.println("  ~(˘▾˘~) WeeeWooo IT's A Re-RUN !!! (~˘▾˘)~");
  }
  FastLED.clear(true);
  for (int k = 0; k < 14; k++)
  {
    leds[k] = boga_c;
  }
  FastLED.show();
  delay(100);
}

void kitOffline()
{
  if (BASICLOG)
  {
    Serial.println("(x╭╮x) KIT IS OFFILINE (x╭╮x)");
  }
  FastLED.clear(true);
  leds[INDICAT_LED] = boga_c;
  FastLED.show();
}

void errorState(CRGB colour)
{
  FastLED.clear(true);
  leds[INDICAT_LED] = colour;
  FastLED.show();
}
void fatalError()
{
  FastLED.clear(true);
  fill_solid(leds, NUM_LEDS, red);
  FastLED.show();
}

void unknownError()
{
  FastLED.clear(true);
  leds[INDICAT_LED] = red;
  FastLED.show();
}
void setUpDone()
{
  FastLED.clear(true);
  leds[INDICAT_LED] = green;
  FastLED.show();
  if (BASICLOG)
  {
    Serial.println(">>> Setup complete.");
  }
}
void redK()
{
  FastLED.clear(true);
  for (int k = 0; k < 14; k++)
  {
    leds[k] = red;
  }
  FastLED.show();
}
void ayeNoDot()
{
  FastLED.clear(true);
  for (int k = 14; k < 18; k++)
  {
    leds[k] = red;
  }
  FastLED.show();
}

void connecting(CRGB _colour)
{
  unsigned long currentT = millis();
  if ((millis() - prevT) >= 500)
  {
    if (state == false)
    {
      leds[INDICAT_LED] = _colour;
    }
    else
    {
      FastLED.clear(true);
    }
    FastLED.show();
    state = !state;
    Serial.print(".");
    prevT = millis();
  }
}

void introTrail()
{
  //(colour, length, decay, randomnes in decay, drawing speed delay)
  meteorRain(boga_c, 4, 64, true, 60);
}
// The bellow code if total copy from https://www.tweaking4all.com/ (full link at the top of the code)
void meteorRain(CRGB colour, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay)
{
  FastLED.clear(true);

  for (int i = 0; i < NUM_LEDS + NUM_LEDS; i++)
  {
    // fade brightness all LEDs one step
    for (int j = 0; j < NUM_LEDS; j++)
    {
      if ((!meteorRandomDecay) || (random(10) > 5))
      {
        fadeToBlack(j, meteorTrailDecay);
      }
    }
    // draw meteor
    for (int j = 0; j < meteorSize; j++)
    {
      if ((i - j < NUM_LEDS) && (i - j >= 0))
      {
        leds[i - j] = colour;
      }
    }
    FastLED.show();
    delay(SpeedDelay);
  }
}
void fadeToBlack(int ledNo, byte fadeValue)
{
  leds[ledNo].fadeToBlackBy(fadeValue);
}
void testColours()
{
  if (BASICLOG)
  {
    Serial.println(">>> Testing Colours...");
  }
  CRGB colours[] = {red, orange, green, boga_c, teal, blue, yellow};
  FastLED.clear(true);
  for (int i = 0; i < 7; i++)
  {
    leds[INDICAT_LED] = colours[i];
    FastLED.show();
    delay(2000);
  }
  FastLED.clear(true);
}