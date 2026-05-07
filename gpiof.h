#ifndef GPIOF_H_
#define GPIOF_H_

#include <stdbool.h>
#include <stdint.h>

void GPIOF_Init(void);
void GPIOF_SetRedLed(bool on);
void GPIOF_ToggleRedLed(void);
uint32_t GPIOF_GetMaskedInterruptStatus(void);
void GPIOF_ClearInterruptStatus(uint32_t mask);

#endif

