#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <dk_buttons_and_leds.h>

#include "local_ble.h"

LOG_MODULE_DECLARE(Local_BLE);

void connectionParametersUpdated(struct bt_conn *conn, uint8_t err) {
	struct bt_conn_info info;
	err = bt_conn_get_info(conn, &info);
	if (err) {
		LOG_ERR("bt_conn_get_info failed (err %d)", err);
		return;
  }

	double connectionInterval = BT_GAP_US_TO_CONN_INTERVAL(info.le.interval_us) * 1.25; // in ms
	uint16_t supervisionTimeout = info.le.timeout * 10; // in ms
	LOG_INF("Connection parameters: interval %.2lf ms, latency %d intervals, timeout %d ms", connectionInterval, info.le.latency, supervisionTimeout);
}

void onLEdataLengthUpdated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info) {
  uint16_t tx_len     = info->tx_max_len; 
  uint16_t tx_time    = info->tx_max_time;
  uint16_t rx_len     = info->rx_max_len;    
  uint16_t rx_time    = info->rx_max_time;
  
  LOG_INF("Data length updated. Length %d/%d bytes, time %d/%d us", tx_len, rx_len, tx_time, rx_time);
}

void onLEphyUpdated(struct bt_conn *conn, struct bt_conn_le_phy_info *param) {
  // PHY Updated
  if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_1M) {
    LOG_INF("PHY updated. New PHY: 1M");
  } else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_2M) {
    LOG_INF("PHY updated. New PHY: 2M");
  } else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_CODED_S8) {
    LOG_INF("PHY updated. New PHY: Long Range");  
  }
}

void updatePHY(struct bt_conn *conn) {
  const struct bt_conn_le_phy_param preferred_phy = {
    .options = BT_CONN_LE_PHY_OPT_NONE,
    .pref_rx_phy = BT_GAP_LE_PHY_2M,
    .pref_tx_phy = BT_GAP_LE_PHY_2M,  
  };
    
  int err = bt_conn_le_phy_update(conn, &preferred_phy);
  if (err) {
    LOG_ERR("bt_conn_le_phy_update() returned %d", err);  
  }
}

void updateDataLength(struct bt_conn *conn) {
  struct bt_conn_le_data_len_param my_data_len = {
    .tx_max_len = BT_GAP_DATA_LEN_MAX,
    .tx_max_time = BT_GAP_DATA_TIME_MAX,  
  };
    
  int err = bt_conn_le_data_len_update(conn, &my_data_len);
  if (err) {		  
    LOG_ERR("data_len_update failed (err %d)", err);
  }
}