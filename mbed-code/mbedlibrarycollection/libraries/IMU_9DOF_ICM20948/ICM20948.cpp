#include "ICM20948.hpp"

IMU_ICM20948::IMU_ICM20948(PinName sda, PinName sck): i2c(sda, sck) {
    // resetting device
    i2c.frequency(400000);
    write(0x6, 0x80);

    wait_us(100000);

    write(0x6, 0x1); // waking chip and setting clock to PLL
    wait_us(100000);

    // setting mag I2C bypass
    write(0xF, 0x2);
    write(0x3, 0xE0);

    setControllingMag(true);
    write(0x31, 0x08, true); // waking magnetometer
    wait_us(100);
    setControllingMag(false);
    //switchBank(2);
}

void IMU_ICM20948::getAccelerationData(float* x, float* y, float* z) {
    // getting accelerometer sensitivity
    switchBank(2);
    char accelConf = read(0x14);
    switchBank(0);
    accelConf = (accelConf >> 1) & 0x3;

    float sensitivity = 16384; // 2G
    if (accelConf == 0b01) {
        sensitivity = 8192;
        
    } else if (accelConf == 0b10) {
        sensitivity = 4096;
    } else if (accelConf == 0b11) {
        sensitivity = 2048; // 16G
    }

    int16_t xRaw = read(0x2D) << 8 | read(0x2E);
    int16_t yRaw = read(0x2F) << 8 | read(0x30);
    int16_t zRaw = read(0x31) << 8 | read(0x32);
    *x = xRaw / sensitivity * this->accelOffsets[0];
    *y = yRaw / sensitivity * this->accelOffsets[1];
    *z = zRaw / sensitivity * this->accelOffsets[2];
}

void IMU_ICM20948::getGyroscopeData(float* x, float* y, float* z) {
    // getting gyro sensitivity
    switchBank(2);
    char gyroConf = read(0x01);
    switchBank(0);

    gyroConf = (gyroConf >> 1) & 0x3;

    float sensitivity = 131;
    if (gyroConf == 0b01) {
        sensitivity = 65.5f;
        
    } else if (gyroConf == 0b10) {
        sensitivity = 32.8f;
    } else if (gyroConf == 0b11) {
        sensitivity = 16.4f; // 16G
    }

    int16_t xRaw = read(0x33) << 8 | read(0x34);
    int16_t yRaw = read(0x35) << 8 | read(0x36);
    int16_t zRaw = read(0x37) << 8 | read(0x38);
    *x = xRaw / sensitivity + this->gyroOffsets[0];
    *y = yRaw / sensitivity + this->gyroOffsets[1];
    *z = zRaw / sensitivity + this->gyroOffsets[2];
}

// uT per axis
void IMU_ICM20948::getMagnetometerData(float* x, float* y, float* z) {
    setControllingMag(true);
    write(0x31, 0x08, true);
    wait_us(10000);
    int16_t xRaw = read(0x12, true) << 8 | read(0x11, true);
    int16_t yRaw = read(0x14, true) << 8 | read(0x13, true);
    int16_t zRaw = read(0x16, true) << 8 | read(0x15, true);
    setControllingMag(false);

    float bitsPerUT = 6.67f;

    *x = xRaw / bitsPerUT;
    *y = yRaw / bitsPerUT;
    *z = zRaw / bitsPerUT;
}

// still calibration. Requires the device be absolutely still during this process
void IMU_ICM20948::calibrate() {
    // getting 10 samples from both the accelerometer and gyro
    // it then averages the 10 samples
    // for the accelerometer, it computes the magnitude of the vector, and since the acceleration due to gravity should be 1,
    // it will store the inverse of this magnitude as a scaling factor.
    // for the gyro, it will treat the average as a bias.
    float ax, ay, az = 0;
    float gx, gy, gz = 0;
    for (int i = 0; 10 > i; i++) {
        float tx, ty, tz;
        getAccelerationData(&tx, &ty, &tz);
        ax += tx;
        ay += ty;
        az += tz;

        getGyroscopeData(&tx, &ty, &tz);
        gx += tx;
        gy += ty;
        gz += tz;

        wait_us(1000);
    }

    this->gyroOffsets[0] = -gx / 10;
    this->gyroOffsets[1] = -gy / 10;
    this->gyroOffsets[2] = -gz / 10;

    ax /= 10;
    ay /= 10;
    az /= 10;

    float accelMag = sqrt(ax*ax + ay*ay + az*az);
    this->accelOffsets[0] = 1/accelMag;
    this->accelOffsets[1] = 1/accelMag;
    this->accelOffsets[2] = 1/accelMag;
}

// load a previous calibration
void IMU_ICM20948::calibrate(float* accelOffsets, float* gyroOffsets) {
    for (int i = 0; 3 > i; i++) {
        this->accelOffsets[i] = accelOffsets[i];
        this->gyroOffsets[i] = gyroOffsets[i];
    }
}

// get the last calibration result.
void IMU_ICM20948::getCalibration(float* accelOffsets, float* gyroOffsets) {
    for (int i = 0; 3 > i; i++) {
        accelOffsets[i] = this->accelOffsets[i];
        gyroOffsets[i] = this->gyroOffsets[i];
    }
}