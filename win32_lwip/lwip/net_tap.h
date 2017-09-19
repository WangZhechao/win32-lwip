#ifndef __PCAP_IF_H__
#define __PCAP_IF_H__

#include "lwip/err.h"

#ifdef __cplusplus
extern "C" {
#endif
	extern err_t
		ethernetif_init(struct netif *netif);

	extern void  ethernetif_input(struct netif *netif);

	err_t open_tap();
	err_t close_tap();
	err_t get_packet(unsigned int* len, char** pkt_data);
	err_t send_packet(char** buf, int len);

#ifdef __cplusplus
}
#endif

#endif