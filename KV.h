#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <queue>
#include <list>
#include <netdb.h>
#include<bits/stdc++.h>

using namespace std;

#define MAXBUF 513
#define MAX_NUM_CLIENTS 5

//number of worker threads
pthread_t worker_thread[10];

//locks and condition variable for queue
pthread_mutex_t queue_lock;
pthread_cond_t queue_cond;

queue<int> fd_queue;

typedef struct Worker_Thread {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    vector<int> client_fd_queue;
    int no_client = 0;
} Worker_Thread;


//list for worker thread pointer
vector<Worker_Thread*> worker_list;

void* do_work(void * arg);
void thread_pool(int size);
struct Worker_Thread create_worker_thread();

int epoll_create(int size);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);


void * work(void * arg);
