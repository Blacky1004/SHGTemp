#ifndef _SERVERTASK_H
#define _SERVERTASK_H
#include "globals.h"
#include "utils.h"
#include <Scheduler.h>
#include <Task.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "WifiTask.h"

String html_pocressor(const String &var)
{
    if (var == "CHIPNAME")
    {
        return syscfg.hostname;
    }

    return String();
}

class ServerTask : public Task
{

private:
    /* data */
    void setupFilePathes()
    {
        server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(LittleFS, "/ap/style.css", "text/css"); });
        server.on("/jquery-3.7.1.js", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(LittleFS, "/ap/jquery-3.7.1.js", "application/javascript"); });
        server.on("/system.js", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(LittleFS, "/www/system.js", "application/javascript"); });
    }

public:
    ServerTask(/* args */) : Task() {}

protected:
    void notFound(AsyncWebServerRequest *request)
    {
        request->send(404, "text/plain", "Not found");
    }
    void setup()
    {

        Serial.println("starting ServerTask.");
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
            String indexFile = "/ap/wifi_index.html";
            if(syscfg.hasInternet) {
                indexFile = "/www/index.html";
            }

            AsyncWebServerResponse *response = request->beginResponse(LittleFS, indexFile, String(), false, html_pocressor);
            request->send(response); });
        setupFilePathes();
        server.begin();
    }

    void loop()
    {
    }
};

#endif