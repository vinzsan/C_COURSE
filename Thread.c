#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *thread_main(void *args){
  char *str = (void *)args;
  printf("%s", str);
  free(str);
  return NULL;
}

int main(){
  pthread_t tid;
  pthread_mutex_lock(&lock);
  char *str = "Hello world\n";
  void *buff = malloc(strlen(str) * sizeof(char) + 1);
  strcpy(buff, str);
  pthread_mutex_unlock(&lock);
  pthread_create(&tid, NULL, thread_main, (void *)buff);
  pthread_join(tid,NULL);
  return 0;
}