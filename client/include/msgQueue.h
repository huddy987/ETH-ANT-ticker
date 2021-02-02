#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

/* Number of messages we allow in the queue before overflow */
#define MESSAGE_QUEUE_SIZE 16

/* Enum for message types */
enum message_id_t
{
    PRICE,
    RESET,
    ON,
    OFF,
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

#endif // MSG_QUEUE_H
