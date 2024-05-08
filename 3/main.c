#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_RECORDS 48000
#define NUM_THREADS 4
#define NUM_RECORDS_PER_THREAD (NUM_RECORDS / NUM_THREADS)

// #define ALWAYS_LOCK 1
#define FAKE_WORK usleep(1000)

/* Mock appplication data structure */
typedef struct {
  int value;
} record_t;

/* Check for matching record_t's */
int records_match(record_t *r1, record_t *r2) { return r1->value == r2->value; }

/* Arguments for each thread */
typedef struct {
  int tid;          /* Thread id */
  record_t *start;  /* Start record */
  int count;        /* Record count */
  record_t *target; /* Match Target */
} thread_args_t;

int match_count = 0;
pthread_mutex_t match_count_mutex;

void *find_matches(void *thread_args) {
  thread_args_t *args = (thread_args_t *)thread_args;

  printf("%d: %p[%d]\n", args->tid, (void *)args->start, args->count);

#if ALWAYS_LOCK
  for (int i = 0; i < args->count; i++) {
    int match = records_match(&args->start[i], args->target);

    if (match) {
      pthread_mutex_lock(&match_count_mutex);
      match_count++;
      FAKE_WORK;
      pthread_mutex_unlock(&match_count_mutex);
    }
  }
#else /* !ALWAYS_LOCK */
  int local_matches = 0;
  int busy_count = 0;
  for (int i = 0; i < args->count; i++) {
    int match = records_match(&args->start[i], args->target);

    if (match) {
      local_matches++;

      int rtn = pthread_mutex_trylock(&match_count_mutex);
      if (rtn == 0) {
        /* Got the lock */
        match_count += local_matches;
        FAKE_WORK;
        pthread_mutex_unlock(&match_count_mutex);
        local_matches = 0;
      } else {
        /* Lock busy */
        busy_count++;
      }
    }
  }

  printf("%d: busy %d times\n", args->tid, busy_count);

  if (local_matches > 0) {
    /* Not choice but to lock */
    pthread_mutex_lock(&match_count_mutex);
    match_count += local_matches;
    pthread_mutex_unlock(&match_count_mutex);
  }
#endif

  return (void *)NULL;
}

void check_thread_rtn(char *msge, int rtn) {
  if (rtn) {
    fprintf(stderr, "Error: %s (%d)\n", msge, rtn);
    exit(1);
  }
}

int blocking_rand(int min, int max) { return (random() % max) + min; }

int main(void) {
  pthread_t threads[NUM_THREADS];
  thread_args_t thread_args[NUM_THREADS];

  record_t records[NUM_RECORDS];
  record_t target_record = {7};
  int expected_count = 0;

  /* Initialize records */
  srandom(time(NULL));
  for (int i = 0; i < NUM_RECORDS; i++) {
    records[i].value = blocking_rand(1, 10);
    if (records_match(&records[i], &target_record)) {
      expected_count++;
    }
  }

  /* Initialize mutex */
  int rtn = pthread_mutex_init(&match_count_mutex, NULL);
  check_thread_rtn("mutex init", rtn);

  /* Fire up the threads */
  record_t *next_group = records;
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_args[i].tid = i;
    thread_args[i].start = next_group;
    thread_args[i].count = NUM_RECORDS_PER_THREAD;
    thread_args[i].target = &target_record;

    rtn = pthread_create(&threads[i], NULL, find_matches,
                         (void *)&thread_args[i]);
    check_thread_rtn("create", rtn);

    next_group += NUM_RECORDS_PER_THREAD;
  }
  /* Join all the child threads */
  for (int i = 0; i < NUM_THREADS; i++) {
    rtn = pthread_join(threads[i], NULL);
    check_thread_rtn("join", rtn);
  }

  /* Output results */
  if (match_count == expected_count) {
    printf("Matched %d as expected\n", match_count);
  } else {
    printf("Matched %d, expected %d\n", match_count, expected_count);
  }
}
