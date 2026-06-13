#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <Display.h>
#include <EspNowManager.h>

// ====== CONFIG ======
#define TRIGGER_PIN 32
#define RESET_PIN 33
#define PWM_PIN 25
#define BUZZER_PIN 26

#define PWM_CHANNEL 0
#define PWM_FREQ 25
#define PWM_RESOLUTION 10 // 10 bits (0–1023)
#define PWM_DUTY 512      // 50%

#define MAX_MUNICAO 4

uint8_t ESP_BARR_ADDR[] = {
  0xD4, 0xE9, 0xF4, 0xBC, 0x8E, 0xA4
};

Display display;

int municao = MAX_MUNICAO;
const TickType_t cooldownTicks = pdMS_TO_TICKS(1000);

TaskHandle_t xTriggerHandle, xResetHandle;
SemaphoreHandle_t xMunicaoMutex;

void vTrigger(void *pvParams);

void vReset(void *pvParams);

void ARDUINO_ISR_ATTR isrTrigger(void);

void ARDUINO_ISR_ATTR isrReset(void);

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len);

void setup() {
  Serial.begin(9600);

  EspNowManager::init(ESP_BARR_ADDR, OnDataRecv);

  display.begin();
  display.showHello();

  // Configura PWM
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
  /*
  */
  attachInterrupt(TRIGGER_PIN, isrTrigger, RISING);
  attachInterrupt(RESET_PIN, isrReset, RISING);
  pinMode(BUZZER_PIN, OUTPUT);

  xMunicaoMutex = xSemaphoreCreateMutex();

  xTaskCreate(vTrigger, "TASK_ATIRAR", 4096, NULL, 2, &xTriggerHandle);
  xTaskCreate(vReset, "TASK_RESETAR", 4096, NULL, 1, &xResetHandle);
}

void loop() {
  // faz nada
}

void vTrigger(void *pvParams) {
  TickType_t lastTriggerTick = 0;

  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if ((xTaskGetTickCount() - lastTriggerTick) > cooldownTicks) {
      bool podeAtirar = false;

      xSemaphoreTake(xMunicaoMutex, portMAX_DELAY);
      if (municao > 0) {
        municao--;
        podeAtirar = true;
      } else {
        EspNowManager::sendOutOfAmno(ESP_BARR_ADDR);
      }
      xSemaphoreGive(xMunicaoMutex);

      if (podeAtirar) {
        lastTriggerTick = xTaskGetTickCount();
        digitalWrite(BUZZER_PIN, HIGH);
        ledcWrite(PWM_CHANNEL, PWM_DUTY);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(BUZZER_PIN, LOW);
        ledcWrite(PWM_CHANNEL, 0);
      }
    }
  }
}

void vReset(void *pvParams) {
  TickType_t lastTriggerTick = 0;
  
  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if ((xTaskGetTickCount() - lastTriggerTick) > cooldownTicks) {
      Serial.println("Regarregando");
      xSemaphoreTake(xMunicaoMutex, portMAX_DELAY);
      municao = MAX_MUNICAO;
      xSemaphoreGive(xMunicaoMutex);
      lastTriggerTick = xTaskGetTickCount();
    }
  }
}

void ARDUINO_ISR_ATTR isrTrigger(void) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(xTriggerHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void ARDUINO_ISR_ATTR isrReset(void) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(xResetHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  char *text = (char*)data;
  if (strcmp(text, "REGARREGAR") == 0) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(xResetHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}