#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <nettinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080
#define THREADS 3
#define BACKLOG 10

pthread_t threads[THREADS];
int tasks[THREADS];
pthread_mutex_t Lock[THREADS];

int main()
{
	pool_init();
	start_server();
	pool_reset();
	return 0;
}

void start_server()
{
	int ld = socket(AF_INET, SOCK_STREAM, 0);
	if (ld = -1)
	{
		printf("Listener create error");
	}
	struct sockaddr_in serv_addr;
	//Создать структуру (port=htons(8080), family=AF_INET, s_add_r=INADDR)

}


