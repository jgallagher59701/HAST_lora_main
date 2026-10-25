
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

// Using F() in this macro reduced RAM use from 67% to 50% in ~1200 LOC
#if DEBUG
#define DPRINTV(fmt, ...) print(F(fmt), __VA_ARGS__)
#define DPRINT(fmt) print(F(fmt))
#define DPRINTF(fmt, f) Serial.print(fmt); Serial.println(f)
#else
#define DPRINTV(fmt, ...)
#define DPRINT(fmt)

#define DPRINTF(fmt, f)
#endif

#endif