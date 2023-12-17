/**************************************************************************/ /**
 * @file      CliThread.c
 * @brief     File for the CLI Thread handler. Uses FREERTOS + CLI
 * @author    Eduardo Garcia
 * @date      2020-02-15

 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "CliThread.h"
#include "VEML6030/veml6030.h"
#include "DistanceDriver/DistanceSensor.h"
#include "IMU\lsm6dso_reg.h"
#include "SeesawDriver/Seesaw.h"
#include "WifiHandlerThread/WifiHandler.h"
#include "Motor/SG90.h"
#include "SSD1306/SSD1306.h"
/******************************************************************************
 * Defines
 ******************************************************************************/
char fw_flag[] = "0:fw.txt"; //firmware downloaded flag
char fw_bin[] = "0:TestA_a10.bin"; //firmware downloaded flag
//uint8_t whitedata[20];
/******************************************************************************
 * Variables
 ******************************************************************************/
static const char pcWelcomeMessage[]  = "FreeRTOS CLI.\r\nType Help to view a list of registered commands.\r\n";

static const CLI_Command_Definition_t xImuGetCommand = {"imu", "imu: Returns a value from the IMU\r\n", (const pdCOMMAND_LINE_CALLBACK)CLI_GetImuData, 0};

static const CLI_Command_Definition_t xOTAUCommand = {"fw", "fw: Download a file and perform an FW update\r\n", (const pdCOMMAND_LINE_CALLBACK)CLI_OTAU, 0};

static const CLI_Command_Definition_t xResetCommand = {"reset", "reset: Resets the device\r\n", (const pdCOMMAND_LINE_CALLBACK)CLI_ResetDevice, 0};

static const CLI_Command_Definition_t xNeotrellisTurnLEDCommand = {"led",
                                                                   "led [keynum][R][G][B]: Sets the given LED to the given R,G,B values.\r\n",
                                                                   (const pdCOMMAND_LINE_CALLBACK)CLI_NeotrellisSetLed,
                                                                   4};

static const CLI_Command_Definition_t xNeotrellisProcessButtonCommand = {"getbutton",
                                                                         "getbutton: Processes and prints the FIFO button buffer from the seesaw.\r\n",
                                                                         (const pdCOMMAND_LINE_CALLBACK)CLI_NeotrellProcessButtonBuffer,
                                                                         0};

static const CLI_Command_Definition_t xDistanceSensorGetDistance = {"getdistance",
                                                                    "getdistance: Returns the distance from the US-100 Sensor.\r\n",
                                                                    (const pdCOMMAND_LINE_CALLBACK)CLI_DistanceSensorGetDistance,
                                                                    0};
static const CLI_Command_Definition_t xAlsGetCommand =
{
	"als",
	"als: Returns two values from the ambient light sensor \r\n",
	CLI_AlsReadData,
	0
};

static const CLI_Command_Definition_t xUnlockMotor = {"unlock", "unlock: Motor unlock\r\n", (const pdCOMMAND_LINE_CALLBACK)CLI_Unlock, 0};
static const CLI_Command_Definition_t xLockMotor = {"lock", "lock: Motor unlock\r\n", (const pdCOMMAND_LINE_CALLBACK)CLI_Lock, 0};
static const CLI_Command_Definition_t xOLED = {"OLED", "start-OLED\r\n", (const pdCOMMAND_LINE_CALLBACK)CLI_OLED, 0};
	
static const CLI_Command_Definition_t xSendDummyGameData = {"game", "game: Sends dummy game data\r\n", (const pdCOMMAND_LINE_CALLBACK)CLI_SendDummyGameData, 0};
static const CLI_Command_Definition_t xI2cScan = {"i2c", "i2c: Scans I2C bus\r\n", (const pdCOMMAND_LINE_CALLBACK)CLI_i2cScan, 0};	

FIL file_object; //FILE OBJECT used on main for the SD Card Test
	

// Clear screen command
const CLI_Command_Definition_t xClearScreen = {CLI_COMMAND_CLEAR_SCREEN, CLI_HELP_CLEAR_SCREEN, CLI_CALLBACK_CLEAR_SCREEN, CLI_PARAMS_CLEAR_SCREEN};

SemaphoreHandle_t cliCharReadySemaphore;  ///< Semaphore to indicate that a character has been received

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
 static void FreeRTOS_read(char *character);
/******************************************************************************
 * Callback Functions
 ******************************************************************************/

/******************************************************************************
 * CLI Thread
 ******************************************************************************/

void vCommandConsoleTask(void *pvParameters)
{
    // REGISTER COMMANDS HERE
    FreeRTOS_CLIRegisterCommand(&xOTAUCommand);
    FreeRTOS_CLIRegisterCommand(&xImuGetCommand);
    FreeRTOS_CLIRegisterCommand(&xClearScreen);
    FreeRTOS_CLIRegisterCommand(&xResetCommand);
    FreeRTOS_CLIRegisterCommand(&xNeotrellisTurnLEDCommand);
    FreeRTOS_CLIRegisterCommand(&xNeotrellisProcessButtonCommand);
    FreeRTOS_CLIRegisterCommand(&xDistanceSensorGetDistance);
    FreeRTOS_CLIRegisterCommand(&xSendDummyGameData);
	FreeRTOS_CLIRegisterCommand(&xI2cScan);
	FreeRTOS_CLIRegisterCommand(&xAlsGetCommand);
	FreeRTOS_CLIRegisterCommand(&xUnlockMotor);
	FreeRTOS_CLIRegisterCommand(&xLockMotor);
	FreeRTOS_CLIRegisterCommand(&xOLED);
	
    char cRxedChar[2];
    unsigned char cInputIndex = 0;
    BaseType_t xMoreDataToFollow;
    /* The input and output buffers are declared static to keep them off the stack. */
    static char pcOutputString[MAX_OUTPUT_LENGTH_CLI], pcInputString[MAX_INPUT_LENGTH_CLI];
    static char pcLastCommand[MAX_INPUT_LENGTH_CLI];
    static bool isEscapeCode = false;
    static char pcEscapeCodes[4];
    static uint8_t pcEscapeCodePos = 0;

    /* This code assumes the peripheral being used as the console has already
    been opened and configured, and is passed into the task as the task
    parameter.  Cast the task parameter to the correct type. */

    /* Send a welcome message to the user knows they are connected. */
    SerialConsoleWriteString((char *)pcWelcomeMessage);

    // Any semaphores/mutexes/etc you needed to be initialized, you can do them here
    cliCharReadySemaphore = xSemaphoreCreateBinary();
    if (cliCharReadySemaphore == NULL) {
        LogMessage(LOG_ERROR_LVL, "Could not allocate semaphore\r\n");
        vTaskSuspend(NULL);
    }

    for (;;) {
        FreeRTOS_read(&cRxedChar[0]);

        if (cRxedChar[0] == '\n' || cRxedChar[0] == '\r') {
            /* A newline character was received, so the input command string is
            complete and can be processed.  Transmit a line separator, just to
            make the output easier to read. */
            SerialConsoleWriteString((char *)"\r\n");
            // Copy for last command
            isEscapeCode = false;
            pcEscapeCodePos = 0;
            strncpy(pcLastCommand, pcInputString, MAX_INPUT_LENGTH_CLI - 1);
            pcLastCommand[MAX_INPUT_LENGTH_CLI - 1] = 0;  // Ensure null termination

            /* The command interpreter is called repeatedly until it returns
            pdFALSE.  See the "Implementing a command" documentation for an
            explanation of why this is. */
            do {
                /* Send the command string to the command interpreter.  Any
                output generated by the command interpreter will be placed in the
                pcOutputString buffer. */
                xMoreDataToFollow = FreeRTOS_CLIProcessCommand(pcInputString,        /* The command string.*/
                                                               pcOutputString,       /* The output buffer. */
                                                               MAX_OUTPUT_LENGTH_CLI /* The size of the output buffer. */
                );

                /* Write the output generated by the command interpreter to the
                console. */
                // Ensure it is null terminated
                pcOutputString[MAX_OUTPUT_LENGTH_CLI - 1] = 0;
                SerialConsoleWriteString(pcOutputString);

            } while (xMoreDataToFollow != pdFALSE);

            /* All the strings generated by the input command have been sent.
            Processing of the command is complete.  Clear the input string ready
            to receive the next command. */
            cInputIndex = 0;
            memset(pcInputString, 0x00, MAX_INPUT_LENGTH_CLI);
            memset(pcOutputString, 0, MAX_OUTPUT_LENGTH_CLI);
        } else {
            /* The if() clause performs the processing after a newline character
is received.  This else clause performs the processing if any other
character is received. */

            if (true == isEscapeCode) {
                if (pcEscapeCodePos < CLI_PC_ESCAPE_CODE_SIZE) {
                    pcEscapeCodes[pcEscapeCodePos++] = cRxedChar[0];
                } else {
                    isEscapeCode = false;
                    pcEscapeCodePos = 0;
                }

                if (pcEscapeCodePos >= CLI_PC_MIN_ESCAPE_CODE_SIZE) {
                    // UP ARROW SHOW LAST COMMAND
                    if (strcasecmp(pcEscapeCodes, "oa")) {
                        /// Delete current line and add prompt (">")
                        sprintf(pcInputString, "%c[2K\r>", 27);
                        SerialConsoleWriteString((char *)pcInputString);
                        /// Clear input buffer
                        cInputIndex = 0;
                        memset(pcInputString, 0x00, MAX_INPUT_LENGTH_CLI);
                        /// Send last command
                        strncpy(pcInputString, pcLastCommand, MAX_INPUT_LENGTH_CLI - 1);
                        cInputIndex = (strlen(pcInputString) < MAX_INPUT_LENGTH_CLI - 1) ? strlen(pcLastCommand) : MAX_INPUT_LENGTH_CLI - 1;
                        SerialConsoleWriteString(pcInputString);
                    }

                    isEscapeCode = false;
                    pcEscapeCodePos = 0;
                }
            }
            /* The if() clause performs the processing after a newline character
            is received.  This else clause performs the processing if any other
            character is received. */

            else if (cRxedChar[0] == '\r') {
                /* Ignore carriage returns. */
            } else if (cRxedChar[0] == ASCII_BACKSPACE || cRxedChar[0] == ASCII_DELETE) {
                char erase[4] = {0x08, 0x20, 0x08, 0x00};
                SerialConsoleWriteString(erase);
                /* Backspace was pressed.  Erase the last character in the input
                buffer - if there are any. */
                if (cInputIndex > 0) {
                    cInputIndex--;
                    pcInputString[cInputIndex] = 0;
                }
            }
            // ESC
            else if (cRxedChar[0] == ASCII_ESC) {
                isEscapeCode = true;  // Next characters will be code arguments
                pcEscapeCodePos = 0;
            } else {
                /* A character was entered.  It was not a new line, backspace
                or carriage return, so it is accepted as part of the input and
                placed into the input buffer.  When a n is entered the complete
                string will be passed to the command interpreter. */
                if (cInputIndex < MAX_INPUT_LENGTH_CLI) {
                    pcInputString[cInputIndex] = cRxedChar[0];
                    cInputIndex++;
                }

                // Order Echo
                cRxedChar[1] = 0;
                SerialConsoleWriteString(&cRxedChar[0]);
            }
        }
    }
}

/**
 * @fn			void FreeRTOS_read(char* character)
 * @brief		This function block the thread unless we received a character
 * @details		This function blocks until UartSemaphoreHandle is released to continue reading characters in CLI
 * @note
 */
static void FreeRTOS_read(char *character)
{
    // We check if there are more characters in the buffer that arrived since the last time
    // This function returns -1 if the buffer is empty, other value otherwise
    int ret = SerialConsoleReadCharacter((uint8_t *)character);

    while (ret == -1) {
        // there are no more characters - block the thread until we receive a semaphore indicating reception of at least 1 character
        xSemaphoreTake(cliCharReadySemaphore, portMAX_DELAY);

        // If we are here it means there are characters in the buffer - we re-read from the buffer to get the newly acquired character
        ret = SerialConsoleReadCharacter((uint8_t *)character);
    }
}

/**
 * @fn			void CliCharReadySemaphoreGiveFromISR(void)
 * @brief		Give cliCharReadySemaphore binary semaphore from an ISR
 * @details
 * @note
 */
void CliCharReadySemaphoreGiveFromISR(void)
{
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(cliCharReadySemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/******************************************************************************
 * CLI Functions - Define here
 ******************************************************************************/

// Example CLI Command. Reads from the IMU and returns data.
BaseType_t CLI_GetImuData(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    static int16_t data_raw_acceleration[3];
    static float acceleration_mg[3];
    uint8_t reg;
    stmdev_ctx_t *dev_ctx = GetImuStruct();

    /* Read output only if new xl value is available */
    lsm6dso_xl_flag_data_ready_get(dev_ctx, &reg);

    if (reg) {
        memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
        lsm6dso_acceleration_raw_get(dev_ctx, data_raw_acceleration);
        acceleration_mg[0] = lsm6dso_from_fs2_to_mg(data_raw_acceleration[0]);
        acceleration_mg[1] = lsm6dso_from_fs2_to_mg(data_raw_acceleration[1]);
        acceleration_mg[2] = lsm6dso_from_fs2_to_mg(data_raw_acceleration[2]);

        snprintf((char *)pcWriteBuffer, xWriteBufferLen, "Acceleration [mg]:X %d\tY %d\tZ %d\r\n", (int)acceleration_mg[0], (int)acceleration_mg[1], (int)acceleration_mg[2]);
    } else {
        snprintf((char *)pcWriteBuffer, xWriteBufferLen, "No data ready! \r\n");
    }
    return pdFALSE;
}

// THIS COMMAND USES vt100 TERMINAL COMMANDS TO CLEAR THE SCREEN ON A TERMINAL PROGRAM LIKE TERA TERM
// SEE http://www.csie.ntu.edu.tw/~r92094/c++/VT100.html for more info
// CLI SPECIFIC COMMANDS
static char bufCli[CLI_MSG_LEN];
BaseType_t xCliClearTerminalScreen(char *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    char clearScreen = ASCII_ESC;
    snprintf(bufCli, CLI_MSG_LEN - 1, "%c[2J", clearScreen);
    snprintf(pcWriteBuffer, xWriteBufferLen, bufCli);
    return pdFALSE;
}

/**
 BaseType_t CLI_OTAU(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
 * @brief	CLI command to download a firmware update file set in the server in to SD card.
 * @param[out] *pcWriteBuffer. Buffer we can use to write the CLI command response to! See other CLI examples on how we use this to write back!
 * @param[in] xWriteBufferLen. How much we can write into the buffer
 * @param[in] *pcCommandString. Buffer that contains the complete input. You will find the additional arguments, if needed. Please see
 https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_CLI_Implementing_A_Command.html#Example_Of_Using_FreeRTOS_CLIGetParameter
 Example 3
 * @return		Returns pdFALSE if the CLI command finished.
 * @note        Please see https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_CLI_Accessing_Command_Line_Parameters.html
                for more information on how to use the FreeRTOS CLI.
 */
// CLI Command. Downloads a firmware update file set in the server.
// added system_reset() in HTTP_DownloadFileTransaction
BaseType_t CLI_OTAU(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    WifiHandlerSetState(WIFI_DOWNLOAD_INIT);
	//vTaskStartScheduler();
	bool init_done = false;
	SerialConsoleWriteString("prior to create flag\r\n");
	// create initialization flag, indicating that the boot loader has been reached once
	FRESULT create_result = 0;
	create_result = f_open(&file_object, (char const *)fw_flag, FA_WRITE | FA_CREATE_ALWAYS);
	SerialConsoleWriteString("created fw flag\r\n");
	f_close(&file_object);
	SerialConsoleWriteString("closed file obj\r\n");
    return pdFALSE;
}

//Unlock motor
BaseType_t CLI_Unlock(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
	snprintf((char *) pcWriteBuffer, xWriteBufferLen, "UNLOCKED motor! \r\n");
	unlock_pwm();
	return pdFALSE;
}
//Lock motor
BaseType_t CLI_Lock(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
	snprintf((char *) pcWriteBuffer, xWriteBufferLen, "LOCKED motor! \r\n");
	lock_pwm();
	return pdFALSE;
}

BaseType_t CLI_OLED(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
	
	//i2c_begin();
	//i2c_clear();
	//i2c_setCursor(0, 0);  //line 1 x,y
	//char _char0[14] = {'W','e','l','c','o','m','e',' ','t','o',' ','t','h','e'};
	//for(uint8_t a = 0; a < 14; a++)
	//{
	//i2c_write_char(_char0[a]);
	//delay_ms(200);
	//}
	//i2c_setCursor(0, 1);  //line 2 x,y
	//char _char1[13] = {'C','o','1','n','1','o','T',' ','B','a','n','k','!'};
	//for(uint8_t a = 0; a < 13; a++)
	//{
	//i2c_write_char(_char1[a]);
	//delay_ms(200);
	//}
	//
	//i2c_setCursor(0, 4);  //x,y
	//char _char4[13] = {'Y','o','u','r',' ','B','a','l','a','n','c','e',':'};
	//for(uint8_t a = 0; a < 13; a++)
	//{
	//i2c_write_char(_char4[a]);
	//delay_ms(200);
	//}
	//
	//i2c_setCursor(0, 5);  //x,y
	//uint8_t Balance= 100; // change the money here
	//i2c_writeDigit(Balance);
	//i2c_setCursor(0, 6);  //x,y
	//i2c_print("USD");
	//vTaskDelay(pdMS_TO_TICKS(1000));

	snprintf((char *) pcWriteBuffer, xWriteBufferLen, "OLED light! \r\n");
	i2c_begin();
	i2c_clear();
	i2c_setCursor(0, 0);  //line 1 x,y
	char _char0[14] = {'W','e','l','c','o','m','e',' ','t','o',' ','t','h','e'};
	for(uint8_t a = 0; a < 14; a++)
	{
		i2c_write_char(_char0[a]);
		delay_ms(200);
	}
	i2c_setCursor(0, 1);  //line 2 x,y
	char _char1[13] = {'C','o','1','n','1','o','T',' ','B','a','n','k','!'};
	for(uint8_t a = 0; a < 13; a++)
	{
		i2c_write_char(_char1[a]);
		delay_ms(200);
	}
	i2c_setCursor(0, 2);  //x,y
	char _char4[13] = {'Y','o','u','r',' ','B','a','l','a','n','c','e',':'};
	for(uint8_t a = 0; a < 13; a++)
	{
		i2c_write_char(_char4[a]);
		delay_ms(200);
	}
	i2c_setCursor(0, 3);  //x,y
	//i2c test
	for(uint8_t a = 1; a < 10; a++)
	{
		i2c_writeDigit(a);
	}
	delay_ms(200);
	
	//uart test
	//	puts("-- PDC_UART Example --\r");
	

	i2c_setCursor(0, 3);  //x,y
	i2c_write_char('\n');
	
	//static const uint8_t key_down [] = {
	//0x00, 0x80, 0xE0, 0xF8, 0xFC, 0xFE, 0xFF, 0x3F, 0x1F, 0x0F, 0x0F, 0x1F, 0x3F, 0xFF, 0xFE, 0xFC,
	//0xF8, 0xF0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF,
	//0xFF, 0xFF, 0xFF, 0xFC, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	//0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	//0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00,
	//0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xF0, 0xFF,
	//0xFF, 0xFF, 0xFF, 0x1F, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	//0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0xFF, 0xFF, 0xFF,
	//0xFF, 0xFF, 0x07, 0x07, 0x07, 0x07, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
	//0x00, 0x00, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0x7E, 0xFC, 0xF8, 0xF8, 0xFC, 0x7E, 0x7F, 0x3F, 0x1F,
	//0x0F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x07,
	//0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0F, 0x0F, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x00
	//};
	//
	//i2c_draw(key_down,32,32,64,32);
	//
	////flag for avoiding constant writing to display
	//uint8_t cleared = 0;
	return pdFALSE;
}

// Example CLI Command. Resets system.
BaseType_t CLI_ResetDevice(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    system_reset();
    return pdFALSE;
}

/**
 BaseType_t CLI_NeotrellisSetLed( int8_t *pcWriteBuffer,size_t xWriteBufferLen,const int8_t *pcCommandString )
 * @brief	CLI command to turn on a given LED to a given R,G,B, value
 * @param[out] *pcWriteBuffer. Buffer we can use to write the CLI command response to! See other CLI examples on how we use this to write back!
 * @param[in] xWriteBufferLen. How much we can write into the buffer
 * @param[in] *pcCommandString. Buffer that contains the complete input. You will find the additional arguments, if needed. Please see
 https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_CLI_Implementing_A_Command.html#Example_Of_Using_FreeRTOS_CLIGetParameter
 Example 3

 * @return		Returns pdFALSE if the CLI command finished.
 * @note         Please see https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_CLI_Accessing_Command_Line_Parameters.html
                                 for more information on how to use the FreeRTOS CLI.

 */
BaseType_t CLI_NeotrellisSetLed(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    snprintf((char *) pcWriteBuffer, xWriteBufferLen, "Students to fill out!");
    // Check code SeesawSetLed and SeesawSetLed
    // How do you get parameters? Checl link in comments!
    // Check that the input is sanitized: Key between 0-15, RGB between 0-255. Print if there is an error!
    // return pdFalse to tell the FreeRTOS CLI your call is done and does not need to call again.
    // This function expects 4 arguments inside pcCommandString: key, R, G, B.
    return pdFALSE;
}

/**
 BaseType_t CLI_NeotrellProcessButtonBuffer( int8_t *pcWriteBuffer,size_t xWriteBufferLen,const int8_t *pcCommandString )
 * @brief	CLI command to process the Neotrellis FIFO button buffer. The Seesaw driver will store all button events until we read them.
 This function will read all the events in the buffer and print the action of each one. For example this is a print:
                 Key 10 pressed
                 Key 11 pressed
                 Key 11 released
                 Key 10 released
                 The function will print "Buffer Empty" if there is nothing on the button buffer.
 * @param[out] *pcWriteBuffer. Buffer we can use to write the CLI command response to! See other CLI examples on how we use this to write back!
 * @param[in] xWriteBufferLen. How much we can write into the buffer
 * @param[in] *pcCommandString. Buffer that contains the complete input. You will find the additional arguments, if needed. Please see
 https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_CLI_Implementing_A_Command.html#Example_Of_Using_FreeRTOS_CLIGetParameter
 Example 3

 * @return		Returns pdFALSE if the CLI command finished.
 * @note         Please see https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_CLI_Accessing_Command_Line_Parameters.html
                                 for more information on how to use the FreeRTOS CLI.

 */
BaseType_t CLI_NeotrellProcessButtonBuffer(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    // See functions SeesawGetKeypadCount and SeesawReadKeypad
    // Returns the number of key events currently on the Seesaw Keypad

    // Returns the number of requested events in the Seesaw FIFO buffer into the buffer variable

    //	NEO_TRELLIS_SEESAW_KEY(number) ;
    // snprintf(pcWriteBuffer,xWriteBufferLen, "count: %d\num_req_eve:%d\t\n",count,num_req_eve);
    // Print to pcWriteBuffer in order.
    // If the string is too long to print, print what you can.
    // The function you write will be useful in the future.
	// We choose PB02 GPIO0
	//set PB02 gpio pin to high
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction  = PORT_PIN_DIR_OUTPUT;
	//config_port_pin.input_pull  = PORT_PIN_PULL_UP;
	port_pin_set_config(PIN_PB02, &config_port_pin);
	port_pin_set_output_level(PIN_PB02, PB02_LOW);
	
    uint8_t buffer[64];
    uint8_t count = SeesawGetKeypadCount();
    if (count >= 1) {
        int32_t res = SeesawReadKeypad(buffer, 1);
        if (res == 0) {
            uint8_t pos, press;
            press = buffer[0] & 0x3;
            pos = buffer[0] >> 2;
            int num = NEO_TRELLIS_SEESAW_KEY(pos);
            if (press == 0x2) {
                snprintf((char *) pcWriteBuffer, xWriteBufferLen, "Button #%d is released\r\n", NEO_TRELLIS_SEESAW_KEY(num));
            } else if (press == 0x3) {
                snprintf((char *) pcWriteBuffer, xWriteBufferLen, "Button #%d is pressed\r\n", NEO_TRELLIS_SEESAW_KEY(num));
            }
        }
		port_pin_set_output_level(PIN_PB02, PB02_HIGH);
        return pdTRUE;
    } else {
        pcWriteBuffer = 0;
		port_pin_set_output_level(PIN_PB02, PB02_HIGH);
        return pdFALSE;
    }
}

/**
 BaseType_t CLI_DistanceSensorGetDistance( int8_t *pcWriteBuffer,size_t xWriteBufferLen,const int8_t *pcCommandString )
 * @brief	Returns distance in mm
 * @param[out] *pcWriteBuffer. Buffer we can use to write the CLI command response to! See other CLI examples on how we use this to write back!
 * @param[in] xWriteBufferLen. How much we can write into the buffer
 * @param[in] *pcCommandString. Buffer that contains the complete input. You will find the additional arguments, if needed. Please see
 https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_CLI_Implementing_A_Command.html#Example_Of_Using_FreeRTOS_CLIGetParameter
 Example 3

 * @return		Returns pdFALSE if the CLI command finished.
 * @note         Please see https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_CLI_Accessing_Command_Line_Parameters.html
                                 for more information on how to use the FreeRTOS CLI.

 */
BaseType_t CLI_DistanceSensorGetDistance(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    uint16_t distance = 0;
    int error = DistanceSensorGetDistance(&distance, 100);
    if (0 != error) {
        snprintf((char *) pcWriteBuffer, xWriteBufferLen, "Sensor Error %d!\r\n", error);
    } else {
        snprintf((char *) pcWriteBuffer, xWriteBufferLen, "Distance: %d mm\r\n", distance);
    }

    error = WifiAddDistanceDataToQueue(&distance);
    if (error == pdTRUE) {
        strcat((char *) pcWriteBuffer, "Distance Data MQTT Post\r\n");
    }
    return pdFALSE;
}

/**
 BaseType_t CLI_SendDummyGameData( int8_t *pcWriteBuffer,size_t xWriteBufferLen,const int8_t *pcCommandString )
 * @brief	Returns dummy game data
 * @param[out] *pcWriteBuffer. Buffer we can use to write the CLI command response to! See other CLI examples on how we use this to write back!
 * @param[in] xWriteBufferLen. How much we can write into the buffer
 * @param[in] *pcCommandString. Buffer that contains the complete input. You will find the additional arguments, if needed. Please see
 https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_CLI_Implementing_A_Command.html#Example_Of_Using_FreeRTOS_CLIGetParameter
 Example 3

 * @return		Returns pdFALSE if the CLI command finished.
 * @note         Please see https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_CLI_Accessing_Command_Line_Parameters.html
                                 for more information on how to use the FreeRTOS CLI.

 */
BaseType_t CLI_SendDummyGameData(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    struct GameDataPacket gamevar;

    gamevar.game[0] = 0;
    gamevar.game[1] = 1;
    gamevar.game[2] = 2;
    gamevar.game[3] = 3;
    gamevar.game[4] = 4;
    gamevar.game[5] = 5;
    gamevar.game[6] = 6;
    gamevar.game[7] = 7;
    gamevar.game[8] = 8;
    gamevar.game[9] = 9;
    gamevar.game[10] = 0xFF;

    int error = WifiAddGameDataToQueue(&gamevar);
    if (error == pdTRUE) {
        snprintf((char *) pcWriteBuffer, xWriteBufferLen, "Dummy Game Data MQTT Post\r\n");
    }
    return pdFALSE;
}


/**************************************************************************/ /**
 * @brief    Scan both i2c
 * @param    p_cli 
 * @param    argc 
 * @param    argv 
 ******************************************************************************/
BaseType_t CLI_i2cScan(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{

		I2C_Data i2cOled; 
        uint8_t address;
		//Send 0 command byte
		uint8_t dataOut[2] = {0,0};
		uint8_t dataIn[2];
		dataOut[0] = 0;
		dataOut[1] = 0;
		i2cOled.address = 0;
		i2cOled.msgIn = (uint8_t*) &dataIn[0];
		i2cOled.lenOut = 1;
		i2cOled.msgOut = (const uint8_t*) &dataOut[0];
		i2cOled.lenIn = 1;

            SerialConsoleWriteString("0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
            for (int i = 0; i < 128; i += 16)
            {
				snprintf(bufCli, CLI_MSG_LEN - 1, "%02x: ", i);
                SerialConsoleWriteString(bufCli);

                for (int j = 0; j < 16; j++)
                {

                    i2cOled.address = (i + j) << 1;

                    	
                    int32_t ret = I2cWriteDataWait(&i2cOled, 100);
                    if (ret == 0)
                    {
						snprintf(bufCli, CLI_MSG_LEN - 1, "%02x: ", i2cOled.address);
                        SerialConsoleWriteString(bufCli);
                    }
                    else
                    {
                        snprintf(bufCli, CLI_MSG_LEN - 1, "X ");
						SerialConsoleWriteString(bufCli);
                    }
                }
                SerialConsoleWriteString( "\r\n");
            }
            SerialConsoleWriteString( "\r\n");
			return pdFALSE;

}

BaseType_t CLI_AlsReadData(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString) {

   // Static variable to check if the sensor is already initialized
   static bool sensorInitialized = false;

    // Check if the sensor is already initialized
    if (!sensorInitialized) {
        int32_t initStatus = veml6030_init();
        if (initStatus != ERROR_NONE) {
            snprintf(pcWriteBuffer, xWriteBufferLen, "VEML6030 initialization failed with code %d\r\n", (int)initStatus);
            return pdFALSE;
        }
        sensorInitialized = true; // Set to true after successful initialization
    }

	//veml6030_read_register_als_white(0x05,whitedata);
	//uint8_t alsdata[20];
	uint8_t whitedata[20];
	if(ERROR_NONE != veml6030_read_register_als_white_cont(0x05,whitedata))
	{
		SerialConsoleWriteString("Error reading veml6030!/r/n");
	}

	uint16_t value = (whitedata[1] << 8) | whitedata[0];
	double luxresult = toLux(value);
	snprintf(pcWriteBuffer, xWriteBufferLen, "Reading sensor data: White status %02X%02X, ActualLuxValue:%u lux\r\n", whitedata[1],whitedata[0],luxresult);
    return pdFALSE; // Return pdFALSE to indicate that the command is complete
}