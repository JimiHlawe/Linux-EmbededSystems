#ifndef SYSMON_H
#define SYSMON_H

#include <pthread.h>
#include <signal.h>

extern volatile sig_atomic_t running;
// Shared system information
typedef struct {

    // Memory
    long mem_total;
    long mem_free;
    long mem_avail;
    long swap_total;
    long swap_free;

    // CPU
    double load1;
    double load5;
    double load15;

    // Processes
    int total_procs;
    int alert_pending;

    // Thread synchronization
    pthread_mutex_t lock;
    pthread_cond_t alert_cond;

} SystemState;

// Thread functions
extern void *mem_sensor_thread(void *arg);
extern void *cpu_sensor_thread(void *arg);
extern void *proc_sensor_thread(void *arg);
extern void *alert_thread(void *arg);

#endif
