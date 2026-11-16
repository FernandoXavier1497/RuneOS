#ifndef RUNEOS_CLOCK_H
#define RUNEOS_CLOCK_H
#include "types.h"

#define CLOCK_HZ 100

void     clock_init(void);
uint32_t clock_ticks(void);
uint32_t clock_uptime_seconds(void);
void     clock_sleep_ms(uint32_t ms);

#endif
