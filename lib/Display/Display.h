#pragma once

#include <Adafruit_SSD1306.h>

class Display
{
private:
  static constexpr int SCREEN_WIDTH = 128;
  static constexpr int SCREEN_HEIGHT = 64;
  static constexpr int OLED_ADDR = 0x3C;  

  Adafruit_SSD1306 oled;

public:
  Display();

  void begin();

  void clear();

  void print(const char *text);

  void println(const char *text);

  void show();

  void showHello();
};