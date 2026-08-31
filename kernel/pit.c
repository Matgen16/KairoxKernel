#include "pit.h"
#include "io.h"
#include "pic.h"

#define PIT_BASE_FREQUENCY 1193182u

#define PIT_CHANNEL0_DATA 0x40
#define PIT_COMMAND       0x43

#define PIT_CMD_CH0_MODE3 0x36

#define IRQ_PIT 0

static volatile uint64_t ticks = 0;
static uint32_t configured_freq_hz = 100;

void pit_init(uint32_t freq_hz)
{
    if (freq_hz == 0) {
        freq_hz = 100;
    }

    uint32_t divisor = PIT_BASE_FREQUENCY / freq_hz;
    if (divisor == 0) {
        divisor = 1;
    }
    if (divisor > 0xFFFF) {
        divisor = 0xFFFF;
    }

    configured_freq_hz = freq_hz;
    ticks = 0;

    outb(PIT_COMMAND, PIT_CMD_CH0_MODE3);
    outb(PIT_CHANNEL0_DATA, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_DATA, (uint8_t)((divisor >> 8) & 0xFF));

    pic_unmask_irq(IRQ_PIT);
}

uint64_t pit_get_ticks(void)
{
    return ticks;
}

uint64_t pit_get_millis(void)
{
    return (ticks * 1000ULL) / configured_freq_hz;
}

void pit_sleep_ms(uint32_t ms)
{
    uint64_t target = pit_get_millis() + ms;
    while (pit_get_millis() < target) {
        __asm__ volatile ("sti; hlt");
    }
}

void pit_handler(void)
{
    ticks++;
    pic_send_eoi(IRQ_PIT);
}