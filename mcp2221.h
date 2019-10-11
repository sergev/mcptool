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

//
// First byte of HID command sent to the MCP2221 chip.
//
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
// Second byte of MCP_CMD_READFLASH command.
//
enum {
    MCP_FLASH_CHIPSETTINGS          = 0x00,
    MCP_FLASH_GPIOSETTINGS          = 0x01,
    MCP_FLASH_USBMANUFACTURER       = 0x02,
    MCP_FLASH_USBPRODUCT            = 0x03,
    MCP_FLASH_USBSERIAL             = 0x04,
    MCP_FLASH_FACTORYSERIAL         = 0x05,
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
} mcp_cmd_status_t;

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
} mcp_reply_status_t;

//
// Read Flash Data: Chip Settings
//
typedef struct {
    uint8_t  command_code;          // 0xb0 = MCP_CMD_READFLASH
    uint8_t  status;                // 0x00 = Command completed successfully
    uint8_t  nbytes;                // Length of this structure
    uint8_t  unused3;               // Don’t care
    struct {
        unsigned password  : 1;     // Chip security: Password-protected
        unsigned lock      : 1;     // Chip security: Permanently locked
        unsigned usbcfg    : 1;     // Initial value for USBCFG pin
        unsigned sspnd     : 1;     // Initial value for SSPND pin
        unsigned ledi2c    : 1;     // Initial value for LEDI2C pin
        unsigned leduarttx : 1;     // Initial value for LEDUARTTX pin
        unsigned leduartrx : 1;     // Initial value for LEDUARTRX pin
        unsigned cdcsernum : 1;     // Use USB serial number for CDC enumeration
    } config0;
    struct {
        unsigned clko_div  : 3;     // Clock Output divider
#define MCP_CLKO_DIV_375KHZ 7       // 375 kHz clock output
#define MCP_CLKO_DIV_750KHZ 6       // 750 kHz clock output
#define MCP_CLKO_DIV_1_5MHZ 5       // 1.5 MHz clock output
#define MCP_CLKO_DIV_3MHZ   4       // 3 MHz clock output
#define MCP_CLKO_DIV_6MHZ   3       // 6 MHz clock output
#define MCP_CLKO_DIV_12MHZ  2       // 12 MHz clock output (factory default)
#define MCP_CLKO_DIV_24MHZ  1       // 24 MHz clock output
#define MCP_CLKO_DIV_OFF    0       // Reserved
        unsigned clko_dc   : 2;     // Clock Output duty cycle
#define MCP_CLKO_DC_75      3       // Duty cycle 75%
#define MCP_CLKO_DC_50      2       // Duty cycle 50% (factory default)
#define MCP_CLKO_DC_25      1       // Duty cycle 25%
#define MCP_CLKO_DC_0       0       // Duty cycle 0%
        unsigned unused    : 3;     // Don’t care
    } config1;                      // Byte 5
    struct {
        unsigned dac_power_up : 5;  // Power-Up DAC value
        unsigned dac_ref_en   : 1;  // Enable Vrm as DAC reference voltage
        unsigned dac_ref_sel  : 2;  // DAC Reference voltage option
#define MCP_REF_4096    3           // Reference voltage is 4.096V
#define MCP_REF_2048    2           // Reference voltage is 2.048V
#define MCP_REF_1024    1           // Reference voltage is 1.024V
#define MCP_REF_OFF     0           // Reference voltage is off
    } config2;
    struct {
        unsigned unused0      : 1;  // Don’t care
        unsigned unused1      : 1;  // Don’t care
        unsigned adc_ref_en   : 1;  // Enable Vrm as ADC reference voltage
        unsigned adc_ref_sel  : 2;  // ADC Reference voltage option
        unsigned intr_pos     : 1;  // Interrupt detection on a positive edge
        unsigned intr_neg     : 1;  // Interrupt detection on a negative edge
        unsigned unused7      : 1;  // Don’t care
    } config3;                      // Byte 7
    uint16_t usb_vid;               // USB Vendor Identifier
    uint16_t usb_pid;               // USB Product Identifier
    uint8_t  usb_power_attrs;       // USB power attributes
    uint8_t  usb_max_power;         // USB requested number of mA (divided by 2)
} mcp_reply_chip_settings_t;

//
// GP Power-Up Settings
//
typedef struct {
    unsigned function   : 3;        // GPx Designation, 0 = GPIO operation
    unsigned dir_input  : 1;        // 0 = GPIO Output, 1 = Input mode
    unsigned output_val : 1;        // Output value at power-up
    unsigned unused : 3;            // Don’t care
} mcp_gpio_config_t;

//
// Read Flash Data: GPIO Settings
//
typedef struct {
    uint8_t  command_code;          // 0xb0 = MCP_CMD_READFLASH
    uint8_t  status;                // 0x00 = Command completed successfully
    uint8_t  nbytes;                // Length of this structure
    uint8_t  unused3;               // Don’t care
    mcp_gpio_config_t gp0;          // GP0 Power-Up Settings
                                    // 0 - GPIO operation
                                    // 1 - SSPND output
                                    // 2 - LED UART RX output
    mcp_gpio_config_t gp1;          // GP1 Power-Up Settings
                                    // 0 - GPIO operation
                                    // 1 - Clock output
                                    // 2 - ADC1 input
                                    // 3 - LED UART TX output
                                    // 4 - Interrupt Detection
    mcp_gpio_config_t gp2;          // GP2 Power-Up Settings
                                    // 0 - GPIO operation
                                    // 1 - USBCFG output
                                    // 2 - ADC2 input
                                    // 3 - DAC1 output
    mcp_gpio_config_t gp3;          // GP3 Power-Up Settings
                                    // 0 - GPIO operation
                                    // 1 - LED I2C output
                                    // 2 - ADC3 input
                                    // 3 - DAC2 output
} mcp_reply_gpio_settings_t;

//
// Get SRAM Data
//
typedef struct {
    uint8_t  command_code;          // 0x61 = MCP_CMD_GETSRAM
    uint8_t  status;                // 0x00 = Command completed successfully
    uint8_t  nbytes_sram;           // Size of the SRAM Chip settings area
    uint8_t  nbytes_gp;             // Size of the SRAM GP settings area
    struct {
        unsigned password  : 1;     // Chip security: Password-protected
        unsigned lock      : 1;     // Chip security: Permanently locked
        unsigned usbcfg    : 1;     // Initial value for USBCFG pin
        unsigned sspnd     : 1;     // Initial value for SSPND pin
        unsigned ledi2c    : 1;     // Initial value for LEDI2C pin
        unsigned leduarttx : 1;     // Initial value for LEDUARTTX pin
        unsigned leduartrx : 1;     // Initial value for LEDUARTRX pin
        unsigned cdcsernum : 1;     // Use USB serial number for CDC enumeration
    } config0;
    struct {
        unsigned clko_divider : 5;  // Clock Output divider value
        unsigned unused       : 3;  // Don’t care
    } config1;                      // Byte 5
    struct {
        unsigned dac_power_up : 5;  // Power-Up DAC value
        unsigned dac_ref_en   : 1;  // Enable Vrm as DAC reference voltage
        unsigned dac_ref_sel  : 2;  // DAC Reference voltage option
    } config2;
    struct {
        unsigned unused0      : 1;  // Don’t care
        unsigned unused1      : 1;  // Don’t care
        unsigned adc_ref_en   : 1;  // Enable Vrm as ADC reference voltage
        unsigned adc_ref_sel  : 2;  // ADC Reference voltage option
        unsigned intr_pos     : 1;  // Interrupt detection on a positive edge
        unsigned intr_neg     : 1;  // Interrupt detection on a negative edge
        unsigned unused7      : 1;  // Don’t care
    } config3;                      // Byte 7
    uint16_t usb_vid;               // USB Vendor Identifier
    uint16_t usb_pid;               // USB Product Identifier
    uint8_t  usb_power_attrs;       // USB power attributes
    uint8_t  usb_max_power;         // USB requested number of mA (divided by 2)

    uint8_t  password[8];           // Current password

    mcp_gpio_config_t gp0;          // GP0 Power-Up Settings
    mcp_gpio_config_t gp1;          // GP1 Power-Up Settings
    mcp_gpio_config_t gp2;          // GP2 Power-Up Settings
    mcp_gpio_config_t gp3;          // GP3 Power-Up Settings
} mcp_reply_sram_data_t;

//
// Get GPIO Values
//
typedef struct {
    uint8_t  command_code;          // 0x51 = MCP_CMD_GETGPIO
    uint8_t  status;                // 0x00 = Command completed successfully
    uint8_t  gp0_pin;               // GP0 pin value
    uint8_t  gp0_direction;         // GP0 direction value (0 output, 1 input)
    uint8_t  gp1_pin;               // GP1 pin value
    uint8_t  gp1_direction;         // GP1 direction value (0 output, 1 input)
    uint8_t  gp2_pin;               // GP2 pin value
    uint8_t  gp2_direction;         // GP2 direction value (0 output, 1 input)
    uint8_t  gp3_pin;               // GP3 pin value
    uint8_t  gp3_direction;         // GP3 direction value (0 output, 1 input)
} mcp_reply_gpio_t;

#pragma pack()
