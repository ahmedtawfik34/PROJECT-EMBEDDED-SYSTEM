#ifndef GPTM_TIMERS_H_
#define GPTM_TIMERS_H_

void GPTM_Stopwatch_Init1Hz(void);
void GPTM_Stopwatch_Start(void);
void GPTM_Stopwatch_Stop(void);
void GPTM_Stopwatch_ClearInterrupt(void);

void GPTM_AdcTrigger_Init1Hz(void);
void GPTM_AdcTrigger_Start(void);
void GPTM_AdcTrigger_Stop(void);

#endif

