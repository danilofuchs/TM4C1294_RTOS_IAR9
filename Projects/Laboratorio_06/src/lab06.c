#include "cmsis_os2.h"        // CMSIS-RTOS
#include "driverbuttons.h"    // device drivers
#include "driverleds.h"       // device drivers
#include "system_tm4c1294.h"  // CMSIS-Core

#define BUFFER_SIZE 8
#define MAX_COUNT 16
#define DONT_WAIT 0

// Circular buffer to hold generated items
uint8_t buffer[BUFFER_SIZE];

// Semaphores to synchronize the ISR with the consumer thread
osSemaphoreId_t empty_slots_id, full_slots_id;

osThreadId_t consumer_id;

uint8_t count, index = 0;
void GPIOJ_Handler(void) {
  ButtonIntClear(USW1);
  count = (count + 1) % MAX_COUNT;
  osSemaphoreAcquire(empty_slots_id, DONT_WAIT);
  buffer[index] = count;
  osSemaphoreRelease(full_slots_id);
  index = (index + 1) % BUFFER_SIZE;
}

void consumer(void* arg) {
  uint8_t index = 0;
  while (1) {
    osSemaphoreAcquire(full_slots_id, osWaitForever);
    uint8_t item = buffer[index];
    osSemaphoreRelease(empty_slots_id);

    LEDWrite(LED4 | LED3 | LED2 | LED1, item);

    index = (index + 1) % BUFFER_SIZE;

    osDelay(500);
  }
}

void main(void) {
  LEDInit(LED1 | LED2 | LED3 | LED4);
  ButtonInit(USW1);
  ButtonIntEnable(USW1);

  osKernelInitialize();

  consumer_id = osThreadNew(consumer, NULL, NULL);

  // starts with BUFFER_SIZE empty slots
  empty_slots_id = osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL);
  // starts with 0 full slots
  full_slots_id = osSemaphoreNew(BUFFER_SIZE, 0, NULL);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}
