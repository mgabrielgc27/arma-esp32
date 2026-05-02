#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define ARMA 25
#define GATILHO 14
#define RESET 26
#define BALA1 32
#define BALA2 33
#define BALA3 27
#define MAX_MUNICAO 3

int municao = MAX_MUNICAO;
const int freq = 1200;
const int resolution_bits = 12;
const int channel = 0;
const int duty = 2048;

SemaphoreHandle_t xMunicaoMutex;

TimerHandle_t xCooldownTimerHandle;

TaskHandle_t xAtirarHandle, xResetarHandle, xAtualizarLedsHandle;

void vAtirar(void *);
void vResetar(void *);
void vAtualizarLeds(void *);

void vCooldownTimerCallback(TimerHandle_t);

void ARDUINO_ISR_ATTR isrGatilho(void);
void ARDUINO_ISR_ATTR isrReset(void);

void setup() {
  Serial.begin(9600);

  attachInterrupt(GATILHO, isrGatilho, FALLING);
  attachInterrupt(RESET, isrReset, FALLING);
  pinMode(BALA1, OUTPUT);
  pinMode(BALA2, OUTPUT);
  pinMode(BALA3, OUTPUT);

  ledcSetup(channel, freq, resolution_bits);
  ledcAttachPin(ARMA, channel);

  xMunicaoMutex = xSemaphoreCreateMutex();

  xCooldownTimerHandle = xTimerCreate("TIRO_TIMER", pdMS_TO_TICKS(500), pdFALSE,
                                      0, vCooldownTimerCallback);

  xTaskCreate(vAtirar, "TASK_ATIRAR", 4096, NULL, 1, &xAtirarHandle);
  xTaskCreate(vResetar, "TASK_RESETAR", 4096, NULL, 1, &xResetarHandle);
  xTaskCreate(vAtualizarLeds, "TASK_ATUALIZAR_LEDS", 4096, NULL, 1,
              &xAtualizarLedsHandle);

  xTaskNotifyGive(xAtualizarLedsHandle);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void vAtirar(void *pvParams) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(30));

    if (digitalRead(GATILHO) == LOW &&
        !xTimerIsTimerActive(xCooldownTimerHandle)) {

      Serial.println("TASK_ATIRAR: GATILHO: " + String(digitalRead(GATILHO)));

      bool podeAtirar = false;

      xSemaphoreTake(xMunicaoMutex, portMAX_DELAY);
      if (municao > 0) {
        municao--;
        podeAtirar = true;
      }
      xSemaphoreGive(xMunicaoMutex);

      if (podeAtirar) {
        xTaskNotifyGive(xAtualizarLedsHandle);
        
        ledcWrite(channel, duty);
        vTaskDelay(pdMS_TO_TICKS(200));
        ledcWrite(channel, 0);
        
        Serial.println("COOLDOWN_TIMER: iniciou o timer");
        xTimerStart(xCooldownTimerHandle, 0);
      }
    }
  }
}

void vResetar(void *pvParams) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(30));

    if (digitalRead(RESET) == LOW) {

      Serial.println("TASK_RESETAR: RESET: " + String(digitalRead(RESET)));

      xSemaphoreTake(xMunicaoMutex, portMAX_DELAY);
      municao = MAX_MUNICAO;
      xSemaphoreGive(xMunicaoMutex);

      ledcWrite(channel, 0);
      
      xTimerStop(xCooldownTimerHandle, 0);

      xTaskNotifyGive(xAtualizarLedsHandle);
    }
  }
}

void vAtualizarLeds(void *pvParams) {
  int copiaMunicao;
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    xSemaphoreTake(xMunicaoMutex, portMAX_DELAY);
    copiaMunicao = municao;
    xSemaphoreGive(xMunicaoMutex);

    Serial.println("TASK_ATUALIZAR_LED: Municao: " + String(copiaMunicao));
    Serial.println();
    
    switch (copiaMunicao) {
    case 3:
      digitalWrite(BALA1, HIGH);
      digitalWrite(BALA2, HIGH);
      digitalWrite(BALA3, HIGH);
      break;
    case 2:
      digitalWrite(BALA1, HIGH);
      digitalWrite(BALA2, HIGH);
      digitalWrite(BALA3, LOW);
      break;
    case 1:
      digitalWrite(BALA1, HIGH);
      digitalWrite(BALA2, LOW);
      digitalWrite(BALA3, LOW);
      break;
    case 0:
      digitalWrite(BALA1, LOW);
      digitalWrite(BALA2, LOW);
      digitalWrite(BALA3, LOW);
      break;
    default:
      break;
    }
  }
}

void vCooldownTimerCallback(TimerHandle_t) {
  Serial.println("COOLDOWN_TIMER: acabou o timer");
  Serial.println();
}

void ARDUINO_ISR_ATTR isrGatilho(void) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(xAtirarHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void ARDUINO_ISR_ATTR isrReset(void) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(xResetarHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}