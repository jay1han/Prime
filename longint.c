#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include "longint.h"

int sprintl(char *output, long num) {
    char temp[32];
    int length, spaces;
  
    sprintf(temp, "%lu", num);
    length = strlen(temp);
    spaces = (length - 1) / 3;

    temp[length + spaces] = 0;
    for (int i = 1; i <= length; i++) {
        temp[length + spaces - i] = temp[length - i];
        if (i % 3 == 0) {
            spaces--;
            if (spaces >= 0) temp[length + spaces - i] = '_';
        }
    }
    
    strcpy(output, temp);
    return strlen(temp);
}

void fprintl(FILE *out, long num) {
    char temp[32];

    sprintl(temp, num);
    fprintf(out, "%s", temp);
}

void sprintlf(char *output, char *fmt, ...) {
    va_list(args);
    char *target = output + strlen(output);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            target += sprintl(target, va_arg(args, long));
        } else *(target++) = *source;
    }
    va_end(args);
    
    *target = 0;
}

void fprintlf(FILE *out, char *fmt, ...) {
    va_list(args);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            fprintl(out, va_arg(args, long));
        } else fprintf(out, "%c", *source);
    }
    va_end(args);
}

void fprintp(FILE *out, long value) {
    if (value < 1e6) fprintf(out, "%.3lfK", (double)value / 1e3);
    else if (value < 1e9) fprintf(out, "%.3lfM", (double)value / 1e6);
    else fprintf(out, "%.3lfG", (double)value / 1e9);
}

void sscanl(char *input, long *value) {
    char temp[32];
    char *source, *target = temp;
    long number = 0;

    for (source = input; *source != 0; source++) {
        if (*source >= '0' && *source <= '9') break;
    }

    for (; (*source >= '0' && *source <= '9') || *source == '_'; source++) {
        if (*source == '_') continue;
        number = number * 10L + (*source - '0');
    }

    *value = number;
}

void fprintt(FILE *out, long sec) {
    long hours = sec / 3600;
    sec -= hours * 3600;
    long mins = sec / 60;
    sec -= mins * 60;
    if (hours > 0) fprintf(out, "%ldh %02ldm %02lds", hours, mins, sec);
    else if (mins > 0) fprintf(out, "%ldm %02lds", mins, sec);
    else fprintf(out, "%lds", sec);
}

double d_since(struct timeval *since) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (double)(now.tv_sec - since->tv_sec)
        + (double)(now.tv_usec - since->tv_usec) / 1e6;
}
