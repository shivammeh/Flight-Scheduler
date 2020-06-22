// force assert()
#if defined(NDEBUG)
#undef NDEBUG
#endif

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


int main(void)
{
  ars_init(2, 2); // two flights, each has two seats
  int t00 = book_flight(0, 0);
  assert(t00 > 0);
  int t01 = book_flight(0, 1);
  assert(t01 > 0);
  int t10 = book_flight(1, 0);
  assert(t10 > 0);
  int t11 = book_flight(1, 1);
  assert(t11 > 0);

  // full; should fail
  assert(book_flight(0, 0) < 0);
  assert(book_flight(0, 1) < 0);
  assert(book_flight(1, 0) < 0);
  assert(book_flight(1, 1) < 0);
  assert(book_flight(0, 2) < 0); // bad flight number

  // full; any change_flight should fail
  assert(change_flight(0, 0, t00, 1) < 0);
  assert(change_flight(0, 1, t01, 0) < 0);
  assert(change_flight(1, 0, t10, 1) < 0);
  assert(change_flight(1, 1, t11, 0) < 0);
  assert(change_flight(2, 2, t00, 1) < 0); // bad flight number

  // should still fail
  assert(book_flight(0, 0) < 0);
  assert(book_flight(0, 1) < 0);
  assert(book_flight(1, 0) < 0);
  assert(book_flight(1, 1) < 0);
  assert(book_flight(0, 2) < 0); // bad flight number

  // cancel bad tickets (should fail)
  assert(false == cancel_flight(2, 0, 1)); // bad user id
  assert(false == cancel_flight(0, 2, 1)); // bad flight number
  assert(false == cancel_flight(0, 0, 100));
  assert(false == cancel_flight(1, 0, 100));
  assert(false == cancel_flight(0, 1, 100));
  assert(false == cancel_flight(1, 1, 100));

  // cancel real tickets
  assert(cancel_flight(0, 0, t00));
  assert(cancel_flight(0, 1, t01));
  assert(cancel_flight(1, 0, t10));
  assert(cancel_flight(1, 1, t11));

  // book again
  t00 = book_flight(0, 0);
  assert(t00 > 0);
  t01 = book_flight(0, 1);
  assert(t01 > 0);
  t10 = book_flight(1, 0);
  assert(t10 > 0);

  // change between the two flights
  int t = change_flight(0, 0, t00, 1);
  assert(t > 0);
  t = change_flight(0, 1, t, 0);
  assert(t > 0);
  t = change_flight(0, 0, t, 1);
  assert(t > 0);
  t = change_flight(0, 1, t, 0);
  assert(t > 0);

  // t00 must be invalid
  assert(false == cancel_flight(0, 0, t00));
  // cancel all valid tickets
  assert(cancel_flight(0, 0, t));
  assert(cancel_flight(0, 1, t01));
  assert(cancel_flight(1, 0, t10));

  int n = 0;
  (void)dump_tickets(&n);
  assert(n == 0);

  printf("no error\n");
}
