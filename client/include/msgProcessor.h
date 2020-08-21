#ifndef MSG_PROCESSOR_H
#define MSG_PROCESSOR_H

#include <Arduino.h>

// Enum for tracking how the price has changed since last time
enum eth_price_change
{
    UP,
    NEUTRAL,
    DOWN,
};

/** @brief Message struct for ETH price updates
 */
typedef struct
{
    float fCurrentETHPrice;
    float fLastETHPrice;
    eth_price_change ePriceChange;
} ETH_msg_t;

// Number of messages we allow in the queue before overflow
#define MESSAGE_QUEUE_SIZE 16

/** @brief Class for processing messages from ANT and dispatching
 *         them to the matrix.
 */
class msgProcessor
{
    public:
        bool read(ETH_msg_t * pfMsgOut);
        bool write(uint8_t * pucBuffer, uint8_t ucBufferLen);
    private:
        /** @brief Message queue struct
        */
        typedef struct
        {
            ETH_msg_t afMsgBuf[MESSAGE_QUEUE_SIZE];
            uint8_t ucReadIdx;
            uint8_t ucWriteIdx;
        } msg_queue_t;

        msg_queue_t stMsgQueue = {0};

        // Need to keep track of last price in class as well as in the message queue
        float fLastETHPrice = 0;

        // ANT broadcast buffer offset for data start
        const uint8_t ucBufStartOffset = 3;

        bool process_ETH_price(uint8_t * pucBuffer, uint8_t ucBufferLen, ETH_msg_t * stNextMsg);
};

#endif // MSG_PROCESSOR_H
