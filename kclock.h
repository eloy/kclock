#ifndef KCLOCK_H
#define KCLOCK_H

#define TICKER_FREQ 20
#define TIME_STR_LEN 20
#include <stdio.h>
#include <Arduino.h>


extern char DEBUG[512];
#define PRINT(...) snprintf(&DEBUG[0], 512, __VA_ARGS__); Serial.println(&DEBUG[0]);


#define max(a,b)                                \
  ({ __typeof__ (a) _a = (a);                   \
    __typeof__ (b) _b = (b);                    \
    _a > _b ? _a : _b; })

#define min(a,b)                                \
  ({ __typeof__ (a) _a = (a);                   \
    __typeof__ (b) _b = (b);                    \
    _a > _b ? _b : _a; })



class Counter {
 public:
 Counter(): duration(0) {
  }

  void tick() {
    this->duration += TICKER_FREQ;
  }

  void print(char *buffer) {
    unsigned long d = this->duration / 1000;

    int seconds = d % 60;
    int minutes = (d / 60) % 60;
    int hours   = (d / (60*60)) % 24;

    snprintf(buffer, TIME_STR_LEN, "%02i:%02i:%02i", hours, minutes, seconds);
  }

 private:
  unsigned long duration;
};


void setup_counters();
void add_counter(Counter *counter);
#endif //KCLOCK_H