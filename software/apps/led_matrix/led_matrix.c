// LED Matrix Driver
// Displays characters on the LED matrix

#include <stdbool.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "app_timer.h"

#include "led_matrix.h"
#include "font.h"
#include "microbit_v2.h"

APP_TIMER_DEF(timer);

static volatile int led = 0;
void next_led(void* _) {
  int current_led;
  int next_led;
  switch (led) {
    case 0:
      current_led = LED_COL1;
      next_led = LED_COL2;
      break;
    case 1:
      current_led = LED_COL2;
      next_led = LED_COL3;
      break;
    case 2:
      current_led = LED_COL3;
      next_led = LED_COL4;
      break;
    case 3:
      current_led = LED_COL4;
      next_led = LED_COL5;
      break;
    case 4:
      current_led = LED_COL5;
      next_led = LED_COL1;
      break;
  }
  led = (led + 1) % 5;

  nrf_gpio_pin_write(current_led, 1);
  nrf_gpio_pin_write(next_led, 0);
}

void led_matrix_init(void) {
  // initialize row pins
  nrf_gpio_pin_dir_set(LED_ROW1, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW2, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW3, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW4, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW5, NRF_GPIO_PIN_DIR_OUTPUT);

  // initialize col pins
  nrf_gpio_pin_dir_set(LED_COL1, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL2, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL3, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL4, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL5, NRF_GPIO_PIN_DIR_OUTPUT);

  // set default values for pins
  nrf_gpio_pin_write(LED_ROW1, 1);
  nrf_gpio_pin_write(LED_ROW2, 0);
  nrf_gpio_pin_write(LED_ROW3, 0);
  nrf_gpio_pin_write(LED_ROW4, 0);
  nrf_gpio_pin_write(LED_ROW5, 0);
  nrf_gpio_pin_write(LED_COL1, 0);
  nrf_gpio_pin_write(LED_COL2, 1);
  nrf_gpio_pin_write(LED_COL3, 1);
  nrf_gpio_pin_write(LED_COL4, 1);
  nrf_gpio_pin_write(LED_COL5, 1);

  // initialize timer(s) (Step 2 and onwards)
  app_timer_init();
  app_timer_create(&timer, APP_TIMER_MODE_REPEATED, next_led);
  app_timer_start(timer, 32768, NULL);


  // set default state for the LED display (Step 3 and onwards)
}
