#include "log.h"

#define __DEBUG__

/* get total line nums of a file 
 * err return -1, ok return line nums
 */
int util_get_file_count_lines(char *file_name)
{
    char c, lc;
    int line_nums = 0;
    lc = 0;

    /* get file nums */
    FILE *fp = fopen(file_name, "r");
    if (!fp) {
        return -1;
    }
    while((c = fgetc(fp)) != EOF) {
        if (c == '\n') line_nums++;
        lc = c;
    }
    if (lc != '\n') line_nums++;
    fclose(fp);
    return line_nums;
}

int system1(char *cmd)
{
    int ret = -1;
    
    if (!cmd) {
        LOGERR("param null.\n");
        return -1;
    }
    
    ret = system(cmd);
    if (ret == -1) {
        LOGERR("system1() error. [cmd:%s] [err:%m]\n", cmd);
    } else {
        if (WIFEXITED(ret)) { /* normal exit script ? */
            ret = WEXITSTATUS(ret);
            if (ret != 0) {
                LOGERR("run shell script fail,  [cmd:%s] [exit code: %d]\n", cmd, ret);
            } else {
                LOGMSG("system1 run ok, [cmd : %s]\n", cmd);
            }
        } else {
            ret = WEXITSTATUS(ret);
            LOGMSG("shell script [%s] exit, status = [%d] \n", cmd, ret);
        }
    }
    
    return ret;
} 
/* delete file content, but only keep the last n lines
 *         n = #keep_line
 */
int log_delete_file_line(char *file_name, int keep_line)
{
    int line_nums = util_get_file_count_lines(file_name);
    char cmd[256];

    if (line_nums > keep_line) {
        snprintf(cmd, sizeof(cmd), "sed -i \'1,%dd\' %s", line_nums - keep_line,  file_name);
        if (system1(cmd)) {
            LOGERR("run cmd err.[cmd : %s]\n", cmd);
            return -1;
        }
    }
    return 0;
}
char *now_to_string(char *buf, int size)
{
	time_t t = time(NULL);
	struct tm *p = localtime(&t);
	snprintf(buf, size, "%04d%02d%02d%02d%02d%02d",
			1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
			p->tm_hour, p->tm_min, p->tm_sec);
	return buf;
}

int recod_log_once(char *log_file, char *log)
{
    char date_str[128] = {0};
    char buf[1024] = {0};
    
    FILE *fp = fopen(log_file,"a+");
    if(!fp) {
        LOGERR("open log file [%s] err.[%m]\n", log_file);
        return -1;
    }
    now_to_string(date_str, sizeof(date_str));
    snprintf(buf, sizeof(buf), "%s %s", date_str, log);
    fwrite(buf, 1, strlen(buf), fp);
    fclose(fp);
    return 0;
}

void log_record(char *log_file, char *log)
{
    #ifdef __DEBUG__
    LOGERR("%s\n", log);
    #endif
    
    int line_nums = util_get_file_count_lines(log_file);

    if (line_nums > MAX_LOG_LINE || line_nums == -1) {
        log_delete_file_line(log_file, LOG_DEL_LINE);
    }
    
    recod_log_once(log_file, log);
}


