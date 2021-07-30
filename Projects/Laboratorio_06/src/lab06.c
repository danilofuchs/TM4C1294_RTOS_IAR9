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

void main_thread(void* arg) {
  for (int i = 0; i < LED_COUNT; i++) {
    uint8_t led = leds[i];

    osMessageQueueId_t queue =
        osMessageQueueNew(QUEUE_MESSAGE_COUNT, QUEUE_MESSAGE_SIZE, NULL);

    pwm_thread_args_t args = {.led = led, .queue_id = queue};
    pwm_controller_t controller = {
        .thread_id = osThreadNew(pwm_thread, (void*)&args, NULL),
        .queue_id = queue,
    };
  }

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