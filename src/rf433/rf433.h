// rf433.h — 433MHz wireless receiver signal detection
// ~10ms scan window, 50µs interval (~200 samples)
// Usage: rf433_init() once, then rf433_scan707() / rf433_scan705() per frame
#ifndef RF433_H
#define RF433_H

#include <stdint.h>

// Initialize RF433 module (call after IOPORT open)
void rf433_init(void);

// Scan P707 for RF carrier pulse (~10ms), returns 1=signal, 0=none
int rf433_scan707(void);

// Scan P705 for RF carrier pulse (~10ms), returns 1=signal, 0=none
int rf433_scan705(void);

#endif /* RF433_H */
