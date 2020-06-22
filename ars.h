#pragma once

#include <stdbool.h>

struct ticket {
  short uid; // user
  short fid; // flight
  int tid; // ticket number (unique to this flight)
};

// helper function
extern void tickets_sort(struct ticket * ts, int n);

// call this from main() to initialize the system with the given parameters
extern void ars_init(int nr_flights, int nr_seats_per_flight);

// if availalbe, assign a seat to the user and a ticket number will be returned
// you must use the same user_id to board, change, or cancel a flight
// return -1 if the flight is full
// one user can buy multiple tickets by calling this function multiple times
extern int book_flight(short user_id, short flight_number);


// if the information matches the record in the system, the ticket will be cancelled
// otherwise, return false
extern bool cancel_flight(short user_id, short flight_number, int ticket_number);

// If a seat is available on the target flight,
// a new ticket id will be issued and
// the old ticket will be cancelled at the same time.
// Otherwise, return -1 and the original ticket id shall remain valid
// corner case: if old_flight_number == new_flight_number, return -1
extern int change_flight(short user_id, short old_flight_number, int old_ticket_number, short new_flight_number);

// return all tickets in the system to the caller in a malloc-ed buffer
struct ticket * dump_tickets(int * n_out);

// similar to book_flight() but it won't immediately return if the flight has been fully booked.
// instead, it block-waits until a ticket is released by another traveler (cancel/change)
extern int book_flight_can_wait(short user_id, short flight_number);
