#include <stdio.h>
#include <time.h>

int main(void) {
  time_t t;

  t = time(NULL);

  printf("Content-Type: text/plain\n");
  printf("\n");

  printf("%s", asctime(localtime(&t)));

  return 0;
}
