#include <ETH.h>
#include <ESPAsyncE131.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoOTA.h>

#define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN   16
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        1
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18

#define DMX_UART_NUM    2
#define DMX_TX_PIN      4

IPAddress localIP(10, 0, 0, 10);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

HardwareSerial dmxSerial(DMX_UART_NUM);
AsyncWebServer server(80);
Preferences preferences;
ESPAsyncE131 e131(1);

uint16_t currentUniverse = 1;
bool dmxActive = false;
unsigned long lastDMXTime = 0;

void saveUniverse(uint16_t u) {
  preferences.begin("cfg", false);
  preferences.putUShort("uni", u);
  preferences.end();
}

uint16_t loadUniverse() {
  preferences.begin("cfg", true);
  uint16_t u = preferences.getUShort("uni", 1);
  preferences.end();
  return u;
}

void sendDMX(const uint8_t *data, size_t length) {
  dmxSerial.end();
  pinMode(DMX_TX_PIN, OUTPUT);
  digitalWrite(DMX_TX_PIN, LOW);
  delayMicroseconds(120);
  digitalWrite(DMX_TX_PIN, HIGH);
  delayMicroseconds(12);
  dmxSerial.begin(250000, SERIAL_8N2, -1, DMX_TX_PIN);
  dmxSerial.write(0);
  dmxSerial.write(data, length);
  dmxSerial.flush();
  dmxActive = true;
  lastDMXTime = millis();
}

void setupWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    String html = R"rawliteral(
    <!DOCTYPE html><html><head><meta charset='utf-8'><title>Universe</title>
    <script>
      function updateStatus() {
        fetch('/status').then(r => r.json()).then(j => {
          document.getElementById('status').innerText = j.dmxActive ? 'DMX aktiv' : 'Wartet auf sACN';
        });
      }
      setInterval(updateStatus, 1000);
      window.onload = updateStatus;
    </script></head><body>
    <h3>Aktuelles Universe: )rawliteral" + String(currentUniverse) + R"rawliteral(</h3>
    <form action='/set'>
    Neues Universe: <input type='number' name='u' min='1' max='63999'>
    <input type='submit' value='Speichern'>
    </form>
    <p>Status: <span id='status'>...</span></p>
    <p><a href='/status'>Status JSON</a></p>
    <hr>
    <p>Version: v1.0<br>by pJ BursT</p>
    </body></html>
    )rawliteral";
    req->send(200, "text/html", html);
  });

  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *req){
    if (req->hasParam("u")) {
      uint16_t u = req->getParam("u")->value().toInt();
      if (u >= 1 && u <= 63999 && u != currentUniverse) {
        saveUniverse(u);
        req->send(200, "text/html", "<p>Gespeichert. Neustart...</p>");
        delay(500);
        ESP.restart();
        return;
      }
    }
    req->redirect("/");
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *req){
    String j = "{";
    j += ""ip":"" + ETH.localIP().toString() + "",";
    j += ""link":" + String(ETH.linkUp() ? "true" : "false") + ",";
    j += ""universe":" + String(currentUniverse) + ",";
    j += ""dmxActive":" + String(dmxActive ? "true" : "false");
    j += "}";
    req->send(200, "application/json", j);
  });

  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- Boot ---");

  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
  ETH.config(localIP, gateway, subnet, dns);
  Serial.print("Warte auf Link..");
  while (!ETH.linkUp()) { delay(200); Serial.print("."); }
  Serial.println("\nLink ok, IP=" + ETH.localIP().toString());

  currentUniverse = loadUniverse();
  Serial.printf("Loaded Universe %u\n", currentUniverse);

  if (e131.begin(E131_MULTICAST, 1, currentUniverse)) {
    Serial.println("sACN Receiver gestartet");
  } else {
    Serial.println("sACN Start FAILED");
  }

  ArduinoOTA.begin();
  setupWebServer();
}

void loop() {
  ArduinoOTA.handle();

  if (millis() - lastDMXTime > 2000) dmxActive = false;

  e131_packet_t pkt;
  if (!e131.isEmpty()) {
    e131.pull(&pkt);
    uint16_t uni = htons(pkt.universe);
    uint16_t len = htons(pkt.property_value_count) - 1;
    if (uni == currentUniverse) {
      Serial.printf("sACN Uni %u, len=%u\n", uni, len);
      sendDMX(&pkt.property_values[1], len);
    }
  }
}