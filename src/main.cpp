#include <Arduino.h>

#if defined (ESP8266)
#include <ESP8266WiFi.h>        // ESP8266 Core WiFi Library         
#else
#include <WiFi.h>               // ESP32 Core WiFi Library    
#endif

#include <WiFiUdp.h>
#include <Arduino_SNMP_Manager.h>

#include <TFT_eSPI.h>

#include "config.h"

//************************************
//* WiFi Settings                    *
//************************************
const char* ssid = WIFI_SSID; // WiFi SSID
const char* password = WIFI_PASSWORD; // WiFi Password

//************************************
//* SNMP Device Info - Synology      *
//************************************
IPAddress router(IP_ADDRESS); // 目标设备IP地址
const char* community = COMMUNITY; // SNMP团体字符串
const int snmpVersion = SNMP_VERSION; // SNMP Version 1 = 0, SNMP Version 2 = 1
unsigned int pollInterval = POLL_INTERVAL; // 6s, delay in milliseconds
// 输入/输出 OID 列表（按需修改每个OID的接口索引）
static const char* oidInOctetsCount64[] = {
  ".1.3.6.1.2.1.31.1.1.1.6.7" // ifInOctets.7
  // 如需监控多个接口，可以添加更多，例如：
  // ".1.3.6.1.2.1.31.1.1.1.6.8",
};
static const char* oidOutOctetsCount64[] = {
  ".1.3.6.1.2.1.31.1.1.1.10.7" // ifOutOctets.7
  // ".1.3.6.1.2.1.31.1.1.1.10.8",
};
static const int IN_OID_COUNT = sizeof(oidInOctetsCount64) / sizeof(oidInOctetsCount64[0]);
static const int OUT_OID_COUNT = sizeof(oidOutOctetsCount64) / sizeof(oidOutOctetsCount64[0]);
const char* oidIfSpeed = ".1.3.6.1.2.1.31.1.1.1.15.7";          // Gauge32 ifSpeed.7
const char* oidSysName = ".1.3.6.1.2.1.1.5.0";              // OctetString SysName e.g. jade-dc
const char* oidUptime = ".1.3.6.1.2.1.1.3.0";               // TimeTicks uptime (hundredths of seconds)
const char* oidCpuLoad = ".1.3.6.1.4.1.6574.1.7.1.0";               // INTEGER CPU Load
const char* oidMemLoad = ".1.3.6.1.4.1.6574.1.7.2.0";               // INTEGER Mem            

// Variables
// 每个OID对应一个响应槽，后续汇总为总量
uint64_t inOctetsResponses[IN_OID_COUNT] = {0};
uint64_t outOctetsResponses[OUT_OID_COUNT] = {0};
uint64_t lastInOctets = 0;  // 上次总和
uint64_t lastOutOctets = 0; // 上次总和
char sysName[65] = {0};
char* sysNameResponse = sysName;
uint32_t uptime = 0;
uint32_t lastUptime = 0;
int cpuLoadResponse = 0;
int memLoadResponse = 0;

// SNMP Objects
WiFiUDP udp;                                           // UDP object used to send and receive packets
SNMPManager snmp = SNMPManager(community);             // Starts an SNMPManager to listen to replies to get-requests
SNMPGet snmpRequest = SNMPGet(community, snmpVersion); // Starts an SNMPGet instance to send requests
// 回调指针数组：每个OID对应一个回调
ValueCallback* callbackInOctets[IN_OID_COUNT];
ValueCallback* callbackOutOctets[OUT_OID_COUNT];
ValueCallback* callbackSysName;
ValueCallback* callbackUptime;
ValueCallback* callbackCpuLoad;
ValueCallback* callbackMemLoad;
//************************************



#define TFT_ROTATION 0
#define TFT_BL 5  // 背光控制引脚
#define TFT_BRIGHTNESS 240 // 背光亮度，0-255
// 色卡
#define COLOR_BLACK     0x0000    // 纯黑
#define COLOR_WHITE     0xFFFF    // 纯白
#define COLOR_LIGHT_GRAY 0xBDF7   // 浅灰色
#define COLOR_DARK_GRAY  0x7BEF   // 深灰色
#define COLOR_RED       0xF800    // 红色
#define COLOR_YELLOW    0xFFE0    // 黄色
#define COLOR_ORANGE    0xFBE0    // 橙色
#define COLOR_BROWN     0x79E0    // 棕色
#define COLOR_GREEN     0x07E0    // 绿色
#define COLOR_CYAN      0x07FF    // 青色
#define COLOR_BLUE      0x001F    // 蓝色
#define COLOR_PINK      0xF81F    // 粉色

// Theme Colors
#define BG_TITLE       0x040F  // 深蓝（标题栏背景）
#define BG_MAIN        0x0813  // 深空蓝（主背景）
// #define BG_MAIN        TFT_BLACK  // 深空蓝（主背景）
#define BG_MODULE      0x1023
#define COLOR_BORDER   COLOR_DARK_GRAY
#define TEXT_MAIN      COLOR_WHITE
#define TEXT_SUB       COLOR_LIGHT_GRAY
#define COLOR_PRIMARY   BG_MAIN
#define COLOR_SUCCESS   COLOR_GREEN
#define COLOR_WARNING   COLOR_YELLOW
#define COLOR_ERROR     COLOR_RED
#define COLOR_INFO      COLOR_LIGHT_GRAY

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite clk = TFT_eSprite(&tft);


//************************************
//* Function declarations            *
//************************************
void getSNMP();
void doSNMPCalculations();
void printVariableValues();
void showMessage(String message);
void drawSystemUI();
void sendSNMPRequest(IPAddress target, struct device* deviceRecord);
void printVariableValues();
//************************************

// 系统监控数据
struct SystemStatus {
  float cpu_usage;    // CPU使用率 (%)
  float mem_usage;    // 内存使用率 (%)
  uint64_t net_ins[50];      // 网络下行速率 B/s
  uint64_t net_outs[50];      // 网络上行速率 B/s
  String sys_state;   // 系统整体状态（正常/警告/错误）
  uint16_t state_color; // 系统状态对应颜色
} sys;

// ===================== 3. 核心工具函数 =====================
// 根据使用率获取负载颜色
uint16_t getLoadColor(float usage) {
  if (usage >= 80) return COLOR_ERROR;
  else if (usage >= 60) return COLOR_WARNING;
  else return COLOR_SUCCESS;
}

void connectWifi() {
  showMessage("Connecting...");
  Serial.print("Connecting.");
  WiFi.begin(ssid, password);
  int connectTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && connectTimeout < 60) { // 最多等待30秒
    delay(500);
    Serial.print(".");
    connectTimeout++;
  }

  // 连接结果判断
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nConnected to %s.\nIP address: %s\n", ssid, WiFi.localIP().toString().c_str());

    tft.fillScreen(BG_MAIN);
    tft.setTextColor(TEXT_MAIN);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Wifi connected", 120, 80);
    tft.setTextSize(2);
    tft.drawString(String("SSID: ") + WiFi.SSID(), 120, 120);
    tft.drawString(String("IP: ") + WiFi.localIP().toString(), 120, 140);

    delay(3000);
  } else {
    Serial.println("\nFailed to connect to WiFi.");
    showMessage("Failed to connect.");
  }

}

void setup() {
  Serial.begin(115200);

  // ----- 初始化TFT-----
  // 初始化背光
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, TFT_BRIGHTNESS); // 亮度，0-255, 0亮度最大
  tft.init();
  tft.setRotation(TFT_ROTATION);
  tft.fillScreen(BG_MAIN);
  tft.setTextWrap(false); // 关闭文字自动换行
}

static bool snmpInit = false;
static unsigned long lastUpdate = 0;
void loop() {
  if(WiFi.status() != WL_CONNECTED) {
    snmpInit = false;

    // ----- 连接WiFi-----
    connectWifi();
    if(WiFi.status() == WL_CONNECTED) {
      tft.fillScreen(BG_MAIN);
    } else {
      delay(30 * 1000); // 30秒后重试
    }
  } else {
    if (!snmpInit) {
      // ----- 初始化SNMP-----
      // snmp初始化
      snmp.setUDP(&udp); // give snmp a pointer to the UDP object
      snmp.begin();      // start the SNMP Manager
      // Get callbacks from creating a handler for each of the OID
      // 为每个输入OID创建Counter64回调
      for (int i = 0; i < IN_OID_COUNT; i++) {
        callbackInOctets[i] = snmp.addCounter64Handler(router, oidInOctetsCount64[i], &inOctetsResponses[i]);
      }
      // 为每个输出OID创建Counter64回调
      for (int i = 0; i < OUT_OID_COUNT; i++) {
        callbackOutOctets[i] = snmp.addCounter64Handler(router, oidOutOctetsCount64[i], &outOctetsResponses[i]);
      }
      callbackSysName = snmp.addStringHandler(router, oidSysName, &sysNameResponse);
      callbackUptime = snmp.addTimestampHandler(router, oidUptime, &uptime);
      callbackCpuLoad = snmp.addIntegerHandler(router, oidCpuLoad, &cpuLoadResponse);
      callbackMemLoad = snmp.addIntegerHandler(router, oidMemLoad, &memLoadResponse);

      snmpInit = true;
    }

    snmp.loop();
    if (millis() - lastUpdate >= pollInterval || millis() < lastUpdate) { // 定时更新，考虑millis()溢出情况
      getSNMP();
      doSNMPCalculations();
      drawSystemUI();

      lastUpdate = millis();
    }
  }
}

void showMessage(String message) {
  tft.fillScreen(BG_MAIN);
  tft.setTextColor(TEXT_MAIN);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(message, 120, 120);
}

// ===================== 4. 界面绘制函数 =====================
void drawSystemUI() {
  // 1. 绘制标题栏
  if (sys.cpu_usage >= 80 || sys.mem_usage >= 80) {
    sys.state_color = COLOR_ERROR;
  } else if (sys.cpu_usage >= 50 || sys.mem_usage >= 50) {
    sys.state_color = COLOR_WARNING;
  } else {
    sys.state_color = COLOR_SUCCESS;
  }
  tft.fillCircle(20, 20, 10, sys.state_color); // 系统状态指示灯
  tft.setFreeFont(&FreeMono9pt7b); // 设置英文字体
  tft.setTextColor(TEXT_MAIN);
  tft.setTextDatum(ML_DATUM); // 左中对齐
  tft.fillRect(40, 0, 200, 44, BG_MAIN);
  tft.drawString(sysNameResponse, 40, 20);
  memset(sysName, 0, sizeof(sysName)); // 清空字符串缓冲区

  tft.drawLine(0, 45, 240, 45, COLOR_BORDER); // 分割线

  // 2. 绘制CPU模块（左上）
  tft.setTextFont(1);
  tft.setTextSize(2);
  tft.setTextColor(TEXT_MAIN);
  tft.drawString("CPU", 10, 65);
  tft.fillRect(60, 55, 60, 20, BG_MAIN);
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(getLoadColor(sys.cpu_usage));
  tft.drawString(String(sys.cpu_usage, 0) + "%", 110, 65);
  tft.fillRect(10, 80, 100, 20, COLOR_BORDER);
  tft.fillRect(10, 80, sys.cpu_usage, 20, getLoadColor(sys.cpu_usage));

  // 3. 绘制内存模块（右上）
  tft.setTextSize(2);
  tft.setTextColor(TEXT_MAIN);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("MEM", 130, 65);
  tft.fillRect(175, 55, 60, 20, BG_MAIN);
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(getLoadColor(sys.mem_usage));
  tft.drawString(String(sys.mem_usage, 0) + "%", 230, 65);
  tft.fillRect(130, 80, 100, 20, COLOR_BORDER);
  tft.fillRect(130, 80, sys.mem_usage, 20, getLoadColor(sys.mem_usage));

  // 4. 绘制网络模块（下方）
  // 4a. 实时速率显示
  String measureUnit;
  float ratio = 1;
  if (sys.net_ins[49] >= 1024 * 1024 * 1024 || sys.net_outs[49] >= 1024 * 1024 * 1024) {
    measureUnit = "GB/s";
    ratio = 1024 * 1024 * 1024;
  } else if (sys.net_ins[49] >= 1024 * 1024 || sys.net_outs[49] >= 1024 * 1024) {
    measureUnit = "MB/s";
    ratio = 1024 * 1024;
  } else {
    measureUnit = "KB/s";
    ratio = 1024;
  }

  uint32_t outColor = COLOR_CYAN;
  uint32_t inColor = COLOR_ORANGE;

  tft.fillRect(0, 220, 240, 20, BG_MAIN); // 清除区域
  tft.fillTriangle(10, 230, 15, 220, 20, 230, outColor); // 上传箭头
  tft.setTextColor(outColor);
  tft.setCursor(30, 220);
  tft.setTextSize(2);
  tft.print(String(sys.net_outs[49] / ratio, 1));
  tft.setTextSize(1);
  tft.print(" " + measureUnit);
  tft.fillTriangle(130, 220, 135, 230, 140, 220, inColor); // 下载箭头
  tft.setTextColor(inColor);
  tft.setCursor(150, 220);
  tft.setTextSize(2);
  tft.print(String(sys.net_ins[49] / ratio, 1));
  tft.setTextSize(1);
  tft.print(" " + measureUnit);

  // 4b. 绘制网络流量图表
  // 确定量纲
  uint64_t max_in = 0;
  uint64_t max_out = 0;
  for (int i = 0; i < 50; i++) {
    if (sys.net_ins[i] > max_in) max_in = sys.net_ins[i];
    if (sys.net_outs[i] > max_out) max_out = sys.net_outs[i];
  }
  String measureUnit2; // 更高一级单位
  if (max_in >= 1024 * 1024 * 1024 || max_out >= 1024 * 1024 * 1024) {
    measureUnit = "GB/s";
    measureUnit2 = "TB/s";
    ratio = 1024 * 1024 * 1024;
  } else if (max_in >= 1024 * 1024 || max_out >= 1024 * 1024) {
    measureUnit = "MB/s";
    measureUnit2 = "GB/s";
    ratio = 1024 * 1024;
  } else {
    measureUnit = "KB/s";
    measureUnit2 = "MB/s";
    ratio = 1024;
  }
  // 确定缩放因子
  int zoom_factor = 1;
  if(max_in / ratio > 500 || max_out / ratio > 500) {
    zoom_factor = 1;
  } else if(max_in / ratio > 100 || max_out / ratio > 100) {
    zoom_factor = 2;
  } else if (max_in / ratio > 50 || max_out / ratio > 50) {
    zoom_factor = 10;
  } else if (max_in / ratio > 10 || max_out / ratio > 10) {
    zoom_factor = 20;
  } else {
    zoom_factor = 100;
  }

  tft.fillRect(0, 106, 240, 114, BG_MAIN);
  tft.drawFastHLine(20, 210, 200, COLOR_BORDER); // X轴
  tft.drawFastVLine(20, 110, 100, COLOR_BORDER); // Y轴
  // Y轴刻度线和标签
  for (int i = 1; i < 5; i++) {
    int y = 210 - i * 20;
    tft.drawFastHLine(20 - 4, y, 4, COLOR_BORDER);
  }
  tft.drawFastHLine(20 - 8, 110, 15, COLOR_BORDER);

  // 垂直标识刻度值
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TEXT_SUB);
  tft.setTextSize(2);
  tft.drawString(zoom_factor == 1 ? measureUnit2 : String(1000 / zoom_factor) + measureUnit, 30, 116);
  // 水平标识刻度值
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TEXT_SUB);
  tft.setTextSize(2);
  tft.drawString(String(pollInterval * 50 / 60000) + " mins", 230, 116);

  // 绘制网络流量图表
  int height = 100;
  for (int i = 0; i < 49; i++) {
    tft.drawLine(20 + 4 * i, 210 - sys.net_ins[i] / ratio / 1024 * height * zoom_factor, 20 + 4 * (i + 1), 210 - sys.net_ins[i + 1] / ratio / 1024 * height * zoom_factor, inColor); //in
    tft.drawLine(20 + 4 * i, 210 - sys.net_outs[i] / ratio / 1024 * height * zoom_factor, 20 + 4 * (i + 1), 210 - sys.net_outs[i + 1] / ratio / 1024 * height * zoom_factor, outColor); //out
  }
}

void doSNMPCalculations() {
  printVariableValues();
  if (uptime == lastUptime) {
    Serial.println("--Data not updated between polls");
    return;
  } else if (uptime < lastUptime) { // Check if device has rebooted which will reset counters
    Serial.println("Uptime < lastUptime. Device restarted?");
    sys = {}; // Reset system status
  } else {
    uint64_t net_in = 0; // in B/s
    uint64_t net_out = 0; // in B/s
    // 汇总所有接口的计数为总量
    uint64_t totalIn = 0;
    uint64_t totalOut = 0;
    for (int i = 0; i < IN_OID_COUNT; i++) totalIn += inOctetsResponses[i];
    for (int i = 0; i < OUT_OID_COUNT; i++) totalOut += outOctetsResponses[i];
    if (lastInOctets != 0 && lastOutOctets != 0) {
      net_in = (float)(totalIn - lastInOctets) / (float)(uptime - lastUptime) * 100; // in B/s
      net_out = (float)(totalOut - lastOutOctets) / (float)(uptime - lastUptime) * 100; // in B/s
    }
    for(int i = 0; i < 49; i++) {
      sys.net_ins[i] = sys.net_ins[i+1];
      sys.net_outs[i] = sys.net_outs[i+1];
    }
    sys.net_ins[49] = net_in;
    sys.net_outs[49] = net_out;
    Serial.printf("Net In: %llu bps, Net Out: %llu bps\n", net_in, net_out);
    sys.cpu_usage = cpuLoadResponse;
    sys.mem_usage = memLoadResponse;
  }
  // 更新上次总量为本次汇总总和
  uint64_t sumIn = 0;
  uint64_t sumOut = 0;
  for (int i = 0; i < IN_OID_COUNT; i++) sumIn += inOctetsResponses[i];
  for (int i = 0; i < OUT_OID_COUNT; i++) sumOut += outOctetsResponses[i];
  lastInOctets = sumIn;
  lastOutOctets = sumOut;
  lastUptime = uptime;
}

void getSNMP() {
  // 添加所有输入 OID 回调指针
  for (int i = 0; i < IN_OID_COUNT; i++) {
    snmpRequest.addOIDPointer(callbackInOctets[i]);
  }
  // 添加所有输出 OID 回调指针
  for (int i = 0; i < OUT_OID_COUNT; i++) {
    snmpRequest.addOIDPointer(callbackOutOctets[i]);
  }
  snmpRequest.addOIDPointer(callbackSysName);
  snmpRequest.addOIDPointer(callbackUptime);
  snmpRequest.addOIDPointer(callbackCpuLoad);
  snmpRequest.addOIDPointer(callbackMemLoad);

  snmpRequest.setIP(WiFi.localIP()); // IP of the listening MCU
  snmpRequest.setPort(161);  // Default is UDP port 161 for SNMP. But can be overriden if necessary.
  snmpRequest.setUDP(&udp);
  snmpRequest.setRequestID(rand() % 5555);
  snmpRequest.sendTo(router);
  snmpRequest.clearOIDList();
}

void printVariableValues() {
  Serial.println("----------------------");
  Serial.printf("sysName: %s\n", sysNameResponse);
  Serial.printf("CPU Load: %d%%\n", cpuLoadResponse);
  Serial.printf("Mem Load: %d%%\n", memLoadResponse);
  Serial.printf("Uptime: %d / 10ms\n", uptime);
}
