#ifndef __SDK_LOG_H__
#define __SDK_LOG_H__

#include <sdk_types.h>

/* Max size of a log massage */
#define SDK_LOG_BUF_MAX_SIZE    128


/* Convenient Micros for log print.
 * NOTE: You should #define SDK_LOG_TAG fisrtly in every file
 * before using it.
 *
 * Example: #define SDK_LOG_TAG "GUI"
 */
#define __LOG_PRINT(_lv, _tag, _fmt, _args...)   \
            sdk_log_print(_lv, _tag, __FILE__, __func__, __LINE__, _fmt, ##_args)

#define LOG_INFO(_fmt, _args...)   \
            __LOG_PRINT(SDK_LOG_LV_INFO, SDK_LOG_TAG, _fmt, ##_args)

#define LOG_WARN(_fmt, _args...)   \
            __LOG_PRINT(SDK_LOG_LV_WARN, SDK_LOG_TAG, _fmt, ##_args)

#define LOG_ERR(_fmt, _args...)   \
            __LOG_PRINT(SDK_LOG_LV_ERR, SDK_LOG_TAG, _fmt, ##_args)




/* Definition of log level */
enum {
    SDK_LOG_LV_INFO,
    SDK_LOG_LV_WARN,
    SDK_LOG_LV_ERR
};

/*******************************************
 * API Functions of Log
 *******************************************/
sdk_ret sdk_log_init();
sdk_ret sdk_log_uninit();

sdk_ret sdk_log_print(int level, 
                      const char *tag, 
                      const char *file,
                      const char *func,
                      int line,
                      const char *fmt, ...);


#endif /* __SDK_LOG_H__ */
