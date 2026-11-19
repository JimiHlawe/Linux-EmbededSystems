# Sysmon - Phase 3

This phase uses separate processes and pipes instead of threads.

Sysmond starts three sensor processes using fork() and execve().

- mem_sensor - sends memory data using a pipe.
- cpu_sensor - sends CPU load using a FIFO.
- proc_sensor - sends process count using a FIFO.

Each sensor sends data every second, and sysmond reads and displays it.

## Makefile

- `make` - compiles all four programs.
- `make run` - starts sysmond and the sensors.
- `make clean` - removes compiled programs.

Press Ctrl+C to stop everything.

## Changes from Phase 2

- Separate processes instead of threads.
- Pipes and FIFOs instead of shared memory.
- Sysmond starts and stops the sensors automatically.
- No mutex or condition variables.
