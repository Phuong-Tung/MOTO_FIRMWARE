#include <Arduino.h>
#include "EG800KClient.h"
#include "EGHTTP.h"
#include "EGUpdater.h"


// ===== User config =====
static const char* APN = "internet"; // e.g. "v-internet"
static const char* APN_USER = "";
static const char* APN_PASS = "";


static const int MODEM_RX = 16; // ESP32 RX <- EG800K TX
static const int MODEM_TX = 17; // ESP32 TX -> EG800K RX
static const int MODEM_PWRKEY = -1; // set to a GPIO if used
static const unsigned long MODEM_BAUD = 115200;


static const char* CURRENT_VERSION = "1.0.8";
static const char* VERSION_URL = "http://raw.githubusercontent.com/Phuong-Tung/MOTO_FIRMWARE/refs/heads/main/version.txt";
static const char* FW_URL = "http://github.com/Phuong-Tung/MOTO_FIRMWARE/releases/latest/download/ESP32_FIRMWARE.ino.esp32c3.bin";


static const uint32_t UPDATE_CHECK_INTERVAL_MS = 5UL*60UL*1000UL;


// ===== Globals =====
HardwareSerial ModemSerial(1);
EG800KClient eg(ModemSerial, MODEM_RX, MODEM_TX, MODEM_PWRKEY, MODEM_BAUD);
EGHTTP http(eg);
EGUpdater updater(http);


QueueHandle_t q_evt;
enum EvtType { EVT_MODEM_READY };
struct AppEvt { EvtType type; };


// ===== Tasks =====
void ModemTask(void*){
Serial.println("[MODEM] begin");
eg.begin();
EGHTTPConfig cfg{ String(APN), String(APN_USER), String(APN_PASS) };
while(!http.beginNetwork(cfg)){
Serial.println("[MODEM] network failed, retry in 5s");
vTaskDelay(pdMS_TO_TICKS(5000));
}
Serial.println("[MODEM] PDP active");
if(q_evt){ AppEvt e{EVT_MODEM_READY}; xQueueSend(q_evt,&e,0); }
for(;;){ vTaskDelay(pdMS_TO_TICKS(1000)); }
}


void UpdaterTask(void*){
uint32_t last=0; bool ready=false;
for(;;){
AppEvt e; if(q_evt && xQueueReceive(q_evt,&e,pdMS_TO_TICKS(10))==pdTRUE){ if(e.type==EVT_MODEM_READY) ready=true; }
uint32_t now=millis();
if(ready && (now-last>=UPDATE_CHECK_INTERVAL_MS)){
last=now;
Serial.println("[UPDATE] check latest version");
String latest="";
bool ok = eg.httpGet(String(VERSION_URL),
[&](uint32_t){ return true; },
[&](const uint8_t* d,size_t n,size_t){ for(size_t i=0;i<n;i++) if(d[i]>=32 && d[i]<=126) latest+=(char)d[i]; return true; }
);
latest.trim();
if(!ok || latest.length()==0){ Serial.println("[UPDATE] fetch version failed"); continue; }
Serial.printf("[UPDATE] current=%s latest=%s\n", CURRENT_VERSION, latest.c_str());
if(latest != String(CURRENT_VERSION)){
Serial.println("[UPDATE] new FW found → start OTA");
updater.downloadAndApply(String(FW_URL),
/*progress*/ [](size_t wrote,size_t total,double bps,double eta){
Serial.printf("[OTA] %u/%u (%.1f%%) | %s | ETA %s\n", (unsigned)wrote,(unsigned)total, 100.0*(double)wrote/(double)total, fmtSpeed(bps).c_str(), fmtETA(eta).c_str());
},
/*log*/ [](const char* msg){ Serial.println(msg); }
);
} else {
Serial.println("[UPDATE] up to date");
}
}
vTaskDelay(pdMS_TO_TICKS(100));
}
}


void setup(){
Serial.begin(115200); delay(200); Serial.println();
Serial.println("==== EG800K-OTA-FreeRTOS example ====");
q_evt = xQueueCreate(4, sizeof(AppEvt));


xTaskCreatePinnedToCore(ModemTask, "ModemTask", 8192, nullptr, 3, nullptr, APP_CPU_NUM);
xTaskCreatePinnedToCore(UpdaterTask, "UpdaterTask", 8192, nullptr, 2, nullptr, APP_CPU_NUM);
}


void loop(){ vTaskDelay(pdMS_TO_TICKS(1000)); }
