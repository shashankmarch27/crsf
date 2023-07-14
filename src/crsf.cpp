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

void crsf::read(crsfpacket_t* packet){
    while(crsf_port->available()){
        uint8_t buffer = crsf_port->read();
        if(header_detected){
            rx_data[rx_index] = buffer;
            rx_index++;
            if(rx_index > rx_data[1]){
                rx_index = 0;
                header_detected = false;
            }
        }
        else{
            if(buffer == 0xEE){
                header_detected = true;
                rx_data[0] = 0xEE;
                rx_index = 1;
            }
        }
    }
}