#include "Battery.h"
#include <Arduino.h>

void Battery::init()
{
    analogReadResolution(12);

    analogSetPinAttenuation(ADC_PIN, ADC_11db);

    pinMode(ADC_PIN, INPUT);
}

float Battery::getVoltage()
{
    int raw = analogRead(ADC_PIN);
    float adcVoltage = ((float)raw / 4095.0f) * 3.3f;
    return adcVoltage * DIVIDER_RATIO;
}

int Battery::getPercent()
{
    float voltage = getVoltage();
    
    if (voltage >= 4.2)
        return 100;

    if (voltage <= 3.0)
        return 0;

    return (int)(((voltage - 3.0f) / (4.2f - 3.0f)) * 100.0f);
}