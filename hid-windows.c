/*
 * HID routines for Windows.
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
#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "util.h"

HANDLE dev = INVALID_HANDLE_VALUE;          // HID device
static unsigned char receive_buf[64];       // receive buffer

//
// Send a request to the device.
// Store the reply into the rdata[] array.
// Terminate in case of errors.
//
void hid_send_recv(const unsigned char *data, unsigned nbytes, unsigned char *rdata, unsigned rlength)
{
    unsigned char buf[64];
    unsigned k;
    DWORD nbytes_received;

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

    // Write to HID device.
    if (!WriteFile(dev, buf, sizeof(buf), NULL, NULL)) {
        fprintf(stderr, "Error %#lx sending to HID device!\n", GetLastError());
        exit(-1);
    }

    // Receive reply.
    if (!ReadFile(dev, receive_buf, sizeof(receive_buf), &nbytes_received, NULL)) {
        fprintf(stderr, "Error %#lx receiving from HID device!\n", GetLastError());
        exit(-1);
    }

    if (nbytes_received != sizeof(receive_buf)) {
        fprintf(stderr, "Short read: %u bytes instead of %u!\n",
            (unsigned)nbytes_received, (unsigned)sizeof(receive_buf));
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
// Open the radio in programming mode.
// Find a HID device with given GUID, vendor ID and product ID.
// Setup `dev' file descriptor.
//
int hid_init(int vid, int pid)
{
    static GUID guid = { 0x4d1e55b2, 0xf16f, 0x11cf, { 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

    HDEVINFO devinfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    if (devinfo == INVALID_HANDLE_VALUE) {
        printf("Cannot get devinfo!\n");
        return -1;
    }

    // Loop through available devices with a given GUID.
    int index;
    SP_INTERFACE_DEVICE_DATA iface;
    iface.cbSize = sizeof(iface);
    dev = INVALID_HANDLE_VALUE;
    for (index=0; SetupDiEnumDeviceInterfaces(devinfo, NULL, &guid, index, &iface); ++index) {

        // Obtain a required size of device detail structure.
        DWORD needed;
        SetupDiGetDeviceInterfaceDetail(devinfo, &iface, NULL, 0, &needed, NULL);

        // Allocate the device detail structure.
        PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)alloca(needed);
        detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
        SP_DEVINFO_DATA did = { sizeof(SP_DEVINFO_DATA) };

        // Get device information.
        if (!SetupDiGetDeviceInterfaceDetail(devinfo, &iface, detail, needed, NULL, &did)) {
            printf("Device %d: cannot get path!\n", index);
            continue;
        }
        //printf("Device %d: path %s\n", index, detail->DevicePath);

        dev = CreateFile(detail->DevicePath, GENERIC_WRITE | GENERIC_READ,
            0, NULL, OPEN_EXISTING, 0, NULL);
        if (dev == INVALID_HANDLE_VALUE) {
            continue;
        }

        // Get the Vendor ID and Product ID for this device.
        HIDD_ATTRIBUTES attrib;
        attrib.Size = sizeof(HIDD_ATTRIBUTES);
        HidD_GetAttributes(dev, &attrib);
        //printf("Vendor/Product: %04x %04x\n", attrib.VendorID, attrib.ProductID);

        // Check the VID/PID.
        if (attrib.VendorID != vid || attrib.ProductID != pid) {
            CloseHandle(dev);
            dev = INVALID_HANDLE_VALUE;
            continue;
        }

        // Required device found.
        break;
    }
    SetupDiDestroyDeviceInfoList(devinfo);

    if (dev == INVALID_HANDLE_VALUE) {
        if (trace_flag) {
            fprintf(stderr, "Cannot find HID device %04x:%04x\n", vid, pid);
        }
        return -1;
    }
    return 0;
}

//
// Close HID device.
//
void hid_close()
{
    if (dev != INVALID_HANDLE_VALUE) {
        CloseHandle(dev);
        dev = INVALID_HANDLE_VALUE;
    }
}
