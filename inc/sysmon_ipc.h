
#ifndef SYSMON_IPC_H
#define SYSMON_IPC_H

#define CPU_FIFO_PATH "/tmp/sysmon_cpu.fifo"
#define PROC_FIFO_PATH "/tmp/sysmon_proc.fifo"

// Memory data
typedef struct {
    long mem_total;
    long mem_free;
    long mem_avail;
    long swap_total;
    long swap_free;
} MemData;

// CPU data
typedef struct {
    double load1;
    double load5;
    double load15;
} CpuData;

// Process data
typedef struct {
    int total_procs;
} ProcData;

#endif
