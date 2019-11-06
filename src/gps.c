#include <stdint.h>
#include <string.h>

#include "isrpipe.h"
#include "isrpipe/read_timeout.h"
#include "periph/gpio.h"
#include "periph/uart.h"

#include "minmea.h"

#include "config.h"
#include "gps.h"
#include "hardware.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"


#define MINMEA_COORD_UPSCALE    (10000000L)
#define UART_READLINE_TIMEOUT   (1000000UL)

static uint8_t _rx_buf_mem[GPS_UART_BUFSIZE];
isrpipe_t gps_uart_isrpipe = ISRPIPE_INIT(_rx_buf_mem);


/* Code parts are taken from https://github.com/RIOT-OS/RIOT/pull/7086 */
static ssize_t _readline(isrpipe_t *isrpipe, char *resp_buf, size_t len, uint32_t timeout)
{
    DEBUG("%s\n", __func__);
    ssize_t res = -1;
    char *resp_pos = resp_buf;

    tsrb_empty(&isrpipe->tsrb);

    memset(resp_buf, 0, len);

    while (len) {
        int read_res;
        if ((read_res = isrpipe_read_timeout(isrpipe, (uint8_t *)resp_pos, 1, timeout)) == 1) {
            if (*resp_pos == '\r') {
                continue;
            }
            if (*resp_pos == '\n') {
                *resp_pos = '\0';
                res = resp_pos - resp_buf;
                goto out;
            }

            resp_pos += read_res;
            len -= read_res;
        }
        else if (read_res == -ETIMEDOUT) {
            res = -ETIMEDOUT;
            break;
        }
    }

out:
    return res;
}

int_least32_t _minmea_tocoord32(int32_t value, int32_t scale)
{
    if (scale == 0)
        return 0;

    int_least32_t degrees = value / (scale * 100);
    int_least32_t minutes = value % (scale * 100);

    degrees *= MINMEA_COORD_UPSCALE;
    minutes *= (MINMEA_COORD_UPSCALE / scale);
    minutes /= 60;

    return (degrees + minutes);
}

int_least32_t _minmea_tometer(int32_t value, int32_t scale)
{
    if (scale == 0)
        return 0;
    return value / scale;
}

int _gps_parse_gga(const char *buf, int32_t *lat, int32_t *lon, int32_t *alt,
                   unsigned *sat, unsigned *fix)
{
    DEBUG("%s\n", __func__);
    struct minmea_sentence_gga frame;
    if(minmea_parse_gga(&frame, buf)) {
        *lat = _minmea_tocoord32(frame.latitude.value, frame.latitude.scale);
        DEBUG(". MINMEA INT32 LAT (%"PRIi32")\n", *lat);
        *lon = _minmea_tocoord32(frame.longitude.value, frame.longitude.scale);
        DEBUG(". MINMEA INT32 LON (%"PRIi32")\n", *lon);
        *alt = _minmea_tometer(frame.height.value, frame.height.scale);
        DEBUG(". MINMEA INT32 ALT (%"PRIi32")\n", *alt);
        *sat = (unsigned)frame.satellites_tracked;
        *fix = (unsigned)frame.fix_quality;
        return 0;
    }
    return (-1);
}

int gps_read(int32_t *lat, int32_t *lon, int32_t *alt,
             unsigned *sat, unsigned *fix)
{
    DEBUG("%s\n", __func__);

    char linebuf[GPS_LINE_BUFSIZE];

    do {
        ssize_t res = _readline(&gps_uart_isrpipe,
                                linebuf, sizeof(linebuf),
                                UART_READLINE_TIMEOUT);
        if (res > 6) {
            DEBUG("gps: %s\n", linebuf);
            if (_gps_parse_gga(linebuf, lat, lon, alt, sat, fix) == 0) {
                return 0;
            }
        }
    } while (1);
    /* unreachable */
    return -ETIMEDOUT;
}

void gps_start(uart_t dev)
{
    DEBUG("%s\n", __func__);
    LED0_ON;
    /* Enable UART */
    uart_poweron(dev);
    /* Enable GPS */
    gpio_clear(GPS_EN_PIN);
}

void gps_stop(uart_t dev)
{
    DEBUG("%s\n", __func__);
    /* Disable GPS */
    //gpio_set(GPS_EN_PIN);
    /* Disable UART */
    uart_poweroff(dev);
    LED0_OFF;
}

void gps_init(uart_t dev, uint32_t baud)
{
    DEBUG("%s\n", __func__);
    /* Initialize UART */
    uart_init(dev, baud, (uart_rx_cb_t) isrpipe_write_one, &gps_uart_isrpipe);
    /* GPS enable pin */
    gpio_init(GPS_EN_PIN, GPIO_OUT);
    /* External IO control 2 line */
    gpio_init(EXT_IO_CTRL2_PIN, GPIO_OUT);
    /* Select GPS enable pin on the external IO 2 mux */
    gpio_set(EXT_IO_CTRL2_PIN);
    /* Disable GPS */
    gpio_set(GPS_EN_PIN);
}
