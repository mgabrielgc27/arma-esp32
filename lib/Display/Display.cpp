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