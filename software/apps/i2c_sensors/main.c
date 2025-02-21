// I2C sensors app
//
// Read from I2C accelerometer/magnetometer on the Microbit

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_twi_mngr.h"

#include "microbit_v2.h"
#include "lsm303agr.h"

// Global variables
NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);
APP_TIMER_DEF(timer);


static void timer_callback(void* _unused) {
  // Do things periodically here
  float temp = lsm303agr_read_temperature();
  printf("temp is %f c\n", temp);

  lsm303agr_measurement_t acc = lsm303agr_read_accelerometer();
  printf("acc x: %f y: %f z: %f\n", acc.x_axis, acc.y_axis, acc.z_axis);

  lsm303agr_measurement_t mag = lsm303agr_read_magnetometer();
  printf("mag x: %f y: %f z: %f\n", mag.x_axis, mag.y_axis, mag.z_axis);
}

int main(void) {
  printf("Board started!\n");

  // initialize app timers
  app_timer_init();
  app_timer_create(&timer, APP_TIMER_MODE_REPEATED, timer_callback);
  app_timer_start(timer, 32768, NULL);

  // Initialize I2C peripheral and driver
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  // WARNING!!
  // These are NOT the correct pins for external I2C communication.
  // If you are using QWIIC or other external I2C devices, the are
  // connected to EDGE_P19 (a.k.a. I2C_QWIIC_SCL) and EDGE_P20 (a.k.a. I2C_QWIIC_SDA)
  i2c_config.scl = I2C_INTERNAL_SCL;
  i2c_config.sda = I2C_INTERNAL_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  i2c_config.interrupt_priority = 0;
  nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);

  // Initialize the LSM303AGR accelerometer/magnetometer sensor
  lsm303agr_init(&twi_mngr_instance);

  //TODO: implement me!

  // Loop forever
  while (1) {
    // Don't put any code in here. Instead put periodic code in a callback using a timer.
    nrf_delay_ms(1000);
  }
}

