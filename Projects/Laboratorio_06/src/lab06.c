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

const osMutexAttr_t led_mutex_attr = {
    "LedMutex",                             // human readable mutex name
    osMutexRecursive | osMutexPrioInherit,  // attr_bits
    NULL,                                   // memory for control block
    0U                                      // size for control block
};

osMutexId_t led1_mutex;
osMutexId_t led2_mutex;
osMutexId_t led3_mutex;
osMutexId_t led4_mutex;

#define QUEUE_MESSAGE_COUNT 8
#define QUEUE_MESSAGE_SIZE sizeof(uint32_t)

void main_thread(void* arg);
void pwm_thread(void* arg);

void main(void) {
  LEDInit(LED1 | LED2 | LED3 | LED4);
  ButtonInit(USW1 | USW2);
  ButtonIntEnable(USW1 | USW2);

  osKernelInitialize();

  led1_mutex = osMutexNew(&led_mutex_attr);
  led2_mutex = osMutexNew(&led_mutex_attr);
  led3_mutex = osMutexNew(&led_mutex_attr);
  led4_mutex = osMutexNew(&led_mutex_attr);

  led1_queue = osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);
  led2_queue = osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);
  led3_queue = osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);
  led4_queue = osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);

  main_thread_id = osThreadNew(main_thread, NULL, NULL);

  if (osKernelGetState() == osKernelReady) osKernelStart();

  while (1)
    ;
}

void set_led_intensity(uint8_t led, uint32_t intensity) {
  if (led == LED1) {
    osMessageQueuePut(led1_queue, &intensity, 0, 0);
    osMessageQueuePut(led2_queue, (void*)0, 0, 0);
    osMessageQueuePut(led3_queue, (void*)0, 0, 0);
    osMessageQueuePut(led4_queue, (void*)0, 0, 0);
  } else if (led == LED2) {
    osMessageQueuePut(led1_queue, (void*)0, 0, 0);
    osMessageQueuePut(led2_queue, &intensity, 0, 0);
    osMessageQueuePut(led3_queue, (void*)0, 0, 0);
    osMessageQueuePut(led4_queue, (void*)0, 0, 0);
  } else if (led == LED2) {
    osMessageQueuePut(led1_queue, (void*)0, 0, 0);
    osMessageQueuePut(led2_queue, (void*)0, 0, 0);
    osMessageQueuePut(led3_queue, &intensity, 0, 0);
    osMessageQueuePut(led4_queue, (void*)0, 0, 0);
  } else if (led == LED2) {
    osMessageQueuePut(led1_queue, (void*)0, 0, 0);
    osMessageQueuePut(led2_queue, (void*)0, 0, 0);
    osMessageQueuePut(led3_queue, (void*)0, 0, 0);
    osMessageQueuePut(led4_queue, &intensity, 0, 0);
  }
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

  uint32_t intensity = 0;
  uint8_t led = LED1;

  while (1) {
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

    uint8_t is_usw1_pressed = ButtonRead(USW1) & USW1;
    uint8_t is_usw2_pressed = ButtonRead(USW2) & USW2;

    if (is_usw1_pressed) {
      if (led == LED1)
        led = LED2;
      else if (led == LED2)
        led = LED3;
      else if (led == LED3)
        led = LED4;
      else if (led == LED4)
        led = LED1;
      set_led_intensity(led, intensity);
    }
    if (is_usw2_pressed) {
      intensity = (intensity + 10) % 110;
      set_led_intensity(led, intensity);
    }
  }
}

void acquire_led_mutex(uint8_t led) {
  if (led == LED1) osMutexAcquire(led1_mutex, osWaitForever);
  if (led == LED2) osMutexAcquire(led2_mutex, osWaitForever);
  if (led == LED3) osMutexAcquire(led3_mutex, osWaitForever);
  if (led == LED4) osMutexAcquire(led4_mutex, osWaitForever);
}
void release_led_mutex(uint8_t led) {
  if (led == LED1) osMutexRelease(led1_mutex);
  if (led == LED2) osMutexRelease(led2_mutex);
  if (led == LED3) osMutexRelease(led3_mutex);
  if (led == LED4) osMutexRelease(led4_mutex);
}

void pwm_thread(void* arg_ptr) {
  uint32_t intensity = 0;

  pwm_thread_args_t* arg = (pwm_thread_args_t*)arg_ptr;

  uint8_t led = arg->led;
  osMessageQueueId_t queue_id = arg->queue_id;

  uint8_t is_led_on = 0;

  while (1) {
    uint32_t tick = osKernelGetTickCount();

    osMessageQueueGet(queue_id, &intensity, NULL, 0);

    uint32_t ticks_on = (PWM_PERIOD * intensity) / 100;
    uint32_t ticks_off = PWM_PERIOD - ticks_on;
    if (is_led_on && ticks_off > 0) {
      acquire_led_mutex(led);
      LEDOff(led);
      is_led_on = 0;
      release_led_mutex(led);
      osDelayUntil(tick + ticks_off);
    } else if (!is_led_on && ticks_on > 0) {
      acquire_led_mutex(led);
      LEDOn(led);
      is_led_on = 1;
      release_led_mutex(led);
      osDelayUntil(tick + ticks_on);
    }
  }
}

void GPIOJ_Handler(void) {
  ButtonIntClear(USW1 | USW2);
  osThreadFlagsSet(main_thread_id, 0x01);
}
