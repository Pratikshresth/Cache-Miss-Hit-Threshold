#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

// Arrays used for calibration
size_t array[5*1024];
size_t hit_histogram[600];
size_t miss_histogram[600];

// Utility functions
void maccess(void* addr) {
    asm volatile("movl (%0), %%eax" : : "c"(addr) : "eax");
}

void clflush(void* addr) {
    asm volatile("mfence\n\tclflush (%0)" : : "r"(addr));
}

uint64_t rdtsc() {
    uint32_t a, d;
    asm volatile("mfence\n\t"
                 "rdtsc\n\t"
                 "mov %%edx, %0\n\t"
                 "mov %%eax, %1\n\t"
                 "mfence"
                 : "=r"(d), "=r"(a) :: "eax", "edx");
    return ((uint64_t)d << 32) | a;
}

size_t onlyreload(void* addr) {
    size_t time = rdtsc();
    maccess(addr);
    return rdtsc() - time;
}

size_t flushandreload(void* addr) {
    size_t time = rdtsc();
    maccess(addr);
    clflush(addr);
    return rdtsc() - time;
}

int getThreshold() {
    memset(array, -1, 5*1024*sizeof(size_t));
    maccess(array + 2*1024);
    sched_yield();
    
    for (int i = 0; i < 12*1024*1024; ++i) {
        size_t d = onlyreload(array + 2*1024);
        hit_histogram[d < 599 ? d : 599]++;
    }
    
    clflush(array + 2*1024);
    
    for (int i = 0; i < 12*1024*1024; ++i) {
        size_t d = flushandreload(array + 2*1024);
        miss_histogram[d < 599 ? d : 599]++;
    }
    
    size_t hit_max = 0, hit_max_i = 0, miss_min_i = 0;
    
    for (size_t i = 0; i < 600; ++i) {
        if (hit_max < hit_histogram[i]) {
            hit_max = hit_histogram[i];
            hit_max_i = i;
        }
        if (miss_histogram[i] > 3 && miss_min_i == 0)
            miss_min_i = i;
    }
    
    size_t min = (size_t)-1;
    size_t min_i = 0;
    
    for (int i = hit_max_i; i < miss_min_i; ++i) {
        if (min > (hit_histogram[i] + miss_histogram[i])) {
            min = hit_histogram[i] + miss_histogram[i];
            min_i = i;
        }
    }
    
    return min_i;
}

int main() {
    int threshold = getThreshold();
    printf("Calculated Threshold: %d\n", threshold);
    return 0;
}
