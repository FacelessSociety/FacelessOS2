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

#include <arch/pci/pci.h>
#include <arch/io/io.h>

#define CONFIG_ADDR 0xCF8
#define CONFIG_DATA 0xCFC


uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    // Convert args into long format.
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    // Create config address.
    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out address.
    outportl(CONFIG_ADDR, address);

    // Get data.
    uint16_t tmp = (uint16_t)((inportl(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}


uint16_t pci_get_device_vendorid(uint8_t bus, uint8_t slot, uint8_t func) {
    return pci_read_word(bus, slot, func, 0);
}


uint16_t pci_get_device_id(uint8_t bus, uint8_t slot, uint8_t func) {
    return pci_read_word(bus, slot, func, 0x2);
}


uint8_t pci_get_device_class(uint8_t bus, uint8_t slot, uint8_t func) {
    return pci_read_word(bus, slot, func, 0xA) >> 8;
}


uint8_t pci_get_device_subclass(uint8_t bus, uint8_t slot, uint8_t func) {
    return pci_read_word(bus, slot, func, 0xA);
}
