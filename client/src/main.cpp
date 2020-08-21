#include <Arduino.h>            // Arduino functionality
#include "hardwareconfig.h"     // hardware pin defines
#include "commonmacros.h"
#include "ANTNode.h"
#include "msgProcessor.h"
#include "FreeRTOS.h"
#include "nrf_soc.h"
//#include <Wire.h>
//#include <SPI.h>
//#include <Adafruit_Protomatter.h>

// Matrix defines TODO: move out
uint8_t pucRGBPins[] = {PIN_R1, PIN_G1, PIN_B1, PIN_R2, PIN_G2, PIN_B2}; // List of RGB pins to pass to Protomatter
uint8_t ucAddrCount = 4; // 4 line addresses for 32 pixel-high matrix
uint8_t pucAddrList[] = {PIN_A, PIN_B, PIN_C, PIN_D}; // List of address pins to pass to Protomatter

// Global message processor object
msgProcessor messages;

// Global ANT object
ANTNode ANT;

// TODO: Global Matrix object
//Adafruit_Protomatter matrix(PIXEL_WIDTH, 4, 1, pucRGBPins, 4, pucAddrList, PIN_CLK, PIN_LAT, PIN_OE, false);

// Task defines
#define ANT_STACK_SZ          (256*4)
#define MATRIX_STACK_SZ       (256*4)
static TaskHandle_t           _ANTHandle;
static void                   ANT_task(void * pArg);
static TaskHandle_t           _matrixHandle;
static void                   matrix_task(void * pArg);

// TODO:
// https://platformio.org/lib/show/13/Adafruit%20GFX%20Library/headers
// https://platformio.org/lib/show/587/RGB%20matrix%20Panel

/** @brief Function to setup the board
 */
void setup()
{
    SERIAL_BEGIN(115200); // Start serial on USB CDC at 115200bps

    // Startup ANT
    if (ANT.start()) // nonzero return from nordic API indicates failure
    {
        SERIAL_PRINTLN("ANT failed to start!");
        while(1); // ANT will be unuseable, don't continue
    }

    // TODO: Init the matrix
    /*if (matrix.begin())
    {
        SERIAL_PRINTLN("Failed to start the Matrix!");
        while(1); // Matrix will be unuseable, don't continue
    }*/

    // Create a tasks for ANT and the matrix
    xTaskCreate(ANT_task, "ANT", ANT_STACK_SZ, NULL, TASK_PRIO_LOW, &_ANTHandle);
    xTaskCreate(matrix_task, "Matrix", MATRIX_STACK_SZ, NULL, TASK_PRIO_LOW, &_matrixHandle);

    // Suspend Loop() to save power, since we didn't have any code there
    suspendLoop();

    SERIAL_PRINTLN("Startup Complete");
}

void loop()
{
    // Suspended. CPU will not run loop() at all.
}

/** @brief Task which runs the ANT loop
 */
void ANT_task(void * pArgs)
{
    (void)pArgs; // Supress compiler complaints

    uint8_t aucPayload[ANT_STANDARD_DATA_PAYLOAD_SIZE];
    while(true)
    {
        if (ANT.get_bcst_buffer(aucPayload) == 0)
        {
            // Push the next ETH price to the message queue
            messages.write(aucPayload, ANT_STANDARD_DATA_PAYLOAD_SIZE);
        }
        else
        {
            // This does not block our other tasks, it just suspends this task
            // to prevent spamming the API. Having a lot of trouble with sd_app_evt_wait()
            // so use this instead.
            delay(100);
        }
    }
}

/** @brief Task which runs the matrix loop
 */
void matrix_task(void * pArgs)
{
    (void)pArgs; // Supress compiler complaints
    ETH_msg_t fETHMsg = {0};

    // Matrix doesn't work yet...just read the message queue and print to serial
    while(1)
    {
        // Wait until we get a message from the ANT thread
        while(!messages.read(&fETHMsg));

        // Print the message we got!
        SERIAL_PRINT("Last Price: ");
        SERIAL_PRINTLN(fETHMsg.fLastETHPrice);
        SERIAL_PRINT("Current Price: ");
        SERIAL_PRINTLN(fETHMsg.fCurrentETHPrice);
        SERIAL_PRINT("Price change: ");
        switch(fETHMsg.ePriceChange)
        {
            case UP:
            {
                SERIAL_PRINTLN("UP");
                break;
            }
            case NEUTRAL:
            {
                SERIAL_PRINTLN("NEUTRAL");
                break;
            }
            case DOWN:
            {
                SERIAL_PRINTLN("DOWN");
                break;
            }
        }

        //Serial.print("Refresh FPS = ~");
        //Serial.println(matrix.getFrameCount());
        //delay(1000);
    }
}
