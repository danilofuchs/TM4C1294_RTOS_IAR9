#include "cmsis_os2.h"        // CMSIS-RTOS
#include "driverleds.h"       // device drivers
#include "system_tm4c1294.h"  // CMSIS-Core

osThreadId_t thread1_id, thread2_id, thread3_id, thread4_id;

typedef struct {
  uint8_t led;
  uint32_t period;
} thread_arg_t;

void blink_thread(void *arg_ptr) {
  thread_arg_t *arg = (thread_arg_t *)arg_ptr;
  uint8_t state = 0;
  uint32_t tick;

  while (1) {
    tick = osKernelGetTickCount();

    state ^= arg->led;
    LEDWrite(arg->led, state);

    osDelayUntil(tick + arg->period);
  }
}

void main(void) {
  LEDInit(LED1 | LED2 | LED3 | LED4);

  osKernelInitialize();

  thread_arg_t thread1_arg = {.led = LED1, .period = 200};
  thread_arg_t thread2_arg = {.led = LED2, .period = 300};
  thread_arg_t thread3_arg = {.led = LED3, .period = 500};
  thread_arg_t thread4_arg = {.led = LED4, .period = 700};

  thread1_id = osThreadNew(blink_thread, (void *)&thread1_arg, NULL);
  thread2_id = osThreadNew(blink_thread, (void *)&thread2_arg, NULL);
  thread3_id = osThreadNew(blink_thread, (void *)&thread3_arg, NULL);
  thread4_id = osThreadNew(blink_thread, (void *)&thread4_arg, NULL);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}
