#pragma once

class Battery
{
private:
    const int ADC_PIN = 34;

public:
    void init();

    float getVoltage();

    int getPercent();
};