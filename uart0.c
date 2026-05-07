#include "uart0.h"

#include "platform.h"

static void uart_send_uint32_internal(uint32_t value)
{
    char buffer[11];
    uint8_t idx = 0;

    if (value == 0U)
    {
        UART0_SendChar('0');
        return;
    }

    while ((value > 0U) && (idx < (uint8_t)sizeof(buffer)))
    {
        buffer[idx++] = (char)('0' + (value % 10U));
        value /= 10U;
    }

    while (idx > 0U)
    {
        UART0_SendChar(buffer[--idx]);
    }
}

void UART0_Init(void)
{
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;

    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R0) == 0U)
    {
    }

    UART0_CTL_R &= ~UART_CTL_UARTEN;
    UART0_IBRD_R = APP_UART_IBRD;
    UART0_FBRD_R = APP_UART_FBRD;
    UART0_LCRH_R = UART_LCRH_WLEN_8;
    UART0_CC_R = 0x0U;

    GPIO_PORTA_AFSEL_R |= 0x03U;
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & ~0xFFU) | 0x11U;
    GPIO_PORTA_DEN_R |= 0x03U;
    GPIO_PORTA_AMSEL_R &= ~0x03U;

    UART0_IM_R = UART_IM_RXIM;
    NVIC_EN0_R |= (1UL << 5); /* IRQ21 => bit5 in EN0 */

    UART0_CTL_R = UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN;
}

bool UART0_TryReadChar(char *out_char)
{
    if ((UART0_FR_R & UART_FR_RXFE) != 0U)
    {
        return false;
    }

    *out_char = (char)(UART0_DR_R & 0xFFU);
    return true;
}

void UART0_SendChar(char ch)
{
    while ((UART0_FR_R & UART_FR_TXFF) != 0U)
    {
    }
    UART0_DR_R = (uint32_t)ch;
}

void UART0_SendString(const char *text)
{
    while ((text != 0) && (*text != '\0'))
    {
        UART0_SendChar(*text);
        ++text;
    }
}

void UART0_SendUnsigned(uint32_t value)
{
    uart_send_uint32_internal(value);
}

void UART0_SendSigned(int32_t value)
{
    uint32_t magnitude;

    if (value < 0)
    {
        UART0_SendChar('-');
        magnitude = (uint32_t)(-value);
    }
    else
    {
        magnitude = (uint32_t)value;
    }

    uart_send_uint32_internal(magnitude);
}

void UART0_SendFixed(float value, uint8_t decimals)
{
    uint8_t i;
    uint32_t whole;
    float fraction;

    if (value < 0.0f)
    {
        UART0_SendChar('-');
        value = -value;
    }

    whole = (uint32_t)value;
    uart_send_uint32_internal(whole);

    if (decimals == 0U)
    {
        return;
    }

    UART0_SendChar('.');
    fraction = value - (float)whole;

    for (i = 0U; i < decimals; ++i)
    {
        uint32_t digit;
        fraction *= 10.0f;
        digit = (uint32_t)fraction;
        UART0_SendChar((char)('0' + (digit % 10U)));
        fraction -= (float)digit;
    }
}

