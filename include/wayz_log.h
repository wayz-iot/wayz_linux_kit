
#ifndef  __WAYZ_LOG_H_
#define  __WAYZ_LOG_H_

#define  WAYZ_FAIL      -1
#define  WAYZ_OK        0


#define LOG_LOCAL_LEVEL  3

/**
 * @brief Log level
 *
 */
typedef enum {
    WAYZ_LOG_NONE = 0,   /*!< No log output */
    WAYZ_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    WAYZ_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    WAYZ_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    WAYZ_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    WAYZ_LOG_VERBOSE,    /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */

    WAYZ_LOG_MAX
} wayz_log_level_t;

void wayz_log_write(const char *filename, const char *func, int line, wayz_log_level_t level, const char *fmt, ...);

#define WAYZ_LOG_LEVEL(level, format, ...) do {                     \
        if (level==WAYZ_LOG_ERROR )          { wayz_log_write(__FILE__, __FUNCTION__, __LINE__,WAYZ_LOG_ERROR,   format, ##__VA_ARGS__); } \
        else if (level==WAYZ_LOG_WARN )      { wayz_log_write(__FILE__, __FUNCTION__, __LINE__,WAYZ_LOG_WARN,    format, ##__VA_ARGS__); } \
        else if (level==WAYZ_LOG_DEBUG )     { wayz_log_write(__FILE__, __FUNCTION__, __LINE__,WAYZ_LOG_DEBUG,   format, ##__VA_ARGS__); } \
        else if (level==WAYZ_LOG_VERBOSE )   { wayz_log_write(__FILE__, __FUNCTION__, __LINE__,WAYZ_LOG_VERBOSE, format, ##__VA_ARGS__); } \
        else                                { wayz_log_write(__FILE__, __FUNCTION__, __LINE__,WAYZ_LOG_INFO,    format, ##__VA_ARGS__); } \
    } while(0)


#define WAYZ_LOG_LEVEL_LOCAL(level, format, ...) do {               \
        if ( LOG_LOCAL_LEVEL >= level ) WAYZ_LOG_LEVEL(level, format, ##__VA_ARGS__); \
    } while(0)

#define WAYZ_LOGE( format, ... ) WAYZ_LOG_LEVEL_LOCAL(WAYZ_LOG_ERROR,   format, ##__VA_ARGS__)
#define WAYZ_LOGW( format, ... ) WAYZ_LOG_LEVEL_LOCAL(WAYZ_LOG_WARN,    format, ##__VA_ARGS__)
#define WAYZ_LOGI( format, ... ) WAYZ_LOG_LEVEL_LOCAL(WAYZ_LOG_INFO,    format, ##__VA_ARGS__)
#define WAYZ_LOGD( format, ... ) WAYZ_LOG_LEVEL_LOCAL(WAYZ_LOG_DEBUG,   format, ##__VA_ARGS__)
#define WAYZ_LOGV( format, ... ) WAYZ_LOG_LEVEL_LOCAL(WAYZ_LOG_VERBOSE, format, ##__VA_ARGS__)


#endif
