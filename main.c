#include "app.h"
#include "platform.h"

int main(void)
{
    /* 1. CRITICAL: Turn on the Floating-Point Unit (FPU) */
    /* This allows the CPU to do math without crashing */
    NVIC_CPAC_R |= 0x00F00000; 

    /* 2. Initialize your application hardware */
    App_Init();

    /* 3. Enable interrupts globally */
    __asm(" CPSIE i"); 

    /* 4. Main sleeping loop */
    while (1)
    {
        __asm(" WFI"); // Wait For Interrupt
        App_Run();
    }
}