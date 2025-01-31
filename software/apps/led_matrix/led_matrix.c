// LED Matrix Driver
// Displays characters on the LED matrix

#include <stdbool.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "app_timer.h"

#include "led_matrix.h"
#include "font.h"
#include "microbit_v2.h"

bool led_states[5][5] = {
  {0, 1, 1, 1, 0},
  {1, 1, 0, 1, 1},
  {1, 0, 0, 0, 1},
  {1, 1, 0, 1, 1},
  {0, 1, 1, 1, 0},
};

uint32_t row_pins[5] = { LED_ROW1, LED_ROW2, LED_ROW3, LED_ROW4, LED_ROW5 };
uint32_t col_pins[5] = { LED_COL1, LED_COL2, LED_COL3, LED_COL4, LED_COL5 };

APP_TIMER_DEF(flicker_timer);
APP_TIMER_DEF(switchchar_timer);

static volatile int current_row = 0;
void flicker(void* _) {
  // change active row
  nrf_gpio_pin_write(row_pins[current_row], 0);
  current_row = (current_row + 1) % 5;
  nrf_gpio_pin_write(row_pins[current_row], 1);

  // change colums to match
  for (int i = 0; i < 5; ++i) {
    nrf_gpio_pin_write(col_pins[i], !led_states[current_row][i]);
  }
}

static volatile struct {
  const char* string;
  int length;
  int current_char;
} display_string;

void set_string(const char* string, int length) {
  printf("Setting string: %s\n", string);
  display_string.string = string;
  display_string.length = length;
  display_string.current_char = 0;
}

void set_char(char c) {
  printf("Setting char: %c\n", c);

  uint8_t* font_states = font[(int)c]; // array of 5 bytes

  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      led_states[i][j] = (font_states[i] >> j) & 1;
    }
  }
}

void switch_char(void* _) {
  set_char(display_string.string[display_string.current_char]);
  display_string.current_char = (display_string.current_char + 1) % display_string.length;
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
  app_timer_create(&flicker_timer, APP_TIMER_MODE_REPEATED, flicker);
  app_timer_start(flicker_timer, 32768 / 500, NULL);
  app_timer_create(&switchchar_timer, APP_TIMER_MODE_REPEATED, switch_char);
  app_timer_start(switchchar_timer, 32768 / 2, NULL);

  // set default state for the LED display (Step 3 and onwards)
}
