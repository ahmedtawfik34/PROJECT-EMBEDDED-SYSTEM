#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdint.h>
#include <stdbool.h>

#include "tm4c123gh6pm_.h"
#include "common_macros.h"

#define APP_CPU_HZ               (50000000UL)
#define APP_UART_BAUD            (9600UL)
#define APP_UART_IBRD            (325UL)
#define APP_UART_FBRD            (33UL)
#define APP_SYSTICK_CLK_HZ       (4000000UL)  /* PIOSC/4 when CLK_SRC=0 */
#define APP_ONE_HZ_RELOAD_4MHZ   (4000000UL - 1UL)
#define APP_ONE_HZ_RELOAD_50MHZ  (50000000UL - 1UL)

#endif

