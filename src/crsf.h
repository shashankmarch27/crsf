#ifndef CRSF_H
#define CRSF_H
#include <Arduino.h>
#include "crsf_protocol.h"

class crsf{

private:
    int tx_pin;
    int rx_pin;

    bool header_detected;
    uint8_t rx_index;
    uint8_t rx_data[64];

    #ifdef ESP32
    HardwareSerial *crsf_port;
    #endif

    bool inverted;

public:
    #ifdef ESP32
    crsf(HardwareSerial *crsf_port, int rx_pin, int tx_pin, bool inverted = UNINVERTED_CRSF);
    #endif

    void init();
    void read(crsfpacket_t* packet);

};

#endif