#include "app.h"

#include <stdbool.h>
#include <stdint.h>

#include "adc_temp.h"
#include "alert.h"
#include "app_logic.h"
#include "app_shared.h"
#include "gpiof.h"
#include "gptm_timers.h"
#include "systick_timer.h"
#include "uart0.h"

static volatile AppMode g_mode = APP_MODE_CALCULATOR;

static volatile bool g_timer_running = false;
static volatile uint16_t g_timer_initial_seconds = 0U;
static volatile uint16_t g_timer_remaining_seconds = 0U;
static volatile bool g_timer_tick_flag = false;
static volatile bool g_timer_done_flag = false;

static volatile bool g_stopwatch_running = false;
static volatile uint32_t g_stopwatch_seconds = 0U;
static volatile bool g_stopwatch_tick_flag = false;

/* UPDATED: Threshold set back to 26.0 C (260 tenths) */
static volatile int16_t g_temp_threshold_tenths = 260; 
static volatile int16_t g_temp_latest_tenths = 0;
static volatile bool g_temp_sample_flag = false;
static volatile bool g_temp_over_threshold_latched = false;

static void app_print_two_digits(uint16_t value)
{
    UART0_SendChar((char)('0' + ((value / 10U) % 10U)));
    UART0_SendChar((char)('0' + (value % 10U)));
}

static void app_print_mmss(uint32_t total_seconds)
{
    uint16_t minutes = (uint16_t)((total_seconds / 60U) % 100U);
    uint16_t seconds = (uint16_t)(total_seconds % 60U);
    app_print_two_digits(minutes);
    UART0_SendChar(':');
    app_print_two_digits(seconds);
}

static void app_print_mode_header(AppMode mode)
{
    UART0_SendString("\r\n==============================\r\n");
    UART0_SendString("Mode: ");

    if (mode == APP_MODE_CALCULATOR) UART0_SendString("Calculator");
    else if (mode == APP_MODE_TIMER) UART0_SendString("Timer");
    else if (mode == APP_MODE_STOPWATCH) UART0_SendString("Stopwatch");
    else UART0_SendString("Temperature Monitor");

    UART0_SendString("\r\n==============================\r\n");
}

static void app_print_help_for_mode(AppMode mode)
{
    UART0_SendString("Global: M0/M1/M2/M3 (mode), H (help)\r\n");

    if (mode == APP_MODE_CALCULATOR)
    {
        UART0_SendString("Calculator format: <num><+|-|*|/><num>=\r\n");
        UART0_SendString("+:Add  -:Sub  *:Mult  /:Div  = :Result\r\n");
    }
    else if (mode == APP_MODE_TIMER) UART0_SendString("Timer commands: Tmm:ss, S(start), P(pause), R(reset)\r\n");
    else if (mode == APP_MODE_STOPWATCH) UART0_SendString("Stopwatch commands: S(start), P(pause), R(reset)\r\n");
    else UART0_SendString("Temp commands: Wxx.x (warning threshold in C)\r\n");
}

static void app_apply_mode(AppMode new_mode)
{
    SysTickTimer_Stop();
    g_timer_running = false;
    GPTM_Stopwatch_Stop();
    g_stopwatch_running = false;
    GPTM_AdcTrigger_Stop();

    g_mode = new_mode;
    AppLogic_SetMode(new_mode);
    app_print_mode_header(new_mode);
    app_print_help_for_mode(new_mode);

    if (new_mode == APP_MODE_TIMER)
    {
        UART0_SendString("Timer value: ");
        app_print_mmss(g_timer_remaining_seconds);
        UART0_SendString("\r\n");
    }
    else if (new_mode == APP_MODE_STOPWATCH)
    {
        UART0_SendString("Stopwatch: ");
        app_print_mmss(g_stopwatch_seconds);
        UART0_SendString("\r\n");
    }
    else if (new_mode == APP_MODE_TEMPERATURE)
    {
        GPTM_AdcTrigger_Start();
        UART0_SendString("Threshold: ");
        UART0_SendFixed((float)g_temp_threshold_tenths / 10.0f, 1U);
        UART0_SendString(" C\r\n");
    }
}

static void app_cycle_mode(int8_t step)
{
    int8_t next = (int8_t)g_mode + step;
    if (next < 0) next = (int8_t)APP_MODE_TEMPERATURE;
    else if (next > (int8_t)APP_MODE_TEMPERATURE) next = (int8_t)APP_MODE_CALCULATOR;
    app_apply_mode((AppMode)next);
}

static bool app_is_near_zero(float x) { return (x < 0.000001f) && (x > -0.000001f); }

static void app_execute_calc(float lhs, float rhs, char op)
{
    float result = 0.0f;
    bool valid = true;

    if (op == '+' || op == 'A') result = lhs + rhs;
    else if (op == '-' || op == 'B') result = lhs - rhs;
    else if (op == '*' || op == 'X' || op == 'C') result = lhs * rhs;
    else if (op == '/')
    {
        if (app_is_near_zero(rhs))
        {
            valid = false;
            UART0_SendString("\r\nError: division by zero.\r\n");
            Alert_RequestBlink3x1Hz();
        }
        else result = lhs / rhs;
    }
    else valid = false;

    if (valid)
    {
        UART0_SendString("\r\nResult = ");
        UART0_SendFixed(result, 3U);
        UART0_SendString("\r\n");
    }
}

static void app_print_parse_error(AppParseError error_code)
{
    UART0_SendString("Parse error: ");
    if (error_code == APP_PARSE_ERR_FORMAT) UART0_SendString("invalid format");
    else if (error_code == APP_PARSE_ERR_RANGE) UART0_SendString("value out of range");
    else UART0_SendString("unknown command");
    UART0_SendString(". Press H for help.\r\n");
}

static void app_dispatch_from_logic(const AppCommand *cmd)
{
    if (cmd == 0) return;
    switch (cmd->type)
    {
        case APP_CMD_MODE_SET: app_apply_mode(cmd->data.mode_set.mode); break;
        case APP_CMD_MODE_STEP: app_cycle_mode(cmd->data.mode_step.step); break;
        case APP_CMD_CALCULATE:
            if (g_mode == APP_MODE_CALCULATOR) app_execute_calc(cmd->data.calc.lhs, cmd->data.calc.rhs, cmd->data.calc.op);
            else UART0_SendString("Calculator expression ignored in this mode.\r\n");
            break;
        case APP_CMD_TIMER_SET:
            if (g_mode == APP_MODE_TIMER) {
                g_timer_initial_seconds = cmd->data.timer_set.total_seconds;
                g_timer_remaining_seconds = cmd->data.timer_set.total_seconds;
                g_timer_running = false; SysTickTimer_Stop();
                UART0_SendString("Timer set to "); app_print_mmss(g_timer_remaining_seconds); UART0_SendString("\r\n");
            } break;
        case APP_CMD_TIMER_START:
            if (g_mode == APP_MODE_TIMER) {
                if (g_timer_remaining_seconds > 0U) { g_timer_running = true; SysTickTimer_Start(); UART0_SendString("Timer started.\r\n"); }
                else UART0_SendString("Set timer first using Tmm:ss.\r\n");
            } break;
        case APP_CMD_TIMER_PAUSE:
            if (g_mode == APP_MODE_TIMER) { g_timer_running = false; SysTickTimer_Stop(); UART0_SendString("Timer paused at "); app_print_mmss(g_timer_remaining_seconds); UART0_SendString("\r\n"); } break;
        case APP_CMD_TIMER_RESET:
            if (g_mode == APP_MODE_TIMER) { g_timer_running = false; SysTickTimer_Stop(); g_timer_remaining_seconds = g_timer_initial_seconds; UART0_SendString("Timer reset to "); app_print_mmss(g_timer_remaining_seconds); UART0_SendString("\r\n"); } break;
        case APP_CMD_STOPWATCH_START: if (g_mode == APP_MODE_STOPWATCH) { g_stopwatch_running = true; GPTM_Stopwatch_Start(); UART0_SendString("Stopwatch started.\r\n"); } break;
        case APP_CMD_STOPWATCH_PAUSE: if (g_mode == APP_MODE_STOPWATCH) { g_stopwatch_running = false; GPTM_Stopwatch_Stop(); UART0_SendString("Stopwatch paused at "); app_print_mmss(g_stopwatch_seconds); UART0_SendString("\r\n"); } break;
        case APP_CMD_STOPWATCH_RESET: if (g_mode == APP_MODE_STOPWATCH) { g_stopwatch_running = false; GPTM_Stopwatch_Stop(); g_stopwatch_seconds = 0U; UART0_SendString("Stopwatch reset to 00:00\r\n"); } break;
        case APP_CMD_TEMP_THRESHOLD_SET:
            g_temp_threshold_tenths = (int16_t)(cmd->data.temp_threshold.threshold_c * 10.0f);
            g_temp_over_threshold_latched = false;
            UART0_SendString("Threshold set to "); UART0_SendFixed((float)g_temp_threshold_tenths / 10.0f, 1U); UART0_SendString(" C\r\n"); break;
        case APP_CMD_HELP: app_print_help_for_mode(g_mode); break;
        case APP_CMD_PARSE_ERROR: app_print_parse_error(cmd->data.parse_error.code); break;
        default: break;
    }
}

void App_Init(void)
{
    UART0_Init(); GPIOF_Init(); SysTickTimer_Init1Hz(); GPTM_Stopwatch_Init1Hz(); GPTM_AdcTrigger_Init1Hz(); AdcTemp_InitTimerTriggered();
    AppLogic_Init(app_dispatch_from_logic);
    UART0_SendString("\r\nMulti-Function Embedded Console (ARM Cortex-M4)\r\n");
    UART0_SendString("Mode switch via PF0/PF4 interrupts and UART commands.\r\n");
    app_apply_mode(APP_MODE_CALCULATOR);
}

void App_Run(void)
{
    if (g_timer_tick_flag) { g_timer_tick_flag = false; if (g_mode == APP_MODE_TIMER) { UART0_SendString("Timer: "); app_print_mmss(g_timer_remaining_seconds); UART0_SendString("\r\n"); } }
    if (g_timer_done_flag) { g_timer_done_flag = false; UART0_SendString("Timer finished.\r\n"); Alert_RequestBlink3x1Hz(); }
    if (g_stopwatch_tick_flag) { g_stopwatch_tick_flag = false; if (g_mode == APP_MODE_STOPWATCH) { UART0_SendString("Stopwatch: "); app_print_mmss(g_stopwatch_seconds); UART0_SendString("\r\n"); } }
    if (g_temp_sample_flag) {
        g_temp_sample_flag = false;
        if (g_mode == APP_MODE_TEMPERATURE) { UART0_SendString("Current Temp: "); UART0_SendFixed((float)g_temp_latest_tenths / 10.0f, 1U); UART0_SendString(" C\r\n"); }
        if ((g_temp_latest_tenths > g_temp_threshold_tenths) && !g_temp_over_threshold_latched) { g_temp_over_threshold_latched = true; UART0_SendString("Warning: temperature exceeded threshold.\r\n"); Alert_RequestBlink3x1Hz(); }
        else if (g_temp_latest_tenths <= g_temp_threshold_tenths) g_temp_over_threshold_latched = false;
    }
    Alert_Process();
}

void App_OnUartRxChar(char ch) { if (ch == '\r') UART0_SendString("\r\n"); else UART0_SendChar(ch); AppLogic_OnUartChar(ch); }
void App_OnModeButtonStep(int8_t step) { app_cycle_mode(step); }
void App_OnSysTickTick(void) { if (g_mode != APP_MODE_TIMER || !g_timer_running) return; if (g_timer_remaining_seconds > 0U) { --g_timer_remaining_seconds; g_timer_tick_flag = true; } if (g_timer_remaining_seconds == 0U) { g_timer_running = false; SysTickTimer_Stop(); g_timer_done_flag = true; } }
void App_OnStopwatchTick(void) { if ((g_mode == APP_MODE_STOPWATCH) && g_stopwatch_running) { ++g_stopwatch_seconds; g_stopwatch_tick_flag = true; } }
void App_OnAdcSampleRaw(uint16_t raw_sample) { g_temp_latest_tenths = AdcTemp_RawToCelsiusTenths(raw_sample); g_temp_sample_flag = true; }