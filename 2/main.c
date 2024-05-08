#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

pthread_mutex_t global_min_lock;

// Para poder hacer INT_MAX se usa limits.h
int global_min = INT_MAX;

// El nombre del tipo es el que yo quiera, creo
typedef struct {
  int tid;       // thread id, es arbitraria, la que yo quiera, es por debugging
  int *list_ptr; // pointer al primer elemento del array
  long list_size; // es un long porque la lista es muy larga
  int local_min;
} thread_args_t;

// la funcion tiene que recibir un solo argumento (void *) y retornar un (void
// *) Como son void *, en realida se puede pasar lo que quieres
void *find_min(void *thread_args) {
  // Hay que castear el void *thread_args a un pointer de tipo thread_args_t
  thread_args_t *args = (thread_args_t *)thread_args;
  // Comp args es un pointer a una struct hay que usar ->
  int *list_ptr = args->list_ptr;
  // List size en este caso no tiene porque ser un long porque es mas pequeña
  // que la lista grande?????
  int list_size = args->list_size;
  int local_min = INT_MAX;

  for (int i = 0; i < list_size; i++) {
    if (list_ptr[i] < local_min) {
      local_min = list_ptr[i];
    }
  }

  // Ahora ya sabemos el local_min, por lo que actualizamos el global_min
  // No se si el lock se puede meter en el if o no, y solo leer el global_min
  // antes, sin poder cambiarlo
  pthread_mutex_lock(&global_min_lock);
  if (local_min < global_min) {
    global_min = local_min;
  }
  pthread_mutex_unlock(&global_min_lock);

  printf("p%02d: %d integers at %p: %d\n", args->tid, list_size,
         (void *)list_ptr, local_min);

  // Esto no es necesario porque ya se está actualizando el global_min
  args->local_min = local_min;
  // Devolmemos la struct
  pthread_exit(args); // Esto es para salir solo del thread
  // se puede hacer tambien con 'return args'
}

#define NUM_THREADS 4

// 16 * sizeof(int) KBytes
#define SUBLIST_SIZE (16 * 1024)
#define LIST_SIZE (SUBLIST_SIZE * NUM_THREADS)
#define ONE_BILLION 1000000000.0

void check_thread_rtn(char *msge, int rtn) {
  if (rtn) {
    fprintf(stderr, "Error: %s (%d)\n", msge, rtn);
    exit(1);
  }
}

double now(void) {
  struct timespec current_time;
  clock_gettime(CLOCK_REALTIME, &current_time);
  return current_time.tv_sec + (current_time.tv_nsec / ONE_BILLION);
}

int main(void) {

  // Array Estructura de los hilos pthread
  pthread_t threads[NUM_THREADS];

  // Array Estructura que se pasa como argumento a cada hilo
  thread_args_t thread_args[NUM_THREADS];

  // Lista a ordenar
  int big_list[LIST_SIZE];

  // Crear una seed
  // Podemos poner una seed fija para que siempre sean los mismos numeros
  srandom(time(NULL));
  // Inicializar la lista de numeros
  for (int i = 0; i < LIST_SIZE; i++) {
    // Divide entre 5 para hacer los numeros mas pequeños
    big_list[i] = random() / 5;
  }

  // Hay que iniciar el mutex
  pthread_mutex_init(&global_min_lock, NULL);

  double start_time = now();

  // No hace falta &, porque big_list es un array
  int *next_sublist = big_list;
  for (int i = 0; i < NUM_THREADS; i++) {
    // El punto porque es una struct, un elemento (struct) del array
    thread_args[i].tid = i;
    // next_sublist ya es un pointer
    thread_args[i].list_ptr = next_sublist;
    thread_args[i].list_size = SUBLIST_SIZE;
    thread_args[i].local_min = -1;
    // Nunca va a ser negativo por como se han inicializado

    // Hemos añadido SUBLIST_SIZE al pointer que era un int*, por eso no hay que
    // multiplicar por sizeof(int)
    next_sublist += SUBLIST_SIZE;

    // Hay que pasar la direccion de la struct porque esta funcion necesita
    // poder escribir en ella Default attr of the thread La funcion Los
    // argumentos de la funcion, la direccion porque la funcion espera un
    // pointer porque va a escribir en esa struct, creo
    int rtn = pthread_create(&threads[i], NULL, find_min, &thread_args[i]);
    check_thread_rtn("create", rtn);
  }

  // Aqui se recogen por orden
  for (int i = 0; i < NUM_THREADS; i++) {
    // Struct para guardar lo que devuelve el thread, en este caso la struct
    // entera
    thread_args_t *thread_rtn;
    // Le pasamos un pointer donde poner lo que recogemos del thread
    // La direccion de la struct donde guardar los datos (pointer to void *
    // (pointer to a generic pointer))
    int rtn = pthread_join(threads[i], (void *)&thread_rtn);
    check_thread_rtn("join", rtn);
    // Como thread_rtn es un pointer hay quye usar ->
    printf("Thread %d returned %d\n", i, thread_rtn->local_min);
  }

  printf("Global minimum %d\n", global_min);
  printf("Run time %.9fs\n", now() - start_time);
}
