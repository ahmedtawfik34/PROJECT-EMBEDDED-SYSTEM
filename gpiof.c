#include "gpiof.h"

#include "platform.h"

void GPIOF_Init(void)
{
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0U)
    {
    }

    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTF_CR_R |= 0x1FU;

    GPIO_PORTF_DIR_R &= ~0x11U; /* PF0 and PF4 input */
    GPIO_PORTF_DIR_R |= 0x02U;  /* PF1 output */

    GPIO_PORTF_DEN_R |= 0x13U;
    GPIO_PORTF_PUR_R |= 0x11U;
    GPIO_PORTF_AFSEL_R &= ~0x13U;
    GPIO_PORTF_AMSEL_R &= ~0x13U;

    GPIO_PORTF_IS_R &= ~0x11U;  /* edge sensitive */
    GPIO_PORTF_IBE_R &= ~0x11U; /* single edge */
    GPIO_PORTF_IEV_R &= ~0x11U; /* falling edge */
    GPIO_PORTF_ICR_R = 0x11U;
    GPIO_PORTF_IM_R |= 0x11U;

    NVIC_EN0_R |= (1UL << 30); /* IRQ46 => bit30 in EN0 */
}

void GPIOF_SetRedLed(bool on)
{
    if (on)
    {
        GPIO_PORTF_DATA_R |= 0x02U;
    }
    else
    {
        GPIO_PORTF_DATA_R &= ~0x02U;
    }
}

void GPIOF_ToggleRedLed(void)
{
    GPIO_PORTF_DATA_R ^= 0x02U;
}

uint32_t GPIOF_GetMaskedInterruptStatus(void)
{
    return GPIO_PORTF_MIS_R & 0x11U;
}

void GPIOF_ClearInterruptStatus(uint32_t mask)
{
    GPIO_PORTF_ICR_R = (mask & 0x11U);
}

