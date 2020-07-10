#include <Arduino.h>            // Arduino functionality
// Softdevice includes
#include "ant_interface.h"
#include "ant_parameters.h"
#include "nrf_sdm.h"
#include "nrf_error.h"

#ifndef ANTNODE_H
#define ANTNODE_H

/**@brief ANT channel configuration structure. From ant_channel_config.h in the SDK
 */
typedef struct
{
    uint8_t  channel_number;        ///< Assigned channel number.
    uint8_t  channel_type;          ///< Channel type (see Assign Channel Parameters in ant_parameters.h: @ref ant_parameters).
    uint8_t  ext_assign;            ///< Extended assign (see Ext. Assign Channel Parameters in ant_parameters.h: @ref ant_parameters).
    uint8_t  rf_freq;               ///< Radio frequency offset from 2400 MHz (for example, 2466 MHz, rf_freq = 66).
    uint8_t  transmission_type;     ///< Transmission type.
    uint8_t  device_type;           ///< Device type.
    uint16_t device_number;         ///< Device number.
    uint16_t channel_period;        ///< The period in 32 kHz counts.
    uint8_t  network_number;        ///< Network number denoting the network key.
} ant_channel_config_t;

/**@brief ANT stack event. From nrf_sdh_ant.h in the SDK
 */
typedef struct
{
    ANT_MESSAGE message;    //!< ANT Message.
    uint8_t     channel;    //!< Channel number.
    uint8_t     event;      //!< Event code.
} ant_evt_t;

/**@brief Class for interfacing with the 7.0.1 S212 ANT softdevice
 */
class ANTNode
{
    public:
        uint32_t start();
        uint32_t get_bcst_buffer(uint8_t * aucPayload);
    private:
        // ANT Channel config
        uint8_t ucChannelNumber = 0;
        ant_channel_config_t channel_config =
        {
            .channel_number    = ucChannelNumber,
            .channel_type      = CHANNEL_TYPE_SLAVE,
            .ext_assign        = 0x00,
            .rf_freq           = 67,
            .transmission_type = 1,
            .device_type       = 1,
            .device_number     = 2020,
            .channel_period    = 8192, // 4 Hz
            .network_number    = 0,
        };

        // NRF52 clock config
        nrf_clock_lf_cfg_t const clock_lf_cfg =
        {
            .source       = NRF_CLOCK_LF_SRC_XTAL,
            .rc_ctiv      = 0, // Unused for this clock source
            .rc_temp_ctiv = 0, // Unused for this clock source
            .accuracy     = NRF_CLOCK_LF_ACCURACY_20_PPM
        };

        uint32_t channel_init(ant_channel_config_t const * pstConfig);
};

#endif // ANTNODE_H