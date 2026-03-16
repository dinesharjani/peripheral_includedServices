#ifndef BT_LOCAL_BLE
#define BT_LOCAL_BLE

#include <zephyr/types.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>

void connectionParametersUpdated(struct bt_conn *conn, uint8_t err);

void onLEdataLengthUpdated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info);

void onLEphyUpdated(struct bt_conn *conn, struct bt_conn_le_phy_info *param);

void updatePHY(struct bt_conn *conn);

void updateDataLength(struct bt_conn *conn);

#endif