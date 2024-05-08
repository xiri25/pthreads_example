#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4
#define BUMPS_PER_THREAD 5000

// Esto es solo para probar con y sin el mutex, comentado porque esta en el
// makefile
// #define USE_MUTEX 1

int shared_counter = 0;

// El mutex, una variable global
pthread_mutex_t shared_counter_mutex;

// Start routine of the thread tiene que tomar como argumento un void* y
// devolver un void*
void *bump_counter() {
  for (int i = 0; i < BUMPS_PER_THREAD; i++) {
#if USE_MUTEX
    pthread_mutex_lock(&shared_counter_mutex);
    shared_counter++;
    pthread_mutex_unlock(&shared_counter_mutex);
#else
    shared_counter++;
#endif
  }
  // Return NULL
  return (void *)NULL;
}

void check_thread_rtn(char *msge, int rtn) {
  if (rtn) {
    fprintf(stderr, "ERROR: %s (%d)\n", msge, rtn);
    exit(1);
  }
}

int main() {
  // Array de la struct pthread_t. los threads basicamente, creo
  pthread_t threads[NUM_THREADS];

  // Set up el mutex (le pasamos la direccion del mutex, que esta como variable
  // global) NULL para la configuraccion por defecto del MUTEX
  int rtn = pthread_mutex_init(&shared_counter_mutex, NULL);
  // Check the return value
  check_thread_rtn("mutex init", rtn);

  for (int i = 0; i < NUM_THREADS; i++) {
    // Creacion del thread (pointer a un pthreads object, es un pointer para
    // poder hacer cambios) 2arg son atributos para el thread , NULL para
    // defecto 3arg funcion 4arg arg para la funcion
    rtn = pthread_create(&threads[i], NULL, bump_counter, NULL);
    check_thread_rtn("create", rtn);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    // Esperar a los nucleos
    // 1arg el thread, ahora no la direccion porque no necesita hacer cambios
    // 2arg NULL es el  valor que se devuelve por el thread
    // El join se hace por orden, indepedientemente de el orden el que acaban
    rtn = pthread_join(threads[i], NULL);
    check_thread_rtn("join", rtn);
  }

  int expected_value = NUM_THREADS * BUMPS_PER_THREAD;
  int exit_value = 0;
  if (shared_counter == expected_value) {
    printf("Ended with %d as expected\n", expected_value);
    // why not printf(..., shared_counter)
  } else {
    printf("Expected %d, got %d - no workie\n", expected_value, shared_counter);
    exit_value = 1;
  }

  exit(exit_value);
}
