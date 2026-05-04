#include "net/Sockets.h"
#include "game/Game.h"
#include "game/Fighter.h"

#include <Arduino.h>
#include <WebSocketsServer.h>

namespace {
  constexpr int WS_PORT     = 81;
  constexpr int MAX_CLIENTS = 8;

  WebSocketsServer ws(WS_PORT);

  // Maps a WebSocket client number to a player id (1 or 2). 0 = unassigned.
  uint8_t clientToPlayer[MAX_CLIENTS] = {0};

  bool parseAction(const String& a, Fighter::Action& out) {
    if      (a == "left")      out = Fighter::Action::LEFT;
    else if (a == "right")     out = Fighter::Action::RIGHT;
    else if (a == "run_left")  out = Fighter::Action::RUN_LEFT;
    else if (a == "run_right") out = Fighter::Action::RUN_RIGHT;
    else if (a == "punch")     out = Fighter::Action::PUNCH;
    else if (a == "kick")      out = Fighter::Action::KICK;
    else if (a == "block")     out = Fighter::Action::BLOCK;
    else if (a == "duck_on")   out = Fighter::Action::DUCK_ON;
    else if (a == "duck_off")  out = Fighter::Action::DUCK_OFF;
    else return false;
    return true;
  }

  void onEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
      case WStype_CONNECTED: {
        // Payload is the requested URL, e.g. "/?p=1".
        String url((const char*)payload);
        int idx    = url.indexOf("p=");
        int player = (idx >= 0) ? url.substring(idx + 2).toInt() : 0;
        if (player != 1 && player != 2) {
          Serial.printf("[WS] connect num=%u rejected (no player)\n", num);
          ws.disconnect(num);
          return;
        }
        if (num < MAX_CLIENTS) clientToPlayer[num] = (uint8_t)player;
        Game::notifyControllerOpened();
        Serial.printf("[WS] connect num=%u player=%d\n", num, player);
        break;
      }
      case WStype_DISCONNECTED:
        Serial.printf("[WS] disconnect num=%u\n", num);
        if (num < MAX_CLIENTS) clientToPlayer[num] = 0;
        break;
      case WStype_TEXT: {
        if (num >= MAX_CLIENTS) return;
        int player = clientToPlayer[num];
        if (player != 1 && player != 2) return;
        String a((const char*)payload, length);
        Fighter::Action action;
        if (!parseAction(a, action)) return;
        Game::onPlayerAction(player, action);
        break;
      }
      default:
        break;
    }
  }
}

namespace Sockets {

  void init() {
    ws.begin();
    ws.onEvent(onEvent);
    Serial.printf("[Sockets] WebSocket server on port %d\n", WS_PORT);
  }

  void loop() {
    ws.loop();
  }

  void closeAll() {
    ws.broadcastTXT("close");
    // The browser navigates on "close" and the resulting socket close fires
    // WStype_DISCONNECTED, which clears clientToPlayer naturally.
  }

}
