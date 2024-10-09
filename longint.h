#ifndef _LONGINT_H_
#define _LONGINT_H_

// Print longs
void fprintl(FILE *out, long num);
void fprintlf(FILE *out, char *fmt, ...);
void fprintpf(FILE *out, char *fmt, ...);
int sprintl(char *output, long num);
void sprintlf(char *output, char *fmt, ...);

// Print time
void fprintt(FILE *out, time_t seconds);

// Scan a punctuated long
void sscanl(char *input, long *value);

#endif
