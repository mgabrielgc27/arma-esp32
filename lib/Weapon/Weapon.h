#pragma once

typedef void (*isrFunc_t)(void);

class Weapon
{
private:
  const int TRIGGER_PIN = 32;
  const int RELOAD_PIN = 33;
  const int PWM_PIN = 25;
  const int PWM_CHANNEL = 0;
  const int PWM_FREQ = 25;
  const int PWM_RESOLUTION = 10; // 10 bits (0–1023)
  const int PWM_DUTY = 512;      // 50%
  const int MAX_AMNO = 10;
  int amno = MAX_AMNO;
public:
  void init(isrFunc_t isrTrigger, isrFunc_t isrReload);
  void reloadAmno();
  bool canShoot();
  void startShooting();
  void stopShooting();
  int getAmno();
};
