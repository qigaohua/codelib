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

int Socket(const char *host, int clientPort)
{
    int sock;
    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;
    
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;

    inaddr = inet_addr(host);
    if (inaddr != INADDR_NONE)
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
    else
    {
        hp = gethostbyname(host);
        if (hp == NULL)
            return -1;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(clientPort);
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) 
		perror("line:%d fun:%s socket() error", __LINE__, __FUNCTION__);

    if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0)
        return -1;
    return sock;
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
    register int i, count;
    register unsigned int val;

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
  
  	/*
  	 *	Something else, probably a multicast. 
  	 */
  	 
  	return(0);
}
