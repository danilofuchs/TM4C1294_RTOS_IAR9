#include "cmsis_os2.h"        // CMSIS-RTOS
#include "driverbuttons.h"    // device drivers
#include "driverleds.h"       // device drivers
#include "system_tm4c1294.h"  // CMSIS-Core

#define BUFFER_SIZE 8
#define BUFFER_INDEX count % BUFFER_SIZE

osSemaphoreId_t empty_slots_id, full_slots_id;
osThreadId_t consumer_id;

uint8_t buffer[BUFFER_SIZE];
uint8_t count = 0;

void GPIOJ_Handler(void) {
  ButtonIntClear(USW1);
  count++;
  osSemaphoreAcquire(empty_slots_id, osWaitForever);
  buffer[BUFFER_INDEX] = count;
  osSemaphoreRelease(full_slots_id);
}

osThreadId_t thread1_id, thread2_id, thread3_id, thread4_id;

void consumer(void *arg) {
  while (1) {
    osSemaphoreAcquire(full_slots_id, osWaitForever);
    uint8_t state = buffer[BUFFER_INDEX];
    osSemaphoreRelease(empty_slots_id);

    LEDWrite(LED4 | LED3 | LED2 | LED1, state);

    osDelay(50);
  }
}

void main(void) {
  LEDInit(LED1 | LED2 | LED3 | LED4);
  ButtonInit(USW1);
  ButtonIntEnable(USW1);

  osKernelInitialize();

  consumer_id = osThreadNew(consumer, NULL, NULL);

  // BUFFER_SIZE empty slots
  empty_slots_id = osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL);
  // 0 full slots
  full_slots_id = osSemaphoreNew(BUFFER_SIZE, 0, NULL);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}
