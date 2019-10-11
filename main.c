/*
 * Configuration tool for MCP2221 chip.
 *
 * Copyright (c) 2019 Serge Vakulenko
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mcp2221.h"
#include "util.h"

//
// MCP2221 USB-I2C/UART Combo
//
#define MCP2221_VID 0x04d8
#define MCP2221_PID 0x00dd

const char version[] = VERSION;
const char *copyright;

extern char *optarg;
extern int optind;

void usage()
{
    fprintf(stderr, "MCP2221 Tool, Version %s, %s\n", version, copyright);
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "    mcptool [options]\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -r     Read confguration from device.\n");
    fprintf(stderr, "    -t     Trace USB protocol.\n");
    exit(-1);
}

//
// Connect to the MCP2221 chip.
//
static void mcp_connect()
{
    if (hid_init(MCP2221_VID, MCP2221_PID) < 0) {
        fprintf(stderr, "No MCP2221 chip detected.\n");
        fprintf(stderr, "Check your USB cable!\n");
        exit(-1);
    }
    fprintf(stderr, "Connect to MCP2221 chip.\n");
}

//
// Close the MCP2221 connection.
//
static void mcp_disconnect()
{
    fprintf(stderr, "Close device.\n");
    hid_close();
}

static void mcp_print_status(mcp_reply_status_t *status)
{
    printf("Hardware Revision: %c%c\n", status->hardware_rev_major, status->hardware_rev_minor);
    printf("Firmware Revision: %c.%c\n", status->firmware_rev_major, status->firmware_rev_minor);
    if (trace_flag) {
        printf("SCL Input: %d\n", status->scl_input);
        printf("SDA Input: %d\n", status->sda_input);
        printf("Interrupt Edge: %d\n", status->intr_edge);
        printf("ADC Channel 0 Input: %d\n", status->adc_ch0);
        printf("ADC Channel 1 Input: %d\n", status->adc_ch1);
        printf("ADC Channel 2 Input: %d\n", status->adc_ch2);
    }
}

static void mcp_print_chip_settings(mcp_reply_chip_settings_t *settings)
{
    printf("USB Vendor ID: 0x%04x\n", settings->usb_vid);
    printf("USB Product ID: 0x%04x\n", settings->usb_pid);
    printf("USB Max Power: %dmA\n", settings->usb_max_power * 2);
    printf("USB Power Attributes: %#x\n", settings->usb_power_attrs);

    const char *freq = "??", *duty = "??";
    switch (settings->config1.clko_div) {
    case MCP_CLKO_DIV_375KHZ: freq = "375 kHz"; break;
    case MCP_CLKO_DIV_750KHZ: freq = "750 kHz"; break;
    case MCP_CLKO_DIV_1_5MHZ: freq = "1.5 MHz"; break;
    case MCP_CLKO_DIV_3MHZ:   freq = "3 MHz"; break;
    case MCP_CLKO_DIV_6MHZ:   freq = "6 MHz"; break;
    case MCP_CLKO_DIV_12MHZ:  freq = "12 MHz"; break;
    case MCP_CLKO_DIV_24MHZ:  freq = "24 MHz"; break;
    case MCP_CLKO_DIV_OFF:    freq = "0 MHz"; break;
    }
    switch (settings->config1.clko_dc) {
    case MCP_CLKO_DC_75:      duty = "75%"; break;
    case MCP_CLKO_DC_50:      duty = "50%"; break;
    case MCP_CLKO_DC_25:      duty = "25%"; break;
    case MCP_CLKO_DC_0:       duty = "0%"; break;
    }
    printf("Clock Output: %s, duty cycle %s\n", freq, duty);

    if (trace_flag) {
        if (settings->config0.lock) {
            printf("Chip security: Permanently locked\n");
        } else if (settings->config0.password) {
            printf("Chip security: Password-protected\n");
        }
        if (settings->config0.usbcfg)
            printf("Initial USBCFG pin: 1\n");
        if (settings->config0.sspnd)
            printf("Initial SSPND pin: 1\n");
        if (settings->config0.ledi2c)
            printf("Initial LEDI2C pin: 1\n");
        if (settings->config0.leduarttx)
            printf("Initial LEDUARTTX pin: 1\n");
        if (settings->config0.leduartrx)
            printf("Initial LEDUARTRX pin: 1\n");

        printf("Power-Up DAC Value: %d\n", settings->config2.dac_power_up);
        if (settings->config2.dac_ref_en)
            printf("DAC Reference voltage: %s\n",
                settings->config2.dac_ref_sel == MCP_REF_4096 ? "4.096V" :
                settings->config2.dac_ref_sel == MCP_REF_2048 ? "2.048V" :
                settings->config2.dac_ref_sel == MCP_REF_1024 ? "1.024V" :
                "Off");
        else
            printf("DAC Reference voltage: Vdd\n");

        if (settings->config3.adc_ref_en)
            printf("ADC Reference voltage: %s\n",
                settings->config3.adc_ref_sel == MCP_REF_4096 ? "4.096V" :
                settings->config3.adc_ref_sel == MCP_REF_2048 ? "2.048V" :
                settings->config3.adc_ref_sel == MCP_REF_1024 ? "1.024V" :
                "Off");
        else
            printf("ADC Reference voltage: Vdd\n");
        if (settings->config3.intr_pos & settings->config3.intr_neg)
            printf("Interrupt Detection: Positive, Negative\n");
        else if (settings->config3.intr_pos)
            printf("Interrupt Detection: Positive\n");
        else if (settings->config3.intr_neg)
            printf("Interrupt Detection: Negative\n");
    }
}

static void mcp_print_gpio_settings(mcp_gpio_config_t *cfg, int index)
{
    if (cfg->function == 0) {
        if (cfg->dir_input) {
            printf("GP%d pin: Input\n", index);
        } else {
            printf("GP%d pin: Output %d\n", index, cfg->output_val);
        }
    } else {
        switch (index) {
        case 0:
            switch (cfg->function) {
            case 1:  printf("GP%d pin: SSPND Output\n", index); break;
            case 2:  printf("GP%d pin: LED UART RX Output\n", index); break;
            default: printf("GP%d pin: Unknown Function %d\n", index, cfg->function); break;
            }
            break;
        case 1:
            switch (cfg->function) {
            case 1:  printf("GP%d pin: Clock Output\n", index); break;
            case 2:  printf("GP%d pin: ADC1 Input\n", index); break;
            case 3:  printf("GP%d pin: LED UART TX Output\n", index); break;
            case 4:  printf("GP%d pin: Interrupt Detection Input\n", index); break;
            default: printf("GP%d pin: Unknown Function %d\n", index, cfg->function); break;
            }
            break;
        case 2:
            switch (cfg->function) {
            case 1:  printf("GP%d pin: USBCFG Output\n", index); break;
            case 2:  printf("GP%d pin: ADC2 Input\n", index); break;
            case 3:  printf("GP%d pin: DAC1 Output\n", index); break;
            default: printf("GP%d pin: Unknown Function %d\n", index, cfg->function); break;
            }
            break;
        case 3:
            switch (cfg->function) {
            case 1:  printf("GP%d pin: LED I2C Output\n", index); break;
            case 2:  printf("GP%d pin: ADC3 Input\n", index); break;
            case 3:  printf("GP%d pin: DAC2 Output\n", index); break;
            default: printf("GP%d pin: Unknown Function %d\n", index, cfg->function); break;
            }
            break;
        default:
            printf("%s: Unknown GP%d pin!\n", __func__, index);
            exit(1);
        }
    }
}

//
// Write Unicode symbol to file.
// Convert to UTF-8 encoding:
// 00000000.0xxxxxxx -> 0xxxxxxx
// 00000xxx.xxyyyyyy -> 110xxxxx, 10yyyyyy
// xxxxyyyy.yyzzzzzz -> 1110xxxx, 10yyyyyy, 10zzzzzz
//
void putc_utf8(unsigned short ch, FILE *out)
{
    if (ch < 0x80) {
        putc(ch, out);
    } else if (ch < 0x800) {
        putc(ch >> 6 | 0xc0, out);
        putc((ch & 0x3f) | 0x80, out);
    } else {
        putc(ch >> 12 | 0xe0, out);
        putc(((ch >> 6) & 0x3f) | 0x80, out);
        putc((ch & 0x3f) | 0x80, out);
    }
}

//
// Print utf16 text as utf8.
//
static void mcp_print_unicode(const char *title, const unsigned char *buf, unsigned nchars)
{
    const unsigned short *text = (const unsigned short*) buf;
    unsigned i;

    printf("%s: ", title);
    for (i=0; i<nchars; i++) {
        unsigned ch = *text++;
        if (!ch)
            break;
        putc_utf8(ch, stdout);
    }
    printf("\n");
}

//
// Print ASCII string.
//
static void mcp_print_ascii(const char *title, const unsigned char *text, unsigned nchars)
{
    unsigned i;

    printf("%s: ", title);
    for (i=0; i<nchars; i++) {
        unsigned ch = *text++;
        if (!ch)
            break;
        putc(ch, stdout);
    }
    printf("\n");
}

static void mcp_print_gpio(mcp_reply_gpio_t *gpio)
{
    printf("GP0 pin: %s %d\n", gpio->gp0_direction == 0 ? "Output" :
        gpio->gp0_direction == 1 ? "Input" : "Unused", gpio->gp0_pin);
    printf("GP1 pin: %s %d\n", gpio->gp1_direction == 0 ? "Output" :
        gpio->gp1_direction == 1 ? "Input" : "Unused", gpio->gp1_pin);
    printf("GP2 pin: %s %d\n", gpio->gp2_direction == 0 ? "Output" :
        gpio->gp2_direction == 1 ? "Input" : "Unused", gpio->gp2_pin);
    printf("GP3 pin: %s %d\n", gpio->gp3_direction == 0 ? "Output" :
        gpio->gp3_direction == 1 ? "Input" : "Unused", gpio->gp3_pin);
}

//
// Read information from MCP2221 chip.
//
static void mcp_download()
{
    //
    // Get chip status.
    //
    unsigned char get_status[1]  = { MCP_CMD_STATUSSET };
    mcp_reply_status_t status;
    hid_send_recv(get_status, sizeof(get_status), &status, sizeof(status));
    if (status.command_code != get_status[0] ||
        status.status != 0)
    {
        fprintf(stderr, "Bad reply from STATUSSET request!\n");
        exit(-1);
    }
    mcp_print_status(&status);

    //
    // Get Flash data: chip settings.
    //
    unsigned char get_chip_settings[2]  = { MCP_CMD_READFLASH, MCP_FLASH_CHIPSETTINGS };
    mcp_reply_chip_settings_t chip_settings;
    hid_send_recv(get_chip_settings, sizeof(get_chip_settings), &chip_settings, sizeof(chip_settings));
    if (chip_settings.command_code != get_chip_settings[0] ||
        chip_settings.status != 0 ||
        chip_settings.nbytes + 4 != sizeof(chip_settings))
    {
        fprintf(stderr, "Bad reply from READFLASH CHIPSETTINGS request!\n");
        exit(-1);
    }
    printf("--- Flash ---\n");
    mcp_print_chip_settings(&chip_settings);

    //
    // Get Flash data: GPIO settings.
    //
    unsigned char get_gpio_settings[2]  = { MCP_CMD_READFLASH, MCP_FLASH_GPIOSETTINGS };
    mcp_reply_gpio_settings_t gpio_settings;
    hid_send_recv(get_gpio_settings, sizeof(get_gpio_settings), &gpio_settings, sizeof(gpio_settings));
    if (gpio_settings.command_code != get_gpio_settings[0] ||
        gpio_settings.status != 0 ||
        gpio_settings.nbytes + 4 != sizeof(gpio_settings))
    {
        fprintf(stderr, "Bad reply from READFLASH GPIOSETTINGS request!\n");
        exit(-1);
    }
    mcp_print_gpio_settings(&gpio_settings.gp0, 0);
    mcp_print_gpio_settings(&gpio_settings.gp1, 1);
    mcp_print_gpio_settings(&gpio_settings.gp2, 2);
    mcp_print_gpio_settings(&gpio_settings.gp3, 3);

    //
    // Get Flash data: USB Manufacturer string.
    //
    unsigned char get_usb_manufecturer[2]  = { MCP_CMD_READFLASH, MCP_FLASH_USBMANUFACTURER };
    unsigned char reply[64];
    hid_send_recv(get_usb_manufecturer, sizeof(get_usb_manufecturer), reply, sizeof(reply));
    if (reply[0] != get_usb_manufecturer[0] ||
        reply[1] != 0 ||
        reply[2] + 2 > sizeof(reply) ||
        reply[3] != 3)
    {
        fprintf(stderr, "Bad reply from READFLASH USBMANUFACTURER request!\n");
        exit(-1);
    }
    mcp_print_unicode("USB Manufacturer", &reply[4], reply[2] / 2 - 1);

    unsigned char get_usb_product[2]  = { MCP_CMD_READFLASH, MCP_FLASH_USBPRODUCT };
    hid_send_recv(get_usb_product, sizeof(get_usb_product), reply, sizeof(reply));
    if (reply[0] != get_usb_manufecturer[0] ||
        reply[1] != 0 ||
        reply[2] + 2 > sizeof(reply) ||
        reply[3] != 3)
    {
        fprintf(stderr, "Bad reply from READFLASH USBPRODUCT request!\n");
        exit(-1);
    }
    mcp_print_unicode("USB Product", &reply[4], reply[2] / 2 - 1);

    unsigned char get_usb_serial[2]  = { MCP_CMD_READFLASH, MCP_FLASH_USBSERIAL };
    hid_send_recv(get_usb_serial, sizeof(get_usb_serial), reply, sizeof(reply));
    if (reply[0] != get_usb_manufecturer[0] ||
        reply[1] != 0 ||
        reply[2] + 2 > sizeof(reply) ||
        reply[3] != 3)
    {
        fprintf(stderr, "Bad reply from READFLASH USBSERIAL request!\n");
        exit(-1);
    }
    mcp_print_unicode("USB Serial", &reply[4], reply[2] / 2 - 1);

    unsigned char get_factory_serial[2]  = { MCP_CMD_READFLASH, MCP_FLASH_FACTORYSERIAL };
    hid_send_recv(get_factory_serial, sizeof(get_factory_serial), reply, sizeof(reply));
    if (reply[0] != get_usb_manufecturer[0] ||
        reply[1] != 0 ||
        reply[2] + 4 > sizeof(reply))
    {
        fprintf(stderr, "Bad reply from READFLASH FACTORYSERIAL request!\n");
        exit(-1);
    }
    mcp_print_ascii("Factory Serial", &reply[4], reply[2]);

    //
    // Get SRAM settings.
    //
    unsigned char get_sram[1] = { MCP_CMD_GETSRAM };
    mcp_reply_sram_data_t sram;
    hid_send_recv(get_sram, sizeof(get_sram), &sram, sizeof(sram));
    if (sram.command_code != get_sram[0] ||
        sram.status != 0 ||
        sram.nbytes_sram + sram.nbytes_gp + 4 != sizeof(sram))
    {
        fprintf(stderr, "Bad reply from GETSRAM request!\n");
        exit(-1);
    }
    printf("--- SRAM ---\n");
    mcp_print_chip_settings((mcp_reply_chip_settings_t*) &sram);
    printf("Password: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
        sram.password[0], sram.password[1], sram.password[2], sram.password[3],
        sram.password[4], sram.password[5], sram.password[6], sram.password[7]);

    mcp_print_gpio_settings(&sram.gp0, 0);
    mcp_print_gpio_settings(&sram.gp1, 1);
    mcp_print_gpio_settings(&sram.gp2, 2);
    mcp_print_gpio_settings(&sram.gp3, 3);

    //
    // Get GPIO values.
    //
    unsigned char get_gpio[1] = { MCP_CMD_GETGPIO };
    mcp_reply_gpio_t gpio;
    hid_send_recv(get_gpio, sizeof(get_gpio), &gpio, sizeof(gpio));
    if (gpio.command_code != get_gpio[0] ||
        gpio.status != 0)
    {
        fprintf(stderr, "Bad reply from GETGPIO request!\n");
        exit(-1);
    }
    printf("--- GPIO ---\n");
    mcp_print_gpio(&gpio);
}

int main(int argc, char **argv)
{
    int read_flag = 0;

    copyright = "Copyright (C) 2019 Serge Vakulenko";
    trace_flag = 0;
    for (;;) {
        switch (getopt(argc, argv, "tr")) {
        case 'r': ++read_flag;  continue;
        case 't': ++trace_flag;  continue;
        default:
            usage();
        case EOF:
            break;
        }
        break;
    }
    argc -= optind;
    argv += optind;
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IOLBF, 0);

    if (read_flag) {
        if (argc != 0)
            usage();

        mcp_connect();
        mcp_download();
        mcp_disconnect();
    } else {
        usage();
    }
    return 0;
}
