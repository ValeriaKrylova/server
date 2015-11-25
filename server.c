#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080
#define BACKLOG 10
#define THREADS 3

pthread_t	threads[THREADS];
int		tasks[THREADS];
pthread_mutex_t	locks[THREADS];
pthread_t reset;
int resId = -1;
void connection_processor(int cd)
{
		printf("Socket acepted [%d]\n", cd);
				
		while (1)
		{
			char *msg = "User@ssh$ \0";
			send(cd, msg, strlen(msg), 0);			
			
			// Принятие команды
			char cmd[1000];
			memset(cmd, '\0', sizeof(cmd));
			recv(cd, &cmd, sizeof(cmd), 0);
			cmd[strlen(cmd) - 2] = '\0';	// Удалить \r\n

			// Печать команды на сервер
			printf("[command]: %s\n", cmd);

			// Перечень команд
			if (strcmp(cmd, "hello\0") == 0)
			{				
				char *msg = "Hello!\r\n\0";
				send(cd, msg, strlen(msg), 0);
			}
			else if (strcmp(cmd, "help\0") == 0)
			{
				char *msg = "help,hello,ls,exit...\r\n\0";
				send(cd, msg, strlen(msg), 0);
				
			}
			else if (strcmp(cmd, "exit\0") == 0)
			{
				// Отключение
				char *msg = "Close!\r\n\0";
				send(cd, msg, strlen(msg), 0);
				
				close(cd);
				break;
			}
			else
			{
				// Остальные команды (ls...)
				FILE *fp = popen(cmd, "r");
				if (fp == NULL) {
				char *msg = "Failed to run command\r\n\0";
				send(cd, msg, strlen(msg), 0);
				}
	
				// Чтение, печать в сокет
				char buf[1000];
				while (fgets(buf, sizeof(buf), fp) != NULL) {
				send(cd, buf, strlen(buf), 0);
				}
	
				pclose(fp);
			}	
		}
		
		printf("Socket closed [%d]\n", cd);
}

void *thread_function(void *arg)
{
	int id = (int)arg;
		
	printf("[info]: Thread %d waiting for task\n", id);			

	pthread_mutex_lock(&locks[id]);
	pthread_mutex_lock(&locks[id]);
	pthread_mutex_unlock(&locks[id]);
		
	printf("[info]: Thread %d start task\n", id);
		
	// Выполнить
	connection_processor(tasks[id]);
	resId = id;
	
	return 0;
}
void *thread_reset()
{	
	printf("[info]: Thread %d waiting for reset\n", resId);			
	while(1){
		if(resId != -1){
			pthread_kill(threads[resId], 0);
			printf("[info]: Thread %d destroyed\n", resId);
			pthread_mutex_destroy(&locks[resId]);

			pthread_mutex_init(&locks[resId], NULL);
			pthread_create(&threads[resId], NULL, thread_function, (void *)(long long)resId);
			printf("[info]: Thread %d created\n", resId);
			resId = -1;
		}
	}	
	return 0;
}

void pool_give_task(int task)
{
	int i;
	for (i = 0; i < THREADS; i++)
	{
		if (pthread_mutex_trylock(&locks[i]))	//Если закрыт, то не можем залочить
		{
			tasks[i] = task;
			pthread_mutex_unlock(&locks[i]);
			return;
		}
		else	//Если был открыт, то открыть еще раз
		{
			pthread_mutex_unlock(&locks[i]);
		}
	}
	
	close(task);	//Закрыть подключение
	printf("[info]: All threads are bisy.\n");
}

void start_server()
{
	int ld = socket(AF_INET, SOCK_STREAM, 0);
	if (ld == -1)
	{
		printf("[error]: Listener create error\n");
	}

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	
	socklen_t size_saddr;
	
	int bindRes = bind(ld, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (bindRes == -1)
	{
		printf("[error]: Bind error\n");
	}
	
	int listenRes = listen(ld, BACKLOG);
	if (listenRes == -1)
	{
		printf("[error]: Listen error\n");
	}
	
	while (1)
	{
		struct sockaddr_in client_addr;
		socklen_t size_caddr;
		
		int cd = accept(ld, (struct sockaddr *)&client_addr, &size_caddr);
		if (cd == -1)
		{
			printf("[error]: Accept error\n");
		}
		
		pool_give_task(cd);
	}
}

int main()
{	
	int i;
	for(i = 0; i < THREADS; i++)
	{
		pthread_mutex_init(&locks[i], NULL);
		pthread_create(&threads[i], NULL, thread_function, (void *)(long long)i);
	}
	pthread_create(&reset, NULL, thread_reset, NULL);
	start_server();
	
	for(i = 0; i < THREADS; i++)
	{
		pthread_kill(threads[i], 0);
		pthread_mutex_destroy(&locks[i]);
	}
	pthread_kill(&reset,0);
	return 0;
}
