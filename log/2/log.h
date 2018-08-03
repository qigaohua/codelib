#ifndef __LOG_H__
#define __LOG_H__ 


#define GH_LOG_ERROR    (1 << 0)
#define GH_LOG_WARN     (1 << 1)
#define GH_LOG_MSG      (1 << 2)
#define GH_LOG_DEBUG    (1 << 3)

#define MAX_LINE  10
#define KEEP_LINE 5

#define CHECK_FMT(a,b) __attribute__((format(printf, a, b)))

typedef void (*log_print_func)(int, const char*);
typedef void (*log_exit_func)(int);

void log_error(int errcode, const char *fmt, ...) CHECK_FMT(2, 3);
void log_xerror(int errcode, const char *fmt, ...) CHECK_FMT(2, 3);
void log_warn(const char *fmt, ...) CHECK_FMT(1,2);
void log_xwarn(const char *fmt, ...) CHECK_FMT(1,2);
void log_xmsg(const char *fmt, ...) CHECK_FMT(1,2);
void log_recold_file(int severity, const char *file, const char *fmt, ...) CHECK_FMT(3, 4);

void log_debug(const char *fmt, ...);

#endif
