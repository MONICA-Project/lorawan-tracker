#include <stdint.h>

#include "periph/uart.h"

#define GPS_LINE_BUFSIZE        (128U)
#define GPS_UART_BUFSIZE        (512U)


int gps_read(int32_t *lat, int32_t *lon, int32_t *alt,
             unsigned *sat, unsigned *fix);

void gps_start(uart_t dev);

void gps_stop(uart_t dev);

void gps_init(uart_t dev, uint32_t baud);
