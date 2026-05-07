#ifndef APP_SHARED_H_
#define APP_SHARED_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    APP_MODE_CALCULATOR = 0,
    APP_MODE_TIMER = 1,
    APP_MODE_STOPWATCH = 2,
    APP_MODE_TEMPERATURE = 3
} AppMode;

typedef enum
{
    APP_CMD_NONE = 0,
    APP_CMD_MODE_SET,
    APP_CMD_MODE_STEP,
    APP_CMD_CALCULATE,
    APP_CMD_TIMER_SET,
    APP_CMD_TIMER_START,
    APP_CMD_TIMER_PAUSE,
    APP_CMD_TIMER_RESET,
    APP_CMD_STOPWATCH_START,
    APP_CMD_STOPWATCH_PAUSE,
    APP_CMD_STOPWATCH_RESET,
    APP_CMD_TEMP_THRESHOLD_SET,
    APP_CMD_HELP,
    APP_CMD_PARSE_ERROR
} AppCommandType;

typedef enum
{
    APP_PARSE_ERR_UNKNOWN = 1,
    APP_PARSE_ERR_FORMAT = 2,
    APP_PARSE_ERR_RANGE = 3
} AppParseError;

typedef struct
{
    AppCommandType type;
    union
    {
        struct
        {
            AppMode mode;
        } mode_set;
        struct
        {
            int8_t step;
        } mode_step;
        struct
        {
            float lhs;
            float rhs;
            char op;
        } calc;
        struct
        {
            uint16_t total_seconds;
        } timer_set;
        struct
        {
            float threshold_c;
        } temp_threshold;
        struct
        {
            AppParseError code;
        } parse_error;
    } data;
} AppCommand;

#ifdef __cplusplus
}
#endif

#endif

