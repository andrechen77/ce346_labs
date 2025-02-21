// LSM303AGR driver for Microbit_v2
//
// Initializes sensor and communicates over I2C
// Capable of reading temperature, acceleration, and magnetic field strength

#include <stdbool.h>
#include <stdint.h>

#include "lsm303agr.h"
#include "nrf_delay.h"
#include <math.h>

// Pointer to an initialized I2C instance to use for transactions
static const nrf_twi_mngr_t* i2c_manager = NULL;

// Helper function to perform a 1-byte I2C read of a given register
//
// i2c_addr - address of the device to read from
// reg_addr - address of the register within the device to read
//
// returns 8-bit read value
static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr) {
  uint8_t rx_buf = 0;
  nrf_twi_mngr_transfer_t const read_transfer[] = {
    //TODO: implement me
    NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0)
  };
  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);
  if (result != NRF_SUCCESS) {
    // Likely error codes:
    //  NRF_ERROR_INTERNAL            (0x0003) - something is wrong with the driver itself
    //  NRF_ERROR_INVALID_ADDR        (0x0010) - buffer passed was in Flash instead of RAM
    //  NRF_ERROR_BUSY                (0x0011) - driver was busy with another transfer still
    //  NRF_ERROR_DRV_TWI_ERR_OVERRUN (0x8200) - data was overwritten during the transaction
    //  NRF_ERROR_DRV_TWI_ERR_ANACK   (0x8201) - i2c device did not acknowledge its address
    //  NRF_ERROR_DRV_TWI_ERR_DNACK   (0x8202) - i2c device did not acknowledge a data byte
    printf("I2C transaction failed! Error: %lX\n", result);
  }

  return rx_buf;
}

// Helper function to perform a 1-byte I2C write of a given register
//
// i2c_addr - address of the device to write to
// reg_addr - address of the register within the device to write
static void i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data) {
  //TODO: implement me
  //Note: there should only be a single two-byte transfer to be performed

  uint8_t msg[2] = { reg_addr, data };
  nrf_twi_mngr_transfer_t const write_transfer[] = {
    //TODO: implement me
    NRF_TWI_MNGR_WRITE(i2c_addr, msg, 2, 0),
  };

  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);
  if (result != NRF_SUCCESS) {
    // Likely error codes:
    //  NRF_ERROR_INTERNAL            (0x0003) - something is wrong with the driver itself
    //  NRF_ERROR_INVALID_ADDR        (0x0010) - buffer passed was in Flash instead of RAM
    //  NRF_ERROR_BUSY                (0x0011) - driver was busy with another transfer still
    //  NRF_ERROR_DRV_TWI_ERR_OVERRUN (0x8200) - data was overwritten during the transaction
    //  NRF_ERROR_DRV_TWI_ERR_ANACK   (0x8201) - i2c device did not acknowledge its address
    //  NRF_ERROR_DRV_TWI_ERR_DNACK   (0x8202) - i2c device did not acknowledge a data byte
    printf("I2C transaction failed! Error: %lX\n", result);
  }
}

// Initialize and configure the LSM303AGR accelerometer/magnetometer
//
// i2c - pointer to already initialized and enabled twim instance
void lsm303agr_init(const nrf_twi_mngr_t* i2c) {
  i2c_manager = i2c;

  // ---Initialize Accelerometer---

  // Reboot acclerometer
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG5_A, 0x80);
  nrf_delay_ms(100); // needs delay to wait for reboot

  // Enable Block Data Update
  // Only updates sensor data when both halves of the data has been read
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG4_A, 0x80);

  // Configure accelerometer at 100Hz, normal mode (10-bit)
  // Enable x, y and z axes
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG1_A, 0x57);

  // Read WHO AM I register
  // Always returns the same value if working
  int id = i2c_reg_read(LSM303AGR_ACC_ADDRESS, WHO_AM_I_A);
  printf("Accelerometer WHO AM I: %X\n", id);
  if (id != 0b00110011) {
    printf("Accelerometer WHO AM I failed!\n");
  } else {
    printf("Accelerometer WHO AM I passed!\n");
  }

  // ---Initialize Magnetometer---

  // Reboot magnetometer
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, CFG_REG_A_M, 0x40);
  nrf_delay_ms(100); // needs delay to wait for reboot

  // Enable Block Data Update
  // Only updates sensor data when both halves of the data has been read
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, CFG_REG_C_M, 0x10);

  // Configure magnetometer at 100Hz, continuous mode
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, CFG_REG_A_M, 0x0C);

  // Read WHO AM I register
  int magnet_id = i2c_reg_read(LSM303AGR_MAG_ADDRESS, WHO_AM_I_M);
  printf("Magnetometer WHO AM I: %X\n", magnet_id);
  if (magnet_id != 0b01000000) {
    printf("Magnetometer WHO AM I failed!\n");
  } else {
    printf("Magnetometer WHO AM I passed!\n");
  }

  // ---Initialize Temperature---

  // Enable temperature sensor
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, TEMP_CFG_REG_A, 0xC0);
}

// Read the internal temperature sensor
//
// Return measurement as floating point value in degrees C
float lsm303agr_read_temperature(void) {
  uint8_t low = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_TEMP_L_A);
  uint8_t high = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_TEMP_H_A);
  int16_t reading = ((uint16_t)high << 8) | (uint16_t)low;

  printf("Temperature reading: %X\n", reading);

  // Convert to degrees C
  float temp = (float)reading * (1.0 / 256.0) + 25.0;

  return temp;
}

lsm303agr_measurement_t lsm303agr_read_accelerometer(void) {
  //TODO: implement me
  uint8_t x_l = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_X_L_A);
  uint8_t x_h = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_X_H_A);
  uint8_t y_l = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Y_L_A);
  uint8_t y_h = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Y_H_A);
  uint8_t z_l = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Z_L_A);
  uint8_t z_h = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Z_H_A);

  int16_t x = ((uint16_t)x_h << 8) | (uint16_t)x_l;
  int16_t y = ((uint16_t)y_h << 8) | (uint16_t)y_l;
  int16_t z = ((uint16_t)z_h << 8) | (uint16_t)z_l;

  x = x >> 6;
  y = y >> 6;
  z = z >> 6;

  float x_val = (float)x * 3.9 * 0.001;
  float y_val = (float)y * 3.9 * 0.001;
  float z_val = (float)z * 3.9 * 0.001;

  lsm303agr_measurement_t measurement = {
    .x_axis = x_val,
    .y_axis = y_val,
    .z_axis = z_val
  };
  return measurement;
}

float acceleration_to_tilt(float x, float y, float z) {
  float phi = atan(sqrt(x * x + y * y) / z);
  return phi / M_PI * 180.0; // convert to degrees
}

lsm303agr_measurement_t lsm303agr_read_magnetometer(void) {
  //TODO: implement me

  uint8_t x_l = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTX_L_REG_M);
  uint8_t x_h = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTX_H_REG_M);
  uint8_t y_l = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTY_L_REG_M);
  uint8_t y_h = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTY_H_REG_M);
  uint8_t z_l = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTZ_L_REG_M);
  uint8_t z_h = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTZ_H_REG_M);

  int16_t x = ((uint16_t)x_h << 8) | (uint16_t)x_l;
  int16_t y = ((uint16_t)y_h << 8) | (uint16_t)y_l;
  int16_t z = ((uint16_t)z_h << 8) | (uint16_t)z_l;

  float sens = 1.5 * 0.1; // milligauss per LSB, times microtesla per milligauss
  // bias is 0
  float x_ut = (float)x * sens;
  float y_ut = (float)y * sens;
  float z_ut = (float)z * sens;

  lsm303agr_measurement_t measurement = {
    .x_axis = x_ut,
    .y_axis = y_ut,
    .z_axis = z_ut
  };

  return measurement;
}

