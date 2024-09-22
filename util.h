#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

#ifndef UTIL_H_
#define UTIL_H_

#ifndef HIDEMINMAX
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#endif

// Arrays used for calibration
size_t array[5*1024];
size_t hit_histogram[600];
size_t miss_histogram[600];

void clflush(void* addr);
void maccess(void* addr);
int getThreshold();
uint64_t rdtsc();

#endif
