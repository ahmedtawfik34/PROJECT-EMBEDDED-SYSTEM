#include <stdint.h>

#include "adc_temp.h"
#include "app.h"
#include "gpiof.h"
#include "gptm_timers.h"
#include "platform.h"
#include "uart0.h"

void UART0_Handler(void)
{
    if ((UART0_MIS_R & UART_MIS_RXMIS) != 0U)
    {
        char received;
        while (UART0_TryReadChar(&received))
        {
            App_OnUartRxChar(received);
        }

        UART0_ICR_R = UART_ICR_RXIC | UART_ICR_RTIC;
    }
}

void GPIOF_Handler(void)
{
    uint32_t status = GPIOF_GetMaskedInterruptStatus();
    GPIOF_ClearInterruptStatus(status);

    if ((status & 0x01U) != 0U)
    {
        App_OnModeButtonStep(+1);
    }

    if ((status & 0x10U) != 0U)
    {
        App_OnModeButtonStep(-1);
    }
}

void SysTick_Handler(void)
{
    App_OnSysTickTick();
}

void TIMER2A_Handler(void)
{
    GPTM_Stopwatch_ClearInterrupt();
    App_OnStopwatchTick();
}

void ADC0SS3_Handler(void)
{
    uint16_t raw_sample = AdcTemp_ReadRawSample();
    AdcTemp_ClearInterrupt();
    App_OnAdcSampleRaw(raw_sample);
}

