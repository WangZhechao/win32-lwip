#include <Windows.h>
#include "lwip/init.h"
#include "lwip/netif.h" 
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "lwip/timers.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "net_tap.h"
#include "client.h"

struct ip_addr ip, mask, gw;
static struct netif netif;


void init()
{
	lwip_init();

	IP4_ADDR(&ip, 192, 168, 1, 189);
	IP4_ADDR(&mask, 255, 255, 255, 0);
	IP4_ADDR(&gw, 192, 168, 1, 1);

	netif_add(&netif, &ip, &mask, &gw, NULL, ethernetif_init, ethernet_input);
	netif_set_default(&netif);
	netif_set_up(&netif);
}


int main()
{
	if(ERR_IF == open_tap()) {
		exit(1);
	}

	init();

	if(client_init() < 0) {
		close_tap();
		exit(1);
	}

	while(true)
	{
		//取数据，投递到lwip协议栈
		ethernetif_input(&netif);

		sys_check_timeouts();
	}


	close_tap();

	return 0;
}