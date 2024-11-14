#ifndef _MEASUREHANDLER_H
#define _MEASUREHANDLER_H

#include "globals.h"

class MeasureHandler
{
private:
    Adafruit_AHTX0 aht;
public:
    system_data_t sysdata;
    bool init() {
        Serial.println("Initialize measurement sensor...");
        if(!aht.begin()){
            Serial.println("Eror while initializing temperature sensor!");
            return false;
        }

        Serial.println("Measurement sensor initialized.");

        return true;
    }

    void getdata() {
        aht.getEvent(&sysdata.hum, &sysdata.temp);
    }
};

#endif