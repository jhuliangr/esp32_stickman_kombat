#include "display/Display.h"

#include <Wire.h>
#include <Adafruit_GFX.h>

namespace {
  constexpr gpio_num_t  OLED_SDA   = GPIO_NUM_21;
  constexpr gpio_num_t  OLED_SCL   = GPIO_NUM_22;
  constexpr int  OLED_RESET = -1;
  constexpr byte OLED_ADDR  = 0x3C;   // try 0x3D if nothing shows up

  // 400 kHz I2C: at the default 100 kHz the flush caps us around 10 FPS.
  constexpr uint32_t I2C_CLOCK = 400000;

  Adafruit_SSD1306 oled(Display::WIDTH, Display::HEIGHT, &Wire, OLED_RESET);
}

namespace Display {

  void init() {
    Wire.begin(OLED_SDA, OLED_SCL);
    Wire.setClock(I2C_CLOCK);
    if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
      Serial.println("[Display] SSD1306 not found.");
      while (true) delay(1000);
    }
    oled.clearDisplay();
    oled.display();
  }

  void showText(const char* line1, const char* line2) {
    oled.clearDisplay();
    oled.setTextSize(2);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 10);
    oled.println(line1);
    if (line2) {
      oled.setCursor(0, 35);
      oled.println(line2);
    }
    oled.display();
  }

  void beginFrame()          { oled.clearDisplay(); }
  void endFrame()            { oled.display(); }
  Adafruit_SSD1306& canvas() { return oled; }

  void sleep() {
    oled.ssd1306_command(SSD1306_DISPLAYOFF);
  }

}
