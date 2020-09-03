#include "arduino.h"
#include "statTracker.h"
#include "commonmacros.h"

///////////////////////////////////////////////////////////////////
// Public Methods
///////////////////////////////////////////////////////////////////

/** @brief Get the current ETH price stats.
 *
 *  @param[out] pstETHStatOut: Pointer to message to copy the stats to
 *
 *  @return true if data was found and read, false if not
 */
bool statTracker::get_current_ETH_stats(ETH_msg_t * pstETHStatOut)
{
    if(pstETHStatOut == NULL)
    {
        ASSERT();
        return false;
    }

    *pstETHStatOut = this->stETHStats;

    return true;
}

/** @brief Write the current ETH price.
 *
 *  @param[in] pfETHPriceIn: Pointer to a float containinng the current ETH price
 *
 *  @return true if write was successful, false if not
 */
bool statTracker::write_current_ETH_price(float * pfETHPriceIn)
{
    if (pfETHPriceIn == NULL)
    {
        ASSERT();
        return false;
    }

    /* Update the price change */
    this->stETHStats.ePriceChange = this->get_price_change(pfETHPriceIn);

    /* Update the last price */
    this->stETHStats.fLastETHPrice = this->stETHStats.fCurrentETHPrice;

    /* Update the current price */
    this->stETHStats.fCurrentETHPrice = *pfETHPriceIn;

    return true;
}

/** @brief Reset the ETH stats
 *
 */
void statTracker::reset()
{
    this->stETHStats = {0};
}

///////////////////////////////////////////////////////////////////
// Private Methods
///////////////////////////////////////////////////////////////////

/** @brief Update the price change from the last price
 *
 *  @param[in] pfETHPriceIn: Pointer to a float containinng the current ETH price
 *
 *  @return true if write was successful, false if not
 */
eth_price_change_t statTracker::get_price_change(float * pfETHPriceIn)
{
    if ((*pfETHPriceIn <= (this->stETHStats.fLastETHPrice * this->fNeutralTolerance)) &&
        (*pfETHPriceIn >= (this->stETHStats.fLastETHPrice / this->fNeutralTolerance)))
    {
        return PRICE_NEUTRAL;
    }
    else if (*pfETHPriceIn > this->stETHStats.fLastETHPrice)
    {
        return PRICE_UP;
    }
    else
    {
        return PRICE_DOWN;
    }
}