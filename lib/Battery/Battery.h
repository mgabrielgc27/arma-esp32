#pragma once

class Battery
{
private:
    static constexpr int ADC_PIN = 34;
    static constexpr float DIVIDER_RATIO = 2.0f;

public:
    void init();

    float getVoltage();

    int getPercent();
};