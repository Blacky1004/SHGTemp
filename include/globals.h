#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <Arduino.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <LittleFS.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "utils.h"
#include <list>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

extern Adafruit_SSD1306 display;
extern AsyncWebServer server;
extern Adafruit_AHTX0 aht;
extern WiFiClient espClient;
extern PubSubClient mqtt_client;

typedef struct __attribute__((packed))
{
    char version[10];
    char hostname[32];
    char ipadress[15];
    uint32_t chipid;
    char mqqt_broker[255];
    uint32_t mqqt_port;
    char mqqt_user[32];
    char mqqt_password[255];
    char mqqt_topicIn[32];
    unsigned long mqtt_intervall;
    uint16_t mqtt_keepAlive;
    uint8_t wifi_cycle;
    uint8_t measure_cycle;
    bool enable_deepsleep;
    char web_user[32];
    char web_password[255];
    char wifi_ssid[32];
    char wifi_passwd[255];
    WiFiMode wifi_mode;
    bool hasInternet;
    bool mqtt_conected;
} system_config_t;
extern system_config_t syscfg;

typedef struct
{
    sensors_event_t temp;
    sensors_event_t hum;
} system_data_t;
extern system_data_t sysdata;

typedef struct {
    public:
    int id;
    String ssid;
    int8_t rssi;
    int8_t ecrytionType;
} wifi_network_t;
extern std::list<wifi_network_t> wifiNetworks;
#endif