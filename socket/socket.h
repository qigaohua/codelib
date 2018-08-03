#ifndef __SOCKET__
#define __SOCKET__
#include <inttypes.h>
#include <syslog.h>
#include <unistd.h>


#define	IN_CLASSA(a)		((((long int) (a)) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSB(a)		((((long int) (a)) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSC(a)		((((long int) (a)) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00

#define NET_ERROR_INFO_EXIT(fmt, ...) \
    do { \
        printf("[%s][%d][error]:" fmt"\n", __FILE__, __LINE__,  ##__VA_ARGS__); \
        syslog(LOG_ERR, "[%s][%d][error]:" fmt"\n", __FILE__, __LINE__,  ##__VA_ARGS__);\
        exit(-1); \
    } while(0);

#define NET_ERROR_INFO(fmt, ...) \
    do { \
        printf("[%s][%d][error]:" fmt"\n", __FILE__, __LINE__,  ##__VA_ARGS__); \
        syslog(LOG_ERR, "[%s][%d][error]:" fmt"\n", __FILE__, __LINE__,  ##__VA_ARGS__);\
    } while(0);


#define NET_LOGG_INFO(fmt, ...) \
    do { \
        printf("[%s][%d][error]:" fmt"\n", __FILE__, __LINE__, ## __VA_ARGS__); \
        syslog(LOG_INFO, "[%s][%d][error]:" fmt"\n", __FILE__, __LINE__, ## __VA_ARGS__);\
    } while(0);

#define check_param(p, type) \
    do { \
        if (!(p)) {\
            NET_LOGG_INFO("Invaild parameter"); \
            return (type); \
        } \
    } while(0)


struct net_info {
    char net_host[128];
    uint16_t net_port;
};
typedef struct net_info NET_INFO, net_info_p;



int tcpSocket(const char *host, int port);
ssize_t Send(int fd, void *buf, size_t num);
ssize_t Recv(int fd, void *buf, size_t num);
unsigned long ip_get_mask(unsigned long addr);
static inline int get_mac_by_name(const char *device, unsigned char *buf);
static inline int str_is_ip(const char *str, int str_len);

#endif

