#pragma once
#include <Arduino.h>


class EG800KClient {
public:
EG800KClient(HardwareSerial &s, int rxPin, int txPin, int pwrKeyPin = -1, unsigned long baud = 115200);
void begin();


bool powerOn();
bool setupNetwork(const String &apn, const String &user = "", const String &pass = "");


bool tcpOpen(const String &host, int port); // QIOPEN
bool tcpClose();
bool tcpSend(const uint8_t *data, size_t len); // QISEND
int tcpRecv(uint8_t *buf, size_t maxLen); // QIRD


// HTTP over raw TCP: header parse + body stream (requires Content-Length)
bool httpGet(const String &url,
std::function<bool(uint32_t)> onLength,
std::function<bool(const uint8_t*, size_t, size_t)> onChunk);


// Helpers
static bool splitHttpUrl(String url, String &host, String &path, int &port);
static int32_t parseContentLength(const String &headers);


private:
HardwareSerial &ser;
int RX, TX, PWRKEY;
unsigned long _baud;


// low-level helpers
void flushInput();
bool waitFor(const char *token, uint32_t timeoutMs);
bool readUntil(const char *token, uint32_t timeoutMs);
String readLine(uint32_t timeoutMs);
size_t readRaw(uint8_t *buf, size_t want, uint32_t timeoutMs);
void writeLine(const char *s);
bool sendAT_OK(const char *cmd, uint32_t timeoutMs = 5000);
bool sendAT_expect(const char *cmd, const char *expect, uint32_t timeoutMs = 5000);
bool waitForRegistered(int timeoutSec);
bool readHeaders(String &out);
};
