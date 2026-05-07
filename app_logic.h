#ifndef APP_LOGIC_H_
#define APP_LOGIC_H_

#include "app_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*AppLogicDispatchFn)(const AppCommand *cmd);

void AppLogic_Init(AppLogicDispatchFn dispatch_fn);
void AppLogic_SetMode(AppMode mode);
AppMode AppLogic_GetMode(void);
void AppLogic_OnUartChar(char ch);

#ifdef __cplusplus
}
#endif

#endif

