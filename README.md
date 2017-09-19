# lwip无操作系统移植

## 编译过程

1. 编译，缺少lwip/opt.h，加入头文件目录：$(ProjectDir)include
2. 编译，缺少lwipopts.h，网上查找，该文件是别人修改的常用设置文件，自建，设空。放到custom目录下。
3. 编译，缺少arch/cc.h文件。按照sys_arch.txt和网上的代码，在include/arch 目录下创建cc.h文件。
4. 编译，缺少lwip/ip_addr.h。加入头文件目录：$(ProjectDir)include/ipv4，$(ProjectDir)include/ipv6。
5. 编译，缺少arch/sys_arch.h。按照sys_arch.txt和网上的代码，在include/arch 目录下创建sys_arch.h文件。
6. 编译，报奇怪的错误：sys.h(113) : error C2143: 语法错误 : 缺少“)”(在“*”的前面)。点击查看位置，发现sys_mutex_t未定义，查找其他代码，发现需要开启LWIP_COMPAT_MUTEX，在sys_arch.h中定义该宏即可。
7. 编译，提示 fatal error C1189: #error :  "BYTE_ORDER is not defined, you have to define it in your cc.h"，在cc.h定义BYTE_ORDER。#define BYTE_ORDER LITTLE_ENDIAN 。
8. 编译，提示未找到arch/perf.h。按照sys_arch.txt和网上的代码，在include/arch 目录下创建perf.h文件。
9. 编译，提示icmp6.c(61) error C2065: “ICMP6_ECHO”: 未声明的标识符。暂时用不到ip6，移除它。
10. 编译，提示链接错误和缺少main函数，证明已经可以编译通过，只需要实现那几个函数即可。


无操作系统版本

11. 开启NO_SYS， 无法解析的外部符号 _sys_now，该符号在函数 _sys_timeouts_init 中被引用 ，需要实现sys_now函数，定义sys_arch.c文件，实现该函数。

12. 修改ethernetif.c文件，将#if 0 改为 #if 1，完善这些函数。

   12.1 修改low_level_init函数内部，设置MAC地址。
   12.2 修改low_level_output函数内部，实现底层发送数据程序。

## 代码Demo

https://github.com/WangZhechao/win32-lwip