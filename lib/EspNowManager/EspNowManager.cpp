#include "EspNowManager.h"
#include <WiFi.h>
#include <esp_now.h>

void EspNowManager::connect(const uint8_t *peerAddress, DataRecvCallback_t OnDataRecv)
{
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Erro ao iniciar ESP-NOW");
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, sizeof(peerAddress));
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Erro ao adicionar peer");
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void EspNowManager::sendMsg(const uint8_t *peerAddress, const uint8_t *data)
{
  esp_err_t result = esp_now_send(peerAddress, data, sizeof(data));

  if (result == ESP_OK)
    Serial.println("Mensagem ACABOU_MUNICAO enviada");
  else
    Serial.println("Erro ao enviar ACABOU_MUNICAO");
}