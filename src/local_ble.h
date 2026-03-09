#ifndef BT_LOCAL_BLE
#define BT_LOCAL_BLE

#include <zephyr/types.h>

static void connectionParametersUpdated(struct bt_conn *conn, uint8_t err);

void on_le_data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info);

void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param);

static void update_phy(struct bt_conn *conn);

static void update_data_length(struct bt_conn *conn);

#endif