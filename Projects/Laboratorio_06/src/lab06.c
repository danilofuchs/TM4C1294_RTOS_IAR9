#include "cmsis_os2.h"        // CMSIS-RTOS
#include "driverbuttons.h"    // device drivers
#include "driverleds.h"       // device drivers
#include "system_tm4c1294.h"  // CMSIS-Core

osThreadId_t main_thread_id;

#define LED_COUNT 4
uint8_t leds[LED_COUNT] = {LED1, LED2, LED3, LED4};

#define PWM_PERIOD 10

typedef struct {
  uint8_t led;
  osMessageQueueId_t queue_id;
} pwm_thread_args_t;

osThreadId_t led1_thread;
osThreadId_t led2_thread;
osThreadId_t led3_thread;
osThreadId_t led4_thread;

osMessageQueueId_t led1_queue;
osMessageQueueId_t led2_queue;
osMessageQueueId_t led3_queue;
osMessageQueueId_t led4_queue;

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

  led1_queue = osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);
  led2_queue = osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);
  led3_queue = osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);
  led4_queue = osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);

  main_thread_id = osThreadNew(main_thread, NULL, NULL);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}

void main_thread(void* arg) {
  pwm_thread_args_t led1_args = {.led = LED1, .queue_id = led1_queue};
  led1_thread = osThreadNew(pwm_thread, (void*)&led1_args, NULL);

  pwm_thread_args_t led2_args = {.led = LED2, .queue_id = led2_queue};
  led2_thread = osThreadNew(pwm_thread, (void*)&led2_args, NULL);

  pwm_thread_args_t led3_args = {.led = LED3, .queue_id = led3_queue};
  led3_thread = osThreadNew(pwm_thread, (void*)&led3_args, NULL);

  pwm_thread_args_t led4_args = {.led = LED4, .queue_id = led4_queue};
  led4_thread = osThreadNew(pwm_thread, (void*)&led4_args, NULL);

  // while (1) {
  // osMessageQueuePut(pwm_controllers[0].queue_id, (void*)1, 0, osWaitForever);
  // osMessageQueuePut(pwm_controllers[1].queue_id, (void*)1, 0, osWaitForever);
  // osMessageQueuePut(pwm_controllers[2].queue_id, (void*)1, 0, osWaitForever);
  // osMessageQueuePut(pwm_controllers[3].queue_id, (void*)1, 0, osWaitForever);
  // }

  osDelay(osWaitForever);
}

void toggleLed(void* arg) {
  uint8_t led = (uint8_t)arg;
  LEDToggle(led);
}

void pwm_thread(void* arg_ptr) {
  uint8_t intensity = 0;

  pwm_thread_args_t* arg = (pwm_thread_args_t*)arg_ptr;

  uint8_t led = arg->led;
  // osMessageQueueId_t queue_id = arg->queue_id;

  uint8_t is_led_on = 0;

  osTimerId_t timer_id = osTimerNew(toggleLed, osTimerOnce, (void*)led, NULL);

  while (1) {
    if (osTimerIsRunning(timer_id)) {
      continue;
    }
    uint32_t ticks_on = (PWM_PERIOD * intensity) / 100;
    uint32_t ticks_off = PWM_PERIOD - ticks_on;
    if (is_led_on && ticks_off > 0) {
      osTimerStart(timer_id, ticks_off);
      is_led_on = 0;
    } else if (!is_led_on && ticks_on > 0) {
      osTimerStart(timer_id, ticks_on);
      is_led_on = 1;
    }
  }
}