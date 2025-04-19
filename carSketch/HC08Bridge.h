#ifndef HC08_BRIDGE_H
#define HC08_BRIDGE_H

#include <Arduino.h>

class HC08Bridge {
public:
    HC08Bridge(HardwareSerial* blePtr, Stream* dbgPtr = &SerialUSB)
        : bleSerial(blePtr), debug(dbgPtr) {}

    /**
     * @brief Initialize BLE UART with specified baud rate.
     */
    void begin(unsigned long baud = 38400) {
        bleSerial->begin(baud);
        delay(200);
        debug->println(F("[bridge] HC-08 Serial ready"));
    }

    /**
     * @brief Call periodically to relay bytes and process local commands.
     */
    void update() {
        handlePCInput();
        handleBLEInput();
    }

    /**
     * @brief Print available local ~commands.
     */
    void showHelp() {
        debug->println(F("\n[bridge] local commands:"));
        debug->println(F("  ~init     : init AT handshake"));
        debug->println(F("  ~link     : set ROLE=M and connect to MAC"));
        debug->println(F("  ~info     : query version, address, name"));
        debug->println(F("  ~help     : this help menu"));
    }

private:
    HardwareSerial* bleSerial;
    Stream* debug;

    char cmdBuf[32];
    uint8_t cmdLen = 0;

    /**
     * @brief Handle input from the PC SerialUSB.
     */
    void handlePCInput() {
        while (SerialUSB.available()) {
            char c = SerialUSB.read();

            if (cmdLen == 0 && c == '~') {
                cmdBuf[cmdLen++] = c;
                continue;
            }

            if (cmdLen > 0) {
                if (c == '\r' || c == '\n') {
                    cmdBuf[cmdLen] = '\0';
                    parseCommand();
                    cmdLen = 0;
                } else if (cmdLen < sizeof(cmdBuf) - 1) {
                    cmdBuf[cmdLen++] = c;
                }
                continue;
            }

            bleSerial->write(c);
        }
    }

    /**
     * @brief Handle input from the HC-08 and forward to PC.
     */
    void handleBLEInput() {
        while (bleSerial->available()) {
            debug->write(bleSerial->read());
        }
    }

    /**
     * @brief Dispatch a ~ command typed from the PC.
     */
    void parseCommand() {
        if (strcmp(cmdBuf, "~init") == 0) {
            sendAT("AT");
            delay(100);
            sendAT("AT+RX");
        } else if (strcmp(cmdBuf, "~link") == 0) {
            sendAT("AT+ROLE=M");
            delay(500);
            sendAT("AT+CLEAR");
            delay(300);
            sendAT("AT+CON64B708839362");
        } else if (strcmp(cmdBuf, "~info") == 0) {
            sendAT("AT+VERSION");
            delay(100);
            sendAT("AT+ADDR");
            delay(100);
            sendAT("AT+NAME");
        } else if (cmdBuf[1]=='~') {
          sendAT(&cmdBuf[2]);
        } else {
            showHelp();
        }
    }

    /**
     * @brief Send an AT command and print both the command and the response.
     * @param cmd The AT command to send.
     */
    void sendAT(const char* cmd) {
        debug->print(F("\n[TX] ")); debug->println(cmd);
        bleSerial->print(cmd);
        bleSerial->print("\r\n");

        uint32_t t0 = millis();
        char c;
        debug->print(F("[RX] "));
        while (millis() - t0 < 1000) {
            if (bleSerial->available()) {
                c = bleSerial->read();
                debug->write(c);
            }
        }
        debug->println();
    }
};

#endif
