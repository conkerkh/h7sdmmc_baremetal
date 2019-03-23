/*
 * printf.c
 *
 *  Created on: Mar 23, 2019
 *      Author: khockuba
 */

#include "usart.h"

int putchar(int c) {
    HAL_UART_Transmit(&huart3, (uint8_t*)&c, 1, 1000);
    return c;
}

int _write (int fd, const void *buf, size_t count) {
    HAL_UART_Transmit(&huart3, (uint8_t*)buf, count, 1000);
    return count;
}
