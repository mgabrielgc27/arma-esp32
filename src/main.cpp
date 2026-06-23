#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <Weapon.h>
#include <Display.h>
#include <Battery.h>
#include <EspNowManager.h>

const uint8_t ESP_BARR_ADDR[] = { 0xD4, 0xE9, 0xF4, 0xBC, 0x8E, 0xA4 };

Display display;
Weapon weapon;
Battery battery;

const TickType_t cooldownTicks = pdMS_TO_TICKS(1000);

TaskHandle_t xTriggerHandle = NULL;
TaskHandle_t xReloadHandle = NULL;
TaskHandle_t xBatteryHandle = NULL;
SemaphoreHandle_t xMunicaoMutex = NULL;

void vTrigger(void *pvParams);

void vReload(void *pvParams);

void vBattery(void *pvParams);

void ARDUINO_ISR_ATTR isrTrigger(void);

void ARDUINO_ISR_ATTR isrReload(void);

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len);

void setup()
{
  Serial.begin(9600);

  weapon.init(isrTrigger, isrReload);
  battery.init();
  display.init();
  display.drawAmno(weapon.getAmno());
  display.show();
  display.drawBattery(battery.getPercent());
  display.show();

  EspNowManager::connect(ESP_BARR_ADDR, OnDataRecv);

  xMunicaoMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(vTrigger, "TASK_ATIRAR", 4096, NULL, 2, &xTriggerHandle, 0);
  xTaskCreatePinnedToCore(vReload, "TASK_RELOAD", 4096, NULL, 1, &xReloadHandle, 0);
  xTaskCreatePinnedToCore(vBattery, "TASK_BATTERY", 4096, NULL, 1, &xBatteryHandle, 1);
}

void loop()
{
  // faz nada
}

void vTrigger(void *pvParams)
{
  TickType_t lastTriggerTick = 0;

  while (true)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if ((xTaskGetTickCount() - lastTriggerTick) > cooldownTicks)
    {
      Serial.println("Botão trigger");
      xSemaphoreTake(xMunicaoMutex, portMAX_DELAY);

      if (weapon.canShoot())
      {
        weapon.startShooting();
        int amno = weapon.getAmno();
        lastTriggerTick = xTaskGetTickCount();
        xSemaphoreGive(xMunicaoMutex);

        vTaskDelay(pdMS_TO_TICKS(200));
        weapon.stopShooting();

        display.drawAmno(amno);
      }
      else
      {
        xSemaphoreGive(xMunicaoMutex);

        char msg[] = "ACABOU_MUNICAO";
        EspNowManager::sendMsg(ESP_BARR_ADDR, (uint8_t *)msg, sizeof(msg));
      }
    }
  }
}

void vReload(void *pvParams)
{
  TickType_t lastTriggerTick = 0;

  while (true)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if ((xTaskGetTickCount() - lastTriggerTick) > cooldownTicks)
    {
      Serial.println("Botão reset");
      int amno;
      xSemaphoreTake(xMunicaoMutex, portMAX_DELAY);
      weapon.reloadAmno();
      amno = weapon.getAmno();
      xSemaphoreGive(xMunicaoMutex);
      display.drawAmno(amno);
      lastTriggerTick = xTaskGetTickCount();
    }
  }
}

void vBattery(void *pvParams)
{
  int lastPercent = -1;

  while (true)
  {
    int percent = battery.getPercent();

    if (percent != lastPercent)
    {
      display.drawBattery(percent);
      display.show();

      lastPercent = percent;
    }

    Serial.printf("Bateria: %d%% (%.2fV)\n", percent, battery.getVoltage());

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void ARDUINO_ISR_ATTR isrTrigger(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (xTriggerHandle != NULL)
  {
    vTaskNotifyGiveFromISR(xTriggerHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void ARDUINO_ISR_ATTR isrReload(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (xReloadHandle != NULL)
  {
    vTaskNotifyGiveFromISR(xReloadHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len)
{
  char text[32];

  memcpy(text, data, len);
  text[len] = '\0';

  if (strcmp(text, "REGARREGAR") == 0)
  {
    xTaskNotifyGive(xReloadHandle);
  }
}