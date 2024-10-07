#ifndef _LONGINT_H_
#define _LONGINT_H_

// Print longs
void printl(long num);
void fprintl(FILE *out, long num);
void printlf(char *fmt, ...);
void fprintlf(FILE *out, char *fmt, ...);
void printpf(char *fmr, ...);
void fprintpf(FILE *out, char *fmr, ...);
int sprintl(char *output, long num);
void sprintlf(char *output, char *fmt, ...);
void printtf(char *fmt, ...);

// Scan a punctuated long
void sscanl(char *input, long *value);

// Show a spinner
void fspin(FILE *out, long number);

#endif
