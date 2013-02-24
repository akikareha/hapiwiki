#include <stdio.h>

extern char **environ;

int main(void)
{
	char **s;

	printf("Content-Type: text/plain\n");
	printf("\n");

	for (s=environ; *s!=NULL; s++) {
		printf("%s\n", *s);
	}

	return 0;
}
