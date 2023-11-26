#include "crsf.h"

#if defined(ESP32)
crsf::crsf(HardwareSerial *crsf_port, int rx_pin, int tx_pin, bool inverted)
{
    this->inverted = inverted;
    this->crsf_port = crsf_port;
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
}

void crsf::init()
{
    crsf_port->begin(CRSF_BAUDRATE, SERIAL_8N1, rx_pin, tx_pin, inverted);
}

#elif defined(ARDUINO_ARCH_RP2040)
crsf::crsf(SerialUART *crsf_port, int rx_pin, int tx_pin)
{
    this->crsf_port = crsf_port;
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
}

void crsf::init()
{
    crsf_port->setRX(rx_pin);
    crsf_port->setTX(tx_pin);
    crsf_port->begin(CRSF_BAUDRATE, SERIAL_8N1);
}

#elif defined(STM32F4xx)
crsf::crsf(HardwareSerial *crsf_port){
    this->crsf_port = crsf_port;
}

void crsf::init(){
    crsf_port->begin(CRSF_BAUDRATE, SERIAL_8N1);
}
#endif

void crsf::read()
{
    while (crsf_port->available()){
        uint8_t buffer = crsf_port->read();
        if (header_detected){
            // rx_index = (rx_index + 1) % (rx_data[1] + 1);
            rx_data[rx_index] = buffer;
            rx_index++;
            if (rx_index > rx_data[1]+2) // packet length = len+2
            {   // whole packet received
                rx_index = 0;
                header_detected = false;
            }
        }
        else{
            if (buffer == CRSF_ADDRESS_FLIGHT_CONTROLLER || buffer == CRSF_ADDRESS_CRSF_TRANSMITTER){
                header_detected = true;
                rx_data[0] = buffer;
                rx_index = 1;
            }
        }

        if (rx_index == sizeof(rx_data) / sizeof(rx_data[0])){
            // if rx_data buffer overflow
            rx_index = 0;
            header_detected = false;
        }
    }
    // Check CRC
    if(!checkCRC()){
        header_detected = false;
        return;
    }
        memcpy(&header, rx_data, sizeof(header));
        
        if (header.device_addr == CRSF_ADDRESS_FLIGHT_CONTROLLER){

            if (header.type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED){
                memcpy(&packet, rx_data + 3, sizeof(packet));
            }
            else if (header.type == CRSF_FRAMETYPE_LINK_STATISTICS){
                Serial.print(header.type);
                Serial.print(" ");
                Serial.println(header.frame_size);
                memcpy(&link_status, rx_data + 3, sizeof(link_status));
            }

        }
        else if (header.device_addr == CRSF_ADDRESS_CRSF_TRANSMITTER){
            if (header.type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED){
                memcpy(&packet, rx_data + 3, sizeof(packet));
            }
        }
    
}

crsf_channels_t crsf::getChannel(){
    return packet;
}

crsfLinkStatistics_t crsf::getlinkStatus(){
    return link_status;
}

uint8_t gen_poly = 0xd5;

uint8_t crsf::calculateCRC(int bytes){   
    uint8_t dividend = 0;
    uint8_t next_byte;
    int StartFromByte = 2; // during crc calcualtion the header and length bytes are excluded
    int numberOfBytesProcessed = StartFromByte; 
    int numberOfBitsLeft = 0;
    bool isMsbOne = false;
    while (true)
    {

        if (numberOfBitsLeft <=0 && numberOfBytesProcessed -StartFromByte>= bytes)
        {
            // ALL BITS PROCEESSED 
            break;
        }
        if (numberOfBitsLeft <= 0 && numberOfBytesProcessed -StartFromByte< bytes)
        {
            // load bits into buffer if empty and if bits available
            next_byte = rx_data[numberOfBytesProcessed++];
            numberOfBitsLeft =8;
        }

        isMsbOne = dividend & 0b10000000;
        dividend = dividend << 1 | (next_byte>>7);   // shift First bit of Next_byte into dividend
        next_byte = next_byte << 1;  // Shift out the first bit
        numberOfBitsLeft --;
        dividend = isMsbOne ? dividend ^ gen_poly : dividend; //if bit aligning with MSB of gen_poly is 1 then do XOR 

    }

    return dividend;
}

bool crsf::checkCRC(){
    return calculateCRC(header.frame_size) == 0 ? true: false;
}