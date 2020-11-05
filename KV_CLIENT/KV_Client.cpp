#include "KV.h"
int sock_fd;

void sigterm_handler(int sig)
{
    printf("Got SIGTERM-Leaving\n");
    close(sock_fd);
    exit(0);
}

int main(int argc, char** argv)
{
    /*--SIGNAL HANDLER COMMANDS--*/
    signal(SIGINT, sigterm_handler);
    signal(SIGTERM, sigterm_handler);
    /*--------END_SIGNALS--------*/
	int s;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    int temp = 1;
    if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int)) < 0)
    {
        perror("Setsockopt failed.\n");
    }
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
    addr.sin_port = htons(8084);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr));

	printf("Client Connected\n");

	char buffer[10];
	while(fgets(buffer, 10, stdin) != NULL){
		printf("SENDING: %s", buffer);
		printf("===\n");
		write(sock_fd, buffer, strlen(buffer));

		memset(buffer, 0, sizeof(buffer));
		// char resp[1000];
		// int len = read(sock_fd, resp, 999);
		// resp[len] = '\0';
		// printf("%s\n", resp);
	}
    close(sock_fd);
    return 0;
}
