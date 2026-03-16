/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/settings/settings.h>
#include <dk_buttons_and_leds.h>

#include "local_ble.h"
#include "local_lbs.h"
#include "my_function.h"

#define DEVICE_NAME             CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN         (sizeof(DEVICE_NAME) - 1)
#define COMPANY_ID_CODE 		0x0059

#define RUN_STATUS_LED          DK_LED1
#define CON_STATUS_LED          DK_LED2
#define RUN_LED_BLINK_INTERVAL  1000

#define USER_LED                DK_LED3
#define USER_BUTTON             DK_BTN1_MSK

LOG_MODULE_REGISTER(Nordic_Peripheral, LOG_LEVEL_INF);

static bool app_button_state;
static int secondaryState;
static struct k_work adv_work;

static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM((BT_LE_ADV_OPT_CONN|BT_LE_ADV_OPT_USE_IDENTITY), 
                800, /*Min Advertising Interval 500ms (800*0.625ms) */
                801, /*Max Advertising Interval 500.625ms (801*0.625ms)*/
                NULL); /* Set to NULL for undirected advertising*/

typedef struct adv_mfg_data {
	uint16_t company_code; /* Company Identifier Code. */
	uint16_t number_press; /* Number of times Button 1 is pressed */
} adv_mfg_data_type;

static adv_mfg_data_type adv_mfg_data = { COMPANY_ID_CODE, 0x00 };
static struct bt_gatt_exchange_params exchange_params;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data)),
};

static struct bt_conn *my_conn;

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_LBS_VAL),
};

static void adv_work_handler(struct k_work *work) {
	int err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return;
	}

	LOG_INF("Advertising successfully started");
}

static void advertising_start(void) {
	k_work_submit(&adv_work);
}

static void app_led_cb(bool led_state) {
	dk_set_led(USER_LED, led_state);
}

static bool app_button_cb(void) {
	return app_button_state;
}

static struct my_lbs_cb app_callbacks = {
	.led_cb    = app_led_cb,
	.button_cb = app_button_cb,
};

static void button_changed(uint32_t button_state, uint32_t has_changed) {
	int err;
	if (has_changed & USER_BUTTON) {
		adv_mfg_data.number_press += 1;
		bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

		uint32_t user_button_state = button_state & USER_BUTTON;
		LOG_INF("Button %s", (user_button_state ? "Pressed" : "Released"));
		// err = bt_lbs_send_button_state(user_button_state);
		// if (err) {
		// 	LOG_ERR("Couldn't send notification. (err: %d)", err);
		// }
		button_state = user_button_state ? true : false;
	}
}

static void connected(struct bt_conn *conn, uint8_t err) {
	if (err) {
		LOG_ERR("Connection failed, err 0x%02x %s", err, bt_hci_err_to_str(err));
		return;
	}

	LOG_INF("Connected");
	my_conn = bt_conn_ref(conn);
	dk_set_led_on(CON_STATUS_LED);

	connectionParametersUpdated(conn, err);

	k_sleep(K_MSEC(1000));  // Delay added to avoid link layer collisions.
	updatePHY(conn);

	k_sleep(K_MSEC(1000));  // Delay added to avoid link layer collisions.
	updateDataLength(conn);
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
	LOG_INF("Disconnected, reason 0x%02x %s", reason, bt_hci_err_to_str(reason));
	bt_conn_unref(conn);
	dk_set_led_off(CON_STATUS_LED);
}

static void recycled_cb(void) {
	LOG_INF("Connection object available from previous conn. Disconnect is complete!");
	advertising_start();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected           = connected,
	.disconnected        = disconnected,
	.recycled            = recycled_cb,
	.le_param_updated    = connectionParametersUpdated,
	.le_phy_updated      = onLEphyUpdated,
	.le_data_len_updated = onLEdataLengthUpdated,
};

static int init_button(void) {
	int err = dk_buttons_init(button_changed);
	if (err) {
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}

	return err;
}

static int secondary_read_cb(void) {
	secondaryState++;
	return secondaryState;
}

static struct secondary_service_cb secondary_callbacks = {
	.sec_cb = secondary_read_cb,
};

int main(void) {
	int blink_status = 0;
	int err;

	LOG_INF("Starting Bluetooth Peripheral LBS sample");

	err = dk_leds_init();
	if (err) {
		LOG_ERR("LEDs init failed (err %d)", err);
		return 0;
	}

	err = init_button();
	if (err) {
		LOG_ERR("Button init failed (err %d)", err);
		return 0;
	}

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return 0;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	// bt_conn_cb_register(&connection_callbacks);

	err = my_lbs_init(&app_callbacks);
	if (err) {
		LOG_ERR("Failed to init LBS (err: %d)", err);
		return -1;
	}

	err = ble_sec_init(&secondary_callbacks);
	if (err) {
		LOG_ERR("Failed to init Secondary Service Callbacks (err: %d)", err);
		return -1;
	}

	LOG_INF("Bluetooth initialized");

	k_work_init(&adv_work, adv_work_handler);
	advertising_start();

	for (;;) {
		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}
