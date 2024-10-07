#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
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

void printl(long num) {
    char temp[32];

    sprintl(temp, num);
    printf("%s", temp);
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

void printlf(char *fmt, ...) {
    va_list(args);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            printl(va_arg(args, long));
        } else printf("%c", *source);
    }
    va_end(args);
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

void printpf(char *fmt, ...) {
    va_list(args);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            long value = va_arg(args, long);
            if (value < 1e6) printf("%.3lfK", (double)value / 1e3);
            else if (value < 1e9) printf("%.3lfM", (double)value / 1e6);
            else printf("%.3lfG", (double)value / 1e9);
        } else printf("%c", *source);
    }
    va_end(args);
}

void fprintpf(FILE *out, char *fmt, ...) {
    va_list(args);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            long value = va_arg(args, long);
            if (value < 1e6) fprintf(out, "%.3lfK", (double)value / 1e3);
            else if (value < 1e9) fprintf(out, "%.3lfM", (double)value / 1e6);
            else fprintf(out, "%.3lfG", (double)value / 1e9);
        } else fprintf(out, "%c", *source);
    }
    va_end(args);
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

void printtf(char *fmt, ...) {
    va_list(args);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            long sec = va_arg(args, long);
            long hours = sec / 3600;
            sec -= hours * 3600;
            long mins = sec / 60;
            sec -= mins * 60;
            if (hours > 0) printf("%ldh %02ldm %02lds", hours, mins, sec);
            else if (mins > 0) printf("%ldm %02lds", mins, sec);
            else printf("%lds", sec);
        } else printf("%c", *source);
    }
    va_end(args);
}

static char spinner[] = "|/-\\";

void fspin(FILE *out, long number) {
    static int spin = 0;

    fprintf(out, "%c", spinner[spin]);
    fprintlf(out, " %\r", number);
    fflush(out);
    if (++spin == 4) spin = 0;
}
