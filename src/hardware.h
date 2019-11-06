#ifndef _HARDWARE_H
#define _HARDWARE_H

/**
 * @Name    GPS enable configuration
 * @{
 */
#define GPS_EN_PORT        GPIOB
#define GPS_EN_PIN_NUM     3
#define GPS_EN_PIN         GPIO_PIN(PORT_B, GPS_EN_PIN_NUM)
/** @} */

/**
 * @name    External IO mux control pin
 * @{
 */
#define EXT_IO_CTRL2_PORT           GPIOA
#define EXT_IO_CTRL2_PIN_NUM        12
#define EXT_IO_CTRL2_PIN            GPIO_PIN(PORT_A, EXT_IO_CTRL2_PIN_NUM)
/** @} */

#endif
