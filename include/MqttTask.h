#ifndef _MQTT_TASK_H
#define _MQTT_TASK_H
#include "globals.h"
#include "utils.h"
#include <ArduinoJson.h>
#include <Scheduler.h>
#include <LeanTask.h>
class MqttTask : public LeanTask
{
private:
    JsonDocument mqttDoc;
    static bool mqtt_available;
    static void callback(char *topic, byte *payload, unsigned int length)
    {
        JsonDocument mqttDoc;
        Serial.printf("Message arrived in topic: '%s'\r\n", topic);
        if (topic == "reset_mcu")
        {
            mqttDoc["id"] = syscfg.chipid;
            mqttDoc["reason"] = "Reset MCU";
            String mqstr;
            serializeJson(mqttDoc, mqstr);
            mqtt_client.publish("disconnect", mqstr.c_str());
            mqtt_client.disconnect();
            mqtt_available = false;
            ESP.restart();
        }
        else if (topic == "update_config")
        {
        }
        else if (topic == "sync_time")
        {
        }
        else
        {
            Serial.printf("Unknown topic '%s' arrived!", topic);
        }
    }
    void connectToMqttBroker()
    {
        while (!mqtt_client.connected())
        {
            Serial.println("Connecting to MQTT...");
            if (mqtt_client.connect(syscfg.hostname, syscfg.mqqt_user, syscfg.mqqt_password))
            {
                Serial.println("connected");
                syscfg.mqtt_conected = true;
                this->mqtt_available = true;
            }
            else
            {
                Serial.print("failed with state ");
                Serial.println(mqtt_client.state());
                this->mqtt_available = false;
                syscfg.mqtt_conected = false;
                this->delay(2000);
            }
        }
    }

protected:
    void setup()
    {
        Serial.println("Start MQTT-TASK setup");
        mqtt_client.setServer(syscfg.mqqt_broker, syscfg.mqqt_port);
        mqtt_client.setCallback(this->callback);
        mqtt_client.setKeepAlive(syscfg.mqtt_keepAlive);
        connectToMqttBroker();
        mqtt_client.subscribe("reset_mcu");
        mqtt_client.subscribe("update_config");
        mqtt_client.subscribe("sync_time");
        this->mqtt_available = true;
        Serial.println("MQTT TASK setup end.");
    }

    void loop()
    {
        mqttDoc["temp"] = sysdata.temp.temperature;
        mqttDoc["hum"] = sysdata.hum.relative_humidity;
        mqttDoc["id"] = syscfg.chipid;
        mqttDoc["hostname"] = syscfg.hostname;
        if (!mqtt_client.connected())
        {
            syscfg.mqtt_conected = false;
            connectToMqttBroker();
        }
        String esp;
        serializeJson(mqttDoc, esp);
        mqtt_client.publish("shgtmp_in", esp.c_str());
        mqtt_client.loop();
    }

public:
    MqttTask(bool _enabled = false, unsigned long _interval = 0) : LeanTask(_enabled, _interval) {}
    static bool mqttAvailable()
    {
        return mqtt_available;
    }
};
#endif