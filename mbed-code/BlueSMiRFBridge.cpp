
#include "BlueSMiRFBridge.h"
    
/* ---------- utils ----------- */
/**
* @brief  Freestanding strlen replacement.
*/
size_t BlueSMiRFBridge::str_len(const char *s) {
    size_t n = 0U;
    while (*s++) ++n;
    return n;
}

/**
* @brief  Write NUL‑terminated string to a BufferedSerial.
*/
void BlueSMiRFBridge::uart_puts(BufferedSerial* u, const char *s) {
    u->write(s, str_len(s));
    u->sync();                                  // ensure TX buffer is sent
}

void BlueSMiRFBridge::uart_gets(BufferedSerial* u, char *resp, size_t size) {
    u->read(resp, size);
}


/**
* @brief  Block until the substring "OK" is received from the BLE UART
*         or a timeout expires.
*
* Purpose : Detect the BlueSMiRF v2 command–mode acknowledgement.
* Result  : Returns true  if "OK" is seen before the deadline,
*           returns false if the timeout elapses first.
* Params  :
*     timeoutMs – maximum time to wait, in milliseconds.
* Notes   :
*     • Uses a 2‑byte rolling window to avoid <cstring> / stdlib calls.
*     • Echoes every received byte to the PC console so you can watch
*       progress in real time.
*     • Requires global BufferedSerial objects named `bt` and `pc`.
*/
bool BlueSMiRFBridge::waitForOK(uint32_t timeoutMs = 2000)
{
    char   win[2] = {0};                 // rolling window ('O','K')
    char   ch;

    Timer  t;                            // mbed timer
    t.start();                           // t.elapsed_time() returns chrono::microseconds

    while (t.elapsed_time() < chrono::milliseconds(timeoutMs))
    {
        if (bt->read(&ch, 1) == 1)       // got a byte from BlueSMiRF?
        {
            pc->write(&ch, 1);            // mirror to USB console

            win[0] = win[1];             // slide window
            win[1] = ch;

            if (win[0] == 'O' && win[1] == 'K')
                return true;             // success
        }
    }
    return false;                        // timed‑out
}

/**
* @brief  Drain all unread bytes from the BLE UART RX buffer.
*
* Purpose : Replace non‑existent BufferedSerial::flush().
* Result  : RX FIFO empty; TX side untouched.
*/
void BlueSMiRFBridge::flushBtRx()
{
    char ch;
    while (bt->readable()) bt->read(&ch, 1);
}

/* millisecond delay helper */
void BlueSMiRFBridge::delay_ms(uint32_t ms) { ThisThread::sleep_for(chrono::milliseconds(ms)); }

/* ---------- command‑mode helpers ----------- */
bool BlueSMiRFBridge::enter_cmd()
{
    uart_puts(pc, "\r\n[bridge] entering CMD…");
    flushBtRx();                               // clear RX garbage

    delay_ms(2100);                              // ≥ MinEscapeTimeMs
    bt->write("$$$", 3);
    bt->sync();
    delay_ms(2100);

    /* wait ≤1 s for 'OK' */
    char win[2] = {0};
    uint32_t t0 = Kernel::Clock::now().time_since_epoch().count();
    while (Kernel::Clock::now().time_since_epoch().count() - t0 < 1000000) {
        char c;
        if (bt->read(&c, 1) == 1) {
            pc->write(&c, 1);                    // live echo to PC
            win[0] = win[1]; win[1] = c;        // 2‑byte window
            if (win[0]=='O' && win[1]=='K') {
                uart_puts(pc, " [OK]\r\n");
                return true;
            }
        }
    }
    uart_puts(pc, " [timeout]\r\n");
    return false;
}

void BlueSMiRFBridge::exit_cmd()   { uart_puts(bt, "ATX\r\n"); }
void BlueSMiRFBridge::reboot_ble() { uart_puts(bt, "ATZ\r\n"); }

/**
* @brief  Configure BlueSMiRF as master, store HC‑08 MAC,
*         initiate SPP connection, then return to data mode.
*
* Result  : When this returns true the BlueSMiRF and HC‑08
*           are in a live transparent link at 115200 bps.
* Params  : none (HC08_ADDR constant below).
*/
bool BlueSMiRFBridge::linkToHC08()
{
    /* ---------- constants ---------- */
    const char HC08_ADDR[] = "7804730E3395";

    /* ---------- step 1: enter CMD ---------- */
    if (!enter_cmd()) return false;

    /* ---------- step 2: configure ---------- */
    uart_puts(pc, "[bridge] set master …\r\n");
    uart_puts(bt,"AT-Role=1\r\n");          /* 1 = master   */
    waitForOK(1000);

    uart_puts(pc, "[bridge] save peer MAC …\r\n");
    uart_puts(bt,"AT-Connect=");
    uart_puts(bt,HC08_ADDR);                /* store MAC    */
    uart_puts(bt,"\r\n");
    waitForOK(1000);

    uart_puts(pc, "[bridge] auto‑connect …\r\n");
    uart_puts(bt,"AT-AutoConnect=1\r\n");   /* connect at boot */
    waitForOK(1000);

    uart_puts(pc, "[bridge] write & reboot …\r\n");
    uart_puts(bt,"ATW\r\n");                /* save NVM      */
    waitForOK(1000);
    uart_puts(bt,"ATZ\r\n");                /* reboot; exits CMD */
    /* after reboot the module starts connecting automatically */

    /* ---------- step 3: wait CONNECT banner ---------- */
    uart_puts(pc,"[bridge] waiting for CONNECT …\r\n");
    char last=0,c;
    uint32_t t0 = Kernel::Clock::now().time_since_epoch().count();
    while(Kernel::Clock::now().time_since_epoch().count() - t0 < 8000000){
        if(bt->read(&c,1)==1){
            pc->write(&c,1);                 /* mirror banner */
            if(last=='C' && c=='d'){        /* "Connected" */
                uart_puts(pc,"\r\n[✓] link up\r\n");
                return true;
            }
            last=c;
        }
    }
    uart_puts(pc,"\r\n[✗] link timeout\r\n");
    return false;
}


void BlueSMiRFBridge::local_help()
{
    uart_puts(pc,
        "\r\n[bridge] local commands:\r\n"
        "  ~cmd      : enter command mode ($$$)\r\n"
        "  ~exit     : leave command mode (ATX)\r\n"
        "  ~reboot   : reboot BlueSMiRF (ATZ)\r\n"
        "  ~link     : connect to HC‑08 and bridge data\r\n"
        "  ~help     : this help\r\n");
}


void BlueSMiRFBridge::initComms() {
    pc->set_blocking(false);
    bt->set_blocking(false);
    cmd_len = 0;
    uart_puts(pc,
        "\r\n========== BlueSMiRF Bridge ==========\r\n"
        "Type '~help' for local commands.\r\n\n");
}

void BlueSMiRFBridge::readPC() {
    char c;
    
    /* --- PC → BLE --- */
    if (pc->read(&c, 1) == 1) {
        /* local tilde command */
        if (cmd_len == 0 && c == '~') { cmd_buf[cmd_len++] = c; return; }
        if (cmd_len > 0) {                      // collecting a tilde cmd
            if (c == '\r' || c == '\n') {
                cmd_buf[cmd_len] = '\0';
                if (cmd_buf[1]=='c') enter_cmd();
                else if (cmd_buf[1]=='e') exit_cmd();
                else if (cmd_buf[1]=='r') reboot_ble();
                else if (cmd_buf[1]=='l') linkToHC08();   // ~link
                else local_help();
                cmd_len = 0;
            } else if (cmd_len < sizeof(cmd_buf)-1) {
                cmd_buf[cmd_len++] = c;
            } 
            return;                           // don't forward
        }
        /* transparent forward */
        bt->write(&c, 1);
    }

    /* --- BLE → PC --- */
    if (bt->read(&c, 1) == 1) pc->write(&c, 1);
}


void BlueSMiRFBridge::transmit(BufferedSerial *u, const char *msg, char *resp) {
    uart_puts(u, msg);
    ThisThread::sleep_for(10ms);
    uart_gets(u, resp);
}