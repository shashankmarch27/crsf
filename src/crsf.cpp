#include "crsf.h"


#if defined(ESP32)
crsf::crsf(HardwareSerial *crsf_port, int rx_pin, int tx_pin, bool inverted){
    this->inverted = inverted;
    this->crsf_port = crsf_port;
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
}

void crsf::init(){
    crsf_port->begin(CRSF_BAUDRATE,SERIAL_8N1,rx_pin,tx_pin,inverted);
}

#elif defined(ARDUINO_ARCH_RP2040)
crsf::crsf(SerialUART *crsf_port, int rx_pin, int tx_pin){
    this->crsf_port = crsf_port;
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
}

void crsf::init(){
    crsf_port->setRX(rx_pin);
    crsf_port->setTX(tx_pin);
    crsf_port->begin(CRSF_BAUDRATE,SERIAL_8N1);
}
#endif

void crsf::read(){
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
            if(buffer == CRSF_ADDRESS_FLIGHT_CONTROLLER){
                header_detected = true;
                rx_data[0] = CRSF_ADDRESS_FLIGHT_CONTROLLER;
                rx_index = 1;
            }
        }

        if(rx_index == sizeof(rx_data)/sizeof(rx_data[0])){
            rx_index = 0;
            header_detected = false;
        
        }   
    }
    memcpy(&header,rx_data,sizeof(header));

    if(header.type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED){
        memcpy(&packet,rx_data + 3,sizeof(packet));
    }
    else if(header.type == CRSF_FRAMETYPE_LINK_STATISTICS){
        memcpy(&link_status,rx_data + 3,sizeof(link_status));
    }
}

crsf_channels_t crsf::getChannel(){
    return packet;
}

crsfLinkStatistics_t crsf::getlinkStatus(){
    return link_status;
}