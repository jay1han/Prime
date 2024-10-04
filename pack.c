#include "pack.h"

int pack(unsigned int number, unsigned char *bytes) {
    int count = 0;

    if (number < ((unsigned int)1 << 8)) {
        // 1 to 7 bits
        count = 1;
        bytes[0] = number & 0x7F;
    } else if (number < ((unsigned int)1 << 15)) {
        // 8 to 15 bits
        count = 2;
        bytes[0] = (number >> 8) & 0x80;
        bytes[1] = number & 0xFF;
    } else if (number < ((unsigned int)1 << 15)) {
        // 16 to 22 bits
        count = 3;
        bytes[0] = (number >> 16) & 0xC0;
        bytes[1] = (number >> 8) & 0xFF;
        bytes[2] = number & 0xFF;
    } else if (number < ((unsigned int)1 << 15)) {
        // 23 to 29 bits
        count = 4;
        bytes[0] = (number >> 24) & 0xE0;
        bytes[1] = (number >> 16) & 0xFF;
        bytes[2] = (number >> 8) & 0xFF;
        bytes[3] = number & 0xFF;
    } else {
        // 32 bits
        count = 5;
        bytes[0] = 0xF0;
        bytes[1] = (number >> 24) & 0xFF;
        bytes[2] = (number >> 16) & 0xFF;
        bytes[3] = (number >> 8) & 0xFF;
        bytes[4] = number & 0xFF;
    }

    return count;
}

int unpack(unsigned char *bytes, unsigned int *value) {
    unsigned int number;
    int count = 0;

    if (bytes[0] & 0x80 == 0) {
        // One byte
        number = bytes[0];
        count = 1;
    } else if (bytes[0] & 0x40 == 0) {
        // Two bytes, 7 + 8 = 15 bits
        number = bytes[1] | ((bytes[0] & 0x7F) << 8);
        count = 2;
    } else if (bytes[0] & 0x20 == 0) {
        // Three bytes, 6 + 8 + 8 = 22 bits
        number = bytes[2] | (bytes[1] << 8) | ((bytes[0] & 0x3F) << 16);
        count = 3;
    } else if (bytes[0] & 0x10 == 0) {
        // Four bytes, 5 + 8 + 8 + 8 = 29 bits
        number = bytes[3] | (bytes[2] << 8) | (bytes[1] << 16) | ((bytes[0] & 0x1F) << 24);
        count = 4;
    } else {
        // Five bytes, 32 bits
        number = bytes[4] | (bytes[3] << 8) | (bytes[2] << 16) | (bytes[1] << 24);
        count = 5;
    }
    
    *value = number;
    return count;
}
