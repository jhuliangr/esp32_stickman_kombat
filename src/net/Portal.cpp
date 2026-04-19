#include "net/Portal.h"
#include "net/PortalPages.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

namespace {
  const char* AP_PASSWORD = "12345678";

  IPAddress apIP(192, 168, 4, 1);
  IPAddress apMask(255, 255, 255, 0);

  WebServer server(80);
  DNSServer dnsServer;
  constexpr byte DNS_PORT = 53;

  void handleRoot() { server.send(200, "text/html; charset=utf-8", PortalPages::selectionPage()); }
  void handleP1()   { server.send(200, "text/html; charset=utf-8", PortalPages::controllerPage(1)); }
  void handleP2()   { server.send(200, "text/html; charset=utf-8", PortalPages::controllerPage(2)); }

  void handleAction() {
    int player = server.arg("p").toInt();
    String a   = server.arg("a");
    Serial.printf("[Portal] p=%d a=%s\n", player, a.c_str());
    server.send(204, "text/plain", "");
  }

  // Captive portals work by resolving every DNS query to the AP, so any
  // request the phone makes ends up on our root handler.
  void handleNotFound() { server.send(200, "text/html; charset=utf-8", PortalPages::selectionPage()); }
}

namespace Portal {

  const char* SSID = "ESP32-Game";

  void init() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, apMask);
    WiFi.softAP(SSID, AP_PASSWORD);
    Serial.printf("[Portal] AP \"%s\" on %s\n", SSID, WiFi.softAPIP().toString().c_str());

    dnsServer.start(DNS_PORT, "*", apIP);

    server.on("/",       handleRoot);
    server.on("/p1",     handleP1);
    server.on("/p2",     handleP2);
    server.on("/action", handleAction);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("[Portal] HTTP server ready");
  }

  void loop() {
    dnsServer.processNextRequest();
    server.handleClient();
  }

}
