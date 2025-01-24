// Virtual timer implementation

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nrf.h"

#include "virtual_timer.h"
#include "virtual_timer_linked_list.h"

void update_cc0_register() {
  __disable_irq();
  node_t* first_node = list_get_first();
  if (first_node != NULL) {
    NRF_TIMER4->CC[0] = first_node->timer_value;
  } else {
    NRF_TIMER4->CC[0] = 0xFFFFFFFF;
  }
  __enable_irq();
}

// This is the interrupt handler that fires on a compare event
void TIMER4_IRQHandler(void) {
  // This should always be the first line of the interrupt handler!
  // It clears the event so that it doesn't happen again
  NRF_TIMER4->EVENTS_COMPARE[0] = 0;

  // You will place your interrupt handler code here
  printf("Interrupt fired!\n");

  node_t *node = list_get_first();
  while (node != NULL && node->timer_value < read_timer()) {
    list_remove_first();
    node->callback();
    if (node->repeated) {
      uint32_t expiration_time = node->timer_value + node->interval;
      node->timer_value = expiration_time;
      list_insert_sorted(node);
      update_cc0_register();
    } else {
      free(node);
    }

    node = list_get_first();
  };
}

// Read the current value of the timer counter
uint32_t read_timer(void) {
  NRF_TIMER4->TASKS_CAPTURE[1] = 1;
  uint32_t value = NRF_TIMER4->CC[1];

  // Should return the value of the internal counter for TIMER4
  return value;
}

// Initialize the timers
void virtual_timer_init(void) {
  // Place your timer initialization code here
  NRF_TIMER4->BITMODE = 3;
  NRF_TIMER4->PRESCALER = 4;
  NRF_TIMER4->TASKS_CLEAR = 1;
  NRF_TIMER4->TASKS_START = 1;

  NRF_TIMER4->INTENSET = 1 << 16; // Enable interrupt for COMPARE[0]
  NVIC_EnableIRQ(TIMER4_IRQn);
}

// This is a private helper function called from multiple public functions with different arguments.
// Starts a timer. This function is called for both one-shot and repeated timers
static uint32_t timer_start(uint32_t microseconds, virtual_timer_callback_t cb, bool repeated) {
  uint32_t expiration_time = read_timer() + microseconds;

  node_t *new_node = malloc(sizeof(node_t));
  new_node->timer_value = expiration_time;
  new_node->callback = cb;
  new_node->repeated = repeated;
  if (repeated) {
    new_node->interval = microseconds;
  }
  __disable_irq();
  list_insert_sorted(new_node);
  __enable_irq();
  update_cc0_register();

  // Return a unique timer ID. (hint: What is guaranteed unique about the timer you have created?)
  return (uint32_t)new_node;
}

// You do not need to modify this function
// Instead, implement timer_start
uint32_t virtual_timer_start(uint32_t microseconds, virtual_timer_callback_t cb) {
  return timer_start(microseconds, cb, false);
}

// You do not need to modify this function
// Instead, implement timer_start
uint32_t virtual_timer_start_repeated(uint32_t microseconds, virtual_timer_callback_t cb) {
  return timer_start(microseconds, cb, true);
}

// Remove a timer by ID.
// Make sure you don't cause linked list consistency issues!
// Do not forget to free removed timers.
void virtual_timer_cancel(uint32_t timer_id) {
  __disable_irq();
  node_t *node = (node_t *)timer_id;
  list_remove(node);
  __enable_irq();
  free(node);

  update_cc0_register();
}

