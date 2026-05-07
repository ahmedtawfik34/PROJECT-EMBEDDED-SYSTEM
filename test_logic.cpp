#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "app_logic.h"

static AppCommand g_last_cmd;
static int g_dispatch_count = 0;

static void test_dispatch(const AppCommand *cmd)
{
    g_last_cmd = *cmd;
    ++g_dispatch_count;
}

static void reset_capture()
{
    memset(&g_last_cmd, 0, sizeof(g_last_cmd));
    g_dispatch_count = 0;
}

static void send_line(const char *line)
{
    while (*line != '\0')
    {
        AppLogic_OnUartChar(*line++);
    }
    AppLogic_OnUartChar('\r');
}

static int nearly_equal(float a, float b, float tol)
{
    return fabsf(a - b) <= tol;
}

int main()
{
    AppLogic_Init(test_dispatch);

    reset_capture();
    send_line("12.5A7.5D");
    assert(g_dispatch_count == 1);
    assert(g_last_cmd.type == APP_CMD_CALCULATE);
    assert(nearly_equal(g_last_cmd.data.calc.lhs, 12.5f, 0.0001f));
    assert(nearly_equal(g_last_cmd.data.calc.rhs, 7.5f, 0.0001f));
    assert(g_last_cmd.data.calc.op == 'A');

    reset_capture();
    send_line("M2");
    assert(g_dispatch_count == 1);
    assert(g_last_cmd.type == APP_CMD_MODE_SET);
    assert(g_last_cmd.data.mode_set.mode == APP_MODE_STOPWATCH);

    AppLogic_SetMode(APP_MODE_TIMER);
    reset_capture();
    send_line("T01:30");
    assert(g_dispatch_count == 1);
    assert(g_last_cmd.type == APP_CMD_TIMER_SET);
    assert(g_last_cmd.data.timer_set.total_seconds == 90U);

    reset_capture();
    send_line("S");
    assert(g_dispatch_count == 1);
    assert(g_last_cmd.type == APP_CMD_TIMER_START);

    AppLogic_SetMode(APP_MODE_TEMPERATURE);
    reset_capture();
    send_line("W37.5");
    assert(g_dispatch_count == 1);
    assert(g_last_cmd.type == APP_CMD_TEMP_THRESHOLD_SET);
    assert(nearly_equal(g_last_cmd.data.temp_threshold.threshold_c, 37.5f, 0.0001f));

    reset_capture();
    send_line("abc");
    assert(g_dispatch_count == 1);
    assert(g_last_cmd.type == APP_CMD_PARSE_ERROR);

    AppLogic_SetMode(APP_MODE_CALCULATOR);
    reset_capture();
    send_line("10A2");
    assert(g_dispatch_count == 1);
    assert(g_last_cmd.type == APP_CMD_PARSE_ERROR);
    assert(g_last_cmd.data.parse_error.code == APP_PARSE_ERR_FORMAT);

    printf("All parser tests passed.\n");
    return 0;
}
