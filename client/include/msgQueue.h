#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <Arduino.h>
#include "ant_parameters.h"
#include "FreeRTOS.h"
#include "commonmacros.h"

/* Number of messages we allow in the queue before overflow */
#define MESSAGE_QUEUE_SIZE 16

/* Enum for message types */
enum message_id_t
{
    PRICE,
    RESET,
    INVALID
};

/* Fix message size to ANT standard payload size since we are only using bcsts */
typedef char message_buf_t[ANT_STANDARD_DATA_PAYLOAD_SIZE];

/** @brief Generic message struct to be returned from the queue
 */
typedef struct
{
    message_buf_t messageBuffer;
    message_id_t messageId;
}message_t;

/** @brief Class for processing messages from ANT and dispatching
 *         them to the matrix.
 */
class msgQueue
{
    public:
        bool read(message_t * pstMsgOut);
        bool write(message_t * pstMsgIn);
        msgQueue()
        {
            /* Attempt to create the read and write mutex */
            xWriteSemaphore = xSemaphoreCreateMutex();
            xReadSemaphore = xSemaphoreCreateMutex();
            if ((xWriteSemaphore == NULL) || (xReadSemaphore == NULL))
            {
                /* ASSERT, not enough space on the heap!! */
                ASSERT();
            }
        }
    private:
        /** @brief Message queue struct
        */
        typedef struct
        {
            message_t stMessageBuf[MESSAGE_QUEUE_SIZE];
            uint8_t ucReadIdx;
            uint8_t ucWriteIdx;
        } msg_queue_t;

        msg_queue_t stMsgQueue = {0};

        /* Mutexes for read/write thread safety */
        SemaphoreHandle_t xWriteSemaphore;
        SemaphoreHandle_t xReadSemaphore;
};

#endif // MSG_QUEUE_H
