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
  int ntxn; // number of operations to do
  int ntickets;
  struct ticket * buf;
};

#define NF ((100))
#define NS ((100))
void * agent_thread(struct thread_info * out)
{
  srandom_u64();
  struct ticket * buf = malloc(sizeof(buf[0]) * NF * NS);
  int n = 0;
  for (int i = 0; i < out->ntxn; i++) {
    int o = random_u64() % (NF * NS);
    if (o < n) { // change or cancel
      if (random_u64() & 0x20000) { // test a bit (any bit), change if it's 1
        short newfid = random_u64() % NF;
        int newtid = change_flight(buf[o].uid, buf[o].fid, buf[o].tid, newfid);
        if (newtid >= 0) { // changed
          buf[o].fid = newfid;
          buf[o].tid = newtid;
        }
      } else { // cancel
        if (cancel_flight(buf[o].uid, buf[o].fid, buf[o].tid) == false) {
          fprintf(stderr, "cancel_flight failed\n");
          pthread_exit(NULL);
        }
        buf[o] = buf[n-1];
        n--;
      }
    } else { // book
      short uid = random_u64() % NS;
      short fid = random_u64() % NF;
      int tid = book_flight(uid, fid);
      if (tid >= 0) {
        buf[n].uid = uid;
        buf[n].fid = fid;
        buf[n].tid = tid;
        n++;
      }
    }
  }
  // let main process the tickets
  out->ntickets = n;
  out->buf = buf;
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
    tis[i].ntxn = 10000000;
    if (0 != pthread_create(&tis[i].pt, NULL, (void *)agent_thread, &tis[i])) {
      fprintf(stderr, "pthread_create failed\n");
      exit(0);
    }
  }
  // wait (and count their tickets)
  int m = 0;
  for (int i = 0; i < nth; i++) {
    pthread_join(tis[i].pt, NULL);
    m += tis[i].ntickets;
  }
  const double dt = time_sec() - t0;

  // compare the system tickets and the worker tickets
  // they should match exactly

  // system tickets
  int n = 0;
  struct ticket * tickets = dump_tickets(&n);
  tickets_sort(tickets, n);
  if (m != n) {
    fprintf(stderr, "ntickets mismatch %d %d\n", m, n);
    exit(0);
  }

  // check tickets returned from workers
  struct ticket * tickets2 = malloc(sizeof(*tickets2) * m);
  m = 0;
  for (int i = 0; i < nth; i++) {
    memcpy(tickets2+m, tis[i].buf, sizeof(*tickets2) * tis[i].ntickets);
    free(tis[i].buf);
    m += tis[i].ntickets;
  }
  assert(m == n);
  tickets_sort(tickets2, m);

  if (memcmp(tickets, tickets2, sizeof(u64) * m)) {
    fprintf(stderr, "tickets mismatch\n");
    exit(0);
  }
  printf("done threads %d tickets %d time %.3lf\n", nth, m, dt);

  free(tickets);
  free(tickets2);
  free(tis);
  exit(0);
}
