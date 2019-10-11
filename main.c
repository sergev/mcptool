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

static void mcp_print_status(mcp_reply_statusset_t *status)
{
    printf("Hardware Revision: %c%c\n", status->hardware_rev_major, status->hardware_rev_minor);
    printf("Firmware Revision: %c.%c\n", status->firmware_rev_major, status->firmware_rev_minor);
    //printf("SCL Input: %d\n", status->scl_input);
    //printf("SDA Input: %d\n", status->sda_input);
    //printf("Interrupt Edge: %d\n", status->intr_edge);
    //printf("ADC Channel 0 Input: %d\n", status->adc_ch0);
    //printf("ADC Channel 1 Input: %d\n", status->adc_ch1);
    //printf("ADC Channel 2 Input: %d\n", status->adc_ch2);
}

//
// Read information from MCP2221 chip.
//
static void mcp_download()
{
    //TODO
    unsigned char cmd_status[1]  = { MCP_CMD_STATUSSET };
    unsigned char cmd_getsram[1] = { MCP_CMD_GETSRAM };
    mcp_reply_statusset_t status;
    unsigned char reply[64];

    hid_send_recv(cmd_status, sizeof(cmd_status), &status, sizeof(status));
    if (status.command_code != MCP_CMD_STATUSSET || status.status != 0) {
        fprintf(stderr, "Bad reply from STATUSSET request!\n");
        exit(-1);
    }
    mcp_print_status(&status);

    hid_send_recv(cmd_getsram, sizeof(cmd_getsram), reply, sizeof(reply));
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
