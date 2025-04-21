

#ifndef ICM20948_INCLUDED
#define ICM20948_INCLUDED

#include "mbed.h"

class IMU_ICM20948 {
    private:
    I2C i2c;
    float accelOffsets[3] = {1, 1, 1}; // scalar
    float gyroOffsets[3] = {0, 0, 0}; // bias
    char lastBank = 0;

    const char devAddr = 0x68<<1; // AD0 is LOW, otherwise should be 0x69<<1.
    const char magAddr = 0x0C<<1;

    public:
    IMU_ICM20948(PinName sda, PinName sck);

    // still calibration. Requires the device be absolutely still during this process
    void calibrate();

    // load a previous calibration
    void calibrate(float* accelOffsets, float* gyroOffsets);

    // get the last calibration result.
    void getCalibration(float* accelOffsets, float* gyroOffsets);

    // in G's. 1G = acceleration due to gravity.
    void getAccelerationData(float* x, float* y, float* z);

    // angular velocity, in degrees per second
    void getGyroscopeData(float* x, float* y, float* z);

    // uT per axis
    void getMagnetometerData(float* x, float* y, float* z);

    private:
    inline void write(char addr, char data, bool isMag = false) {
        char buf[2];
        buf[0] = addr;
        buf[1] = data;
        i2c.write(isMag ? magAddr : devAddr, buf, 2, false);
    }

    inline char read(char addr, bool isMag = false) {
        char buf = addr;
        i2c.write(isMag ? magAddr : devAddr, &buf, 1, true);
        i2c.read(isMag ? magAddr : devAddr, &buf, 1, false);
        return buf;
    }

    inline void switchBank(char bank) {
        if (bank == lastBank) {
            return;
        }
        write(0x7F, bank << 4);
        lastBank = bank;
    }

    inline void setControllingMag(bool controlling) {
        switchBank(0);
        char userControlReg = read(0x3);
        if (controlling) {
            userControlReg &= ~0x20;
        } else {
            userControlReg |= 0x20;
        }
        write(0x3, userControlReg);
    }
};

#endif // ICM20948_INCLUDED