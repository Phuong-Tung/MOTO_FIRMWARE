#include "EG800KClient.h"
int n = ser.readBytes(buf+got, want-got);
if (n>0) got += (size_t)n; else delay(1);
}
return got;
}


void EG800KClient::writeLine(const char *s) { ser.print(s); ser.print("\r\n"); }


bool EG800KClient::sendAT_OK(const char *cmd, uint32_t timeoutMs) { writeLine(cmd); return waitFor("OK\r\n", timeoutMs); }


bool EG800KClient::sendAT_expect(const char *cmd, const char *expect, uint32_t timeoutMs) { writeLine(cmd); return waitFor(expect, timeoutMs); }


bool EG800KClient::waitForRegistered(int timeoutSec) {
sendAT_OK("AT+CPIN?");
uint32_t t0 = millis();
while ((millis()-t0) < (uint32_t)timeoutSec*1000UL) {
writeLine("AT+CREG?");
String line = readLine(1000);
if (line.indexOf(",1")>=0 || line.indexOf(",5")>=0) return true;
delay(1000);
}
return false;
}


int32_t EG800KClient::parseContentLength(const String &headers) {
int idx = headers.indexOf("Content-Length:");
if (idx < 0) idx = headers.indexOf("content-length:");
if (idx < 0) return -1;
int end = headers.indexOf('\n', idx);
String v = headers.substring(idx, end >= 0 ? end : headers.length());
int colon = v.indexOf(':');
if (colon >= 0) v = v.substring(colon+1);
v.trim();
return v.toInt();
}


bool EG800KClient::splitHttpUrl(String url, String &host, String &path, int &port) {
host = ""; path = "/"; port = 80; url.trim();
if (url.startsWith("https://")) return false; // not supported in this minimal client
if (url.startsWith("http://")) url.remove(0,7);
int slash = url.indexOf('/');
String hostport = (slash<0)? url : url.substring(0,slash);
path = (slash<0)? "/" : url.substring(slash);
int colon = hostport.indexOf(':');
if (colon>=0){ host = hostport.substring(0,colon); port = max(1, hostport.substring(colon+1).toInt()); }
else { host = hostport; port = 80; }
return host.length()>0;
}


bool EG800KClient::readHeaders(String &out) {
out = ""; uint32_t t0 = millis(); String acc;
while (millis()-t0 < 10000) {
while (ser.available()) { char c=(char)ser.read(); acc+=c; if (acc.endsWith("\r\n\r\n")) { out = acc; return true; } }
delay(2);
}
return false;
}
