#include <Arduino.h>
#include "main.h"
#include <Ticker.h>
#include <Fonts/FreeSans9pt7b.h>

uint32_t chipId = 0;
system_config_t syscfg;
system_data_t sysdata;
String hostname = "";
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer server(80);
Adafruit_AHTX0 aht;

const char *mqtt_broker = "192.168.1.107"; // EMQX broker endpoint
const char *mqtt_topic = "shgtmp_in";      // MQTT topic
const char *mqtt_username = "shguser";     // MQTT username for authentication
const char *mqtt_password = "start123";    // MQTT password for authentication
const int mqtt_port = 1884;                // MQTT port (TCP)

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
JsonDocument sysDoc;
JsonDocument cfgDoc;

WiFiTask *wifiTask;
DisplayTask *displayTask;
ServerTask *serverTask;
MeasureTask *measureTask;
MqttTask *mqttTask;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getChipId() >> (40 - i)) & 0xff) << i;
  }
  syscfg.chipid = chipId;
  Serial.println("");
  Serial.println("Starting system...");

  bool fsIsOk = false;
  fsIsOk = LittleFS.begin();

  if (!fsIsOk)
  {
    Serial.println("Format filesystem...");
    LittleFS.format();
    fsIsOk = LittleFS.begin();
  }

  if (!fsIsOk)
  {
    Serial.println("Filesystem are not initiailzed! System halted.");
    return;
  }

  // Systemconfig laden
  syscfg.chipid = chipId;
  syscfg.enable_deepsleep = false;
  syscfg.measure_cycle = 30;
  strcpy(syscfg.mqqt_broker, "192.168.1.107");
  strcpy(syscfg.mqqt_password, "Sphinx7!");
  syscfg.mqqt_port = 1884;
  strcpy(syscfg.mqqt_topicIn, "shgtmp_in");
  strcpy(syscfg.ipadress, "0.0.0.0");
  strcpy(syscfg.hostname, "");
  strcpy(syscfg.mqqt_user, "shguser");
  strcpy(syscfg.web_password, "admin");
  strcpy(syscfg.web_user, "admin");
  strcpy(syscfg.wifi_ssid, "");
  strcpy(syscfg.wifi_passwd, "");
  syscfg.hasInternet = false;
  syscfg.wifi_cycle = 10;
  syscfg.mqtt_conected = false;
  syscfg.mqtt_intervall = 300000;
  syscfg.mqtt_keepAlive = 30000;
  
  hostname = std::move("Zimmer-" + String(chipId));
  strcpy(syscfg.hostname, hostname.c_str());

  File sysFile = LittleFS.open("/var/system.json", "r");
  if (sysFile)
  {
    String sysContent = "";
    while (sysFile.available())
    {
      sysContent += char(sysFile.read());
    }
    sysFile.close();

    if (sysContent != "")
    {
      DeserializationError jerr = deserializeJson(sysDoc, sysContent);
      if (jerr)
      {
        Serial.println(jerr.c_str());
      }
      else
      {
        strcpy(syscfg.mqqt_broker, sysDoc["mqtt_server"].as<String>().c_str());
        strcpy(syscfg.mqqt_user, sysDoc["mqtt_user"].as<String>().c_str());
        strcpy(syscfg.mqqt_password, sysDoc["mqtt_password"].as<String>().c_str());
        strcpy(syscfg.mqqt_topicIn, sysDoc["mqtt_topic"].as<String>().c_str());
        strcpy(syscfg.hostname, sysDoc["hostename"].as<String>().c_str());
        strcpy(syscfg.web_password, sysDoc["web_password"].as<String>().c_str());
        strcpy(syscfg.web_user, sysDoc["web_user"].as<String>().c_str());
        syscfg.enable_deepsleep = sysDoc["enable_deepsleep"].as<bool>();
        syscfg.wifi_cycle = sysDoc["wifi_cycle"].as<uint8_t>();
        syscfg.measure_cycle = sysDoc["temp_cycle"].as<uint8_t>();
        syscfg.mqqt_port = sysDoc["mqtt_port"].as<uint32_t>();
        syscfg.mqtt_intervall =sysDoc["mqtt_interval"].as<unsigned long>();
        syscfg.mqtt_keepAlive = sysDoc["mqtt_keep_alive"].as<uint16_t>();
        if(syscfg.mqqt_port == 0){
          syscfg.mqqt_port = 1884;
        }
        if(syscfg.mqtt_intervall == 0) {
          syscfg.mqtt_intervall = 300000;
        }
        if(syscfg.mqtt_keepAlive == 0) {
          syscfg.mqtt_keepAlive = 30000;
        }
      }
    }
  }

  File cfgFile = LittleFS.open("/var/wifisetup.json", "r");
  if (!cfgFile)
  {
    Serial.println("Wifi configuration not found. Create default.");
    syscfg.wifi_mode = WIFI_AP;
    strcpy(syscfg.wifi_ssid, hostname.c_str());
    strcpy(syscfg.wifi_passwd, genreateRandomString(10).c_str());
  }
  else
  {
    Serial.println("WiFi configuration found. Read setup....");
    String content = "";
    while (cfgFile.available())
    {
      content += char(cfgFile.read());
    }
    cfgFile.close();
    Serial.println("WiFi configuration setup readed.");
    if (content != "")
    {
      DeserializationError jerr = deserializeJson(cfgDoc, content);
      if (jerr)
        Serial.println(jerr.c_str());
      if (cfgDoc["ssid"] == "" || cfgDoc["password"] == "")
      {
        syscfg.wifi_mode = WIFI_AP;
      }
      else
      {
        strcpy(syscfg.wifi_ssid, cfgDoc["ssid"].as<String>().c_str());
        strcpy(syscfg.wifi_passwd, cfgDoc["password"].as<String>().c_str());
        syscfg.wifi_mode = WIFI_STA;
      }
    }
    else
    {
      syscfg.wifi_mode = WIFI_AP;
    }
  }
  if (!aht.begin())
  {
    Serial.println("Could not find AHT? Check wiring");
    return;
  }
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.drawBitmap(0, 20, image_shg_logo, 128, 44, 1);
  display.display();
  delay(3500);

  measureTask = new MeasureTask(true, 30000);
  Scheduler.start(measureTask);

  displayTask = new DisplayTask(true, 10);
  Scheduler.start(displayTask);

  wifiTask = new WiFiTask(true, 10);
  Scheduler.start(wifiTask);

  mqttTask = new MqttTask(true, syscfg.mqtt_intervall);
  Scheduler.start(mqttTask);

  serverTask = new ServerTask();
  Scheduler.start(serverTask);
  Serial.println("System started. Enjoy ;)");
  Scheduler.begin();
}

void loop()
{
}