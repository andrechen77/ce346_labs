/*
 * Copyright (c) 2021, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/display.h>
#include <string.h>
#include "display.h"

#define PIXEL_BIT(idx, val)  (val ? BIT(idx) : 0)
#define PIXEL_MASK(...)      (FOR_EACH_IDX(PIXEL_BIT, (|), __VA_ARGS__))

static struct display_capabilities caps;
uint8_t display_buf[5];
static const struct display_buffer_descriptor buf_desc = {
	.buf_size = sizeof(display_buf),
	.width = 5,
	.height = 5,
	.pitch = 8,
};

const struct device* dev;

int display_update(void) {
	int ret;
	ret = display_write(dev, 0, 0, &buf_desc, display_buf);
	if (ret < 0) {
		fprintf(stderr, "display_write failed: %u/%d\n",
			__LINE__, ret);
		return ret;
	}

	ret = display_blanking_off(dev);
	if (ret < 0) {
		fprintf(stderr, "display_blanking_off failed: %u/%d\n",
			__LINE__, ret);
		return ret;
	}

	return 0;
}

int display_init(void) {
	int ret;
	dev = DEVICE_DT_GET_ONE(nordic_nrf_led_matrix);

	if (!dev) {
		fprintf(stderr, "Display device not ready\n");
		return 0;
	}

	display_get_capabilities(dev, &caps);
	if (!(caps.supported_pixel_formats & PIXEL_FORMAT_MONO01)) {
		fprintf(stderr, "Expected pixel format not supported\n");
		return 0;
	}

	ret = display_set_pixel_format(dev, PIXEL_FORMAT_MONO01);
	if (ret < 0) {
		fprintf(stderr, "display_set_pixel_format failed: %u/%d\n",
			__LINE__, ret);
	}

	ret = display_set_brightness(dev, 0x7F);
	if (ret < 0) {
		fprintf(stderr, "display_set_brightness failed: %u/%d\n",
			__LINE__, ret);
	}

	fprintf(stderr, "Initialized display\n");

	display_buf[0] = 0b00011111;
	display_buf[1] = 0b00011111;
	display_buf[2] = 0b00011111;
	display_buf[3] = 0b00011111;
	display_buf[4] = 0b00011111;

	display_update();
	return 0;
}

void write_ring_pattern(uint8_t outer_ring, uint8_t inner_ring, bool center) {
	memset(display_buf, 0, sizeof(display_buf)); // Clear buffer

	// Define outer ring LED positions (clockwise order)
	const uint8_t outer_ring_positions[16][2] = {
		{0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {1,4}, {2,4}, {3,4},
		{4,4}, {4,3}, {4,2}, {4,1}, {4,0}, {3,0}, {2,0}, {1,0}
	};

	// Define inner ring LED positions (clockwise order)
	const uint8_t inner_ring_positions[8][2] = {
		{1,1}, {1,2}, {1,3}, {2,3}, {3,3}, {3,2}, {3,1}, {2,1}
	};

	// Light up outer ring LEDs
	for (uint8_t i = 0; i < outer_ring && i < 16; i++) {
		uint8_t row = outer_ring_positions[i][0];
		uint8_t col = outer_ring_positions[i][1];
		display_buf[row] |= (1 << col);
	}

	// Light up inner ring LEDs
	for (uint8_t i = 0; i < inner_ring && i < 8; i++) {
		uint8_t row = inner_ring_positions[i][0];
		uint8_t col = inner_ring_positions[i][1];
		display_buf[row] |= (1 << col);
	}

	// Light up center LED if requested
	if (center) {
		display_buf[2] |= (1 << 2);
	}

	display_update();
}

void write_row_pattern(uint8_t n) {
	// Ensure n is within the valid range
	if (n > 25) {
		n = 25;
	}

	// Clear the display buffer
	memset(display_buf, 0, sizeof(display_buf));

	// Set LEDs in row-major order
	for (int i = 0; i < n; i++) {
		int row = i / 5;  // Determine the row (0 to 4)
		int col = i % 5;  // Determine the column (0 to 4)
		display_buf[row] |= (1 << col);  // Set the corresponding bit to 1
	}

	display_update();
}

