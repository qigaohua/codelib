


#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)


/*根据key从文件fname中找出对应值
 *如：char * mysql_user = conf_get_by_key(ISMS_CONF_IRCS, "isms_mysql_user");
 * */
char *conf_get_by_key(const char *fname, const char *key)
{
    int len = 0;
    int key_len = 0;
    char buf[4096];
    FILE *fp = NULL;
    char *p = NULL;

    if (fname == NULL || key == NULL) {
        return NULL;
    }
    key_len = strlen(key);
    if (key_len < 1) {
        return NULL;
    }
    fp = fopen(fname, "rb");
    if (fp == NULL) {
        return NULL;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == '#') {
            continue;
        }
        len = strlen(buf);
        if (len < 2) {
            continue;
        }
        /* Remove the EOL */
        /* EOL == CR */
        if (buf[len - 1] == '\r') {
            buf[len - 1] = 0;
            len -= 1;
        } else if (buf[len - 1] == '\n') {
            /* EOL = CR+LF */
            if (buf[len - 2] == '\r') {
                buf[len - 2] = 0;
                len -= 2;
            } else {
                /* EOL = LF */
                buf[len - 1] = 0;
                len -= 1;
            }
        }

        /* Skip the empty line */
        if (len == 0) {
            continue;
        }
        if (strncasecmp(buf, key, key_len) == 0 && buf[key_len] == '=') {
            p = malloc(len - key_len);
            if (p == NULL) {
                syslog(LOG_INFO, "Malloc memory error.\n");
                fclose(fp);
                return NULL;
            }
            strcpy(p, buf + key_len + 1);
        }
    }

    fclose(fp);

    return p;
}

#define LOCK_FILE "/var/run/jfind_ircs.pid"   
#define LOCK_MODE (S_IRUSR|S_IWUSR|S_IRGRP)

int is_already_running(const char *lock_file, mode_t lock_mode) {
    int ret, fd;
    char buf[32];
    struct flock fl;

    fd = open(lock_file, O_RDWR|O_CREAT, lock_mode);
    if (fd < 0) {
#ifdef DEBUG
        syslog(LOG_INFO, "open lock file[%s] error.\n", lock_file);
#endif
        exit(1);
    }
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    ret = fcntl(fd, F_SETLK, &fl);
    if (ret) {
        /* already running or some error. */
        close(fd);
        return 1;
    }
    /* O.K. write the pid */
    ret = ftruncate(fd,0);
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)getpid());
    ret = write(fd, buf, strlen(buf) + 1);

    return 0;
}



#ifdef __DAEMON__
static void daemon_init() {
    int ret, i;
    pid_t pid;
    struct sigaction sa;

    ret = umask(0);
    pid = fork();
    if (pid < 0) {
        exit(1);
    } else if (pid != 0) {
        exit(0);
    }

    ret = setsid();
    if (ret == ((pid_t) -1)) {
        exit(1);
    }

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    ret = sigaction(SIGHUP, &sa, NULL);
    if (ret < 0) {
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        exit(1);
    } else if (pid != 0) {
        exit(0);
    }

    ret = chdir("/");
    if (ret) {
        exit(1);
    }
    return;
}
#endif

