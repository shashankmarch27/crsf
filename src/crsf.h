#ifndef CRSF_H
#define CRSF_H
#include <Arduino.h>
#include "crsf_protocol.h"

class crsf
{

private:
    int tx_pin;
    int rx_pin;

    bool header_detected;
    uint8_t rx_index;
    uint8_t rx_data[CRSF_MAX_PACKET_SIZE];

#ifdef ESP32
    HardwareSerial *crsf_port;

#elif defined(ARDUINO_ARCH_RP2040)
    SerialUART *crsf_port;
#endif

    bool inverted;

    crsf_channels_t packet;
    crsfLinkStatistics_t link_status;
    crsf_header_t header;

public:
#ifdef ESP32
    crsf(HardwareSerial *crsf_port, int rx_pin, int tx_pin, bool inverted = UNINVERTED_CRSF);

#elif defined(ARDUINO_ARCH_RP2040)
    crsf(SerialUART *crsf_port, int rx_pin, int tx_pin);
#endif

    void init();
    void read();

    crsf_channels_t getChannel();
    crsfLinkStatistics_t getlinkStatus();
    uint8_t calculateCRC(int bytes);
    bool checkCRC();
};

#endif