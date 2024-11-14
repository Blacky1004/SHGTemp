#ifndef _DISPLAYTASK_H
#define _DISPLAYTASK_H
#include "globals.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Scheduler.h>
#include <Task.h>

class DisplayTask : public Task
{
public:
    DisplayTask(bool _enabled = false, unsigned long _interval = 0) : Task(_enabled, _interval) {}

private:
protected:
    void setup()
    {
        display.clearDisplay();
        Serial.println("DisplayTask setup()");
    }

    void loop()
    {
        static unsigned long ts = 0;

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 2);
        display.printf("%s", syscfg.hostname);
        display.setTextSize(2);
        display.setTextColor(WHITE);

        display.setCursor(0, 25);
        display.print("T");
        display.setCursor(50, 25);
        display.printf("%.1f", sysdata.temp.temperature);
        display.print(char(247));
        display.print("C");

        display.setCursor(0, 45);
        display.print("L");
        display.setCursor(50, 45);
        display.printf("%.1f", sysdata.hum.relative_humidity);
        display.print("%");
        display.display();
        yield();
    }
};
#endif