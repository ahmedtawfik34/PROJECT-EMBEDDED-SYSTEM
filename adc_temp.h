#ifndef ADC_TEMP_H_
#define ADC_TEMP_H_

#include <stdint.h>

void AdcTemp_InitTimerTriggered(void);
void AdcTemp_ClearInterrupt(void);
uint16_t AdcTemp_ReadRawSample(void);
int16_t AdcTemp_RawToCelsiusTenths(uint16_t raw_sample);

#endif

