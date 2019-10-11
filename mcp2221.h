/*
 * Declarations for Microchip MCP2221 chip.
 *
 * Copyright (C) 2019 Serge Vakulenko
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdint.h>
#pragma pack(1)

enum {
    MCP_CMD_STATUSSET               = 0x10,
    MCP_CMD_READFLASH               = 0xB0,
    MCP_CMD_WRITEFLASH              = 0xB1,
    MCP_CMD_FLASHPASS               = 0xB2,
    MCP_CMD_I2CWRITE                = 0x90,
    MCP_CMD_I2CWRITE_REPEATSTART    = 0x92,
    MCP_CMD_I2CWRITE_NOSTOP         = 0x94,
    MCP_CMD_I2CREAD                 = 0x91,
    MCP_CMD_I2CREAD_REPEATSTART     = 0x93,
    MCP_CMD_I2CREAD_GET             = 0x40,
    MCP_CMD_SETGPIO                 = 0x50,
    MCP_CMD_GETGPIO                 = 0x51,
    MCP_CMD_SETSRAM                 = 0x60,
    MCP_CMD_GETSRAM                 = 0x61,
    MCP_CMD_RESET                   = 0x70,
};

//
// Status/Set Parameters
//
typedef struct {
    uint8_t  command_code;          // 0x10 = MCP_CMD_STATUSSET
    uint8_t  unused1;               // Any value
    uint8_t  cancel_i2c;            // 0x10 = cancel the current I2C transfer
    uint8_t  set_i2c_speed;         // 0x20 = set the I2C communication speed
    uint8_t  i2c_clock_divider;     // value of the I2C system clock divider
} mcp_cmd_statusset_t;

typedef struct {
    uint8_t  command_code;          // 0x10 = MCP_CMD_STATUSSET
    uint8_t  status;                // 0x00 = Command completed successfully
    uint8_t  cancel_i2c;            // 0x00 = No special operation
                                    // 0x10 = Transfer was marked for cancellation
                                    // 0x11 = Already in Idle mode
    uint8_t  set_i2c_speed;         // 0x00 = No special operation
                                    // 0x20 = New communication speed is being set
                                    // 0x21 = Speed change rejected
    uint8_t  i2c_requested_divider; // Value of the I2C system clock divider
    uint8_t  unused5;               // Don’t care
    uint8_t  unused6;               // Don’t care
    uint8_t  unused7;               // Don’t care
    uint8_t  i2c_machine_state;     // Internal I2C state machine state value
    uint16_t i2c_transfer_length;   // Requested I2C transfer length
    uint16_t i2c_transfered;        // Number of already transferred bytes
    uint8_t  i2c_buf_count;         // Internal I2C data buffer counter
    uint8_t  i2c_current_divider;   // Current I2C speed divider
    uint8_t  i2c_current_timeout;   // Current I2C timeout value
    uint16_t i2c_address;           // I2C address being used
    uint8_t  unused18;              // Don’t care
    uint8_t  unused19;              // Don’t care
    uint8_t  unused20;              // Don’t care
    uint8_t  unused21;              // Don’t care
    uint8_t  scl_input;             // SCL line value, as read from the pin
    uint8_t  sda_input;             // SDA line value, as read from the pin
    uint8_t  intr_edge;             // Interrupt edge detector state, 0 or 1
    uint8_t  i2c_read_pending;      // 0, 1 or 2
    uint8_t  unused26;              // Don’t care
    uint8_t  unused27;              // Don’t care
    uint8_t  unused28;              // Don’t care
    uint8_t  unused29;              // Don’t care
    uint8_t  unused30;              // Don’t care
    uint8_t  unused31;              // Don’t care
    uint8_t  unused32;              // Don’t care
    uint8_t  unused33;              // Don’t care
    uint8_t  unused34;              // Don’t care
    uint8_t  unused35;              // Don’t care
    uint8_t  unused36;              // Don’t care
    uint8_t  unused37;              // Don’t care
    uint8_t  unused38;              // Don’t care
    uint8_t  unused39;              // Don’t care
    uint8_t  unused40;              // Don’t care
    uint8_t  unused41;              // Don’t care
    uint8_t  unused42;              // Don’t care
    uint8_t  unused43;              // Don’t care
    uint8_t  unused44;              // Don’t care
    uint8_t  unused45;              // Don’t care
    uint8_t  hardware_rev_major;    // ‘A’
    uint8_t  hardware_rev_minor;    // ‘6’
    uint8_t  firmware_rev_major;    // ‘1’
    uint8_t  firmware_rev_minor;    // ‘1’
    uint16_t adc_ch0;               // ADC channel 0 input value
    uint16_t adc_ch1;               // ADC channel 1 input value
    uint16_t adc_ch2;               // ADC channel 2 input value
} mcp_reply_statusset_t;

#pragma pack()
