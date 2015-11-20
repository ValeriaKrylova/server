#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
	int a = 10;

	pid_t fr = fork();

	if (fr == 0)
	{
		printf("Child : %d\n", a);
	}
	else
	{
		printf("Parent : %d\n", a);
	}


	return 0;
}
