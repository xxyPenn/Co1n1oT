// Copyright (c) Konstantin Belyalov. All rights reserved.
// Licensed under the MIT license.

#include "veml6030.h"
#include "i2cDriver/I2cDriver.h"
#include "SerialConsole.h"
#include "math.h"
#include "WifiHandlerThread/WifiHandler.h"


// Raw to lux conversion
double lux;
double luxlasttime;
const float calibration_factor = 0.0288;

// VEML6030 I2C address
#define VEML6030_I2C_ADDRESS 0x48//0x48 //0x10
#define VEML6030_I2C_ADDRESS_SEC 0x10//0x48 //0x10
#define VEML6030_I2C_WRITE_ADDRESS 0x90//0x90 //0x20
#define VEML6030_I2C_WRITE_ADDRESS_SEC 0x20//0x90 //0x20
#define VEML6030_I2C_READ_ADDRESS 0x91//0x91 //0x21
#define VEML6030_I2C_READ_ADDRESS_SEC 0x21//0x91 //0x21
#define VEML6030_I2C_READ_COMMAND 0x04

uint8_t msgOutal[64];
I2C_Data AmbientLightData;
static xSemaphoreHandle als_data_sent;

int32_t veml6030_read_register_als_white(uint8_t reg, uint8_t *buffer) {
	uint8_t payload[1] = {reg};

	AmbientLightData.address = VEML6030_I2C_ADDRESS;
	AmbientLightData.msgOut = payload;
	AmbientLightData.lenOut = 1;
	AmbientLightData.msgIn = buffer;
	AmbientLightData.lenIn = 2;
	uint8_t error = 0;
	
	//xSemaphoreTake(als_data_sent, portMAX_DELAY);
	//vTaskSuspendAll();
	error = I2cReadDataWait(&AmbientLightData, 0, WAIT_I2C_LINE_MS);
	//xTaskResumeAll();
	//xSemaphoreGive(als_data_sent);
	
	if (error != ERROR_NONE) {
		LogMessage(LOG_ERROR_LVL, "Error reading from VEML6030: Status code %d", error);
	}
	//for(int idx = 0; idx < 20; idx++){
		//LogMessage(LOG_ERROR_LVL, "%2x", buffer[idx]);
	//}
	////*value = (payload[1] << 8) | payload[0]; need to transfer to somewhere else
	//uint8_t alsdata[20];
//
	//uint16_t value = (AmbientLightData.msgIn[1] << 8) | AmbientLightData.msgIn[0];
	//double luxresult = toLux(value);
	//char dec_string[6];
	//if (AmbientLightData.msgIn[1]>>4 == 0){
		//SerialConsoleWriteString("Coin passing!\r\n");
		//delay_ms(250);
	//}
	return error;
}

int32_t veml6030_read_register_als_white_cont(uint8_t reg, uint8_t *buffer) {
	// Initialize variables
	uint8_t error = 0;
	struct BalanceDataPacket balance;
	// I2C packets waiting to be sent to sensor
	uint8_t payload[1] = {reg};
	AmbientLightData.address = VEML6030_I2C_ADDRESS;
	AmbientLightData.msgOut = payload;
	AmbientLightData.lenOut = 1;
	AmbientLightData.msgIn = buffer;
	AmbientLightData.lenIn = 2;
	bool is_quarter = false;
	
	uint16_t balance_num = 0;
	balance.balance = balance_num;
	balance.increment = 0;
	while(1){
		// Print to debug
		//I2cReadDataWait(&AmbientLightData, 0, WAIT_I2C_LINE_MS);
		//LogMessage(LOG_ERROR_LVL, "6030 reading: %02x%02x \r\n", AmbientLightData.msgIn[1],AmbientLightData.msgIn[0]);
		//delay_ms(100);
		// Spin on reading the first sensor, if no coin passing then keep reading until a coin passes
		AmbientLightData.address = VEML6030_I2C_ADDRESS;
		// Check if there's any sensor read error
		error = I2cReadDataWait(&AmbientLightData, 0, WAIT_I2C_LINE_MS);
		if (error != ERROR_NONE) {
			LogMessage(LOG_ERROR_LVL, "Error reading from VEML6030: Status code %d", error);
		}
		// If a coin passes the first sensor, read the second sensor to check if the coin passes the second sensor
		if (AmbientLightData.msgIn[1] != 0xFF){
			AmbientLightData.address = VEML6030_I2C_ADDRESS_SEC;
			// read the second sensor several times, if any single time it passes, break and print result
			for(int idx = 0; idx < 5; idx++){
				I2cReadDataWait(&AmbientLightData, 0, WAIT_I2C_LINE_MS);
				delay_ms(100);
				// If the second coin passes, print quarter coin passing
				if (AmbientLightData.msgIn[1]>>4 == 0){
					is_quarter = true;
					break;
				}
				else{
					is_quarter = false;
				}
			}
			if (is_quarter) {
				balance_num += 25;
				is_quarter = false;
				balance.balance = balance_num;
				balance.increment = 25;
				int error = xQueueSend(xQueueBalanceBuffer, &balance, (TickType_t)10);
				SerialConsoleWriteString("Quarter coin passing!\r\n");
			} else {
				balance_num += 10;
				balance.balance = balance_num;
				balance.increment = 10;
				int error = xQueueSend(xQueueBalanceBuffer, &balance, (TickType_t)10);
				SerialConsoleWriteString("Dime coin passing!\r\n");
			}
		} else {
			//continue;
		}
		//uint8_t alsdata[20];
//
		//uint16_t value = (AmbientLightData.msgIn[1] << 8) | AmbientLightData.msgIn[0];
		//double luxresult = toLux(value);
		//char dec_string[6];
		//if (AmbientLightData.msgIn[1]>>4 == 0){
			//SerialConsoleWriteString("Coin passing!\r\n");
			//delay_ms(250);
		//}
	}
	//*value = (payload[1] << 8) | payload[0]; need to transfer to somewhere else
	return error;
}

/*
int32_t veml6030_read_register_als_white(uint8_t reg, uint8_t *buffer) {
	//uint8_t error = ERROR_NONE;
    
	uint8_t payload[3] = {VEML6030_I2C_WRITE_ADDRESS, VEML6030_I2C_READ_COMMAND, VEML6030_I2C_READ_ADDRESS};
//first write and then read
    //uint8_t error = veml6030_write_register(REG_ALS_CONF,(1 << 11) | (0x00 << 6) | (0 << 1));
    //if(ERROR_NONE != error)
	//{
	//	SerialConsoleWriteString("Error writing veml6030!/r/n");
	//}
		AmbientLightData.address = VEML6030_I2C_ADDRESS;
		AmbientLightData.msgOut = payload;
		AmbientLightData.lenOut = 3;
		AmbientLightData.msgIn = buffer;
		AmbientLightData.lenIn = sizeof(payload);
	uint8_t error = 0;
	error = I2cReadDataWait(&AmbientLightData, 0, WAIT_I2C_LINE_MS);
	if (error != ERROR_NONE) {
		LogMessage(LOG_ERROR_LVL, "Error reading from VEML6030: Status code %d", error);
	}
	//*value = (payload[1] << 8) | payload[0]; need to transfer to somewhere else
	return error;
}
*/

// Initialize VEML6030
int32_t veml6030_init() {
	LogMessage(LOG_INFO_LVL, "Initializing VEML6030...");
	als_data_sent = xSemaphoreCreateMutex();
	//vTaskDelay(2500);
	// Configure the ALS_CONF register with desired settings
    //Set gain x2, integration time 100 ms, power on
    uint16_t conf_value = (1 << 11) | (0x00 << 6) | (0 << 1) | (0<<16);
	int32_t status = veml6030_write_register(REG_ALS_CONF, conf_value, VEML6030_I2C_ADDRESS);
	if (status != ERROR_NONE) {
		LogMessage(LOG_ERROR_LVL, "VEML6030 sensor 1 initialization failed: Status code %d", status);
		return status;
	}
	status = veml6030_write_register(REG_ALS_CONF, conf_value, VEML6030_I2C_ADDRESS_SEC);
	if (status != ERROR_NONE) {
		LogMessage(LOG_ERROR_LVL, "VEML6030 sensor 2 initialization failed: Status code %d", status);
		return status;
	}
	LogMessage(LOG_INFO_LVL, "VEML6030 successfully initialized");
	return ERROR_NONE;
}
uint16_t VEML6030_SUM=0;
double toLux(uint16_t total_num)
{
	//uint16_t total_num;
	//uint8_t lowsb  = GG_LSB;
	//uint8_t	highsb = GG_MSB;
	uint8_t num3,num2,num1,num0;
	uint16_t sum;
	
	//total_num = (highsb<<8 | lowsb);
	num0 = total_num&0xF;
	num1 = (total_num>>4)&0xF;
	num2 = (total_num>>8)&0xF;
	num3 = (total_num>>12)&0xF;
	
	sum = num0*1+num1*16+num2*16*16+num3*16*16*16;
	
	VEML6030_SUM=sum;
	
	
	lux = sum*calibration_factor;
	
	return lux;
}


static int32_t veml6030_read_register(uint8_t reg, uint8_t *buffer) {
	uint8_t error = ERROR_NONE;
	uint8_t payload[2] = {reg, 0};

		AmbientLightData.address = VEML6030_I2C_ADDRESS;
		AmbientLightData.msgOut = payload;
		AmbientLightData.lenOut = 1;
		AmbientLightData.msgIn = buffer;
		AmbientLightData.lenIn = sizeof(payload);

	error = I2cReadDataWait(&AmbientLightData, 0, WAIT_I2C_LINE_MS);
	if (error != ERROR_NONE) {
		LogMessage(LOG_ERROR_LVL, "Error reading from VEML6030: Status code %d", error);
		return error;
	}
	//*value = (payload[1] << 8) | payload[0];
	return (payload[1] << 8) | payload[0];
}


static int32_t veml6030_write_register(uint8_t reg, uint16_t value, uint8_t addr) {
	uint8_t payload[3] = {reg, value & 0xFF, value >> 8};//0xff gives me lower bits and value >> 8gives me higher bit
	AmbientLightData.address = addr;
	AmbientLightData.msgOut = payload;
	AmbientLightData.lenOut = sizeof(payload);
	AmbientLightData.lenIn = 0;
	
	return I2cWriteDataWait(&AmbientLightData, WAIT_I2C_LINE_MS);
}