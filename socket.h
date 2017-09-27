#ifndef __SOCKET__
#define __SOCKET__


#define	IN_CLASSA(a)		((((long int) (a)) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSB(a)		((((long int) (a)) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSC(a)		((((long int) (a)) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00

int Socket(const char *host, int clientPort);
unsigned long ip_get_mask(unsigned long addr);
static inline int get_mac_by_name(const char *device, unsigned char *buf);
static inline int str_is_ip(const char *str, int str_len);

#endif

