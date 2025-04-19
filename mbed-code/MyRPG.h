#include "mbed.h"

#ifndef MYRPG_H
#define MYRPG_H

class MyRPG {
    private:
        InterruptIn RPG_A; //encoder A and B pins/bits use interrupts
        InterruptIn RPG_B;
        InterruptIn RPG_PB; //encode pushbutton switch "SW" on PCB
        const int lookup_table[16] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
        volatile int old_enc;
        volatile int new_enc;
        volatile int enc_count;

        void Enc_change_ISR(void) {
            new_enc = RPG_A<<1 | RPG_B;//current encoder bits
            //check truth table for -1,0 or +1 added to count
            enc_count = enc_count + lookup_table[old_enc<<2 | new_enc];
            old_enc = new_enc;
        };
        
    
    public:
        MyRPG(PinName a, PinName b, PinName pb): RPG_A(a, PullUp), RPG_B(b, PullUp), RPG_PB(pb) {
            old_enc = 0;
            new_enc = 0;
            enc_count = 30;
            
            
            RPG_A.rise(callback(this, &MyRPG::Enc_change_ISR));
            RPG_A.fall(callback(this, &MyRPG::Enc_change_ISR));
            RPG_B.rise(callback(this, &MyRPG::Enc_change_ISR));
            RPG_B.fall(callback(this, &MyRPG::Enc_change_ISR));

        };

        uint8_t readRPG() {
            if (enc_count>60) enc_count = 60;
            if (enc_count<0) enc_count = 0;
            return enc_count;
        };

        


};




#endif