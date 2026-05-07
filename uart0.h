#ifndef UART0_H_
#define UART0_H_

#include <stdbool.h>
#include <stdint.h>

void UART0_Init(void);
bool UART0_TryReadChar(char *out_char);
void UART0_SendChar(char ch);
void UART0_SendString(const char *text);
void UART0_SendUnsigned(uint32_t value);
void UART0_SendSigned(int32_t value);
void UART0_SendFixed(float value, uint8_t decimals);

#endif

