#ifndef CLOCK_H
#define CLOCK_H

#define PIT_CHAN0_REG 0X40
#define PIT_CHAN2_REG 0X42
#define PIT_CTRL_REG 0X43

#define HZ 100
#define OSCILLATOR 1193182*4
#define CLOCK_COUNTER (OSCILLATOR / HZ)
#define JIFFY (1000 / HZ)

#define SPEAKER_REG 0X61
#define BEEP_HZ 440
#define BEEP_COUNT (OSCILLATOR/BEEP_HZ)


void clock_handler(int vector);
void pit_init();
void clock_init();

void beep_start();
void beep_stop();

#endif // CLOCK_H

