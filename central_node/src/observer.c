#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <zephyr/pm/pm.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <hal/nrf_gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include "headers/main_observer.h"
#include <zephyr/bluetooth/hci.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/bluetooth/bluetooth.h>

// Logger
LOG_MODULE_DECLARE(gateway, LOG_LEVEL_DBG);

// Program Constants
#define STACKSIZE	 2048
#define THREAD0_PRIORITY 7

// UART Configuration Parameters
#define UART_BUF_SIZE        16
#define UART_TX_TIMEOUT_MS   100
#define UART_RX_TIMEOUT_MS   100
#define UART_TX_BUF_SIZE         256
#define UART_RX_MSG_QUEUE_SIZE   12
#define CHUNK_SIZE 8

// Function semaphores
K_SEM_DEFINE(tx_done, 1, 1);
K_SEM_DEFINE(rx_disabled, 0, 1);

// Structure to represent a single item in a message queue for UART communication
struct uart_msg_queue_item {			// By using this structure, you can create instances of uart_msg_queue_item to represent individual items in a message queue for UART communication. 
	uint8_t bytes[UART_BUF_SIZE];		// Each instance will have its own bytes array to hold the data and a length value to indicate the size of the data. 
	uint32_t length;				// This structure provides a convenient way to organize and manage UART messages within a queue.
};

// UART device
static const struct device *dev_uart;
static uint8_t last_data[CHUNK_SIZE] = {0};

// UART TX fifo
RING_BUF_DECLARE(app_tx_fifo, UART_TX_BUF_SIZE);
volatile int bytes_claimed;

// UART RX primary buffers
uint8_t uart_double_buffer[2][UART_BUF_SIZE];
uint8_t *uart_buf_next = uart_double_buffer[1];

// UART RX message queue
K_MSGQ_DEFINE(uart_rx_msgq, sizeof(struct uart_msg_queue_item), UART_RX_MSG_QUEUE_SIZE, 4);

// Function to get the next available buffer for UART TX
static int uart_tx_get_from_queue(void){
	uint8_t *data_ptr;
	bytes_claimed = ring_buf_get_claim(&app_tx_fifo, &data_ptr, UART_TX_BUF_SIZE);	// Try to claim any available bytes in the FIFO

	if(bytes_claimed > 0) { 	// Start a UART transmission based on the number of available bytes
		uart_tx(dev_uart, data_ptr, bytes_claimed, SYS_FOREVER_MS);
	}

	return bytes_claimed;
}

void app_uart_async_callback(const struct device *uart_dev,struct uart_event *evt, void *user_data){
	static struct uart_msg_queue_item new_message;

	switch (evt->type) {
		case UART_TX_DONE:
			ring_buf_get_finish(&app_tx_fifo, bytes_claimed); // Empty the FIFO of the bytes that were transmitted

			if(uart_tx_get_from_queue() == 0) { // If there are no more bytes to transmit
				k_sem_give(&tx_done); // Release the semaphore
			}
			break;
		
		case UART_RX_RDY:
			memcpy(new_message.bytes, evt->data.rx.buf + evt->data.rx.offset, evt->data.rx.len); // Copy the received data into the message structure
			new_message.length = evt->data.rx.len;
			if(k_msgq_put(&uart_rx_msgq, &new_message, K_NO_WAIT) != 0){
				printk("Error: Uart RX message queue full!\n");
			}
			break;
		
		case UART_RX_BUF_REQUEST:
			uart_rx_buf_rsp(dev_uart, uart_buf_next, UART_BUF_SIZE); // Provide the next available buffer for UART RX
			break;

		case UART_RX_BUF_RELEASED:
			uart_buf_next = evt->data.rx_buf.buf; // Update the next available buffer for UART RX
			break;

		case UART_RX_DISABLED:
			k_sem_give(&rx_disabled); // Release the semaphore
			break;
		
		default:
			break;
	}
}

static void app_uart_init(void)
{
	dev_uart = device_get_binding("UART_0"); // Get the UART binding

	if (dev_uart == NULL) {
		printk("Failed to get UART binding\n");
		return;
	}

	uart_callback_set(dev_uart, app_uart_async_callback, NULL); // Set the UART callback
	uart_rx_enable(dev_uart, uart_double_buffer[0], UART_BUF_SIZE, UART_RX_TIMEOUT_MS); // Enable UART RX
}

// Function to send UART data, by writing it to a ring buffer (FIFO) in the application
// WARNING: This function is not thread safe! If you want to call this function from multiple threads a semaphore should be used
static int app_uart_send(const uint8_t * data_ptr, uint32_t data_len)
{
	while(1) {
		// Try to move the data into the TX ring buffer
		uint32_t written_to_buf = ring_buf_put(&app_tx_fifo, data_ptr, data_len);
		data_len -= written_to_buf;
		
		// In case the UART TX is idle, start transmission
		if(k_sem_take(&tx_done, K_NO_WAIT) == 0) {
			uart_tx_get_from_queue();
		}   
		
		// In case all the data was written, exit the loop
		if(data_len == 0) break;

		// In case some data is still to be written, sleep for some time and run the loop one more time
		k_msleep(10);
		data_ptr += written_to_buf;
	}

	return 0;
}

// Scanning BLE Callback Function
#if defined(CONFIG_BT_EXT_ADV)
     // Callback function for parsing the data
	static bool data_cb(struct bt_data *data, void *user_data)
	{
		uint8_t write_array[CHUNK_SIZE];
		write_array[0] = 0;


		if (data->data_len >= CHUNK_SIZE) {
			// Check if the data is the same as last processed
			if (memcmp(data->data, last_data, CHUNK_SIZE) == 0) {
				return false; // Skip processing if data is the same
			}

			// Update last processed data
			memcpy(last_data, data->data, CHUNK_SIZE);

			switch (data->type) {
				case BT_DATA_NAME_SHORTENED:
				case BT_DATA_NAME_COMPLETE:
				case BT_DATA_MANUFACTURER_DATA:
					int index = 0;
					int j = 0;

					// Storing CHUNK_SIZE bytes of data at once
					for (j = 0; j < CHUNK_SIZE; j++) {
						// Using char (char = 1 byte uint8_t type)
						char number = data->data[j + index];
						// Appending them in write array
						write_array[j] = number;
					}

					char formatted_string[128];
					snprintf(formatted_string, sizeof(formatted_string), 
							"AT+QMTPUBEX=0,0,0,0,\"channels/2588719/publish\",68\r\nfield5=%d&field1=%d&field2=%d&field3=%d&field4=%d&status=MQTTPUBLISH\r\n", 
							write_array[0], write_array[1], write_array[2], write_array[3], write_array[4]
					);
					app_uart_send((const uint8_t *)formatted_string, strlen(formatted_string));

			}
    	}
		return true;
	}

	// Callback function for scanning
	static void scan_recv(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf)
	{
		/* Executed when we recieve the packet from advertising node. */
		char le_addr[BT_ADDR_LE_STR_LEN];
		uint8_t data_status;
		uint16_t data_len;
		char name[30];
		data_len = buf->len;
		bt_data_parse(buf, data_cb, name);
		data_status = BT_HCI_LE_ADV_EVT_TYPE_DATA_STATUS(info->adv_props);
		bt_addr_le_to_str(info->addr, le_addr, sizeof(le_addr));
	}

	// Callback structure for scanning
	static struct bt_le_scan_cb scan_callbacks = {
		.recv = scan_recv,
	};

#endif /* CONFIG_BT_EXT_ADV */

// Function to start the observer
int observer_start(void)
{
	// scanning part /////////////////////////////////////////////////////////////////////////////////
	bt_addr_le_t addr1;
	bt_addr_le_from_str("FE:BB:EF:AF:BA:13", "random", &addr1); // scannable device address
	int err = bt_le_filter_accept_list_add(&addr1);

	if (err == 0) {
		printk("Successfully added the address to Filter Accept List..\n");
	} else {
		printk("Failed to update Filter Accept List..\n");
	}

	// Scanning parameters
	struct bt_le_scan_param scan_param = {
		.type = BT_LE_SCAN_TYPE_PASSIVE,
		.options = BT_LE_SCAN_OPT_FILTER_DUPLICATE | BT_LE_SCAN_OPT_FILTER_ACCEPT_LIST,
		.interval = 0x0030,
		.window = 0x0030,
	};

	// Register the scanning callbacks
	bt_le_scan_cb_register(&scan_callbacks);

     // trasnmission to Thingspeak part ////////////////////////////////////////////////////////////////////////
	printk("UART Async example started\n");
	app_uart_init();

	// Array of test strings
	const uint8_t *test_strings[] = {
		"AT+QMTCFG=\"recv/mode\",0,0,1\r\n",
		"AT+QMTOPEN=0,\"mqtt3.thingspeak.com\",1883\r\n",
		"AT+QMTCONN=0,\"KxsxCgMeBiYMJxMgIgk1BDg\",\"KxsxCgMeBiYMJxMgIgk1BDg\",\"j5UcciTBw1jlqc5vVv3k8q50\"\r\n"
	};

	// Number of test strings
	const size_t num_test_strings = sizeof(test_strings) / sizeof(test_strings[0]);
	// Current string index
	size_t current_string_index = 0;

	// Send the first three test strings sequentially
	for (current_string_index = 0; current_string_index < num_test_strings; current_string_index++) {
		app_uart_send(test_strings[current_string_index], strlen(test_strings[current_string_index]));
		// Wait for the response to be processed
		k_msleep(4000);
	}

	// Start scanning and register callbacks

	err = bt_le_scan_start(&scan_param, NULL);

	if (err) {
		LOG_INF("Start scanning failed (err %d)\n", err);
		return err;
	}

	LOG_INF("Started scanning...\n");
	
	return 0;
}


	

