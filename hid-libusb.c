/*
 * HID routines for Linux, via libusb-1.0.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb.h>
#include "util.h"

static libusb_context *ctx = NULL;          // libusb context
static libusb_device_handle *dev;           // libusb device

#define HID_INTERFACE       2               // HID interface index
#define TIMEOUT_MSEC        500             // receive timeout
#define BULK_WRITE_ENDPOINT 0x03            // output to HID device
#define BULK_READ_ENDPOINT  0x83            // input from HID device

//
// Perform USB bulk transfer.
// Return a number of transferred bytes, or -1 in case of error.
//
static int bulk_transfer(uint8_t type, uint8_t *data, int length)
{
    int result, transfered, retry;

    for (retry = 0; retry < 10; retry++) {
        result = libusb_bulk_transfer(dev, type, data, length, &transfered, TIMEOUT_MSEC);
        if (result >= 0)
            return transfered;

        if (result != LIBUSB_ERROR_PIPE)
            break;

        // Sometimes the chip does not recognize the command, for unknown reason.
        // Need to repeat.
        usleep(10000);
    }
    fprintf(stderr, "%s: Failed to %s %d bytes '%s'\n", __func__,
            (type == BULK_WRITE_ENDPOINT) ? "write" : "read", length,
            libusb_error_name(result));
    return -1;
}

//
// Send a request to the device.
// Store the reply into the rdata[] array.
// Terminate in case of errors.
//
void hid_send_recv(const unsigned char *data, unsigned nbytes, void *rdata, unsigned rlength)
{
    unsigned char buf[64];
    unsigned char reply[64];
    unsigned k;

    memset(buf, 0, sizeof(buf));
    if (nbytes > 0)
        memcpy(buf, data, nbytes);

    if (trace_flag > 0) {
        fprintf(stderr, "---Send");
        for (k=0; k<nbytes; ++k) {
            if (k != 0 && (k & 15) == 0)
                fprintf(stderr, "\n       ");
            fprintf(stderr, " %02x", buf[k]);
        }
        fprintf(stderr, "\n");
    }

    // Send request to the device.
    if (bulk_transfer(BULK_WRITE_ENDPOINT, buf, sizeof(buf)) < 0) {
        fprintf(stderr, "Fatal write error!\n");
        exit(-1);
    }

    // Get reply.
    memset(reply, 0, sizeof(reply));
    int reply_len = bulk_transfer(BULK_READ_ENDPOINT, reply, sizeof(reply));
    if (reply_len < 0) {
        exit(-1);
    }
    if (reply_len != sizeof(reply)) {
        fprintf(stderr, "Short read: %d bytes instead of %d!\n",
            reply_len, (int)sizeof(reply));
        exit(-1);
    }
    if (trace_flag > 0) {
        fprintf(stderr, "---Recv");
        for (k=0; k<reply_len; ++k) {
            if (k != 0 && (k & 15) == 0)
                fprintf(stderr, "\n       ");
            fprintf(stderr, " %02x", reply[k]);
        }
        fprintf(stderr, "\n");
    }
    memcpy(rdata, reply, rlength);
}

//
// Connect to the specified device.
// Initiate the programming session.
//
int hid_init(int vid, int pid)
{
    int error = libusb_init(&ctx);
    if (error < 0) {
        fprintf(stderr, "libusb init failed: %d: %s\n",
            error, libusb_strerror(error));
        exit(-1);
    }

    dev = libusb_open_device_with_vid_pid(ctx, vid, pid);
    if (!dev) {
        if (trace_flag) {
            fprintf(stderr, "Cannot find USB device %04x:%04x\n",
                vid, pid);
        }
        libusb_exit(ctx);
        ctx = 0;
        return -1;
    }
    if (libusb_kernel_driver_active(dev, HID_INTERFACE)) {
        libusb_detach_kernel_driver(dev, HID_INTERFACE);
    }

    error = libusb_claim_interface(dev, HID_INTERFACE);
    if (error < 0) {
        fprintf(stderr, "Failed to claim USB interface: %d: %s\n",
            error, libusb_strerror(error));
        libusb_close(dev);
        libusb_exit(ctx);
        ctx = 0;
        exit(-1);
    }
    return 0;
}

void hid_close()
{
    if (!ctx)
        return;

    libusb_release_interface(dev, HID_INTERFACE);
    libusb_close(dev);
    libusb_exit(ctx);
    ctx = 0;
}
