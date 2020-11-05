#include "KV.h"
char server_buffer[MAXBUF];
int clients_available = 0;
int main()
{
    int check;
    //create a socket
    int server_soc_fd = socket(AF_INET, SOCK_STREAM, 0);

    //epoll
    struct epoll_event events[MAX_NUM_CLIENTS];
    int epoll_fds = epoll_create(MAX_NUM_CLIENTS);
    int addrlen, nfds;

    //mutex and condition variable initialization
    pthread_mutex_init(&queue_lock, NULL);
    pthread_cond_init(&queue_cond, NULL);

    struct sockaddr_in addr;
    struct sockaddr_in client;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8085);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_soc_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        perror("Address cannot bind.\n");
        exit(1);
    }

    if (listen(server_soc_fd, 10) != 0)
    {
        perror("Error in Listen.\n");
        exit(1);
    }

    printf("Waiting for connection...\n");

    for (int i = 0; i < 10; i++)
    {
        pthread_create(&worker_thread[i], NULL, work, NULL);
    }

    while (1)
    {
        if (clients_available < 1)
        {
            // while (1)
            // {
                static struct epoll_event ev;
                memset(&client, 0, sizeof(client));
                addrlen = sizeof(client);
                ev.data.fd = accept(server_soc_fd, (struct sockaddr *)&client, (socklen_t *)&addrlen);
                if (ev.data.fd == -1)
                {
                    printf("Breaking Out\n");
                    break;
                }
                else
                {
                    printf("CLient accepted\n");
                    clients_available++;
                    ev.events = EPOLLIN;
                    epoll_ctl(epoll_fds, EPOLL_CTL_ADD, ev.data.fd, &ev);
                }
            // }
        }
        nfds = epoll_wait(epoll_fds, events, 5, 10000);
        for (int i = 0; i < nfds; i++)
        {
            // add mutex and signal for the queue
            printf("Another msg comes\n");
            pthread_mutex_lock(&queue_lock);
            fd_queue.push(events[i].data.fd);
            pthread_cond_signal(&queue_cond);
            pthread_mutex_unlock(&queue_lock);
            // memset(buffer, 0, MAXBUF);
            // read(events[i].data.fd, buffer, MAXBUF);
            // puts(buffer);
        }
    }

    return 0;
}

void * work(void * arg)
{
    char buffer[MAXBUF];

    //check for element in queue if empty then sleep
    // use mutex and cond var
    while(1)
    {
        pthread_mutex_lock(&queue_lock);
        while(fd_queue.empty())
            pthread_cond_wait(&queue_cond, &queue_lock);
        int temp = fd_queue.front();
        fd_queue.pop();
        memset(buffer, 0, MAXBUF);
        read(temp, buffer, MAXBUF);
        puts(buffer);
        pthread_mutex_unlock(&queue_lock);
    }
    pthread_exit(NULL);
}
