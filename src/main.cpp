#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <Weapon.h>
#include <Display.h>
#include <EspNowManager.h>

constexpr int BUZZER_PIN = 26;

const uint8_t ESP_BARR_ADDR[] = {
  0xD4, 0xE9, 0xF4, 0xBC, 0x8E, 0xA4
};

Display display;
Weapon weapon;

const TickType_t cooldownTicks = pdMS_TO_TICKS(1000);

TaskHandle_t xTriggerHandle, xReloadHandle;
SemaphoreHandle_t xMunicaoMutex;

void vTrigger(void *pvParams);

void vReload(void *pvParams);

void ARDUINO_ISR_ATTR isrTrigger(void);

void ARDUINO_ISR_ATTR isrReload(void);

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len);

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);

  weapon.init(isrTrigger, isrReload);

  display.begin();
  display.drawAmno(weapon.getAmno());
  display.drawBattery(60);
  
  EspNowManager::connect(ESP_BARR_ADDR, OnDataRecv);

  xMunicaoMutex = xSemaphoreCreateMutex();

  xTaskCreate(vTrigger, "TASK_ATIRAR", 4096, NULL, 2, &xTriggerHandle);
  xTaskCreate(vReload, "TASK_ReloadAR", 4096, NULL, 1, &xReloadHandle);
}

void loop() {
  // faz nada
}

void vTrigger(void *pvParams) {
  TickType_t lastTriggerTick = 0;

  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if ((xTaskGetTickCount() - lastTriggerTick) > cooldownTicks) {
      bool canShoot = false;
      int amno;
      xSemaphoreTake(xMunicaoMutex, portMAX_DELAY);
      canShoot = weapon.canShoot();
      amno = weapon.getAmno();
      xSemaphoreGive(xMunicaoMutex);
      
      if (canShoot) {
        weapon.startShooting();
        vTaskDelay(pdMS_TO_TICKS(100));
        weapon.stopShooting();
        lastTriggerTick = xTaskGetTickCount();
        display.drawAmno(amno-1);
      } else {
        char msg[] = "ACABOU_MUNICAO";
        EspNowManager::sendMsg(ESP_BARR_ADDR, (uint8_t*)msg, sizeof(msg));
      }
    }
  }
}

void vReload(void *pvParams) {
  TickType_t lastTriggerTick = 0;
  
  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if ((xTaskGetTickCount() - lastTriggerTick) > cooldownTicks) {
      int amno;
      Serial.println("Regarregando");
      xSemaphoreTake(xMunicaoMutex, portMAX_DELAY);
      weapon.reloadAmno();
      amno = weapon.getAmno();
      xSemaphoreGive(xMunicaoMutex);
      display.drawAmno(amno);
      lastTriggerTick = xTaskGetTickCount();
    }
  }
}

void ARDUINO_ISR_ATTR isrTrigger(void) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(xTriggerHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void ARDUINO_ISR_ATTR isrReload(void) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(xReloadHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  char *text = (char*)data;
  if (strcmp(text, "REGARREGAR") == 0) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(xReloadHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}