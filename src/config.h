#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

#include "net/loramac.h"
#include "xtimer.h"

/* LoRaWAN application configurations */
#define LORAWAN_BUF_SIZE                (64U)
#define LORAWAN_TX_PORT                 (1U)
#define LORAWAN_DATARATE                LORAMAC_DR_3
#define APP_LORAWAN_JOIN_RETRY_TIME     10

/* we use "Dynamic Sensor Payload for our data */
#define CAYENNE_LPP_CHANNEL_GPS         (1)
#define CAYENNE_LPP_CHANNEL_SAT         (2)

#define GPS_UART_BAUDRATE               (9600)
#define GPS_UART_DEV                    UART_DEV(1)
#define GPS_QUALITY_THRESHOLD           (4U)
#define GPS_COUNTER_THRESHOLD           (32U)

/* we must respect the duty cycle limitations */
#define APP_SLEEP_TIME_S                (19U)

#endif
