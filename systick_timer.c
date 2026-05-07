#include "systick_timer.h"

#include "platform.h"

void SysTickTimer_Init1Hz(void)
{
    NVIC_ST_CTRL_R = 0U;
    NVIC_ST_RELOAD_R = APP_ONE_HZ_RELOAD_4MHZ;
    NVIC_ST_CURRENT_R = 0U;
}

void SysTickTimer_Start(void)
{
    NVIC_ST_CURRENT_R = 0U;
    NVIC_ST_CTRL_R = NVIC_ST_CTRL_INTEN | NVIC_ST_CTRL_ENABLE;
}

void SysTickTimer_Stop(void)
{
    NVIC_ST_CTRL_R = 0U;
}

