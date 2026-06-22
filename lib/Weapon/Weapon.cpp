#include "Weapon.h"
#include <Arduino.h>

void Weapon::init(isrFunc_t isrTrigger, isrFunc_t isrReload)
{
  pinMode(RELOAD_PIN, INPUT_PULLDOWN);
  pinMode(TRIGGER_PIN, INPUT_PULLDOWN);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
  attachInterrupt(TRIGGER_PIN, isrTrigger, RISING);
  attachInterrupt(RELOAD_PIN, isrReload, RISING);
}

void Weapon::reloadAmno()
{
  amno = MAX_AMNO;
}

bool Weapon::canShoot()
{
  if (amno > 0)
    return true;
  else
    return false;
}

void Weapon::startShooting()
{
  amno--;
  ledcWrite(PWM_CHANNEL, PWM_DUTY);
  digitalWrite(BUZZER_PIN, HIGH);
}

void Weapon::stopShooting()
{
  ledcWrite(PWM_CHANNEL, 0);
  digitalWrite(BUZZER_PIN, LOW);
}

int Weapon::getAmno()
{
  return amno;
}
