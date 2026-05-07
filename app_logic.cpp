#include "app_logic.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace
{
struct LogicState
{
    AppMode mode;
    AppLogicDispatchFn dispatch;
    char line_buffer[80];
    unsigned int line_len;
};

LogicState g_logic = {APP_MODE_CALCULATOR, 0, {0}, 0U};

void dispatch_command(const AppCommand &cmd)
{
    if (g_logic.dispatch != 0)
    {
        g_logic.dispatch(&cmd);
    }
}

void emit_parse_error(AppParseError code)
{
    AppCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = APP_CMD_PARSE_ERROR;
    cmd.data.parse_error.code = code;
    dispatch_command(cmd);
}

void emit_help()
{
    AppCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = APP_CMD_HELP;
    dispatch_command(cmd);
}

bool parse_float_strict(const char *text, float *value)
{
    char *end_ptr = 0;
    if ((text == 0) || (*text == '\0')) return false;
    *value = strtof(text, &end_ptr);
    return (end_ptr != text) && (*end_ptr == '\0');
}

bool parse_uint_in_range(const char *text, unsigned int min_v, unsigned int max_v, unsigned int *out_v)
{
    char *end_ptr = 0;
    unsigned long value;
    if ((text == 0) || (*text == '\0')) return false;
    value = strtoul(text, &end_ptr, 10);
    if ((end_ptr == text) || (*end_ptr != '\0')) return false;
    if ((value < min_v) || (value > max_v)) return false;
    *out_v = (unsigned int)value;
    return true;
}

void normalize_line(const char *src, char *dst, unsigned int dst_size)
{
    unsigned int i = 0U;
    unsigned int o = 0U;
    while ((src[i] != '\0') && (o + 1U < dst_size))
    {
        char c = src[i++];
        if ((c == ' ') || (c == '\t')) continue;
        if ((c >= 'a') && (c <= 'z')) c = (char)(c - ('a' - 'A'));
        dst[o++] = c;
    }
    dst[o] = '\0';
}

void handle_mode_set(const char *line)
{
    AppCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    if ((line[0] != 'M') || (line[2] != '\0') || (line[1] < '0') || (line[1] > '3'))
    {
        emit_parse_error(APP_PARSE_ERR_FORMAT);
        return;
    }
    cmd.type = APP_CMD_MODE_SET;
    cmd.data.mode_set.mode = (AppMode)(line[1] - '0');
    dispatch_command(cmd);
}

void handle_calculator_line(const char *line)
{
    AppCommand cmd;
    char lhs_text[32], rhs_text[32];
    float lhs, rhs;
    unsigned int i, op_pos = 0U;
    char op = '\0';
    unsigned int len = (unsigned int)strlen(line);

    // FIX: Format is now <num><symbol><num>= 
    if ((len < 4U) || (line[len - 1U] != '='))
    {
        emit_parse_error(APP_PARSE_ERR_FORMAT);
        return;
    }

    for (i = 0U; i + 1U < len; ++i)
    {
        char c = line[i];
        // FIX: Recognize standard symbols +, -, *, /, X
        if ((c == '+') || (c == '-') || (c == '*') || (c == 'X') || (c == '/'))
        {
            if (op != '\0') { emit_parse_error(APP_PARSE_ERR_FORMAT); return; }
            op = (c == 'X') ? '*' : c; // Internalize X as *
            op_pos = i;
        }
    }

    if ((op == '\0') || (op_pos == 0U) || (op_pos + 1U >= len - 1U))
    {
        emit_parse_error(APP_PARSE_ERR_FORMAT);
        return;
    }

    memset(lhs_text, 0, sizeof(lhs_text));
    memset(rhs_text, 0, sizeof(rhs_text));
    strncpy(lhs_text, line, op_pos);
    strncpy(rhs_text, line + op_pos + 1U, (len - 1U) - (op_pos + 1U));

    if (!parse_float_strict(lhs_text, &lhs) || !parse_float_strict(rhs_text, &rhs))
    {
        emit_parse_error(APP_PARSE_ERR_FORMAT);
        return;
    }

    memset(&cmd, 0, sizeof(cmd));
    cmd.type = APP_CMD_CALCULATE;
    cmd.data.calc.lhs = lhs;
    cmd.data.calc.rhs = rhs;
    cmd.data.calc.op = op; // Send standard symbol to app.c
    dispatch_command(cmd);
}

void handle_timer_line(const char *line)
{
    AppCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    if ((line[0] == 'S') && (line[1] == '\0')) { cmd.type = APP_CMD_TIMER_START; dispatch_command(cmd); return; }
    if ((line[0] == 'P') && (line[1] == '\0')) { cmd.type = APP_CMD_TIMER_PAUSE; dispatch_command(cmd); return; }
    if ((line[0] == 'R') && (line[1] == '\0')) { cmd.type = APP_CMD_TIMER_RESET; dispatch_command(cmd); return; }
    if (line[0] == 'T')
    {
        unsigned int mm = 0U, ss = 0U, mm_len;
        const char *colon = strchr(line + 1, ':');
        char mm_text[4] = {0}, ss_text[3] = {0};
        if (colon == 0) { emit_parse_error(APP_PARSE_ERR_FORMAT); return; }
        mm_len = (unsigned int)(colon - (line + 1));
        if ((mm_len == 0U) || (mm_len > 2U)) { emit_parse_error(APP_PARSE_ERR_FORMAT); return; }
        strncpy(mm_text, line + 1, mm_len);
        strncpy(ss_text, colon + 1, 2U);
        if ((colon[1] == '\0') || (colon[2] == '\0') || (colon[3] != '\0')) { emit_parse_error(APP_PARSE_ERR_FORMAT); return; }
        if (!parse_uint_in_range(mm_text, 0U, 99U, &mm) || !parse_uint_in_range(ss_text, 0U, 59U, &ss)) { emit_parse_error(APP_PARSE_ERR_RANGE); return; }
        cmd.type = APP_CMD_TIMER_SET;
        cmd.data.timer_set.total_seconds = (uint16_t)(mm * 60U + ss);
        dispatch_command(cmd);
        return;
    }
    emit_parse_error(APP_PARSE_ERR_UNKNOWN);
}

void handle_stopwatch_line(const char *line)
{
    AppCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    if ((line[0] == 'S') && (line[1] == '\0')) cmd.type = APP_CMD_STOPWATCH_START;
    else if ((line[0] == 'P') && (line[1] == '\0')) cmd.type = APP_CMD_STOPWATCH_PAUSE;
    else if ((line[0] == 'R') && (line[1] == '\0')) cmd.type = APP_CMD_STOPWATCH_RESET;
    else { emit_parse_error(APP_PARSE_ERR_UNKNOWN); return; }
    dispatch_command(cmd);
}

void handle_temperature_line(const char *line)
{
    AppCommand cmd;
    float threshold;
    if (line[0] != 'W') { emit_parse_error(APP_PARSE_ERR_UNKNOWN); return; }
    if (!parse_float_strict(line + 1, &threshold)) { emit_parse_error(APP_PARSE_ERR_FORMAT); return; }
    if ((threshold < -40.0f) || (threshold > 125.0f)) { emit_parse_error(APP_PARSE_ERR_RANGE); return; }
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = APP_CMD_TEMP_THRESHOLD_SET;
    cmd.data.temp_threshold.threshold_c = threshold;
    dispatch_command(cmd);
}

void process_line()
{
    char normalized[80];
    const char *line = normalized;
    normalize_line(g_logic.line_buffer, normalized, sizeof(normalized));
    if (line[0] == '\0') return;
    if ((strcmp(line, "H") == 0) || (strcmp(line, "HELP") == 0)) { emit_help(); return; }
    if ((line[0] == 'M') && (line[1] != '\0')) { handle_mode_set(line); return; }
    if (g_logic.mode == APP_MODE_CALCULATOR) handle_calculator_line(line);
    else if (g_logic.mode == APP_MODE_TIMER) handle_timer_line(line);
    else if (g_logic.mode == APP_MODE_STOPWATCH) handle_stopwatch_line(line);
    else handle_temperature_line(line);
}

} // namespace

extern "C" void AppLogic_Init(AppLogicDispatchFn dispatch_fn)
{
    g_logic.mode = APP_MODE_CALCULATOR;
    g_logic.dispatch = dispatch_fn;
    g_logic.line_len = 0U;
    memset(g_logic.line_buffer, 0, sizeof(g_logic.line_buffer));
}

extern "C" void AppLogic_SetMode(AppMode mode)
{
    g_logic.mode = mode;
    g_logic.line_len = 0U;
    memset(g_logic.line_buffer, 0, sizeof(g_logic.line_buffer));
}

extern "C" AppMode AppLogic_GetMode(void) { return g_logic.mode; }

extern "C" void AppLogic_OnUartChar(char ch)
{
    if ((ch == '\r') || (ch == '\n'))
    {
        g_logic.line_buffer[g_logic.line_len] = '\0';
        process_line();
        g_logic.line_len = 0U;
        memset(g_logic.line_buffer, 0, sizeof(g_logic.line_buffer));
        return;
    }

    if ((ch == '\b') || (ch == 127))
    {
        if (g_logic.line_len > 0U) { --g_logic.line_len; g_logic.line_buffer[g_logic.line_len] = '\0'; }
        return;
    }

    if (!isprint((unsigned char)ch)) return;

    if (g_logic.line_len + 1U < sizeof(g_logic.line_buffer))
    {
        g_logic.line_buffer[g_logic.line_len++] = ch;
        
        // FIX: Instant trigger when '=' is pressed
        if ((g_logic.mode == APP_MODE_CALCULATOR) && (ch == '='))
        {
            g_logic.line_buffer[g_logic.line_len] = '\0';
            process_line();
            g_logic.line_len = 0U;
            memset(g_logic.line_buffer, 0, sizeof(g_logic.line_buffer));
        }
    }
}