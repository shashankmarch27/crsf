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
#endif

void crsf::read()
{
    while (crsf_port->available())
    {
        uint8_t buffer = crsf_port->read();

        if (header_detected)
        {
            rx_data[rx_index] = buffer;
            rx_index++;
            if (rx_index > rx_data[1])
            {
                rx_index = 0;
                header_detected = false;
            }
        }
        else
        {
            if (buffer == CRSF_ADDRESS_FLIGHT_CONTROLLER || buffer == CRSF_ADDRESS_CRSF_TRANSMITTER)
            {
                header_detected = true;
                rx_data[0] = buffer;
                rx_index = 1;
            }
        }

        if (rx_index == sizeof(rx_data) / sizeof(rx_data[0]))
        {
            rx_index = 0;
            header_detected = false;
        }
    }

    memcpy(&header, rx_data, sizeof(header));

    if (header.device_addr == CRSF_ADDRESS_FLIGHT_CONTROLLER)
    {
        if (header.type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED)
        {
            memcpy(&packet, rx_data + 3, sizeof(packet));
        }
        else if (header.type == CRSF_FRAMETYPE_LINK_STATISTICS)
        {
            memcpy(&link_status, rx_data + 3, sizeof(link_status));
        }
    }
    else if (header.device_addr == CRSF_ADDRESS_CRSF_TRANSMITTER)
    {
        if (header.type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED)
        {
            memcpy(&packet, rx_data + 3, sizeof(packet));
        }
    }
}

crsf_channels_t crsf::getChannel()
{
    return packet;
}

crsfLinkStatistics_t crsf::getlinkStatus()
{
    return link_status;
}

uint16_t gen_poly = 0b0000000100110001; //x8 + x5 + x4 + 1
uint16_t crsf::calculateCRC(uint8_t *rx_data, int bytes)
{
    uint16_t temp = gen_poly;
    int ActualMsbLocation = -1;
    while(temp!=0)
    {
        temp = temp>>1;
        ActualMsbLocation++;
    }
    
    if (ActualMsbLocation>=16 | ActualMsbLocation <0)
    {
        return 0; // errror
    }
    uint16_t dividend = 0;
    uint8_t next_byte;
    int numberOfBytesProcessed = 0;
    int numberOfBitsLeft = 0;
    while (true)
    {

        if (numberOfBitsLeft <=0 && numberOfBytesProcessed >= bytes)
        {
            // ALL BITS PROCEESSED 
            break;
        }
        if (numberOfBitsLeft <= 0 && numberOfBytesProcessed < bytes)
        {
            // load bits into buffer if empty and if bits available
            next_byte = rx_data[numberOfBytesProcessed++];
            numberOfBitsLeft =8;
        }

        
        dividend = dividend << 1 | (next_byte>>7);   // shift First bit of Next_byte into dividend
        next_byte = next_byte << 1;  // Shift out the first bit
        numberOfBitsLeft --;
        dividend = (dividend & 1<<ActualMsbLocation) ? dividend ^ gen_poly : dividend; //if bit aligning with MSB of gen_poly is 1 then do XOR 
        
    }

    return dividend;
}

bool crsf::checkCRC(uint8_t *rx_data, int bytes)
{
}