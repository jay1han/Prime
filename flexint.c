#include "flexint.h"

typedef struct flexint_s {
    long number;
    unsigned char flex[9];
    int size;
} flexint_t;

inline int flex_fold(long number, unsigned char *bytes) {
    int count = 0;

    if (number < ((long)1 << 7)) {
        // 1 to 7 bits
        count = 1;
        bytes[0] = number;
    } else if (number < ((long)1 << 14)) {
        // 8 to 14 bits
        count = 2;
        bytes[0] = (number >> 8) | 0x80;
        bytes[1] = number & 0xFF;
    } else if (number < ((long)1 << 21)) {
        // 15 to 21 bits
        count = 3;
        bytes[0] = (number >> 16) | 0xC0;
        bytes[1] = (number >> 8) & 0xFF;
        bytes[2] = number & 0xFF;
    } else if (number < ((long)1 << 28)) {
        // 22 to 28 bits
        count = 4;
        bytes[0] = (number >> 24) | 0xE0;
        bytes[1] = (number >> 16) & 0xFF;
        bytes[2] = (number >> 8) & 0xFF;
        bytes[3] = number & 0xFF;
    } else if (number < ((long)1 << 35)) {
        // 29 to 35 bits
        count = 5;
        bytes[0] = (number >> 32) | 0xF0;
        bytes[1] = (number >> 24) & 0xFF;
        bytes[2] = (number >> 16) & 0xFF;
        bytes[3] = (number >> 8) & 0xFF;
        bytes[4] = number & 0xFF;
    } else if (number < ((long)1 << 42)) {
        // 36 to 42 bits
        count = 6;
        bytes[0] = (number >> 40) | 0xF8;
        bytes[1] = (number >> 32) & 0xFF;
        bytes[2] = (number >> 24) & 0xFF;
        bytes[3] = (number >> 16) & 0xFF;
        bytes[4] = (number >> 8) & 0xFF;
        bytes[5] = number & 0xFF;
    } else if (number < ((long)1 << 49)) {
        // 43 to 49 bits
        count = 7;
        bytes[0] = (number >> 48) | 0xFC;
        bytes[1] = (number >> 40) & 0xFF;
        bytes[2] = (number >> 32) & 0xFF;
        bytes[3] = (number >> 24) & 0xFF;
        bytes[4] = (number >> 16) & 0xFF;
        bytes[5] = (number >> 8) & 0xFF;
        bytes[6] = number & 0xFF;
    } else {
        // 50 to 64 bits
        count = 9;
        bytes[0] = 0xFE;
        bytes[1] = (number >> 56) & 0xFF;
        bytes[2] = (number >> 48) & 0xFF;
        bytes[3] = (number >> 40) & 0xFF;
        bytes[4] = (number >> 32) & 0xFF;
        bytes[5] = (number >> 24) & 0xFF;
        bytes[6] = (number >> 16) & 0xFF;
        bytes[7] = (number >> 8) & 0xFF;
        bytes[8] = number & 0xFF;
    }

    return count;
}

inline int flex_open(unsigned char *bytes, long *value) {
    long number;
    int count = 0;

    if ((bytes[0] & 0x80) == 0) {
        // One byte, 7 bits
        number = bytes[0] & 0x7F;
        count = 1;
    } else if ((bytes[0] & 0x40) == 0) {
        // Two bytes, 6 + 8 = 14 bits
        number = bytes[1] | ((long)(bytes[0] & 0x3F) << 8);
        count = 2;
    } else if ((bytes[0] & 0x20) == 0) {
        // Three bytes, 5 + 8 + 8 = 21 bits
        number = bytes[2] | ((long)bytes[1] << 8) | ((long)(bytes[0] & 0x1F) << 16);
        count = 3;
    } else if ((bytes[0] & 0x10) == 0) {
        // Four bytes, 4 + 8 + 8 + 8 = 28 bits
        number = bytes[3] | ((long)bytes[2] << 8) | ((long)bytes[1] << 16) | (((long)bytes[0] & 0x0F) << 24);
        count = 4;
    } else if ((bytes[0] & 0x08) == 0) {
        // 5 bytes, 3 + 4 * 8 = 35 bits
        number = bytes[4] | ((long)bytes[3] << 8) | ((long)bytes[2] << 16) | ((long)bytes[1] << 24) | (((long)bytes[0] & 0x07) << 32);
        count = 5;
    } else if ((bytes[0] & 0x04) == 0) {
        // 6 bytes, 2 + 5 * 8 = 42 bits
        number = bytes[5] | ((long)bytes[4] << 8) | ((long)bytes[3] << 16) | ((long)bytes[2] << 24) | ((long)bytes[1] << 32) | (((long)bytes[0] & 0x03) << 40);
        count = 6;
    } else if ((bytes[0] & 0x02) == 0) {
        // 7 bytes, 1 + 6 * 8 = 49 bits
        number = bytes[6] | ((long)bytes[5] << 8) | ((long)bytes[4] << 16) | ((long)bytes[3] << 24) | ((long)bytes[2] << 32) | ((long)bytes[1] << 40) | (((long)bytes[0] & 0x01) << 48);
        count = 7;
    } else {
        // 9 bytes, 64 bits
        number = bytes[8] | ((long)bytes[7] << 8) | ((long)bytes[6] << 16) | ((long)bytes[5] << 24) | ((long)bytes[4] << 32) | ((long)bytes[3] << 40) | ((long)bytes[2] << 48) | ((long)bytes[1] << 56);
        count = 9;
    }
    
    *value = number;
    return count;
}
