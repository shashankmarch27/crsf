# CRSF
Yet another crossfire library but this time for our beloved ESP32 🙂. 

## CRC calculation

For CRC calculation we just have to divide the input data by the crc polynomal (for CRSF it is 0x1D5) and do so until all the input data is processed the code for the calculation can be found below:

```c++
uint16_t gen_poly = 0x1d5; //x8 + x5 + x4 + 1
uint8_t rx_data[4]= {0x28,0x00,0xea,0x54};
uint16_t calculateCRC(int bytes)
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
        return 0; // error
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
```

The above code is a for a generic crc and can divide and give remained of any two polynomials. But since we know that in CRC8 the cofficient of x^8 is always 1 and we do the shift and xor when the msb in the divident becomes 1, it is gurranted that the xor of the shifted out msb and cofficient of the x^8 would be zero hence we tend to leave out the 9th bit(cofficient of x^8) out of the polynomial since it would be 1 always then neglecting it wont change the calculation. Now since the polynomial(which becomes 0xD5) can be fit into 8 bits it becomes much easier to implement in harwdare as well as save space in software. Sample code after modifications can be found below ->

```c++
uint8_t gen_poly = 0xd5; //x8 + x5 + x4 + 1
uint8_t rx_data[4]= {0x28,0x00,0xea,0x54};
uint8_t calculateCRC(int bytes)
{   
    uint8_t dividend = 0;
    uint8_t next_byte;
    int numberOfBytesProcessed = 0; 
    int numberOfBitsLeft = 0;
    bool isMsbOne = false;
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

        isMsbOne = dividend & 0b10000000;
        dividend = dividend << 1 | (next_byte>>7);   // shift First bit of Next_byte into dividend
        next_byte = next_byte << 1;  // Shift out the first bit
        numberOfBitsLeft --;
        dividend = isMsbOne ? dividend ^ gen_poly : dividend; //if bit aligning with MSB of gen_poly is 1 then do XOR 

    }

    return dividend;
}
```


## Check List
- [ ] Crc Check failed when trying to read rc link packet
