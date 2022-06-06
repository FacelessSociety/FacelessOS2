/*
 *  MIT License
 *
 *  Copyright (c) 2022 FacelessSociety, Ian Marco Moffett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */



#include <debug/log.h>
#include <libkern/string.h>
#include <stddef.h>
#include <stdint.h>

static void(*term_write)(const char* string, size_t length);

void log_init(void* _term_write) {
    term_write = _term_write;
}

void kwrite(const char* str) {
    term_write(str, strlen(str));
}


void log(char* fmt, ...) {
    // Setup the arg pointer.
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    uint8_t* ptr;

    size_t pos = 0;

    // Start looping.
    for (ptr = fmt; *ptr != '\0' && pos < strlen(fmt); ++ptr, ++pos) {
        if (*ptr == '%') {
            ++ptr;

            switch (*ptr++) {
                case 's':
                    kwrite(va_arg(arg_ptr, char*));
                    break;
                case 'd':
                    kwrite(dec2str(va_arg(arg_ptr, int)));
                    break;
                case 'x':
                    kwrite(hex2str(va_arg(arg_ptr, int)));
                    break;
                case 'c':
                    {
                        char terminated[2] = {va_arg(arg_ptr, int), 0};
                        kwrite(terminated);
                        break;
                    }
            }
        }

        char terminated[2] = {*ptr, 0};
        kwrite(terminated);
    }

}
