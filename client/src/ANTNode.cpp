#include <Arduino.h>
#include "hardwareconfig.h"
#include "commonmacros.h"
#include "ANTNode.h"
// Softdevice includes
#include "ant_interface.h"
#include "ant_parameters.h"
#include "nrf_sdm.h"
#include "nrf_error.h"
#include "nrfx_power.h"

/** @brief Function to be called when an application fault occurs
 */
static void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    // If debug, we loop forever. If in release, reset the device
    FATAL_ERROR("In app fault loop!");
}

///////////////////////////////////////////////////////////////////
// Public Methods
///////////////////////////////////////////////////////////////////

/** @brief Method for starting the antNode by enabling the softdevice and opening an ANT channel
 */
uint32_t ANTNode::start()
{
    uint32_t err_code = NRF_SUCCESS;

    // Must release NRF_POWER before enabling the SD
    nrfx_power_usbevt_disable();
    nrfx_power_usbevt_uninit();
    nrfx_power_uninit();

    // Enable the softdevice
    err_code = sd_softdevice_enable(&(this->clock_lf_cfg), app_error_fault_handler, ANT_LICENSE_KEY);
    ERROR_CODE_CHECK(err_code); // Check if softdevice init failed

    // Need to reenable USB now
    sd_power_usbdetected_enable(true);
    sd_power_usbpwrrdy_enable(true);
    sd_power_usbremoved_enable(true);

    // Init ANT channel
    err_code = channel_init(&(this->channel_config));
    ERROR_CODE_CHECK(err_code); // Check if ANT init failed

    err_code = sd_ant_channel_open(ucChannelNumber);
    ERROR_CODE_CHECK(err_code); // Check if failed to open ANT channel

    return err_code;
}

/** @brief Method for getting the current ANT stack message buffer (only checks bcst data on channel 0)
 *
 *  @param[in] aucPayload: Buffer to copy the ANT message to. It is assumed that aucPayload is of size ANT_STANDARD_DATA_PAYLOAD_SIZE.
 *
 *  @return NRF_SUCCESS if successful, NRF error code if unsucessful
 */
uint32_t ANTNode::get_bcst_buffer(uint8_t * aucPayload)
{
    uint32_t err_code = NRF_SUCCESS;
    ant_evt_t ant_evt;
    uint8_t * aucNextPayload = 0;
    uint8_t ucNextMesgID = 0;

    err_code = sd_ant_event_get(&ant_evt.channel, &ant_evt.event, ant_evt.message.aucMessage);
    ERROR_CODE_CHECK(err_code);

    aucNextPayload = ant_evt.message.stMessage.uFramedData.stFramedData.uMesgData.stMesgData.aucPayload;
    ucNextMesgID = ant_evt.message.stMessage.uFramedData.stFramedData.ucMesgID;

    /*Serial.print("Channel: ");
    Serial.println(ant_evt.channel);
    Serial.print("Event: ");
    Serial.println(ant_evt.event, HEX);
    Serial.print("MsgID: ");
    Serial.println(ucNextMesgID, HEX);*/

    // We only care about channel 0, RX broadcast
    if ((ant_evt.channel != this->ucChannelNumber) ||
        (ant_evt.event != EVENT_RX) ||
        (ucNextMesgID != MESG_BROADCAST_DATA_ID))
    {
        return NRF_ERROR_NOT_FOUND;
    }

    // This is a message we care about, copy the buffer
    memcpy(aucPayload, aucNextPayload, ANT_STANDARD_DATA_PAYLOAD_SIZE);

    return err_code;
}


///////////////////////////////////////////////////////////////////
// Private Methods
///////////////////////////////////////////////////////////////////

/** @brief Method for initializing an ANT channel
 */
uint32_t ANTNode::channel_init(ant_channel_config_t const * pstConfig)
{
    uint32_t err_code = NRF_SUCCESS;
    // Set Channel Number.
    err_code = sd_ant_channel_assign(pstConfig->channel_number,
                                     pstConfig->channel_type,
                                     pstConfig->network_number,
                                     pstConfig->ext_assign);
    ERROR_CODE_CHECK(err_code);

    // Set Channel ID.
    err_code = sd_ant_channel_id_set(pstConfig->channel_number,
                                     pstConfig->device_number,
                                     pstConfig->device_type,
                                     pstConfig->transmission_type);
    ERROR_CODE_CHECK(err_code);

    // Set Channel RF frequency.
    err_code = sd_ant_channel_radio_freq_set(pstConfig->channel_number, pstConfig->rf_freq);
    ERROR_CODE_CHECK(err_code);

    // Set Channel period.
    err_code = sd_ant_channel_period_set(pstConfig->channel_number, pstConfig->channel_period);
    ERROR_CODE_CHECK(err_code);

    return err_code;
}