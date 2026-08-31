#ifndef PIT_H
#define PIT_H

#include <stdint.h>

void pit_init(uint32_t freq_hz);

uint64_t pit_get_ticks(void);

uint64_t pit_get_millis(void);

void pit_sleep_ms(uint32_t ms);

void pit_handler(void);

#endif