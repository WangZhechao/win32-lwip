#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__


typedef void* sys_sem_t;
typedef struct queue *sys_mbox_t;
typedef void* sys_thread_t;

#define SYS_MBOX_NULL   (sys_mbox_t)0
#define SYS_SEM_NULL    (sys_sem_t)0

/* let sys.h use binary semaphores for mutexes */
#define LWIP_COMPAT_MUTEX 1


#endif
