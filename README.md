
# Sysmon - Phase 2

This phase uses threads instead of separate processes.

The three sensor threads read memory, CPU load and process information from /proc.

The data is saved in a shared struct instead of files.

I used a mutex to protect shared data and a condition variable for CPU load alerts above 1.50.

## Makefile

- `make` - compiles the program.
- `make run` - runs sysmond.
- `make clean` - removes compiled files.

Press Ctrl+C to stop the program.

## Changes from Phase 1

- One process with five threads.
- Shared memory instead of .dat files.
- Mutex to protect shared data.
- Alert thread for high CPU load.
