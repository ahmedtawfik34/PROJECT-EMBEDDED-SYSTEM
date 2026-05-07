#include "alert.h"

#include <stdint.h>

#include "gpiof.h"

static volatile uint8_t g_pending_alert_sequences = 0U;

static void delay_half_second_approx(void)
{
    volatile uint32_t i;
    for (i = 0U; i < 2500000U; ++i)
    {
        __asm(" NOP");
    }
}

void Alert_RequestBlink3x1Hz(void)
{
    if (g_pending_alert_sequences < 10U)
    {
        ++g_pending_alert_sequences;
    }
}

void Alert_Process(void)
{
    uint8_t i;

    if (g_pending_alert_sequences == 0U)
    {
        return;
    }

    --g_pending_alert_sequences;

    for (i = 0U; i < 3U; ++i)
    {
        GPIOF_SetRedLed(true);
        delay_half_second_approx();
        GPIOF_SetRedLed(false);
        delay_half_second_approx();
    }
}

