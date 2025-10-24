#pragma once
#include <Arduino.h>
#include <Update.h>
#include "EGHTTP.h"


typedef std::function<void(size_t wrote, size_t total, double bps, double etaSec)> OtaProgressCb;


typedef std::function<void(const char *msg)> OtaLogCb;


class EGUpdater {
public:
EGUpdater(EGHTTP &http) : _http(http) {}


bool downloadAndApply(const String &url, OtaProgressCb onProgress, OtaLogCb onLog);


private:
EGHTTP &_http;
};
