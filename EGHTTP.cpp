#include "EGHTTP.h"


bool EGHTTP::beginNetwork(const EGHTTPConfig &cfg) {
return _cli.powerOn() && _cli.setupNetwork(cfg.apn, cfg.user, cfg.pass);
}


bool EGHTTP::get(const String &url,
std::function<bool(uint32_t)> onLength,
std::function<bool(const uint8_t*, size_t, size_t)> onChunk) {
return _cli.httpGet(url, onLength, onChunk);
}
