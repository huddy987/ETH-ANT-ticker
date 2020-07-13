#include <Arduino.h>            // Arduino functionality
#include "hardwareconfig.h"     // hardware pin defines
#include "commonmacros.h"
#include "ANTNode.h"

// Global ANT class
ANTNode ANT;

// TODO:
// https://platformio.org/lib/show/13/Adafruit%20GFX%20Library/headers
// https://platformio.org/lib/show/587/RGB%20matrix%20Panel

/**@brief Function to setup the board
 */
void setup()
{
    pinMode(DEBUG_LED, OUTPUT);
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

/**@brief Function which loops continually
 */
void loop()
{
#ifdef DEBUG
    digitalWrite(DEBUG_LED, LOW);
#endif // DEBUG
    uint8_t aucPayload[ANT_STANDARD_DATA_PAYLOAD_SIZE];
    // For now, just loop and check that the broadcast buffer is being updated and fetched
    if (ANT.get_bcst_buffer(aucPayload) == 0)
    {
        // We got a good payload. For now just sanity check the payload.
        uint32_t payload_sum = 0;
        for(int i = 0; i < ANT_STANDARD_DATA_PAYLOAD_SIZE; i++)
        {
            payload_sum += aucPayload[i];
        }
        Serial.print("Payload Received with value: ");
        Serial.println(payload_sum);
    }
}
