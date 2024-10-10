#ifndef _LONGINT_H_
#define _LONGINT_H_

// Print longs
void fprintl(FILE *out, long num);
void fprintlf(FILE *out, char *fmt, ...);
void fprintp(FILE *out, long value);
int sprintl(char *output, long num);
void sprintlf(char *output, char *fmt, ...);

// Print time
void fprintt(FILE *out, time_t seconds);

// Scan a punctuated long
void sscanl(char *input, long *value);

// Microseconds
double d_since(struct timeval *since);

#endif
