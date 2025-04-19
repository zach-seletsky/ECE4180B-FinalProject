// #ifndef BLUESMIRF_CONTROLLER_H
// #define BLUESMIRF_CONTROLLER_H

// #include "mbed.h"

// extern "C" {
//     #include <string.h>  // For strlen, memset, strncmp
// }

// #define MAX_RESPONSE 64

// class BlueSMiRFController {
// public:
//     BlueSMiRFController(PinName tx, PinName rx, int baud = 115200)
//         : ble(tx, rx, baud) {
//         ble.set_format(8, BufferedSerial::None, 1);
//     }

//     // Initialize the module and enter command mode
//     bool initialize() {
//         clear_buffer();

//         // Wait for idle line
//         ThisThread::sleep_for(1000ms);

//         if (!enter_command_mode()) {
//             debug("Failed to enter command mode.");
//             return false;
//         }

//         if (!disable_echo()) {
//             debug("Failed to disable echo.");
//             return false;
//         }

//         return true;
//     }

//     // Run a single configuration command and wait for OK
//     bool send_command(const char *cmd, bool expectOK = true) {
//         write_raw(cmd);
//         ThisThread::sleep_for(300ms);
//         int n = read_response(response, sizeof(response));

//         if (expectOK) {
//             return contains_ok(response, n);
//         }
//         return true;
//     }

//     // Factory reset, set master/slave mode, etc.
//     void apply_default_configuration() {
//         send_command("SF,1\r");      // Factory reset
//         ThisThread::sleep_for(500ms);

//         send_command("S~,0\r");      // Connect to any
//         send_command("SM,0\r");      // Manual connect
//         send_command("W\r");         // Save config
//         send_command("R,1\r", false);// Reboot (no OK expected)

//         ThisThread::sleep_for(1000ms);
//         enter_command_mode();        // Re-enter
//         disable_echo();
//     }

//     // Send arbitrary string (used in data mode)
//     void send_data(const char *data) {
//         ble.write(data, strlen(data));
//     }

//     // Request a settings dump
//     void dump_configuration() {
//         send_command("D\r", false);
//         int n = read_response(response, sizeof(response));
//         print_hex(response, n);
//     }

// private:
//     BufferedSerial ble;
//     char response[MAX_RESPONSE];

//     void clear_buffer() {
//         while (ble.readable()) {
//             char tmp[1];
//             ble.read(tmp, 1);
//         }
//     }

//     bool enter_command_mode() {
//         ble.write("$$$", 3);
//         ThisThread::sleep_for(300ms);
//         int n = read_response(response, sizeof(response));
//         return contains(response, n, "CMD");
//     }

//     bool disable_echo() {
//         return send_command("E,0\r");
//     }

//     void write_raw(const char *cmd) {
//         ble.write(cmd, strlen(cmd));
//     }

//     int read_response(char *buffer, int size) {
//         memset(buffer, 0, size);
//         return ble.read(buffer, size);
//     }

//     void debug(const char *msg) {
//         printf("[BlueSMiRF] %s\n", msg);
//     }

//     void print_hex(const char *data, int len) {
//         for (int i = 0; i < len; ++i) {
//             if (i % 12 == 0) printf("\n");
//             printf("%02x ", static_cast<unsigned char>(data[i]));
//         }
//         printf("\n");
//     }

//     bool contains(const char *buf, int len, const char *substr) {
//         for (int i = 0; i <= len - (int)strlen(substr); ++i) {
//             if (strncmp(&buf[i], substr, strlen(substr)) == 0)
//                 return true;
//         }
//         return false;
//     }

//     bool contains_ok(const char *buf, int len) {
//         return contains(buf, len, "OK");
//     }
// };

// #endif // BLUESMIRF_CONTROLLER_H
