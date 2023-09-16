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

uint16_t gen_poly = 0b0000000100000111;
uint16_t crsf::calculateCRC(uint8_t *rx_data, int bytes)
{
    uint16_t divident = 0;
    uint8_t next_byte;
    int numberOfBytesProcessed = 0;
    int numberOfBitsProcessed = 0;
    while (true)
    {

        if (numberOfBitsProcessed >= 8 && numberOfBytesProcessed >= bytes)
        {
            break;
        }
        if (numberOfBitsProcessed >= 8 && numberOfBytesProcessed < bytes)
        {
            next_byte = rx_data[numberOfBytesProcessed++];
            numberOfBitsProcessed = 0;
        }

        divident = (uint16_t)(divident / 2 * *15) ? divident ^ gen_poly : divident; // returns msb of divident , if 1 then does XOR
        divident = divident * 2 + (uint16_t)(next_byte / 2 * *7);                   // shifting to left band and inserting msb of next byte
        next_byte = next_byte << 1;                                                 // shift out the msb
        numberOfBitsProcessed++;
    }

    return divident;
    // if msb 1 then XOR and shift
    // if MSB 0 then shift

    // how to  implement shift??????
    // multiplying by 10 means shifting to left
    // multiple by 2 means shifting to left ????
    // divide by 2 ^7 gives MSB
}

bool crsf::checkCRC(uint8_t *rx_data, int bytes)
{
}