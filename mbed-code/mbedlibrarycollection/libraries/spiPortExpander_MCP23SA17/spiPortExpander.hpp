

#ifndef SPI_PORT_EPANDER_INCLUDED
#define SPI_PORT_EPANDER_INCLUDED

#undef __ARM_FP
#include <mbed.h>

class SPIPortExpander {
private:
    SPI& spiBus;
    DigitalOut cs;
    DigitalOut resetOut;
    uint8_t address;

public:
    class DigitalOut {
        SPIPortExpander* expander;
        bool isA;
        uint8_t number;

    public:
        DigitalOut(SPIPortExpander* expander, bool isA, uint8_t number);

        void write(uint8_t value);
    };

    class DigitalIn {
        SPIPortExpander* expander;
        bool isA;
        uint8_t number;

    public:
        DigitalIn(SPIPortExpander* expander, bool isA, uint8_t number);

        uint8_t read();
    };

    SPIPortExpander(SPI& spi, PinName cs, PinName reset, uint8_t address);

    void reset();

    DigitalOut getDigitalOut(bool isA, uint8_t pinNumber);

    DigitalIn getDigitalIn(bool isA, uint8_t pinNumber);

    // internal method, for use by the children classes
    void write(uint8_t command, uint8_t data);

    // internal method, for use by the children classes
    uint8_t read(uint8_t command);
};

#endif // SPI_PORT_EXPANDER_INCLUDED