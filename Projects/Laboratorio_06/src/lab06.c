#include "cmsis_os2.h"        // CMSIS-RTOS
#include "driverbuttons.h"    // device drivers
#include "driverleds.h"       // device drivers
#include "system_tm4c1294.h"  // CMSIS-Core

osThreadId_t main_thread_id;

typedef struct {
  uint8_t led;
  osMessageQueueId_t queue_id;
} pwm_thread_args_t;

typedef struct {
  osThreadId_t thread_id;
  osMessageQueueId_t queue_id;
} pwm_controller_t;

pwm_controller_t pwm_controllers[4];

#define QUEUE_MESSAGE_COUNT 8
#define QUEUE_MESSAGE_SIZE 1

// void GPIOJ_Handler(void) { ButtonIntClear(USW1); }

void main_thread(void* arg);
void pwm_thread(void* arg);

void main(void) {
  LEDInit(LED1 | LED2 | LED3 | LED4);
  // ButtonInit(USW1);
  // ButtonIntEnable(USW1);

  osKernelInitialize();

  main_thread_id = osThreadNew(main_thread, NULL, NULL);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}

void create_pwm_thread(uint8_t index, uint8_t led) {
  osMessageQueueId_t queue_id =
      osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);

  pwm_thread_args_t args = {.led = led, .queue_id = queue_id};

  osThreadId_t thread_id = osThreadNew(pwm_thread, (void*)&args, NULL);

  pwm_controller_t controller = {
      .thread_id = thread_id,
      .queue_id = queue_id,
  };

  pwm_controllers[index] = controller;
}

void main_thread(void* arg) {
  uint8_t led_index = 0;

  osMessageQueueId_t queue =
      osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);

  pwm_thread_args_t args = {.led = LED1, .queue_id = queue};
  pwm_controller_t controller = {
      .thread_id = osThreadNew(pwm_thread, (void*)&args, NULL),
      .queue_id = queue,
  };

  create_pwm_thread(0, LED1);
  // create_pwm_thread(1, LED2);
  // create_pwm_thread(2, LED3);
  // create_pwm_thread(3, LED4);

  while (1) {
    osDelay(500);
  }
}

void pwm_thread(void* arg_ptr) {
  uint8_t intensity = 0;
  pwm_thread_args_t* arg = (pwm_thread_args_t*)arg_ptr;

  uint8_t led = arg->led;
  osMessageQueueId_t queue_id = arg->queue_id;

  while (1) {
    LEDOn(led);
    osThreadYield();
  }
}