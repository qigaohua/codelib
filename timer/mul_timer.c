#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h> 
#include "mul_timer.h"

static struct _timer_manage timer_manage;

static void sig_func(int signo);

/* success, return 0; failed, return -1 */
int init_mul_timer(void)
{
    int ret;
    
    memset(&timer_manage, 0, sizeof(struct _timer_manage));
    if( (timer_manage.old_sigfunc = signal(SIGALRM, sig_func)) == SIG_ERR)
    {
		printf("file:%s line:%d  :signal error.\n", __FILE__, __LINE__);
        return (-1);
    }
    timer_manage.new_sigfunc = sig_func;
    
    timer_manage.value.it_value.tv_sec = MUL_TIMER_RESET_SEC;
    timer_manage.value.it_value.tv_usec = 0;
    timer_manage.value.it_interval.tv_sec = TIMER_UNIT;
    timer_manage.value.it_interval.tv_usec = 0;
    ret = setitimer(ITIMER_REAL, &timer_manage.value, &timer_manage.ovalue); 
    
    return (ret);
}


/* success, return 0; failed, return -1 */
int destroy_mul_timer(void)
{
    int ret;
    
    if( (signal(SIGALRM, timer_manage.old_sigfunc)) == SIG_ERR)
    {

		perror("signal error");
        return (-1);
    }

    ret = setitimer(ITIMER_REAL, &timer_manage.ovalue, &timer_manage.value);
    if(ret < 0)
    {
		perror("setitimer error");
        return (-1);
    } 
    memset(&timer_manage, 0, sizeof(struct _timer_manage));
    
    return(0);
}


/* success, return timer handle(>=0); failed, return -1 */
timer_handle_t set_a_timer(int interval, int (* timer_proc) (void *arg, int arg_len), void *arg, int arg_len)
{
    int i;
    
    if(timer_proc == NULL || interval <= 0)
    {
		printf("file:%s line:%d  :func arg error.\n", __FILE__, __LINE__);
        return (-1);
    } 
    
    for(i = 0; i < MAX_TIMER_CNT; i++)
    {
        if(timer_manage.timer_info[i].state == 1)
            continue;
        
        memset(&timer_manage.timer_info[i], 0, sizeof(timer_manage.timer_info[i]));
        timer_manage.timer_info[i].timer_proc = timer_proc;
        if(arg != NULL)
        {
            if(arg_len > MAX_FUNC_ARG_LEN)
            {
				printf("file:%s line:%d  :func arg error.\n", __FILE__, __LINE__);
                return (-1);
            }
            memcpy(timer_manage.timer_info[i].func_arg, arg, arg_len);
            timer_manage.timer_info[i].arg_len = arg_len;
        }
        timer_manage.timer_info[i].interval = interval;
        timer_manage.timer_info[i].elapse = 0;
        timer_manage.timer_info[i].state = 1;
        break;
    }
    
    if(i >= MAX_TIMER_CNT)
    {
		printf("file:%s line:%d  :timer handle > MAX_TIMER_CNT.\n", __FILE__, __LINE__);
        return (-1);
    }
    return (i);
}


/* success, return 0; failed, return -1 */
int del_a_timer(timer_handle_t handle)
{
    if(handle < 0 || handle >= MAX_TIMER_CNT)
    {
		printf("file:%s line:%d  :func arg error.\n", __FILE__, __LINE__);
        return (-1);
    }
    
    memset(&timer_manage.timer_info[handle], 0, sizeof(timer_manage.timer_info[handle]));
    
    return (0);
}


static void sig_func(int signo)
{
    int i;
    for(i = 0; i < MAX_TIMER_CNT; i++)
    {
        if(timer_manage.timer_info[i].state == 0)
            continue;
        timer_manage.timer_info[i].elapse++;
        if(timer_manage.timer_info[i].elapse == timer_manage.timer_info[i].interval)
        {
            timer_manage.timer_info[i].elapse = 0;
            timer_manage.timer_info[i].timer_proc(timer_manage.timer_info[i].func_arg, timer_manage.timer_info[i].arg_len);
        }
    }
}


