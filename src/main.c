#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "byteorder.h"
#include "periph/uart.h"
#include "xtimer.h"
#include "fmt.h"

#include "net/loramac.h"
#include "semtech_loramac.h"

#include "lorawan-keys.t1.h"
#include "hardware.h"
#include "config.h"
#include "gps.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"


semtech_loramac_t loramac;
static uint8_t buf[LORAWAN_BUF_SIZE];

void lorawan_setup(void)
{
    DEBUG(". %s\n", __func__);
    uint8_t devaddr[LORAMAC_DEVADDR_LEN];
    uint8_t netskey[LORAMAC_NWKSKEY_LEN];
    uint8_t appskey[LORAMAC_APPSKEY_LEN];
    /* Convert identifiers and application key */
    fmt_hex_bytes(devaddr, LORAWAN_DEVADDR);
    fmt_hex_bytes(netskey, LORAWAN_NETSKEY);
    fmt_hex_bytes(appskey, LORAWAN_APPSKEY);

    /* Initialize the loramac stack */
    semtech_loramac_init(&loramac);
    semtech_loramac_set_dr(&loramac, LORAWAN_DATARATE);
    semtech_loramac_set_devaddr(&loramac, devaddr);
    semtech_loramac_set_nwkskey(&loramac, netskey);
    semtech_loramac_set_appskey(&loramac, appskey);
    DEBUG(".. uplink counter %"PRIu32"\n", semtech_loramac_get_uplink_counter(&loramac));
    /* Try to join by Over The Air Activation */
    DEBUG(".. LoRaWAN join: ");
    //LED1_ON;
    int ret = semtech_loramac_join(&loramac, LORAMAC_JOIN_ABP);
    if (ret != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        printf("[FAIL] lorawan join failed with %d\n", ret);
    }
    DEBUG("[DONE]\n");
    /* set loramac params */
    semtech_loramac_set_tx_mode(&loramac, LORAMAC_TX_UNCNF);
    semtech_loramac_set_tx_port(&loramac, LORAWAN_TX_PORT);
}

int create_buf(int32_t lat, int32_t lon, int16_t alt, uint8_t sat,
               uint8_t *buf, size_t maxlen)
{
    DEBUG("%s\n", __func__);

    size_t len = sizeof(lat) + sizeof(lon) + sizeof(alt) + sizeof(sat);
    if (maxlen < len) {
        return (-1);
    }
    memset(buf, 0, maxlen);
    lat = htonl(lat);
    memcpy(buf, &lat, sizeof(lat));
    buf += sizeof(lat);
    lon = htonl(lon);
    memcpy(buf, &lon, sizeof(lon));
    buf += sizeof(lon);
    alt = htons(alt);
    memcpy(buf, &alt, sizeof(alt));
    buf += sizeof(alt);
    memcpy(buf, &sat, sizeof(sat));

    return len;
}

void lorawan_send(semtech_loramac_t *loramac, uint8_t *buf, uint8_t len)
{
    DEBUG(". %s\n", __func__);
    /* try to send data */
    DEBUG(".. send: ");
    unsigned ret = semtech_loramac_send(loramac, buf, len);
    if (ret != SEMTECH_LORAMAC_TX_DONE)  {
        DEBUG("[FAIL] Cannot send data, ret code: %d\n", ret);
    }
    else {
        DEBUG("[DONE]\n");
    }
}

int main(void)
{
    /* set LED0 on */
    LED0_ON;
    /* Enable the onboard Step Up regulator */
    EN3V3_ON;
    /* Initialize and enable gps */
    gps_init(GPS_UART_DEV, GPS_UART_BAUDRATE);
    /* Setup LoRa parameters and OTAA join */
    lorawan_setup();

    unsigned gps_quality = 0;
    bool gps_off = true;
    unsigned counter = 0;
    while (1) {
        if (gps_off) {
            gps_start(GPS_UART_DEV);
            gps_off = false;
        }

        int32_t lat  = 0;
        int32_t lon  = 0;
        int32_t alt  = 0;
        unsigned sat = 0;
        unsigned fix = 0;

        if(gps_read(&lat, &lon, &alt, &sat, &fix) == 0) {
            DEBUG(". got GPS data\n");
            gps_quality += fix;
            counter++;
            if (gps_quality > GPS_QUALITY_THRESHOLD) {
                /* got enough samples */
                DEBUG(".. send (%"PRIi32",%"PRIi32",%"PRIi32",%u)\n", lat, lon, alt, sat);
                gps_stop(GPS_UART_DEV);
                gps_quality = 0;
                counter = 0;
                gps_off = true;
                int len = create_buf(lat, lon, alt, sat, &buf[0], LORAWAN_BUF_SIZE);
                if (len > 0) {
                    lorawan_send(&loramac, buf, len);
                    semtech_loramac_save_config(&loramac);
                    DEBUG(". uplink counter %"PRIu32"\n", semtech_loramac_get_uplink_counter(&loramac));
                    xtimer_sleep(APP_SLEEP_TIME_S);
                }
            }
            else if (counter > GPS_COUNTER_THRESHOLD) {
                /* no GPS data for some time, go to sleep and try again */
                DEBUG(".. retry after sleep\n");
                gps_stop(GPS_UART_DEV);
                gps_quality = 0;
                counter = 0;
                gps_off = true;
                xtimer_sleep(APP_SLEEP_TIME_S);
            }
            else {
                /* not enough samples yet */
                DEBUG(".. wait for more\n");
            }
        }
    }

    return 0;
}
