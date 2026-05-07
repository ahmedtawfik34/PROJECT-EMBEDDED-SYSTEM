#include "adc_temp.h"
#include "platform.h"

void AdcTemp_InitTimerTriggered(void)
{
    SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R0;
    while ((SYSCTL_PRADC_R & SYSCTL_RCGCADC_R0) == 0U)
    {
    }

    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN3;
    ADC0_EMUX_R = (ADC0_EMUX_R & ~0xF000U) | ADC_EMUX_EM3_TIMER;
    ADC0_SSMUX3_R = 0U;
    ADC0_SSCTL3_R = ADC_SSCTL3_TS0 | ADC_SSCTL3_IE0 | ADC_SSCTL3_END0;
    ADC0_IM_R |= ADC_IM_MASK3;
    ADC0_ISC_R = ADC_ISC_IN3;
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN3;

    NVIC_EN0_R |= (1UL << 17); /* IRQ33 => bit17 in EN0 */
}

void AdcTemp_ClearInterrupt(void)
{
    ADC0_ISC_R = ADC_ISC_IN3;
}

uint16_t AdcTemp_ReadRawSample(void)
{
    return (uint16_t)(ADC0_SSFIFO3_R & 0x0FFFU);
}

int16_t AdcTemp_RawToCelsiusTenths(uint16_t raw_sample)
{
    /* ORIGINAL FORMULA: 10xTemp(C) = 1475 - (2475 * sample / 4096) */
    /* This is what gave you the 17 to 29 range */
    int32_t value = 1475 - (int32_t)(((uint32_t)2475U * (uint32_t)raw_sample + 2048U) / 4096U);
    return (int16_t)value;
}