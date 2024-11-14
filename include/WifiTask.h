#ifndef _WIFITASK_H
#define _WIFITASK_H
#include "globals.h"
#include "utils.h"
#include <Scheduler.h>
#include <Task.h>
#include <list>
#include <iostream>
#include "ESP8266Ping.h"

class WiFiTask : public Task
{
    private:
    void loadWifiNetworks()
    {
        int n = WiFi.scanNetworks();
        if (n == 0)
        {
            Serial.println("there are no Networks available!");
            syscfg.wifi_mode = WIFI_AP;
        }
        else
        {
            wifiNetworks.clear();
            for (int i = 0; i < n; i++)
            {
                wifi_network_t w;
                w.id = i;
                w.ecrytionType = WiFi.encryptionType(i);
                w.rssi = WiFi.RSSI(i);
                w.ssid = WiFi.SSID(i);
                wifiNetworks.push_back(w);
            }
        }
    }
    wl_status_t startSta()
    {
        wl_status_t result = WL_DISCONNECTED;
        try
        {
            Serial.println("using STA Mode...");
            WiFi.mode(WIFI_STA);
            WiFi.hostname(syscfg.hostname);
            Serial.printf("connecting to '%s' with password '%s'\n", syscfg.wifi_ssid, syscfg.wifi_passwd);

            result = WiFi.begin(syscfg.wifi_ssid, syscfg.wifi_passwd);
            Serial.printf("WiFi begin status %d\n", result);
            if (checkConnection(true))
            {
                Serial.printf("connected to %s\n", WiFi.SSID().c_str());
                Serial.printf("my hostname: %s\n", WiFi.getHostname());
                Serial.printf("my IP: %s\n", WiFi.localIP().toString().c_str());
                strcpy(syscfg.ipadress, WiFi.localIP().toString().c_str());
                result = WiFi.status();
            }
            else
            {
                result = startAp();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            result = startAp();
        }

        return result;
    }

    wl_status_t startAp()
    {
        wl_status_t result = WL_DISCONNECTED;
        try
        {
            WiFi.mode(WIFI_AP);
            syscfg.wifi_mode = WIFI_AP;

            if (WiFi.softAP(syscfg.wifi_ssid, syscfg.wifi_passwd))
            {
                syscfg.wifi_mode = WIFI_AP;
                strcpy(syscfg.ipadress, WiFi.softAPIP().toString().c_str());
                Serial.println("WiFi started as AccesPoint.");
                Serial.printf("My ip is: %s\n", syscfg.ipadress);
                Serial.println("please connect to this network and configurate:");
                Serial.printf("SSID: '%s'\n", syscfg.wifi_ssid);
                Serial.printf("Password: '%s'\n", syscfg.wifi_passwd);
                return WiFi.status();
            }

            Serial.println("Error while start AP mode.");

            return WL_DISCONNECTED;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        return result;
    }

    bool checkConnection(bool verbose)
    {
        int c = 0;
        while (c < 20)
        {
            if (WiFi.status() == WL_CONNECTED)
                return true;
            this->delay(500);
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

        return false;
    }

public:
    WiFiTask(bool _enabled = false, unsigned long _interval = 0) : Task(_enabled, _interval) {}
    std::list<wifi_network_t> wifiNetworks;
    wl_status_t actualConnectionStatus;
      
protected:
    void setup()
    {
        Serial.println("WifiTask setup()");
        Serial.println("loading available WiFi networks...");
        this->loadWifiNetworks();

        if (syscfg.wifi_mode == WIFI_STA)
        {
            this->actualConnectionStatus = startSta();
        }
        else
        {
            this->actualConnectionStatus = this->startAp();
        }
    }
    
    void loop()
    {
        static unsigned long ts = 0;
        if (millis() - ts > syscfg.wifi_cycle * 1000)
        {
            Serial.println("Check WiFiConnection");
            this->actualConnectionStatus = WiFi.status();
            if (actualConnectionStatus == WL_CONNECTED)
            {
                syscfg.hasInternet = Ping.ping("www.google.com");
            }

            ts = millis();
        }
    }


};
#endif