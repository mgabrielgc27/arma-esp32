#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <WiFi.h>
#include <esp_now.h>

// ====== CONFIG ======
#define TRIGGER_PIN 32
#define RESET_PIN 33
#define PWM_PIN 25
#define BUZZER_PIN 26
#define LED1_PIN 27
#define LED2_PIN 14
#define LED3_PIN 12
#define LED4_PIN 13

#define PWM_CHANNEL 0
#define PWM_FREQ 27
#define PWM_RESOLUTION 10 // 10 bits (0–1023)
#define PWM_DUTY 512      // 50%

#define MAX_MUNICAO 4

uint8_t peerAddress[] = {
  0xD4, 0xE9, 0xF4, 0xBC, 0x8E, 0xA4
};

int municao = MAX_MUNICAO;
const TickType_t cooldownTicks = pdMS_TO_TICKS(1000);

TaskHandle_t xTriggerHandle, xResetHandle, xUpdateLedsHandle;
SemaphoreHandle_t xMunicaoMutex;  

void vTrigger(void *pvParams);

void vReset(void *pvParams);

void vUpdateLeds(void *pvParams);

void ARDUINO_ISR_ATTR isrTrigger(void);

void ARDUINO_ISR_ATTR isrReset(void);

void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len);

void setup() {
  Serial.begin(9600);
  // MAC ARMA D8:13:2A:74:28:BC
  // MAC BARRACA D4:E9:F4:BC:8E:A4

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, sizeof(peerAddress));
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if(esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Erro ao adicionar peer");
  }

  esp_now_register_recv_cb(OnDataRecv);

  // Configura PWM
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
  /*
  */
  attachInterrupt(TRIGGER_PIN, isrTrigger, RISING);
  attachInterrupt(RESET_PIN, isrReset, RISING);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);

  xMunicaoMutex = xSemaphoreCreateMutex();

  xTaskCreate(vTrigger, "TASK_ATIRAR", 4096, NULL, 2, &xTriggerHandle);
  xTaskCreate(vReset, "TASK_RESETAR", 4096, NULL, 1, &xResetHandle);
  xTaskCreate(vUpdateLeds, "TASK_ATUALIZAR_LEDS", 4096, NULL, 1, &xUpdateLedsHandle);

  xTaskNotifyGive(xUpdateLedsHandle);
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
        const char msg[] = "ACABOU_MUNICAO";
        esp_err_t result = esp_now_send(peerAddress, (uint8_t*)msg, strlen(msg) + 1);

        if(result == ESP_OK)
          Serial.println("Mensagem ACABOU_MUNICAO enviada");
        else
          Serial.println("Erro ao enviar ACABOU_MUNICAO");
      }
      xSemaphoreGive(xMunicaoMutex);

      if (podeAtirar) {
        lastTriggerTick = xTaskGetTickCount();
        xTaskNotifyGive(xUpdateLedsHandle);
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
      xTaskNotifyGive(xUpdateLedsHandle);
      lastTriggerTick = xTaskGetTickCount();
    }
  }
}

void vUpdateLeds(void *pvParams) {
  int copiaMunicao;
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    xSemaphoreTake(xMunicaoMutex, portMAX_DELAY);
    copiaMunicao = municao;
    xSemaphoreGive(xMunicaoMutex);

    Serial.println("TASK_ATUALIZAR_LED: Municao: " + String(copiaMunicao));
    Serial.println();
    
    switch (copiaMunicao) {
    case 4:
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, HIGH);
      digitalWrite(LED3_PIN, HIGH);
      digitalWrite(LED4_PIN, HIGH);
      break;
    case 3:
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, HIGH);
      digitalWrite(LED3_PIN, HIGH);
      digitalWrite(LED4_PIN, LOW);
      break;
    case 2:
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, HIGH);
      digitalWrite(LED3_PIN, LOW);
      digitalWrite(LED4_PIN, LOW);
      break;
    case 1:
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, LOW);
      digitalWrite(LED3_PIN, LOW);\
      digitalWrite(LED4_PIN, LOW);
      break;
    case 0:
      digitalWrite(LED1_PIN, LOW);
      digitalWrite(LED2_PIN, LOW);
      digitalWrite(LED3_PIN, LOW);
      digitalWrite(LED4_PIN, LOW);
      break;
    default:
      break;
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