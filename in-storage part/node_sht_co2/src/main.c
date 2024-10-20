#include <zephyr/zephyr.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/pm/pm.h>
#include <hal/nrf_gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/bluetooth/controller.h>
#include <stdio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor/mhz19b.h>


struct bt_le_ext_adv *adv;                                                            // Declare a pointer to Bluetooth Low Energy extended advertisement structure
const struct device *const sht = DEVICE_DT_GET_ANY(sensirion_sht4x);                // Retrieve the SHT device from the device tree
const struct device *dev; //Runtime device structure (in ROM) per driver instance

int8_t temp_value1, temp_value2;                                                    // Declare variables to store temperature and humidity values
int8_t hum_value1, hum_value2;
int8_t co2_value1, co2_value2;                                                      // Declare variables to store CO2 values
struct sensor_value val; //Sensor Readout Value

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int err;                                                                           // Declare a variable to store error codes

struct bt_le_adv_param adv_param = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_NONE | BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_USE_IDENTITY | BT_LE_ADV_OPT_NO_2M,
                                                        0x30,
                                                        0x30, // advertising interval is set to 3 seconds
                                                        NULL);
// start advertising with no timeout and exactly one extended advertising event allowed in the extended advertising set.
struct bt_le_ext_adv_start_param ext_adv_param = BT_LE_EXT_ADV_START_PARAM_INIT(0, 2); // number of events is set to 2.
static uint8_t mfg_data[8] = {0};                                                     // Define a static array to store manufacturer data with a size of 8 bytes
static const struct bt_data ad[] = {                                                  
      BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 8),
  };



//function for creating a random static address and setting it as the identity address of the device.
static void set_random_static_address(void)                     // Function to set a random static address
{
  
  printk("Starting iBeacon Demo\n");                           // Print a message indicating the start of the iBeacon demo

  bt_addr_le_t addr;                                            // Declare a Bluetooth Low Energy address structure

  // convert the string into a binary address and stores this address in a buffer whose address is addr.
  err = bt_addr_le_from_str("FE:BB:EF:AF:BA:13", "random", &addr); // Only random static address can be given when the type is set to "random"
  if (err)                                                          // Check if there was an error in setting the Bluetooth address
  {
    printk("Invalid BT address (err %d)\n", err);                    // Print an error message indicating the failure to set the Bluetooth address
  }

  // create a new Identity address. This must be done before enabling Bluetooth. Addr is the address used for the new identity
  err = bt_id_create(&addr, NULL);                                    // Attempt to create a new Bluetooth identity using the provided address
  if (err < 0)                                                             // Check if there was an error in creating the new Bluetooth identity
  {
    printk("Creating new ID failed (err %d)\n", err);                        // Print an error message indicating the failure to create a new identity
  }
  printk("Created new address\n");                                          // Print a message indicating the successful creation of a new address
}



// function to create the extended advertising set using the advertising parameters.
void adv_param_init(void)                                         // Function to initialize advertising parameters
{
  int err;                                                        
 
  err = bt_le_ext_adv_create(&adv_param, NULL, &adv);           // Attempt to create an extended advertising set

  if (err)                                                   // Check if there was an error in creating the advertising set
  {
    printk("Failed to create advertising set (err %d)\n", err);       // Print an error message indicating the failure to create the advertising set
    return;                                                           
  }
  printk("Created extended advertising set \n");                       // Print a message indicating the successful creation of the extended advertising set
}

void co2_sensor_init(void){

	int ret;    // Declare a variable to store return codes
	dev = DEVICE_DT_GET_ANY(winsen_mhz19b); //This is a macro used to retreive a reference to a device node from the device tree.

	if (!device_is_ready(dev)) { //Verify that a device is ready for use. 
		printk("sensor: device not found.\n"); // Print an error message indicating that the sensor device was not found
		return;  // Return 0 to indicate failure
	}

	val.val1 = 5000; // Set the value of val1 to 5000
	ret = sensor_attr_set(dev, SENSOR_CHAN_CO2, SENSOR_ATTR_FULL_SCALE, &val); //This is likely a function provided by sensor related library or fremework.
	                                                                           //It's being called with four arguments.
	if (ret != 0) {       // Check if the return code is not equal to 0
		printk("failed to set range to %d\n", val.val1);  // Print an error message indicating the failure to set the range
		return;   // Return 0 to indicate failure
	}

	val.val1 = 1;   // Set the value of val1 to 1
	ret = sensor_attr_set(dev, SENSOR_CHAN_CO2, SENSOR_ATTR_MHZ19B_ABC, &val);  //dev – Pointer to the sensor device
                                                                                //chan – The channel the attribute belongs to, if any. Some attributes may only be set for all channels of a device, depending on device capabilities.
                                                                                //attr – The attribute to set
                                                                                //val – The value to set the attribute to


	if (ret != 0) {         // Check if the return code is not equal to 0
		printk("failed to set ABC to %d\n", val.val1);   // Print an error message indicating the failure to set ABC
		return;  // Return 0 to indicate failure
	}

	printk("\tOK\n");    // Print a message indicating success

	printk("Reading configurations from sensor:\n");   // Print a message indicating that configurations are being read from the sensor

	ret = sensor_attr_get(dev, SENSOR_CHAN_CO2, SENSOR_ATTR_FULL_SCALE, &val); //Get an attribute for a sensor
   

	if (ret != 0) {    // Check if the return code is not equal to 0
		printk("failed to get range\n");   // Print an error message indicating the failure to get the range
		return;    // Return 0 to indicate failure
	}

	printk("Sensor range is set to %dppm\n", val.val1);  // Print the sensor range

	ret = sensor_attr_get(dev, SENSOR_CHAN_CO2, SENSOR_ATTR_MHZ19B_ABC, &val);   // Get the attribute SENSOR_ATTR_MHZ19B_ABC for SENSOR_CHAN_CO2 from the sensor device
	if (ret != 0) {    // Check if the return code is not equal to 0
		printk("failed to get ABC\n");  // Print an error message indicating the failure to get the ABC attribute
		return;   // Return 0 to indicate failure
	}

}
// function to start advertising
void start_adv(void);                                               // Function to start advertising

// 
void fetch_temp_data(void)                                            // Function to fetch temperature data
{

 // Set manufacturer data for device identification
  
  mfg_data[0] = 0x1A;                                       // Data type: Complete list of 16-bit UUIDs available        
  
                                                            //currently looping only once since i=1
    struct sensor_value temp, hum;                           // Declare variables to store temperature and humidity values
   
   err = sensor_sample_fetch(sht);                          // Fetch sensor samples from the SHT device
    
    if (err == 0)                                           // Check if the sampling operation was successful
    {
      sensor_channel_get(sht, SENSOR_CHAN_AMBIENT_TEMP, &temp); // fetch the temperature reading and store it in temp

      sensor_channel_get(sht, SENSOR_CHAN_HUMIDITY, &hum);           // Get humidity data from the sensor
      printf("Temperature= %f; Humidity= %f\n", sensor_value_to_double(&temp), sensor_value_to_double(&hum));       // Print temperature and humidity values
    }   
    else
    {
      printf("ERROR: Temperature data update failed: %d\n", err);                       // Print an error message indicating the failure to update temperature data
    }
    

    temp_value1 = temp.val1;          // this variable stores only the value before the decimal
    temp_value2 = temp.val2 / 10000; // this variable stores only the value after the decimal

    hum_value1 = hum.val1;          // this variable stores only the value before the decimal
    hum_value2 = hum.val2 / 10000; // this variable stores only the value after the decimal


  // put the logged sensor data into data buffers

    mfg_data[1] = temp_value1;                     // Store the first byte of temperature value
    mfg_data[2] = hum_value1;                      // Store the first byte of humidity value

    	if (sensor_sample_fetch(dev) != 0) {     // Check if the sensor sample fetch operation fails
			printk("sensor: sample fetch fail.\n");   // Print an error message indicating the failure
			return;   // Return 0 to indicate failure
	}

		if (sensor_channel_get(dev, SENSOR_CHAN_CO2, &val) != 0) {  // Check if the sensor channel get operation fails
			printk("sensor: channel get fail.\n");  // Print an error message indicating the failure
			return;   // Return 0 to indicate failure
		}
		printk("\tCo2 reading: %d(ppm)\n", val.val1);  // Print the CO2 reading obtained from the sensor
	co2_value1 = val.val1 / 100;          // this variable stores only the value before the decimal
	co2_value2 = val.val1 % 100;          // this variable stores only the value after the decimal

    mfg_data[3] = co2_value1;                            // Set the 6th byte of manufacturer data to 0
    mfg_data[4] = co2_value2;                            // Set the 7th byte of manufacturer data to 0
    mfg_data[5] = 0x00;
    mfg_data[6] = 0x00;
    mfg_data[7] = 0x00;
  start_adv();                                                         // Function to start advertising

}


void start_adv(void)            
{
  
  /* Start advertising */

  err = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);                       // Set advertising data for the extended advertising set
  if (err)                                                                              // Check if there was an error in setting the advertising data
  {
    printk("Failed (err %d)\n", err);                                                    // Print an error message indicating the failure to set the advertising data
    return;
  }
  printk("Start Extended Advertising...");                                                     // Print a message indicating the start of extended advertising
  err = bt_le_ext_adv_start(adv, &ext_adv_param); // BT_LE_EXT_ADV_START_DEFAULT);

  if (err)                                                                                   // Start extended advertising with the specified parameters
  {
    printk("Failed to start extended advertising "                                           // Check if there was an error in starting extended advertising
           "(err %d)\n",
           err);
    return;
  }
  printk("done.\n");                                                      // Print a message indicating the completion of extended advertising start

  // enter sleep mode after advertising
  k_msleep(50);                                           // Delay execution for 100 milliseconds
  gpio_pin_toggle_dt(&led);                               // Toggle the state of the LED
  bt_le_ext_adv_stop(adv);                               // Stop extended advertising
  printk("Stopped advertising..!!\n");                   // Print a message indicating the cessation of advertising
  k_msleep(4950);
  fetch_temp_data();                                     // Fetch temperature data

}


void main(void)                             // main function
{
  printk("Starting Temperature and Humidity Node\n");             // Print a message indicating the start of the Temperature and Humidity Node
  gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);               // Configure the GPIO pin for the LED
  /* Create a random static address and set it as the identity address of the device */
  set_random_static_address();
	co2_sensor_init();

  /* Initialize the Bluetooth Subsystem */
  err = bt_enable(NULL);                                               // Enable Bluetooth stack initialization
  if (err)                                                            // Check if there was an error in initializing the Bluetooth stack
  {
    printk("Bluetooth init failed (err %d)\n", err);                 // Print an error message indicating the failure to initialize Bluetooth
  }
  adv_param_init();                                                  // Initialize advertising parameters
  fetch_temp_data();                                                 // Fetch temperature data
}