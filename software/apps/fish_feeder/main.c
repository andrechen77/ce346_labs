#include "microbit_v2.h"
#include "ultrasonic.h"
#include "timer.h"
#include "nrf_delay.h"

void my_print() {
  printf("HELLO WORLD\n");
}

int main() {
  printf("Hello, world!\n");

  timer_init();
  ultrasonic_init();

  while (true) {
    float distance = sample_sensors_distance_cm();
    printf("Distance: %f cm\n", distance);

    execute_event(5000, my_print);
    nrf_delay_ms(1000);
  }

  return 0;
}
