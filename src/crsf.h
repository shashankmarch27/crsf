#ifndef CRSF_H
#define CRSF_H
#include <Arduino.h>
#include "crsf_protocol.h"

class crsf{

private:
    int tx_pin;
    int rx_pin;

    #ifdef ESP32
    HardwareSerial *crsf_port;
    #endif

    bool inverted;

public:
    #ifdef ESP32
    crsf(HardwareSerial *crsf_port, int rx_pin, int tx_pin, bool inverted = UNINVERTED_CRSF);
    #endif

    void init();
    void read();

};

#endif