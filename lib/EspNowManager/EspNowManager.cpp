#include "EspNowManager.h"
#include <WiFi.h>
#include <esp_now.h>

void EspNowManager::init(uint8_t *peerAddress, DataRecvCallback_t OnDataRecv)
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

void EspNowManager::sendOutOfAmno(uint8_t *peerAddress)
{
  const char msg[] = "ACABOU_MUNICAO";
  esp_err_t result = esp_now_send(peerAddress, (uint8_t *)msg, strlen(msg) + 1);

  if (result == ESP_OK)
    Serial.println("Mensagem ACABOU_MUNICAO enviada");
  else
    Serial.println("Erro ao enviar ACABOU_MUNICAO");
}