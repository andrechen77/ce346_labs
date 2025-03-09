// #include "pin_defs.h"
// #include "ultrasonic.h"
// #include <zephyr/kernel.h>
// #include <zephyr/device.h>
// #include <zephyr/drivers/gpio.h>
// #include <stdio.h>
// #include "pwm.h"
// #include "adc.h"

// #define ADC_THREAD_STACK_SIZE 1024  // Adjust based on usage
// #define ADC_THREAD_PRIORITY 5       // Higher number = lower priority
// #define ADC_THRESHOLD 100

// K_THREAD_STACK_DEFINE(adc_thread_stack, ADC_THREAD_STACK_SIZE);
// static struct k_thread adc_thread_data;

// void adc_thread(void* p1, void* p2, void* p3) {
// 	while (1) {
// 		int val = adcread();
// 		if (val > ADC_THRESHOLD) {
// 			// set duty to something
// 			printf("Hello world!\n");
// 			pwm_set_duty((float)val / 4096.0);
// 		} else {
// 			// set duty to 0
// 			pwm_set_duty(0);
// 		}
// 		k_sleep(K_MSEC(10));
// 	}
// }


// int main(void) {
// 	fprintf(stderr, "Hello, there!\n");
// 	k_sleep(K_MSEC(10)); // let the RTOS do its thing

// 	// Initialize the ADC
// 	if (adc_begin() != 0) {
// 		fprintf(stderr, "Error initializing ADC\n");
// 		return -1;
// 	}
// 	fprintf(stderr, "ADC initialized\n");

// 	// Initialize the PWM
// 	pwm_init();
// 	fprintf(stderr, "PWM initialized\n");

// 	k_thread_create(&adc_thread_data, adc_thread_stack,
// 		K_THREAD_STACK_SIZEOF(adc_thread_stack),
// 		adc_thread, NULL, NULL, NULL,
// 		ADC_THREAD_PRIORITY, 0, K_NO_WAIT);

// 	// while (1) {
// 	// 	pwm_set_duty(0);
// 	// 	k_sleep(K_SECONDS(1));
// 	// 	pwm_set_duty(0.5);
// 	// 	k_sleep(K_SECONDS(1));
// 	// }


// 	// while (1) {
// 	// 	int reading = adcread();
// 	// 	fprintf(stderr, "ADC reading: %d\n", reading);
// 	// 	k_sleep(K_SECONDS(1));
// 	// }

// 	// fprintf(stderr, "Initializing ultrasonic sensors\n");
// 	// ultrasonic_init();
// 	// fprintf(stderr, "Ultrasonic sensors initialized\n");
// 	// k_sleep(K_MSEC(1000));

// 	// while (1) {
// 	// 	double distance = sample_sensors_distance_cm();
// 	// 	fprintf(stderr, "Distance: %f cm\n", distance);
// 	// 	k_sleep(K_MSEC(1000));
// 	// }

// 	while (1) {
// 		k_sleep(K_SECONDS(1));
// 	}

// 	return -1;
// }

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/ias.h>

 /* Custom Service Variables */
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static const struct bt_uuid_128 service_uuid = BT_UUID_INIT_128(
    BT_UUID_CUSTOM_SERVICE_VAL
);

#define BT_UUID_READ_CHAR        BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x9ABC, 0xDEF012345679))
#define BT_UUID_WRITE_CHAR       BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x9ABC, 0xDEF012345680))

static const char read_value[] = "Hello BLE!";
static uint8_t write_value[20];

static ssize_t read_char_callback(struct bt_conn *conn,
                                    const struct bt_gatt_attr *attr,
                                    void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, read_value, sizeof(read_value));
}

static ssize_t write_char_callback(struct bt_conn *conn,
                                    const struct bt_gatt_attr *attr,
                                    const void *buf, uint16_t len, uint16_t offset,
                                    uint8_t flags)
{
    if (offset + len > sizeof(write_value)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(write_value + offset, buf, len);
    printk("Received Write: %.*s\n", len, (char *)buf);
    
    return len;
}

BT_GATT_SERVICE_DEFINE(custom_svc,
    BT_GATT_PRIMARY_SERVICE(&service_uuid),
    BT_GATT_CHARACTERISTIC(BT_UUID_READ_CHAR, BT_GATT_CHRC_READ,
                            BT_GATT_PERM_READ, read_char_callback, NULL, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_WRITE_CHAR, BT_GATT_CHRC_WRITE,
                            BT_GATT_PERM_WRITE, NULL, write_char_callback, write_value),
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL),

};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static void connected(struct bt_conn* conn, uint8_t err) {
	if (err) {
		fprintf(stderr, "Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
	} else {
		fprintf(stderr, "Connected\n");
	}
}

static void disconnected(struct bt_conn* conn, uint8_t reason) {
	fprintf(stderr, "Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(void) {
	int err;

	fprintf(stderr, "Bluetooth initialized\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_ONE_TIME, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		fprintf(stderr, "Advertising failed to start (err %d)\n", err);
		return;
	}

	fprintf(stderr, "Advertising successfully started\n");
}


int main(void) {
	struct bt_gatt_attr* vnd_ind_attr;
	char str[BT_UUID_STR_LEN];
	int err;

	err = bt_enable(NULL);
	if (err) {
		fprintf(stderr, "Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	bt_ready();

	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(1));

		/* Vendor indication simulation */
		// if (simulate_vnd && vnd_ind_attr) {
		// 	if (indicating) {
		// 		continue;
		// 	}

		// 	ind_params.attr = vnd_ind_attr;
		// 	ind_params.func = indicate_cb;
		// 	ind_params.destroy = indicate_destroy;
		// 	ind_params.data = &indicating;
		// 	ind_params.len = sizeof(indicating);

		// 	if (bt_gatt_indicate(NULL, &ind_params) == 0) {
		// 		indicating = 1U;
		// 	}
		// }
	}
	return 0;
}