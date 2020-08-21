#include "msgProcessor.h"

///////////////////////////////////////////////////////////////////
// Public Methods
///////////////////////////////////////////////////////////////////

/** @brief Read from the message queue. Should be called from matrix
 *         thread.
 *
 *  @param[out] pfMsgOut: Pointer to float to copy the next message to.
 *
 *  @return true if data was found and read, false if not
 */
bool msgProcessor::read(ETH_msg_t * pfMsgOut)
{
    if (this->stMsgQueue.ucReadIdx == this->stMsgQueue.ucWriteIdx)
    {
        // Caught up to write index, nothing to read here!
        return false;
    }

    *pfMsgOut = this->stMsgQueue.afMsgBuf[this->stMsgQueue.ucReadIdx];
    this->stMsgQueue.ucReadIdx = (++(this->stMsgQueue.ucReadIdx) % MESSAGE_QUEUE_SIZE);
    return true;
}

/** @brief Write to the message queue. Should be called from the ANT thread
 *
 *  @param[in] pucBuffer ANT broadcast buffer
 *  @param[in] ucBufferLen Length of the ANT broadcast buffer
 *
 *  @return true if write was successful, false if not
 */
bool msgProcessor::write(uint8_t * pucBuffer, uint8_t ucBufferLen)
{
    if (((this->stMsgQueue.ucWriteIdx + 1) % MESSAGE_QUEUE_SIZE) == this->stMsgQueue.ucReadIdx)
    {
        // This is an overflow
        return false;
    }

    ETH_msg_t stNextETHMsg = {0};

    if(!process_ETH_price(pucBuffer, ucBufferLen, &stNextETHMsg))
    {
        // Return true but no new data is written to the queue
        return true;
    }

    this->stMsgQueue.afMsgBuf[this->stMsgQueue.ucWriteIdx] = stNextETHMsg;
    this->stMsgQueue.ucWriteIdx = (++(this->stMsgQueue.ucWriteIdx) % MESSAGE_QUEUE_SIZE);
    return true;
}

///////////////////////////////////////////////////////////////////
// Private Methods
///////////////////////////////////////////////////////////////////

/** @brief Process ANT buffer to extract the ETH message
 *
 *  @param[in] pucBuffer ANT broadcast buffer
 *  @param[in] ucBufferLen Length of the ANT broadcast buffer
 *  @param[out] stNextMsg Next ETH message
 *
 *  @return True if new data is provided, False if the data is identical
 */
bool msgProcessor::process_ETH_price(uint8_t * pucBuffer, uint8_t ucBufferLen, ETH_msg_t * stNextMsg)
{
    // Initialized as the cents part
    float fCurrentETHPrice = (float)pucBuffer[ucBufStartOffset] / 100;
    // + 1 since we already extracted the cents part
    uint8_t ucReadStartIdx = ucBufStartOffset + 1;

    for(uint8_t i = ucReadStartIdx; i < ucBufferLen; i++)
    {
        fCurrentETHPrice += (pucBuffer[i] << ((i - (ucReadStartIdx))*8));
    }

    if (fCurrentETHPrice > this->fLastETHPrice)
    {
        stNextMsg->ePriceChange = UP;
    }
    // TODO: Make a small range around neutral for the enum return
    // We use the == case to return false (old data provided)
    else if (fCurrentETHPrice == this->fLastETHPrice)
    {
        //stNextMsg->ePriceChange = NEUTRAL;
        return false;
    }
    else
    {
        stNextMsg->ePriceChange = DOWN;
    }

    // Populate the struct
    stNextMsg->fLastETHPrice = this->fLastETHPrice;
    stNextMsg->fCurrentETHPrice = fCurrentETHPrice;

    // Update the class-tracked price
    this->fLastETHPrice = fCurrentETHPrice;

    return true;
}