#include "KV.h"


void thread_pool(int no_workers)
{   
    for(long i = 0; i < no_workers; i++)
    {
        Worker_Thread *worker = (Worker_Thread*)malloc(sizeof(Worker_Thread));
        worker_list.push_back(worker);
        pthread_mutex_init(&(worker->mutex), NULL);
        pthread_cond_init(&(worker->cond), NULL);
        pthread_create(&(worker->thread), NULL, do_work, (void *) i);
    }
}
void * do_work(void * args)
{
    long index = (long) args;
    printf("I am thread %ld", index);
    pthread_exit(NULL);
}