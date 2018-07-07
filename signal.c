#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/syscall.h>



static void sigsegv_handler(int signal)
{
    char **strings;
    size_t i;
    size_t size;
    uint32_t thread_id = syscall(__NR_gettid);
    void *array[20];

    syslog(LOG_ERR, "thread id %u sig num is %d \n", thread_id, signal);
    size = backtrace (array, 20); 
    strings = backtrace_symbols (array, size);
    syslog(LOG_ERR, "Obtained %zd stack frames.\n", size);
    for (i = 0; i < size; i++) {
        syslog(LOG_ERR, "%s\n", strings[i]);
    }    
    free(strings);
    exit(1);
}

static inline void catch_sig_init(void)
{
    signal(SIGABRT, sigsegv_handler);
    signal(SIGALRM, sigsegv_handler);
    signal(SIGBUS, sigsegv_handler);
    signal(SIGFPE, sigsegv_handler);
    signal(SIGHUP, sigsegv_handler);
    signal(SIGILL, sigsegv_handler);
    signal(SIGKILL, sigsegv_handler);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGQUIT, sigsegv_handler);
    signal(SIGSEGV, sigsegv_handler);
    signal(SIGTERM, sigsegv_handler);
    signal(SIGTRAP, sigsegv_handler);
    signal(SIGXCPU, sigsegv_handler);
    signal(SIGXFSZ, sigsegv_handler);
}

