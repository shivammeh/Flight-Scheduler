#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#include "ars.h"
#include "utils.h"

struct thread_info {
  pthread_t pt;
};


#define NF ((100))
#define NS ((5))

// NTXN must be a multiple of NF
#define NTXN ((NF * 100000))

void * agent_thread(struct thread_info * ti)
{
  (void)ti; // unused
  srandom_u64();
  // one ticket per flight
  struct ticket * buf = calloc(1, sizeof(buf[0]) * NF);
  for (int i = 0; i < NF; i++) {
    buf[i].tid = -1; // no ticket
  }

  for (int i = 0; i < NTXN; i++) {
    int fid = i % NF;
    if (buf[fid].tid >= 0) { // has ticket, cancel
      if (cancel_flight(buf[fid].uid, buf[fid].fid, buf[fid].tid) == false) {
        fprintf(stderr, "cancel_flight failed\n");
        pthread_exit(NULL);
      } else {
        buf[fid].tid = -1;
      }
    } else { // book
      short uid = random_u64() % NS;
      int tid = book_flight_can_wait(uid, fid);
      if (tid < 0) {
        fprintf(stderr, "book failed\n");
        pthread_exit(NULL);
      }
      buf[fid].uid = uid;
      buf[fid].fid = fid;
      buf[fid].tid = tid;
    }
  }
  free(buf);
  return NULL;
}

int
main(int argc, char ** argv)
{
  ars_init(NF, NS);
  // default is 1
  int nth = argc > 1 ? atoi(argv[1]) : 1;
  printf("%d threads\n", nth);

  const double t0 = time_sec();
  struct thread_info *tis = malloc(sizeof(tis[0]) * nth);
  // create <nth> workers and wait for them to finish
  for (int i = 0; i < nth; i++) {
    if (0 != pthread_create(&tis[i].pt, NULL, (void *)agent_thread, &tis[i])) {
      fprintf(stderr, "pthread_create failed\n");
      exit(0);
    }
  }
  // wait (and count their tickets)
  for (int i = 0; i < nth; i++) {
    pthread_join(tis[i].pt, NULL);
  }
  const double dt = time_sec() - t0;

  // system tickets
  int n = 0;
  struct ticket * tickets = dump_tickets(&n);
  assert(n == 0);

  printf("done threads %d time %.3lf\n", nth, dt);

  free(tickets);
  free(tis);
  exit(0);
}
