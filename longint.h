#ifndef _LONGINT_H_
#define _LONGINT_H_

// Print longs
void fprintl(FILE *out, long num);
void fprintlf(FILE *out, char *fmt, ...);
void fprintp(FILE *out, long value);
int sprintl(char *output, long num);
void sprintlf(char *output, char *fmt, ...);

// Print time
void fprintt(FILE *out, long seconds);

// Scan a punctuated long
void sscanl(char *input, long *value);

// Microseconds
float d_since(struct timeval *since);

void *d_new();
void d_add(void *arg, float value);
float d_avg(void *arg);
void d_reset(void *arg);
void d_end(void *arg);

#endif
