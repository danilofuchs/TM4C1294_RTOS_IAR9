#include "cmsis_os2.h"        // CMSIS-RTOS
#include "driverbuttons.h"    // device drivers
#include "driverleds.h"       // device drivers
#include "system_tm4c1294.h"  // CMSIS-Core

#define BUFFER_SIZE 8
#define DONT_WAIT 0

// Circular buffer to hold generated items
typedef struct {
  uint8_t items[BUFFER_SIZE];
  uint8_t index;
} circular_buffer_t;

void circularBufferAddItem(circular_buffer_t* buffer, uint8_t item) {
  buffer->items[buffer->index] = item;
  buffer->index = (buffer->index + 1) % BUFFER_SIZE;
}
uint8_t circularBufferGetItem(circular_buffer_t* buffer) {
  return buffer->items[buffer->index];
}

circular_buffer_t buffer;

// Semaphores to synchronize the ISR with the consumer thread
osSemaphoreId_t empty_slots_id, full_slots_id;

osThreadId_t consumer_id;

uint8_t count = 0;
void GPIOJ_Handler(void) {
  ButtonIntClear(USW1);
  count++;
  osSemaphoreAcquire(empty_slots_id, DONT_WAIT);
  circularBufferAddItem(&buffer, count);
  osSemaphoreRelease(full_slots_id);
}

void consumer(void* arg) {
  while (1) {
    osSemaphoreAcquire(full_slots_id, osWaitForever);
    uint8_t item = circularBufferGetItem(&buffer);
    osSemaphoreRelease(empty_slots_id);

    LEDWrite(LED4 | LED3 | LED2 | LED1, item);

    osDelay(500);
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
