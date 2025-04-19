#include "mbed.h"


#ifndef BLUESMIRFBRIDGE_H
#define BLUESMIRFBRIDGE_H


class BlueSMiRFBridge {    
    private:
        BufferedSerial* bt;
        BufferedSerial* pc;
        char cmd_buf[32];
        size_t cmd_len;

        /* --------------- utils ---------------- */
        size_t str_len(const char *s);

        void uart_puts(BufferedSerial* u, const char *s);

        void uart_gets(BufferedSerial* u, char *resp, size_t size=4);

        bool waitForOK(uint32_t timeoutMs);

        void flushBtRx();

        /* millisecond delay helper */
        void delay_ms(uint32_t ms);

        /* ---------- command‑mode helpers ----------- */
        bool enter_cmd();
        void exit_cmd();
        void reboot_ble();

        bool linkToHC08();


        void local_help();

    public:

        BlueSMiRFBridge(BufferedSerial* bt, BufferedSerial* pc): bt(bt), pc(pc) {}
        
        void initComms();

        void readPC();

        void transmit(BufferedSerial* u, const char* msg, char* resp);
            

};
#endif