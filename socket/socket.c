#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <unistd.h>

#include "socket.h"



int open_clientfd(const char *hostname, const char *port)
{
    int sfd, s;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM; // open a connection
    hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
    hints.ai_protocol = 0;          // any protocol
    s = getaddrinfo(hostname, port, &hints, &listp);
    if (0 != s) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(s));
        return -1;
    }

    for(p = listp; p != NULL; p = p->ai_next) {
        sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (-1 == sfd) 
            continue;
        if (connect(sfd, p->ai_addr, p->ai_addrlen) != -1)
            break;
        
        close(sfd);    
    }

    freeaddrinfo(listp);

    if ( !p ) {
        fprintf(stderr, "Can't open a connection.\n");
        return -1;
    }

    return sfd;
}


int open_listenfd(const char *port)
#define LISTENQ  1024
{
    int sfd, s, optval = 1;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM; // open a connection
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // any ip address
    //设置了AI_NUMERICSERV 标志并且该参数未设置为NULL，
    //那么该参数必须是一个指向10进制的端口号字符串
    hints.ai_flags = AI_NUMERICSERV;   // using port number
    hints.ai_protocol = 0;          // any protocol
    // 设置了AI_PASSIVE 且 第一个参数为NULL,
    // 返回的地址是通配符地址(wildcard address, IPv4时是INADDR_ANY,IPv6时是IN6ADDR_ANY_INIT)
    s = getaddrinfo(NULL, port, &hints, &listp);
    if (0 != s) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(s));
        return -1;
    }

    for(p = listp; p != NULL; p = p->ai_next) {
        sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (-1 == sfd) 
            continue;

        if (-1 == setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, 
                    (const void *)&optval, sizeof(int))) {
            fprintf(stderr, "setsockopt failed: %m.\n");
            close(sfd);return -1;
        }

        if (bind(sfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(sfd);    
    }

    freeaddrinfo(listp);

    if ( !p ) {
        fprintf(stderr, "Can't open a connection.\n");
        return -1;
    }

    if (-1 == listen(sfd, LISTENQ)) {
        fprintf(stderr, "listen failed: %m\n");
        close(sfd);
        return -1;
    }

    return sfd;
}



ssize_t rio_readn(int fd, void *buf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *ptr = (char *)buf;

    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return (-1);   // error
        }
        else if (nread == 0)   // EOF
            break;

        ptr += nread;
        nleft -= nread;
    }

    return (n - nleft);
}


ssize_t rio_writen(int fd, void *buf, size_t n)
{
    size_t nleft = n;
    ssize_t nwrite;

    char *ptr = (char *)buf;

    while (nleft > 0) {
        if ((nwrite = write(fd, ptr, nleft)) <= 0) {
            if (errno == EINTR)
                nwrite = 0;
            else
                return (-1); // error
        }

        ptr += nwrite;
        nleft -= nwrite;
    }

    return n;
}

/*
 * 带有缓冲区的RIO函数。
 * 缓冲区存在的目的是为了减少因多次调用系统级IO函数，陷入内核态而带来的额外开销
 */
#define RIO_BUFSIZE 4096
typedef struct {
    int rio_fd;                //与内部缓冲区关联的描述符
    size_t rio_cnt;            //缓冲区中剩下的字节数
    char *rio_bufptr;          //指向缓冲区中下一个未读的字节
    char rio_buf[RIO_BUFSIZE]; 
} rio_t;


void rio_readinitb(rio_t *rp, int fd) 
{
    rp->rio_fd = fd;  
    rp->rio_cnt = 0;  
    rp->rio_bufptr = rp->rio_buf;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0) {  //缓冲区为空，调用read填充
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, 
                sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0) {
            if (errno != EINTR) /* Interrupted by sig handler return */
                return -1;

        }
        else if (rp->rio_cnt == 0)  /* EOF */
            return 0;
        else 
            rp->rio_bufptr = rp->rio_buf; /* Reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;          
    if (rp->rio_cnt < n)   
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}


ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char *)usrbuf;

    while (nleft > 0) {
        if ((nread = rio_read(rp, bufp, nleft)) < 0) 
            return -1;          /* errno set by read() */ 
        else if (nread == 0)
            break;              /* EOF */

        nleft -= nread;
        bufp += nread;
    }

    return (n - nleft);         /* return >= 0 */
}

// 读一行
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    size_t  n; 
    ssize_t rc;
    char c, *bufp = (char *)usrbuf;

    for (n = 1; n < maxlen; n++) { 
        if ((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n') {
                n++;
                break;

            }
        } else if (rc == 0) {
            if (n == 1)
                return 0; //第一次读取就到了EOF
            else
                break;    //读了一些数据后遇到EOF

        } else
            return -1;    /* Error */

    }
    *bufp = 0;

    return n-1;
}


/*通过网络接口（如eth1）获取 mac地址
 *device: eth1
 *buf:  mac
 
struct ifreq {
#define IFHWADDRLEN	6
	union
	{
		char	ifrn_name[IFNAMSIZ];		// if name, e.g. "en0" 
	} ifr_ifrn;
	
	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		struct	sockaddr ifru_netmask;
		struct  sockaddr ifru_hwaddr;
		short	ifru_flags;
		int	ifru_ivalue;
		int	ifru_mtu;
		struct  ifmap ifru_map;
		char	ifru_slave[IFNAMSIZ];	// Just fits the size 
		char	ifru_newname[IFNAMSIZ];
		void __user *	ifru_data;
		struct	if_settings ifru_settings;
	} ifr_ifru;
};
#endif                                  // __UAPI_DEF_IF_IFREQ 
 * 
 * */
static inline int get_mac_by_name(const char *device, unsigned char *buf)
{
	int sockfd, ret;
	struct ifreq req;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		return 1;
	}
	snprintf(req.ifr_name, sizeof(req.ifr_name), "%s", device);
	ret = ioctl(sockfd, SIOCGIFHWADDR, &req);
	close(sockfd);
	if (ret == -1) {
		return 1;
	}
	memcpy(buf, req.ifr_hwaddr.sa_data, ETH_ALEN);
	return 0;
}

/*判断字符串是不是一个合法ip*/
static inline int str_is_ip(const char *str, int str_len)
{
    int i, count;
    unsigned int val;

    for (i = 0, count = 0, val = 0; i < str_len; i++) {
        if (str[i] > '0' && str[i] <= '9') {
            val = val * 10 + (str[i] - 0x30);
            if (val > 255) {
                return 0;
            }
        } else if (str[i] == '0') {
            if (val == 0) {
                if (i == str_len -1) {
                    break;
                } else if (str[i+1] != '.') {
                    return 0;
                }
            } else {
                val = val * 10 + (str[i] - 0x30);
                if (val > 255) {
                    return 0;
                }
            }
        } else if (str[i] == '.') {
            count++;
            if (count > 3) {
                return 0;
            }
            val = 0;
        } else if (str[i] == ':') {
            break;
        } else {
            return 0;
        }
    }
    if (count == 3) {
        return 1;
    }

    return 1;
}



unsigned long ip_get_mask(unsigned long addr)
{
  	unsigned long dst;

  	if (addr == 0L) 
  		return(0L);	/* special case */

  	dst = ntohl(addr);
  	if (IN_CLASSA(dst)) 
  		return(htonl(IN_CLASSA_NET));
  	if (IN_CLASSB(dst)) 
  		return(htonl(IN_CLASSB_NET));
  	if (IN_CLASSC(dst)) 
  		return(htonl(IN_CLASSC_NET));

  	return(0);
}
