#include "cmsis_os2.h"        // CMSIS-RTOS
#include "driverleds.h"       // device drivers
#include "system_tm4c1294.h"  // CMSIS-Core

osThreadId_t thread1_id, thread2_id;

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
  LEDInit(LED2 | LED1);

  osKernelInitialize();

  thread_arg_t thread1_arg = {.led = LED1, .period = 100};
  thread_arg_t thread2_arg = {.led = LED2, .period = 200};

  thread1_id = osThreadNew(blink_thread, (void *)&thread1_arg, NULL);
  thread2_id = osThreadNew(blink_thread, (void *)&thread2_arg, NULL);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}  // main
