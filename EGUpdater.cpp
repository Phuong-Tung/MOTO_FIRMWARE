#include "EGUpdater.h"


static String fmtSpeed(double bps){ if(bps<1024) return String((int)bps)+" B/s"; if(bps<1048576) return String(bps/1024.0,1)+" KB/s"; return String(bps/1048576.0,2)+" MB/s"; }
static String fmtETA(double s){ if(s<0) s=0; uint32_t t=(uint32_t)(s+0.5); char b[16]; snprintf(b,sizeof(b),"%02u:%02u",t/60,t%60); return String(b); }


bool EGUpdater::downloadAndApply(const String &url, OtaProgressCb onProgress, OtaLogCb onLog){
uint32_t contentLen=0; size_t total=0; uint32_t t0=millis();


bool ok = _http.get(url,
[&](uint32_t len){ contentLen=len; if(onLog) onLog((String("[OTA] Content-Length=")+len).c_str()); return Update.begin(len); },
[&](const uint8_t* data,size_t n,size_t totalSoFar){ total=totalSoFar; size_t w=Update.write(data,n); if(w!=n){ if(onLog) onLog((String("[OTA] Write error at ")+totalSoFar).c_str()); return false; } double el=(millis()-t0)/1000.0; double bps= (el>0)? total/el:0; double remain=(bps>0)? (contentLen-total)/bps:-1; if(onProgress) onProgress(total,contentLen,bps,remain); return true; }
);


if(!ok){ if(onLog) onLog("[HTTP] Download failed"); return false; }
if(!Update.end()){ if(onLog) onLog((String("[OTA] Update.end failed: ")+Update.errorString()).c_str()); return false; }
if(onLog) onLog("[OTA] Update OK. Rebooting in 2s..."); delay(2000); ESP.restart();
return true;
}
