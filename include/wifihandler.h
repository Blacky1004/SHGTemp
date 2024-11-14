#ifndef _WIFIHANDLER_H
#define _WIFIHANDLER_H
#include "globals.h"
#include "ArduinoJson.h"
#include <list>
#include <iostream>
#include <Ticker.h>
#include <ESP8266Ping.h>

class WiFiHandler
{
private:
    typedef struct
    {
    public:
        int id;
        String ssid;
        int8_t rssi;
        int8_t encryptionType;
    } wifi_network_t;

    String mySSID = "";
    JsonDocument cfgDoc;

    bool check_wifi(bool verbose = false)
    {
        bool result = false;
        int c = 0;
        while (c < 20)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                return true;
            }

            delay(500);
            if (verbose)
            {
                Serial.print(".");
            }
            c++;
        }
        if (verbose)
        {
            Serial.println(" failed!");
        }
        return result;
    }

    wl_status_t startSta()
    {
        wl_status_t status = WL_DISCONNECTED;
        try
        {
            Serial.println("using STA mode...");
            WiFi.mode(WIFI_STA);
            WiFi.hostname(mySSID);
            Serial.printf("connect to %s with password '%s'\n", this->wifi_ssid, this->wifi_password);

            status = WiFi.begin(this->wifi_ssid, this->wifi_password);
            if (check_wifi(true))
            {
                Serial.printf("connected to %s\n", WiFi.SSID().c_str());
                Serial.printf("my hostname: %s\n", WiFi.getHostname());
                Serial.printf("my IP: %s\n", WiFi.localIP().toString().c_str());
                status = WiFi.status();
            }
            else
            {
                status = startAP();
            }
        }
        catch (const std::exception &e)
        {
            Serial.printf("ERROR: %s\n", e.what());
            status = startAP();
        }

        return status;
    }

    wl_status_t startAP()
    {
        WiFi.mode(WIFI_AP);
        this->wifi_mode = WIFI_AP;
        strcpy(this->hostname, mySSID.c_str());
        strcpy(this->wifi_ssid, mySSID.c_str());
        strcpy(this->wifi_password, genreateRandomString(10).c_str());
        if (WiFi.softAP(mySSID, this->wifi_password))
        {
            this->wifi_mode = WIFI_AP;
            strcpy(this->IPAddress, WiFi.softAPIP().toString().c_str());
            Serial.println("WiFi started as AccesPoint.");
            Serial.printf("My ip is: %s\n", this->IPAddress);
            Serial.println("please connect to this network and configurate:");
            Serial.printf("SSID: '%s'\n", this->wifi_ssid);
            Serial.printf("Password: '%s'\n", this->wifi_password);
            return WiFi.status();
        }

        Serial.println("Error while start AP mode.");

        return WL_DISCONNECTED;
    }

    void load_WiFiNetworks()
    {
        int n = WiFi.scanNetworks();
        if (n == 0)
        {
            Serial.println("there are no Networks available!");
            this->wifi_mode = WIFI_AP;
        }
        else
        {
            networkList.clear();
            for (int i = 0; i < n; i++)
            {
                wifi_network_t w;
                w.id = i;
                w.encryptionType = WiFi.encryptionType(i);
                w.rssi = WiFi.RSSI(i);
                w.ssid = WiFi.SSID(i);
                networkList.push_back(w);
            }
        }
    }
public:
    char wifi_ssid[32];
    char wifi_password[255];
    std::list<wifi_network_t> networkList;
    WiFiMode_t wifi_mode;
    wl_status_t aktualStatus;
    bool has_inet = false;
    char hostname[32];
    char IPAddress[15];    
    WiFiHandler() 
    {
      
    };
    ~WiFiHandler() {};
    wl_status_t connect(void)
    {
        try
        {
            Serial.println("initialize WiFi handler...");

            cfgDoc["ssid"] = "";
            cfgDoc["password"] = "";
            mySSID = std::move("SHG-" + String(syscfg.chipid));
            load_WiFiNetworks();
            Serial.println("check for local wifi configuration....");
            File cfgFile = LittleFS.open("/wifisetup.json", "r");
            if (!cfgFile)
            {
                Serial.println("Wifi configuration not found. Create default.");
                this->wifi_mode = WIFI_AP;
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
                        this->wifi_mode = WIFI_AP;
                    }
                    else
                    {
                        strcpy(this->wifi_ssid, cfgDoc["ssid"].as<String>().c_str());
                        strcpy(this->wifi_password, cfgDoc["password"].as<String>().c_str());
                        this->wifi_mode = WIFI_STA;
                    }
                }
                else
                {
                    this->wifi_mode = WIFI_AP;
                    mySSID = std::move("SHG-" + String(syscfg.chipid));
                }
            }
            Serial.printf("found %d WiFi networks.\n", this->networkList.size());
            Serial.println("starting WiFi...");
            if (this->wifi_mode == WIFI_STA)
            {
                this->aktualStatus = startSta();
            }
            else
            {
                this->aktualStatus = startAP();
            }
            Serial.println("WiFi handler initialized.");
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        return this->aktualStatus;
    }

    wl_status_t re_connect(char *ssid, char *password)
    {
        wl_status_t result = WL_DISCONNECTED;
        if (WiFi.status() == WL_CONNECTED)
        {
            WiFi.disconnect();
        }

        return result;
    }

    void ActualizeWiFiNetwotks()
    {
        this->load_WiFiNetworks();
    }
    
    void loop(void){
        this->aktualStatus = WiFi.status();
        if(this->aktualStatus == WL_CONNECTED) {
            this->has_inet = Ping.ping("www.google.de");
        }
    }
protected:
    void SaveWiFiConfig(JsonDocument doc, bool reload = false)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            WiFi.disconnect();
            delay(100);
            this->cfgDoc = doc;
            try
            {
                String content = "";
                serializeJsonPretty(cfgDoc, content);
                File cfgFile = LittleFS.open("/wifisetup.json", "w");
                if (!cfgFile)
                {
                    throw std::runtime_error("error while opening '/wifisetup.json' for output!");
                }

                int bytesWritten = cfgFile.print(content);
                if (bytesWritten > 0)
                {
                    Serial.println("WiFi configuration successfully written.");
                }
                else
                    throw std::runtime_error("error while writing to '/wifisetup.json!");
                cfgFile.close();
                if (reload)
                {
                    strcpy(this->wifi_ssid, cfgDoc["ssid"].as<String>().c_str());
                    strcpy(this->wifi_password, cfgDoc["password"].as<String>().c_str());
                    re_connect(this->wifi_ssid, this->wifi_password);
                }
            }
            catch (const std::exception &e)
            {
                Serial.printf("WiFi ERROR: %s\n", e.what());
            }
        }
    }
};

#endif