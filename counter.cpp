#include "kclock.h"
#include "SimpleList.h"
#include "TimerOne.h"

SimpleList<Counter*> counters;

void add_counter(Counter *counter) {
  counters.push_back(counter);
}



void send_tick() {
  for (SimpleList<Counter *>::iterator itr = counters.begin(); itr != counters.end(); ++itr) {
    Counter *counter = (Counter *)*itr;
    counter->tick();
  }
}




void setup_counters() {
  Timer1.initialize(20000);
  Timer1.attachInterrupt( send_tick );
}
