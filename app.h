#ifndef APP_H_
#define APP_H_

#include <stdint.h>

void App_Init(void);
void App_Run(void);

void App_OnUartRxChar(char ch);
void App_OnModeButtonStep(int8_t step);
void App_OnSysTickTick(void);
void App_OnStopwatchTick(void);
void App_OnAdcSampleRaw(uint16_t raw_sample);

#endif

