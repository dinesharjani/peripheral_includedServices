#ifndef MY_FUNCTION_H
#define MY_FUNCTION_H

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <soc.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/addr.h>
#include <zephyr/settings/settings.h>

/** @brief Callback type for when the button state is pulled. */
typedef int (*sec_cb_t)(void);

/** @brief Callback struct used by the LBS Service. */
struct secondary_service_cb {
	
	/** Read callback. */
	sec_cb_t sec_cb;
};

/** @brief Callback type for when the button state is pulled. */
// typedef int (*sec_cb_t)(void);

typedef void (*secondary_cb_t)(const uint8_t secondary_state);

/**@brief Function for initializing the Custom Service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_sec_init(struct secondary_service_cb *callbacks);

#endif