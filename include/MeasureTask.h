#ifndef _MEASURETASK_H
#define _MEASURETASK_H
#include "globals.h"
#include "utils.h"
#include <Adafruit_AHTX0.h>
#include <Scheduler.h>
#include <LeanTask.h>

class MeasureTask : public LeanTask
{
private:
    /* data */
public:
    MeasureTask(bool _enabled = false, unsigned long _interval = 0) : LeanTask(_enabled, _interval) {}

protected:
    void setup()
    {
        
        Serial.println("MeasureTask setup()");
    }

    void loop()
    {
        aht.getEvent(&sysdata.hum, &sysdata.temp);
        Serial.println("Messung");
    }
};

#endif