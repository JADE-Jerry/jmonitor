/*
 * config.h
 *
 * 项目配置：在此填写 WiFi、SNMP 与轮询相关设置。
 * 注意：不要将包含真实密码或私密信息的仓库公开推送到公共代码托管平台。
 */
#ifndef CONFIG_H
#define CONFIG_H

// --- 本地私有配置（优先） -----------------------------------------------
// 支持在项目的 include/ 目录下创建一个不提交到 Git 的 `config_local.h`，
// 在其中定义 WIFI_SSID、WIFI_PASSWORD 等敏感项。
// 此处尝试安全地包含该文件（若编译器支持 __has_include）：
#if defined(__has_include)
	#if __has_include("config_local.h")
		#include "config_local.h"
	#endif
#endif

// --- WiFi 设置 --------
// WiFi SSID
#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_PASSWORD"
#endif

// --- 网络 / SNMP 设置 -----------------------------------------------------
// 目标设备 IP 地址（四个整数，以逗号分隔）
#ifndef IP_ADDRESS
#define IP_ADDRESS 192, 168, 1, 5
#endif

// SNMP 读取 community 字符串（通常为只读 community，例如 "public"）
#ifndef COMMUNITY
#define COMMUNITY "public"
#endif

// SNMP 版本：0 = SNMPv1, 1 = SNMPv2c
#ifndef SNMP_VERSION
#define SNMP_VERSION 1
#endif

// 轮询间隔（毫秒），表示两次 SNMP 轮询之间的延迟。例如 6000 = 6 秒
#ifndef POLL_INTERVAL
#define POLL_INTERVAL 6000
#endif

#endif // CONFIG_H
