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
#define ANT_STACK_SZ                   (256*4)
#define MATRIX_STACK_SZ                (256*4)
#define INTERRUPT_HNDLR_STACK_SZ       (256*2)
static TaskHandle_t                    _ANTHandle;
static void                            ANT_task(void * pArg);
static TaskHandle_t                    _matrixHandle;
static void                            matrix_task(void * pArg);
static TaskHandle_t                    _interruptHandlerHandle;
static void                            interrupt_handler_task(void *pArg);

// Message variable populated by the button ISRs
static message_id_t nextMessage = INVALID;

// Forward declarations
void setup_interrupts();
void start_tasks();

// TODO:
// https://platformio.org/lib/show/13/Adafruit%20GFX%20Library/headers
// https://platformio.org/lib/show/587/RGB%20matrix%20Panel

///////////////////////////////////////////////////////////////////////
//INTERRUPTS
///////////////////////////////////////////////////////////////////////
void reset_IRQHandler()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    nextMessage = RESET;
    vTaskNotifyGiveFromISR(_interruptHandlerHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void on_IRQHandler()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    nextMessage = ON;
    vTaskNotifyGiveFromISR(_interruptHandlerHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void off_IRQHandler()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    nextMessage = OFF;
    vTaskNotifyGiveFromISR(_interruptHandlerHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


///////////////////////////////////////////////////////////////////////
//FUNCTIONS
///////////////////////////////////////////////////////////////////////

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

    // Setup Interrupts
    setup_interrupts();

    // Create tasks
    start_tasks();

    /* Suspend Loop() to save power, since we didn't have any code there */
    /* Need to not suspend in DEBUG mode since the looptask periodically checks for serial events */
    #ifndef DEBUG
    suspendLoop();
    #endif

    SERIAL_PRINTLN("Startup Complete\n");
}

/**  @brief Function to setup interrupts
*/
void setup_interrupts()
{
    /* Setup buttons as inputs */
    pinMode(BUTTON_RESET, INPUT_PULLUP);
    pinMode(BUTTON_ON, INPUT_PULLUP);
    pinMode(BUTTON_OFF, INPUT_PULLUP);

    /* Attach interrupts to buttons */
    attachInterrupt(BUTTON_RESET, reset_IRQHandler, FALLING);
    attachInterrupt(BUTTON_ON, on_IRQHandler, FALLING);
    attachInterrupt(BUTTON_OFF, off_IRQHandler, FALLING);
}

/** @brief Function for starting tasks
*/
void start_tasks()
{
    xTaskCreate(ANT_task, "ANT", ANT_STACK_SZ, NULL, TASK_PRIO_LOW, &_ANTHandle);
    xTaskCreate(matrix_task, "Matrix", MATRIX_STACK_SZ, NULL, TASK_PRIO_LOW, &_matrixHandle);
    xTaskCreate(interrupt_handler_task, "Interrupt Handler", INTERRUPT_HNDLR_STACK_SZ, NULL, TASK_PRIO_LOW, &_interruptHandlerHandle);
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
    ETH_msg_t stNextETHPrice; /* Struct to copy out of stat tracker */

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
                SERIAL_PRINTLN("Received Reset\n");
                break;
            }
            case ON:
            {
                // TODO: If the matrix is off, turn it on
                SERIAL_PRINTLN("Received On\n");
                break;
            }
            case OFF:
            {
                // TODO: If the matrix is on, turn it off
                SERIAL_PRINTLN("Received Off\n");
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

/** @brief Task which handles receiving messages from button ISRs to put them into the message queue.
 *         Required because the ISRs cannot directly use the queue since there are critical sections inside
 *         the queue write method.
 */
void interrupt_handler_task(void * pArgs)
{
    (void)pArgs; /* Supress compiler complaints */
    message_t stNextMsg = {0}; /* Struct to copy into the message queue */

    while(true)
    {
        /* Block until we get notified from an ISR. The pdTRUE argument specifies
           that we will clear the notification to 0 when taking it, essentially treating
           it like a binary semaphore */
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
        {
            /* Check if we are about to push an invalid message */
            if(nextMessage == INVALID)
            {
                ASSERT();
            }

            /* Push the next event into the queue */
            stNextMsg.messageId = nextMessage;
            messages.write(&stNextMsg);

            /* Do this so we can be sure we aren't somehow pushing stale data
               into the queue. We protect with the assert above
            */
            nextMessage = INVALID;
        }
        else
        {
            // This should never happen if we are blocking indefinitely
            ASSERT();
        }
    }
}