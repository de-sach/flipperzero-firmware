
#include <stdint.h>
#include "cc1101.h"
#include "furi.h"
#include "furi_hal_subghz.h"
#include "furi/core/log.h"

#include "utils/lp_filt.h"

#define M_TAG "subghz a"
#define TO_MHZ(x) (x * 1000u * 1000u)
#define MAX_RUN_COUNT (200u)
#define RSSI_TRESHOLD (90u)

static const uint32_t subghz_my_freq_list[] = {
    TO_MHZ(300),
    TO_MHZ(315),
    TO_MHZ(390),
    TO_MHZ(915),
};

int32_t subghz_analyzer_entry(void* p) {
    UNUSED(p);

    furi_hal_subghz_reset();
    LowpassFilt* filter = lowpass_init(8u);

    for(uint32_t index = 0u; index < MAX_RUN_COUNT; ++index) {
        furi_hal_spi_acquire(&furi_hal_spi_bus_handle_subghz);

        cc1101_switch_to_idle(&furi_hal_spi_bus_handle_subghz);
        uint32_t frequency =
            cc1101_set_frequency(&furi_hal_spi_bus_handle_subghz, subghz_my_freq_list[0]);
        cc1101_calibrate(&furi_hal_spi_bus_handle_subghz);
        furi_check(
            cc1101_wait_status_state(&furi_hal_spi_bus_handle_subghz, CC1101StateIDLE, 10000));
        cc1101_switch_to_rx(&furi_hal_spi_bus_handle_subghz);
        furi_hal_spi_release(&furi_hal_spi_bus_handle_subghz);
        float rssi = furi_hal_subghz_get_rssi();
        int32_t rssii = (int32_t)rssi;
        int32_t rssid = ((int32_t)(rssi * 1000)) + (rssii * 1000);
        FURI_LOG_I(M_TAG, "measured freq %lu: rssi %ld.%03ld", frequency, rssii, rssid);
        uint32_t rssi_unsigned = (uint32_t)(-1 * rssii);
        lowpassfilt_add(filter, rssi_unsigned);
        uint32_t averageRssi = lowpass_get(filter);
        FURI_LOG_I(M_TAG, "average rssi -%lu", averageRssi);
        if(averageRssi < RSSI_TRESHOLD) {
            /* frequency has been detected, try to analyze the signal */
            FURI_LOG_I(M_TAG, "average rssi above treshold -%lu", averageRssi);
            /* load a subghz recever to capture the data on this frequency for some time, 
               then if data is received, send it to all available parsers */
        }
    }
    lowpass_deinit(filter);
    filter = NULL;
    return 0;
}