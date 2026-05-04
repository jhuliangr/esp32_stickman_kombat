#include "net/Portal.h"
#include "net/PortalPages.h"
#include "game/Game.h"

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

  void handleRoot()     { server.send(200, "text/html; charset=utf-8", PortalPages::gamesPage()); }
  void handleStickman() { server.send(200, "text/html; charset=utf-8", PortalPages::selectionPage()); }

  void handleP1() {
    Game::notifyControllerOpened();
    server.send(200, "text/html; charset=utf-8", PortalPages::controllerPage(1));
  }
  void handleP2() {
    Game::notifyControllerOpened();
    server.send(200, "text/html; charset=utf-8", PortalPages::controllerPage(2));
  }

  // Once a player is on the gamepad, OS-level captive-portal probes
  // (Android /generate_204, iOS /hotspot-detect.html, etc.) need to see
  // a canonical "you're online" response. If we keep replying with HTML,
  // the OS thinks the network is still captive and yanks the captive
  // browser back to the foreground, replacing the user's controller page.
  void handleNotFound() {
    if (Game::isInCombat()) {
      String uri = server.uri();
      if (uri.indexOf("hotspot-detect") >= 0 ||
          uri.indexOf("library/test/success") >= 0) {
        server.send(200, "text/html",
          "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
        return;
      }
      server.send(204, "text/plain", "");
      return;
    }
    // Pre-game: serve the hub so the captive-portal popup opens with our UI.
    server.send(200, "text/html; charset=utf-8", PortalPages::gamesPage());
  }
}

namespace Portal {

  const char* SSID = "ESP32-Game";

  void init() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, apMask);
    WiFi.softAP(SSID, AP_PASSWORD);
    Serial.printf("[Portal] AP \"%s\" on %s\n", SSID, WiFi.softAPIP().toString().c_str());

    dnsServer.start(DNS_PORT, "*", apIP);

    server.on("/",         handleRoot);
    server.on("/stickman", handleStickman);
    server.on("/p1",       handleP1);
    server.on("/p2",       handleP2);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("[Portal] HTTP server ready");
  }

  void loop() {
    dnsServer.processNextRequest();
    server.handleClient();
  }

}
