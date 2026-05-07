#include "gptm_timers.h"

#include "platform.h"

void GPTM_Stopwatch_Init1Hz(void)
{
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R2;
    while ((SYSCTL_PRTIMER_R & SYSCTL_PRTIMER_R2) == 0U)
    {
    }

    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
    TIMER2_TAILR_R = APP_ONE_HZ_RELOAD_50MHZ;
    TIMER2_TAPR_R = 0U;
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;
    TIMER2_IMR_R |= TIMER_IMR_TATOIM;

    NVIC_EN0_R |= (1UL << 23); /* IRQ39 => bit23 in EN0 */
}

void GPTM_Stopwatch_Start(void)
{
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;
    TIMER2_CTL_R |= TIMER_CTL_TAEN;
}

void GPTM_Stopwatch_Stop(void)
{
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
}

void GPTM_Stopwatch_ClearInterrupt(void)
{
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;
}

void GPTM_AdcTrigger_Init1Hz(void)
{
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0;
    while ((SYSCTL_PRTIMER_R & SYSCTL_PRTIMER_R0) == 0U)
    {
    }

    TIMER0_CTL_R &= ~TIMER_CTL_TAEN;
    TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;
    TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
    TIMER0_TAILR_R = APP_ONE_HZ_RELOAD_50MHZ;
    TIMER0_TAPR_R = 0U;
    TIMER0_CTL_R |= TIMER_CTL_TAOTE; /* trigger output for ADC */
}

void GPTM_AdcTrigger_Start(void)
{
    TIMER0_CTL_R |= TIMER_CTL_TAEN;
}

void GPTM_AdcTrigger_Stop(void)
{
    TIMER0_CTL_R &= ~TIMER_CTL_TAEN;
}

