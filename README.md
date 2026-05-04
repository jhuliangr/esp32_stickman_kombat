# ESP32 Multiplayer Fighter

A two-player fighting game that runs entirely on an ESP32. The game is
rendered on a tiny SSD1306 OLED, and each player controls their fighter
from their phone — no app, no internet, no account: just connect to the
WiFi hotspot the ESP32 broadcasts and a captive portal pops up with the
virtual gamepad.

```
   +-------------------+
   |  =======  ======= |        
   |   P1         P2   |      
   |    o          o   |  
   |   /|\        /|\  |      
   |   / \        / \  |       
   |  ===============  |      
   +-------------------+
       OLED 128x64
```

---

## Hardware

| Part         | Notes                                            |
| ------------ | ------------------------------------------------ |
| ESP32 board  | Any "esp32dev" devkit (ESP32-WROOM-32 etc.)      |
| OLED display | SSD1306 128x64, I2C (default address `0x3C`)     |
| Buzzer       | Passive piezo buzzer (active also works)         |
| Push button  | Any momentary tactile switch (power / reset)     |
| Jumper wires |                                                  |

### Wiring

| OLED   | ESP32   |     | Buzzer | ESP32   |     | Button | ESP32   |
| ------ | ------- | --- | ------ | ------- | --- | ------ | ------- |
| VCC    | 3.3V    |     | +      | GPIO 17 |     | leg A  | GPIO 33 |
| GND    | GND     |     | -      | GND     |     | leg B  | GND     |
| SCL    | GPIO 22 |
| SDA    | GPIO 21 |

The button uses the ESP32's internal pull-up, so no external resistor is
needed — just wire one leg to GPIO 33 and the other to GND.

If your screen does not light up, the address might be `0x3D` instead of
`0x3C`. Change it in [`src/display/Display.cpp`](src/display/Display.cpp).
The buzzer pin can be changed in [`src/audio/Audio.cpp`](src/audio/Audio.cpp).
The power-button pin can be changed in [`src/power/Power.cpp`](src/power/Power.cpp)
(must be an RTC-capable GPIO so the chip can wake from deep sleep).

---

## Build & flash

The project uses [PlatformIO](https://platformio.org/). With it installed:

```bash
pio run               # compile
pio run -t upload     # flash to a connected ESP32
pio device monitor    # open the serial console (115200 baud)
```

Dependencies are pinned in [`platformio.ini`](platformio.ini):

- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets) (real-time controller transport)

---

## How to play

1. Power the ESP32. The OLED shows `Connect to: ESP32-Game`.
2. Each player connects their phone to the WiFi network
   `ESP32-Game` (password `12345678`).
3. The captive-portal page opens automatically and shows the **games
   hub**. Pick **Stickman Kombat** → choose **Player 1** or **Player 2**.
   As soon as one pad is open, the fight is shown on the OLED — even
   before anyone hits a button.
4. The pad opens a WebSocket back to the ESP32, so inputs are pushed
   instantly and a session close from the host (the power button) makes
   the page bounce back to the games hub on its own.
5. First fighter to drop the other's HP to zero wins the round; the
   scoreboard updates and a new round starts after ~2.5 s.

The scoreboard persists for as long as the ESP32 stays powered.

### Controls (per phone)

```
+---------------------+----------------+
|                     |  PUNCH   KICK  |
|       SLIDER        |                |
|   <----- o ----->   |  BLOCK   DUCK  |
|                     |                |
+---------------------+----------------+
```

| Input  | Action                                                 |
| ------ | ------------------------------------------------------ |
| Slider | Drag to walk. Push past the orange marks to **run**.   |
| PUNCH  | Quick shoulder-level jab (8 dmg).                      |
| KICK   | High kick aimed at the **head** (15 dmg).              |
| BLOCK  | Toggle guard. Damage drops to 1 while up.              |
| DUCK   | **Hold** to crouch. Removes the head from the hitbox.  |

The pad is fully **multi-touch**: drag the slider with one thumb while
tapping attack buttons with the other.

### Power button

A single button on GPIO 33 doubles as a power switch and a "give up"
control:

| Current state                      | Press behaviour                            |
| ---------------------------------- | ------------------------------------------ |
| Splash (`Connect to: ESP32-Game`)  | Shows `Press again to power off` for 3 s   |
| Splash + confirm prompt visible    | Enters deep sleep (acts as powered off)    |
| Splash + 3 s elapsed without press | Confirm prompt disappears, splash returns  |
| Mid-fight                          | Aborts the round, returns to splash        |
| Deep sleep                         | Wakes the chip; full splash + AP come back |

Deep sleep brings the chip down to ~10 µA; on wake the firmware boots
fresh, so the WiFi AP, the OLED, and the score counter all reset.

### Sound effects

The buzzer plays short non-blocking SFX driven by the same loop as the
renderer:

| When                                  | Sound                                |
| ------------------------------------- | ------------------------------------ |
| First controller page opens           | Ascending C5–E5–G5 arpeggio          |
| A clean hit lands (HP drops)          | Sharp two-tone snap                  |
| A hit is soaked by **BLOCK**          | Low dull thud                        |
| Round ends (KO)                       | Descending 4-note fanfare            |

Each effect preempts the previous one, so the audio always tracks the
most recent event without queueing up.

### Tips

- A high KICK is dodged by **DUCKing**, not by blocking.
- BLOCK reduces damage but doesn't dodge — handy against punches.
- Running covers ground twice as fast but you can't change direction
  while attacking.

---

## Architecture

The firmware is organised by concern. Each module has a header in
`include/<area>/` and an implementation in `src/<area>/`.

```
include/                     src/
  display/                     display/
    Display.h          <---->    Display.cpp        OLED driver wrapper
  audio/                       audio/
    Audio.h            <---->    Audio.cpp          Non-blocking buzzer SFX
  power/                       power/
    Power.h            <---->    Power.cpp          Button + deep sleep / reset
  game/                        game/
    Fighter.h          <---->    Fighter.cpp        State, input, hitboxes
    FighterRenderer.h  <---->    FighterRenderer.cpp  Stick-figure drawing
    Arena.h            <---->    Arena.cpp          Two fighters + KO + score
    Game.h             <---->    Game.cpp           Splash + frame loop
  net/                         net/
    Portal.h           <---->    Portal.cpp         WiFi AP + DNS + HTTP pages
    PortalPages.h      <---->    PortalPages.cpp    HTML / CSS / JS (hub + pad)
    Sockets.h          <---->    Sockets.cpp        WebSocket transport for inputs
                                main.cpp            setup() / loop()
```

### Module flow

```
                                                          +--------+
   HTTP (port 80) ------>  Portal (HTML pages)            |        |
                                                          | Phones |
   WebSocket (port 81) -----> Sockets ---> Game.onAction <------> WS
                                |             |           |        |
                                v             v           +--------+
                              close       Arena (KO -> banner + score++)
                              broadcast      |
                                +------------+------------+
                                v                         v
                          +---------+              +---------+
                          | Fighter |              | Fighter |
                          +---------+              +---------+
                                |                         |
                                +-- FighterRenderer ------+
                                            |
                                            v
                                       +---------+
                                       | Display | --- I2C ---> OLED
                                       +---------+
```

Highlights worth knowing if you read the code:

- **Fighter is logic-only.** All drawing lives in `FighterRenderer`,
  which only reads from `Fighter` through public getters. You can change
  the look of the stick figure without touching the combat code.
- **Three-phase attacks.** Each punch and kick goes through a
  `WINDUP -> ACTIVE -> RECOVERY` timeline. Damage only lands during the
  ACTIVE window, which gives the opponent a real chance to react.
- **Stride animation.** A 4-phase `stridePhase_` plus a `runningStride_`
  flag drive the walking and running poses; the same data feeds both
  legs and arms so they swing opposite to each other.
- **Splash + auto-start.** The OLED stays on the "Connect to: ..." text
  until *anyone* opens a pad page (`Game::notifyControllerOpened()`),
  then it switches into the live fight render.
- **Captive portal.** A wildcard DNS resolver maps every name to the
  ESP32, which is what triggers most phones' "captive portal detected"
  popup automatically. While a match is in progress the not-found
  handler returns the OS-canonical "you're online" probe response so
  the captive browser stops yanking players out of the gamepad.
- **Inputs over WebSocket.** Each pad opens a single `ws://host:81/?p=N`
  socket and sends the raw action name as a text frame (`punch`,
  `run_left`, etc.). The host can push `close` to make every pad bounce
  back to the games hub instantly — that's how the power button can
  abort a round without polling.
- **Games hub.** `/` lists available games (just Stickman Kombat for
  now); each game has its own player-picker page (`/stickman` → `/p1`
  or `/p2`).

### Adding a new action

1. Add an enum value to `Fighter::Action` in
   [`include/game/Fighter.h`](include/game/Fighter.h).
2. Handle it in `Fighter::doAction()` in
   [`src/game/Fighter.cpp`](src/game/Fighter.cpp).
3. Map a string name to it in `parseAction()` in
   [`src/net/Sockets.cpp`](src/net/Sockets.cpp).
4. Add a button (or slider zone) in
   [`src/net/PortalPages.cpp`](src/net/PortalPages.cpp) that calls
   `send('<name>')` (which is just `ws.send('<name>')`).

If the new action needs its own pose, also add a `drawXxx` helper in
[`src/game/FighterRenderer.cpp`](src/game/FighterRenderer.cpp) and
dispatch to it from `render()`.

---

## Configuration

Most knobs live near the top of their module's `.cpp` file:

| What                     | Where                                          |
| ------------------------ | ---------------------------------------------- |
| WiFi SSID / password     | `src/net/Portal.cpp` (`SSID`, `AP_PASSWORD`)   |
| WebSocket port           | `src/net/Sockets.cpp` (`WS_PORT`)              |
| OLED I2C address / pins  | `src/display/Display.cpp`                      |
| Buzzer pin / SFX tones   | `src/audio/Audio.cpp` (`BUZZER_PIN`, `SFX_*`)  |
| Power-button pin / timeout | `src/power/Power.cpp` (`BUTTON_PIN`, `CONFIRM_TIMEOUT_MS`) |
| Walk / run step size     | `src/game/Fighter.cpp` (`WALK_STEP_PX`, `RUN_STEP_PX`) |
| Attack timings           | `include/game/Fighter.h` (`PUNCH_*`, `KICK_*`) |
| Damage values            | `src/game/Arena.cpp` (`checkAttack`)           |
| Frame rate cap           | `src/game/Game.cpp` (`MIN_FRAME_MS`)           |
| Restart delay after KO   | `src/game/Arena.cpp` (`RESTART_MS`)            |
