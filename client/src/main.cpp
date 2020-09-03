#include <Arduino.h>            // Arduino functionality
#include "hardwareconfig.h"     // hardware pin defines
#include "commonmacros.h"
#include "ANTNode.h"
#include "msgQueue.h"
#include "statTracker.h"
#include "FreeRTOS.h"
#include "nrf_soc.h"
//#include <Wire.h>
//#include <SPI.h>
//#include <Adafruit_Protomatter.h>

// Matrix defines TODO: move out
//uint8_t pucRGBPins[] = {PIN_R1, PIN_G1, PIN_B1, PIN_R2, PIN_G2, PIN_B2}; // List of RGB pins to pass to Protomatter
//uint8_t ucAddrCount = 4; // 4 line addresses for 32 pixel-high matrix
//uint8_t pucAddrList[] = {PIN_A, PIN_B, PIN_C, PIN_D}; // List of address pins to pass to Protomatter

// Global ANT object
static ANTNode ANT;

// Global message queue object
static msgQueue messages;

// Global ETH price stat tracker
static statTracker stats;

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
        ASSERT();
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

    /* Suspend Loop() to save power, since we didn't have any code there */
    /* Need to not suspend in DEBUG mode since the looptask periodically checks for serial events */
    #ifndef DEBUG
    suspendLoop();
    #endif

    SERIAL_PRINTLN("Startup Complete\n");
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
    message_t stNextMsg = {0}; /* Struct to copy into the message queue */

    uint8_t aucPayload[ANT_STANDARD_DATA_PAYLOAD_SIZE];
    while(true)
    {
        if (ANT.get_bcst_buffer(aucPayload) == 0)
        {
            /* Prepare the message to push to the queue */
            memcpy(stNextMsg.messageBuffer, aucPayload, sizeof(message_buf_t));
            stNextMsg.messageId = PRICE;

            /* Push it to the queue */
            messages.write(&stNextMsg);
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
    (void)pArgs; /* Supress compiler complaints */
    message_t stNextMsg = {0}; /* Struct to copy out of the message queue */
    ETH_msg_t stNextETHPrice = {0}; /* Struct to copy out of stat tracker */

    // TODO: Matrix doesn't work yet...just read the message queue and print to serial
    while(1)
    {
        /* Wait until we get a message from the ANT thread */
        while(!messages.read(&stNextMsg));

        /* Process the received message */
        switch(stNextMsg.messageId)
        {
            case PRICE:
            {
                /* First convert from bytes to float for the ETH price...*/
                /* Initialized as the cents part */
                stNextETHPrice.fCurrentETHPrice = (float)stNextMsg.messageBuffer[ANT_MESSAGE_START_IDX] / 100;
                /* + 1 since we already extracted the cents part */
                uint8_t ucReadStartIdx = ANT_MESSAGE_START_IDX + 1;

                for(uint8_t i = ucReadStartIdx; i < ANT_STANDARD_DATA_PAYLOAD_SIZE; i++)
                {
                    stNextETHPrice.fCurrentETHPrice += (stNextMsg.messageBuffer[i] << ((i - (ucReadStartIdx))*8));
                }

                /* Next, pass to the stat tracker to update stats.
                   If FALSE is returned, continue since we are repeating the same
                   price and should not let this bubble up to the matrix display
                */
                if (!stats.write_current_ETH_price(&(stNextETHPrice.fCurrentETHPrice)))
                {
                    continue;
                }
                break;
            }
            case RESET:
            {
                /* Reset the ETH price stats */
                stats.reset();
                break;
            }
            case INVALID:
            {
                /* Should never need to handle the invalid messageID, fallthru to default */
            }
            default:
            {
                ASSERT();
                continue;
            }
        }

        /* At this point it is assumed we can read from the stat tracker with updated stats */
        stats.get_current_ETH_stats(&stNextETHPrice);

        // Print the message we got!
        SERIAL_PRINT("Last Price: ");
        SERIAL_PRINTLN(stNextETHPrice.fLastETHPrice);
        SERIAL_PRINT("Current Price: ");
        SERIAL_PRINTLN(stNextETHPrice.fCurrentETHPrice);
        SERIAL_PRINT("Price change: ");
        switch(stNextETHPrice.ePriceChange)
        {
            case PRICE_UP:
            {
                SERIAL_PRINTLN("UP");
                break;
            }
            case PRICE_NEUTRAL:
            {
                SERIAL_PRINTLN("NEUTRAL");
                break;
            }
            case PRICE_DOWN:
            {
                SERIAL_PRINTLN("DOWN");
                break;
            }
            case PRICE_INVALID:
            {
                /* Fallthru to default */
            }
            default:
            {
                ASSERT();
            }
        }

        //Serial.print("Refresh FPS = ~");
        //Serial.println(matrix.getFrameCount());
        //delay(1000);
    }
}
