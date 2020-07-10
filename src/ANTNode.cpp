#include <Arduino.h>            // Arduino functionality
#include "hardwareconfig.h"     // hardware pin defines
#include "antNode.h"
// Softdevice includes
#include "ant_interface.h"
#include "ant_parameters.h"
#include "nrf_sdm.h"
#include "nrf_error.h"

/**@Brief function to be called when an application fault occurs
 */
static void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    // If debug, we loop forever. If in release, reset the device
#ifdef DEBUG
    while(1)
    {
        Serial.println("In app fault loop!");
        digitalWrite(DEBUG_LED, LOW);
        delay(500);
        digitalWrite(DEBUG_LED, HIGH);
    }
#else
    sd_nvic_SystemReset();
#endif // DEBUG
}

///////////////////////////////////////////////////////////////////
// Public Methods
///////////////////////////////////////////////////////////////////

/**@brief Method for starting the antNode by enabling the softdevice and opening an ANT channel
 */
uint32_t ANTNode::start()
{
    uint32_t err_code = NRF_SUCCESS;

    // Enable the softdevice
    err_code = sd_softdevice_enable(&(this->clock_lf_cfg), app_error_fault_handler, ANT_LICENSE_KEY);
    if (err_code != NRF_SUCCESS)
    {
        return err_code; // Failed to init softdevice
    }

    // Init ANT channel
    err_code = channel_init(&(this->channel_config));
    if(err_code != NRF_SUCCESS)
    {
        return err_code; // Failed ANT channel init
    }

    err_code = sd_ant_channel_open(ucChannelNumber);
    if (err_code != NRF_SUCCESS)
    {
        return err_code; // Failed ANT channel open
    }

    return err_code;
}

/**@brief Method for getting the current ANT stack message buffer (only checks bcst data on channel 0)
 *
 * @param[in] aucPayload: Buffer to copy the ANT message to. It is assumed that aucPayload is of size ANT_STANDARD_DATA_PAYLOAD_SIZE.
 *
 * @return NRF_SUCCESS if successful, NRF error code if unsucessful
 */
uint32_t ANTNode::get_bcst_buffer(uint8_t * aucPayload)
{
    uint32_t err_code = NRF_SUCCESS;
    ant_evt_t ant_evt;
    uint8_t * aucNextPayload = ant_evt.message.stMessage.uFramedData.stFramedData.uMesgData.stMesgData.aucPayload;
    uint8_t ucNextMesgID = ant_evt.message.stMessage.uFramedData.stFramedData.ucMesgID;

    err_code = sd_ant_event_get(&ant_evt.channel, &ant_evt.event, ant_evt.message.aucMessage);
    if (err_code != NRF_SUCCESS)
    {
        /*Serial.print("Failed to get ant event: ");
        Serial.println(err_code);*/
        return err_code; // Failed to get ant event :(
    }

    /*Serial.print("Channel: ");
    Serial.println(ant_evt.channel);
    Serial.print("Event: ");
    Serial.println(ant_evt.event);
    Serial.print("MsgID: ");
    Serial.println(ucNextMesgID);*/

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

/**@brief Method for initializing an ANT channel
 */
uint32_t ANTNode::channel_init(ant_channel_config_t const * pstConfig)
{
    uint32_t err_code = NRF_SUCCESS;
    // Set Channel Number.
    err_code = sd_ant_channel_assign(pstConfig->channel_number,
                                     pstConfig->channel_type,
                                     pstConfig->network_number,
                                     pstConfig->ext_assign);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Set Channel ID.
    err_code = sd_ant_channel_id_set(pstConfig->channel_number,
                                     pstConfig->device_number,
                                     pstConfig->device_type,
                                     pstConfig->transmission_type);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Set Channel RF frequency.
    err_code = sd_ant_channel_radio_freq_set(pstConfig->channel_number, pstConfig->rf_freq);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Set Channel period.
    err_code = sd_ant_channel_period_set(pstConfig->channel_number, pstConfig->channel_period);

    return err_code;
}