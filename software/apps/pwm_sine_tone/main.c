// PWM Sine App
//
// Create a sine wave and use PWM to play it at diferent frequencies

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf.h"
#include "nrf_delay.h"
#include "nrfx_pwm.h"

#include "microbit_v2.h"

// PWM configuration
static const nrfx_pwm_t PWM_INST = NRFX_PWM_INSTANCE(0);

// Holds a pre-computed sine wave
#define SINE_BUFFER_SIZE 500
uint16_t sine_buffer[SINE_BUFFER_SIZE] = { 0 };

// Sample data configurations
// Note: this is a 32 kB buffer (about 25% of RAM)
#define SAMPLING_FREQUENCY 16000 // 16 kHz sampling rate
#define BUFFER_SIZE 16000 // one second worth of data
uint16_t samples[BUFFER_SIZE] = { 0 }; // stores PWM duty cycle values

/*** Initialization & handling code ***/

static void gpio_init(void) {
  // Initialize pins
  // Microphone pin MUST be high drive
  nrf_gpio_pin_dir_set(LED_MIC, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_cfg(LED_MIC, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT,
    NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0H1, NRF_GPIO_PIN_NOSENSE);

  // Enable microphone
  nrf_gpio_pin_set(LED_MIC);
}

static void pwm_init(void) {

  // Initialize the PWM
  // SPEAKER_OUT is the output pin, mark the others as NRFX_PWM_PIN_NOT_USED
  // Set the clock to 16 MHz
  // Set a countertop value based on sampling frequency and repetitions

  nrfx_pwm_config_t config;
  config.output_pins[0] = SPEAKER_OUT;
  config.output_pins[1] = NRFX_PWM_PIN_NOT_USED;
  config.output_pins[2] = NRFX_PWM_PIN_NOT_USED;
  config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;
  config.irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY;
  config.base_clock = NRF_PWM_CLK_16MHz;
  config.count_mode = NRF_PWM_MODE_UP;
  config.top_value = 16000000 / SAMPLING_FREQUENCY / 2;
  config.load_mode = NRF_PWM_LOAD_COMMON;
  config.step_mode = NRF_PWM_STEP_AUTO;

  nrfx_pwm_init(&PWM_INST, &config, NULL);
}

static void compute_sine_wave(uint16_t max_value) {
  for (int i = 0; i < SINE_BUFFER_SIZE; i++) {
    // what percent into the sine wave are we?
    float percent = (float)i / (float)SINE_BUFFER_SIZE;

    // get sine value
    float twopi = 2.0 * 3.14159;
    float radians = twopi * percent;
    float sine_value = sin(radians);

    // translate from "-1 to 1" into "0 to 2"
    float abs_sine = sine_value + 1.0;

    // scale from "0 to 2" into "0 to 1"
    float mag_1_sine = abs_sine / 2.0;

    // scale to the max_value desired ("0 to max_value")
    // and truncate to an integer value
    uint16_t value = (uint16_t)(max_value * mag_1_sine);

    // save value in buffer
    sine_buffer[i] = value;
  }
}

static void play_note(uint16_t frequency) {

  // determine number of sine values to "step" per played sample
  // units are (sine-values/cycle) * (cycles/second) / (samples/second) = (sine-values/sample)
  float step_size = (float)SINE_BUFFER_SIZE * (float)frequency / (float)SAMPLING_FREQUENCY;

  // Fill sample buffer based on frequency
  // Each element in the sample buffer will be an element from the sine_buffer
  // The first should be sine_buffer[0],
  //    then each successive sample will be "step_size" further along the sine wave
  // Be sure to convert the cumulative step_size into an integer to access an item in sine_buffer
  // If the cumulative steps are greater than SINE_BUFFER_SIZE, wrap back around zero
  //    but don't just set it to zero, as that would be discontinuous
  float current_step = 0.0;
  for (int i = 0; i < BUFFER_SIZE; i += 1) {
    samples[i] = sine_buffer[(int)current_step % SINE_BUFFER_SIZE];
    current_step += step_size;
  }

  // Create the pwm sequence (nrf_pwm_sequence_t) using the samples
  // Do not make another buffer for this. You can reuse the sample buffer
  // You should set a non-zero repeat value (this is how many times each _sample_ repeats)
  nrf_pwm_sequence_t pwm_sequence = {
    .values.p_common = samples,
    .length = BUFFER_SIZE,
    .repeats = 1,
    .end_delay = 0,
  };

  // Start playback of the samples
  // You will need to pass in a flag to loop the sound
  // The playback count here is the number of times the entire buffer will repeat
  //    (which doesn't matter if you set the loop flag)
  nrfx_pwm_simple_playback(&PWM_INST, &pwm_sequence, 1, NRFX_PWM_FLAG_LOOP);
}

int main(void) {
  printf("Board started!\n");

  // initialize GPIO
  gpio_init();

  // initialize the PWM
  pwm_init();

  // compute the sine wave values
  // You should pass in COUNTERTOP-1 here as the maximum value
  compute_sine_wave(16000000 / SAMPLING_FREQUENCY - 1);

  // Play the A4 tone for one second
  printf("A4\n");
  play_note(440);
  nrf_delay_us(1000000);

  // Play the C#5 tone for one second
  printf("C#5\n");
  play_note(554);
  nrf_delay_us(1000000);

  // Play the E5 tone for one second
  printf("E5\n");
  play_note(659);
  nrf_delay_us(1000000);

  // Play the A5 tone for one second
  printf("A5\n");
  play_note(880);
  nrf_delay_us(1000000);

  // Stop all noises
  nrfx_pwm_stop(&PWM_INST, true);
}

