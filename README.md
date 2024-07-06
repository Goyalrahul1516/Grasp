**Temperature, Humidity and CO2 monitor using thingspeak mqtt Part 1 - SHT40 and CO2 sensor data to thingspeak** 

**Codes:** 

**Node to BLE kit** -[ https://github.com/Goyalrahul1516/Grasp/tree/main/node_sht_co2 ](https://github.com/Goyalrahul1516/Grasp/tree/main/node_sht_co2)

**BLE kit to thingspeak mqtt** -[ https://github.com/Goyalrahul1516/Grasp/tree/main/central_node ](https://github.com/Goyalrahul1516/Grasp/tree/main/central_node)

**Modules** 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.001.jpeg)

Fig: BLE Node nRF52dk\_nRF52832 (SHT40 Embedded) 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.002.jpeg)

Fig: BLE Kit nRF52dk\_nRF52832 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.003.jpeg)

Fig: GSM Module QUECTEL EC200U-CN **Connections** 

1. **SHT40 and co2 to node** 

**Node   SHT40(inbuilt)** 

No Connections 

**Node   CO2** 

Rx  Tx 

Tx  Rx 

-----  Gnd (external 5v supply Gnd) -----  +5v (external supply +5v) 

2. **BLE kit to gsm module** 

**Kit  GSM** 

P0.08   Tx 

P0.06   Rx 

Gnd  Gnd 

----  Vdd (External power supply) 

**Process** 

1. For BLE Node 
   1. nothing to change in main.c (though you can change the advertising interval of your node) 
   1. All configurations are done in prj.conf file 
   1. Flash the code 
1. For BLE Kit 
1. In main.c change the credentials and channel ID to your thingspeak channel and mqtt device 
2. All configurations are done in prj.conf file 
2. Flash the code 

**Thingspeak mqtt publish setup in Central node Observer.c** 

**Fill credentials in observer.c** 

` `"AT+QMTCFG=\"recv/mode\",0,0,1\r\n",** 

` `"AT+QMTOPEN=0,\"mqtt3.thingspeak.com\",1883\r\n", 

` `"AT+QMTCONN=0,\"<ClientID>\",\"<Username>\",\"<password>\"\r\n" 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.004.png)

Fig: Change credentials in observer.c of Central node 

**Fill channel id in payload** 

**For Single field AT+QMTPUBEX=0,0,0,0,\"channels/<channelID>/publish/fields/<field\_no>\",2\r\ %d \r\n** Eg: “AT+QMTPUBEX=0,0,0,0,\"channels/2345234/publish/fields/field1\",2\r\ %d \r\n”, value 

**For multi fields** 

**AT+QMTPUBEX=0,0,0,0,\"channels/<channelID>/publish\",<payload\_length>\r\nfield1=%d&field2 =%d& status=MQTTPUBLISH\r\n** 

Eg: AT+QMTPUBEX=0,0,0,0,\"channels/2342344/publish\",68\r\nfield5=%d&field1=%d&field2=%d&field 3=%d&field4=%d&status=MQTTPUBLISH\r\n 

Here <payload\_length> is length of string “field1=%d&field2=%d& status=MQTTPUBLISH” and 68 is the length of “field5=%d&field1=%d&field2=%d&field3=%d&field4=%d&status=MQTTPUBLISH” 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.005.png)

Fig: Change channel ID in observer.c of central node 

**Thingspeak setup** 

1. **SIGNUP to thingspeak using university account if available** 
1. **Create a new channel** 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.006.jpeg)

3. **Go to devices > MQTT > Create a new device > add this channel to that device**  
3. **Give Publish and subscribe permissions** 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.007.png)

5. **After Adding the device , you will get the following credentials , download them.**  
5. **These will be used in the AT commands mentioned and similarly in the code** 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.008.png)

This is how your channel would look like 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.009.jpeg)

7. **Click on the private button** 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.010.jpeg)

**This channel ID will also be used while publishing to MQTT You can add widgets to your dashboard too** 

**Part 2 – Published data analysis** 

1. **Create a MATLAB analysis script to send back alert/message or any other reaction to the incoming data in fields of channel and save it** 

My analysis code 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.011.jpeg)

***Note: You need to learn about read and write Api keys of Thingspeak channels and how they work, in this code the api\_key used is a write one which writes data to field number 6 channel*** 

See this to know more :[ https://in.mathworks.com/help/thingspeak/matlab-analysis-app.html ](https://in.mathworks.com/help/thingspeak/matlab-analysis-app.html)

2. **Create an event identifier to check incoming data at regular intervals using React app** 
1. Open apps section and choose react** 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.012.jpeg)

2. Create new react**  

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.013.jpeg)

3. Enter all necessary details for the react app to activate and call the analysis code, then save the app** 

see this[ https://in.mathworks.com/help/thingspeak/react-app.html** ](https://in.mathworks.com/help/thingspeak/react-app.html)

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.014.jpeg)

**To see data you can**  

1. **use the visualizations** 

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.015.jpeg)

2. **Use python script to fetch data and save in csv file**  

**Code :[ https://github.com/Goyalrahul1516/Grasp/blob/main/mqtt.py** ](https://github.com/Goyalrahul1516/Grasp/blob/main/mqtt.py)**

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.016.jpeg)

3. **Use MATLAB**  

**Code :[ https://github.com/Goyalrahul1516/Grasp/blob/main/data_fetch.m** ](https://github.com/Goyalrahul1516/Grasp/blob/main/data_fetch.m)**

![](Aspose.Words.56cf22ad-9580-4d04-80e6-7a0935641acf.017.jpeg)
