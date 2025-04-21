#include "spiPortExpander.hpp"

SPIPortExpander::SPIPortExpander(SPI& spi, PinName cs, PinName reset, uint8_t address): spiBus(spi), cs(mbed::DigitalOut(cs)), resetOut(mbed::DigitalOut(reset)) {
    this->resetOut.write(1);
    this->cs.write(1);
    spi.frequency(100000);

    this->address = address;

    this->reset();
}

void SPIPortExpander::reset() {
    this->resetOut.write(0);
    wait_us(100);
    this->resetOut.write(1);
    wait_us(100);
}

void SPIPortExpander::write(uint8_t command, uint8_t data) {
    char cmd[3];
    cmd[0] = 0x40 | (this->address << 1);
    cmd[1] = command;
    cmd[2] = data;
    this->cs.write(0);
    this->spiBus.write(cmd, 3, nullptr, 0);
    this->cs.write(1);
}

uint8_t SPIPortExpander::read(uint8_t command) {
    char cmd[3];
    cmd[0] = 0x41 | (this->address << 1);
    cmd[1] = command;
    cmd[2] = 0;

    char recv[3];
    recv[0] = 0;
    recv[1] = 0;
    recv[2] = 0;
    this->cs.write(0);
    this->spiBus.write(cmd, 3, recv, 3);
    this->cs.write(1);
    return recv[2];
}

SPIPortExpander::DigitalOut SPIPortExpander::getDigitalOut(bool isA, uint8_t pinNumber) {
    return DigitalOut(this, isA, pinNumber);
}

SPIPortExpander::DigitalIn SPIPortExpander::getDigitalIn(bool isA, uint8_t pinNumber) {
    return DigitalIn(this, isA, pinNumber);
}

SPIPortExpander::DigitalOut::DigitalOut(SPIPortExpander* expander, bool isA, uint8_t number) {
    this->expander = expander;
    this->isA = isA;
    this->number = number;

    // configuring the pin direction
    uint8_t cmd = this->isA ? 0x00 : 0x01; // Direction register
    uint8_t directions = this->expander->read(cmd);
    directions &= ~(0x1 << this->number);
    this->expander->write(cmd, directions);
}

void SPIPortExpander::DigitalOut::write(uint8_t value) {
    uint8_t cmdWrite = this->isA ? 0x12 : 0x13; // GPIO register address
    uint8_t cmdRead = this->isA ? 0x14 : 0x15; // OLAT registers address

    uint8_t latchValues = this->expander->read(cmdRead);
    latchValues &= ~(0x1 << this->number);
    latchValues |= (value != 0) << this->number;

    this->expander->write(cmdWrite, latchValues);
}

SPIPortExpander::DigitalIn::DigitalIn(SPIPortExpander* expander, bool isA, uint8_t number) {
    this->expander = expander;
    this->isA = isA;
    this->number = number;

    // configuring the pin direction
    uint8_t cmd = this->isA ? 0x00 : 0x01; // Direction register
    uint8_t directions = this->expander->read(cmd);
    directions |= 0x1 << this->number;
    this->expander->write(cmd, directions);
}

uint8_t SPIPortExpander::DigitalIn::read() {
    uint8_t cmdRead = this->isA ? 0x12 : 0x13; // GPIO register address
    return (this->expander->read(cmdRead) >> this->number) & 0x1;
}