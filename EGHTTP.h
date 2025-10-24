#pragma once
#include <Arduino.h>
#include "EG800KClient.h"


struct EGHTTPConfig {
String apn;
String user;
String pass;
};


class EGHTTP {
public:
EGHTTP(EG800KClient &client) : _cli(client) {}
bool beginNetwork(const EGHTTPConfig &cfg);
bool get(const String &url,
std::function<bool(uint32_t)> onLength,
std::function<bool(const uint8_t*, size_t, size_t)> onChunk);
private:
EG800KClient &_cli;
};
