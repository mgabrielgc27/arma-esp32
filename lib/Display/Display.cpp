#include "Display.h"
#include <Arduino.h>
#include <Wire.h>

Display::Display()
    : oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1)
{
}

void Display::begin()
{
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("Falha ao inicializar o display");

    while (true)
      ;
  }

  oled.clearDisplay();
  oled.display();
}

void Display::clear()
{
  oled.clearDisplay();
}

void Display::print(const char *text)
{
  oled.print(text);
}

void Display::println(const char *text)
{
  oled.println(text);
}

void Display::show()
{
  oled.display();
}

void Display::showHello()
{
  oled.clearDisplay();

  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);

  oled.println("Hello World!");
  oled.println("ESP32 + OOP");

  oled.display();
}

void Display::drawBattery(int batteryPercent)
{
  oled.fillRect(70, 0, 58, 12, SSD1306_BLACK);

  int x = 100;
  int y = 0;

  // Corpo da bateria
  oled.drawRect(x, y, 20, 10, SSD1306_WHITE);

  // Terminal da bateria
  oled.fillRect(x + 20, y + 3, 2, 4, SSD1306_WHITE);

  // Nível da bateria
  int level = map(batteryPercent, 0, 100, 0, 18);
  oled.fillRect(x + 1, y + 1, level, 8, SSD1306_WHITE);

  // Porcentagem
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(70, 1);
  oled.printf("%d%%", batteryPercent);
}

void Display::drawAmno(int amno)
{
  int x = (128 - 36) / 2;
  int y = (64 - 24) / 2;

  oled.fillRect(x, y, 36, 24, SSD1306_BLACK);

  oled.setTextSize(3);

  String text = String(amno);

  int16_t x1, y1;
  uint16_t w, h;

  oled.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int centerX = (SCREEN_WIDTH - w) / 2;
  int centerY = (SCREEN_HEIGHT - h) / 2;

  oled.setCursor(centerX, centerY);
  oled.print(text);

  oled.display();
}