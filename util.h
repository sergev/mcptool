/*
 * Auxiliary functions.
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

//
// Program version.
//
extern const char version[];
extern const char *copyright;

//
// Trace data i/o via the serial port.
//
int trace_flag;

//
// HID functions.
//
int hid_init(int vid, int pid);
const char *hid_identify(void);
void hid_close(void);
void hid_send_recv(const unsigned char *data, unsigned nbytes, void *rdata, unsigned rlength);
void hid_read_block(int bno, unsigned char *data, int nbytes);
void hid_read_finish(void);
void hid_write_block(int bno, unsigned char *data, int nbytes);
void hid_write_finish(void);
