#include "cmsis_os2.h"        // CMSIS-RTOS
#include "driverbuttons.h"    // device drivers
#include "driverleds.h"       // device drivers
#include "system_tm4c1294.h"  // CMSIS-Core

osThreadId_t controller_thread_id;
osThreadId_t pwm_thread_ids[4];

// void GPIOJ_Handler(void) { ButtonIntClear(USW1); }

void controller_thread(void* arg);
void pwm_thread(void* arg);

void main(void) {
  LEDInit(LED1 | LED2 | LED3 | LED4);
  // ButtonInit(USW1);
  // ButtonIntEnable(USW1);

  osKernelInitialize();

  controller_thread_id = osThreadNew(controller_thread, NULL, NULL);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}

void controller_thread(void* arg) {
  pwm_thread_ids[0] = osThreadNew(pwm_thread, (void*)LED1, NULL);
  pwm_thread_ids[1] = osThreadNew(pwm_thread, (void*)LED2, NULL);
  pwm_thread_ids[2] = osThreadNew(pwm_thread, (void*)LED3, NULL);
  pwm_thread_ids[3] = osThreadNew(pwm_thread, (void*)LED4, NULL);
}

void pwm_thread(void* arg) {
  uint8_t intensity = 0;
  uint8_t led = (uint8_t)arg;
  while (1) {
    LEDOn(led);
    osThreadYield();
  }
}