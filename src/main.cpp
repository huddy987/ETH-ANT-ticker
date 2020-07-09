#include <Arduino.h>            // Arduino functionality
#include "hardwareconfig.h"     // hardware pin defines
#include "ant_interface.h"      // For ANT softdevice
#include "nrf_sdm.h"

// TODO:
// https://platformio.org/lib/show/13/Adafruit%20GFX%20Library/headers
// https://platformio.org/lib/show/587/RGB%20matrix%20Panel

// ANT Channel Defines
#define ANT_CHANNEL_TYPE            CHANNEL_TYPE_SLAVE
#define ANT_CHANNEL_NUM             0
#define ANT_CHANNEL_RF_FREQ         67
#define ANT_TRANSMISSION_TYPE       1
#define ANT_DEVICE_TYPE             1
#define ANT_DEVICE_NUMBER           2020
#define ANT_PERIOD                  8192 // 4 Hz
#define ANT_NETWORK_NUMBER          0

/**@Brief function to be called when an application fault occurs
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
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

/**@brief Function to setup the board
 */
void setup()
{
    Serial.begin(115200); // init serial at 115200bps
#ifdef DEBUG
    pinMode(DEBUG_LED, OUTPUT);
#endif // DEBUG

    nrf_clock_lf_cfg_t const clock_lf_cfg =
    {
        .source       = NRF_CLOCK_LF_SRC_XTAL,
        .rc_ctiv      = 0, // Unused for this clock source
        .rc_temp_ctiv = 0, // Unused for this clock source
        .accuracy     = NRF_CLOCK_LF_ACCURACY_20_PPM
    };

    sd_softdevice_enable(&clock_lf_cfg, app_error_fault_handler, ANT_LICENSE_KEY);
    Serial.println("Softdevice enabled");

    // Init the matrix
}

/**@brief Function which loops continually
 */
void loop()
{
#ifdef DEBUG
    digitalWrite(DEBUG_LED, LOW);
#endif // DEBUG
}
