#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>

#define NUM_LEDS 8
#define DATA_PIN 10
#define MAX_BRIGHTNESS 50
#define MIN_BRIGHTNESS 0
#define DELAY 15
#define PULSE_INTERVAL 3000 // 3 seconds for pulsing
#define RAINBOW_INTERVAL 20000 // 20 seconds for rainbow
#define TRANSITION_DURATION 1000 // 1 second transition

const char* ssid = "Eirini-Board🇮🇹🤝🇬🇷";
const char* password = "tortellini";

CRGB leds[NUM_LEDS];
CRGB color = CRGB::Red;
unsigned long lastSwitchTime = 0;
bool isPulsing = true;
uint8_t hue = 0;
uint8_t globalBrightness = MAX_BRIGHTNESS;
bool ledOn = true; // State to manage LED on/off

WebServer server(80);

void setup() {
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(globalBrightness);
  Serial.begin(115200);

  // Set up Wi-Fi Access Point
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(IPAddress(192,168,1,69), IPAddress(192,168,1,69), IPAddress(255,255,255,0));
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  
  // Catch-all handler for redirection
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.1.69/");
    server.send(302, "text/plain", "");
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  if (!ledOn) {
    delay(100); // Small delay to avoid busy looping
    return; // Skip the loop if the LEDs are turned off
  }

  unsigned long currentTime = millis();
  
  if ((isPulsing && currentTime - lastSwitchTime >= PULSE_INTERVAL) ||
      (!isPulsing && currentTime - lastSwitchTime >= RAINBOW_INTERVAL)) {
    transitionEffect();
    isPulsing = !isPulsing;
    lastSwitchTime = currentTime;
    Serial.println(isPulsing ? "Switching to Pulse" : "Switching to Rainbow");
  }

  if (isPulsing) {
    doPulseEffect();
  } else {
    doRainbowEffect();
  }
}

void handleRoot() {
  String html = "<html lang='en'><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Ειρήνη-Board 🇮🇹🤝🇬🇷</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; background-color: #f4f4f4; }";
  html += "h1 { color: #333; }";
  html += "p { margin: 20px 0; }";
  html += ".container { max-width: 600px; margin: 0 auto; padding: 20px; background: #fff; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += "button { background-color: #e91e63; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; border-radius: 4px; transition: background-color 0.3s ease; }";
  html += "button:hover { background-color: #c2185b; }";
  html += "</style>";
  html += "<script>";
  html += "function updateMessage() {";
  html += "  var now = new Date();";
  html += "  var day = ('0' + now.getDate()).slice(-2);";
  html += "  var month = ('0' + (now.getMonth() + 1)).slice(-2);";
  html += "  if (day === '21' && month === '01') {";
  html += "    document.getElementById('message').innerText = 'Auguri!!';";
  html += "  }";
  html += "}";
  html += "window.onload = updateMessage;";
  html += "function sendRequest(action) {";
  html += "  fetch('/' + action, { method: 'GET' })";
  html += "    .then(response => response.text())";
  html += "    .then(text => console.log(text))";
  html += "    .catch(error => console.error('Error:', error));";
  html += "}";
  html += "</script>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Ειρήνη-Board 🇮🇹🤝🇬🇷</h1>";
  html += "<p id='message'>❤️ from Italy</p>";
  html += "<p><button onclick=\"sendRequest('on')\">Turn On</button></p>";
  html += "<p><button onclick=\"sendRequest('off')\">Turn Off</button></p>";
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleOn() {
  ledOn = true;
  globalBrightness = MAX_BRIGHTNESS;
  FastLED.setBrightness(globalBrightness);
  FastLED.show();
  lastSwitchTime = millis(); // Reset the animation loop
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleOff() {
  ledOn = false;
  globalBrightness = MIN_BRIGHTNESS;
  FastLED.setBrightness(globalBrightness);
  FastLED.show();
  server.sendHeader("Location", "/");
  server.send(303);
}

void transitionEffect() {
  unsigned long startTime = millis();
  if (isPulsing) {
    // Transition from pulse to rainbow
    while (millis() - startTime < TRANSITION_DURATION) {
      float progress = (float)(millis() - startTime) / TRANSITION_DURATION;
      globalBrightness = MIN_BRIGHTNESS + (MAX_BRIGHTNESS - MIN_BRIGHTNESS) * progress;
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = blend(color, CHSV(hue + (i * 256 / NUM_LEDS), 255, 255), progress * 255);
      }
      FastLED.setBrightness(globalBrightness);
      FastLED.show();
      delay(10);
    }
  } else {
    // Transition from rainbow to pulse (just dimming)
    while (millis() - startTime < TRANSITION_DURATION) {
      float progress = (float)(millis() - startTime) / TRANSITION_DURATION;
      globalBrightness = MAX_BRIGHTNESS - (MAX_BRIGHTNESS - MIN_BRIGHTNESS) * progress;
      FastLED.setBrightness(globalBrightness);
      FastLED.show();
      delay(10);
    }
    // Ensure brightness is at minimum before starting pulse
    globalBrightness = MIN_BRIGHTNESS;
    FastLED.setBrightness(globalBrightness);
    FastLED.show();
  }
}

void doPulseEffect() {
  for (int brightness = MIN_BRIGHTNESS; brightness <= MAX_BRIGHTNESS; brightness++) {
    globalBrightness = brightness;
    FastLED.setBrightness(globalBrightness);
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
    delay(DELAY);
  }
  for (int brightness = MAX_BRIGHTNESS; brightness >= MIN_BRIGHTNESS; brightness--) {
    globalBrightness = brightness;
    FastLED.setBrightness(globalBrightness);
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
    delay(DELAY);
  }
}

void doRainbowEffect() {
  fill_rainbow(leds, NUM_LEDS, hue, 255 / NUM_LEDS);
  FastLED.setBrightness(MAX_BRIGHTNESS);
  FastLED.show();
  hue++;
  delay(10);
}
