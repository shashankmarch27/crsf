#include "crsf.h"

crsf::crsf(HardwareSerial *crsf_port, int rx_pin, int tx_pin, bool inverted){
    this->inverted = inverted;
    this->crsf_port = crsf_port;
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
}

void crsf::init(){
    crsf_port->begin(BAUDRATE_CRSF,SERIAL_8N1,rx_pin,tx_pin,inverted);
}

void crsf::read(){
    while(crsf_port->available()){
        crsf_port->println(crsf_port->read());
    }
}