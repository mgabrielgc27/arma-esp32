#include "EspNowManager.h"
#include <WiFi.h>
#include <esp_now.h>

void EspNowManager::connect(const uint8_t *peerAddress, DataRecvCallback_t OnDataRecv)
{
  WiFi.mode(WIFI_STA);
  esp_err_t result = esp_now_init();
  if (result != ESP_OK)
  {
    Serial.printf("Erro ao iniciar ESP-NOW: %d", result);

    while (true)
      ;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Erro ao adicionar peer");
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void EspNowManager::sendMsg(const uint8_t *peerAddress, const uint8_t *data, unsigned int len)
{
  esp_err_t result = esp_now_send(peerAddress, data, len);

  if (result == ESP_OK)
    Serial.println("Mensagem ACABOU_MUNICAO enviada");
  else
    Serial.printf("Erro ao enviar ACABOU_MUNICAO: %d\n", result);
}