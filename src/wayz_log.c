
#include <stdarg.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "wayz_log.h"



#define LOG_COLOR_HEAD      "\033[0;%dm"
#define LOG_BOLD_HEAD       "\033[1;%dm"
#define LOG_COLOR_END       "\033[0m"

static const int s_log_color[WAYZ_LOG_MAX] = {
    0,  //  WAYZ_LOG_NONE
    31, //  WAYZ_LOG_ERROR
    33, //  WAYZ_LOG_WARN
    32, //  WAYZ_LOG_INFO
    0,  //  WAYZ_LOG_DEBUG
    0,  //  WAYZ_LOG_VERBOSE
};

// static const char s_log_prefix[WAYZ_LOG_MAX] = {
//     'N', //  LOG_NONE
//     'E', //  LOG_ERROR
//     'W', //  LOG_WARN
//     'I', //  LOG_INFO
//     'D', //  LOG_DEBUG
//     'V', //  LOG_VERBOSE
// };

static int wayz_log_write_str(const char *s)
{
    int ret;

    do {
        ret = putchar(*s);
    } while (ret != EOF && *++s);

    return ret;
}

void wayz_log_write(const char *filename, const char *func, int line, wayz_log_level_t level, const char *fmt, ...)
{
    int ret;
    va_list va;
    char *pbuf;

	static char buf[16];
    int color = level >= WAYZ_LOG_MAX ? 0 : s_log_color[level];

    if (color) {
        sprintf(buf, LOG_COLOR_HEAD, color);
        ret = wayz_log_write_str(buf);
        if (ret == EOF)
            goto out;
    }

    if (WAYZ_LOG_ERROR == level || WAYZ_LOG_WARN == level)
    {
        ret = asprintf(&pbuf, "[%s(%d)]: ", filename, line);
        if (ret < 0)
            goto out;
        ret = wayz_log_write_str(pbuf);
        free(pbuf);
        if (ret == EOF)
        {
            goto out;
        }
    }

    va_start(va, fmt);
    ret = vasprintf(&pbuf, fmt, va);
    va_end(va);
    if (ret < 0)
        goto out;
    ret = wayz_log_write_str(pbuf);
    free(pbuf);
    if (ret == EOF)
        goto out;
	
    if (color) {
        ret = wayz_log_write_str(LOG_COLOR_END);
        if (ret == EOF)
            goto out;
    }
out:
    if (ret > 0)
        putchar('\n');

}


