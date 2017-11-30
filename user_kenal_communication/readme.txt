博客地址：https://www.ibm.com/developerworks/cn/linux/l-kerns-usrs/index.html?ca=drs-



一、引言
一般地，在使用虚拟内存技术的多任务系统上，内核和应用有不同的地址空间，因此，在内核和应用之间以及在应用与应用之间进行数据交换需要专门的机制来实现，众所周知，进程间通信（IPC）机制就是为实现应用与应用之间的数据交换而专门实现的，大部分读者可能对进程间通信比较了解，但对应用与内核之间的数据交换机制可能了解甚少，本文将详细介绍 Linux 系统下内核与应用进行数据交换的各种方式，包括内核启动参数、模块参数与 sysfs、sysctl、系统调用、netlink、procfs、seq_file、debugfs 和 relayfs。
二、内核启动参数
Linux 提供了一种通过 bootloader 向其传输启动参数的功能，内核开发者可以通过这种方式来向内核传输数据，从而控制内核启动行为。
通常的使用方式是，定义一个分析参数的函数，而后使用内核提供的宏 __setup把它注册到内核中，该宏定义在 linux/init.h 中，因此要使用它必须包含该头文件：
1
__setup("para_name=", parse_func)
para_name 为参数名，parse_func 为分析参数值的函数，它负责把该参数的值转换成相应的内核变量的值并设置那个内核变量。内核为整数参数值的分析提供了函数 get_option 和 get_options，前者用于分析参数值为一个整数的情况，而后者用于分析参数值为逗号分割的一系列整数的情况，对于参数值为字符串的情况，需要开发者自定义相应的分析函数。在源代码包中的内核程序kern-boot-params.c 说明了三种情况的使用。该程序列举了参数为一个整数、逗号分割的整数串以及字符串三种情况，读者要想测试该程序，需要把该程序拷贝到要使用的内核的源码目录树的一个目录下，为了避免与内核其他部分混淆，作者建议在内核源码树的根目录下创建一个新目录，如 examples，然后把该程序拷贝到 examples 目录下并重新命名为 setup_example.c，并且为该目录创建一个 Makefile 文件：
1
obj-y = setup_example.o
Makefile 仅许这一行就足够了，然后需要修改源码树的根目录下的 Makefile文件的一行，把下面行
1
core-y          := usr/
修改为
1
core-y          := usr/ examples/
注意：如果读者创建的新目录和重新命名的文件名与上面不同，需要修改上面所说 Makefile 文件相应的位置。 做完以上工作就可以按照内核构建步骤去构建新的内核，在构建好内核并设置好lilo或grub为该内核的启动条目后，就可以启动该内核，然后使用lilo或grub的编辑功能为该内核的启动参数行增加如下参数串：
setup_example_int=1234 setup_example_int_array=100,200,300,400 setup_example_string=Thisisatest
当然，该参数串也可以直接写入到lilo或grub的配置文件中对应于该新内核的内核命令行参数串中。读者可以使用其它参数值来测试该功能。
下面是作者系统上使用上面参数行的输出：
setup_example_int=1234
setup_example_int_array=100,200,300,400
setup_example_int_array includes 4 intergers
setup_example_string=Thisisatest
读者可以使用
1
dmesg | grep setup
来查看该程序的输出。
三、模块参数与sysfs
内核子系统或设备驱动可以直接编译到内核，也可以编译成模块，如果编译到内核，可以使用前一节介绍的方法通过内核启动参数来向它们传递参数，如果编译成模块，则可以通过命令行在插入模块时传递参数，或者在运行时，通过sysfs来设置或读取模块数据。
Sysfs是一个基于内存的文件系统，实际上它基于ramfs，sysfs提供了一种把内核数据结构，它们的属性以及属性与数据结构的联系开放给用户态的方式，它与kobject子系统紧密地结合在一起，因此内核开发者不需要直接使用它，而是内核的各个子系统使用它。用户要想使用 sysfs 读取和设置内核参数，仅需装载 sysfs 就可以通过文件操作应用来读取和设置内核通过 sysfs 开放给用户的各个参数：
$ mkdir -p /sysfs
$ mount -t sysfs sysfs /sysfs
注意，不要把 sysfs 和 sysctl 混淆，sysctl 是内核的一些控制参数，其目的是方便用户对内核的行为进行控制，而 sysfs 仅仅是把内核的 kobject 对象的层次关系与属性开放给用户查看，因此 sysfs 的绝大部分是只读的，模块作为一个 kobject 也被出口到 sysfs，模块参数则是作为模块属性出口的，内核实现者为模块的使用提供了更灵活的方式，允许用户设置模块参数在 sysfs 的可见性并允许用户在编写模块时设置这些参数在 sysfs 下的访问权限，然后用户就可以通过sysfs 来查看和设置模块参数，从而使得用户能在模块运行时控制模块行为。
对于模块而言，声明为 static 的变量都可以通过命令行来设置，但要想在 sysfs下可见，必须通过宏 module_param 来显式声明，该宏有三个参数，第一个为参数名，即已经定义的变量名，第二个参数则为变量类型，可用的类型有 byte, short, ushort, int, uint, long, ulong, charp 和 bool 或 invbool，分别对应于 c 类型 char, short, unsigned short, int, unsigned int, long, unsigned long, char * 和 int，用户也可以自定义类型 XXX（如果用户自己定义了 param_get_XXX，param_set_XXX 和 param_check_XXX）。该宏的第三个参数用于指定访问权限，如果为 0，该参数将不出现在 sysfs 文件系统中，允许的访问权限为 S_IRUSR， S_IWUSR，S_IRGRP，S_IWGRP，S_IROTH 和 S_IWOTH 的组合，它们分别对应于用户读，用户写，用户组读，用户组写，其他用户读和其他用户写，因此用文件的访问权限设置是一致的。
在源代码包中的内核模块 module-param-exam.c 是一个利用模块参数和sysfs来进行用户态与内核态数据交互的例子。该模块有三个参数可以通过命令行设置，下面是作者系统上的运行结果示例：
$ insmod ./module-param-exam.ko my_invisible_int=10 
 my_visible_int=20 mystring="Hello,World"
 my_invisible_int = 10
 my_visible_int = 20
 mystring = 'Hello,World'
 $ ls /sys/module/module_param_exam/parameters/
 mystring  my_visible_int
 $ cat /sys/module/module_param_exam/parameters/mystring
 Hello,World
 $ cat /sys/module/module_param_exam/parameters/my_visible_int
 20
 $ echo 2000 > /sys/module/module_param_exam/parameters/my_visible_int
 $ cat /sys/module/module_param_exam/parameters/my_visible_int
 2000
 $ echo "abc" > /sys/module/module_param_exam/parameters/mystring
 $ cat /sys/module/module_param_exam/parameters/mystring
 abc
 $ rmmod module_param_exam
 my_invisible_int = 10
 my_visible_int = 2000
 mystring = 'abc'
 四、sysctl
 Sysctl是一种用户应用来设置和获得运行时内核的配置参数的一种有效方式，通过这种方式，用户应用可以在内核运行的任何时刻来改变内核的配置参数，也可以在任何时候获得内核的配置参数，通常，内核的这些配置参数也出现在proc文件系统的/proc/sys目录下，用户应用可以直接通过这个目录下的文件来实现内核配置的读写操作，例如，用户可以通过
 1
 Cat /proc/sys/net/ipv4/ip_forward
 来得知内核IP层是否允许转发IP包，用户可以通过
 1
 echo 1 > /proc/sys/net/ipv4/ip_forward
 把内核 IP 层设置为允许转发 IP 包，即把该机器配置成一个路由器或网关。 一般地，所有的 Linux 发布也提供了一个系统工具 sysctl，它可以设置和读取内核的配置参数，但是该工具依赖于 proc 文件系统，为了使用该工具，内核必须支持 proc 文件系统。下面是使用 sysctl 工具来获取和设置内核配置参数的例子：
 $ sysctl net.ipv4.ip_forward
 net.ipv4.ip_forward = 0
 $ sysctl -w net.ipv4.ip_forward=1
 net.ipv4.ip_forward = 1
 $ sysctl net.ipv4.ip_forward
 net.ipv4.ip_forward = 1
 注意，参数 net.ipv4.ip_forward 实际被转换到对应的 proc 文件/proc/sys/net/ipv4/ip_forward，选项 -w 表示设置该内核配置参数，没有选项表示读内核配置参数，用户可以使用 sysctl -a 来读取所有的内核配置参数，对应更多的 sysctl 工具的信息，请参考手册页 sysctl(8)。
 但是 proc 文件系统对 sysctl 不是必须的，在没有 proc 文件系统的情况下，仍然可以，这时需要使用内核提供的系统调用 sysctl 来实现对内核配置参数的设置和读取。
 在源代码包中给出了一个实际例子程序，它说明了如何在内核和用户态使用sysctl。头文件 sysctl-exam.h 定义了 sysctl 条目 ID，用户态应用和内核模块需要这些 ID 来操作和注册 sysctl 条目。内核模块在文件 sysctl-exam-kern.c 中实现，在该内核模块中，每一个 sysctl 条目对应一个 struct ctl_table 结构，该结构定义了要注册的 sysctl 条目的 ID(字段 ctl_name)，在 proc 下的名称（字段procname），对应的内核变量（字段data，注意该该字段的赋值必须是指针），条目允许的最大长度（字段maxlen，它主要用于字符串内核变量，以便在对该条目设置时，对超过该最大长度的字符串截掉后面超长的部分），条目在proc文件系统下的访问权限（字段mode），在通过proc设置时的处理函数（字段proc_handler，对于整型内核变量，应当设置为&proc_dointvec，而对于字符串内核变量，则设置为 &proc_dostring），字符串处理策略（字段strategy，一般这是为&sysctl_string）。
 Sysctl 条目可以是目录，此时 mode 字段应当设置为 0555，否则通过 sysctl 系统调用将无法访问它下面的 sysctl 条目，child 则指向该目录条目下面的所有条目，对于在同一目录下的多个条目，不必一一注册，用户可以把它们组织成一个 struct ctl_table 类型的数组，然后一次注册就可以，但此时必须把数组的最后一个结构设置为NULL，即
 1
 2
 3
{
	    .ctl_name = 0
}
注册sysctl条目使用函数register_sysctl_table(struct ctl_table *, int)，第一个参数为定义的struct ctl_table结构的sysctl条目或条目数组指针，第二个参数为插入到sysctl条目表中的位置，如果插入到末尾，应当为0，如果插入到开头，则为非0。内核把所有的sysctl条目都组织成sysctl表。
当模块卸载时，需要使用函数unregister_sysctl_table(struct ctl_table_header *)解注册通过函数register_sysctl_table注册的sysctl条目，函数register_sysctl_table在调用成功时返回结构struct ctl_table_header，它就是sysctl表的表头，解注册函数使用它来卸载相应的sysctl条目。 用户态应用sysctl-exam-user.c通过sysctl系统调用来查看和设置前面内核模块注册的sysctl条目（当然如果用户的系统内核已经支持proc文件系统，可以直接使用文件操作应用如cat, echo等直接查看和设置这些sysctl条目）。
下面是作者运行该模块与应用的输出结果示例：
$ insmod ./sysctl-exam-kern.ko
$ cat /proc/sys/mysysctl/myint
0
$ cat /proc/sys/mysysctl/mystring
$ ./sysctl-exam-user
mysysctl.myint = 0
mysysctl.mystring = ""
$ ./sysctl-exam-user 100 "Hello, World"
old value: mysysctl.myint = 0
new value: mysysctl.myint = 100
old vale: mysysctl.mystring = ""
new value: mysysctl.mystring = "Hello, World"
$ cat /proc/sys/mysysctl/myint
100
$ cat /proc/sys/mysysctl/mystring
Hello, World
$
五、系统调用
系统调用是内核提供给应用程序的接口，应用对底层硬件的操作大部分都是通过调用系统调用来完成的，例如得到和设置系统时间，就需要分别调用 gettimeofday 和 settimeofday 来实现。事实上，所有的系统调用都涉及到内核与应用之间的数据交换，如文件系统操作函数 read 和 write，设置和读取网络协议栈的 setsockopt 和 getsockopt。本节并不是讲解如何增加新的系统调用，而是讲解如何利用现有系统调用来实现用户的数据传输需求。
一般地，用户可以建立一个伪设备来作为应用与内核之间进行数据交换的渠道，最通常的做法是使用伪字符设备，具体实现方法是：
1．定义对字符设备进行操作的必要函数并设置结构 struct file_operations
结构 struct file_operations 非常大，对于一般的数据交换需求，只定义 open, read, write, ioctl, mmap 和 release 函数就足够了，它们实际上对应于用户态的文件系统操作函数 open, read, write, ioctl, mmap 和 close。这些函数的原型示例如下：
ssize_t exam_read (struct file * file, char __user * buf, size_t count, 
		loff_t * ppos)
{
	…
}
ssize_t exam_write(struct file * file, const char __user * buf, size_t count, 
		 loff_t * ppos)
{
	…
}
int exam_ioctl(struct inode * inode, struct file * file, unsigned int cmd,
		  unsigned long argv)
{
	…
}
int exam_mmap(struct file *, struct vm_area_struct *)
{
	…
}
int exam_open(struct inode * inode, struct file * file)
{
	…
}
int exam_release(struct inode * inode, struct file * file)
{
	…
}
在定义了这些操作函数后需要定义并设置结构struct file_operations
struct file_operations exam_file_ops = {
	    .owner = THIS_MODULE,
		    .read = exam_read,
			    .write = exam_write,
				    .ioctl = exam_ioctl,
					    .mmap = exam_mmap,
						    .open = exam_open,
							    .release = exam_release,
};
2. 注册定义的伪字符设备并把它和上面的 struct file_operations 关联起来：
1
2
int exam_char_dev_major;
exam_char_dev_major = register_chrdev(0, "exam_char_dev", &exam_file_ops);
注意，函数 register_chrdev 的第一个参数如果为 0，表示由内核来确定该注册伪字符设备的主设备号，这是该函数的返回为实际分配的主设备号，如果返回小于 0，表示注册失败。因此，用户在使用该函数时必须判断返回值以便处理失败情况。为了使用该函数必须包含头文件 linux/fs.h。
在源代码包中给出了一个使用这种方式实现用户态与内核态数据交换的典型例子，它包含了三个文件： 头文件 syscall-exam.h 定义了 ioctl 命令，.c 文件 syscall-exam-user.c为用户态应用，它通过文件系统操作函数 mmap 和 ioctl 来与内核态模块交换数据，.c 文件 syscall-exam-kern.c 为内核模块，它实现了一个伪字符设备，以便与用户态应用进行数据交换。为了正确运行应用程序 syscall-exam-user，需要在插入模块 syscall-exam-kern 后创建该实现的伪字符设备，用户可以使用下面命令来正确创建设备：
1
$ mknod /dev/mychrdev c `dmesg | grep "char device mychrdev" | sed 's/.*major is //g'` 0
然后用户可以通过 cat 来读写 /dev/mychrdev，应用程序 syscall-exam-user则使用 mmap 来读数据并使用 ioctl 来得到该字符设备的信息以及裁减数据内容，它只是示例如何使用现有的系统调用来实现用户需要的数据交互操作。
下面是作者运行该模块的结果示例：
$ insmod ./syscall-exam-kern.ko
char device mychrdev is registered, major is 254
$ mknod /dev/mychrdev c `dmesg | grep "char device mychrdev" | sed 's/.*major is //g'` 0
$ cat /dev/mychrdev
$ echo "abcdefghijklmnopqrstuvwxyz" > /dev/mychrdev
$ cat /dev/mychrdev
abcdefghijklmnopqrstuvwxyz
$ ./syscall-exam-user
User process: syscall-exam-us(1433)
	Available space: 65509 bytes
	Data len: 27 bytes
	Offset in physical: cc0 bytes
	mychrdev content by mmap:
	abcdefghijklmnopqrstuvwxyz
	$ cat /dev/mychrdev
	abcde
	$
	六、netlink
	Netlink 是一种特殊的 socket，它是 Linux 所特有的，类似于 BSD 中的AF_ROUTE 但又远比它的功能强大，目前在最新的 Linux 内核（2.6.14）中使用netlink 进行应用与内核通信的应用很多，包括：路由 daemon（NETLINK_ROUTE），1-wire 子系统（NETLINK_W1），用户态 socket 协议（NETLINK_USERSOCK），防火墙（NETLINK_FIREWALL），socket 监视（NETLINK_INET_DIAG），netfilter 日志（NETLINK_NFLOG），ipsec 安全策略（NETLINK_XFRM），SELinux 事件通知（NETLINK_SELINUX），iSCSI 子系统（NETLINK_ISCSI），进程审计（NETLINK_AUDIT），转发信息表查询（NETLINK_FIB_LOOKUP），netlink connector(NETLINK_CONNECTOR),netfilter 子系统（NETLINK_NETFILTER），IPv6 防火墙（NETLINK_IP6_FW），DECnet 路由信息（NETLINK_DNRTMSG），内核事件向用户态通知（NETLINK_KOBJECT_UEVENT），通用 netlink（NETLINK_GENERIC）。
	Netlink 是一种在内核与用户应用间进行双向数据传输的非常好的方式，用户态应用使用标准的 socket API 就可以使用 netlink 提供的强大功能，内核态需要使用专门的内核 API 来使用 netlink。
	Netlink 相对于系统调用，ioctl 以及 /proc 文件系统而言具有以下优点：
	1，为了使用 netlink，用户仅需要在 include/linux/netlink.h 中增加一个新类型的 netlink 协议定义即可， 如 #define NETLINK_MYTEST 17 然后，内核和用户态应用就可以立即通过 socket API 使用该 netlink 协议类型进行数据交换。但系统调用需要增加新的系统调用，ioctl 则需要增加设备或文件， 那需要不少代码，proc 文件系统则需要在 /proc 下添加新的文件或目录，那将使本来就混乱的 /proc 更加混乱。
	2. netlink是一种异步通信机制，在内核与用户态应用之间传递的消息保存在socket缓存队列中，发送消息只是把消息保存在接收者的socket的接收队列，而不需要等待接收者收到消息，但系统调用与 ioctl 则是同步通信机制，如果传递的数据太长，将影响调度粒度。
	3．使用 netlink 的内核部分可以采用模块的方式实现，使用 netlink 的应用部分和内核部分没有编译时依赖，但系统调用就有依赖，而且新的系统调用的实现必须静态地连接到内核中，它无法在模块中实现，使用新系统调用的应用在编译时需要依赖内核。
	4．netlink 支持多播，内核模块或应用可以把消息多播给一个netlink组，属于该neilink 组的任何内核模块或应用都能接收到该消息，内核事件向用户态的通知机制就使用了这一特性，任何对内核事件感兴趣的应用都能收到该子系统发送的内核事件，在后面的文章中将介绍这一机制的使用。
	5．内核可以使用 netlink 首先发起会话，但系统调用和 ioctl 只能由用户应用发起调用。
	6．netlink 使用标准的 socket API，因此很容易使用，但系统调用和 ioctl则需要专门的培训才能使用。
	用户态使用 netlink
	用户态应用使用标准的socket APIs， socket(), bind(), sendmsg(), recvmsg() 和 close() 就能很容易地使用 netlink socket，查询手册页可以了解这些函数的使用细节，本文只是讲解使用 netlink 的用户应该如何使用这些函数。注意，使用 netlink 的应用必须包含头文件 linux/netlink.h。当然 socket 需要的头文件也必不可少，sys/socket.h。
	为了创建一个 netlink socket，用户需要使用如下参数调用 socket():
		1
		socket(AF_NETLINK, SOCK_RAW, netlink_type)
		第一个参数必须是 AF_NETLINK 或 PF_NETLINK，在 Linux 中，它们俩实际为一个东西，它表示要使用netlink，第二个参数必须是SOCK_RAW或SOCK_DGRAM， 第三个参数指定netlink协议类型，如前面讲的用户自定义协议类型NETLINK_MYTEST， NETLINK_GENERIC是一个通用的协议类型，它是专门为用户使用的，因此，用户可以直接使用它，而不必再添加新的协议类型。内核预定义的协议类型有：
#define NETLINK_ROUTE           0       /* Routing/device hook                          */
#define NETLINK_W1              1       /* 1-wire subsystem                             */
#define NETLINK_USERSOCK        2       /* Reserved for user mode socket protocols      */
#define NETLINK_FIREWALL        3       /* Firewalling hook                             */
#define NETLINK_INET_DIAG       4       /* INET socket monitoring                       */
#define NETLINK_NFLOG           5       /* netfilter/iptables ULOG */
#define NETLINK_XFRM            6       /* ipsec */
#define NETLINK_SELINUX         7       /* SELinux event notifications */
#define NETLINK_ISCSI           8       /* Open-iSCSI */
#define NETLINK_AUDIT           9       /* auditing */
#define NETLINK_FIB_LOOKUP      10
#define NETLINK_CONNECTOR       11
#define NETLINK_NETFILTER       12      /* netfilter subsystem */
#define NETLINK_IP6_FW          13
#define NETLINK_DNRTMSG         14      /* DECnet routing messages */
#define NETLINK_KOBJECT_UEVENT  15      /* Kernel messages to userspace */
#define NETLINK_GENERIC         16
		对于每一个netlink协议类型，可以有多达 32多播组，每一个多播组用一个位表示，netlink 的多播特性使得发送消息给同一个组仅需要一次系统调用，因而对于需要多拨消息的应用而言，大大地降低了系统调用的次数。
		函数 bind() 用于把一个打开的 netlink socket 与 netlink 源 socket 地址绑定在一起。netlink socket 的地址结构如下：
		struct sockaddr_nl
{
	  sa_family_t    nl_family;
	    unsigned short nl_pad;
		  __u32          nl_pid;
		    __u32          nl_groups;
};
字段 nl_family 必须设置为 AF_NETLINK 或着 PF_NETLINK，字段 nl_pad 当前没有使用，因此要总是设置为 0，字段 nl_pid 为接收或发送消息的进程的 ID，如果希望内核处理消息或多播消息，就把该字段设置为 0，否则设置为处理消息的进程 ID。字段 nl_groups 用于指定多播组，bind 函数用于把调用进程加入到该字段指定的多播组，如果设置为 0，表示调用者不加入任何多播组。
传递给 bind 函数的地址的 nl_pid 字段应当设置为本进程的进程 ID，这相当于 netlink socket 的本地地址。但是，对于一个进程的多个线程使用 netlink socket 的情况，字段 nl_pid 则可以设置为其它的值，如：
1
pthread_self() << 16 | getpid();
因此字段 nl_pid 实际上未必是进程 ID,它只是用于区分不同的接收者或发送者的一个标识，用户可以根据自己需要设置该字段。函数 bind 的调用方式如下：
1
bind(fd, (struct sockaddr*)&nladdr, sizeof(struct sockaddr_nl));
fd为前面的 socket 调用返回的文件描述符，参数 nladdr 为 struct sockaddr_nl 类型的地址。 为了发送一个 netlink 消息给内核或其他用户态应用，需要填充目标 netlink socket 地址 ，此时，字段 nl_pid 和 nl_groups 分别表示接收消息者的进程 ID 与多播组。如果字段 nl_pid 设置为 0，表示消息接收者为内核或多播组，如果 nl_groups为 0，表示该消息为单播消息，否则表示多播消息。 使用函数 sendmsg 发送 netlink 消息时还需要引用结构 struct msghdr、struct nlmsghdr 和 struct iovec，结构 struct msghdr 需如下设置：
1
2
3
4
struct msghdr msg;
memset(&msg, 0, sizeof(msg));
msg.msg_name = (void *)&(nladdr);
msg.msg_namelen = sizeof(nladdr);
其中 nladdr 为消息接收者的 netlink 地址。
struct nlmsghdr 为 netlink socket 自己的消息头，这用于多路复用和多路分解 netlink 定义的所有协议类型以及其它一些控制，netlink 的内核实现将利用这个消息头来多路复用和多路分解已经其它的一些控制，因此它也被称为netlink 控制块。因此，应用在发送 netlink 消息时必须提供该消息头。
struct nlmsghdr
{
	  __u32 nlmsg_len;   /* Length of message */
	    __u16 nlmsg_type;  /* Message type*/
		  __u16 nlmsg_flags; /* Additional flags */
		    __u32 nlmsg_seq;   /* Sequence number */
			  __u32 nlmsg_pid;   /* Sending process PID */
};
字段 nlmsg_len 指定消息的总长度，包括紧跟该结构的数据部分长度以及该结构的大小，字段 nlmsg_type 用于应用内部定义消息的类型，它对 netlink 内核实现是透明的，因此大部分情况下设置为 0，字段 nlmsg_flags 用于设置消息标志，可用的标志包括：
/* Flags values */
#define NLM_F_REQUEST           1       /* It is request message.       */
#define NLM_F_MULTI             2       /* Multipart message, terminated by NLMSG_DONE */
#define NLM_F_ACK               4       /* Reply with ack, with zero or error code */
#define NLM_F_ECHO              8       /* Echo this request            */
/* Modifiers to GET request */
#define NLM_F_ROOT      0x100   /* specify tree root    */
#define NLM_F_MATCH     0x200   /* return all matching  */
#define NLM_F_ATOMIC    0x400   /* atomic GET           */
#define NLM_F_DUMP      (NLM_F_ROOT|NLM_F_MATCH)
/* Modifiers to NEW request */
#define NLM_F_REPLACE   0x100   /* Override existing            */
#define NLM_F_EXCL      0x200   /* Do not touch, if it exists   */
#define NLM_F_CREATE    0x400   /* Create, if it does not exist */
#define NLM_F_APPEND    0x800   /* Add to end of list           */
标志NLM_F_REQUEST用于表示消息是一个请求，所有应用首先发起的消息都应设置该标志。
标志NLM_F_MULTI 用于指示该消息是一个多部分消息的一部分，后续的消息可以通过宏NLMSG_NEXT来获得。
宏NLM_F_ACK表示该消息是前一个请求消息的响应，顺序号与进程ID可以把请求与响应关联起来。
标志NLM_F_ECHO表示该消息是相关的一个包的回传。
标志NLM_F_ROOT 被许多 netlink 协议的各种数据获取操作使用，该标志指示被请求的数据表应当整体返回用户应用，而不是一个条目一个条目地返回。有该标志的请求通常导致响应消息设置NLM_F_MULTI标志。注意，当设置了该标志时，请求是协议特定的，因此，需要在字段 nlmsg_type 中指定协议类型。
标志 NLM_F_MATCH 表示该协议特定的请求只需要一个数据子集，数据子集由指定的协议特定的过滤器来匹配。
标志 NLM_F_ATOMIC 指示请求返回的数据应当原子地收集，这预防数据在获取期间被修改。
标志 NLM_F_DUMP 未实现。
标志 NLM_F_REPLACE 用于取代在数据表中的现有条目。
标志 NLM_F_EXCL_ 用于和 CREATE 和 APPEND 配合使用，如果条目已经存在，将失败。
标志 NLM_F_CREATE 指示应当在指定的表中创建一个条目。
标志 NLM_F_APPEND 指示在表末尾添加新的条目。
内核需要读取和修改这些标志，对于一般的使用，用户把它设置为 0 就可以，只是一些高级应用（如 netfilter 和路由 daemon 需要它进行一些复杂的操作），字段 nlmsg_seq 和 nlmsg_pid 用于应用追踪消息，前者表示顺序号，后者为消息来源进程 ID。下面是一个示例：
#define MAX_MSGSIZE 1024
char buffer[] = "An example message";
struct nlmsghdr nlhdr;
nlhdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSGSIZE));
strcpy(NLMSG_DATA(nlhdr),buffer);
nlhdr->nlmsg_len = NLMSG_LENGTH(strlen(buffer));
nlhdr->nlmsg_pid = getpid();  /* self pid */
nlhdr->nlmsg_flags = 0;
结构 struct iovec 用于把多个消息通过一次系统调用来发送，下面是该结构使用示例：
struct iovec iov;
iov.iov_base = (void *)nlhdr;
iov.iov_len = nlh->nlmsg_len;
msg.msg_iov = &iov;
msg.msg_iovlen = 1;
在完成以上步骤后，消息就可以通过下面语句直接发送：
1
sendmsg(fd, &msg, 0);
应用接收消息时需要首先分配一个足够大的缓存来保存消息头以及消息的数据部分，然后填充消息头，添完后就可以直接调用函数 recvmsg() 来接收。
#define MAX_NL_MSG_LEN 1024
struct sockaddr_nl nladdr;
struct msghdr msg;
struct iovec iov;
struct nlmsghdr * nlhdr;
nlhdr = (struct nlmsghdr *)malloc(MAX_NL_MSG_LEN);
iov.iov_base = (void *)nlhdr;
iov.iov_len = MAX_NL_MSG_LEN;
msg.msg_name = (void *)&(nladdr);
msg.msg_namelen = sizeof(nladdr);
msg.msg_iov = &iov;
msg.msg_iovlen = 1;
recvmsg(fd, &msg, 0);
注意：fd为socket调用打开的netlink socket描述符。
在消息接收后，nlhdr指向接收到的消息的消息头，nladdr保存了接收到的消息的目标地址，宏NLMSG_DATA(nlhdr)返回指向消息的数据部分的指针。
在linux/netlink.h中定义了一些方便对消息进行处理的宏，这些宏包括：
1
2
#define NLMSG_ALIGNTO   4
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )
宏NLMSG_ALIGN(len)用于得到不小于len且字节对齐的最小数值。
1
#define NLMSG_LENGTH(len) ((len)+NLMSG_ALIGN(sizeof(struct nlmsghdr)))
宏NLMSG_LENGTH(len)用于计算数据部分长度为len时实际的消息长度。它一般用于分配消息缓存。
1
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))
宏NLMSG_SPACE(len)返回不小于NLMSG_LENGTH(len)且字节对齐的最小数值，它也用于分配消息缓存。
1
#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
宏NLMSG_DATA(nlh)用于取得消息的数据部分的首地址，设置和读取消息数据部分时需要使用该宏。
1
2
#define NLMSG_NEXT(nlh,len)      ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
		                      (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
宏NLMSG_NEXT(nlh,len)用于得到下一个消息的首地址，同时len也减少为剩余消息的总长度，该宏一般在一个消息被分成几个部分发送或接收时使用。
1
2
3
#define NLMSG_OK(nlh,len) ((len) >= (int)sizeof(struct nlmsghdr) && \
		                           (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
		                           (nlh)->nlmsg_len <= (len))
宏NLMSG_OK(nlh,len)用于判断消息是否有len这么长。
1
#define NLMSG_PAYLOAD(nlh,len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))
宏NLMSG_PAYLOAD(nlh,len)用于返回payload的长度。
函数close用于关闭打开的netlink socket。
netlink内核API
netlink的内核实现在.c文件net/core/af_netlink.c中，内核模块要想使用netlink，也必须包含头文件linux/netlink.h。内核使用netlink需要专门的API，这完全不同于用户态应用对netlink的使用。如果用户需要增加新的netlink协议类型，必须通过修改linux/netlink.h来实现，当然，目前的netlink实现已经包含了一个通用的协议类型NETLINK_GENERIC以方便用户使用，用户可以直接使用它而不必增加新的协议类型。前面讲到，为了增加新的netlink协议类型，用户仅需增加如下定义到linux/netlink.h就可以：
1
#define NETLINK_MYTEST  17
只要增加这个定义之后，用户就可以在内核的任何地方引用该协议。
在内核中，为了创建一个netlink socket用户需要调用如下函数：
1
2
struct sock *
netlink_kernel_create(int unit, void (*input)(struct sock *sk, int len));
参数unit表示netlink协议类型，如NETLINK_MYTEST，参数input则为内核模块定义的netlink消息处理函数，当有消息到达这个netlink socket时，该input函数指针就会被引用。函数指针input的参数sk实际上就是函数netlink_kernel_create返回的struct sock指针，sock实际是socket的一个内核表示数据结构，用户态应用创建的socket在内核中也会有一个struct sock结构来表示。下面是一个input函数的示例：
void input (struct sock *sk, int len)
{
	 struct sk_buff *skb;
	  struct nlmsghdr *nlh = NULL;
	   u8 *data = NULL;
	    while ((skb = skb_dequeue(&sk->receive_queue)) 
				       != NULL) {
			 /* process netlink message pointed by skb->data */
			 nlh = (struct nlmsghdr *)skb->data;
			  data = NLMSG_DATA(nlh);
			   /* process netlink message with header pointed by 
				    * nlh and data pointed by data
					  */
			   }   
}
函数input()会在发送进程执行sendmsg()时被调用，这样处理消息比较及时，但是，如果消息特别长时，这样处理将增加系统调用sendmsg()的执行时间，对于这种情况，可以定义一个内核线程专门负责消息接收，而函数input的工作只是唤醒该内核线程，这样sendmsg将很快返回。
函数skb = skb_dequeue(&sk->receive_queue)用于取得socket sk的接收队列上的消息，返回为一个struct sk_buff的结构，skb->data指向实际的netlink消息。
函数skb_recv_datagram(nl_sk)也用于在netlink socket nl_sk上接收消息，与skb_dequeue的不同指出是，如果socket的接收队列上没有消息，它将导致调用进程睡眠在等待队列nl_sk->sk_sleep，因此它必须在进程上下文使用，刚才讲的内核线程就可以采用这种方式来接收消息。
下面的函数input就是这种使用的示例：
1
2
3
4
void input (struct sock *sk, int len)
{
	  wake_up_interruptible(sk->sk_sleep);
}
当内核中发送netlink消息时，也需要设置目标地址与源地址，而且内核中消息是通过struct sk_buff来管理的， linux/netlink.h中定义了一个宏：
1
#define NETLINK_CB(skb)         (*(struct netlink_skb_parms*)&((skb)->cb))
来方便消息的地址设置。下面是一个消息地址设置的例子：
1
2
3
NETLINK_CB(skb).pid = 0;
NETLINK_CB(skb).dst_pid = 0;
NETLINK_CB(skb).dst_group = 1;
字段pid表示消息发送者进程ID，也即源地址，对于内核，它为 0， dst_pid 表示消息接收者进程 ID，也即目标地址，如果目标为组或内核，它设置为 0，否则 dst_group 表示目标组地址，如果它目标为某一进程或内核，dst_group 应当设置为 0。
在内核中，模块调用函数 netlink_unicast 来发送单播消息：
1
int netlink_unicast(struct sock *sk, struct sk_buff *skb, u32 pid, int nonblock);
参数sk为函数netlink_kernel_create()返回的socket，参数skb存放消息，它的data字段指向要发送的netlink消息结构，而skb的控制块保存了消息的地址信息，前面的宏NETLINK_CB(skb)就用于方便设置该控制块， 参数pid为接收消息进程的pid，参数nonblock表示该函数是否为非阻塞，如果为1，该函数将在没有接收缓存可利用时立即返回，而如果为0，该函数在没有接收缓存可利用时睡眠。
内核模块或子系统也可以使用函数netlink_broadcast来发送广播消息：
1
2
void netlink_broadcast(struct sock *sk, struct sk_buff *skb, 
		      u32 pid, u32 group, int allocation);
前面的三个参数与netlink_unicast相同，参数group为接收消息的多播组，该参数的每一个代表一个多播组，因此如果发送给多个多播组，就把该参数设置为多个多播组组ID的位或。参数allocation为内核内存分配类型，一般地为GFP_ATOMIC或GFP_KERNEL，GFP_ATOMIC用于原子的上下文（即不可以睡眠），而GFP_KERNEL用于非原子上下文。
在内核中使用函数sock_release来释放函数netlink_kernel_create()创建的netlink socket：
1
void sock_release(struct socket * sock);
注意函数netlink_kernel_create()返回的类型为struct sock，因此函数sock_release应该这种调用：
1
sock_release(sk->sk_socket);
sk为函数netlink_kernel_create()的返回值。
在源代码包中给出了一个使用 netlink 的示例，它包括一个内核模块 netlink-exam-kern.c 和两个应用程序 netlink-exam-user-recv.c, netlink-exam-user-send.c。内核模块必须先插入到内核，然后在一个终端上运行用户态接收程序，在另一个终端上运行用户态发送程序，发送程序读取参数指定的文本文件并把它作为 netlink 消息的内容发送给内核模块，内核模块接受该消息保存到内核缓存中，它也通过proc接口出口到 procfs，因此用户也能够通过 /proc/netlink_exam_buffer 看到全部的内容，同时内核也把该消息发送给用户态接收程序，用户态接收程序将把接收到的内容输出到屏幕上。
小结
本文是系列文章的第一篇，它详细介绍了五种用户空间与内核空间的数据交换方式，并通过实际例子程序向读者讲解了如何在内核开发中使用这些技术，其中内核启动参数方式是单向的，即只能向内核传递，而不能从内核获取，其余的均可以进行双向数据交换，即既可以从用户应用传递给内核，有可以从内核传递给应用态应用。netlink 是一种双向的数据交换方式，它使用起来非常简单高效，特别是它的广播特性在一些应用中非常方便。作者认为，它是所有这些用户态与内核态数据交换方式中最有效的最强大的方式。




一、procfs
procfs是比较老的一种用户态与内核态的数据交换方式，内核的很多数据都是通过这种方式出口给用户的，内核的很多参数也是通过这种方式来让用户方便设置的。除了sysctl出口到/proc下的参数，procfs提供的大部分内核参数是只读的。实际上，很多应用严重地依赖于procfs，因此它几乎是必不可少的组件。前面部分的几个例子实际上已经使用它来出口内核数据，但是并没有讲解如何使用，本节将讲解如何使用procfs。
Procfs提供了如下API：
1
2
struct proc_dir_entry *create_proc_entry(const char *name, mode_t mode,
		                                          struct proc_dir_entry *parent)
	该函数用于创建一个正常的proc条目，参数name给出要建立的proc条目的名称，参数mode给出了建立的该proc条目的访问权限，参数parent指定建立的proc条目所在的目录。如果要在/proc下建立proc条目，parent应当为NULL。否则它应当为proc_mkdir返回的struct proc_dir_entry结构的指针。
	1
	extern void remove_proc_entry(const char *name, struct proc_dir_entry *parent)
	该函数用于删除上面函数创建的proc条目，参数name给出要删除的proc条目的名称，参数parent指定建立的proc条目所在的目录。
	1
	struct proc_dir_entry *proc_mkdir(const char * name, struct proc_dir_entry *parent)
	该函数用于创建一个proc目录，参数name指定要创建的proc目录的名称，参数parent为该proc目录所在的目录。
	1
	2
	3
	4
	extern struct proc_dir_entry *proc_mkdir_mode(const char *name, mode_t mode,
			                        struct proc_dir_entry *parent);
struct proc_dir_entry *proc_symlink(const char * name,
		                struct proc_dir_entry * parent, const char * dest)
	该函数用于建立一个proc条目的符号链接，参数name给出要建立的符号链接proc条目的名称，参数parent指定符号连接所在的目录，参数dest指定链接到的proc条目名称。
	1
	2
	3
	struct proc_dir_entry *create_proc_read_entry(const char *name,
			        mode_t mode, struct proc_dir_entry *base,
					        read_proc_t *read_proc, void * data)
	该函数用于建立一个规则的只读proc条目，参数name给出要建立的proc条目的名称，参数mode给出了建立的该proc条目的访问权限，参数base指定建立的proc条目所在的目录，参数read_proc给出读去该proc条目的操作函数，参数data为该proc条目的专用数据，它将保存在该proc条目对应的struct file结构的private_data字段中。
	1
	2
	struct proc_dir_entry *create_proc_info_entry(const char *name,
			        mode_t mode, struct proc_dir_entry *base, get_info_t *get_info)
	该函数用于创建一个info型的proc条目，参数name给出要建立的proc条目的名称，参数mode给出了建立的该proc条目的访问权限，参数base指定建立的proc条目所在的目录，参数get_info指定该proc条目的get_info操作函数。实际上get_info等同于read_proc，如果proc条目没有定义个read_proc，对该proc条目的read操作将使用get_info取代，因此它在功能上非常类似于函数create_proc_read_entry。
	1
	2
	struct proc_dir_entry *proc_net_create(const char *name,
			        mode_t mode, get_info_t *get_info)
	该函数用于在/proc/net目录下创建一个proc条目，参数name给出要建立的proc条目的名称，参数mode给出了建立的该proc条目的访问权限，参数get_info指定该proc条目的get_info操作函数。
	1
	2
	struct proc_dir_entry *proc_net_fops_create(const char *name,
			        mode_t mode, struct file_operations *fops)
	该函数也用于在/proc/net下创建proc条目，但是它也同时指定了对该proc条目的文件操作函数。
	1
	void proc_net_remove(const char *name)
	该函数用于删除前面两个函数在/proc/net目录下创建的proc条目。参数name指定要删除的proc名称。
	除了这些函数，值得一提的是结构struct proc_dir_entry，为了创建一了可写的proc条目并指定该proc条目的写操作函数，必须设置上面的这些创建proc条目的函数返回的指针指向的struct proc_dir_entry结构的write_proc字段，并指定该proc条目的访问权限有写权限。
	为了使用这些接口函数以及结构struct proc_dir_entry，用户必须在模块中包含头文件linux/proc_fs.h。
	在源代码包中给出了procfs示例程序procfs_exam.c，它定义了三个proc文件条目和一个proc目录条目，读者在插入该模块后应当看到如下结构：
	1
	2
	3
	$ ls /proc/myproctest
	aint        astring     bigprocfile
	$
	读者可以通过cat和echo等文件操作函数来查看和设置这些proc文件。特别需要指出，bigprocfile是一个大文件（超过一个内存页），对于这种大文件，procfs有一些限制，因为它提供的缓存，只有一个页，因此必须特别小心，并对超过页的部分做特别的考虑，处理起来比较复杂并且很容易出错，所有procfs并不适合于大数据量的输入输出，后面一节seq_file就是因为这一缺陷而设计的，当然seq_file依赖于procfs的一些基础功能。
	二、seq_file
	一般地，内核通过在procfs文件系统下建立文件来向用户空间提供输出信息，用户空间可以通过任何文本阅读应用查看该文件信息，但是procfs有一个缺陷，如果输出内容大于1个内存页，需要多次读，因此处理起来很难，另外，如果输出太大，速度比较慢，有时会出现一些意想不到的情况，Alexander Viro实现了一套新的功能，使得内核输出大文件信息更容易，该功能出现在2.4.15（包括2.4.15）以后的所有2.4内核以及2.6内核中，尤其是在2.6内核中，已经大量地使用了该功能。
	要想使用seq_file功能，开发者需要包含头文件linux/seq_file.h，并定义与设置一个seq_operations结构（类似于file_operations结构）:
	struct seq_operations {
		        void * (*start) (struct seq_file *m, loff_t *pos);
				        void (*stop) (struct seq_file *m, void *v);
						        void * (*next) (struct seq_file *m, void *v, loff_t *pos);
								        int (*show) (struct seq_file *m, void *v);
	};
start函数用于指定seq_file文件的读开始位置，返回实际读开始位置，如果指定的位置超过文件末尾，应当返回NULL，start函数可以有一个特殊的返回SEQ_START_TOKEN，它用于让show函数输出文件头，但这只能在pos为0时使用，next函数用于把seq_file文件的当前读位置移动到下一个读位置，返回实际的下一个读位置，如果已经到达文件末尾，返回NULL，stop函数用于在读完seq_file文件后调用，它类似于文件操作close，用于做一些必要的清理，如释放内存等，show函数用于格式化输出，如果成功返回0，否则返回出错码。
Seq_file也定义了一些辅助函数用于格式化输出：
1
int seq_putc(struct seq_file *m, char c);
函数seq_putc用于把一个字符输出到seq_file文件。
1
int seq_puts(struct seq_file *m, const char *s);
函数seq_puts则用于把一个字符串输出到seq_file文件。
1
int seq_escape(struct seq_file *, const char *, const char *);
函数seq_escape类似于seq_puts，只是，它将把第一个字符串参数中出现的包含在第二个字符串参数中的字符按照八进制形式输出，也即对这些字符进行转义处理。
1
2
int seq_printf(struct seq_file *, const char *, ...)
	        __attribute__ ((format (printf,2,3)));
			函数seq_printf是最常用的输出函数，它用于把给定参数按照给定的格式输出到seq_file文件。
			1
			int seq_path(struct seq_file *, struct vfsmount *, struct dentry *, char *);
			函数seq_path则用于输出文件名，字符串参数提供需要转义的文件名字符，它主要供文件系统使用。
			在定义了结构struct seq_operations之后，用户还需要把打开seq_file文件的open函数，以便该结构与对应于seq_file文件的struct file结构关联起来，例如，struct seq_operations定义为：
			struct seq_operations exam_seq_ops = {
				    .start = exam_seq_start,
					   .stop = exam_seq_stop,
					      .next = exam_seq_next,
						     .show = exam_seq_show
			};
那么，open函数应该如下定义：
static int exam_seq_open(struct inode *inode, struct file *file)
{
	        return seq_open(file, &exam_seq_ops);
};
注意，函数seq_open是seq_file提供的函数，它用于把struct seq_operations结构与seq_file文件关联起来。 最后，用户需要如下设置struct file_operations结构：
struct file_operations exam_seq_file_ops = {
	        .owner   = THIS_MODULE,
			        .open    = exm_seq_open,
					        .read    = seq_read,
							        .llseek  = seq_lseek,
									        .release = seq_release
};
注意，用户仅需要设置open函数，其它的都是seq_file提供的函数。
然后，用户创建一个/proc文件并把它的文件操作设置为exam_seq_file_ops即可：
struct proc_dir_entry *entry;
entry = create_proc_entry("exam_seq_file", 0, NULL);
if (entry)
	entry->proc_fops = &exam_seq_file_ops;
	对于简单的输出，seq_file用户并不需要定义和设置这么多函数与结构，它仅需定义一个show函数，然后使用single_open来定义open函数就可以，以下是使用这种简单形式的一般步骤：
	1．定义一个show函数
	int exam_show(struct seq_file *p, void *v)
{
	…
}
2. 定义open函数
int exam_single_open(struct inode *inode, struct file *file)
{
	        return(single_open(file, exam_show, NULL));
}
注意要使用single_open而不是seq_open。
3. 定义struct file_operations结构
struct file_operations exam_single_seq_file_operations = {
	        .open           = exam_single_open,
			        .read           = seq_read,
					        .llseek         = seq_lseek,
							        .release        = single_release,
};
注意，如果open函数使用了single_open，release函数必须为single_release，而不是seq_release。 在源代码包中给出了一个使用seq_file的具体例子seqfile_exam.c，它使用seq_file提供了一个查看当前系统运行的所有进程的/proc接口，在编译并插入该模块后，用户通过命令"cat /proc/ exam_esq_file"可以查看系统的所有进程。
三、debugfs
内核开发者经常需要向用户空间应用输出一些调试信息，在稳定的系统中可能根本不需要这些调试信息，但是在开发过程中，为了搞清楚内核的行为，调试信息非常必要，printk可能是用的最多的，但它并不是最好的，调试信息只是在开发中用于调试，而printk将一直输出，因此开发完毕后需要清除不必要的printk语句，另外如果开发者希望用户空间应用能够改变内核行为时，printk就无法实现。因此，需要一种新的机制，那只有在需要的时候使用，它在需要时通过在一个虚拟文件系统中创建一个或多个文件来向用户空间应用提供调试信息。
有几种方式可以实现上述要求：
使用procfs，在/proc创建文件输出调试信息，但是procfs对于大于一个内存页（对于x86是4K）的输出比较麻烦，而且速度慢，有时回出现一些意想不到的问题。
使用sysfs（2.6内核引入的新的虚拟文件系统），在很多情况下，调试信息可以存放在那里，但是sysfs主要用于系统管理，它希望每一个文件对应内核的一个变量，如果使用它输出复杂的数据结构或调试信息是非常困难的。
使用libfs创建一个新的文件系统，该方法极其灵活，开发者可以为新文件系统设置一些规则，使用libfs使得创建新文件系统更加简单，但是仍然超出了一个开发者的想象。
为了使得开发者更加容易使用这样的机制，Greg Kroah-Hartman开发了debugfs（在2.6.11中第一次引入），它是一个虚拟文件系统，专门用于输出调试信息，该文件系统非常小，很容易使用，可以在配置内核时选择是否构件到内核中，在不选择它的情况下，使用它提供的API的内核部分不需要做任何改动。
使用debugfs的开发者首先需要在文件系统中创建一个目录，下面函数用于在debugfs文件系统下创建一个目录：
1
struct dentry *debugfs_create_dir(const char *name, struct dentry *parent);
参数name是要创建的目录名，参数parent指定创建目录的父目录的dentry，如果为NULL，目录将创建在debugfs文件系统的根目录下。如果返回为-ENODEV，表示内核没有把debugfs编译到其中，如果返回为NULL，表示其他类型的创建失败，如果创建目录成功，返回指向该目录对应的dentry条目的指针。
下面函数用于在debugfs文件系统中创建一个文件：
struct dentry *debugfs_create_file(const char *name, mode_t mode,
		                               struct dentry *parent, void *data,
									                                  struct file_operations *fops);
参数name指定要创建的文件名，参数mode指定该文件的访问许可，参数parent指向该文件所在目录，参数data为该文件特定的一些数据，参数fops为实现在该文件上进行文件操作的fiel_operations结构指针，在很多情况下，由seq_file（前面章节已经讲过）提供的文件操作实现就足够了，因此使用debugfs很容易，当然，在一些情况下，开发者可能仅需要使用用户应用可以控制的变量来调试，debugfs也提供了4个这样的API方便开发者使用：
    struct dentry *debugfs_create_u8(const char *name, mode_t mode, 
			                                     struct dentry *parent, u8 *value);
    struct dentry *debugfs_create_u16(const char *name, mode_t mode, 
			                                      struct dentry *parent, u16 *value);
    struct dentry *debugfs_create_u32(const char *name, mode_t mode, 
			                                      struct dentry *parent, u32 *value);
    struct dentry *debugfs_create_bool(const char *name, mode_t mode, 
			struct dentry *parent, u32 *value);
参数name和mode指定文件名和访问许可，参数value为需要让用户应用控制的内核变量指针。
当内核模块卸载时，Debugfs并不会自动清除该模块创建的目录或文件，因此对于创建的每一个文件或目录，开发者必须调用下面函数清除：
1
void debugfs_remove(struct dentry *dentry);
参数dentry为上面创建文件和目录的函数返回的dentry指针。
在源代码包中给出了一个使用debufs的示例模块debugfs_exam.c，为了保证该模块正确运行，必须让内核支持debugfs，debugfs是一个调试功能，因此它位于主菜单Kernel hacking，并且必须选择Kernel debugging选项才能选择，它的选项名称为Debug Filesystem。为了在用户态使用debugfs，用户必须mount它，下面是在作者系统上的使用输出：
$ mkdir -p /debugfs
$ mount -t debugfs debugfs /debugfs
$ insmod ./debugfs_exam.ko
$ ls /debugfs
debugfs-exam
$ ls /debugfs/debugfs-exam
u8_var      u16_var     u32_var     bool_var
$ cd /debugfs/debugfs-exam
$ cat u8_var
0
$ echo 200 > u8_var
$ cat u8_var
200
$ cat bool_var
N
$ echo 1 > bool_var
$ cat bool_var
Y
四、relayfs
relayfs是一个快速的转发（relay）数据的文件系统，它以其功能而得名。它为那些需要从内核空间转发大量数据到用户空间的工具和应用提供了快速有效的转发机制。
Channel是relayfs文件系统定义的一个主要概念，每一个channel由一组内核缓存组成，每一个CPU有一个对应于该channel的内核缓存，每一个内核缓存用一个在relayfs文件系统中的文件文件表示，内核使用relayfs提供的写函数把需要转发给用户空间的数据快速地写入当前CPU上的channel内核缓存，用户空间应用通过标准的文件I/O函数在对应的channel文件中可以快速地取得这些被转发出的数据mmap来。写入到channel中的数据的格式完全取决于内核中创建channel的模块或子系统。
relayfs的用户空间API：
relayfs实现了四个标准的文件I/O函数，open、mmap、poll和close
open()，打开一个channel在某一个CPU上的缓存对应的文件。
mmap()，把打开的channel缓存映射到调用者进程的内存空间。
read()，读取channel缓存，随后的读操作将看不到被该函数消耗的字节，如果channel的操作模式为非覆盖写，那么用户空间应用在有内核模块写时仍可以读取，但是如果channel的操作模式为覆盖式，那么在读操作期间如果有内核模块进行写，结果将无法预知，因此对于覆盖式写的channel，用户应当在确认在channel的写完全结束后再进行读。
poll()，用于通知用户空间应用转发数据跨越了子缓存的边界，支持的轮询标志有POLLIN、POLLRDNORM和POLLERR。
close()，关闭open函数返回的文件描述符，如果没有进程或内核模块打开该channel缓存，close函数将释放该channel缓存。
注意：用户态应用在使用上述API时必须保证已经挂载了relayfs文件系统，但内核在创建和使用channel时不需要relayfs已经挂载。下面命令将把relayfs文件系统挂载到/mnt/relay。
1
mount -t relayfs relayfs /mnt/relay
relayfs内核API：
relayfs提供给内核的API包括四类：channel管理、写函数、回调函数和辅助函数。
Channel管理函数包括：
relay_open(base_filename, parent, subbuf_size, n_subbufs, overwrite, callbacks)
	relay_close(chan)
	relay_flush(chan)
	relay_reset(chan)
	relayfs_create_dir(name, parent)
	relayfs_remove_dir(dentry)
	relay_commit(buf, reserved, count)
	relay_subbufs_consumed(chan, cpu, subbufs_consumed)
	写函数包括：
	relay_write(chan, data, length)
	__relay_write(chan, data, length)
	relay_reserve(chan, length)
	回调函数包括：
	subbuf_start(buf, subbuf, prev_subbuf_idx, prev_subbuf)
	buf_mapped(buf, filp)
	buf_unmapped(buf, filp)
	辅助函数包括：
	relay_buf_full(buf)
	subbuf_start_reserve(buf, length)
	前面已经讲过，每一个channel由一组channel缓存组成，每个CPU对应一个该channel的缓存，每一个缓存又由一个或多个子缓存组成，每一个缓存是子缓存组成的一个环型缓存。
	函数relay_open用于创建一个channel并分配对应于每一个CPU的缓存，用户空间应用通过在relayfs文件系统中对应的文件可以访问channel缓存，参数base_filename用于指定channel的文件名，relay_open函数将在relayfs文件系统中创建base_filename0..base_filenameN-1，即每一个CPU对应一个channel文件，其中N为CPU数，缺省情况下，这些文件将建立在relayfs文件系统的根目录下，但如果参数parent非空，该函数将把channel文件创建于parent目录下，parent目录使用函数relay_create_dir创建，函数relay_remove_dir用于删除由函数relay_create_dir创建的目录，谁创建的目录，谁就负责在不用时负责删除。参数subbuf_size用于指定channel缓存中每一个子缓存的大小，参数n_subbufs用于指定channel缓存包含的子缓存数，因此实际的channel缓存大小为(subbuf_size x n_subbufs)，参数overwrite用于指定该channel的操作模式，relayfs提供了两种写模式，一种是覆盖式写，另一种是非覆盖式写。使用哪一种模式完全取决于函数subbuf_start的实现，覆盖写将在缓存已满的情况下无条件地继续从缓存的开始写数据，而不管这些数据是否已经被用户应用读取，因此写操作决不失败。在非覆盖写模式下，如果缓存满了，写将失败，但内核将在用户空间应用读取缓存数据时通过函数relay_subbufs_consumed()通知relayfs。如果用户空间应用没来得及消耗缓存中的数据或缓存已满，两种模式都将导致数据丢失，唯一的区别是，前者丢失数据在缓存开头，而后者丢失数据在缓存末尾。一旦内核再次调用函数relay_subbufs_consumed()，已满的缓存将不再满，因而可以继续写该缓存。当缓存满了以后，relayfs将调用回调函数buf_full()来通知内核模块或子系统。当新的数据太大无法写入当前子缓存剩余的空间时，relayfs将调用回调函数subbuf_start()来通知内核模块或子系统将需要使用新的子缓存。内核模块需要在该回调函数中实现下述功能：
	初始化新的子缓存；
	如果1正确，完成当前子缓存；
	如果2正确，返回是否正确完成子缓存切换；
	在非覆盖写模式下，回调函数subbuf_start()应该如下实现：
	static int subbuf_start(struct rchan_buf *buf,
			                        void *subbuf,
									            void *prev_subbuf,
												            unsigned int prev_padding)
{
	    if (prev_subbuf)
			        *((unsigned *)prev_subbuf) = prev_padding;
		    if (relay_buf_full(buf))
				        return 0;
			    subbuf_start_reserve(buf, sizeof(unsigned int));
				    return 1;
}
如果当前缓存满，即所有的子缓存都没读取，该函数返回0，指示子缓存切换没有成功。当子缓存通过函数relay_subbufs_consumed()被读取后，读取者将负责通知relayfs，函数relay_buf_full()在已经有读者读取子缓存数据后返回0，在这种情况下，子缓存切换成功进行。
在覆盖写模式下， subbuf_start()的实现与非覆盖模式类似：
static int subbuf_start(struct rchan_buf *buf,
		                        void *subbuf,
								            void *prev_subbuf,
											            unsigned int prev_padding)
{
	    if (prev_subbuf)
			        *((unsigned *)prev_subbuf) = prev_padding;
		    subbuf_start_reserve(buf, sizeof(unsigned int));
			    return 1;
}
只是不做relay_buf_full()检查，因为此模式下，缓存是环行的，可以无条件地写。因此在此模式下，子缓存切换必定成功，函数relay_subbufs_consumed() 也无须调用。如果channel写者没有定义subbuf_start()，缺省的实现将被使用。 可以通过在回调函数subbuf_start()中调用辅助函数subbuf_start_reserve()在子缓存中预留头空间，预留空间可以保存任何需要的信息，如上面例子中，预留空间用于保存子缓存填充字节数，在subbuf_start()实现中，前一个子缓存的填充值被设置。前一个子缓存的填充值和指向前一个子缓存的指针一道作为subbuf_start()的参数传递给subbuf_start()，只有在子缓存完成后，才能知道填充值。subbuf_start()也被在channel创建时分配每一个channel缓存的第一个子缓存时调用，以便预留头空间，但在这种情况下，前一个子缓存指针为NULL。
内核模块使用函数relay_write()或__relay_write()往channel缓存中写需要转发的数据，它们的区别是前者失效了本地中断，而后者只抢占失效，因此前者可以在任何内核上下文安全使用，而后者应当在没有任何中断上下文将写channel缓存的情况下使用。这两个函数没有返回值，因此用户不能直接确定写操作是否失败，在缓存满且写模式为非覆盖模式时，relayfs将通过回调函数buf_full来通知内核模块。
函数relay_reserve()用于在channel缓存中预留一段空间以便以后写入，在那些没有临时缓存而直接写入channel缓存的内核模块可能需要该函数，使用该函数的内核模块在实际写这段预留的空间时可以通过调用relay_commit()来通知relayfs。当所有预留的空间全部写完并通过relay_commit通知relayfs后，relayfs将调用回调函数deliver()通知内核模块一个完整的子缓存已经填满。由于预留空间的操作并不在写channel的内核模块完全控制之下，因此relay_reserve()不能很好地保护缓存，因此当内核模块调用relay_reserve()时必须采取恰当的同步机制。
当内核模块结束对channel的使用后需要调用relay_close() 来关闭channel，如果没有任何用户在引用该channel，它将和对应的缓存全部被释放。
函数relay_flush()强制在所有的channel缓存上做一个子缓存切换，它在channel被关闭前使用来终止和处理最后的子缓存。
函数relay_reset()用于将一个channel恢复到初始状态，因而不必释放现存的内存映射并重新分配新的channel缓存就可以使用channel，但是该调用只有在该channel没有任何用户在写的情况下才可以安全使用。
回调函数buf_mapped() 在channel缓存被映射到用户空间时被调用。
回调函数buf_unmapped()在释放该映射时被调用。内核模块可以通过它们触发一些内核操作，如开始或结束channel写操作。
在源代码包中给出了一个使用relayfs的示例程序relayfs_exam.c，它只包含一个内核模块，对于复杂的使用，需要应用程序配合。该模块实现了类似于文章中seq_file示例实现的功能。
当然为了使用relayfs，用户必须让内核支持relayfs，并且要mount它，下面是作者系统上的使用该模块的输出信息：
$ mkdir -p /relayfs
$ insmod ./relayfs-exam.ko
$ mount -t relayfs relayfs /relayfs
$ cat /relayfs/example0
…
$
relayfs是一种比较复杂的内核态与用户态的数据交换方式，本例子程序只提供了一个较简单的使用方式，对于复杂的使用，请参考relayfs用例页面http://relayfs.sourceforge.net/examples.html。
小结
本文是该系列文章最后一篇，它详细地讲解了其余四种用户空间与内核空间的数据交换方式，并通过实际例子程序向读者讲解了如何在内核开发中使用这些技术，其中seq_file是单向的，即只能向内核传递，而不能从内核获取，而另外三种方式均可以进行双向数据交换，即既可以从用户应用传递给内核，又可以从内核传递给应用态应用。procfs一般用于向用户出口少量的数据信息，或用户通过它设置内核变量从而控制内核行为。seq_file实际上依赖于procfs，因此为了使用seq_file，必须使内核支持procfs。debugfs用于内核开发者调试使用，它比其他集中方式都方便，但是仅用于简单类型的变量处理。relayfs是一种非常复杂的数据交换方式，要想准确使用并不容易，但是如果使用得当，它远比procfs和seq_file功能强大。
