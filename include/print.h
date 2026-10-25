
/**
 * A version of printf for the Arduino.
 * 11/20/22
 * James Gallagher <jhrg@mac.com>
 */

#ifndef print_h_
#define print_h_

void print(const char *fmt, ...);
void print(const __FlashStringHelper *fmt, ...);

void flush();

#endif