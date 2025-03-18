#include "ble.h"
#include "info.h"

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
#include "cmd.h"

/* Custom Service Variables */
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static const struct bt_uuid_128 service_uuid = BT_UUID_INIT_128(
    BT_UUID_CUSTOM_SERVICE_VAL
);

#define BT_UUID_READ_CHAR           BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x9ABC, 0xDEF012345679))
#define BT_UUID_WRITE_CHAR          BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x9ABC, 0xDEF012345680))
#define BT_UUID_GATT_CHRC_USER_DESC BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x9ABC, 0xDEF012345681))

#define BUF_SIZE 512

static char read_value[BUF_SIZE];
static uint8_t write_value[BUF_SIZE];

static ssize_t read_char_callback(struct bt_conn* conn,
    const struct bt_gatt_attr* attr,
    void* buf, uint16_t len, uint16_t offset) {
    fprintf(stderr, "Received Read\n");
    info_to_buf_str(read_value, BUF_SIZE);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, read_value, sizeof(read_value));
}

static ssize_t write_char_callback(struct bt_conn* conn,
    const struct bt_gatt_attr* attr,
    const void* buf, uint16_t len, uint16_t offset,
    uint8_t flags) {
    if (offset + len > sizeof(write_value)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(write_value + offset, buf, len);

    // terminate strings
    if (offset + len + 1 >= BUF_SIZE) {
        write_value[BUF_SIZE - 1] = 0;
    } else {
        write_value[offset + len] = 0;
    }

    fprintf(stderr, "Received Write: %.*s\n", len, (char*)buf);

    char* orig_write_value = write_value;
    execute_command(write_value, BUF_SIZE);

    char** bruh = &write_value;
    *bruh = orig_write_value;

    return len;
}

BT_GATT_SERVICE_DEFINE(custom_svc,
    BT_GATT_PRIMARY_SERVICE(&service_uuid),
    BT_GATT_CHARACTERISTIC(BT_UUID_READ_CHAR, BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ, read_char_callback, NULL, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_GATT_CHRC_USER_DESC, BT_GATT_PERM_READ,
        NULL, NULL, "All feeder settings"),
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

int ble_init(void) {
    int err = bt_enable(NULL);

    if (err) {
        return -1;
    }

    fprintf(stderr, "Bluetooth initialized\n");

    bt_ready();
    return 0;
}
