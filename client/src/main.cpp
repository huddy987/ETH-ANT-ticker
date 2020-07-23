#include <Arduino.h>            // Arduino functionality
#include "hardwareconfig.h"     // hardware pin defines
#include "commonmacros.h"
#include "ANTNode.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Protomatter.h>
#include <Adafruit_GFX.h>

// Matrix defines TODO: move out
uint8_t pucRGBPins[] = {PIN_R1, PIN_G1, PIN_B1, PIN_R2, PIN_G2, PIN_B2}; // List of RGB pins to pass to Protomatter
uint8_t ucAddrCount = 4; // 4 line addresses for 32 pixel-high matrix
uint8_t pucAddrList[] = {PIN_A, PIN_B, PIN_C, PIN_D}; // List of address pins to pass to Protomatter

// ANT broadcast buffer offset for data start TODO: move out
#define BCST_BUFFER_OFFSET 3
// Global values for last ETH price TODO: move out
float fNextEthPrice = 0;
float fLastEthPrice = 0;

// Global ANT object
ANTNode ANT;

// TODO: Global Matrix object
//Adafruit_Protomatter matrix(PIXEL_WIDTH, 4, 1, pucRGBPins, 4, pucAddrList, PIN_CLK, PIN_LAT, PIN_OE, false);

// TODO:
// https://platformio.org/lib/show/13/Adafruit%20GFX%20Library/headers
// https://platformio.org/lib/show/587/RGB%20matrix%20Panel

/**@brief Function to setup the board
 */
void setup()
{
    SERIAL_BEGIN(115200); // Start serial on USB CDC at 115200bps

    // Startup ANT
    if (ANT.start()) // nonzero return from nordic API indicates failure
    {
        Serial.println("ANT failed to start!");
        while(1); // ANT will be unuseable, don't continue
    }

    // TODO: Init the matrix

    SERIAL_PRINTLN("Startup Complete");
}


/**@Brief Convert the received ETH price from the buffer to a float
 * @param[in] pucBuffer ANT broadcast buffer
 * @param[in] ucBufferLen Length of the ANT broadcast buffer
 *
 * @return NRF_SUCCESS if successful, NRF error code if unsucessful
 */
float get_current_eth_price(uint8_t * pucBuffer, uint8_t ucBufferLen)
{
    // Initialized as the cents part
    float fNextEthPrice = (float)pucBuffer[BCST_BUFFER_OFFSET] / 100;
    uint8_t ucReadStartIdx = BCST_BUFFER_OFFSET + 1;
    // + 1 since we already extracted the cents part
    for(uint8_t i = ucReadStartIdx; i < ucBufferLen; i++)
    {
        fNextEthPrice += (pucBuffer[i] << ((i - (ucReadStartIdx))*8));
    }
    return fNextEthPrice;
}

/**@brief Function which loops continually
 */
void loop()
{
    // TODO: Delete this loop. Move ANT onto one thread, matrix onto another thread.
    // ANT thread to signal matrix thread when price update occurs

    uint8_t aucPayload[ANT_STANDARD_DATA_PAYLOAD_SIZE];
    // For now, just loop and check that the broadcast buffer is being updated and fetched
    if (ANT.get_bcst_buffer(aucPayload) == 0)
    {
        // TODO: Move the processing into a class so we can store some stats too.
        //       - Ignore bcst buffers that repeat the same data
        //       - Ignore 0 (empty) buffers
        // We got a good payload. For now just convert the received price to float and print
        fNextEthPrice = get_current_eth_price(aucPayload, ANT_STANDARD_DATA_PAYLOAD_SIZE);
        if(fNextEthPrice != fLastEthPrice)
        {
            SERIAL_PRINTLN(fNextEthPrice);
        }
        fLastEthPrice = fNextEthPrice;
    }
}
