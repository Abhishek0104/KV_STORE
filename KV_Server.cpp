#include "KV.h"

#define MAX_CLIENT_PER_THREAD 2

int server_soc;

void sigterm_handler(int sig)
{
    printf("Got SIGTERM-Leaving\n");
    close(server_soc);
    exit(0);
}

int main()
{

    /*--SIGNAL HANDLER COMMANDS--*/
    signal(SIGINT, sigterm_handler);
    signal(SIGTERM, sigterm_handler);
    /*--------END_SIGNALS--------*/

    // Create a socket
    server_soc = socket(AF_INET, SOCK_STREAM, 0);

    // Reusable Socket
    int temp = 1;
    if(setsockopt(server_soc, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int)) < 0)
    {
        perror("Setsockopt failed.\n");
    }

    struct sockaddr_in client;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8084);
    addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_soc, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        perror("Address cannot bind.\n");
        exit(1);
    }

    // Listen for connection
    if (listen(server_soc, 10) != 0)
    {
        perror("Error in Listen.\n");
        exit(1);
    }
    printf("Server Connected..\n");
    // Thread Creation
    int no_threads = 10;
    thread_pool(no_threads);

    // Accept connections
    for(int i = 0; i < 20; i++)
    {
        memset(&client, 0, sizeof(client));
        int addrlen = sizeof(client);
        int fd = accept(server_soc, (struct sockaddr *)&client, (socklen_t *)&addrlen);
        printf("New Client accepted.\n");
        for(int k = 0; k < no_threads; k++)
        {
            if(worker_list[k]->no_client < 2)
            {
                printf("Goes to thread no. %d\n", k);
                pthread_mutex_lock(&(worker_list[k]->mutex));
                worker_list[k]->client_fd_queue.push_back(fd);
                pthread_cond_signal(&(worker_list[k]->cond));
                pthread_mutex_unlock(&(worker_list[k]->mutex));
                worker_list[k]->no_client++;
                break;
            }
        }
    }

    //close the server socket
    close(server_soc);
}

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
    int temp_client_count = 0;
    printf("I am thread %ld\n", index);

    char buffer[MAXBUF];

    // Epoll create
    struct epoll_event events[MAX_CLIENT_PER_THREAD];
    int epoll_fds = epoll_create(MAX_CLIENT_PER_THREAD);
    int nfds;


    pthread_mutex_lock(&(worker_list[index]->mutex));
    if(worker_list[index]->client_fd_queue.size() == 0)
        pthread_cond_wait(&(worker_list[index]->cond), &(worker_list[index]->mutex));
    pthread_mutex_unlock(&(worker_list[index]->mutex));
    //add to epoll list
    while(1)
    {
        if(temp_client_count < worker_list[index]->no_client)
        {
            static struct epoll_event ev;
            ev.data.fd = worker_list[index]->client_fd_queue[temp_client_count];
            ev.events = EPOLLIN;
            epoll_ctl(epoll_fds, EPOLL_CTL_ADD, ev.data.fd, &ev);
            temp_client_count++;
        }
        nfds = epoll_wait(epoll_fds, events, 5, 10000);
        for (int i = 0; i < nfds; i++)
        {
            memset(buffer, 0, MAXBUF);
            read(events[i].data.fd, buffer, MAXBUF);
            puts(buffer);
        }
    }
    pthread_exit(NULL);
}