#ifndef _MYHEAD_H
#define _MYHEAD_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <dirent.h>
#include <stropts.h>

#include <pthread.h>
#include <semaphore.h>


//探月车
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>



#define MAX_WAITING_TASKS	1000
#define MAX_ACTIVE_THREADS	20

struct task
{
	void *(*do_task)(void *arg);
	void *arg;

	struct task *next;
};

typedef struct thread_pool
{
	pthread_mutex_t lock;
	pthread_cond_t  cond;

	bool shutdown;

	struct task *task_list;

	pthread_t *tids;

	unsigned max_waiting_tasks;
	unsigned waiting_tasks;
	unsigned active_threads;
}thread_pool;

//一个结构体 存放文件路径和复制后路径的
struct file   
{  
    char causefile[4096];  
    char copyfile[4096];  
};  

bool init_pool(thread_pool *pool, unsigned int threads_number);
bool add_task(thread_pool *pool, void *(*do_task)(void *arg), void *task);
int  add_thread(thread_pool *pool, unsigned int additional_threads_number);
int  remove_thread(thread_pool *pool, unsigned int removing_threads_number);
bool destroy_pool(thread_pool *pool);

void *routine(void *arg);

//保存起始存储时间以及拷贝完毕后的时间
time_t copy_start_time;
time_t copy_end_time;

#endif












