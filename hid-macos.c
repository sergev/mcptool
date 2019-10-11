/*
 * HID routines for Mac OS X.
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
#include <string.h>
#include <unistd.h>
#include <IOKit/hid/IOHIDManager.h>
#include "util.h"

static volatile IOHIDDeviceRef dev;         // device handle
static unsigned char transfer_buf[64];      // device buffer
static unsigned char receive_buf[64];       // receive buffer
static volatile int nbytes_received = 0;    // receive result

//
// Send a request to the device.
// Store the reply into the rdata[] array.
// Terminate in case of errors.
//
void hid_send_recv(const unsigned char *data, unsigned nbytes, void *rdata, unsigned rlength)
{
    unsigned char buf[64];
    unsigned k;
    IOReturn result;

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
    nbytes_received = 0;
    memset(receive_buf, 0, sizeof(receive_buf));
again:
    // Write to HID device.
    result = IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, 0, buf, sizeof(buf));
    if (result != kIOReturnSuccess) {
        fprintf(stderr, "HID output error: %d!\n", result);
        exit(-1);
    }

    // Run main application loop until reply received.
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, 0);
    for (k = 0; nbytes_received <= 0; k++) {
        usleep(100);
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, 0);
        if (k >= 1000) {
            if (trace_flag > 0) {
                fprintf(stderr, "No response from HID device!\n");
            }
            goto again;
        }
    }

    if (nbytes_received != sizeof(receive_buf)) {
        fprintf(stderr, "Short read: %d bytes instead of %d!\n",
            nbytes_received, (int)sizeof(receive_buf));
        exit(-1);
    }
    if (trace_flag > 0) {
        fprintf(stderr, "---Recv");
        for (k=0; k<nbytes_received; ++k) {
            if (k != 0 && (k & 15) == 0)
                fprintf(stderr, "\n       ");
            fprintf(stderr, " %02x", receive_buf[k]);
        }
        fprintf(stderr, "\n");
    }
    memcpy(rdata, receive_buf, rlength);
}

//
// Callback: data is received from the HID device
//
static void callback_input(void *context,
    IOReturn result, void *sender, IOHIDReportType type,
    uint32_t reportID, uint8_t *data, CFIndex nbytes)
{
    if (result != kIOReturnSuccess) {
        fprintf(stderr, "HID input error: %d!\n", result);
        exit(-1);
    }

    if (nbytes > sizeof(receive_buf)) {
        fprintf(stderr, "Too large HID input: %d bytes!\n", (int)nbytes);
        exit(-1);
    }

    nbytes_received = nbytes;
    if (nbytes > 0)
        memcpy(receive_buf, data, nbytes);
}

//
// Callback: device specified in the matching dictionary has been added
//
static void callback_open(void *context,
    IOReturn result, void *sender, IOHIDDeviceRef deviceRef)
{
    IOReturn o = IOHIDDeviceOpen(deviceRef, kIOHIDOptionsTypeSeizeDevice);
    if (o != kIOReturnSuccess) {
        fprintf(stderr, "Cannot open HID device!\n");
        exit(-1);
    }

    // Register input callback.
    IOHIDDeviceRegisterInputReportCallback(deviceRef,
        transfer_buf, sizeof(transfer_buf), callback_input, 0);

    dev = deviceRef;
}

//
// Callback: device specified in the matching dictionary has been removed
//
static void callback_close(void *ontext,
    IOReturn result, void *sender, IOHIDDeviceRef deviceRef)
{
    // De-register input callback.
    IOHIDDeviceRegisterInputReportCallback(deviceRef, transfer_buf, sizeof(transfer_buf), NULL, NULL);
}

//
// Launch the IOHIDManager.
//
int hid_init(int vid, int pid)
{
    // Create the USB HID Manager.
    IOHIDManagerRef HIDManager = IOHIDManagerCreate(kCFAllocatorDefault,
                                                    kIOHIDOptionsTypeNone);

    // Create an empty matching dictionary for filtering USB devices in our HID manager.
    CFMutableDictionaryRef matchDict = CFDictionaryCreateMutable(kCFAllocatorDefault,
        2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    // Specify the USB device manufacturer and product in our matching dictionary.
    CFDictionarySetValue(matchDict, CFSTR(kIOHIDVendorIDKey), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vid));
    CFDictionarySetValue(matchDict, CFSTR(kIOHIDProductIDKey), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pid));

    // Apply the matching to our HID manager.
    IOHIDManagerSetDeviceMatching(HIDManager, matchDict);
    CFRelease(matchDict);

    // The HID manager will use callbacks when specified USB devices are connected/disconnected.
    IOHIDManagerRegisterDeviceMatchingCallback(HIDManager, &callback_open, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(HIDManager, &callback_close, NULL);

    // Add the HID manager to the main run loop
    IOHIDManagerScheduleWithRunLoop(HIDManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);

    // Open the HID mangager
    IOReturn IOReturn = IOHIDManagerOpen(HIDManager, kIOHIDOptionsTypeNone);
    if (IOReturn != kIOReturnSuccess) {
        if (trace_flag) {
            fprintf(stderr, "Cannot find USB device %04x:%04x\n", vid, pid);
        }
        return -1;
    }

    // Run main application loop until device found.
    int k;
    for (k=0; ; k++) {
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, 0);
        if (dev)
            return 0;

        if (k >= 3) {
            if (trace_flag) {
                fprintf(stderr, "Cannot find USB device %04x:%04x\n", vid, pid);
            }
            return -1;
        }
        usleep(10000);
    }
}

//
// Close HID device.
//
void hid_close()
{
    if (!dev)
        return;

    IOHIDDeviceClose(dev, kIOHIDOptionsTypeNone);
    dev = 0;
}
