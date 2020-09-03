#include "msgQueue.h"
#include "commonmacros.h"

///////////////////////////////////////////////////////////////////
// Public Methods
///////////////////////////////////////////////////////////////////

/** @brief Read from the message queue.
 *
 *  @param[out] pstMsgOut: Pointer to message to copy the next message to.
 *
 *  @return true if data was found and read, false if not
 */
bool msgQueue::read(message_t * pstMsgOut)
{
    if (pstMsgOut == NULL)
    {
        ASSERT();
        return false;
    }

    if (this->stMsgQueue.ucReadIdx == this->stMsgQueue.ucWriteIdx)
    {
        /* Caught up to write index, nothing to read here! */
        return false;
    }

    if (xSemaphoreTake(this->xReadMutex, portMAX_DELAY) != pdTRUE)
    {
        /* Could not obtain the semaphore. This is not expected under normal circumstances */
        ASSERT();
        return false;
    }

    *pstMsgOut = this->stMsgQueue.stMessageBuf[this->stMsgQueue.ucReadIdx];
    this->stMsgQueue.ucReadIdx = (++(this->stMsgQueue.ucReadIdx) % MESSAGE_QUEUE_SIZE);

    xSemaphoreGive(this->xReadMutex);

    return true;
}

/** @brief Write to the message queue.
 *
 *  @param[in] pstMsgIn: Pointer to the next message to copy in
 *
 *  @return true if write was successful, false if not
 */
bool msgQueue::write(message_t * pstMsgIn)
{
    if (((this->stMsgQueue.ucWriteIdx + 1) % MESSAGE_QUEUE_SIZE) == this->stMsgQueue.ucReadIdx)
    {
        /* This is an overflow */
        ASSERT();
        return false;
    }

    if(xSemaphoreTake(this->xWriteMutex, portMAX_DELAY) != pdTRUE)
    {
        /* Could not obtain the semaphore. This is not expected under normal circumstances */
        ASSERT();
        return false;
    }

    memcpy(&(this->stMsgQueue.stMessageBuf[this->stMsgQueue.ucWriteIdx]), pstMsgIn, sizeof(message_buf_t));
    this->stMsgQueue.ucWriteIdx = (++(this->stMsgQueue.ucWriteIdx) % MESSAGE_QUEUE_SIZE);

    xSemaphoreGive(this->xWriteMutex);

    return true;
}

///////////////////////////////////////////////////////////////////
// Private Methods
///////////////////////////////////////////////////////////////////