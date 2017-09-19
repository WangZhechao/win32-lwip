#include "client.h"

#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"

static struct tcp_pcb *client_pcb;
static ip_addr remote_addr = {0};
static u16_t remote_port = 0;


enum client_states
{
	ES_NONE = 0,
	ES_BINDED,
	ES_CONNECTED,
	ES_RECEIVED,
	ES_CLOSING
};


struct client_t
{
	u8_t state;
	u8_t retries;
	struct tcp_pcb *pcb;
	/* pbuf (chain) to recycle */
	struct pbuf *p;
};

void client_close(struct tcp_pcb *tpcb, struct client_t *cs);
err_t client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
void client_send(struct tcp_pcb *tpcb, struct client_t *cs);
err_t client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void client_error(void *arg, err_t err);
err_t client_poll(void *arg, struct tcp_pcb *tpcb);
err_t client_connect();
int client_init(void);


void client_close(struct tcp_pcb *tpcb, struct client_t *cs)
{
	tcp_arg(tpcb, NULL);
	tcp_sent(tpcb, NULL);
	tcp_recv(tpcb, NULL);
	tcp_err(tpcb, NULL);
	tcp_poll(tpcb, NULL, 0);

	if (cs != NULL)
	{
		mem_free(cs);
	}  
	tcp_close(tpcb);

	printf("called client_close function!\n");
}

err_t client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	struct client_t *cs = NULL;

	LWIP_UNUSED_ARG(err);
	LWIP_ASSERT("arg != NULL",arg != NULL);
	cs = (struct client_t *)arg;

	if(!client_pcb) {
		cs->state = ES_CLOSING;
		return ERR_ABRT;
	}

	cs->state = ES_CONNECTED;

	tcp_recv(client_pcb, client_recv);
	tcp_poll(client_pcb, client_poll, 0);

	return ERR_OK;
}


err_t client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct client_t *cs;

	LWIP_UNUSED_ARG(len);

	cs = (struct client_t *)arg;
	cs->retries = 0;

	if(cs->p != NULL)
	{
		/* still got pbufs to send */
		tcp_sent(tpcb, client_sent);
		client_send(tpcb, cs);
	}
	else
	{
		/* no more pbufs to send */
		if(cs->state == ES_CLOSING)
		{
			client_close(tpcb, cs);
		}
	}
	return ERR_OK;
}


void client_send(struct tcp_pcb *tpcb, struct client_t *cs)
{
	struct pbuf *ptr;
	err_t wr_err = ERR_OK;

	while ((wr_err == ERR_OK) &&
		(cs->p != NULL) && 
		(cs->p->len <= tcp_sndbuf(tpcb)))
	{
		ptr = cs->p;

		/* enqueue data for transmission */
		wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
		if (wr_err == ERR_OK)
		{
			u16_t plen;
			u8_t freed;

			plen = ptr->len;
			/* continue with next pbuf in chain (if any) */
			cs->p = ptr->next;
			if(cs->p != NULL)
			{
				/* new reference! */
				pbuf_ref(cs->p);
			}
			/* chop first pbuf from chain */
			do
			{
				/* try hard to free pbuf */
				freed = pbuf_free(ptr);
			}
			while(freed == 0);

			/* we can read more data now */
			tcp_recved(tpcb, plen);
		}
		else if(wr_err == ERR_MEM)
		{
			/* we are low on memory, try later / harder, defer to poll */
			cs->p = ptr;
		}
		else
		{
			/* other problem ?? */
		}
	}
}


err_t client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	struct client_t *cs;
	err_t ret_err;

	LWIP_ASSERT("arg != NULL",arg != NULL);
	cs = (struct client_t *)arg;

	if(p == NULL)
	{
		cs->state = ES_CLOSING;
		if(cs->p == NULL) {
			client_close(tpcb, cs);
		} else {
			tcp_sent(tpcb, client_sent);
			client_send(tpcb, cs);
		}

		ret_err = ERR_OK;
	}
	else if(err != ERR_OK)
	{
		//不可能
		if(p != NULL)
		{
			cs->p = NULL;
			pbuf_free(p);
		}

		ret_err = err;
	}
	else if(cs->state == ES_CONNECTED)
	{
		cs->state = ES_RECEIVED;
		cs->p = p;

		tcp_sent(tpcb, client_sent);
		client_send(tpcb, cs);
		ret_err = ERR_OK;
	}
	else if(cs->state == ES_RECEIVED)
	{
		if(cs->p == NULL)
		{
			cs->p = p;
			tcp_sent(tpcb, client_sent);
			client_send(tpcb, cs);
		}
		else
		{
			struct pbuf *ptr = NULL;

			ptr = cs->p;
			pbuf_chain(ptr, p);
		}

		ret_err = ERR_OK;
	}
	else if(cs->state == ES_CLOSING)
	{
		tcp_recved(tpcb, p->tot_len);
		cs->p = NULL;
		pbuf_free(p);
		ret_err = ERR_OK;
	}
	else
	{
		/* unkown es->state, trash data  */
		tcp_recved(tpcb, p->tot_len);
		cs->p = NULL;
		pbuf_free(p);
		ret_err = ERR_OK;
	}

	return ret_err;
}


void client_error(void *arg, err_t err)
{
	struct client_t *cs = NULL;

	LWIP_UNUSED_ARG(err);

	cs = (struct client_t *)arg;
	if (cs != NULL)
	{
		mem_free(cs);
	}

	printf("err = %d\n", err);
}


err_t client_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct client_t *es;

	es = (struct client_t *)arg;
	if (es != NULL)
	{
		if (es->p != NULL)
		{
			/* there is a remaining pbuf (chain)  */
			tcp_sent(tpcb, client_sent);
			client_send(tpcb, es);
		}
		else
		{
			/* no remaining pbuf (chain)  */
			if(es->state == ES_CLOSING)
			{
				client_close(tpcb, es);
			}
		}
		ret_err = ERR_OK;
	}
	else
	{
		/* nothing to be done */
		tcp_abort(tpcb);
		ret_err = ERR_ABRT;
	}
	return ret_err;
}


err_t client_connect()
{
	if(!client_pcb) {
		return -1;
	}

	struct client_t *cs = (struct client_t*)mem_malloc(sizeof(struct client_t));
	if(!cs) {
		return -3;
	}

	cs->state = ES_BINDED;
	cs->pcb = client_pcb;
	cs->retries = 0;
	cs->p = NULL;

	tcp_arg(client_pcb, cs);
	tcp_err(client_pcb, client_error);

	//远程地址
	IP4_ADDR(&remote_addr, 192, 168, 1, 51);
	remote_port = 3000;

	err_t err = tcp_connect(client_pcb, &remote_addr, remote_port, client_connected);
	if(err != ERR_OK) {
		mem_free(cs);
		cs = NULL;
		return -4;
	}

	return 0;
}


int client_init(void)
{
	client_pcb = tcp_new();
	if(!client_pcb) {
		return -1;
	}

	err_t err = tcp_bind(client_pcb, IP_ADDR_ANY, 0);
	if(err != ERR_OK) {
		memp_free(MEMP_TCP_PCB, client_pcb);
		return -2;
	}

	return client_connect();
}