#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h"      // device drivers
#include "cmsis_os2.h"       // CMSIS-RTOS

osThreadId_t thread1_id, thread2_id;

void thread(void *arg)
{
  uint8_t led = (uint32_t)arg;
  uint8_t state = 0;
  uint32_t tick;

  while (1)
  {
    tick = osKernelGetTickCount();

    state ^= led;
    LEDWrite(led, state);

    osDelayUntil(tick + 100);
  }
}

void main(void)
{
  LEDInit(LED2 | LED1);

  osKernelInitialize();

  thread1_id = osThreadNew(thread, (void *)LED1, NULL);
  thread2_id = osThreadNew(thread, (void *)LED2, NULL);

  if (osKernelGetState() == osKernelReady)
    osKernelStart();

  while (1)
    ;
} // main
