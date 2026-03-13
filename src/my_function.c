
#include "my_function.h"
#include "local_lbs.h"

LOG_MODULE_DECLARE(MY_FUNCTION_H);

#define BT_UUID_PRI_SRC BT_UUID_128_ENCODE(0x6E400001, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E)
#define BT_UUID_MY_SERVICE BT_UUID_DECLARE_128(BT_UUID_PRI_SRC)

// #define BT_UUID_HUE_SERV BT_UUID_128_ENCODE(0x57A70000, 0x9350, 0x11ED, 0xA1EB, 0x0242AC120002)
// #define BT_UUID_SECONDARY_HUE_SERVICE BT_UUID_DECLARE_128(BT_UUID_HUE_SERV)

#define BT_UUID_LED_VAL BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_LED_CHAR BT_UUID_DECLARE_128(BT_UUID_LED_VAL)

#define CUSTOM_READ_VALUE_CHAR_UUID 0x1401
#define CUSTOM_SECOND_VALUE_CHAR_UUID 0x1402

#define BT_UUID_SEC_VAL BT_UUID_128_ENCODE(0x932C32BD, 0x0002, 0x47A2, 0x835A, 0xA8D455B859DD)
#define BT_UUID_SEC_CHAR BT_UUID_DECLARE_128(BT_UUID_SEC_VAL)

static bool read_state;

static int primaryState;
static int secondaryState;
static int includedServiceCharacteristicState;
static int includedServiceSecondCharacteristicState;

static struct secondary_service_cb secondary_cb;

static ssize_t readSecondaryState(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) {
    const char *value = attr->user_data;
	
	if (secondary_cb.sec_cb) {
        secondaryState = secondary_cb.sec_cb();
        LOG_INF("[READ] Secondary Attribute: handle: %u, conn: %p, value %d", attr->handle, (void *)conn, secondaryState);
        return bt_gatt_attr_read(conn, attr, buf, len, offset, (void *)secondaryState, sizeof(secondaryState));
    }
    return 0;
}

static ssize_t readPrimaryState(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) {
    const char *value = attr->user_data;

    if (secondary_cb.sec_cb) {
        primaryState++;
        LOG_INF("[READ] Primary Attribute: handle: %u, conn: %p, value %d", attr->handle, (void *)conn, primaryState);
        return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
    }
	return 0;
}

static ssize_t readIncludedServiceCharacteristic(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) {
    const char *value = attr->user_data;

    includedServiceCharacteristicState++;
    LOG_INF("[READ] Included Service Attribute: handle: %u, conn: %p, value %d", attr->handle, (void *)conn, includedServiceCharacteristicState);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
}

static ssize_t readSecondIncludedServiceCharacteristic(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) {
    const char *value = attr->user_data;
    includedServiceSecondCharacteristicState++;
    LOG_INF("[READ] Included Service (Second) Attribute: handle: %u, conn: %p, value %d", attr->handle, (void *)conn, includedServiceSecondCharacteristicState);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
}

static ssize_t read_button(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) {
    const char *value = attr->user_data;
	LOG_INF("Attribute read, handle: %u, conn: %p", attr->handle, (void *)conn);
    return 0;
}

BT_GATT_SERVICE_DEFINE(secondary_service, BT_GATT_SECONDARY_SERVICE(BT_UUID_LBS),

    BT_GATT_CHARACTERISTIC(BT_UUID_SEC_CHAR, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, readIncludedServiceCharacteristic, NULL, &includedServiceCharacteristicState),

    BT_GATT_CHARACTERISTIC(BT_UUID_LED_CHAR, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, readSecondIncludedServiceCharacteristic, NULL, &includedServiceSecondCharacteristicState),
);

BT_GATT_SERVICE_DEFINE(my_service, BT_GATT_PRIMARY_SERVICE(BT_UUID_MY_SERVICE),

    BT_GATT_INCLUDE_SERVICE(&secondary_service.attrs[0]),

    BT_GATT_CHARACTERISTIC(CUSTOM_READ_VALUE_CHAR_UUID, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, readPrimaryState, NULL, &primaryState),
		       
    BT_GATT_CHARACTERISTIC(BT_UUID_LED_CHAR, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, read_button, NULL, &read_state),
);

// BT_GATT_SERVICE_DEFINE(ess_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),

// 	/* Temperature Sensor 1 */
// 	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
// 			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
// 			       BT_GATT_PERM_READ,
// 			       read_u16, NULL, &sensor_1.temp_value),
// 	BT_GATT_DESCRIPTOR(BT_UUID_ES_MEASUREMENT, BT_GATT_PERM_READ,
// 			   read_es_measurement, NULL, &sensor_1.meas),
// 	BT_GATT_CUD(SENSOR_1_NAME, BT_GATT_PERM_READ),
// 	BT_GATT_DESCRIPTOR(BT_UUID_VALID_RANGE, BT_GATT_PERM_READ,
// 			   read_temp_valid_range, NULL, &sensor_1),
// 	BT_GATT_DESCRIPTOR(BT_UUID_ES_TRIGGER_SETTING,
// 			   BT_GATT_PERM_READ, read_temp_trigger_setting,
// 			   NULL, &sensor_1),
// 	BT_GATT_CCC(temp_ccc_cfg_changed,
// 		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

// 	/* Temperature Sensor 2 */
// 	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
// 			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
// 			       BT_GATT_PERM_READ,
// 			       read_u16, NULL, &sensor_2.temp_value),
// 	BT_GATT_DESCRIPTOR(BT_UUID_ES_MEASUREMENT, BT_GATT_PERM_READ,
// 			   read_es_measurement, NULL, &sensor_2.meas),
// 	BT_GATT_CUD(SENSOR_2_NAME, BT_GATT_PERM_READ),
// 	BT_GATT_DESCRIPTOR(BT_UUID_VALID_RANGE, BT_GATT_PERM_READ,
// 			   read_temp_valid_range, NULL, &sensor_2),
// 	BT_GATT_DESCRIPTOR(BT_UUID_ES_TRIGGER_SETTING,
// 			   BT_GATT_PERM_READ, read_temp_trigger_setting,
// 			   NULL, &sensor_2),
// 	BT_GATT_CCC(temp_ccc_cfg_changed,
// 		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

// 	/* Humidity Sensor */
// 	BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY, BT_GATT_CHRC_READ,
// 			       BT_GATT_PERM_READ,
// 			       read_u16, NULL, &sensor_3.humid_value),
// 	BT_GATT_CUD(SENSOR_3_NAME, BT_GATT_PERM_READ),
// 	BT_GATT_DESCRIPTOR(BT_UUID_ES_MEASUREMENT, BT_GATT_PERM_READ,
// 			   read_es_measurement, NULL, &sensor_3.meas),
// );

// static ssize_t write_led(struct bt_conn *conn,
// 			 const struct bt_gatt_attr *attr,
// 			 const void *buf, uint16_t len, uint16_t offset, uint8_t flags) {
// 	LOG_INF("Attribute write, handle: %u, conn: %p", attr->handle,
// 		(void *)conn);

// 	if (len != 1U) {
// 		LOG_INF("Write led: Incorrect data length");
// 		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
// 	}

// 	if (offset != 0) {
// 		LOG_INF("Write led: Incorrect data offset");
// 		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
// 	}

// 	if (lbs_cb.led_cb) {
// 		//Read the received value 
// 		uint8_t val = *((uint8_t *)buf);

// 		if (val == 0x00 || val == 0x01) {
// 			//Call the application callback function to update the LED state
// 			lbs_cb.led_cb(val ? true : false);
// 		} else {
// 			LOG_INF("Write led: Incorrect value");
// 			return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
// 		}
// 	}

// 	return len;
// }

// /* A function to register application callbacks for the LED and Button characteristics  */
// int my_lbs_init(struct my_lbs_cb *callbacks) {
// 	if (callbacks) {
// 		lbs_cb.led_cb = callbacks->led_cb;
// 		lbs_cb.button_cb = callbacks->button_cb;
// 	}

// 	return 0;
// }

uint32_t ble_sec_init(struct secondary_service_cb *callbacks) {
    if (callbacks) {
        secondary_cb.sec_cb = callbacks->sec_cb;
    }
    return 0;
}