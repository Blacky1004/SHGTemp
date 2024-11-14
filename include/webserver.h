#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include "globals.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

enum webserver_type_t {APSERVER, STASERVER};

void webserver_init(webserver_type_t serverType);

#endif