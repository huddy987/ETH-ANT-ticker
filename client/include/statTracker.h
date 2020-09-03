#ifndef STAT_TRACKER_H
#define STAT_TRACKER_H

/* Enum for tracking how the price has changed since last time */
enum eth_price_change_t
{
    PRICE_INVALID = 0, /* So when we zero out the struct this is set */
    PRICE_UP,
    PRICE_NEUTRAL,
    PRICE_DOWN
};

/** @brief Message struct for ETH price updates
 */
typedef struct
{
    float fCurrentETHPrice;
    float fLastETHPrice;
    eth_price_change_t ePriceChange;
} ETH_msg_t;

/** @brief Class for tracking ETH stats
 */
class statTracker
{
    public:
        bool get_current_ETH_stats(ETH_msg_t * pstETHStatOut);
        bool write_current_ETH_price(float * pfETHPriceIn);
        void reset();
    private:
        /* Tolerance for the "neutral" case */
        const float fNeutralTolerance = 1.005;
        ETH_msg_t stETHStats = {0};
        eth_price_change_t get_price_change(float * pfETHPriceIn);
};

#endif // STAT_TRACKER_H