#pragma once

#include <cstdint>

// MAC ARMA D8:13:2A:74:28:BC
// MAC BARRACA D4:E9:F4:BC:8E:A4

typedef void (*DataRecvCallback_t)(const uint8_t *mac, const uint8_t *data, int len);

class EspNowManager
{
private:
  
public:
  static void connect(const uint8_t* peerAddress, DataRecvCallback_t OnDataRecv);

  static void sendMsg(const uint8_t *peerAddress, const uint8_t *data, unsigned int len);
};
