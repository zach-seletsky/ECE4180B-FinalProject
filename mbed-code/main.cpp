
#include "mbed.h"
#include <cstring>
#include "BlueSMiRFBridge.h"
#include "uLCD.hpp"
#include "MyRPG.h"
#include "MyUtils.h"
#include "mpr121.h"


#define KEYS 8
// BRG565: Blue [15–11], Red [10–5], Green [4–0]
// #define COLOR_BRG565(r, g, b)  (((b & 0xF8) << 8) | ((r & 0xF8) << 3) | ((g & 0xF8) >> 3))
// Basic Colors
#define COLOR_WHITE     0xFFFF  // (B=31, R=31, G=31)
#define COLOR_BLACK     0x0000  // (B=0,  R=0,  G=0)
#define COLOR_RED       0x07C0  // (B=0,  R=31, G=0)
#define COLOR_GREEN     0x001F  // (B=0,  R=0,  G=31)
#define COLOR_BLUE      0xF800  // (B=31, R=0,  G=0)
#define COLOR_YELLOW    0x07FF  // (B=0,  R=31, G=31)
#define COLOR_CYAN      0xF81F  // (B=31, R=0,  G=31)
#define COLOR_MAGENTA   0xFFC0  // (B=31, R=31, G=0)
#define COLOR_GRAY      0x8410  // (B=16, R=16, G=16)
#define COLOR_ORANGE    0x06D4  // (B=0,  R=31, G≈20)
#define COLOR_PURPLE    0xF3C0  // (B=30, R=30, G=0)
#define COLOR_BROWN     0x0540  // (B=0,  R=20, G=10)
#define COLOR_PINK      0xFECF  // (B=31, R=31, G=15)
#define COLOR_LIGHTBLUE 0xF81F  // (B=31, R=0,  G=31)
#define COLOR_LIME      0x03E0  // (B=0,  R=15, G=31)
#define COLOR_NAVY      0x8000  // (B=16, R=0,  G=0)
#define COLOR_TEAL      0xF81F  // (B=31, R=0,  G=31)



/* ---------- UARTs ----------- */
BufferedSerial pc(USBTX, USBRX, 115200);      // USB console
BufferedSerial bt(p13, p14, 38400);     // BlueSMiRF link
I2C kb(p9, p10); 

BlueSMiRFBridge bridge(&bt, &pc);
Mpr121 keyboard(&kb, Mpr121::Address::ADD_VSS);

// inputs
DigitalIn switch1(p30);
DigitalIn brake(p29, PullDown);
DigitalIn gas(p26, PullDown);
// RPG---------------------------
MyRPG rpg(p5,p6,p7);

uLCD screen(p28, p27, p25, uLCD::BAUD_115200);      
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);    

const char keyChars[KEYS] = {'0','1','2','3','4','5','6','7'};
const int password[3] = {0,1,2};
const uint16_t colors[] = {0xFFFF, 0x0000, 0xFF00, 0x00FF, 0xF800, 0x07E0, 0x001F, 0xF81F, };
//                          white, black,  blue,   yellow, blue(?), pink, green, lightblue(?)

uint8_t gasState;
uint8_t brakeState;
uint8_t gearState;
uint8_t encoderState;
uint8_t collision; // 0= no collision, 1= collision

/* ---------- Testing Methods ----------- */
void testBtnsAndSwitch() {
    ThisThread::sleep_for(100ms);
        if (debounceBtn(&brake)){
            led1.write(1);
        } else {
            led1.write(0); 
        }

        if (debounceBtn(&gas)) {
            led2.write(1);
        } else {
            led2.write(0);
        }
        if (switch1.read()) {
            led3.write(1);
        } else {
            led3.write(0);
        }
}

void setOnBrdLEDS(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t l4) {
    led1.write(l1);
    led2.write(l2);
    led3.write(l3);
    led4.write(l4);
}

uint8_t testRPG() {
    ThisThread::sleep_for(100ms);
    // testBtnsAndSwitch();
    uint8_t encoderVal = rpg.readRPG();
    if (encoderVal == 0) {
        setOnBrdLEDS(1,0,0,0);
    } else if (encoderVal < 20) {
        setOnBrdLEDS(1,1,0,0);
    } else if (encoderVal < 40) {
        setOnBrdLEDS(1,1,1,0);
    } else {
        setOnBrdLEDS(1,1,1,1);
    }
    return encoderVal;
}

int testEncoder(int msgCnt) {
    // int msgCnt = 0;
    
    ThisThread::sleep_for(100ms);
    // testBtnsAndSwitch();
    int encoderVal = testRPG();
    screen.printf("Encoder: %d\n", encoderVal);
    msgCnt += 1;
    if (msgCnt > 10) {
        screen.cls();
        return 0;
    }
    return msgCnt;
}

void initScreen() {
    screen.cls();
    screen.locate(0,0);
    ThisThread::sleep_for(200ms);
    screen.printf("Bluetooth: Y");
    screen.printf("\nGear: F");
    screen.printf("\nGas: 0");
    screen.printf("\nBrake: 0");
    screen.printf("\nEncoder: 0");
    screen.printf("\nKeys: ");
    screen.printf("\nCollision: N");
}


int readKB(int* keys, char* keyString) {
    int keysPressed = keyboard.readTouchData();
    int str = 0;
    int numPressed = 0;
    for (int i=0; i<KEYS; i++) {
        keys[i] = keysPressed & (1 << i);
        if (keys[i]) {
            keyString[str] = keyChars[i];
            keyString[str+1] = ',';
            str += 2;
            numPressed += 1;
        }
    }
    keyString[str-1] = '\0';
    keyString[13] = '\0';

    return numPressed;

}

int readKB(int* keys) {
    char keyString[16] = {};
    return readKB(keys, keyString);
}

void initUnlockScreen() {
    screen.cls();
    screen.locate(2, 3);
    screen.printf("ENTER PASSWORD");
    screen.drawCircleFilled(30, 60, 5, 0);
    screen.drawCircleFilled(60, 60, 5, 0);
    screen.drawCircleFilled(90, 60, 5, 0);

}

void testULCDColors() {
    screen.cls();
    int yOffset = 10;
    ThisThread::sleep_for(200ms);
    for (int i=0; i<8; i++) {
        screen.drawRectangleFilled(0, yOffset*i, 128, yOffset*(i+1), colors[i]);
    }
    
}

int getExactKeyNum(int* keys) {
    for (int i=0; i<KEYS; i++) {
        if (keys[i]) {
            return i;
        }
    }
    return -1;
}

bool verifyPassword(int* toVerify) {
    for (int i=0; i<3; i++) {
        if (password[i] != toVerify[i]) {
            return false;
        }
    }
    return true;
}

void readPassword() {
    bool locked = true;
    int numsEntered = 0;
    int keys[8] = {};
    int code[3] = {};
    int circleOffset = 30;

    while (locked) {
        int pressed = readKB(keys);
        ThisThread::sleep_for(200ms);
        if (pressed == 1 && numsEntered < 3) {
            int curr = getExactKeyNum(keys);
            if (curr != -1) {
                code[numsEntered] = curr;
                numsEntered += 1;
            }
        }
        int del = debounceBtn(&brake);
        if (del && numsEntered > 0) {
            code[numsEntered] = 0;
            numsEntered -= 1;
        }
        int enter = debounceBtn(&gas);
        if (enter && numsEntered == 3) {
            locked = !verifyPassword(code);
            if (locked) {
                // screen.cls();
                screen.drawRectangleFilled(20, 50, 110, 70, COLOR_RED); 
                ThisThread::sleep_for(1s);
                screen.drawRectangleFilled(20, 50, 110, 70, COLOR_BLACK); 
                numsEntered = 0;
            } else {
                screen.drawRectangleFilled(20, 50, 110, 70, COLOR_GREEN); 
                ThisThread::sleep_for(1s);            
                return;    
            }
        }
        for (int i=0; i<3; i++) {
            if (i < numsEntered) {
                screen.drawCircleFilled(30 + i*circleOffset, 60, 5, COLOR_BLUE); // blue?
            } else {
                screen.drawCircleFilled(30 + i*circleOffset, 60, 5, COLOR_WHITE); //white?
            }
        }
    }
// colors:
// 0x07e0 == magenta?
}

bool readBtns() {
    bool change = false;
    //int keysPressedInt = keyboard.readTouchData();
    int gasPressed = debounceBtn(&gas);
    int brakePressed = debounceBtn(&brake);
    int switchState = switch1.read();
    int encoder = rpg.readRPG();
    char gear = 'F';

    if (switchState) {
        gear = 'R';
    }

    if (gearState != char(switchState+48) || gasState != char(gasPressed+48) || brakeState != char(brakePressed+48) || encoderState != char(encoder+65)) {
        // change occured
        gearState = char(switchState+48);
        gasState = char(gasPressed+48);
        brakeState = char(brakePressed+48);
        encoderState = char(encoder+65);
        change = true;
    }
    
    if (change) {
        screen.locate(6, 1);
        screen.printf("%c", gear);
        screen.locate(5, 2);
        screen.printf("%d", gasPressed);
        screen.locate(7, 3);
        screen.printf("%d", brakePressed);
        screen.locate(9, 4);
        screen.printf("%2d", encoder);

        screen.locate(5, 5);
        char keyString[16];
        int keysPressed[KEYS] = {};
        readKB(keysPressed, keyString);
        screen.printf("%13s", keyString);
    }
    // screen.cls();
    
    return change;
}


// constructs packet and sends/receives resp
// packet: gear|gas|brake|encoder
//          |1byte       | 1byte
// 0=transmit error, 1 = success, no collison, 2=success, collison
uint8_t updateBT() {
    
    char msg[12] ={'~','|', gearState,'|', gasState, '|', brakeState, '|', encoderState, '|', '~', '\0'};
    char resp[6]; //= {~, |, K, C, |, ~}

    pc.write(msg, 12); 
    bt.write(msg, 12);
    ThisThread::sleep_for(10ms);
    bt.read(resp, 6);
    pc.write(resp, 6);
    uint8_t valid = 0;
    // validate resp
    if (resp[0] == '~' && resp[1] == '|') {
        if (resp[2] == 'K' && resp[4] == '|' && resp[5] == '~') {
            valid = 1;
            if (resp[3] == 'C') {
                collision = 1;
            } else if (resp[3] == 'N') {
                collision = 0;
            } else {
                valid=0;
            }
        } else {
            // receieved NACK
        }
    }

    if (!valid) {
        char c;
        while (bt.read(&c, 1) == 1); // flush recv buffer
    }

   return valid;
}

/* ---------- main loop ----------- */
int main()
{
    // testULCDColors();
    bridge.initComms();
    screen.cls();
    
    initUnlockScreen();
    readPassword();
    ThisThread::sleep_for(100ms);
    initScreen();
    brakeState = '0';
    encoderState = char(65);
    gasState = '0';
    gearState = '0';

    while (true) {
        bool change = readBtns();
        // bridge.readPC(); // for testing
        if (change) {
            uint8_t success = updateBT();
            if (!success) {
                pc.write("BadPacket", 9);
            }
        }
        ThisThread::sleep_for(100ms);
    }
}
