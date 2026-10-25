
/**
 * A version of printf for the Arduino.
 * 11/20/22
 * James Gallagher <jhrg@mac.com>
 */

#ifndef print_h_
#define print_h_

void print(const char *fmt, ...);
void print(const __FlashStringHelper *fmt, ...);

// Print using %02d
// TODO needs work
//#define frac(f) (int)((f > 0.0) ? (int(f*100)%100) : round((f+ceil(f))*100.0))
#define frac(f) (int(f * 100) % 100)

void flush();

#endif