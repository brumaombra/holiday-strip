#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

#include "display.h"
#include "scenes.h"
#include "secrets.h"

static const uint32_t FRAME_MS = 33;
static const float SCENE_SECONDS = 20.0f;
static const float FADE_SECONDS = 1.0f;
static const char *TIMEZONE = "CET-1CEST,M3.5.0,M10.5.0/3";

struct Settings {
  bool on = true;
  uint8_t brightness = 60;
  int scene = -1;  // -1 rotates through all scenes
  uint32_t maxMilliamps = 3000;
  bool countdown = true;
  String message = "BUON NATALE * MERRY CHRISTMAS";
};

static Settings settings;
static Preferences prefs;
static WebServer server(80);

static Frame frameA, frameB, output;
static int currentScene = 0;
static int nextScene = -1;
static uint32_t sceneStartMs = 0, fadeStartMs = 0, lastFrameMs = 0;

static void loadSettings() {
  prefs.begin("matrix", true);
  settings.on = prefs.getBool("on", settings.on);
  settings.brightness = prefs.getUChar("brightness", settings.brightness);
  settings.scene = prefs.getInt("scene", settings.scene);
  settings.maxMilliamps = prefs.getUInt("maxma", settings.maxMilliamps);
  settings.countdown = prefs.getBool("countdown", settings.countdown);
  settings.message = prefs.getString("message", settings.message);
  prefs.end();
  if (settings.scene >= scenes::count()) settings.scene = -1;
}

static void saveSettings() {
  prefs.begin("matrix", false);
  prefs.putBool("on", settings.on);
  prefs.putUChar("brightness", settings.brightness);
  prefs.putInt("scene", settings.scene);
  prefs.putUInt("maxma", settings.maxMilliamps);
  prefs.putBool("countdown", settings.countdown);
  prefs.putString("message", settings.message);
  prefs.end();
}

static void applySceneSettings() {
  scenes::setMessage(settings.message);
  scenes::setCountdown(settings.countdown);
}

static void startScene(int index) {
  currentScene = index;
  nextScene = -1;
  scenes::get(index)->begin();
  sceneStartMs = millis();
}

static void startFade(int index) {
  if (index == currentScene || nextScene >= 0) return;
  nextScene = index;
  scenes::get(index)->begin();
  fadeStartMs = millis();
}

static void renderFrame() {
  uint32_t now = millis();
  float dt = min(0.1f, (now - lastFrameMs) / 1000.0f);
  lastFrameMs = now;

  if (!settings.on) {
    fillFrame(output, {0, 0, 0});
    display::show(output, 0, settings.maxMilliamps);
    return;
  }

  int wanted = settings.scene;
  float sceneTime = (now - sceneStartMs) / 1000.0f;
  if (wanted >= 0 && wanted != currentScene) startFade(wanted);
  if (wanted < 0 && sceneTime >= SCENE_SECONDS) startFade((currentScene + 1) % scenes::count());

  scenes::get(currentScene)->draw(frameA, sceneTime, dt);
  if (nextScene < 0) {
    memcpy(output, frameA, sizeof(Frame));
    display::show(output, settings.brightness, settings.maxMilliamps);
    return;
  }

  float fadeTime = (now - fadeStartMs) / 1000.0f;
  scenes::get(nextScene)->draw(frameB, fadeTime, dt);
  float k = fadeTime / FADE_SECONDS;
  for (int y = 0; y < HEIGHT; y++)
    for (int x = 0; x < WIDTH; x++) output[y][x] = blendColor(frameA[y][x], frameB[y][x], k);
  display::show(output, settings.brightness, settings.maxMilliamps);

  if (k >= 1.0f) {
    currentScene = nextScene;
    nextScene = -1;
    sceneStartMs = fadeStartMs;
  }
}

static String jsonEscape(const String &s) {
  String out;
  for (char c : s) {
    if (c == '"' || c == '\\') out += '\\';
    out += c;
  }
  return out;
}

static void handleState() {
  String json = "{\"on\":" + String(settings.on ? "true" : "false") +
                ",\"brightness\":" + settings.brightness + ",\"scene\":" + settings.scene +
                ",\"current_scene\":" + currentScene + ",\"max_milliamps\":" + settings.maxMilliamps +
                ",\"estimated_milliamps\":" + display::estimatedMilliamps() +
                ",\"countdown\":" + (settings.countdown ? "true" : "false") + ",\"message\":\"" +
                jsonEscape(settings.message) + "\",\"scenes\":[";
  for (int i = 0; i < scenes::count(); i++) {
    if (i) json += ",";
    json += "\"" + String(scenes::get(i)->name()) + "\"";
  }
  json += "],\"uptime_s\":" + String(millis() / 1000) + "}";
  server.send(200, "application/json", json);
}

// GET /api/set?on=1&brightness=80&scene=-1&message=...&countdown=1&max_milliamps=3000
static void handleSet() {
  if (server.hasArg("on")) settings.on = server.arg("on") == "1";
  if (server.hasArg("brightness")) settings.brightness = constrain(server.arg("brightness").toInt(), 0, 255);
  if (server.hasArg("scene")) {
    int scene = server.arg("scene").toInt();
    settings.scene = (scene >= 0 && scene < scenes::count()) ? scene : -1;
  }
  if (server.hasArg("max_milliamps"))
    settings.maxMilliamps = constrain(server.arg("max_milliamps").toInt(), 1000, 60000);
  if (server.hasArg("countdown")) settings.countdown = server.arg("countdown") == "1";
  if (server.hasArg("message")) {
    String message = server.arg("message");
    message.toUpperCase();
    settings.message = message.substring(0, 200);
  }
  applySceneSettings();
  saveSettings();
  handleState();
}

static const char INDEX_HTML[] PROGMEM = R"html(<!doctype html>
<html><head><meta name="viewport" content="width=device-width,initial-scale=1"><title>Christmas matrix</title>
<style>
body{font-family:sans-serif;background:#0b1a2e;color:#eee;max-width:480px;margin:auto;padding:1em}
h1{color:#ff5252}label{display:block;margin-top:1.2em}input,select,button{font-size:1em;width:100%;box-sizing:border-box;margin-top:.3em}
button{padding:.6em;background:#1b7f3a;color:#fff;border:0;border-radius:6px}small{color:#9ab}
</style></head><body>
<h1>&#127876; Christmas matrix</h1>
<label><input type="checkbox" id="on" style="width:auto"> Display on</label>
<label>Brightness <span id="bv"></span><input type="range" id="brightness" min="5" max="255"></label>
<label>Scene<select id="scene"></select></label>
<label>Message<input id="message" maxlength="200"></label>
<label><input type="checkbox" id="countdown" style="width:auto"> Christmas countdown</label>
<label>Power limit (mA)<input type="number" id="max_milliamps" min="1000" max="60000" step="500"></label>
<button id="save">Save message</button>
<p><small id="status"></small></p>
<script>
const $=id=>document.getElementById(id);
function show(s){
  $('on').checked=s.on;$('brightness').value=s.brightness;$('bv').textContent=s.brightness;
  if(!$('scene').options.length){$('scene').add(new Option('Rotate all scenes',-1));s.scenes.forEach((n,i)=>$('scene').add(new Option(n,i)));}
  $('scene').value=s.scene;$('countdown').checked=s.countdown;$('max_milliamps').value=s.max_milliamps;
  if(document.activeElement!==$('message'))$('message').value=s.message;
  $('status').textContent='Showing: '+s.scenes[s.current_scene]+' | estimated current: '+s.estimated_milliamps+' mA';
}
function set(q){fetch('/api/set?'+q).then(r=>r.json()).then(show);}
$('on').onchange=e=>set('on='+(e.target.checked?1:0));
$('brightness').oninput=e=>$('bv').textContent=e.target.value;
$('brightness').onchange=e=>set('brightness='+e.target.value);
$('scene').onchange=e=>set('scene='+e.target.value);
$('countdown').onchange=e=>set('countdown='+(e.target.checked?1:0));
$('max_milliamps').onchange=e=>set('max_milliamps='+e.target.value);
$('save').onclick=()=>set('message='+encodeURIComponent($('message').value));
function poll(){fetch('/api/state').then(r=>r.json()).then(show);}
poll();setInterval(poll,3000);
</script></body></html>)html";

static void setupNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  configTzTime(TIMEZONE, "pool.ntp.org", "time.google.com");

  server.on("/", [] { server.send_P(200, "text/html", INDEX_HTML); });
  server.on("/api/state", handleState);
  server.on("/api/set", handleSet);
  // Raw RGB of the current frame (72x12, row by row), before gamma and brightness
  server.on("/api/frame", [] {
    server.send_P(200, "application/octet-stream", (const char *)output, sizeof(Frame));
  });
  server.begin();

  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onStart([] {
    fillFrame(output, {0, 0, 0});
    display::show(output, 0, settings.maxMilliamps);
  });
  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  display::begin();
  loadSettings();
  applySceneSettings();
  setupNetwork();
  startScene(settings.scene >= 0 ? settings.scene : 0);
  lastFrameMs = millis();
}

void loop() {
  static bool announced = false;
  if (!announced && WiFi.status() == WL_CONNECTED) {
    announced = true;
    MDNS.addService("http", "tcp", 80);
    Serial.printf("Connected, IP: %s\n", WiFi.localIP().toString().c_str());
  }
  if (WiFi.status() != WL_CONNECTED) announced = false;

  server.handleClient();
  ArduinoOTA.handle();

  if (millis() - lastFrameMs >= FRAME_MS) renderFrame();
}
